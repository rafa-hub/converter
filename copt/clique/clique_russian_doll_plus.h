////////////////////////////////
// clique_russian_doll_plus.h: interface for CliqueDollPlus class which tests iterative flow based algorithms for MCP as incMaxCLQ
//							   uses improved InfraOpPlus inference engine for P-MaxSAT
//                
// initial date: 27/07/16
// last update: 26/09/16
// author: pablo san segundo


#ifndef  __CLIQUE_RUSSIAN_DOLL_PLUS_H__
#define  __CLIQUE_RUSSIAN_DOLL_PLUS_H__

#include "clique.h"
#include "../init_color_ub.h"
#include "../amts/amts_exec.h"
#include "infra_tools.h"				//vertex driven infra-chrom inferences
#include "infra_tools_plus.h"	

class CliqueDollPlus:public Clique<ugraph>{
   static const int ROOT_NODE=-1;
   static const int HEAD_COLOR=0;				
public:
	
	CliqueDollPlus(ugraph* g, param_t p)				:Clique<ugraph>(g, p) {};
	CliqueDollPlus(param_t p)							:Clique<ugraph>(p)  {};
	virtual ~CliqueDollPlus(){}
	InfraOpPlus<ugraph, bitarray> & get_iop()  {return iop;}
						
	virtual int init_others();

	//new painting functions only dedicated to find UB, not branching 
	int paint_UB(const bitarray&);									//IS coloring (for bitstring encoding of subgraphs)
	int paint_UB_R(const bitarray&, int maxac);						//*** CHECK: POSSIBLE BUG-14/10/2016

	//new incmaxsatz attemtps
	int solve_first_nodes(int depth, nodelist_t& l_v);
	int solve_first_nodes_incMaxCLQ(int depth, nodelist_t& l_v);
	int estimate_ub_first_nodes(int old_ub, int node, int pos_node, nodelist_t& l_v);
			
	//tested search procedures
	int expand_doll (int maxac, nodelist_t& l_v);					//driver for bitstring imlpementation (main variant)
	int expand_doll_R (int maxac, nodelist_t& l_v);	
	int expand_doll_R_no_pmaxsat (int maxac, nodelist_t& l_v);		//for tests and initial RLF sorting
	
	//interface for induced subgraph computation
	void run_subgraph(bitarray& bbs);	
			
	//test framework
	virtual	int set_up();
	virtual	void run();	
	virtual	void tear_down(){ Clique<ugraph>::tear_down();}

private:
	InfraOpPlus<ugraph, bitarray> iop;
	bitarray bbchild;
};

inline
void CliqueDollPlus::run_subgraph(bitarray& bbs){
////////////////
// runs normal search for the subgraph passed (non-unrolling of first level)
// first update: 9/11/16
//
//
// REMARKS: assumes setup (initial bounds, solution) and allocation are done

	maxno=get_result().get_lower_bound();

	//set list of nodes at root as nodes in bbs
	bbs.init_scan(bbo::NON_DESTRUCTIVE);
	Clique<ugraph>::m_lroot.index=0;	
	while(true){
		int v=bbs.next_bit();
		if(v==EMPTY_ELEM) break;
		 Clique<ugraph>::m_lroot.nodos[ Clique<ugraph>::m_lroot.index++]=v;
	}
			
	//algorithm selection
	switch(param.alg){
	case BBMC_DOLL:
		expand_doll(0, Clique<ugraph>::m_lroot);
		break;
	case BBMCR_DOLL:
		expand_doll_R(0, Clique<ugraph>::m_lroot);
		break;	
	case BBMCR_NOX_DOLL:
		expand_doll_R_no_pmaxsat(0, Clique<ugraph>::m_lroot);
		break;	
	default:
		LOG_ERROR("run-non_unrolled:unknown clique algorithm");
	}

	res.set_UB(maxno);
}	

inline
int CliqueDollPlus::init_others(){

	Clique<ugraph>::init_others();
	
	//empty set
	iop.set_graph(g);
	iop.init(m_alloc);							/* check allocation space */
	//iop.init(g->number_of_vertices());
	m_bbroot.erase_bit();
	Clique<ugraph>::m_lroot.index=m_size;		//contains number of elements, not num_elem-1 as in other variants
	bbchild.init(m_size);
	bbchild.set_bit(0, m_size-1);				/* recent improvement for solve_first_nodes function */

	return 0;
}

inline
int CliqueDollPlus::set_up(){
//////////////
// allocates memory, evaluates initial bounds and determines 
// initial trivial solutions
//
// RETURN VALUE: -1 Error, 0-ok, >0 trivial solution found

	int sol=0;
	res.clear();
	res.set_name(g->get_name());

	//determine allocation info
	search_alloc_t info;							//loads BBMC configuration for search allocation
	info.set(search_alloc_t::ALLOC_COLOR_SETS);		//for recoloring
	switch(param.alg){
	case BBMC:
	case BBMCR:
	case BBMC_DOLL:
	case BBMC_DOLL_LISTS:
	case BBMCR_DOLL:
	case BBMCR_DOLL_LISTS:
		break;
	default:
		LOG_ERROR("CliqueDollPlus::setup unknown algorithm");
		return -1;
	}
	
	//actual set_up
	if(param.unrolled){
		LOG_ERROR("CliqueDollPlus::setup unrolled variant undefined");
		return -1;
	}else{
		if( (sol=set_up_non_unrolled(info))>0 ){
				LOG_INFO("[w="<<sol<<"]"<<" TRIVIALLY SOLVED DURING PRECOMPUTATION");
				res.set_UB(sol);
		}else{ 
			//Trivial solution not found
			//*** additional extra-initialization non_unrolled case ***
		
			//UB
			if(param.init_preproc!=HEUR){
				LOG_PRINT("COMPUTING UB");
				InitColorUB c(*g);
#ifdef ENHANCED_INIT_UB
				c.Compute_UB_enhanced_last(m_lcol[0]);
#else
				c.Compute_UB_last(m_lcol[0]);
#endif
				//output to screen (TODO-LOGGER)
				stringstream sstr("");
				for(int i=0; i<m_size; i++){
					sstr<<m_lcol[0][i]<<" ";
				}
				sstr<<endl;
				LOG_INFO(sstr.str());
			}
						
			//LB: using amts heuristic
			if(param.init_preproc!=UB){
				LOG_PRINT("COMPUTING LB");
				//****TODO: place in setup and check for TRIVIAL SOLUTION
				AMTSexec a(RESTARTS, ITERATIONS_PER_RESTART);
				int lb_amts=a.run(*g);
				if(lb_amts>res.get_lower_bound()){
					maxno=lb_amts;
					res.set_LB(lb_amts);
				}
			}
		}
	}	
	return sol;
}

