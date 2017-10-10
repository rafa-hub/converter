////////////////////////////////
// clique_infra_plus.h: interface for the CliqueInfraPlus class which contains different implementations of an enhanced
//						infra-chromatic filter for bit-parallel, non-iterative MCP algorithms 
//
// initial date:15/12/16
// last update: 12/12/16
// author: pablo san segundo


#ifndef  __CLIQUE_INFRA_PLUS_H__
#define  __CLIQUE_INFRA_PLUS_H__

#include "clique.h"
#include "../init_color_ub.h"
#include "../amts/amts_exec.h"
#include "bitscan/bbalg.h"
#include "infra_tools.h"
#include "infra_tools_plus.h"
#include "../common/common_macros.h"
#include "../setup.h"

using namespace com;												//for common types (here bb_t)

class CliqueInfraPlus: public Clique<ugraph>{
////////////////////////
// data structure for infra-chrom operations
	InfraOpPlus<ugraph,bitarray> iop;
		
public:
	CliqueInfraPlus(ugraph* g, param_t p)			:Clique<ugraph>(g, p)  {};
	CliqueInfraPlus(param_t p)						:Clique<ugraph>(p) {};
	virtual ~CliqueInfraPlus(){}
	void set_param(param_t p)						{Clique<ugraph>::clear_all(); CLQParam::set_param(p);}	
	
	virtual int init_color_sets();	
	virtual void clear_color_sets();

////////////
//coloring
	void paint_sel					(int depth);									//basic painting routine with pmaxsat branching filter
	void paint_sel_R				(int depth);									
		
	void paint_sel_enhanced			(int depth);									//filters all possible cand. nodes (2/7/17)
		
	//drivers
	void expand_sel			(int maxac, bitarray& l_bb , nodelist_t& l_v);	//selective framework  (currently main variant)
	void expand_sel_R		(int maxac, bitarray& l_bb , nodelist_t& l_v);	//selective framework  (currently main variant)
		
	void expand_sel_enhanced(int maxac, bitarray& l_bb , nodelist_t& l_v);	//selective framework  (currently main variant)
				
	//test framework
	virtual	int set_up();
	virtual	void run();	
	virtual	void tear_down(){ Clique<ugraph>::tear_down();}

	//root pre-processing
	int partitioning(int lb, bitarray& bbsg, nodelist_t& lv);				//partitions subgraph in bbsg: lv will contain branching set that can improve size of feasible solution (lb)
};

inline
int  CliqueInfraPlus::partitioning(int LB, bitarray& bbsg, nodelist_t& lv){
////////////////////
// similar to selective painting but uses LB instead of KMIN, bbsg as input and
// lv as candidate set
//
// RETURNS: number of candidates
//
// COMMENTS: currently no recoloring during ISET building
//
// ***TODO 1) add recoloring, 2) attempt to push all uncolored nodes 
		
	int cmax=1, nBB=EMPTY_ELEM, v=EMPTY_ELEM;
	lv.index=EMPTY_ELEM;
	int pc=(m_unsel=bbsg).popcn64();
	if(pc==0) return 0;													/* should not happen */
	
/////////////////////////////////////////////
//finds LB ISETS 

	iop.node_state_active.erase_bit();
	iop.NB_OF_COLORS=1;
	iop.m_colSets[iop.NB_OF_COLORS].erase_bit(false);					/* lazy erasing, will be updated later*/	
	while(true){ 
		for(int i=0; i<iop.NB_OF_BB_NODES; i++){
			iop.m_colSets[iop.NB_OF_COLORS].bb.get_bitboard(i)=m_unsel.get_bitboard(i);
		}

		iop.m_colSets[iop.NB_OF_COLORS].bb.init_scan(bbo::NON_DESTRUCTIVE);
		while(true){
			v=iop.m_colSets[iop.NB_OF_COLORS].bb.next_bit(nBB,m_unsel);
			if(v==EMPTY_ELEM)
				break;

			//stores color label
			iop.m_colSets[iop.NB_OF_COLORS].size++;							/* the node is already there, simply increment size: equivalent to push(v) */
			iop.node_iset_no[v]=iop.NB_OF_COLORS;
			iop.node_state_active.set_bit(v);

			//checks exit condition
			if( (--pc)==0){
				if(iop.NB_OF_COLORS==1){									/* case: all nodes independent, singleton color set	*/
					int v=bbsg.lsbn64();
					lv.nodos[++lv.index]=v;				
				}
				return 0;													/* all nodes pruned (colored with at most LB colors) */
			}

			//actual painting routine, equivalent to: m_colSets[col].bb.erase_block(nBB,g->get_neighbors(v));		
			bitarray& nbor=g->get_neighbors(v);
			for(int i=nBB; i<iop.NB_OF_BB_NODES; i++){
				iop.m_colSets[iop.NB_OF_COLORS].bb.get_bitboard(i)&= ~nbor.get_bitboard(i);
			}
		}//next node of current color

		//exit condition for partial coloring up to, and INCLUDING, LB
		if(iop.NB_OF_COLORS>=LB ){											/* >= is strictly necessary */
			//iop.set_node_state_active(LISTA_BB(depth));					/* active nodes already set */			
			iop.init_inc_maxsatz();											/* includes setting all colors active */
			break;
		}

		//increments color and erases next color in color_db
		iop.m_colSets[++iop.NB_OF_COLORS].erase_bit(false);					/* lazy erasing, will be updated later*/
	}//next color

/////////////////////////////
// reduces candidate set using sat filter
	

	m_unsel.init_scan(bbo::DESTRUCTIVE);
//	iop.print_db(true, true);
	while(true){
		int v=m_unsel.next_bit_del();
		if(v==EMPTY_ELEM) break;
		iop.add_node_to_new_color(v);
		iop.color_unit_stack.push(iop.NB_OF_COLORS);
		if(!iop.inc_maxsatz(v) /* conflict not found- exit */ ){  
														
			lv.nodos[++lv.index]=v;
			while(true){
				v=m_unsel.next_bit_del();
				if(v==EMPTY_ELEM){
					iop.reset_enlarged_isets();
					return lv.index+1;									
				}
				lv.nodos[++lv.index]=v;
			}		
			
		}else{ 
			;/* nodes cut-conflict  */
		}
		
	}//next node
	
	iop.reset_enlarged_isets();		/* reset context operations: check if this is necessary */
	return lv.index+1;
}

