////////////////////////////////
// clique_weighted.h: interface for CliqueWeighted class to solve MWCP for non-sparse graphs
//					  adapts MCP ideas to MWCP
//                
// initial date: 6/10/16
// last update: 6/10/16
// author: pss, ff

#ifndef  __CLIQUE_WEIGHTED_H__
#define  __CLIQUE_WEIGHTED_H__


#include <functional>					// std::plus
#include <numeric>						// std::accumulate
#include <algorithm>
#include <iterator>

#include "clique.h"
#include "../init_color_ub.h"
#include "../init_color_ub_weighted.h"
#include "../amts/amts_exec.h"
#include "infra_weighted_tools.h"

//using namespace std;

class CliqueWeighted:public Clique<ugraph>{
   static const int ROOT_NODE=-1;
   static const int HEAD_COLOR=0;				
public:
	CliqueWeighted(ugraph* g, param_t p)						:Clique<ugraph>(g, p), m_lfv(NULL) {};
	CliqueWeighted(param_t p)									:Clique<ugraph>(p), m_lfv(NULL)  {};
	virtual ~CliqueWeighted(){clear_first_vertices();}
	
//allocation
virtual int init_others();
	void clear_first_vertices()					{if(m_lfv!=NULL) delete [] m_lfv; m_lfv=NULL; }
	int init_first_vertices();
		
	//new painting functions only dedicated to find UB, not branching 
	int paint_UB_ow   (const bitarray&);						//IS coloring (for bitstring encoding of subgraphs)
	int paint_UB_R_ow (int maxac, const bitarray&);				//IS with recoloring
	int paint_UB_X_ow (int maxac, const bitarray&);				//*** TODO simple infra-chrom
		
	int paint_UB 	 (const bitarray&);
	int paint_UB_R	 (int maxac, const bitarray&);	
	int paint_UB_X	 (int maxac, const bitarray&);	


	int paint_UB_R_satz (int maxac, const bitarray&);	
	int paint_UB_R_minw_satz (int maxac, const bitarray&);
	int paint_UB_X_minw_satz (int maxac, const bitarray&);
	int paint_UB_RX_minw_satz (int maxac, const bitarray&);

	int paint_UB_X_ow_satz (int maxac, const bitarray&);		
	

	//new incmaxsatz attemtps
	int solve_first_nodes_ow	(int depth, int maxac, nodelist_t& l_v);
	int estimate_ub_first_nodes	(int old_ub, int node, int pos_node, nodelist_t& l_v);

	int solve_first_nodes		(int depth, int maxac, nodelist_t& l_v);
	
				
	//tested search procedures
	int expand_weighted (int depth, int maxac, nodelist_t& l_v);					//driver for bitstring imlpementation (main variant)
	int expand_weighted_R (int depth, int maxac, nodelist_t& l_v);	
	int expand_weighted_X (int depth, int maxac, nodelist_t& l_v);

	int expand_weighted_R_satz (int depth, int maxac, nodelist_t& l_v);

///////
//initial bounds
	virtual	int initial_bounds(int& lb, int& ub,  KCore<ugraph>* = NULL); 
	
	//test framework
	virtual	int set_up();
	virtual	void run();	
	virtual	void tear_down(){ Clique<ugraph>::tear_down(); /*clear_first_vertices();*/}

private:
	bitarray bbchild;
	int* m_lfv;							//[COLORS] info of color sets (usually weights, could be nodes)
	int m_nCol;							//number of colors used at each node
	bitarray m_forbidden;				//forbidden set of colors for simple infrachrom
	InfraOpW<ugraph, bitarray> iop;		//pmax-sat bound (used only incrementally) 
};

int CliqueWeighted::init_others(){
//////////////
// allocates extra space
//
// RETURNS 0: ok, -1 error

	Clique<ugraph>::init_others();

	//list of first nodes of color sets (default value UB: NV+100)
	if(init_first_vertices()==-1){
		return -1;
	}

	//forbidden colors
	m_forbidden.init(m_size);

	//iop
	iop.set_graph(g);
	iop.init(m_size);		
	iop.set_clause_weights(m_lfv);
								
	//empty set
	m_bbroot.erase_bit();
	Clique<ugraph>::m_lroot.index=m_size;		//contains number of elements, not num_elem-1 as in other variants
	bbchild.init(m_size);
	maxac=0;									//initial solution empty
	return 0;
}

int CliqueWeighted::init_first_vertices(){
	clear_first_vertices();
	int NC=g->number_of_vertices()+1;					/* colors start at 1*/
	try{
		m_lfv=new int[NC];
	} catch(exception& e){
		LOG_ERROR("CliqueWeighted::init_first_vertices(): error");
		LOG_ERROR(e.what());
		return -1;
	}

	for(int i=0; i<NC; i++){
		m_lfv[i]=NC+100;							//upper bound con any vertex
	}
	m_nCol=0;
	return 0;
}