inline
void CliqueDollPlus::run(){
	//algorithm
	if(param.unrolled){
		LOG_ERROR("CliqueDollPlus::run: unrolling not defined for this familiy of algorithms");
		return;
	}else{
		res.tic();
		switch(param.alg){
		case BBMC_DOLL:
			expand_doll(0, Clique<ugraph>::m_lroot);
			break;
		case BBMCR_DOLL:
			expand_doll_R(0, Clique<ugraph>::m_lroot);
			break;
								
		default:
			LOG_ERROR("CliqueDollPlus::run-non_unrolled:unknown clique algorithm");
		}

		res.toc();
	}

	//read solution
	res.set_UB(maxno);
	LOG_INFO("[w:"<<res.get_upper_bound()<<","<<res.get_user_time()<<"s]");
}	



inline
int CliqueDollPlus::expand_doll (int maxac, nodelist_t& l_v){
//////////////////////
//	initial date: 20/09/15
//	author: pss
//	COMMENTS:  HEAD_COLOR BACK_UP and RESTORE (a nice improvement, possibly related to cache issues) 
	

	/*vint vec(l_v.nodos,l_v.nodos+l_v.index);
	com::stl::print_collection<vector<int>>(vec); cout<<endl;*/
	

	int v, maxUB=0, LB=maxno-maxac;			//|Smax|-|S|
	res.inc_number_of_steps();

//////////////////////////////////////
//SOLUTION CHECK
	if(LB==0){
		//solution found
		maxno=maxac+1;						//NEW GLOBAL OPTIMUM FOUND

#ifdef STORE_SOLUTION
		res.set_UB(maxno);
		res.clear_all_solutions();
		m_path[maxac]=l_v.nodos[0];
		res.add_solution(maxno, m_path);

	#ifdef VIEW_PROGRESS
		stringstream sstr("");
		res.print_first_sol(sstr);
		LOG_INFO(sstr.str());
	#endif
#endif

		//update color at root node with new solution and return to root node
		m_lcol[HEAD_COLOR][m_path[0]]=maxno;
		return ROOT_NODE;
	}
////////////////////////////////
		
	int pos_node=solve_first_nodes(maxac, l_v);						//note: cleans solved set container LISTA_BB 									

/////////////////////////////////////////////////
//MAIN LOOP
	for(int j=pos_node; j<l_v.index; j++){
		v=l_v.nodos[j];
	//	if(m_lcol[HEAD_COLOR][v]>(LB+1)) m_lcol[HEAD_COLOR][v]=(LB+1);		//only one improvement
		
#ifdef ROOT_VERTEX_PROGRESS
		if(maxac==0){
			cout<<"root vertex: "<<v<<" :"<<m_lcol[HEAD_COLOR][v-1]<<endl;
		}
#endif	

//////////
//pruning by inheriting father UB (not tested by solved-first-nodes)
		if( m_lcol[HEAD_COLOR][v]<=LB){
			//LOG_INFO("CUT SHOULD NOT OCCURR");
			LISTA_BB(maxac).set_bit(v);
			continue;
		}
		
////////////
// Child node generation (neighbor vertices below v), combined with computation of icremental bounds
		AND(g->get_neighbors(v), LISTA_BB(maxac), bbchild);	
		bbchild.to_old_vector(LISTA_L(maxac).nodos, LISTA_L(maxac).index);

		/*bbchild.print();cout<<endl;
		for(int i=0; i<LISTA_L(maxac).index; i++){
			cout<<LISTA_L(maxac).nodos[i]<<" ";
		}
		cout<<"----------------------------------"<<endl;*/

		maxUB=0;
#ifdef  RDOLL_INC_UB_CUT
		for(int i=0; i<LISTA_L(maxac).index; i++){
			int w=LISTA_L(maxac).nodos[i];
			if (maxUB<m_lcol[HEAD_COLOR][w] )
				maxUB=m_lcol[HEAD_COLOR][w];		
		}
#endif
		//subsumes leaf node detection and size-UB cut
		maxUB=min<int>((LISTA_L(maxac).index+1), (maxUB+1));			
		if(maxUB<m_lcol[HEAD_COLOR][v]){			
			m_lcol[HEAD_COLOR][v]=maxUB;
			if(maxUB<=LB){
			//	LOG_INFO("INC UB CUT");
				LISTA_BB(maxac).set_bit(v);
				continue;
			}
		}

///////////////////////////////////////////////////////
//TIGHT ESTIMATE OF UPPER BOUND


////////////
// Color (Tomita) and Infrachrom attempts to reduce the bound
		
		maxUB=paint_UB(bbchild);							//configures infrachrom params also
		if(++maxUB<m_lcol[HEAD_COLOR][v]){					//note that this is by no means always true
			m_lcol[HEAD_COLOR][v]=maxUB;
			if(maxUB<=LB){
				//LOG_INFO("COLOR CUT");
				LISTA_BB(maxac).set_bit(v);
				continue;
			}
		}
				
////////////////////
//Infra-chrom (it seems this is the vast majority of cases from here)
//Note: The configuration of infra-chrom has been done during coloring:
//1-iop.update_color_sizes(maxUB_stored);
//2-iop.set_node_state_active(bbchild);
//3-iop.set_color_nb(maxUB_stored);	
		
		if( (maxUB-LB)==1 ){															//Note that m_lcol[HEAD_COLOR][v]-LB does not hold
			if(iop.filter()){
				//res.inc_counter(0,1);
				m_lcol[HEAD_COLOR][v]-=1;
				//LOG_INFO("ONE-SHOT-NO-SORT");
				LISTA_BB(maxac).set_bit(v);
				continue;
			}
		}

		int nb_conf=iop.init_maxsatz(v, LB);
		if(nb_conf==EMPTY_ELEM){
			if(m_lcol[HEAD_COLOR][v]>LB)
					m_lcol[HEAD_COLOR][v]=LB;
			//LOG_INFO("DEEP INFRA-CHROM CUT");
			LISTA_BB(maxac).set_bit(v);
			continue;
		}else if(m_lcol[HEAD_COLOR][v]>(maxUB-nb_conf)){
			m_lcol[HEAD_COLOR][v]=maxUB-nb_conf;
		}
////////////////////////////////////////////////////////////
		
		
////////////////////
// BACK-UP HEAD_COLORS
		for(int i=0; i<LISTA_L(maxac).index; i++){
			m_lcol[maxac+1][LISTA_L(maxac).nodos[i]]=m_lcol[HEAD_COLOR][LISTA_L(maxac).nodos[i]];
		}
	
///////////////////////////////////////////////////////
// CANDIDATE EXPANSION

		//sets path
		m_path[maxac]=v;
				
		//child node expansion
		if( (maxUB=expand_doll(maxac+1, LISTA_L(maxac)) )==ROOT_NODE ){ 
			if(maxac!=0){																
				return ROOT_NODE;
			}else{		//ROOT NODE
				LB=maxno-maxac;  	//update LB becuase a new solution is found
			}
		}
		
//////////////////////////////////////////////
// BACKTRACK

	///////////////
	//RESTORE
		for(int i=0; i<LISTA_L(maxac).index; i++){
			m_lcol[HEAD_COLOR][LISTA_L(maxac).nodos[i]]=m_lcol[maxac+1][LISTA_L(maxac).nodos[i]];
		}

	////////////////////
	//updates UB of father vertex
		if(maxUB!=ROOT_NODE){
			if(++maxUB<m_lcol[HEAD_COLOR][v]) {
				m_lcol[HEAD_COLOR][v]=maxUB;
				//no need to cut here since it the vertex will be added anyway to the solved set
				/*if(maxUB<=LB){
					LOG_ERROR("UB IMPROVED ON BACKTRACK TO LB:"<<maxUB<<":"<<LB);
				}*/
			}
		}

		//adds expanded vertex to solved set
		LISTA_BB(maxac).set_bit(v);
		
	}// next node
//END OF MAIN LOOP
/////////////////////////////////////////////////////////

#ifdef RDOLL_MIN_UB_ON_RETURN
	maxUB=0;
	for(int i=0; i<l_v.index; i++){
		if(maxUB<m_lcol[HEAD_COLOR][l_v.nodos[i]]){
				maxUB=m_lcol[HEAD_COLOR][l_v.nodos[i]];
		}
	}

	return min<int>(LB, maxUB);
#else	
	return LB;
#endif
}