inline
void CliqueInfraPlus::clear_color_sets (){
	iop.clear();
}

inline
int CliqueInfraPlus::init_color_sets(){

	clear_color_sets();

	try{
		iop.set_graph(g);
		if(iop.init(m_alloc+1)==-1){
			runtime_error r("CliqueInfraPlus::init_color_sets()-error allocating infra-chrom ColorSets");		//***check this exception is caught below
			throw r;
		}
		
	}catch(exception& e){
		throw;
	}

	return 0;
}

inline
int CliqueInfraPlus::set_up(){
//////////////
// allocates memory, evaluates initial bounds and determines 
// initial trivial solutions
//
// RETURN VALUE: -1 Error, 0-ok (solution not found), >0 size of solution found

	int sol=0;
	res.clear();
	res.set_name(g->get_name());

	//determine allocation info
	search_alloc_t info;							//loads BBMC configuration for search allocation
	info.set(search_alloc_t::ALLOC_COLOR_SETS);		//for recoloring

	switch(param.alg){
	case BBMCR:
	case BBMCL:
	case BBMCL_R:
	case BBMCL_PLUS:
	break;
	default:
		LOG_ERROR("CliqueInfraPlus::setup unknown algorithm");
		return -1;
	}
	
	//actual set_up
	if(param.unrolled){
		LOG_ERROR("CliqueInfraPlus::setup unrolled variant not defined");
		return -1;
	}else{

/////////////////////
// PARITION SETUP (note: ONLY FOR SELECTIVE COLORING ALGORITHMS)

#ifdef PARTITION_SETUP
		Result r; /* to compute time*/
		LOG_INFO("----------STARTING PARTITION SETUP------------------");
		int ub=min<int>(param.ub, m_size), lb=param.lb, sol=0; 
		SETUP<CliqueInfraPlus> st(*this);
		r.tic();
		st.init_sort(SETUP<CliqueInfraPlus>::MIN_WIDTH_BIG);								/* also possible SETUP<CliqueInfraPlus>::MIN_WIDTH*/
		r.toc();
		LOG_INFO("SORTING FINISHED-"<<"t:"<<r.get_user_time());
									
		//determine feasible solution(lb) based on previous order 
		vint vset;
		r.tic();
		int lbp=st.lower_bound(vset);
		r.tic();
		if(lbp>=lb){
			res.clear_all_solutions();
			res.add_solution(vset);	
			lb=lbp;
		}

		LOG_INFO("BASIC LOWER BOUND COMPUTED-"<<"lb:"<<lb<<" t:"<<r.get_user_time());

		//strong feasible solution (if required)
		if(param.init_preproc!=UB){
			r.tic();
			AMTSexec a(RESTARTS, ITERATIONS_PER_RESTART);
			int lb_amts=a.run(*g);
			r.toc();
			if(lb_amts>=lb){
				res.clear_all_solutions();
				res.add_solution(a.get_nodes());	
				lb=lb_amts;
			}
			LOG_INFO("AMTS LOWER BOUND COMPUTED-:"<<"lb:"<<lb<<" t:"<<r.get_user_time());
		}
				
		//update
		maxno=lb;
		res.set_LB(lb);
		
		//kcore upper bound for reduced memory allocation
		KCore<ugraph> kc(*g);
		r.tic();
		kc.kcore();
		r.toc();
		int ubkc=kc.get_kcore_number()+1;
		ub=min<int>(ub, ubkc);
		LOG_INFO("KCORE ANALYSYS FOR UB DONE-:"<<"ub:"<<ub<<" t:"<<r.get_user_time());

		//check if the problem is already solved
		if(lb==ub){
			LOG_INFO("[w="<<lb<<"]"<<" Solved during partitioning at root");
			res.set_UB(lb);
			return lb;
		}

		//memory allocation
		info.remove(search_alloc_t::ALLOC_COLOR_LABELS);				/* color bounds not necessary-algorithm specific */
		info.size=ub;													/* reduced memory allocation considering current ub */
		r.tic();
		if(search_allocation(info)==-1) return -1;
		r.toc();
		LOG_INFO("MEM ALLOCATION FINISHED-"<< "t:"<<r.get_user_time());

		//partitioning
		r.tic();
		if(st.partitioning(lb)==0){
			r.toc();
			LOG_INFO("PARTITIONING COMPUTED-"<<" t:"<<r.get_user_time());
			LOG_INFO("[w="<<lb<<"]"<<" Solved during partitioning at root");
			res.set_UB(lb);
			sol=lb;	

			//output solution to screen
			stringstream sstr("");
			com::stl::print_collection(decode_first_solution(),sstr);
			LOG_INFO(sstr.str());
			///////////////////
		}
		r.toc();
		LOG_INFO("PARTITIONING COMPUTED-"<<" t:"<<r.get_user_time());
		LOG_INFO("w:["<<lb<<","<<ub<<"]");
		LOG_INFO("----------END PARTITION SETUP------------------");
		return sol;
	}
	
//END OF PARITIIONING SETUP
//////////////////////
#else

		if( (sol=set_up_non_unrolled(info))>0 ){
				res.set_UB(sol);
		}else{//Trivial solution not found 

			//*** additional extra-initialization non_unrolled case ***
		

////////////////////
// PREPROC
		
			//UB
			if(param.init_preproc!=HEUR){
				LOG_PRINT("COMPUTING UB");
				InitColorUB c(*g);

#ifdef ENHANCED_INIT_UB
				c.Compute_UB_enhanced_last(m_lcol[0]);
#else
				//c.Compute_UB_last(m_lcol[0]);
				c.Compute_incUB(m_lcol[0]);					//new linear UB (27/01/17)
				c.Compute_trivial_UB(m_lcol[0]);			//new linear-trivial UB (17/03/17)
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
					res.add_solution(a.get_nodes());		/* stores initial lower bound */
				}
			}
		}
	}//end_if--param.unrolled	
	
	//trivial solution
	if(sol>0){
		LOG_INFO("[w="<<sol<<"]"<<" TRIVIALLY SOLVED DURING PRECOMPUTATION");
	}

	//com::stl::print_collection(res.get_first_solution());
	return sol;