inline
int CliqueWeighted::set_up(){
//////////////
// allocates memory, reads weights, evaluates initial bounds and determines 
// initial trivial solutions
//
// RETURN VALUE: -1 Error, 0-ok, >0 trivial solution found
//
// *** TODO: MANY CHANGES HERE BECAUSE OF WEIGHTS
	
	int sol=0;
	res.clear();
	res.set_name(g->get_name());

	//determine allocation info
	search_alloc_t info;							//loads BBMC configuration for search allocation
	info.set(search_alloc_t::ALLOC_COLOR_SETS);		//for recoloring
	switch(param.alg){
	case BBMC_WEIGHTED:
	case BBMCR_WEIGHTED:
	case BBMCX_WEIGHTED:
	case BBMCR_SAT_WEIGHTED:
		break;
	default:
		LOG_ERROR("CliqueWeighted::setup unknown algorithm");
		return -1;
	}
			
	//actual set_up
	if(param.unrolled){
		LOG_ERROR("CliqueWeighted::setup unrolled variant undefined");
		return -1;
	}else{
		if( (sol=set_up_non_unrolled(info))>0 ){
				LOG_INFO("[w="<<sol<<"]"<<" TRIVIALLY SOLVED DURING PRECOMPUTATION");
				res.set_UB(sol);
		}else{ 
			//Trivial solution not found
			//*** additional extra-initialization non_unrolled case ***

			LOG_INFO("----------------------------------"<<endl);
			g->print_weights(cout);
			LOG_INFO("----------------------------------"<<endl);
		
			//UB
			if(param.init_preproc!=HEUR){
				LOG_PRINT("COMPUTING UB");
				InitColorUBW c(*g);
				c.Compute_UB_last(m_lcol[0]);
			
				//output to screen
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
				AMTSexec a(RESTARTS, ITERATIONS_PER_RESTART, WMODE_WEIGHTS);	
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
void CliqueWeighted::run(){
	//algorithm
	if(param.unrolled){
		LOG_ERROR("CliqueWeighted::run: unrolling not defined for this familiy of algorithms");
		return;
	}else{
		res.tic();
		switch(param.alg){
		case BBMC_WEIGHTED:
			expand_weighted(0 /*depth*/, 0 /* size of initial sol*/, Clique<ugraph>::m_lroot); 
			break;
		case BBMCR_WEIGHTED:
			expand_weighted_R(0 /*depth*/, 0 /* size of initial sol*/, Clique<ugraph>::m_lroot); 
			break;	
		case BBMCX_WEIGHTED:
			expand_weighted_X(0 /*depth*/, 0 /* size of initial sol*/, Clique<ugraph>::m_lroot); 
			break;
		case BBMCR_SAT_WEIGHTED:
			expand_weighted_R_satz(0 /*depth*/, 0 /* size of initial sol*/, Clique<ugraph>::m_lroot); 
			break;
		default:
			LOG_ERROR("CliqueWeighted::run-non_unrolled:unknown clique algorithm");
		}
		res.toc();
	}

	//read solution
	res.set_UB(maxno);
	LOG_INFO("[w:"<<res.get_upper_bound()<<","<<res.get_user_time()<<"s]");
}	

inline
int CliqueWeighted::expand_weighted (int depth, int maxac, nodelist_t& l_v){
//////////////////////
//	initial date: 20/09/15
//	author: pss
//	COMMENTS:  HEAD_COLOR BACK_UP and RESTORE (an optimization related to cache issues) 
	
	/*vint vec(l_v.nodos,l_v.nodos+l_v.index);
	com::stl::print_collection<vector<int>>(vec); cout<<endl;*/
	
	int v, wv, maxUB=0; /* LB=maxno-maxac;	*/				//|Smax|-|S|
	res.inc_number_of_steps();
			
	//LISTA_BB(depth).erase_bit();
	int pos_node=solve_first_nodes(depth, maxac, l_v);			//note: also cleans solved set container LISTA_BB 	

	/*for(int i=0; i<l_v.index; i++){
			cout<<l_v.nodos[i]<<" ";
	}
	cout<<"Depth:"<<depth<<"--------------------------------"<<endl;*/

/////////////////////////////////////////////////
//MAIN LOOP
	for(int j=pos_node; j<l_v.index; j++){
		v=l_v.nodos[j];
		wv=g->get_wv(v);
			
#ifdef ROOT_VERTEX_PROGRESS
		if(depth==0){
			cout<<"root vertex: "<<v<<" :"<<m_lcol[HEAD_COLOR][v-1]<<endl;
		}
#endif	

//////////
//pruning by inheriting father UB (not tested by solved-first-nodes)
		if( m_lcol[HEAD_COLOR][v]<=(maxno-maxac)/*LB*/){
		//	//LOG_INFO("CUT SHOULD NOT OCCURR");
			LISTA_BB(depth).set_bit(v);
			continue;
		}
		
////////////
// Child node generation (neighbor vertices below v), combined with computation of icremental bounds
		AND(g->get_neighbors(v), LISTA_BB(depth), bbchild);	
		bbchild.to_old_vector(LISTA_L(depth).nodos, LISTA_L(depth).index);

		/*bbchild.print();cout<<endl;
		for(int i=0; i<LISTA_L(depth).index; i++){
			cout<<LISTA_L(depth).nodos[i]<<" ";
		}
		cout<<"----------------------------------"<<endl;*/

//////////
// Leaf node: check for a new solution

		//Leaf node: updates incumbent if necessary
		if( LISTA_L(depth).index==0){
			if((maxac+wv)>maxno){
				maxno=maxac+wv;						//NEW GLOBAL OPTIMUM FOUND

#ifdef STORE_SOLUTION
				res.set_UB(maxno);
				res.clear_all_solutions();
				m_path[depth]=v;
				res.add_solution(depth+1 /*size 1-based*/, m_path);

		#ifdef VIEW_PROGRESS
				stringstream sstr("");
				res.print_first_sol(sstr);

				//print weights
				vint sol1=res.get_first_solution();		//only one solution
				int wsol=0;
				for(int i=0; i<=depth; i++){
					wsol+=iop.m_lw[sol1[i]];
				}

				////if(!is_clique(sol1)){
				////	LOG_ERROR("bizarre clique");
				////}

				sstr<<"["<<wsol<<"]";
				LOG_INFO(sstr.str());
			
		#endif
#endif
					
			}
			m_lcol[HEAD_COLOR][v]=wv;
			LISTA_BB(depth).set_bit(v);
			continue;
		}
//////////////////
// simple linear bounds

		maxUB=0;
#ifdef  RDOLL_INC_UB_CUT
		//maximum ub of any of its adjacent predecessors
		int size_ub=0;
		for(int i=0; i<LISTA_L(depth).index; i++){
			int w=LISTA_L(depth).nodos[i];
			size_ub+=g->get_wv(w);
			if (maxUB<m_lcol[HEAD_COLOR][w] )
					maxUB=m_lcol[HEAD_COLOR][w];		
		}

		//subsumes size_ub and inc_ub
		maxUB=min<int>(size_ub,maxUB);	
#endif
		
		maxUB+=wv;

		if(maxUB<m_lcol[HEAD_COLOR][v]){			
			m_lcol[HEAD_COLOR][v]=maxUB;
			if(maxUB<=(maxno-maxac) /*LB*/){
				//LOG_INFO("INC UB CUT");
				LISTA_BB(depth).set_bit(v);
				continue;
			}
		}

///////////////////////////////////////////////////////
//TIGHTER ESTIMATES OF UPPER BOUND

////////////
// Color (Tomita) and Infrachrom attempts to reduce the bound
		
		maxUB=paint_UB(bbchild);							//configures infrachrom params also
		maxUB+=wv;
		if(maxUB<m_lcol[HEAD_COLOR][v]){					//note that this is by no means always true
			m_lcol[HEAD_COLOR][v]=maxUB;
			if(maxUB<=(maxno-maxac) /*LB*/){
				//LOG_INFO("COLOR CUT");
				LISTA_BB(depth).set_bit(v);
				continue;
			}
		}
				
////////////////////
//Infra-chrom (it seems this is the vast majority of cases from here)
//Note: The configuration of infra-chrom has been done during coloring:
//1-iop.update_color_sizes(maxUB_stored);
//2-iop.set_node_state_active(bbchild);
//3-iop.set_color_nb(maxUB_stored);	
		
		//*** sort color weights by size (20/10/16)
		//sort(m_lfv[0], m_lfv[0]+(maxUB-wv), std::greater<int>());

		//if( (maxUB-LB)==1 ){								//Note that m_lcol[HEAD_COLOR][v]-LB does not hold
		//	if(iop.filter()){
		//		//res.inc_counter(0,1);
		//		m_lcol[HEAD_COLOR][v]-=1;
		//		//LOG_INFO("ONE-SHOT-NO-SORT");
		//		LISTA_BB(maxac).set_bit(v);
		//		continue;
		//	}
		//}

		//int nb_conf=iop.init_maxsatz(v, LB);
		//if(nb_conf==EMPTY_ELEM){
		//	if(m_lcol[HEAD_COLOR][v]>LB)
		//			m_lcol[HEAD_COLOR][v]=LB;
		//	//LOG_INFO("DEEP INFRA-CHROM CUT");
		//	LISTA_BB(maxac).set_bit(v);
		//	continue;
		//}else if(m_lcol[HEAD_COLOR][v]>(maxUB-nb_conf)){
		//	m_lcol[HEAD_COLOR][v]=maxUB-nb_conf;
		//}
		
////////////////////////////////////////////////////////////
				
////////////////////
// BACK-UP HEAD_COLORS
		const int DEPTH_PLUS_ONE=depth+1;
		for(int i=0; i<LISTA_L(depth).index; i++){
			m_lcol[DEPTH_PLUS_ONE][LISTA_L(depth).nodos[i]]=m_lcol[HEAD_COLOR][LISTA_L(depth).nodos[i]];
		}

			
///////////////////////////////////////////////////////
// CANDIDATE EXPANSION

		//sets path
		m_path[depth]=v;
		maxUB=expand_weighted(depth+1, maxac+wv, LISTA_L(depth)) ;
		
//////////////////////////////////////////////
// BACKTRACK
	
	///////////////
	//RESTORE
		for(int i=0; i<LISTA_L(depth).index; i++){
			m_lcol[HEAD_COLOR][LISTA_L(depth).nodos[i]]=m_lcol[DEPTH_PLUS_ONE][LISTA_L(depth).nodos[i]];
		}

	////////////////////
	//updates UB of father vertex
		//if(maxUB!=ROOT_NODE){
		
			maxUB+=wv;
			if(maxUB<m_lcol[HEAD_COLOR][v]) {
				m_lcol[HEAD_COLOR][v]=maxUB;
				//no need to cut here since it the vertex will be added anyway to the solved set
				/*if(maxUB<=LB){
					LOG_ERROR("UB IMPROVED ON BACKTRACK TO LB:"<<maxUB<<":"<<LB);
				}*/
			}
	//	}

		//adds expanded vertex to solved set
		LISTA_BB(depth).set_bit(v);
		
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
	return min<int>(maxno-maxac /*not LB*/, maxUB); 
#else	
	return maxno-maxac;  /*not LB*/
#endif
}

inline
int CliqueWeighted::expand_weighted_R (int depth, int maxac, nodelist_t& l_v){
//////////////////////
//	initial date: 20/09/15
//	author: pss
//	COMMENTS:  HEAD_COLOR BACK_UP and RESTORE (an optimization related to cache issues) 
	
	/*vint vec(l_v.nodos,l_v.nodos+l_v.index);
	com::stl::print_collection<vector<int>>(vec); cout<<endl;*/
	
	int v, wv, maxUB=0; /* LB=maxno-maxac;	*/				//|Smax|-|S|
	res.inc_number_of_steps();
			
	//LISTA_BB(depth).erase_bit();
	int pos_node=solve_first_nodes(depth, maxac, l_v);			//note: also cleans solved set container LISTA_BB 	

	/*for(int i=0; i<l_v.index; i++){
			cout<<l_v.nodos[i]<<" ";
	}
	cout<<"Depth:"<<depth<<"--------------------------------"<<endl;*/

/////////////////////////////////////////////////
//MAIN LOOP
	for(int j=pos_node; j<l_v.index; j++){
		v=l_v.nodos[j];
		wv=g->get_wv(v);
			
#ifdef ROOT_VERTEX_PROGRESS
		if(depth==0){
			cout<<"root vertex: "<<v<<" :"<<m_lcol[HEAD_COLOR][v-1]<<endl;
		}
#endif	

//////////
//pruning by inheriting father UB (not tested by solved-first-nodes)
		if( m_lcol[HEAD_COLOR][v]<=(maxno-maxac)/*LB*/){
		//	//LOG_INFO("CUT SHOULD NOT OCCURR");
			LISTA_BB(depth).set_bit(v);
			continue;
		}
		
////////////
// Child node generation (neighbor vertices below v), combined with computation of icremental bounds
		AND(g->get_neighbors(v), LISTA_BB(depth), bbchild);	
		bbchild.to_old_vector(LISTA_L(depth).nodos, LISTA_L(depth).index);

		/*bbchild.print();cout<<endl;
		for(int i=0; i<LISTA_L(depth).index; i++){
			cout<<LISTA_L(depth).nodos[i]<<" ";
		}
		cout<<"----------------------------------"<<endl;*/

//////////
// Leaf node: check for a new solution

		//Leaf node: updates incumbent if necessary
		if( LISTA_L(depth).index==0){
			if((maxac+wv)>maxno){
				maxno=maxac+wv;						//NEW GLOBAL OPTIMUM FOUND

#ifdef STORE_SOLUTION
				res.set_UB(maxno);
				res.clear_all_solutions();
				m_path[depth]=v;
				res.add_solution(depth+1 /*size 1 based*/, m_path);

		#ifdef VIEW_PROGRESS
				stringstream sstr("");
				res.print_first_sol(sstr);

				//print weights
				//vint sol1=res.get_first_solution();		//only one solution
				//int wsol=0;
				//for(int i=0; i<=depth; i++){
				//	wsol+=g->get_wv(sol1[i]);
				//}

				////if(!is_clique(sol1)){
				////	LOG_ERROR("bizarre clique");
				////}

				//sstr<<"["<<wsol<<"]";
				LOG_INFO(sstr.str());
		#endif
#endif
				//update color at root node with new solution and return to root node
				//m_lcol[HEAD_COLOR][m_path[0]]=maxno;
				//return ROOT_NODE;
			}
			m_lcol[HEAD_COLOR][v]=wv;
			LISTA_BB(depth).set_bit(v);
			continue;
		}
//////////////////
// simple linear bounds

		maxUB=0;
#ifdef  RDOLL_INC_UB_CUT
		//maximum ub of any of its adjacent predecessors
		int size_ub=0;
		for(int i=0; i<LISTA_L(depth).index; i++){
			int w=LISTA_L(depth).nodos[i];
			size_ub+=g->get_wv(w);
			if (maxUB<m_lcol[HEAD_COLOR][w] )
					maxUB=m_lcol[HEAD_COLOR][w];		
		}

		//subsumes size_ub and inc_ub
		maxUB=min<int>(size_ub,maxUB);	
#endif
		
		maxUB+=wv;

		if(maxUB<m_lcol[HEAD_COLOR][v]){			
			m_lcol[HEAD_COLOR][v]=maxUB;
			if(maxUB<=(maxno-maxac) /*LB*/){
				//LOG_INFO("INC UB CUT");
				LISTA_BB(depth).set_bit(v);
				continue;
			}
		}

///////////////////////////////////////////////////////
//TIGHTER ESTIMATES OF UPPER BOUND

////////////
// Color (Tomita) and Infrachrom attempts to reduce the bound
		
		maxUB=paint_UB_R(maxac+wv,bbchild);							//configures infrachrom params also
		maxUB+=wv;
		if(maxUB<m_lcol[HEAD_COLOR][v]){					//note that this is by no means always true
			m_lcol[HEAD_COLOR][v]=maxUB;
			if(maxUB<=(maxno-maxac) /*LB*/){
				//LOG_INFO("RECOLOR CUT");
				LISTA_BB(depth).set_bit(v);
				continue;
			}
		}
				
////////////////////
//Infra-chrom (it seems this is the vast majority of cases from here)
//Note: The configuration of infra-chrom has been done during coloring:
//1-iop.update_color_sizes(maxUB_stored);
//2-iop.set_node_state_active(bbchild);
//3-iop.set_color_nb(maxUB_stored);	
		
		//if( (maxUB-LB)==1 ){															//Note that m_lcol[HEAD_COLOR][v]-LB does not hold
		//	if(iop.filter()){
		//		//res.inc_counter(0,1);
		//		m_lcol[HEAD_COLOR][v]-=1;
		//		//LOG_INFO("ONE-SHOT-NO-SORT");
		//		LISTA_BB(maxac).set_bit(v);
		//		continue;
		//	}
		//}

		//int nb_conf=iop.init_maxsatz(v, LB);
		//if(nb_conf==EMPTY_ELEM){
		//	if(m_lcol[HEAD_COLOR][v]>LB)
		//			m_lcol[HEAD_COLOR][v]=LB;
		//	//LOG_INFO("DEEP INFRA-CHROM CUT");
		//	LISTA_BB(maxac).set_bit(v);
		//	continue;
		//}else if(m_lcol[HEAD_COLOR][v]>(maxUB-nb_conf)){
		//	m_lcol[HEAD_COLOR][v]=maxUB-nb_conf;
		//}
		
////////////////////////////////////////////////////////////
				
////////////////////
// BACK-UP HEAD_COLORS
		const int DEPTH_PLUS_ONE=depth+1;
		for(int i=0; i<LISTA_L(depth).index; i++){
			m_lcol[DEPTH_PLUS_ONE][LISTA_L(depth).nodos[i]]=m_lcol[HEAD_COLOR][LISTA_L(depth).nodos[i]];
		}

			
///////////////////////////////////////////////////////
// CANDIDATE EXPANSION

		//sets path
		m_path[depth]=v;

		//child node expansion
		//if( (maxUB=expand_weighted(depth+1, maxac+wv, LISTA_L(depth)) )==ROOT_NODE ){ 
		//	//if(depth!=0){	
		//	//	LB=maxno-maxac;
		//	//	//return ROOT_NODE;
		//	//}else{		//ROOT NODE
		//	//	LB=maxno-maxac;  	//update LB becuase a new maxno (solution) has been found
		//	//}
		//}
		maxUB=expand_weighted_R(depth+1, maxac+wv, LISTA_L(depth)) ;
		
//////////////////////////////////////////////
// BACKTRACK
		//LB=maxno-maxac;		//just in case
	///////////////
	//RESTORE
		for(int i=0; i<LISTA_L(depth).index; i++){
			m_lcol[HEAD_COLOR][LISTA_L(depth).nodos[i]]=m_lcol[DEPTH_PLUS_ONE][LISTA_L(depth).nodos[i]];
		}

	////////////////////
	//updates UB of father vertex
		//if(maxUB!=ROOT_NODE){
		
			maxUB+=wv;
			if(maxUB<m_lcol[HEAD_COLOR][v]) {
				m_lcol[HEAD_COLOR][v]=maxUB;
				//no need to cut here since it the vertex will be added anyway to the solved set
				/*if(maxUB<=LB){
					LOG_ERROR("UB IMPROVED ON BACKTRACK TO LB:"<<maxUB<<":"<<LB);
				}*/
			}
	//	}

		//adds expanded vertex to solved set
		LISTA_BB(depth).set_bit(v);
		
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
	return min<int>(maxno-maxac /*not LB*/, maxUB); 
#else	
	return maxno-maxac;  /*not LB*/
#endif
}

inline
int CliqueWeighted::expand_weighted_X (int depth, int maxac, nodelist_t& l_v){
//////////////////////
//	initial date: 20/09/15
//	author: pss
//	COMMENTS:  HEAD_COLOR BACK_UP and RESTORE (an optimization related to cache issues) 
	
	/*vint vec(l_v.nodos,l_v.nodos+l_v.index);
	com::stl::print_collection<vector<int>>(vec); cout<<endl;*/
	
	int v, wv, maxUB=0; /* LB=maxno-maxac;	*/				//|Smax|-|S|
	res.inc_number_of_steps();
			
	//LISTA_BB(depth).erase_bit();
	int pos_node=solve_first_nodes(depth, maxac, l_v);					//note: also cleans solved set container LISTA_BB 	
	//int pos_node=solve_first_nodes_inc(depth, maxac, l_v);			//note: also cleans solved set container LISTA_BB 	
	

	/*for(int i=0; i<l_v.index; i++){
			cout<<l_v.nodos[i]<<" ";
	}
	cout<<"Depth:"<<depth<<"--------------------------------"<<endl;*/

/////////////////////////////////////////////////
//MAIN LOOP
	for(int j=pos_node; j<l_v.index; j++){
		v=l_v.nodos[j];
		wv=g->get_wv(v);
			
#ifdef ROOT_VERTEX_PROGRESS
		if(depth==0){
			cout<<"root vertex: "<<v<<" :"<<m_lcol[HEAD_COLOR][v-1]<<endl;
		}
#endif	

//////////
//pruning by inheriting father UB (not tested by solved-first-nodes)
		if( m_lcol[HEAD_COLOR][v]<=(maxno-maxac)/*LB*/){
		//	LOG_ERROR("CUT SHOULD NOT OCCURR");
			LISTA_BB(depth).set_bit(v);
			continue;
		}
		
////////////
// Child node generation (neighbor vertices below v), combined with computation of icremental bounds
		AND(g->get_neighbors(v), LISTA_BB(depth), bbchild);	
		bbchild.to_old_vector(LISTA_L(depth).nodos, LISTA_L(depth).index);

		/*bbchild.print();cout<<endl;
		for(int i=0; i<LISTA_L(depth).index; i++){
			cout<<LISTA_L(depth).nodos[i]<<" ";
		}
		cout<<"----------------------------------"<<endl;*/

//////////
// Leaf node: check for a new solution

		//Leaf node: updates incumbent if necessary
		if( LISTA_L(depth).index==0){
			if((maxac+wv)>maxno){
				maxno=maxac+wv;						//NEW GLOBAL OPTIMUM FOUND

#ifdef STORE_SOLUTION
				res.set_UB(maxno);
				res.clear_all_solutions();
				m_path[depth]=v;
				res.add_solution(depth+1 /*size 1 based*/, m_path);

		#ifdef VIEW_PROGRESS
				stringstream sstr("");
				res.print_first_sol(sstr);

				////print weights
				//vint sol1=res.get_first_solution();		//only one solution
				//int wsol=0;
				//for(int i=0; i<=depth; i++){
				//	wsol+=g->get_wv(sol1[i]);
				//}

				//////if(!is_clique(sol1)){
				//////	LOG_ERROR("bizarre clique");
				//////}

				//sstr<<"["<<wsol<<"]";
				LOG_INFO(sstr.str());
		#endif
#endif
				//update color at root node with new solution and return to root node
				//m_lcol[HEAD_COLOR][m_path[0]]=maxno;
				//return ROOT_NODE;
			}
			m_lcol[HEAD_COLOR][v]=wv;
			LISTA_BB(depth).set_bit(v);
			continue;
		}
//////////////////
// simple linear bounds

		maxUB=0;
#ifdef  RDOLL_INC_UB_CUT
		//maximum ub of any of its adjacent predecessors
		int size_ub=0;
		for(int i=0; i<LISTA_L(depth).index; i++){
			int w=LISTA_L(depth).nodos[i];
			size_ub+=g->get_wv(w);
			if (maxUB<m_lcol[HEAD_COLOR][w] )
					maxUB=m_lcol[HEAD_COLOR][w];		
		}

		//subsumes size_ub and inc_ub
		maxUB=min<int>(size_ub,maxUB);	
#endif
		
		maxUB+=wv;

		if(maxUB<m_lcol[HEAD_COLOR][v]){			
			m_lcol[HEAD_COLOR][v]=maxUB;
			if(maxUB<=(maxno-maxac) /*LB*/){
				//LOG_INFO("INC UB CUT");
				LISTA_BB(depth).set_bit(v);
				continue;
			}
		}

///////////////////////////////////////////////////////
//TIGHTER ESTIMATES OF UPPER BOUND

////////////
// Color (Tomita) and Infrachrom attempts to reduce the bound
		
		maxUB=paint_UB_X(maxac+wv,bbchild);					//configures infrachrom params also
		maxUB+=wv;
		if(maxUB<m_lcol[HEAD_COLOR][v]){					//note that this is by no means always true
			m_lcol[HEAD_COLOR][v]=maxUB;
			if(maxUB<=(maxno-maxac) /*LB*/){
				//LOG_INFO("SIMPLE INFRA-CHROM CUT");
				LISTA_BB(depth).set_bit(v);
				continue;
			}
		}
				
////////////////////
//Infra-chrom (it seems this is the vast majority of cases from here)
//Note: The configuration of infra-chrom has been done during coloring:
//1-iop.update_color_sizes(maxUB_stored);
//2-iop.set_node_state_active(bbchild);
//3-iop.set_color_nb(maxUB_stored);	
		
		//if( (maxUB-LB)==1 ){															//Note that m_lcol[HEAD_COLOR][v]-LB does not hold
		//	if(iop.filter()){
		//		//res.inc_counter(0,1);
		//		m_lcol[HEAD_COLOR][v]-=1;
		//		//LOG_INFO("ONE-SHOT-NO-SORT");
		//		LISTA_BB(maxac).set_bit(v);
		//		continue;
		//	}
		//}

		//int nb_conf=iop.init_maxsatz(v, LB);
		//if(nb_conf==EMPTY_ELEM){
		//	if(m_lcol[HEAD_COLOR][v]>LB)
		//			m_lcol[HEAD_COLOR][v]=LB;
		//	//LOG_INFO("DEEP INFRA-CHROM CUT");
		//	LISTA_BB(maxac).set_bit(v);
		//	continue;
		//}else if(m_lcol[HEAD_COLOR][v]>(maxUB-nb_conf)){
		//	m_lcol[HEAD_COLOR][v]=maxUB-nb_conf;
		//}
		
////////////////////////////////////////////////////////////
				
////////////////////
// BACK-UP HEAD_COLORS
		const int DEPTH_PLUS_ONE=depth+1;
		for(int i=0; i<LISTA_L(depth).index; i++){
			m_lcol[DEPTH_PLUS_ONE][LISTA_L(depth).nodos[i]]=m_lcol[HEAD_COLOR][LISTA_L(depth).nodos[i]];
		}

			
///////////////////////////////////////////////////////
// CANDIDATE EXPANSION

		//sets path
		m_path[depth]=v;

		//child node expansion
		//if( (maxUB=expand_weighted(depth+1, maxac+wv, LISTA_L(depth)) )==ROOT_NODE ){ 
		//	//if(depth!=0){	
		//	//	LB=maxno-maxac;
		//	//	//return ROOT_NODE;
		//	//}else{		//ROOT NODE
		//	//	LB=maxno-maxac;  	//update LB becuase a new maxno (solution) has been found
		//	//}
		//}
		maxUB=expand_weighted_X(depth+1, maxac+wv, LISTA_L(depth));
		
//////////////////////////////////////////////
// BACKTRACK
		//LB=maxno-maxac;		//just in case
	///////////////
	//RESTORE
		for(int i=0; i<LISTA_L(depth).index; i++){
			m_lcol[HEAD_COLOR][LISTA_L(depth).nodos[i]]=m_lcol[DEPTH_PLUS_ONE][LISTA_L(depth).nodos[i]];
		}

	////////////////////
	//updates UB of father vertex
		//if(maxUB!=ROOT_NODE){
		
			maxUB+=wv;
			if(maxUB<m_lcol[HEAD_COLOR][v]) {
				m_lcol[HEAD_COLOR][v]=maxUB;
				//no need to cut here since it the vertex will be added anyway to the solved set
				/*if(maxUB<=LB){
					LOG_ERROR("UB IMPROVED ON BACKTRACK TO LB:"<<maxUB<<":"<<LB);
				}*/
			}
	//	}

		//adds expanded vertex to solved set
		LISTA_BB(depth).set_bit(v);
		
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
	return min<int>(maxno-maxac /*not LB*/, maxUB); 
#else	
	return maxno-maxac;  /*not LB*/
#endif
}

inline
int CliqueWeighted::expand_weighted_R_satz (int depth, int maxac, nodelist_t& l_v){
//////////////////////
//	initial date: 20/09/15
//	author: pss
//	COMMENTS:  HEAD_COLOR BACK_UP and RESTORE (an optimization related to cache issues) 
	
	/*vint vec(l_v.nodos,l_v.nodos+l_v.index);
	com::stl::print_collection<vector<int>>(vec); cout<<endl;*/
	
	int v, wv, maxUB=0; /* LB=maxno-maxac;	*/				//|Smax|-|S|
	res.inc_number_of_steps();
			
	LISTA_BB(depth).erase_bit();
	int pos_node=solve_first_nodes(depth, maxac, l_v);					//note: also cleans solved set container LISTA_BB 	
	
	/*for(int i=0; i<l_v.index; i++){
			cout<<l_v.nodos[i]<<" ";
	}
	cout<<"Depth:"<<depth<<"--------------------------------"<<endl;*/

/////////////////////////////////////////////////
//MAIN LOOP
	for(int j=pos_node; j<l_v.index; j++){
		v=l_v.nodos[j];
		wv=g->get_wv(v);
			
#ifdef ROOT_VERTEX_PROGRESS
		if(depth==0){
			cout<<"root vertex: "<<v<<" :"<<m_lcol[HEAD_COLOR][v-1]<<endl;
		}
#endif	

//////////
//pruning by inheriting father UB (not tested by solved-first-nodes)
		if( m_lcol[HEAD_COLOR][v]<=(maxno-maxac)/*LB*/){
			m_lcol[HEAD_COLOR][v]=estimate_ub_first_nodes(m_lcol[HEAD_COLOR][v], v, j, l_v);
			//	m_lcol[HEAD_COLOR][v]=estimate_ub_first_nodes(m_lcol[HEAD_COLOR][v], v, j, l_v);
			//LOG_ERROR("CUT SHOULD NOT OCCURR");
			LISTA_BB(depth).set_bit(v);
			continue;
		}
		
////////////
// Child node generation (neighbor vertices below v), combined with computation of icremental bounds
		AND(g->get_neighbors(v), LISTA_BB(depth), bbchild);	
		bbchild.to_old_vector(LISTA_L(depth).nodos, LISTA_L(depth).index);

		/*bbchild.print();cout<<endl;
		for(int i=0; i<LISTA_L(depth).index; i++){
			cout<<LISTA_L(depth).nodos[i]<<" ";
		}
		cout<<"----------------------------------"<<endl;*/

//////////
// Leaf node: check for a new solution

		//Leaf node: updates incumbent if necessary
		if( LISTA_L(depth).index==0){
			if((maxac+wv)>maxno){
				maxno=maxac+wv;						//NEW GLOBAL OPTIMUM FOUND

#ifdef STORE_SOLUTION
				res.set_UB(maxno);
				res.clear_all_solutions();
				m_path[depth]=v;
				res.add_solution(depth+1 /*size 1 based*/, m_path);

		#ifdef VIEW_PROGRESS
				stringstream sstr("");
				res.print_first_sol(sstr);

				//print weights
				vint sol1=res.get_first_solution();		//only one solution
				int wsol=0;
				for(int i=0; i<=depth; i++){
					wsol+=iop.m_lw[sol1[i]];
				}

				////if(!is_clique(sol1)){
				////	LOG_ERROR("bizarre clique");
				////}

				sstr<<"["<<wsol<<"]";
				LOG_INFO(sstr.str());
		#endif
#endif
				//update color at root node with new solution and return to root node
				//m_lcol[HEAD_COLOR][m_path[0]]=maxno;
				//return ROOT_NODE;
			}
			m_lcol[HEAD_COLOR][v]=wv;
			LISTA_BB(depth).set_bit(v);
			continue;
		}
//////////////////
// simple linear bounds

		maxUB=0;
#ifdef  RDOLL_INC_UB_CUT
		//maximum ub of any of its adjacent predecessors
		int size_ub=0;
		for(int i=0; i<LISTA_L(depth).index; i++){
			int w=LISTA_L(depth).nodos[i];
			size_ub+=wv;
			if (maxUB<m_lcol[HEAD_COLOR][w] )
					maxUB=m_lcol[HEAD_COLOR][w];		
		}

		//subsumes size_ub and inc_ub
		maxUB=min<int>(size_ub,maxUB);	
#endif
		
		maxUB+=wv;

		if(maxUB<m_lcol[HEAD_COLOR][v]){			
			m_lcol[HEAD_COLOR][v]=maxUB;
			if(maxUB<=(maxno-maxac) /*LB*/){
				//LOG_INFO("INC UB CUT");
				LISTA_BB(depth).set_bit(v);
				continue;
			}
		}

///////////////////////////////////////////////////////
//TIGHTER ESTIMATES OF UPPER BOUND

////////////
// Color (Tomita) and Infrachrom attempts to reduce the bound
		
		maxUB=paint_UB_R_minw_satz(maxac+wv,bbchild);					//configures infrachrom params also
	//	maxUB=paint_UB_RX_minw_satz(maxac+wv,bbchild);					//configures infrachrom params also
	//	maxUB=paint_UB_X_minw_satz(maxac+wv,bbchild);					//configures infrachrom params also
		maxUB+=wv;
		if(maxUB<m_lcol[HEAD_COLOR][v]){								//note that this is by no means always true
			m_lcol[HEAD_COLOR][v]=maxUB;
			if(maxUB<=(maxno-maxac) /*LB*/){
				//LOG_INFO("SIMPLE INFRA-CHROM CUT");

			//	iop.reset_context_for_weights_of_literals();
				LISTA_BB(depth).set_bit(v);
				continue;
			}
		}
				
////////////////////
//Infra-chrom (it seems this is the vast majority of cases from here)
//Note: The configuration of infra-chrom has been done during coloring:
//1-iop.update_color_sizes(maxUB_stored);
//2-iop.set_node_state_active(bbchild);
//3-iop.set_color_nb(maxUB_stored);	
		
		//if( (maxUB-LB)==1 ){									//Note that m_lcol[HEAD_COLOR][v]-LB does not hold
		//	if(iop.filter()){
		//		//res.inc_counter(0,1);
		//		m_lcol[HEAD_COLOR][v]-=1;
		//		//LOG_INFO("ONE-SHOT-NO-SORT");
		//		LISTA_BB(maxac).set_bit(v);
		//		continue;
		//	}
		//}

		int lb=maxno-maxac;
		int nb_conf=iop.init_weighted_maxsatz(v, maxUB, lb);
		int new_ub=maxUB-nb_conf;
			
		if(new_ub<m_lcol[HEAD_COLOR][v]){					//note that this is by no means always true
			m_lcol[HEAD_COLOR][v]=new_ub;
			if(new_ub<=lb /*LB*/){
				//LOG_INFO("SIMPLE WEIGHTED INFRA-CHROM CUT");
				LISTA_BB(depth).set_bit(v);
				continue;
			}
		}
		
////////////////////////////////////////////////////////////
				
////////////////////
// BACK-UP HEAD_COLORS
		const int DEPTH_PLUS_ONE=depth+1;
		for(int i=0; i<LISTA_L(depth).index; i++){
			m_lcol[DEPTH_PLUS_ONE][LISTA_L(depth).nodos[i]]=m_lcol[HEAD_COLOR][LISTA_L(depth).nodos[i]];
		}

			
///////////////////////////////////////////////////////
// CANDIDATE EXPANSION

		//sets path
		m_path[depth]=v;
		maxUB=expand_weighted_R_satz(depth+1, maxac+wv, LISTA_L(depth));
		
//////////////////////////////////////////////
// BACKTRACK
	
	///////////////
	//RESTORE
		for(int i=0; i<LISTA_L(depth).index; i++){
			m_lcol[HEAD_COLOR][LISTA_L(depth).nodos[i]]=m_lcol[DEPTH_PLUS_ONE][LISTA_L(depth).nodos[i]];
		}

	////////////////////
	//updates UB of father vertex
		//if(maxUB!=ROOT_NODE){
		
			maxUB+=wv;
			if(maxUB<m_lcol[HEAD_COLOR][v]) {
				m_lcol[HEAD_COLOR][v]=maxUB;
				//no need to cut here since it the vertex will be added anyway to the solved set
				/*if(maxUB<=LB){
					LOG_ERROR("UB IMPROVED ON BACKTRACK TO LB:"<<maxUB<<":"<<LB);
				}*/
			}
	//	}

		//adds expanded vertex to solved set
		LISTA_BB(depth).set_bit(v);
		
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
	return min<int>(maxno-maxac /*not LB*/, maxUB); 
#else	
	return maxno-maxac;  /*not LB*/
#endif
}

inline
int CliqueWeighted::paint_UB_ow(const bitarray& bb){
////////////////////////
// classical independent set coloring of subgraph bb
// extended to the weighted version
// first update: 10/10/16
// last update: 10/10/16
//
// RETURNS: color-based upper bound for weighted graph or EMPTY_ELEM if error
//
// REMARKS: 
// I. fast implementation: does not store colors, nor sizes, not even the total number of colors 
// II. not valid for post-processing as in reNumber
			
	int nBB=EMPTY_ELEM, v=EMPTY_ELEM;
	int pc=(m_unsel=bb).popcn64();
	
	if(pc==0)							//***emtpy check tests, remove for optimal performance
		return 0;					

	int wUB=0;
	bool is_first_vertex;
	while(true){ 
		m_sel=m_unsel;
		m_sel.init_scan(bbo::DESTRUCTIVE);
		is_first_vertex=true;
		while(true){
			v=m_sel.next_bit_del(nBB,m_unsel);
			if(v==EMPTY_ELEM)
				break;

			//updates total weight
			if(is_first_vertex){
				wUB+=g->get_wv(v);
				is_first_vertex=false;
			}

			//checks exit condition
			if((--pc)==0){
				return wUB;
			}						
			m_sel.erase_block(nBB,g->get_neighbors(v));		
		}
	}
	LOG_ERROR("paint_UB():bizarre coloring");
	return EMPTY_ELEM;									//should not reach here 
}

inline
int CliqueWeighted::paint_UB(const bitarray& bb){
////////////////////////
// classical independent set coloring of subgraph bb
// extended to the weighted version
// first update: 10/10/16
// last update: 10/10/16
//
// RETURNS: color-based upper bound for weighted graph or EMPTY_ELEM if error
//
// REMARKS: 
// I. fast implementation: does not store colors, nor sizes, not even the total number of colors 
// II. not valid for post-processing as in reNumber
			
	int nBB=EMPTY_ELEM, v=EMPTY_ELEM;
	int pc=(m_unsel=bb).popcn64();
	int maxwcol;

	if(pc==0)							//***emtpy check tests, remove for optimal performance
		return 0;					

	int wUB=0;
	//bool is_first_vertex;
	while(true){ 
		m_sel=m_unsel;
		m_sel.init_scan(bbo::DESTRUCTIVE);
		//is_first_vertex=true;
		maxwcol=0;
		while(true){
			v=m_sel.next_bit_del(nBB,m_unsel);
			if(v==EMPTY_ELEM)
				break;

			//updates total weight
			int wv=g->get_wv(v);
			if(maxwcol<wv){
				maxwcol=wv;
			}
			/*if(is_first_vertex){
				wUB+=g->get_wv(v);
				is_first_vertex=false;
			}*/

			//checks exit condition
			if((--pc)==0){
				return wUB+maxwcol;
			}						
			m_sel.erase_block(nBB,g->get_neighbors(v));		
		}
		wUB+=maxwcol;
	}
	LOG_ERROR("paint_UB():bizarre coloring");
	return EMPTY_ELEM;									//should not reach here 
}
inline
int CliqueWeighted::paint_UB_R_ow(int maxac, const bitarray& bb){
////////////////////////
// classical independent set coloring of subgraph bb with recoloring
// extended to the weighted version
// first update: 10/10/16
// last update: 10/10/16
//
// PARAMS: maxac-weight of current clique built, including current expanded node, bb-the subgraph to be colored
//
// RETURNS: color-based upper bound for weighted graph or EMPTY_ELEM if error
//

	int col=1, nBB=EMPTY_ELEM, v=EMPTY_ELEM, vswap=EMPTY_ELEM;
	int pc=(m_unsel=bb).popcn64();
	const int KMINW=maxno-maxac;
	
	if(pc==0)							//***empty check tests, remove for optimal performance
		return 0;					

	int wUB=0;
	bool is_first_vertex;
	while(true){ 
		m_colsets[col]=m_unsel;
		m_colsets[col].init_scan(bbo::NON_DESTRUCTIVE);
		is_first_vertex=true;
		while(true){
next_v:		v=m_colsets[col].next_bit(nBB,m_unsel);
			if(v==EMPTY_ELEM)
				break;
			int wv=g->get_wv(v);
//////////////////////////////
//RECOLORING ATTEMPT
			if( ((wUB+wv)>KMINW) && col>=3){
			//if( (wUB>KMINW) && col>=3){
			//if(col>=3 ){
				for(int recol=1; recol<col-1; recol++){
					/*if(v<m_lfv[recol]) 
						continue;*/
					int pc_swap=m_colsets[recol].single_disjoint(g->get_neighbors(v), vswap);
					if(pc_swap==1){	//candidate color class found
						//for(int j=recol+1; j<kmin; j++){
						for(int j=recol+1; j<col; j++){
							//if(vswap<m_lfv[j]) continue;
							if(m_colsets[j].is_disjoint(g->get_neighbors(vswap))){
							
								//erase 'from' color sets
								m_colsets[recol].erase_bit(vswap);
								m_colsets[col].erase_bit(v);

								int wvswap=g->get_wv(vswap);

								//update weights according to recol
								if(wvswap>=m_lfv[recol]){
									int vfrec=m_colsets[recol].lsbn64();
									if(vfrec==EMPTY_ELEM || v<vfrec){
										m_lfv[recol]=wv;
										wUB+=wv-wvswap;
									}else {
										int wvfrec=g->get_wv(vfrec);
										m_lfv[recol]=wvfrec;
										wUB+=(wvfrec-wvswap);
									}

								//	LOG_ERROR("MOVING FIRST VERTEX: VSWAP");
									
								}else if (wv>m_lfv[recol]){
									wUB+=(wv-m_lfv[recol]);
									m_lfv[recol]=wv;
								}

								//update weights according to j
								if(wvswap>m_lfv[j]){
									wUB+=(wvswap-m_lfv[j]);
									m_lfv[j]=wvswap;
								}

								//set 'to' color sets
								m_colsets[j].set_bit(vswap);
								m_colsets[recol].set_bit(v);
										
								if((--pc)==0){
									return wUB;
								}else goto next_v;
							}
						}
					} else if(pc_swap==0){
						//LOG_ERROR("PC_SWAP=0: VSWAP");
						/*int creco=m_colsets[recol].lsbn64();
						if(creco==EMPTY_ELEM){
							wUB+=g->get_wv(v);
						}else if(v<creco) 
							wUB+=(g->get_wv(v)-g->get_wv(creco));*/

						/*if(v<m_lfv[recol]){
							LOG_ERROR("PCSWAP=0: bizarre color set");
						}*/
						/*wUB+=(g->get_wv(v)-g->get_wv(m_lfv[recol]));
						m_lfv[recol]=v;*/

						if (wv>m_lfv[recol]){
							wUB+=(wv-m_lfv[recol]);
							m_lfv[recol]=wv;
						}

						//update color sets
						m_colsets[col].erase_bit(v);
						m_colsets[recol].set_bit(v);

						if((--pc)==0){
							return wUB;
						}else goto next_v;
					}
				}
			}
///////////////////////////////

			//updates total weight
			if(is_first_vertex){
				m_lfv[col]=wv;
				wUB+=wv;
				is_first_vertex=false;
			}

			//checks exit condition
			if((--pc)==0){
				return wUB;
			}						
			m_colsets[col].erase_block(nBB,g->get_neighbors(v));		
		}

		if(!is_first_vertex)
						col++;
		
	}
	LOG_ERROR("paint_UB():bizarre coloring");
	return EMPTY_ELEM;									//should not reach here 
}

inline
int CliqueWeighted::paint_UB_R_satz(int maxac, const bitarray& bb){
////////////////////////
// classical independent set coloring of subgraph bb with recoloring
// extended to the weighted version
// first update: 10/10/16
// last update: 10/10/16
//
// PARAMS: maxac-weight of current clique built, including current expanded node, bb-the subgraph to be colored
//
// RETURNS: color-based upper bound for weighted graph or EMPTY_ELEM if error
//

	int col=1, nBB=EMPTY_ELEM, v=EMPTY_ELEM, vswap=EMPTY_ELEM, wUB=0;
	int pc=(m_unsel=bb).popcn64();
	const int KMINW=maxno-maxac; const int NB_OF_BB_NODES_MINUS_ONE=iop.NB_OF_BB_NODES-1;
	bool is_first_vertex=true;
	
	if(pc==0)							//***empty check tests, remove for optimal performance
		return 0;					

	iop.m_colSets[1].erase_bit(true);	//lazy: size=0
	while(true){ 
		bitarray& bbcol=iop.m_colSets[col].bb;
		for(int i=0; i<iop.NB_OF_BB_NODES; i++){
			bbcol.get_bitboard(i)=m_unsel.get_bitboard(i);
		}
		bbcol.init_scan(bbo::NON_DESTRUCTIVE);
		is_first_vertex=true;
		while(true){
next_v:		v=bbcol.next_bit(nBB,m_unsel);
			if(v==EMPTY_ELEM)
				break;
			int wv=g->get_wv(v);
//////////////////////////////
//RECOLORING ATTEMPT
			if( ((wUB+wv)>KMINW) && col>=3){
			//if( (wUB>KMINW) && col>=3){
			//if(col>=3 ){
				for(int recol=1; recol<col-1; recol++){
					/*if(v<m_lfv[recol]) 
						continue;*/
					int pc_swap=iop.m_colSets[recol].bb.single_disjoint(0, NB_OF_BB_NODES_MINUS_ONE, g->get_neighbors(v), vswap);
					if(pc_swap==1){	//candidate color class found
						//for(int j=recol+1; j<kmin; j++){
						for(int j=recol+1; j<col; j++){
							//if(vswap<m_lfv[j]) continue;
							if(iop.m_colSets[j].bb.is_disjoint(0, NB_OF_BB_NODES_MINUS_ONE, g->get_neighbors(vswap))){
							
								//erase 'from' color sets
								iop.m_colSets[recol].erase_bit(vswap);
								iop.m_colSets[col].bb.erase_bit(v);
								iop.node_iset_no[vswap]=j;
								iop.node_iset_no[v]=recol;
								
								int wvswap=g->get_wv(vswap);
								//if(wvswap==/*>=*/m_lfv[recol]){
								//	if(wv>wvswap || iop.m_colSets[recol].size==0){
								//		wUB+=(wv-wvswap);
								//		m_lfv[recol]=wv;
								//	}else{  /* I do not know the best UB so I leave the one I have */
								//		; 
								//	}
								//}else if (wv>m_lfv[recol]){
								//	wUB+=(wv-m_lfv[recol]);
								//	m_lfv[recol]=wv;
								//}

								if (wv>m_lfv[recol] || iop.m_colSets[recol].size==0){
									wUB+=(wv-m_lfv[recol]);
									m_lfv[recol]=wv;
								}


								//update weights according to recol
								//if(wvswap>=m_lfv[recol]){
								//	int vfrec=iop.m_colSets[recol].bb.lsbn64();
								//	if(vfrec==EMPTY_ELEM || v<vfrec){
								//		/*m_lfv[recol]=wv;
								//		wUB+=(wv-wvswap);*/
								//	}else {
								//		int wvfrec=g->get_wv(vfrec);
								//		/*m_lfv[recol]=wvfrec;
								//		wUB+=(wvfrec-wvswap);*/
								//	}

								//}else if (wv>m_lfv[recol]){
								//	wUB+=(wv-m_lfv[recol]);
								//	m_lfv[recol]=wv;
								//}


								//update weights according to j
								if(wvswap>m_lfv[j]){
									wUB+=(wvswap-m_lfv[j]);
									m_lfv[j]=wvswap;
								}

								//set 'to' color sets
								iop.m_colSets[j].push(vswap);
								iop.m_colSets[recol].push(v);
																		
								if((--pc)==0){
									iop.set_node_state_active(bb);
									if(iop.m_colSets[col].size==0){	//** TODO:optimize
										m_nCol=col-1;
										iop.set_color_nb(m_nCol);										
									}else{
										m_nCol=col;
										iop.set_color_nb(col);										
									}
									return wUB;
								}else goto next_v;
							}
						}
					} else if(pc_swap==0){
						if (wv>m_lfv[recol]){
							wUB+=(wv-m_lfv[recol]);
							m_lfv[recol]=wv;
						}

						//update color sets
						iop.m_colSets[col].bb.erase_bit(v);
						iop.m_colSets[recol].push(v);
						iop.node_iset_no[v]=recol;
					
						if((--pc)==0){
							iop.set_node_state_active(bb);
							if(iop.m_colSets[col].size==0){	//** TODO:optimize
								m_nCol=col-1;
								iop.set_color_nb(m_nCol);
							}else{
								m_nCol=col;
								iop.set_color_nb(col);
							}
							return wUB;
						}else goto next_v;
					}
				}
			}
///////////////////////////////

			//stores color label
			iop.m_colSets[col].size++;						//adds v (already in the bitset)
			iop.node_iset_no[v]=col;

			//updates total weight
			if(is_first_vertex){
				m_lfv[col]=wv;
				wUB+=wv;
				is_first_vertex=false;
			}else if (wv>m_lfv[col]){
				wUB+=(wv-m_lfv[col]);
				m_lfv[col]=wv;
			}
			
			//checks exit condition
			if((--pc)==0){
				m_nCol=col;
				iop.set_color_nb(col);
				iop.set_node_state_active(bb);
				//sort(m_lfv+1, m_lfv+col+1 , greater<int>());

			/*	int res=accumulate(m_lfv+1, m_lfv+col+1, 0, std::plus<int>());
				copy(m_lfv+1,m_lfv+col+1, ostream_iterator<int>(cout," ") ); cout<<endl;
				if(res!=wUB){
					LOG_ERROR("paint::BIZARRE");
				}*/

				return wUB;
			}	

			bitarray& nbor=g->get_neighbors(v);
			for(int i=nBB; i<iop.NB_OF_BB_NODES; i++){
				bbcol.get_bitboard(i)&= ~nbor.get_bitboard(i);
			}
			//iop.m_colSets[col].bb.erase_block(nBB,g->get_neighbors(v));
					
		}
		if(!is_first_vertex){
			col++;
			iop.m_colSets[col].erase_bit(true);		//lazy: size=0
			//iop.m_colSets[col].bb.get_bitboard(iop.NB_OF_BB_NODES)=0;  //*** necessary?
		}
	}
	LOG_ERROR("paint_UB():bizarre coloring");
	return EMPTY_ELEM;									//should not reach here 
}


inline
int CliqueWeighted::paint_UB_R_minw_satz(int maxac, const bitarray& bb){
////////////////////////
// classical independent set coloring of subgraph bb with recoloring
// extended to the weighted version
//
// ORDERIN REQUIRED: vertices sorted by non_increasing weight
// first update:30/10/16
// last update: 30/10/16
//
// PARAMS: maxac-weight of current clique built, including current expanded node, bb-the subgraph to be colored
//
// RETURNS: color-based upper bound for weighted graph or EMPTY_ELEM if error
//

	int col=1, nBB=EMPTY_ELEM, v=EMPTY_ELEM, vswap=EMPTY_ELEM, wUB=0;
	int pc=(m_unsel=bb).popcn64();
	const int KMINW=maxno-maxac; const int NB_OF_BB_NODES_MINUS_ONE=iop.NB_OF_BB_NODES-1;
	bool is_first_vertex=true;
	
	if(pc==0)							//***empty check tests, remove for optimal performance
		return 0;					

	iop.m_colSets[1].erase_bit(true);	//lazy: size=0
	while(true){ 
		bitarray& bbcol=iop.m_colSets[col].bb;
		for(int i=0; i<iop.NB_OF_BB_NODES; i++){
			bbcol.get_bitboard(i)=m_unsel.get_bitboard(i);
		}
		bbcol.init_scan(bbo::NON_DESTRUCTIVE);
		is_first_vertex=true;
		while(true){
next_v:		v=bbcol.next_bit(nBB,m_unsel);
			if(v==EMPTY_ELEM)
				break;
			int wv=iop.m_lw[v];
//////////////////////////////
//RECOLORING ATTEMPT
			if( ((wUB+wv)>KMINW) && col>=3){
			//if( (wUB>KMINW) && col>=3){
			//if(col>=3 ){
				for(int recol=1; recol<col-1; recol++){
					if(wv>m_lfv[recol] && iop.m_colSets[recol].size!=1)		//weight packing in colors!
						continue;
					int pc_swap=iop.m_colSets[recol].bb.single_disjoint(0, NB_OF_BB_NODES_MINUS_ONE, g->get_neighbors(v), vswap);
					if(pc_swap==1){	//candidate color class found
						for(int j=recol+1; j<col; j++){
							if(g->get_wv(vswap)>m_lfv[j]) continue;			//weight packing in colors!
							if(iop.m_colSets[j].bb.is_disjoint(0, NB_OF_BB_NODES_MINUS_ONE, g->get_neighbors(vswap))){
							
								//erase 'from' color sets
								iop.m_colSets[recol].erase_bit(vswap);
								iop.m_colSets[col].bb.erase_bit(v);
								iop.node_iset_no[vswap]=j;
								iop.node_iset_no[v]=recol;
								
								int wvswap=g->get_wv(vswap);
																
								//update weights for recol
								wUB-=m_lfv[recol];
								if (wv>m_lfv[recol] || iop.m_colSets[recol].size==0){		//v moves to an empty color (w was a singleton in recol)
									wUB+=wv;
									m_lfv[recol]=wv;
								}else if(wvswap==/*>=*/m_lfv[recol]){
									int w1=g->get_wv(iop.m_colSets[recol].bb.msbn64());
									if(wv>w1){
										wUB+=wv;
										m_lfv[recol]=wv;
									}else{
										wUB+=w1;
										m_lfv[recol]=w1;
									}
								}
								
								//update weights for j
								if(wvswap>m_lfv[j]){
									wUB+=(wvswap-m_lfv[j]);
									m_lfv[j]=wvswap;
								}

								//set 'to' color sets
								iop.m_colSets[j].push(vswap);
								iop.m_colSets[recol].push(v);
																		
								if((--pc)==0){
									iop.set_node_state_active(bb);
									if(iop.m_colSets[col].size==0){	//** TODO:optimize
										m_nCol=col-1;
										iop.set_color_nb(m_nCol);										
									}else{
										m_nCol=col;
										iop.set_color_nb(col);										
									}
									return wUB;
								}else goto next_v;
							}
						}
					} else if(pc_swap==0){
						if (wv>m_lfv[recol]){
							wUB+=(wv-m_lfv[recol]);
							m_lfv[recol]=wv;
						}

						//update color sets
						iop.m_colSets[col].bb.erase_bit(v);
						iop.m_colSets[recol].push(v);
						iop.node_iset_no[v]=recol;
					
						if((--pc)==0){
							iop.set_node_state_active(bb);
							if(iop.m_colSets[col].size==0){	//** TODO:optimize
								m_nCol=col-1;
								iop.set_color_nb(m_nCol);
							}else{
								m_nCol=col;
								iop.set_color_nb(col);
							}
							return wUB;
						}else goto next_v;
					}
				}
			}
///////////////////////////////

			//stores color label
			iop.m_colSets[col].size++;						//adds v (already in the bitset)
			iop.node_iset_no[v]=col;

			//updates total weight
			if(is_first_vertex){
				m_lfv[col]=wv;
				wUB+=wv;
				is_first_vertex=false;
			}else if (wv>m_lfv[col]){
				wUB+=(wv-m_lfv[col]);
				m_lfv[col]=wv;
			}
			
			//checks exit condition
			if((--pc)==0){
				m_nCol=col;
				iop.set_color_nb(col);
				iop.set_node_state_active(bb);
				//sort(m_lfv+1, m_lfv+col+1 , greater<int>());

			/*	int res=accumulate(m_lfv+1, m_lfv+col+1, 0, std::plus<int>());
				copy(m_lfv+1,m_lfv+col+1, ostream_iterator<int>(cout," ") ); cout<<endl;
				if(res!=wUB){
					LOG_ERROR("paint::BIZARRE");
				}*/

				return wUB;
			}	

			bitarray& nbor=g->get_neighbors(v);
			for(int i=nBB; i<iop.NB_OF_BB_NODES; i++){
				bbcol.get_bitboard(i)&= ~nbor.get_bitboard(i);
			}
			//iop.m_colSets[col].bb.erase_block(nBB,g->get_neighbors(v));
					
		}
		if(!is_first_vertex){
			col++;
			iop.m_colSets[col].erase_bit(true);		//lazy: size=0
			//iop.m_colSets[col].bb.get_bitboard(iop.NB_OF_BB_NODES)=0;  //*** necessary?
		}
	}
	LOG_ERROR("paint_UB():bizarre coloring");
	return EMPTY_ELEM;									//should not reach here 
}

inline
int CliqueWeighted::paint_UB_RX_minw_satz(int maxac, const bitarray& bb){
////////////////////////
// classical independent set coloring of subgraph bb with recoloring
// extended to the weighted version
//
// ORDERING REQUIRED: vertices sorted by in non_increasing weight (to compute update weights of clauses on recoloring)
// first update:30/10/16
// last update: 31/10/16
//
// PARAMS: maxac-weight of current clique built, including current expanded node, bb-the subgraph to be colored
//
// RETURNS: color-based upper bound for weighted graph or EMPTY_ELEM if error
//
// REMARKS
// 1.Should NOT be used with p-MaxSAT bounds!



	int col=1, nBB=EMPTY_ELEM, v=EMPTY_ELEM, vswap=EMPTY_ELEM, wUB=0;
	int pc=(m_unsel=bb).popcn64();
	const int KMINW=maxno-maxac; const int NB_OF_BB_NODES_MINUS_ONE=iop.NB_OF_BB_NODES-1;
	bool is_first_vertex=true;
	iop.set_node_state_active(bb);
	int lv[3];
	
	//ASSERT
	if(pc==0)							//***empty check tests, remove for optimal performance
		return 0;					

	//iop.node_removed_stack.erase();
	iop.m_colSets[1].erase_bit(true);	//lazy: size=0
	m_forbidden.erase_bit();
	while(true){ 
		bitarray& bbcol=iop.m_colSets[col].bb;
		for(int i=0; i<iop.NB_OF_BB_NODES; i++){
			bbcol.get_bitboard(i)=m_unsel.get_bitboard(i);
		}
		bbcol.init_scan(bbo::NON_DESTRUCTIVE);
		//is_first_vertex=true;
		while(true){
next_v:		v=bbcol.next_bit(nBB,m_unsel);
			if(v==EMPTY_ELEM)
				break;
			int wv=g->get_wv(v);
//////////////////////////////
//RECOLORING ATTEMPT
			if( ((wUB+wv)>KMINW) && col>=3){
			//if( (wUB>KMINW) && col>=3){
			//if(col>=3 ){
				for(int recol=1; recol<col-1; recol++){
					if(m_forbidden.is_bit(recol) || (wv>m_lfv[recol] && iop.m_colSets[recol].size!=1) )		
						continue;
					int pc_swap=iop.m_colSets[recol].bb.single_disjoint(0, NB_OF_BB_NODES_MINUS_ONE, g->get_neighbors(v), vswap);
					/*if(vswap!=EMPTY_ELEM && !iop.bb_node_state_active.is_bit(vswap)){
						LOG_ERROR("VSWAP FOUND IS NON-ACTIVE-"<<vswap);
						continue;
					}*/

					if(pc_swap==1){	//candidate color class found
						for(int j=recol+1; j<col; j++){
							if(m_forbidden.is_bit(j) || g->get_wv(vswap)>m_lfv[j]) continue;			
							if(iop.m_colSets[j].bb.is_disjoint(0, NB_OF_BB_NODES_MINUS_ONE, g->get_neighbors(vswap))){

								//erase 'from' color sets
								iop.m_colSets[recol].erase_bit(vswap);
								iop.m_colSets[col].bb.erase_bit(v);
								iop.node_iset_no[vswap]=j;
								iop.node_iset_no[v]=recol;

								int wvswap=g->get_wv(vswap);

								//update weights for recol
								wUB-=m_lfv[recol];
								if (wv>m_lfv[recol] || iop.m_colSets[recol].size==0){		//v moves to an empty color (w was a singleton in recol)
									wUB+=wv;
									m_lfv[recol]=wv;
								}else if(wvswap==/*>=*/m_lfv[recol]){
									int w1=g->get_wv(iop.m_colSets[recol].bb.msbn64());		//!MUST BE ACTIVE!
									if(wv>w1){
										wUB+=wv;
										m_lfv[recol]=wv;
									}else{
										wUB+=w1;
										m_lfv[recol]=w1;
									}
								}

								//update weights for j
								if(wvswap>m_lfv[j]){
									wUB+=(wvswap-m_lfv[j]);
									m_lfv[j]=wvswap;
								}

								//set 'to' color sets
								iop.m_colSets[j].push(vswap);
								iop.m_colSets[recol].push(v);

								if((--pc)==0){
									if(iop.m_colSets[col].size==0){	//** TODO:optimize
										m_nCol=col-1;
										iop.set_color_nb(m_nCol);										
									}else{
										m_nCol=col;
										iop.set_color_nb(col);										
									}
									return wUB;
								}else goto next_v;
							}else if( /*wv>m_lfv[recol] &&  wv>m_lfv[j] &&*/ iop.m_colSets[j].bb.is_disjoint(g->get_neighbors(v),g->get_neighbors(vswap)) ){ //**INDEXES FOR NON-ADDED NODES
								//LOG_INFO("SIMPLE INFRACHROM CONDITIONS MET");
								m_forbidden.set_bit(recol);
								m_forbidden.set_bit(j);
														
								
								lv[0]=m_lfv[recol];
								lv[1]=m_lfv[j]; 
								lv[2]=wv;
								sort(lv, lv+3);		//non_decreasing weights-lv[0] minimum

								wUB-=(m_lfv[recol]+m_lfv[j]);
								wUB+=(lv[1]+lv[2]);

								//attempt at using infra-chrom afterwards
								//iop.node_removed_stack.push(v);
								//wUB+=wv;
																
								////simplification of clauses
								//if(lv[0]==wv ){
								//	//simplify original clauses: changes original weights so must be reset in termination
								//	

								//	if( m_lfv[recol]>wv ){
								//		iop.simplify_clause_during_recoloring(recol, wv);
								//												
								//	/*	iop.check_consistency_clause(recol,false);
								//		iop.print_clause(recol);*/
								//	}

								//	if(m_lfv[j]>wv){
								//		iop.simplify_clause_during_recoloring(j, wv);
								//	//	iop.check_consistency_clause(j, false);
								//	//	iop.print_clause(j);
								//	}
								//	iop.bb_node_state_active.erase_bit(v);
								//}else if(lv[0]==m_lfv[recol]){
								//	//LOG_INFO("SWAPPING CLAUSES");
								//	//***remove literals of recol
								//	iop.swap_clause_during_recoloring(recol, v);	
								//	//wUB+=(wv-m_lfv[recol]);
								//}else if(lv[0]==m_lfv[j]){
								//	//LOG_INFO("SWAPPING CLAUSES");
								//	iop.swap_clause_during_recoloring(j, v);
								//	//wUB+=(wv-m_lfv[j]);
								//}

								//update color DB: STORE THE VERTEX AS UNIT FOR P-MAXSAT BOUND
								iop.m_colSets[col].bb.erase_bit(v);
								//iop.bb_node_state_active.erase_bit(v);

								if((--pc)==0){
									if(iop.m_colSets[col].size==0){		//** TODO:optimize
										m_nCol=col-1;
										iop.set_color_nb(m_nCol);										
									}else{
										m_nCol=col;
										iop.set_color_nb(col);										
									}
									return wUB;
								}else goto next_v;
							}

						}
					} else if(pc_swap==0){
						if (wv>m_lfv[recol]){
							wUB+=(wv-m_lfv[recol]);
							m_lfv[recol]=wv;
						}

						//update color sets
						iop.m_colSets[col].bb.erase_bit(v);
						iop.m_colSets[recol].push(v);
						iop.node_iset_no[v]=recol;
					
						if((--pc)==0){
							if(iop.m_colSets[col].size==0){	//** TODO:optimize
								m_nCol=col-1;
								iop.set_color_nb(m_nCol);
							}else{
								m_nCol=col;
								iop.set_color_nb(col);
							}
							return wUB;
						}else goto next_v;
					}
				}
			}
///////////////////////////////

			//stores color label
			iop.m_colSets[col].size++;						//adds v (already in the bitset)
			iop.node_iset_no[v]=col;

			//updates total weight
			if(is_first_vertex){
				m_lfv[col]=wv;
				wUB+=wv;
				is_first_vertex=false;
			}else if (wv>m_lfv[col]){
				wUB+=(wv-m_lfv[col]);
				m_lfv[col]=wv;
			}
			
			//checks exit condition
			if((--pc)==0){
				m_nCol=col;
				iop.set_color_nb(col);
				return wUB;
			}	

			bitarray& nbor=g->get_neighbors(v);
			for(int i=nBB; i<iop.NB_OF_BB_NODES; i++){
				bbcol.get_bitboard(i)&= ~nbor.get_bitboard(i);
			}
			//iop.m_colSets[col].bb.erase_block(nBB,g->get_neighbors(v));
					
		}
			
		if(!is_first_vertex){
			//iop.print_db(true, true);
			col++;
			iop.m_colSets[col].erase_bit();		//lazy: size=0
			is_first_vertex=true;
			//iop.m_colSets[col].bb.get_bitboard(iop.NB_OF_BB_NODES)=0;  //*** necessary?
		}
	}
	LOG_ERROR("paint_UB():bizarre coloring");
	return EMPTY_ELEM;									//should not reach here 
}

inline
int CliqueWeighted::paint_UB_X_minw_satz(int maxac, const bitarray& bb){
////////////////////////
// classical independent set coloring of subgraph bb with recoloring
// extended to the weighted version
//
// ORDERIN REQUIRED: vertices sorted by in non_increasing weight
// first update:30/10/16
// last update: 30/10/16
//
// PARAMS: maxac-weight of current clique built, including current expanded node, bb-the subgraph to be colored
//
// RETURNS: color-based upper bound for weighted graph or EMPTY_ELEM if error
//

	int col=1, nBB=EMPTY_ELEM, v=EMPTY_ELEM, vswap=EMPTY_ELEM, wUB=0;
	int pc=(m_unsel=bb).popcn64();
	const int KMINW=maxno-maxac; const int NB_OF_BB_NODES_MINUS_ONE=iop.NB_OF_BB_NODES-1;
	bool is_first_vertex=true;
	iop.set_node_state_active(bb);
	int lv[3];
	
	//ASSERT
	if(pc==0)							//***empty check tests, remove for optimal performance
		return 0;					

	iop.node_removed_stack.erase();
	iop.m_colSets[1].erase_bit(true);	//lazy: size=0
	m_forbidden.erase_bit();
	while(true){ 
		bitarray& bbcol=iop.m_colSets[col].bb;
		for(int i=0; i<iop.NB_OF_BB_NODES; i++){
			bbcol.get_bitboard(i)=m_unsel.get_bitboard(i);
		}
		bbcol.init_scan(bbo::NON_DESTRUCTIVE);
		is_first_vertex=true;
		while(true){
next_v:		v=bbcol.next_bit(nBB,m_unsel);
			if(v==EMPTY_ELEM)
				break;
			int wv=g->get_wv(v);				
//////////////////////////////
//RECOLORING ATTEMPT
			if( ((wUB+wv)>KMINW) && col>=3){
			//if( (wUB>KMINW) && col>=3){
			//if(col>=3 ){
				for(int recol=1; recol<col-1; recol++){
					if(m_forbidden.is_bit(recol)  || iop.m_lw[v]>m_lfv[recol]  ) continue;	//weigth packing!
					int pc_swap=iop.m_colSets[recol].bb.single_disjoint(0, NB_OF_BB_NODES_MINUS_ONE, g->get_neighbors(v), vswap);
					if(vswap!=EMPTY_ELEM && !iop.bb_node_state_active.is_bit(vswap)){
						LOG_ERROR("VSWAP FOUND IS NON-ACTIVE-"<<vswap);
						continue;
					}
					if(pc_swap==1){	//candidate color class found
						for(int j=recol+1; j<col; j++){
							if( m_forbidden.is_bit(j) || iop.m_lw[vswap]>m_lfv[j] ) continue;			//weigth packing!
							if(iop.m_colSets[j].bb.is_disjoint(g->get_neighbors(v),g->get_neighbors(vswap)) ){ //**INDEXES FOR NON-ADDED NODES
								//LOG_INFO("SIMPLE INFRACHROM CUT");
								m_forbidden.set_bit(recol);
								m_forbidden.set_bit(j);
								
								iop.node_removed_stack.push(v);
								wUB+=wv;

								//lv[0]=m_lfv[recol];
								//lv[1]=m_lfv[j]; 
								//lv[2]=iop.m_lw[v];
								//sort(lv, lv+3);		//non_decreasing weights-lv[0] minimum

								//wUB-=(m_lfv[recol]+m_lfv[j]);		
								//wUB+=(lv[1]+lv[2]);
								
								////update clauses
								//if(lv[0]==iop.m_lw[v]){
								//								
								//	if( m_lfv[recol]>iop.m_lw[v] ){
								//		iop.simplify_clause_during_recoloring(recol, iop.m_lw[v]);
								//		/*iop.check_consistency_clause(recol,false);
								//		iop.print_clause(recol);*/
								//	}

								//	if(m_lfv[j]>iop.m_lw[v]){
								//		iop.simplify_clause_during_recoloring(j, iop.m_lw[v]);
								//	//	iop.check_consistency_clause(j, false);
								//	//	iop.print_clause(j);
								//	}
								//	iop.bb_node_state_active.erase_bit(v);
								//}else if(lv[0]==m_lfv[recol]){
								//	//LOG_INFO("SWAPPING CLAUSES");
								//	//***remove literals of recol
								//	iop.swap_clause_during_recoloring(recol, v);	
								//	wUB+=(iop.m_lw[v]-m_lfv[recol]);
								//}else if(lv[0]==m_lfv[j]){
								//	//LOG_INFO("SWAPPING CLAUSES");
								//	iop.swap_clause_during_recoloring(j, v);
								//	wUB+=(iop.m_lw[v]-m_lfv[j]);
								//}

								//update color DB
								iop.m_colSets[col].bb.erase_bit(v);
								//iop.bb_node_state_active.erase_bit(v);

								if((--pc)==0){
									if(iop.m_colSets[col].size==0){		//** TODO:optimize
										m_nCol=col-1;
										iop.set_color_nb(m_nCol);										
									}else{
										m_nCol=col;
										iop.set_color_nb(col);										
									}
									return wUB;
								}else goto next_v;
							}
						}
					} else if(pc_swap==0){
						if (wv>m_lfv[recol]){
							wUB+=(wv-m_lfv[recol]);
							m_lfv[recol]=wv;
						}

						//update color sets
						iop.m_colSets[col].bb.erase_bit(v);
						iop.m_colSets[recol].push(v);
						iop.node_iset_no[v]=recol;
					
						if((--pc)==0){
							if(iop.m_colSets[col].size==0){	//** TODO:optimize
								m_nCol=col-1;
								iop.set_color_nb(m_nCol);
							}else{
								m_nCol=col;
								iop.set_color_nb(col);
							}
							return wUB;
						}else goto next_v;
					}
				}
			}
///////////////////////////////

			//stores color label
			iop.m_colSets[col].size++;						//adds v (already in the bitset)
			iop.node_iset_no[v]=col;

			//updates total weight
			if(is_first_vertex){
				m_lfv[col]=wv;
				wUB+=m_lfv[col];
				is_first_vertex=false;
			}else if (wv>m_lfv[col]){
				wUB+=(wv-m_lfv[col]);
				m_lfv[col]=wv;
			}
			
			//checks exit condition
			if((--pc)==0){
				m_nCol=col;
				iop.set_color_nb(col);
				return wUB;
			}	

			bitarray& nbor=g->get_neighbors(v);
			for(int i=nBB; i<iop.NB_OF_BB_NODES; i++){
				bbcol.get_bitboard(i)&= ~nbor.get_bitboard(i);
			}
			//iop.m_colSets[col].bb.erase_block(nBB,g->get_neighbors(v));
		}
			
		if(!is_first_vertex){
			//iop.print_db(true, true);
			col++;
			iop.m_colSets[col].erase_bit(true);		//lazy: size=0
			is_first_vertex=true;
			//iop.m_colSets[col].bb.get_bitboard(iop.NB_OF_BB_NODES)=0;  //*** necessary?
		}
	}
	LOG_ERROR("paint_UB():bizarre coloring");
	return EMPTY_ELEM;									//should not reach here 
}

inline
int CliqueWeighted::paint_UB_R(int maxac, const bitarray& bb){
////////////////////////
// classical independent set coloring of subgraph bb with recoloring
// extended to the weighted version
// first update: 10/10/16
// last update: 10/10/16
//
// PARAMS: maxac-weight of current clique built, including current expanded node, bb-the subgraph to be colored
//
// RETURNS: color-based upper bound for weighted graph or EMPTY_ELEM if error
//

	int col=1, nBB=EMPTY_ELEM, v=EMPTY_ELEM, vswap=EMPTY_ELEM;
	int pc=(m_unsel=bb).popcn64();
	const int KMINW=maxno-maxac;
	
	if(pc==0)							//***empty check tests, remove for optimal performance
		return 0;					

	int wUB=0;
	
	bool is_first_vertex;
	while(true){ 
		m_colsets[col]=m_unsel;
		m_colsets[col].init_scan(bbo::NON_DESTRUCTIVE);
		is_first_vertex=true;
		while(true){
next_v:		v=m_colsets[col].next_bit(nBB,m_unsel);
			if(v==EMPTY_ELEM)
				break;
			int wv=g->get_wv(v);
//////////////////////////////
//RECOLORING ATTEMPT
			if( ((wUB+wv)>KMINW) && col>=3){
			//if( (wUB>KMINW) && col>=3){
			//if(col>=3 ){
				for(int recol=1; recol<col-1; recol++){
					/*if(v<m_lfv[recol]) 
						continue;*/
					int pc_swap=m_colsets[recol].single_disjoint(g->get_neighbors(v), vswap);
					if(pc_swap==1){	//candidate color class found
						//for(int j=recol+1; j<kmin; j++){
						for(int j=recol+1; j<col; j++){
							//if(vswap<m_lfv[j]) continue;
							if(m_colsets[j].is_disjoint(g->get_neighbors(vswap))){
							
								//erase 'from' color sets
								m_colsets[recol].erase_bit(vswap);
								m_colsets[col].erase_bit(v);
								
								int wvswap=g->get_wv(vswap);
								if(wvswap==/*>=*/m_lfv[recol]){
									if(wv>wvswap){
										wUB+=(wv-wvswap);
										m_lfv[recol]=wv;
									}else{  /* I do not know the best UB so I leave the one I have */
										; 
									}
								}else if (wv>m_lfv[recol]){
									wUB+=(wv-m_lfv[recol]);
									m_lfv[recol]=wv;
								}
								
								//update weights according to j
								if(wvswap>m_lfv[j]){
									wUB+=(wvswap-m_lfv[j]);
									m_lfv[j]=wvswap;
								}

								//set 'to' color sets
								m_colsets[j].set_bit(vswap);
								m_colsets[recol].set_bit(v);
										
								if((--pc)==0){
									return wUB;
								}else goto next_v;
							}
						}
					} else if(pc_swap==0){
						//LOG_ERROR("PC_SWAP=0: VSWAP");
						/*int creco=m_colsets[recol].lsbn64();
						if(creco==EMPTY_ELEM){
							wUB+=g->get_wv(v);
						}else if(v<creco) 
							wUB+=(g->get_wv(v)-g->get_wv(creco));*/

						/*if(v<m_lfv[recol]){
							LOG_ERROR("PCSWAP=0: bizarre color set");
						}*/
						/*wUB+=(g->get_wv(v)-g->get_wv(m_lfv[recol]));
						m_lfv[recol]=v;*/

						if (wv>m_lfv[recol]){
							wUB+=(wv-m_lfv[recol]);
							m_lfv[recol]=wv;
						}

						//update color sets
						m_colsets[col].erase_bit(v);
						m_colsets[recol].set_bit(v);

						if((--pc)==0){
							return wUB;
						}else goto next_v;
					}
				}
			}
///////////////////////////////

			//updates total weight
			if(is_first_vertex){
				m_lfv[col]=wv;
				wUB+=wv;
				is_first_vertex=false;
			}else if(wv>m_lfv[col]){
				wUB+=(wv-m_lfv[col]);
				m_lfv[col]=wv;				
			}
						
			//checks exit condition
			if((--pc)==0){
					return wUB;
			}						
			m_colsets[col].erase_block(nBB,g->get_neighbors(v));		
		}

		//new color if current col is not empty
		if(!is_first_vertex)
						col++;
	}
	LOG_ERROR("paint_UB():bizarre coloring");
	return EMPTY_ELEM;									//should not reach here 
}
inline
int CliqueWeighted::paint_UB_X_ow(int maxac, const bitarray& bb){
////////////////////////
// classical independent set coloring of subgraph bb with simple infrachrom bounds
// extended to the weighted version
// first update: 17/10/16
// last update: 17/10/16
//
// PARAMS: maxac-weight of current clique built, including current expanded node, bb-the subgraph to be colored
//
// RETURNS: upper bound for weighted graph or EMPTY_ELEM if error


	int col=1, nBB=EMPTY_ELEM, v=EMPTY_ELEM, vswap=EMPTY_ELEM;
	int pc=(m_unsel=bb).popcn64();
	const int KMINW=maxno-maxac;
	int lv[3];
	
	if(pc==0)							//***empty check tests, remove for optimal performance
		return 0;					

	int wUB=0;
	bool is_first_vertex;
	m_forbidden.erase_bit();
	while(true){ 
		m_colsets[col]=m_unsel;
		m_colsets[col].init_scan(bbo::NON_DESTRUCTIVE);
		is_first_vertex=true;
		while(true){
next_v:		v=m_colsets[col].next_bit(nBB,m_unsel);
			if(v==EMPTY_ELEM)
				break;
			int wv=g->get_wv(v);
//////////////////////////////
//RECOLORING ATTEMPT
			if( ((wUB+wv)>KMINW) && col>=3 ){
			//if( (wUB>KMINW) && col>=3){
			//if(col>=3 ){
				for(int recol=1; recol<col-1; recol++){
					if(m_forbidden.is_bit(recol)) continue;
					int pc_swap=m_colsets[recol].single_disjoint(g->get_neighbors(v), vswap);
					if(pc_swap==1){	//candidate color class found
						for(int j=recol+1; j<col; j++){
							if(m_forbidden.is_bit(j)) continue;
							
							if(m_colsets[j].is_disjoint(g->get_neighbors(vswap))){
							
								//erase 'from' color sets
								m_colsets[recol].erase_bit(vswap);
								m_colsets[col].erase_bit(v);

								int wvswap=g->get_wv(vswap);

								//update weights according to recol
								if(wvswap>=m_lfv[recol]){
									int vfrec=m_colsets[recol].lsbn64();
									if(vfrec==EMPTY_ELEM || v<vfrec){
										m_lfv[recol]=wv;
										wUB+=(wv-wvswap);
									}else {
										int wvfrec=g->get_wv(vfrec);
										m_lfv[recol]=wvfrec;
										wUB+=(wvfrec-wvswap);
									}
								}else if (wv>m_lfv[recol]){
									wUB+=(wv-m_lfv[recol]);
									m_lfv[recol]=wv;
								}

								//update weights according to j
								if(wvswap>m_lfv[j]){
									wUB+=(wvswap-m_lfv[j]);
									m_lfv[j]=wvswap;
								}

								//set 'to' color sets
								m_colsets[j].set_bit(vswap);
								m_colsets[recol].set_bit(v);
															
								if((--pc)==0){
									return wUB;
								}else goto next_v;
							}else if( m_colsets[j].is_disjoint(g->get_neighbors(v),g->get_neighbors(vswap)) ){
								//infrachrom attempt
								//LOG_ERROR("SIMPLE INFRACHROM CUT");
								m_forbidden.set_bit(recol);
								m_forbidden.set_bit(j);
								lv[0]=m_lfv[recol];
								lv[1]=m_lfv[j];
								lv[2]=wv;
								sort(lv, lv+3);
								wUB-=(m_lfv[recol]+m_lfv[j]);
								wUB+=(lv[1]+lv[2]);
							
								//if(lv[0]==m_lfv[j]){		/* min weight*/
								//	wUB+=(wv-m_lfv[j]);
								//}else if(lv[1]==m_lfv[recol]){
								//	wUB+=(wv-m_lfv[recol]);
								//}

								if((--pc)==0){
									return wUB;
								}else goto next_v;
							}
						}
					} else if(pc_swap==0){
						if(wv>m_lfv[recol]){
							wUB+=(wv-m_lfv[recol]);
							m_lfv[recol]=wv;
						}

						//update color sets
						m_colsets[col].erase_bit(v);
						m_colsets[recol].set_bit(v);

						if((--pc)==0){
							return wUB;
						}else goto next_v;
					}
				}
			}
///////////////////////////////

			//updates total weight
			if(is_first_vertex){
				m_lfv[col]=wv;
				wUB+=wv;
				is_first_vertex=false;
			}
			//checks exit condition
			if((--pc)==0){
				return wUB;
			}						
			m_colsets[col].erase_block(nBB,g->get_neighbors(v));		
		}
		if(!is_first_vertex)
						col++;
	}
	LOG_ERROR("paint_UB():bizarre coloring");
	return EMPTY_ELEM;									//should not reach here 
}

inline
int CliqueWeighted::paint_UB_X_ow_satz(int maxac, const bitarray& bb){
////////////////////////
// classical independent set coloring of subgraph bb with simple infrachrom bounds
// extended to the weighted version
// first update: 17/10/16
// last update: 17/10/16
//
// PARAMS: maxac-weight of current clique built, including current expanded node, bb-the subgraph to be colored
//
// RETURNS: upper bound for weighted graph or EMPTY_ELEM if error


	int col=1, nBB=EMPTY_ELEM, v=EMPTY_ELEM, vswap=EMPTY_ELEM;
	int pc=(m_unsel=bb).popcn64();
	const int KMINW=maxno-maxac;
	int lv[3];
	
	if(pc==0)							//***empty check tests, remove for optimal performance
		return 0;					

	int wUB=0;
	bool is_first_vertex;
	m_forbidden.erase_bit();
	while(true){ 
		m_colsets[col]=m_unsel;
		m_colsets[col].init_scan(bbo::NON_DESTRUCTIVE);
		is_first_vertex=true;
		while(true){
next_v:		v=m_colsets[col].next_bit(nBB,m_unsel);
			if(v==EMPTY_ELEM)
				break;
			int wv=g->get_wv(v);
//////////////////////////////
//RECOLORING ATTEMPT
			if( ((wUB+wv)>KMINW) && col>=3 ){
			//if( (wUB>KMINW) && col>=3){
			//if(col>=3 ){
				for(int recol=1; recol<col-1; recol++){
					if(m_forbidden.is_bit(recol)) continue;
					int pc_swap=m_colsets[recol].single_disjoint(g->get_neighbors(v), vswap);
					if(pc_swap==1){	//candidate color class found
						for(int j=recol+1; j<col; j++){
							if(m_forbidden.is_bit(j)) continue;
							
							if(m_colsets[j].is_disjoint(g->get_neighbors(vswap))){
							
								//erase 'from' color sets
								m_colsets[recol].erase_bit(vswap);
								m_colsets[col].erase_bit(v);

								int wvswap=g->get_wv(vswap);

								//update weights according to recol
								if(wvswap>=m_lfv[recol]){
									int vfrec=m_colsets[recol].lsbn64();
									if(vfrec==EMPTY_ELEM || v<vfrec){
										m_lfv[recol]=wv;
										wUB+=(wv-wvswap);
									}else {
										int wvfrec=g->get_wv(vfrec);
										m_lfv[recol]=wvfrec;
										wUB+=(wvfrec-wvswap);
									}
								}else if (wv>m_lfv[recol]){
									wUB+=(wv-m_lfv[recol]);
									m_lfv[recol]=wv;
								}

								//update weights according to j
								if(wvswap>m_lfv[j]){
									wUB+=(wvswap-m_lfv[j]);
									m_lfv[j]=wvswap;
								}

								//set 'to' color sets
								m_colsets[j].set_bit(vswap);
								m_colsets[recol].set_bit(v);
															
								if((--pc)==0){
									return wUB;
								}else goto next_v;
							}else if( m_colsets[j].is_disjoint(g->get_neighbors(v),g->get_neighbors(vswap)) ){
								//infrachrom attempt
								//LOG_ERROR("SIMPLE INFRACHROM CUT");
								m_forbidden.set_bit(recol);
								m_forbidden.set_bit(j);
								lv[0]=m_lfv[recol]; lv[1]=m_lfv[j]; lv[2]=wv;
								sort(lv, lv+3);
								wUB-=(m_lfv[recol]+m_lfv[j]);
								wUB+=(lv[1]+lv[2]);
							
								//if(lv[0]==m_lfv[j]){		/* min weight*/
								//	wUB+=(wv-m_lfv[j]);
								//}else if(lv[1]==m_lfv[recol]){
								//	wUB+=(wv-m_lfv[recol]);
								//}

								if((--pc)==0){
									return wUB;
								}else goto next_v;
							}
						}
					} else if(pc_swap==0){
						if(wv>m_lfv[recol]){
							wUB+=(wv-m_lfv[recol]);
							m_lfv[recol]=wv;
						}

						//update color sets
						m_colsets[col].erase_bit(v);
						m_colsets[recol].set_bit(v);

						if((--pc)==0){
							return wUB;
						}else goto next_v;
					}
				}
			}
///////////////////////////////

			//updates total weight
			if(is_first_vertex){
				m_lfv[col]=wv;
				wUB+=wv;
				is_first_vertex=false;
			}
			//checks exit condition
			if((--pc)==0){
				return wUB;
			}						
			m_colsets[col].erase_block(nBB,g->get_neighbors(v));		
		}
		if(!is_first_vertex)
						col++;
	}
	LOG_ERROR("paint_UB():bizarre coloring");
	return EMPTY_ELEM;									//should not reach here 
}


inline
int CliqueWeighted::paint_UB_X(int maxac, const bitarray& bb){
////////////////////////
// classical independent set coloring of subgraph bb with simple infrachrom bounds
// extended to the weighted version
// first update: 17/10/16
// last update: 17/10/16
//
// PARAMS: maxac-weight of current clique built, including current expanded node, bb-the subgraph to be colored
//
// RETURNS: upper bound for weighted graph or EMPTY_ELEM if error


	int col=1, nBB=EMPTY_ELEM, v=EMPTY_ELEM, vswap=EMPTY_ELEM;
	int pc=(m_unsel=bb).popcn64();
	const int KMINW=maxno-maxac;
	int lv[3];
	
	if(pc==0)							//***empty check tests, remove for optimal performance
		return 0;					

	int wUB=0;
	bool is_first_vertex;
	m_forbidden.erase_bit();
	while(true){ 
		m_colsets[col]=m_unsel;
		m_colsets[col].init_scan(bbo::NON_DESTRUCTIVE);
		is_first_vertex=true;
		while(true){
next_v:		v=m_colsets[col].next_bit(nBB,m_unsel);
			if(v==EMPTY_ELEM)
				break;
			int wv=g->get_wv(v);
//////////////////////////////
//RECOLORING ATTEMPT
			if( col>=3 && ((wUB+wv)>KMINW) ){
			//if( (wUB>KMINW) && col>=3){
			//if(col>=3 ){
				for(int recol=1; recol<col-1; recol++){
					if(m_forbidden.is_bit(recol)) continue;
					int pc_swap=m_colsets[recol].single_disjoint(g->get_neighbors(v), vswap);
					if(pc_swap==1){	//candidate color class found
						for(int j=recol+1; j<col; j++){
							if(m_forbidden.is_bit(j)) continue;
							
							if(m_colsets[j].is_disjoint(g->get_neighbors(vswap))){
							
								//erase 'from' color sets
								m_colsets[recol].erase_bit(vswap);
								m_colsets[col].erase_bit(v);


								int wvswap=g->get_wv(vswap);
								if(wvswap==/*>=*/m_lfv[recol]){
									if(wv>wvswap){
										wUB+=(wv-wvswap);
										m_lfv[recol]=wv;
									}else{  /* I do not know the best UB so I leave the one I have */
										; 
									}
								}else if (wv>m_lfv[recol]){
									wUB+=(wv-m_lfv[recol]);
									m_lfv[recol]=wv;
								}
								
								//update weights according to j
								if(wvswap>m_lfv[j]){
									wUB+=(wvswap-m_lfv[j]);
									m_lfv[j]=wvswap;
								}
																				

								//set 'to' color sets
								m_colsets[j].set_bit(vswap);
								m_colsets[recol].set_bit(v);
															
								if((--pc)==0){
									return wUB;
								}else goto next_v;
							}else if( m_colsets[j].is_disjoint(g->get_neighbors(v),g->get_neighbors(vswap)) ){
								//infrachrom attempt
								//LOG_ERROR("SIMPLE INFRACHROM CUT");
								
								////working
								//m_forbidden.set_bit(recol);
								//m_forbidden.set_bit(j);


								lv[0]=m_lfv[recol];
								lv[1]=m_lfv[j];
								lv[2]=wv;
								sort(lv, lv+3);  /*increasing order of weights*/
								

								wUB-=(m_lfv[j]+m_lfv[recol]);		//*** optimize
								wUB+=(lv[1]+lv[2]);

								//conditonal update of forbidden clauses
								if(lv[0]>=m_lfv[recol]){
									m_forbidden.set_bit(recol);
								}else m_lfv[recol]-=lv[0];

								if(lv[0]>=m_lfv[j]){
									m_forbidden.set_bit(j);
								}else m_lfv[j]-=lv[0];


								
							
								if((--pc)==0){
									return wUB;
								}else goto next_v;
							}
						}
					} else if(pc_swap==0){
						
						if (wv>m_lfv[recol]){
							wUB+=(wv-m_lfv[recol]);
							m_lfv[recol]=wv;
						}
						
						//update color sets
						m_colsets[col].erase_bit(v);
						m_colsets[recol].set_bit(v);

						if((--pc)==0){
							return wUB;
						}else goto next_v;
					}
				}
			}
///////////////////////////////

			//updates total weight
			if(is_first_vertex){
				m_lfv[col]=wv;
				wUB+=wv;
				is_first_vertex=false;
			}else if(wv>m_lfv[col]){
				wUB+=(wv-m_lfv[col]);
				m_lfv[col]=wv;				
			}

			//checks exit condition
			if((--pc)==0){
				return wUB;
			}						
			m_colsets[col].erase_block(nBB,g->get_neighbors(v));		
		}
		if(!is_first_vertex)
						col++;
	}
	LOG_ERROR("paint_UB():bizarre coloring");
	return EMPTY_ELEM;									//should not reach here 
}
inline
int CliqueWeighted::solve_first_nodes_ow(int depth, int maxac, nodelist_t& l_v){
/////////////////
// INPUT: The full list of nodes that make the current subgraph
// RETURNS: number of solved nodes 
//
// COMMENTS: 
// 1.Labels the solved nodes as best possible 
// 2.kmin cannot be equal to 0 (else inc_maxsatz could be called without being initialized when cmax==1)
 
	int v, wv, nb_solved_v=0, cmax=1;
	const int KMIN=maxno-maxac;
	bool is_existing_col;
		
	LISTA_BB(depth).erase_bit();
	m_colsets[1].erase_bit();
	int wUB=g->get_wv(l_v.nodos[0]);				//first vertex, cannot be empty

	for(int j=0; j<l_v.index; j++){
		//estrategias
		v=l_v.nodos[j];
		wv=g->get_wv(v);
									
		//inner loop: search for SEQ color for v
		is_existing_col=false;
		for(int col=1; col<=cmax; col++){
			is_existing_col=g->get_neighbors(v).is_disjoint(0, WDIV(v), m_colsets[col]);			//**TODO optimize? note: the reasoning does not include added nodes
			
			//color found for vertex
			if(is_existing_col){
				m_colsets[col].set_bit(v);
				break;
			}
		}

		//color not found: new color
		if(!is_existing_col){
			m_colsets[++cmax].erase_bit();				
			m_colsets[cmax].set_bit(v);
			wUB+=wv;										//first color
		}	

		//exit condition: impossible to cut
		if(wUB>KMIN) 
				break;		
		
		m_lcol[HEAD_COLOR][v]=estimate_ub_first_nodes(min<int>(m_lcol[HEAD_COLOR][v], wUB), v, j, l_v);
		nb_solved_v++;
		LISTA_BB(depth).set_bit(v);
	}

	//reset context operations
	return nb_solved_v;
}

inline
int CliqueWeighted::solve_first_nodes(int depth, int maxac, nodelist_t& l_v){
/////////////////
// INPUT: The full list of nodes that make the current subgraph
// RETURNS: number of solved nodes 
//
// COMMENTS: 
// 1.Labels the solved nodes as best possible 
// 2.kmin cannot be equal to 0 (else inc_maxsatz could be called without being initialized when cmax==1)
 
	int v, wv, nb_solved_v=0, cmax=1;
	const int KMIN=maxno-maxac;
	bool is_existing_col;
		
	LISTA_BB(depth).erase_bit();
	m_colsets[1].erase_bit();
	int wUB=g->get_wv(l_v.nodos[0]);				//first vertex, cannot be empty
	m_lfv[1]=wUB;

	for(int j=0; j<l_v.index; j++){
		//estrategias
		v=l_v.nodos[j];
		wv=iop.m_lw[v];
									
		//inner loop: search for SEQ color for v
		is_existing_col=false;
		for(int col=1; col<=cmax; col++){
			is_existing_col=g->get_neighbors(v).is_disjoint(0, WDIV(v), m_colsets[col]);			//**TODO optimize? note: the reasoning does not include added nodes
			
			//color found for vertex
			if(is_existing_col){
				m_colsets[col].set_bit(v);
				if(m_lfv[col]<wv){
					wUB+=(wv-m_lfv[col]);	
					m_lfv[col]=wv;
				}
				break;
			}
		}

		//color not found: new color
		if(!is_existing_col){
			m_colsets[++cmax].erase_bit();				
			m_colsets[cmax].set_bit(v);
			wUB+=wv;										
			m_lfv[cmax]=wv;
		}	

		//lower bounds as much as possible always
		m_lcol[HEAD_COLOR][v]=estimate_ub_first_nodes(min<int>(m_lcol[HEAD_COLOR][v], wUB), v, j, l_v);

		//exit condition: impossible to cut
		if( (wUB>KMIN) && (m_lcol[HEAD_COLOR][v]>KMIN)) 
				break;		
		
		
		nb_solved_v++;
		LISTA_BB(depth).set_bit(v);
	}

	//reset context operations
	return nb_solved_v;
}


inline
int CliqueWeighted::estimate_ub_first_nodes(int old_ub /* inherited from child node*/, int node, int pos_node, nodelist_t& l_v){
////////////////////
//
// PARAMS: old_ub: the father ub, node: the vertex to bound, pos_node: the position in the stack, 
//				   l_v: the stack
	int max=0;
	//int max_iset_nb=0;									//computes ub concerned with current colorng of solve_first_nodes
	int solved_node=EMPTY_ELEM;

	//loop over previous solved nodes
	for(int i=0; i<pos_node; i++) {
		solved_node=l_v.nodos[i];
		/*if (max_iset_nb<iop.node_iset_no[solved_node])		
			max_iset_nb=iop.node_iset_no[solved_node];*/
		if (g->is_edge(node, solved_node)) {
			if (max<m_lcol[HEAD_COLOR][solved_node])
					max=m_lcol[HEAD_COLOR][solved_node];
		}
	}

	max+=g->get_wv(node);				// add node itself into the clique
	if(old_ub<max)
			max=old_ub;
		
	/*if (max_iset_nb<iop.node_iset_no[node])
			max_iset_nb=iop.node_iset_no[node];

	if (max_iset_nb<max-1){  
		LOG_ERROR("BOUND CHANGED IN SOLVE FIRST NODES:"<<max_iset_nb+1<<":"<<max);
		max=max_iset_nb+1;
	}*/

	return max;
}

inline
int CliqueWeighted::initial_bounds(int& lb, int& ub,  KCore<ugraph>* pkcore){
////////////////////////////////
// Determines simple initial lower and upper bounds for the MWCP 
//
// RETURNS value>0 if solution is found else 0
//
// 1.UB: sum of all the weights
// 2.LB: sum of weights of an initial greedy clique (vertices should already be ordered)

	vint vset;
	int ub1=std::accumulate (g->get_weights().begin(), g->get_weights().end(), 0, plus<int>());
	
	//find  clique greedily
	int lb1=0;
	bitarray bb(g->number_of_vertices());
	bb.set_bit(0, g->number_of_vertices()-1);		
	bb.init_scan(bbo::DESTRUCTIVE);
	while(true){
		int v=bb.next_bit_del();
		if (v==EMPTY_ELEM) break;
		lb1+=g->get_wv(v);
		bb&=g->get_neighbors(v);	
		vset.push_back(v);
	}

	lb=max<int>(lb,lb1);	
	ub=min<int>(ub,ub1);
	int sol=CLQParam::is_trivial_sol(lb, ub);
	res.add_solution(vset);							//just in case it cannot be improved!

return sol;
}


#endif 