inline
int CliqueDollPlus::expand_doll_R (int maxac, nodelist_t& l_v){
//////////////////////
//	initial date: 20/09/15
//	author: pss
//	COMMENTS:  HEAD_COLOR BACK_UP and RESTORE (a nice improvement, possibly related to cache issues) 
	

	/*vint vec(l_v.nodos,l_v.nodos+l_v.index);
	com::stl::print_collection<vector<int>>(vec); cout<<endl;*/
	
	int v, maxUB=0, LB=maxno-maxac;			//|Smax|-|S|
	res.inc_number_of_steps();
		
//////////////////////////////////////
//SOLUTION CHECK
	if(LB==0){
		//solution found
		maxno=maxac+1;							//NEW GLOBAL OPTIMUM FOUND

#ifdef STORE_SOLUTION
		res.set_UB(maxno);
		res.clear_all_solutions();
		m_path[maxac]=l_v.nodos[0];				//first vertex in the list forms part of a maximal solution
		res.add_solution(maxno, m_path);

	#ifdef VIEW_PROGRESS
		stringstream sstr("");
		res.print_first_sol(sstr);
		LOG_INFO(sstr.str());
	#endif
#endif
		//update color at root node with new solution and return to root node
		m_lcol[HEAD_COLOR][m_path[0]]=maxno;
		return ROOT_NODE;
	}
////////////////////////////////////////
	
	//int pos_node=solve_first_nodes(maxac, l_v);							//erases LISTA_BB(maxac)	
	int pos_node=solve_first_nodes_incMaxCLQ(maxac, l_v);					//erases LISTA_BB(maxac)	
		

/////////////////////////////////////////////////
//MAIN LOOP
	for(int j=pos_node; j<l_v.index; j++){
		v=l_v.nodos[j];
		
#ifdef ROOT_VERTEX_PROGRESS
		if(maxac==0){
			cout<<"root vertex: "<<v<<" :"<<m_lcol[HEAD_COLOR][v-1]<<endl;
		}
#endif	
		
//////////
//pruning by inheriting father UB (not tested by solved-first-nodes)
		if( m_lcol[HEAD_COLOR][v]<=LB){
			//LOG_INFO("CUT SHOULD NOT OCCURR:"<<v<<" ["<<maxac<<"]");
			LISTA_BB(maxac).set_bit(v);
			continue;
		}
		
////////////
// Child node generation (neighbor vertices below v), combined with computation of icremental bounds
		AND(g->get_neighbors(v), LISTA_BB(maxac), bbchild);	
		bbchild.to_old_vector(LISTA_L(maxac).nodos, LISTA_L(maxac).index);

		maxUB=0;
#ifdef  RDOLL_INC_UB_CUT
		for(int i=0; i<LISTA_L(maxac).index; i++){
			int w=LISTA_L(maxac).nodos[i];
			if (maxUB<m_lcol[HEAD_COLOR][w] )							//all neighbors of v
					maxUB=m_lcol[HEAD_COLOR][w];		

		}
#endif
		//subsumes leaf node detection and size-UB cut
		maxUB=min<int>((LISTA_L(maxac).index+1), (maxUB+1));			
		if(maxUB<m_lcol[HEAD_COLOR][v]){			
			m_lcol[HEAD_COLOR][v]=maxUB;
			if(maxUB<=LB){
				//LOG_ERROR("INC UB CUT");
				LISTA_BB(maxac).set_bit(v);
				continue;
			}
		}

////////////////////////////////////////////////////////
//TIGHT ESTIMATE OF UPPER BOUND

	//	LOG_INFO(v<<" ["<<maxac<<"]");


////////////
// Color (Tomita) and Infrachrom attempts to reduce the bound
		maxUB=iop.paint_R(bbchild, LB /*maxno-maxac*/);		/* new painting routine */
		if(++maxUB<m_lcol[HEAD_COLOR][v]){					//note that this is by no means always true
			m_lcol[HEAD_COLOR][v]=maxUB;
			if(maxUB<=LB){
				//LOG_INFO("COLOR CUT:"<<v<<" ["<<maxac<<"]");
				LISTA_BB(maxac).set_bit(v);
				continue;
			}
		}
				
////////////////////
//Infra-chrom (it seems this is the vast majority of cases from here)
//Note: The configuration of infra-chrom has been done during coloring:
//1-iop.update_color_sizes(maxUB_stored);
//2-iop.set_node_state_active(bbchild);
//3-iop.set_color_nb(maxUB_stored);	
		
		if( (maxUB-LB)==1 ){															//Note that m_lcol[HEAD_COLOR][v]-LB does not hold
			if(iop.filter()){
				//res.inc_counter(0,1);
				m_lcol[HEAD_COLOR][v]-=1;
				//LOG_INFO("ONE-SHOT-FILTER:"<<v);
				LISTA_BB(maxac).set_bit(v);
				continue;
			}
		}

		int nb_conf=iop.init_maxsatz(LB-1 /* check value >=1 */);						/* (1) */
		//int nb_conf=iop.init_maxsatz(v, LB);											/*this is equivalent to (1)*/
		if(nb_conf==InfraOpPlus<ugraph, bitarray>::MAX_NB_OF_CONFLICTS){
			if(m_lcol[HEAD_COLOR][v]>LB)
					m_lcol[HEAD_COLOR][v]=LB;
			//LOG_INFO("DEEP INFRA-CHROM CUT");
			LISTA_BB(maxac).set_bit(v);
			continue;
		}else if(m_lcol[HEAD_COLOR][v]>(maxUB-nb_conf)){
			m_lcol[HEAD_COLOR][v]=maxUB-nb_conf;
		}
////////////////////////////////////////////////////////////
		
		
////////////////////
// BACK-UP HEAD_COLORS
		for(int i=0; i<LISTA_L(maxac).index; i++){
			m_lcol[maxac+1][LISTA_L(maxac).nodos[i]]=m_lcol[HEAD_COLOR][LISTA_L(maxac).nodos[i]];
		}
	
///////////////////////////////////////////////////////
// CANDIDATE EXPANSION

		//sets path
		m_path[maxac]=v;
				
		//child node expansion
		if( (maxUB=expand_doll_R(maxac+1, LISTA_L(maxac)) )==ROOT_NODE ){ 
			if(maxac!=0){																
				return ROOT_NODE;
			}else{		//ROOT NODE
				LB=maxno-maxac;  	//update LB becuase a new solution is found
			}
		}
		
//////////////////////////////////////////////
// BACKTRACK

	///////////////
	//RESTORE
		for(int i=0; i<LISTA_L(maxac).index; i++){
			m_lcol[HEAD_COLOR][LISTA_L(maxac).nodos[i]]=m_lcol[maxac+1][LISTA_L(maxac).nodos[i]];
		}

	////////////////////
	//updates UB of father vertex
		if(maxUB!=ROOT_NODE){
			if(++maxUB<m_lcol[HEAD_COLOR][v]) {
				m_lcol[HEAD_COLOR][v]=maxUB;
				//no need to cut here since it the vertex will be added anyway to the solved set
				/*if(maxUB<=LB){
					LOG_ERROR("UB IMPROVED ON BACKTRACK TO LB:"<<maxUB<<":"<<LB);
				}*/
			}
		}

		//adds expanded vertex to solved set
		LISTA_BB(maxac).set_bit(v);
		
	}// next node
//END OF MAIN LOOP
/////////////////////////////////////////////////////////

#ifdef RDOLL_MIN_UB_ON_RETURN
	maxUB=0;
	for(int i=0; i<l_v.index; i++){
		if(maxUB<m_lcol[HEAD_COLOR][l_v.nodos[i]]){
				maxUB=m_lcol[HEAD_COLOR][l_v.nodos[i]];
		}
	}

	if(maxUB<LB){
		LOG_ERROR("MIN UB ON RETURN USEFUL");
	}
	return min<int>(LB, maxUB);
#else	
	return LB;	
#endif
}