#endif

}

inline
void CliqueInfraPlus::run(){
	//algorithm
	if(param.unrolled){
		LOG_ERROR("CliqueInfraPlus::run: unrolling not defined for this familiy of algorithms");
		return;
	}else{
		res.tic();
		switch(param.alg){
		case BBMCL_R:
			expand_sel_R(0, Clique<ugraph>::m_bbroot, Clique<ugraph>::m_lroot);
			break;
		case BBMCL:
			expand_sel(0, Clique<ugraph>::m_bbroot, Clique<ugraph>::m_lroot);
			break;
		case BBMCL_PLUS:
			expand_sel_enhanced(0, Clique<ugraph>::m_bbroot, Clique<ugraph>::m_lroot);
			break;
		default:
			LOG_ERROR("CliqueInfraPlus::run-non_unrolled:unknown clique algorithm");
		}

		res.toc();
	}

	//read solution
	res.set_UB(maxno);
	//LOG_INFO("[w:"<<res.get_upper_bound()<<",t:"<<res.get_user_time()<<"s]");
}	


inline
void CliqueInfraPlus::expand_sel (int maxac, bitarray& l_bb , nodelist_t& l_v){
////////////////////////
// recursive search algorithm

	int v;
	res.inc_number_of_steps();
	
	//main loop
	while(l_v.index>=0){
			
		//Estrategias
		v=l_v.nodos[l_v.index--];

		//CUT by color (since [Konc & Janecic, 2007] this is of limited use (only for first branch))
#ifdef STRONG_ROOT_COLORING
		if(maxac==0){
			if( (m_lcol[maxac][v]+maxac)<=maxno ){
				l_bb.erase_bit(v);
				continue;
			}
		}
#endif

#ifdef ROOT_VERTEX_PROGRESS
		if(maxac==0)
			cout<<"root vertex: "<<v<<endl;
#endif
/////////////////////////////////
// CHILD NODE GENERATION
		
		//Node generation by masking
		AND(g->get_neighbors(v), l_bb, LISTA_BB(maxac));		//optimized when place second the bitset with higher population
		
		//Leaf node: updates incumbent if necessary
		if( LISTA_BB(maxac).is_empty()){
			if(maxac>=maxno){
				maxno=maxac+1;						//NEW GLOBAL OPTIMUM FOUND

				#ifdef STORE_SOLUTION
					res.set_UB(maxno);
					res.clear_all_solutions();
					m_path[maxac]=v;
					res.add_solution(maxno, m_path);
				
					#ifdef VIEW_PROGRESS
						stringstream sstr("");
						res.print_first_sol(sstr);
						LOG_INFO(sstr.str());
					#endif
					
				#endif
			
			}
		l_bb.erase_bit(v);
		continue;
		}
		//approx. coloring (generates child list of nodes in LISTA_L(maxac))
		paint_sel(maxac);
	
		//cuts if there are no child nodes of v
		if(LISTA_L(maxac).index<0){
			l_bb.erase_bit(v);
			continue;
		}
				
///////////////////////////////////////////////////////
// CANDIDATE EXPANSION

		//sets path
		m_path[maxac]=v;
				
		//Generacion de nuevos nodos
		expand_sel(maxac+1,LISTA_BB(maxac),LISTA_L(maxac));  

		//////////////////////////////////////////////
		// BACKTRACK (does not delete v from path since it will be overwritten in the same level of search tree)
		l_bb.erase_bit(v); 

	}// next node
}