inline
int CliqueDollPlus::expand_doll_R_no_pmaxsat (int maxac, nodelist_t& l_v){
//////////////////////
//	initial date: 20/09/15
//	author: pss
//	COMMENTS:  HEAD_COLOR BACK_UP and RESTORE (a nice improvement, possibly related to cache issues) 
	

	/*vint vec(l_v.nodos,l_v.nodos+l_v.index);
	com::stl::print_collection<vector<int>>(vec); cout<<endl;*/
	
	int v, maxUB=0, LB=maxno-maxac;			//|Smax|-|S|
	res.inc_number_of_steps();
		
//////////////////////////////////////
//SOLUTION CHECK
	if(LB==0){
		//solution found
		maxno=maxac+1;							//NEW GLOBAL OPTIMUM FOUND

#ifdef STORE_SOLUTION
		res.set_UB(maxno);
		res.clear_all_solutions();
		m_path[maxac]=l_v.nodos[0];				//first vertex in the list forms part of a maximal solution
		res.add_solution(maxno, m_path);

	#ifdef VIEW_PROGRESS
		stringstream sstr("");
		res.print_first_sol(sstr);
		LOG_INFO(sstr.str());
	#endif
#endif
		//update color at root node with new solution and return to root node
		m_lcol[HEAD_COLOR][m_path[0]]=maxno;
		return ROOT_NODE;
	}
////////////////////////////////////////
	
	//int pos_node=solve_first_nodes(maxac, l_v);							//erases LISTA_BB(maxac)	
	int pos_node=solve_first_nodes_incMaxCLQ(maxac, l_v);	

	//int pos_node=solve_first_nodes_incMaxCLQ(maxac, l_v);					//erases LISTA_BB(maxac)	
		

/////////////////////////////////////////////////
//MAIN LOOP
	for(int j=pos_node; j<l_v.index; j++){
		v=l_v.nodos[j];
		
#ifdef ROOT_VERTEX_PROGRESS
		if(maxac==0){
			cout<<"root vertex: "<<v<<" :"<<m_lcol[HEAD_COLOR][v-1]<<endl;
		}
#endif	
		
//////////
//pruning by inheriting father UB (not tested by solved-first-nodes)
		if( m_lcol[HEAD_COLOR][v]<=LB){
			//LOG_INFO("CUT SHOULD NOT OCCURR:"<<v<<" ["<<maxac<<"]");
			LISTA_BB(maxac).set_bit(v);
			continue;
		}
		
////////////
// Child node generation (neighbor vertices below v), combined with computation of icremental bounds
		AND(g->get_neighbors(v), LISTA_BB(maxac), bbchild);	
		bbchild.to_old_vector(LISTA_L(maxac).nodos, LISTA_L(maxac).index);

		maxUB=0;
#ifdef  RDOLL_INC_UB_CUT
		for(int i=0; i<LISTA_L(maxac).index; i++){
			int w=LISTA_L(maxac).nodos[i];
			if (maxUB<m_lcol[HEAD_COLOR][w] )							//all neighbors of v
					maxUB=m_lcol[HEAD_COLOR][w];		

		}
#endif
		//subsumes leaf node detection and size-UB cut
		maxUB=min<int>((LISTA_L(maxac).index+1), (maxUB+1));			
		if(maxUB<m_lcol[HEAD_COLOR][v]){			
			m_lcol[HEAD_COLOR][v]=maxUB;
			if(maxUB<=LB){
				//LOG_ERROR("INC UB CUT");
				LISTA_BB(maxac).set_bit(v);
				continue;
			}
		}

////////////////////////////////////////////////////////
//TIGHT ESTIMATE OF UPPER BOUND

	//	LOG_INFO(v<<" ["<<maxac<<"]");


////////////
// Color (Tomita) and Infrachrom attempts to reduce the bound
		maxUB=iop.paint_R(bbchild, LB /*maxno-maxac*/);		/* new painting routine */
		if(++maxUB<m_lcol[HEAD_COLOR][v]){					//note that this is by no means always true
			m_lcol[HEAD_COLOR][v]=maxUB;
			if(maxUB<=LB){
				//LOG_INFO("COLOR CUT:"<<v<<" ["<<maxac<<"]");
				LISTA_BB(maxac).set_bit(v);
				continue;
			}
		}
				
////////////////////
//Infra-chrom (it seems this is the vast majority of cases from here)
//Note: The configuration of infra-chrom has been done during coloring:
//1-iop.update_color_sizes(maxUB_stored);
//2-iop.set_node_state_active(bbchild);
//3-iop.set_color_nb(maxUB_stored);	
		
		//*** REMOVED ***
		
////////////////////////////////////////////////////////////
		
		
////////////////////
// BACK-UP HEAD_COLORS
		for(int i=0; i<LISTA_L(maxac).index; i++){
			m_lcol[maxac+1][LISTA_L(maxac).nodos[i]]=m_lcol[HEAD_COLOR][LISTA_L(maxac).nodos[i]];
		}
	
///////////////////////////////////////////////////////
// CANDIDATE EXPANSION

		//sets path
		m_path[maxac]=v;
				
		//child node expansion
		if( (maxUB=expand_doll_R_no_pmaxsat(maxac+1, LISTA_L(maxac)) )==ROOT_NODE ){ 
			if(maxac!=0){																
				return ROOT_NODE;
			}else{		//ROOT NODE
				LB=maxno-maxac;  	//update LB becuase a new solution is found
			}
		}
		
//////////////////////////////////////////////
// BACKTRACK

	///////////////
	//RESTORE
		for(int i=0; i<LISTA_L(maxac).index; i++){
			m_lcol[HEAD_COLOR][LISTA_L(maxac).nodos[i]]=m_lcol[maxac+1][LISTA_L(maxac).nodos[i]];
		}

	////////////////////
	//updates UB of father vertex
		if(maxUB!=ROOT_NODE){
			if(++maxUB<m_lcol[HEAD_COLOR][v]) {
				m_lcol[HEAD_COLOR][v]=maxUB;
				//no need to cut here since it the vertex will be added anyway to the solved set
				/*if(maxUB<=LB){
					LOG_ERROR("UB IMPROVED ON BACKTRACK TO LB:"<<maxUB<<":"<<LB);
				}*/
			}
		}

		//adds expanded vertex to solved set
		LISTA_BB(maxac).set_bit(v);
		
	}// next node
//END OF MAIN LOOP
/////////////////////////////////////////////////////////

#ifdef RDOLL_MIN_UB_ON_RETURN
	maxUB=0;
	for(int i=0; i<l_v.index; i++){
		if(maxUB<m_lcol[HEAD_COLOR][l_v.nodos[i]]){
				maxUB=m_lcol[HEAD_COLOR][l_v.nodos[i]];
		}
	}

	if(maxUB<LB){
		LOG_ERROR("MIN UB ON RETURN USEFUL");
	}
	return min<int>(LB, maxUB);
#else	
	return LB;	
#endif
}