inline
void CliqueInfraPlus::expand_sel_enhanced (int maxac, bitarray& l_bb , nodelist_t& l_v){
////////////////////////
// recursive search algorithm

	int v;
	res.inc_number_of_steps();

	//main loop
	while(l_v.index>=0){
			
		//Estrategias
		v=l_v.nodos[l_v.index--];

		//CUT by color (since [Konc & Janecic, 2007] this is of limited use (only for first branch))
#ifdef STRONG_ROOT_COLORING
		if(maxac==0){
			if( (m_lcol[maxac][v]+maxac)<=maxno ){
				l_bb.erase_bit(v);
				continue;
			}
		}
#endif

#ifdef ROOT_VERTEX_PROGRESS
		if(maxac==0)
			cout<<"root vertex: "<<v<<endl;
#endif
/////////////////////////////////
// CHILD NODE GENERATION
		
		//Node generation by masking
		AND(g->get_neighbors(v), l_bb, LISTA_BB(maxac));		//optimized when place second the bitset with higher population
		
		//Leaf node: updates incumbent if necessary
		if( LISTA_BB(maxac).is_empty()){
			if(maxac>=maxno){
				maxno=maxac+1;						//NEW GLOBAL OPTIMUM FOUND

				#ifdef STORE_SOLUTION
					res.set_UB(maxno);
					res.clear_all_solutions();
					m_path[maxac]=v;
					res.add_solution(maxno, m_path);
				
					#ifdef VIEW_PROGRESS
						stringstream sstr("");
						res.print_first_sol(sstr);
						LOG_INFO(sstr.str());
					#endif
					
				#endif
			
			}
		l_bb.erase_bit(v);
		continue;
		}
		//approx. coloring (generates child list of nodes in LISTA_L(maxac))
		paint_sel_enhanced(maxac);
		/*LISTA_L(maxac).print(cout, true); cout<<endl;
		LISTA_BB(maxac).print(); cout<<endl;*/
		
	//	cin.get();


		//cuts if there are no child nodes of v
		if(LISTA_L(maxac).index<0){
			l_bb.erase_bit(v);
			continue;
		}
				
///////////////////////////////////////////////////////
// CANDIDATE EXPANSION

		//sets path
		m_path[maxac]=v;
				
		//Generacion de nuevos nodos
		expand_sel_enhanced(maxac+1,LISTA_BB(maxac),LISTA_L(maxac));  

		//////////////////////////////////////////////
		// BACKTRACK (does not delete v from path since it will be overwritten in the same level of search tree)
		l_bb.erase_bit(v); 

	}// next node
}

inline
void CliqueInfraPlus::expand_sel_R (int maxac, bitarray& l_bb , nodelist_t& l_v){
////////////////////////
// recursive search algorithm

	int v;
	res.inc_number_of_steps();

	//main loop
	while(l_v.index>=0){
			
		//Estrategias
		v=l_v.nodos[l_v.index--];

		//CUT by color (since [Konc & Janecic, 2007] this is of limited use (only for first branch))
#ifdef STRONG_ROOT_COLORING
		if(maxac==0){
			if( (m_lcol[maxac][v]+maxac)<=maxno ){
				l_bb.erase_bit(v);
				continue;
			}
		}
#endif

#ifdef ROOT_VERTEX_PROGRESS
		if(maxac==0)
			cout<<"root vertex: "<<v<<endl;
#endif
/////////////////////////////////
// CHILD NODE GENERATION
		
		//Node generation by masking
		AND(g->get_neighbors(v), l_bb, LISTA_BB(maxac));		//optimized when place second the bitset with higher population
		
		//Leaf node: updates incumbent if necessary
		if( LISTA_BB(maxac).is_empty()){
			if(maxac>=maxno){
				maxno=maxac+1;						//NEW GLOBAL OPTIMUM FOUND

				#ifdef STORE_SOLUTION
					res.set_UB(maxno);
					res.clear_all_solutions();
					m_path[maxac]=v;
					res.add_solution(maxno, m_path);
				
					#ifdef VIEW_PROGRESS
						stringstream sstr("");
						res.print_first_sol(sstr);
						LOG_INFO(sstr.str());
					#endif
					
				#endif
			
			}
		l_bb.erase_bit(v);
		continue;
		}
		//approx. coloring (generates child list of nodes in LISTA_L(maxac))
		paint_sel_R(maxac);

		//cuts if there are no child nodes of v
		if(LISTA_L(maxac).index<0){
			l_bb.erase_bit(v);
			continue;
		}
				
///////////////////////////////////////////////////////
// CANDIDATE EXPANSION

		//sets path
		m_path[maxac]=v;
				
		//Generacion de nuevos nodos
		expand_sel_R(maxac+1,LISTA_BB(maxac),LISTA_L(maxac));  

		//////////////////////////////////////////////
		// BACKTRACK (does not delete v from path since it will be overwritten in the same level of search tree)
		l_bb.erase_bit(v); 

	}// next node
}

inline
void  CliqueInfraPlus::paint_sel (int depth){
////////////
// Basic independent set coloring routine with pmaxsat branching filter
//
	int cmax=1; int KMIN=maxno-depth; int nBB=EMPTY_ELEM; int v=EMPTY_ELEM; 
	LISTA_L(depth).index=EMPTY_ELEM;								
	const int DEPTHPLUS1=depth+1;
	
	int pc=(m_unsel=LISTA_BB(depth)).popcn64();
	if(pc==0) return;														/*empty subgraph-should not ocurr */		
	
/////////////////////////////////////////////
//color first KMIN nodes as usual

	iop.node_state_active.erase_bit();
	iop.NB_OF_COLORS=1;
	iop.m_colSets[iop.NB_OF_COLORS].erase_bit(false);						/* lazy erasing, will be updated later*/	
	while(true){ 
		for(int i=0; i<iop.NB_OF_BB_NODES; i++){
			iop.m_colSets[iop.NB_OF_COLORS].bb.get_bitboard(i)=m_unsel.get_bitboard(i);
		}

		iop.m_colSets[iop.NB_OF_COLORS].bb.init_scan(bbo::NON_DESTRUCTIVE);
		while(true){
			v=iop.m_colSets[iop.NB_OF_COLORS].bb.next_bit(nBB,m_unsel);
			if(v==EMPTY_ELEM)
				break;

			//stores color label
			iop.m_colSets[iop.NB_OF_COLORS].size++;							/* the node is already there, simply increment size: equivalent to push(v) */
			iop.node_iset_no[v]=iop.NB_OF_COLORS;
			//m_lcol[DEPTHPLUS1][v]=iop.NB_OF_COLORS;							/* is this needed? */
			iop.node_state_active.set_bit(v);
				
			//checks exit condition
			if( (--pc)==0){
				if(iop.NB_OF_COLORS==1){
					int v=LISTA_BB(depth).lsbn64();
					LISTA_L(depth).nodos[++LISTA_L(depth).index]=v;
					//m_lcol[DEPTHPLUS1][v]=1;								/* is this needed? */
				}
				return;														/* all nodes colored with <KMIN colors */
			}

			//actual painting routine, equivalent to: m_colSets[col].bb.erase_block(nBB,g->get_neighbors(v));		
			bitarray& nbor=g->get_neighbors(v);
			for(int i=nBB; i<iop.NB_OF_BB_NODES; i++){
				iop.m_colSets[iop.NB_OF_COLORS].bb.get_bitboard(i)&= ~nbor.get_bitboard(i);
			}
		}//next node of current color

		//exit condition for partial coloring up to, and including, kmin
		if((iop.NB_OF_COLORS+1)>=KMIN ){									/* >= is strictly necessary */
			//iop.set_node_state_active(LISTA_BB(depth));					/* active nodes already set */			
			iop.init_inc_maxsatz();											/* includes setting all colors active */
			break;
		}

		//increments color and erases next color in color_db
		iop.m_colSets[++iop.NB_OF_COLORS].erase_bit(false);					/* lazy erasing, will be updated later*/
	}//next color

/////////////////////////////
// pmax-sat incremental filter
	m_unsel.init_scan(bbo::DESTRUCTIVE);
//	iop.print_db(true, true);
	while(true){
		int v=m_unsel.next_bit_del();
		if(v==EMPTY_ELEM) break;
		iop.add_node_to_new_color(v);
		iop.color_unit_stack.push(iop.NB_OF_COLORS);
		if(!iop.inc_maxsatz(v) /* conflict not found- exit */ ){  

			//if(!iop.filter_non_enlarged()){
			//	LISTA_L(depth).nodos[++LISTA_L(depth).index]=v;
			//}/*else{
			//	LOG_INFO("DYNAMIC FILTER:"<<v);
			//}*/
			
			/*bool result=iop.test_by_eliminate_failed_nodes();
			if(result==false){
				LISTA_L(depth).nodos[++LISTA_L(depth).index]=v;
			}*/

								
			LISTA_L(depth).nodos[++LISTA_L(depth).index]=v;
			while(true){
				v=m_unsel.next_bit_del();
				if(v==EMPTY_ELEM){
					iop.reset_enlarged_isets();
					return;									//first non-cut node
				}
				LISTA_L(depth).nodos[++LISTA_L(depth).index]=v;
			}		
			
		}else{ /* nodes cut-conflict  */
			//m_lcol[DEPTHPLUS1][v]=iop.NB_OF_COLORS;	
			;
		}
		
	}//next node
	
	iop.reset_enlarged_isets();		/* reset context operations: check if this is necessary */
	return;								
}