inline
int CliqueDollPlus::paint_UB(const bitarray& bb){
////////////////////////
// classical independent set coloring of subgraph bb
// first update: 4/7/16
//
// REMARKS: 
//1. does NOT update color sizes
//2. stores color labels
		
	int col=1; int nBB=EMPTY_ELEM; int v=EMPTY_ELEM;
		
	
	iop.m_colSets[1].erase_bit();
	int pc=(m_unsel=bb).popcn64();
	if(pc==0) return 0;					//***emtpy check tests, remove for optimal performance

	while(true){ 
		bitarray& bbcol=iop.m_colSets[col].bb;
		for(int i=0; i<iop.NB_OF_BB_NODES; i++){
			bbcol.get_bitboard(i)=m_unsel.get_bitboard(i);
		}
				
		bbcol.init_scan(bbo::NON_DESTRUCTIVE);
		while(true){
			v=bbcol.next_bit(nBB,m_unsel);
			if(v==EMPTY_ELEM)
				break;

			//stores color label
			iop.m_colSets[col].size++;			//***TODO simply increment size: equivalent to push(v)
			iop.node_iset_no[v]=col;

			//checks exit condition
			if((--pc)==0){
				iop.set_color_nb(col);
				iop.set_node_state_active(bb);
				return col;
			}

			bitarray& nbor=g->get_neighbors(v);
			for(int i=nBB; i<iop.NB_OF_BB_NODES; i++){
				bbcol.get_bitboard(i)&= ~nbor.get_bitboard(i);
			}
			//iop.m_colSets[col].bb.erase_block(nBB,g->get_neighbors(v));		
		}
		
		//increments color and erases heap
		iop.m_colSets[++col].erase_bit();
	}

	LOG_ERROR("paint_UB_BB():bizarre coloring");
	return EMPTY_ELEM;		//should not reach here: ERROR
}