inline
void  CliqueInfraPlus::paint_sel_enhanced (int depth){
////////////
// Enhances basic painting routine of incremental sat filter by considering other candidates
// apart from the first which is not pruned.
//
// date of creation: 3/7/17
//
// COMMENTS: 
//  i. Proves that this strategy is incomplete
//  ii. In general requires more steps than without the enhancement (possibly becasuse the deeper levels have a bigger branching factor)
//  iii. Might be interesting for the MWCP
//  iv. Is this possible combined with recoloring?


	int cmax=1; int KMIN=maxno-depth; int nBB=EMPTY_ELEM; int v=EMPTY_ELEM; 
	LISTA_L(depth).index=EMPTY_ELEM;								
	const int DEPTHPLUS1=depth+1;
	
	int pc=(m_unsel=LISTA_BB(depth)).popcn64();
	if(pc==0) return;														/*empty subgraph-should not ocurr */		
	
/////////////////////////////////////////////
//color first KMIN nodes as usual

	iop.node_state_active.erase_bit();
	iop.NB_OF_COLORS=1;
	iop.m_colSets[iop.NB_OF_COLORS].erase_bit(false);						/* lazy erasing, will be updated later*/	
	while(true){ 
		for(int i=0; i<iop.NB_OF_BB_NODES; i++){
			iop.m_colSets[iop.NB_OF_COLORS].bb.get_bitboard(i)=m_unsel.get_bitboard(i);
		}

		iop.m_colSets[iop.NB_OF_COLORS].bb.init_scan(bbo::NON_DESTRUCTIVE);
		while(true){
			v=iop.m_colSets[iop.NB_OF_COLORS].bb.next_bit(nBB,m_unsel);
			if(v==EMPTY_ELEM)
				break;

			//stores color label
			iop.m_colSets[iop.NB_OF_COLORS].size++;							/* the node is already there, simply increment size: equivalent to push(v) */
			iop.node_iset_no[v]=iop.NB_OF_COLORS;
			//m_lcol[DEPTHPLUS1][v]=iop.NB_OF_COLORS;							/* is this needed? */
			iop.node_state_active.set_bit(v);
				
			//checks exit condition
			if( (--pc)==0){
				if(iop.NB_OF_COLORS==1){
					int v=LISTA_BB(depth).lsbn64();
					LISTA_L(depth).nodos[++LISTA_L(depth).index]=v;
					//m_lcol[DEPTHPLUS1][v]=1;								/* is this needed? */
				}
				return;														/* all nodes colored with <KMIN colors */
			}

			//actual painting routine, equivalent to: m_colSets[col].bb.erase_block(nBB,g->get_neighbors(v));		
			bitarray& nbor=g->get_neighbors(v);
			for(int i=nBB; i<iop.NB_OF_BB_NODES; i++){
				iop.m_colSets[iop.NB_OF_COLORS].bb.get_bitboard(i)&= ~nbor.get_bitboard(i);
			}
		}//next node of current color

		//exit condition for partial coloring up to, and including, kmin
		if((iop.NB_OF_COLORS+1)>=KMIN ){									/* >= is strictly necessary */
			//iop.set_node_state_active(LISTA_BB(depth));					/* active nodes already set */			
			iop.init_inc_maxsatz();											/* includes setting all colors active */
			break;
		}

		//increments color and erases next color in color_db
		iop.m_colSets[++iop.NB_OF_COLORS].erase_bit(false);					/* lazy erasing, will be updated later*/
	}//next color


	//iop.print_db(true, true);


/////////////////////////////
// pmax-sat incremental filter: filters all possible cand. nodes
	m_unsel.init_scan(bbo::DESTRUCTIVE);
	while(true){
		int v=m_unsel.next_bit_del();
		if(v==EMPTY_ELEM ){
			break;
		}
		
		iop.add_node_to_new_color(v);
		iop.color_unit_stack.push(iop.NB_OF_COLORS);
	//	iop.print_db(true, true);
		if(!iop.inc_maxsatz(v) /* conflict not found- exit */ ){ 

			//if(!iop.filter_non_enlarged()){
			//	LISTA_L(depth).nodos[++LISTA_L(depth).index]=v;
			//}/*else{
			//	LOG_INFO("DYNAMIC FILTER:"<<v);
			//}*/
			
			/*bool result=iop.test_by_eliminate_failed_nodes();
			if(result==false){
				LISTA_L(depth).nodos[++LISTA_L(depth).index]=v;
			}*/
			if(depth<=1){			//or any other heuristic here: DEFAULT should be no heuristic
			
				//removes unpruned vertex from DB, else it is incomplete
				iop.node_state_active.erase_bit(v);
				iop.color_state_active.erase_bit(iop.NB_OF_COLORS);	
				iop.NB_OF_COLORS--;
				//		iop.print_db(true, true);

				//remove v from unit color stack
				if(iop.color_unit_stack.pt>1){
					iop.color_unit_stack.stack[0]=iop.color_unit_stack.stack[iop.color_unit_stack.pt-1];
					iop.color_unit_stack.pt--;
				}else iop.color_unit_stack.pt=0;
							
				//adds v to the final candidate set
				LISTA_L(depth).nodos[++LISTA_L(depth).index]=v;
			}else{
			
				//final update of candidate set
				LISTA_L(depth).nodos[++LISTA_L(depth).index]=v;
				while(true){
					v=m_unsel.next_bit_del();
					if(v==EMPTY_ELEM){
						iop.reset_enlarged_isets();
						return;									//first non-cut node
					}
					LISTA_L(depth).nodos[++LISTA_L(depth).index]=v;
				}		
			}
		}//end-conflict not found for vertex 	
	
		//vertex conflicting-CUT

	}//next node
		
	iop.reset_enlarged_isets();		/* reset context operations: check if this is necessary */
	return;								
}