inline
int CliqueDollPlus::paint_UB_R(const bitarray& bb, int maxac){
////////////////////////
// classical independent set coloring of subgraph bb
// last update: 26/9/16
// 
//
// REMARKS: 
//	1. does NOT update color sizes
//	2. stores color labels
//	3. Uses renumbering up to col (col>kmin), not just kmin! 
//
//*** TODO: POSSIBLE BUG! maxac is a global variable NOT UPDATED!
//*** added parameter maxac (5/11/16)
		
	int col=1, nBB=EMPTY_ELEM, v=EMPTY_ELEM, vswap=EMPTY_ELEM, kmin=maxno-maxac;
	const int KMIN_MINUS_ONE=kmin-1, NB_OF_BB_NODES_MINUS_ONE=iop.NB_OF_BB_NODES-1;
			
	iop.m_colSets[1].erase_bit();
	int pc=(m_unsel=bb).popcn64();
	if(pc==0) return 0;					

	while(true){ 
		bitarray& bbcol=iop.m_colSets[col].bb;
		for(int i=0; i<iop.NB_OF_BB_NODES; i++){
			bbcol.get_bitboard(i)=m_unsel.get_bitboard(i);
		}
				
		bbcol.init_scan(bbo::NON_DESTRUCTIVE);
		while(true){
next_v:		v=bbcol.next_bit(nBB,m_unsel);
			if(v==EMPTY_ELEM)
				break;				//new color
////////////////////////////
//RECOLORING ATTEMPT
			if( (col>=kmin) && (kmin>=3) ){
				//for(int recol=1; recol<KMIN_MINUS_ONE; recol++){
				for(int recol=1; recol<col-1; recol++){
					int pc_swap=iop.m_colSets[recol].bb.single_disjoint(0, NB_OF_BB_NODES_MINUS_ONE, g->get_neighbors(v), vswap);
					if(pc_swap==1){	//candidate color class found
						//for(int j=recol+1; j<kmin; j++){
						for(int j=recol+1; j<col; j++){
							if(iop.m_colSets[j].bb.is_disjoint(0, NB_OF_BB_NODES_MINUS_ONE, g->get_neighbors(vswap))){

								iop.m_colSets[j].push(vswap);
								iop.m_colSets[recol].push(v);
								iop.m_colSets[recol].erase_bit(vswap);
								iop.m_colSets[col].bb.erase_bit(v);

								iop.node_iset_no[vswap]=j;
								iop.node_iset_no[v]=recol;

								if((--pc)==0){
									//	LOG_INFO("CUTTING RECOLOR");
									iop.set_node_state_active(bb);
									if(iop.m_colSets[col].size==0){	//** TODO:optimize
										iop.set_color_nb(col-1);
										return col-1;
									}else{
										iop.set_color_nb(col);
										return col;
									}
								}else goto next_v;
							}

						}
					} else if(pc_swap==0){
						iop.m_colSets[col].bb.erase_bit(v);
						iop.m_colSets[recol].push(v);
						iop.node_iset_no[v]=recol;

						//empty check of unsel in case vertex swapped is the last one
						if((--pc)==0){
							//	LOG_INFO("CUTTING RECOLOR");
							iop.set_node_state_active(bb);
							if(iop.m_colSets[col].size==0){	//** TODO:optimize
								iop.set_color_nb(col-1);
								return col-1;
							}else{
								iop.set_color_nb(col);
								return col;
							}

						}else goto next_v;
					}
				}
			}
///////////////////////////////////////

			//stores color label
			iop.m_colSets[col].size++;						//adds v (already in the bitset)
			iop.node_iset_no[v]=col;

			//checks exit condition
			if((--pc)==0){
				iop.set_color_nb(col);
				iop.set_node_state_active(bb);
				return col;
			}

			bitarray& nbor=g->get_neighbors(v);
			for(int i=nBB; i<iop.NB_OF_BB_NODES; i++){
				bbcol.get_bitboard(i)&= ~nbor.get_bitboard(i);
			}
			//iop.m_colSets[col].bb.erase_block(nBB,g->get_neighbors(v));		
		}
		
		col++;
		iop.m_colSets[col].erase_bit();
	}

	LOG_ERROR("paint_UB_BB():bizarre coloring");
	return EMPTY_ELEM;		//should not reach here: ERROR
}

inline
int CliqueDollPlus::solve_first_nodes_incMaxCLQ(int maxac, nodelist_t& l_v){
/////////////////
// INPUT: The full list of nodes that make the current subgraph
// RETURNS: number of solved nodes (nodes that are cut)
//
// IMPORTANT: It is imperative to increment memory allocation for colors (see comment 2): iop.init()
//
// COMMENTS: 
// 1.Labels the solved nodes as best possible. 
// 2.Colors with FAKE variables are NOT OPEN (note this can increase the size of the coloring artificially)
// 3.kmin cannot be equal to 0 (else inc_maxsatz could be called without being initialized when cmax==1)
//
  
	int nb_solved_v=0, v, nBBv=EMPTY_ELEM; 
	const int KMIN=maxno-maxac;
	bool is_existing_col, init_flag=false; 
	
		
	iop.NB_OF_COLORS=1;
	LISTA_BB(maxac).erase_bit();
	iop.m_colSets[iop.NB_OF_COLORS].erase_bit();
	iop.node_state_active.erase_bit();
	for(int j=0; j<l_v.index; j++){
		
		//estrategias
		v=l_v.nodos[j];
									
		//inner loop: search for SEQ color for v
		is_existing_col=false;
		for(int col=1; col<=iop.NB_OF_COLORS; col++){
			is_existing_col=g->get_neighbors(v).is_disjoint(0, WDIV(v), iop.m_colSets[col].bb) && (iop.m_colSets[col].bb.get_bitboard(iop.NB_OF_BB_NODES)==0);
			
			//color found for vertex
			if(is_existing_col){
				iop.m_colSets[col].push(v);							//adds vertex and updates bitstring
				iop.node_state_active.set_bit(v);
				iop.node_iset_no[v]=col;
				break;
			}
		}

		//color not found: new color
		if(!is_existing_col){
			iop.add_node_to_new_color(v);
			if(iop.NB_OF_COLORS>KMIN ){								//&& (m_lcol[0][v]>kmin) is not complete
				if(init_flag==false){
					init_flag=true;
					iop.init_inc_maxsatz();							//sets all colors to active and fills UNIT_STACK
				}
			}	
		}
		
		if(iop.NB_OF_COLORS<=KMIN || is_existing_col){
			m_lcol[HEAD_COLOR][v]=estimate_ub_first_nodes(min<int>(m_lcol[HEAD_COLOR][v],iop.NB_OF_COLORS), v, j, l_v);
			nb_solved_v++;
		}else if(!is_existing_col){
			iop.color_unit_stack.push(iop.NB_OF_COLORS);	/*last color which has to the new singleton */
			if(iop.is_enlarged_saturated() || !iop.inc_maxsatz(v) /* conflict not found */ ){  
					m_lcol[HEAD_COLOR][v]=estimate_ub_first_nodes(min<int>(m_lcol[HEAD_COLOR][v],iop.NB_OF_COLORS), v, j, l_v);			//*** check if it does something
					break;										
			}else{
				m_lcol[HEAD_COLOR][v]=estimate_ub_first_nodes(min<int>(m_lcol[HEAD_COLOR][v],iop.NB_OF_COLORS), v, j, l_v);
				nb_solved_v++;
			}
		}
	
		LISTA_BB(maxac).set_bit(v);
	}//next node
		
	iop.reset_enlarged_isets();		/* reset context operations: check which operations here are strictly necessary */
	return nb_solved_v;
}