inline
void  CliqueInfraPlus::paint_sel_R (int depth){
////////////
// Basic independent set coloring routine with pmaxsat branching filter and
// recoloring
//
// TESTING FILTER OF FIRST VERTEX THAT IS NOT PRUNED!

	int cmax=1; int KMIN=maxno-depth; int nBB=EMPTY_ELEM; int v=EMPTY_ELEM; 
	LISTA_L(depth).index=EMPTY_ELEM;								
	const int DEPTHPLUS1=depth+1;
	
	int pc=(m_unsel=LISTA_BB(depth)).popcn64();
	if(pc==0) return;														/*empty subgraph-should not ocurr */		
	
/////////////////////////////////////////////
//color first KMIN nodes as usual

	iop.node_state_active.erase_bit();
	iop.NB_OF_COLORS=1;
	iop.m_colSets[iop.NB_OF_COLORS].erase_bit(false);						/* lazy erasing, will be updated later*/	
	while(true){ 
		for(int i=0; i<iop.NB_OF_BB_NODES; i++){
			iop.m_colSets[iop.NB_OF_COLORS].bb.get_bitboard(i)=m_unsel.get_bitboard(i);
		}

		iop.m_colSets[iop.NB_OF_COLORS].bb.init_scan(bbo::NON_DESTRUCTIVE);
		while(true){
			v=iop.m_colSets[iop.NB_OF_COLORS].bb.next_bit(nBB,m_unsel);
			if(v==EMPTY_ELEM)
				break;

			//stores color label
			iop.m_colSets[iop.NB_OF_COLORS].size++;							/* the node is already there, simply increment size: equivalent to push(v) */
			iop.node_iset_no[v]=iop.NB_OF_COLORS;
			//m_lcol[DEPTHPLUS1][v]=iop.NB_OF_COLORS;							/* is this needed? */
			iop.node_state_active.set_bit(v);
				
			//checks exit condition
			if( (--pc)==0){
				if(iop.NB_OF_COLORS==1){
					int v=LISTA_BB(depth).lsbn64();
					LISTA_L(depth).nodos[++LISTA_L(depth).index]=v;
					//m_lcol[DEPTHPLUS1][v]=1;								/* is this needed? */
				}
				return;														/* all nodes colored with <KMIN colors */
			}

			//actual painting routine, equivalent to: m_colSets[col].bb.erase_block(nBB,g->get_neighbors(v));		
			bitarray& nbor=g->get_neighbors(v);
			for(int i=nBB; i<iop.NB_OF_BB_NODES; i++){
				iop.m_colSets[iop.NB_OF_COLORS].bb.get_bitboard(i)&= ~nbor.get_bitboard(i);
			}
		}//next node of current color

		//exit condition for partial coloring up to, and including, kmin
		if((iop.NB_OF_COLORS+1)>=KMIN ){									/* >= is strictly necessary */
				break;
		}

		//increments color and erases next color in color_db
		iop.m_colSets[++iop.NB_OF_COLORS].erase_bit(false);					/* lazy erasing, will be updated later*/
	}//next color


/////////////////////////////////////////////////
// RECOLORING ATTEMPT: insert unlabeled nodes in KMIN-1 PARTIAL COLORING
		
	const int NB_OF_BB_NODES_MINUS_ONE=iop.NB_OF_BB_NODES-1;
	int vswap;
	//const int NB_OF_COLORS_MINUS_ONE=iop.NB_OF_COLORS-1;
	if(iop.NB_OF_COLORS>=2){
		m_unsel.init_scan(bbo::NON_DESTRUCTIVE);
		while(true){
next_v:		int v=m_unsel.next_bit();
			if(v==EMPTY_ELEM) break;
			for(int recol=1; recol<iop.NB_OF_COLORS; recol++){
				int pc_swap=iop.m_colSets[recol].bb.single_disjoint(0, NB_OF_BB_NODES_MINUS_ONE, g->get_neighbors(v), vswap);
				if(pc_swap==1){	//candidate color class found
					//for(int j=recol+1; j<kmin; j++){
					for(int j=recol+1; j<=iop.NB_OF_COLORS; j++){
						if(iop.m_colSets[j].bb.is_disjoint(0, NB_OF_BB_NODES_MINUS_ONE, g->get_neighbors(vswap))){

							iop.m_colSets[j].push(vswap);
							iop.m_colSets[recol].push(v);
							iop.m_colSets[recol].erase_bit(vswap);
							m_unsel.erase_bit(v);

							iop.node_iset_no[vswap]=j;
							iop.node_iset_no[v]=recol;

							iop.node_state_active.set_bit(v);
						//	LOG_INFO("RECOLORED");

							if((--pc)==0)
								return;
							else  goto next_v;
						}
					}//next j
				} else if(pc_swap==0){
					m_unsel.erase_bit(v);
					iop.m_colSets[recol].push(v);
					iop.node_iset_no[v]=recol;

					iop.node_state_active.set_bit(v);

					//empty check of unsel in case vertex swapped is the last one
					if((--pc)==0)
						 return;
					else break;			/* next node */
				}
			}//next recol

		}//next node
	} //endif
		
/////////////////////////////
// pmax-sat incremental filter
	iop.init_inc_maxsatz();													/* includes setting all colors active */
	m_unsel.init_scan(bbo::DESTRUCTIVE);
	while(true){
		int v=m_unsel.next_bit_del();
		if(v==EMPTY_ELEM) break;
		iop.add_node_to_new_color(v);
		iop.color_unit_stack.push(iop.NB_OF_COLORS);
		if(!iop.inc_maxsatz(v) /* conflict not found- exit */ ){  
			if(!iop.filter_non_enlarged()){
				LISTA_L(depth).nodos[++LISTA_L(depth).index]=v;
				
			}/*else{
				LOG_INFO("FILTERED DYNAMICALLY:"<<v);
			}*/
			while(true){
				v=m_unsel.next_bit_del();
				if(v==EMPTY_ELEM){
					iop.reset_enlarged_isets();
					return;									//first non-cut node
				}
				LISTA_L(depth).nodos[++LISTA_L(depth).index]=v;
			}		
			
		}else{ /* nodes cut-conflict  */
			//m_lcol[DEPTHPLUS1][v]=iop.NB_OF_COLORS;	
			;
		}
		
	}//next node
	
	iop.reset_enlarged_isets();		/* reset context operations: check if this is necessary */
	return;								
}





#endif