inline
int CliqueDollPlus::solve_first_nodes(int maxac, nodelist_t& l_v){
/////////////////
// INPUT: The full list of nodes that make the current subgraph
// RETURNS: number of solved nodes 
//
// COMMENTS: 
// 1.Labels the solved nodes as best possible 
// 2.kmin cannot be equal to 0 (else inc_maxsatz could be called without being initialized when cmax==1)
 
	int nb_solved_v=0, v, cmax=1, nBBv=EMPTY_ELEM; 
	const int KMIN=maxno-maxac, KMIN_PLUS_ONE=KMIN+1;
	bool is_existing_col, init_flag=false; 
	
	
	LISTA_BB(maxac).erase_bit();
	//iop.color_state_active.erase_bit();							//if init_inc_maxsatx is not called this is obligatory, but it is not the case here
	iop.m_colSets[1].erase_bit();
	iop.node_state_active.erase_bit();
	for(int j=0; j<l_v.index; j++){
		
		//estrategias
		v=l_v.nodos[j];
									
		//inner loop: search for SEQ color for v
		is_existing_col=false;
		for(int col=1; col<=cmax; col++){
			is_existing_col=g->get_neighbors(v).is_disjoint(0, WDIV(v), iop.m_colSets[col].bb);			//**TODO optimize? note: the reasoning does not include added nodes
			

			//color found for vertex
			if(is_existing_col){
				iop.m_colSets[col].push(v);							//adds vertex and updates bitstring
				iop.node_state_active.set_bit(v);
				iop.node_iset_no[v]=col;
				break;
			}
		}

		//color not found: new color
		if(!is_existing_col){
		//	cout<<"color alloc:"<<m_alloc<<endl;
			iop.m_colSets[++cmax].erase_bit();				//clears color (previous use in same level)
			iop.m_colSets[cmax].push(v);
			iop.node_state_active.set_bit(v);
			iop.node_iset_no[v]=cmax;
			iop.color_state_active.set_bit(cmax);

			iop.set_color_nb(cmax);							//has to be placed here

			if(cmax>KMIN ){									//&& (m_lcol[0][v]>kmin) is not complete
				if(init_flag==false){
					init_flag=true;
					iop.init_inc_maxsatz();					//sets all colors to active and fills UNIT_STACK
				}
			}	
		}	
				
		if(cmax>KMIN_PLUS_ONE)	break;						 //**note cmax==1 is not possible
		if( cmax>KMIN && m_lcol[HEAD_COLOR][v]>KMIN ){		
				if(!is_existing_col)
					iop.color_unit_stack.push(cmax);

				if(iop.inc_maxsatz(v)!=true){
					break;										//unable to prove inconsistency-STOP
				}
		}/*else if(cmax>KMIN){
			LOG_ERROR("SOLVING FIRST NODES WITH FATHER UB");
		}*/

		m_lcol[HEAD_COLOR][v]=estimate_ub_first_nodes(min<int>(m_lcol[HEAD_COLOR][v],cmax), v, j, l_v);
		nb_solved_v++;
		LISTA_BB(maxac).set_bit(v);
	}

	//reset context operations
	iop.reset_enlarged_isets();
	return nb_solved_v;

}

inline
int CliqueDollPlus::estimate_ub_first_nodes(int old_ub /* inherited from child node*/, int node, int pos_node, nodelist_t& l_v){
////////////////////
//
// PARAMS: old_ub: the father ub, node: the vertex to bound, pos_node: the position in the stack, 
//				   l_v: the stack

				
	int max=0, max_iset_nb=0, solved_node;				//computes ub concerned with current colorng of solve_first_nodes
	
	//loop over previous solved nodes
	for(int i=0; i<pos_node; i++) {
		solved_node=l_v.nodos[i];
		/*if (max_iset_nb<iop.node_iset_no[solved_node])		
			max_iset_nb=iop.node_iset_no[solved_node];*/
		if (g->is_edge(node, solved_node)) {
			if (max<m_lcol[HEAD_COLOR][solved_node])
					max=m_lcol[HEAD_COLOR][solved_node];
			if (max_iset_nb<iop.node_iset_no[solved_node])		 //*** check if this is correct for incremental_maxsatz (7/11/16)
					max_iset_nb=iop.node_iset_no[solved_node];
		}
	}

	max++; // add node itself into the clique
	//if (old_ub<max)
	//	max=old_ub;
	//	
	///*if (max_iset_nb<iop.node_iset_no[node])
	//		max_iset_nb=iop.node_iset_no[node];*/

	//if (max_iset_nb<max-1){  
	//	//LOG_ERROR("BOUND CHANGED IN SOLVE FIRST NODES:"<<max_iset_nb+1<<":"<<max);
	//	max=max_iset_nb+1;
	//	//cin.get();
	//}	
	
	
	return com::mat::min3(max, old_ub, max_iset_nb+1);
	//return max;

}


#endif