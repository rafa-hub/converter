////////////////////////////////
// clique_all_max_sol.h: interface for the CliqueAll class which contains different implementations of an enhanced
//				   infra-chromatic filter for bit-parallel, non-iterative MCP algorithms 
//
// IMPORTANT: Note the number of solutions stored is determined as a property of result object in utils folder
//
// initial date:11/11/16
// last update: 11/11/16
// author: pablo san segundo


#ifndef  __CLIQUE_ALL_MAX_SOL_H__
#define  __CLIQUE_ALL_MAX_SOL_H__

#include "clique.h"
#include "../init_color_ub.h"
#include "../amts/amts_exec.h"
#include "bitscan/bbalg.h"
#include "infra_tools.h"
#include "../common/common_macros.h"

using namespace com;												//for common types (here bb_t)

class CliqueAll: public Clique<ugraph>{
		
public:
explicit CliqueAll(ugraph* g, param_t p)		:Clique<ugraph>(g, p)  {};
	CliqueAll(param_t p)						:Clique<ugraph>(p) {};
	virtual ~CliqueAll(){}
	void set_param(param_t p)					{Clique<ugraph>::clear_all(); CLQParam::set_param(p);}	


	virtual int init_bitarrays();	
	int allocate	(int ub);					
////////////
//coloring which does not cut ties
	void paint_sat_R_sel (int depth);	
		
	//drivers
	//void expand_sat_R_sel (int maxac, bitarray& l_bb , nodelist_t& l_v);	//infrachrom at the end
	bool expand_sat_R_sel (int maxac, bitarray& l_bb , nodelist_t& l_v);	//infrachrom at the end
	

	//test framework
	virtual	int set_up();
	virtual	void run();	
	virtual	void tear_down(){ Clique<ugraph>::tear_down();}

	//interface for subgraph
	virtual int set_up_subgraph(bitarray& bbs, search_alloc_t* pinfo=NULL);
	void run_subgraph(bitarray& bbs);	
	void run_subgraph_with_ordering(bitarray& bbs,  clqo::init_order_t o, gbbs::place_t p );	

	int set_up_non_unrolled();

///////////////
//data members
	bitarray m_forbidden;
};

inline
int CliqueAll::init_bitarrays(){

	Clique<ugraph>::init_bitarrays();

	//empty set
	m_forbidden.init(m_size);
	return 0;
}

inline
int CliqueAll::set_up_subgraph(bitarray& bbs, search_alloc_t* pinfo){
///////////////////
//overrides to remove initial solution stored in results

	Clique<ugraph>::set_up_subgraph(bbs, pinfo);
	get_result().clear_all_solutions();
	return 0;
}


inline
int CliqueAll::set_up(){
//////////////
// allocates memory, evaluates initial bounds and determines 
// initial trivial solutions
//
// RETURN VALUE: -1 Error, 0-ok, >0 trivial solution found
	
	res.clear();						
	res.set_name(g->get_name());
	
	int sol=set_up_non_unrolled();
	if(sol==-1) return -1;
	else if (sol>0) res.set_UB(sol);
	else{
		////////////////////
		// PREPROC: strong UB and LB (optional)

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
	
	//cleans solution buffer: removes LB feasible solution, since it will be computed on the fly
 	res.clear_all_solutions();			
	return 0;
}

inline
int CliqueAll::allocate	(int ub){
///////////////////
// allocates memory according to ub

	search_alloc_t info;							
	info.size=ub;
	switch(param.alg){
	case BBMCXR_L:
		info.set(search_alloc_t::ALLOC_COLOR_SETS);	
	break;
	default:
		LOG_ERROR("CliqueAll::setup unknown algorithm");
		return -1;
	}
	
	return search_allocation(info);
}

inline 
int CliqueAll::set_up_non_unrolled(){
//////////////////
// overrides to be able to work with lb=ub

	PrecisionTimer pt;
	int ub=param.ub; 
	int lb=param.lb;
	
	//initial reordering of adjacency matrix following degeneracy
	if(param.init_order!=NONE){
		LOG_PRINT("initial ordering----------------");
		CliqueSort<ugraph> o(*g);
		if(o.reorder( o.new_order(param.init_order, gbbs::PLACE_LF), get_decoder())==-1){
			LOG_ERROR("set_up_unrolled: error during reordering");
			return -1;
		}
		LOG_PRINT("------------------------------------");
	}
	
	//compute simple initial bounds
	int sol=initial_bounds(lb, ub);
	maxno=lb;
	res.set_LB(lb);
	res.set_UB(ub);
	if(sol>0){
		LOG_INFO("w:["<<lb<<","<<ub<<"]-sol found");
	}else{
		LOG_INFO("w:["<<lb<<","<<ub<<"]");
	}

	//mmemory allocation
	if (allocate(ub)==-1) return -1;
	return sol;
}

inline
void CliqueAll::run(){
	//algorithm
	if(param.unrolled){
		LOG_ERROR("CliqueAll::run: unrolling not defined for this familiy of algorithms");
		return;
	}else{
		res.tic();
		switch(param.alg){
		case BBMCXR_L:
			expand_sat_R_sel(0, Clique<ugraph>::m_bbroot, Clique<ugraph>::m_lroot);
			break;
		default:
			LOG_ERROR("CliqueAll::run-non_unrolled:unknown clique algorithm");
		}

		res.toc();
	}

	//read solution
	res.set_UB(maxno);
	LOG_INFO("[w:"<<res.get_upper_bound()<<","<<res.get_user_time()<<"s]");
}

inline
void CliqueAll::run_subgraph(bitarray& bbs){

	maxno=get_result().get_lower_bound();
	m_bbroot=bbs;
	m_lroot.index=-1;
	bbs.init_scan(bbo::NON_DESTRUCTIVE);
	while(true){
		int v=bbs.next_bit();
		if(v==EMPTY_ELEM) break;
		m_lroot.nodos[++m_lroot.index]=v;
	}
	
	res.tic();
	switch(param.alg){
	case BBMCXR_L:
		expand_sat_R_sel(0, Clique<ugraph>::m_bbroot, Clique<ugraph>::m_lroot);
		break;
	default:
		LOG_ERROR("CliqueAll::run-non_unrolled:unknown clique algorithm");
	}

	res.toc();
	
	//read solution
	res.set_UB(maxno);
	LOG_INFO("[w:"<<res.get_upper_bound()<<","<<res.get_user_time()<<"s]");
}

inline
void CliqueAll::run_subgraph_with_ordering(bitarray& bbs,  clqo::init_order_t o, gbbs::place_t p ){
//////////////
// sorts input subgraph bbs by different criteria

	maxno=get_result().get_lower_bound();
	m_bbroot=bbs;
	if(Clique<ugraph>::sort_root_subgraph(bbs, o, p)==-1){
		LOG_ERROR("CliqueAll::run_subgraph:-bizarre sorting of root");
	}else{

		//change root UBs that are now not valid: *** TODO change computation of bounds
		Clique<ugraph>::nodelist_t& nl=Clique<ugraph>::get_root_nodelist();
		int NV=bbs.popcn64();
		bbs.init_scan(bbo::NON_DESTRUCTIVE);
		while(true){
			int v=bbs.next_bit();
			if(v==EMPTY_ELEM) break;
			Clique<ugraph>::m_lcol[0][v]=NV;
		}
		
		res.tic();
		switch(param.alg){
		case BBMCXR_L:
			expand_sat_R_sel(0, Clique<ugraph>::m_bbroot, Clique<ugraph>::m_lroot);
			break;
		default:
			LOG_ERROR("CliqueAll::run-non_unrolled:unknown clique algorithm");
		}

		res.toc();

		//read solution
		res.set_UB(maxno);
		LOG_INFO("[w:"<<res.get_upper_bound()<<","<<res.get_user_time()<<"s]");
	}
}

//inline
//void CliqueAll::expand_sat_R_sel (int maxac, bitarray& l_bb , nodelist_t& l_v){
//////////////////////////
//// recursive search algorithm
//		
//	int v;
////main loop
//	res.inc_number_of_steps();
//
//	while(l_v.index>=0){
//			
//		//Estrategias
//		v=l_v.nodos[l_v.index--];
//	
//		//ROOT CUT
//#ifdef STRONG_ROOT_COLORING
//		if(maxac==0){
//			if( (m_lcol[maxac][v]+maxac)< /*=*/maxno )
//				continue;
//		}
//
//#endif
//
//#ifdef ROOT_VERTEX_PROGRESS
//		if(maxac==0)
//			cout<<"root vertex: "<<v<<endl;
//#endif
//
//	
///////////////////////////////////
//// CHILD NODE GENERATION
//		
//		//Node generation by masking
//		AND(g->get_neighbors(v), l_bb, LISTA_BB(maxac));		//optimized when place second the bitset with higher population
//		
//	
//		//Leaf node: updates incumbent if necessary
//		if( LISTA_BB(maxac).is_empty()){
//			if(maxac>=maxno){
//				maxno=maxac+1;						//NEW GLOBAL OPTIMUM FOUND
//
//				#ifdef STORE_SOLUTION
//					res.set_UB(maxno);
//					res.clear_all_solutions();
//					m_path[maxac]=v;
//					res.add_solution(maxno, m_path);
//				
//					#ifdef VIEW_PROGRESS
//						stringstream sstr("");
//						res.print_first_sol(sstr);
//						LOG_INFO(sstr.str());
//					#endif
//					
//				#endif				
//			}else if(maxac==(maxno-1)){
//				//new maximum solution of same size
//				#ifdef STORE_SOLUTION
//					 m_path[maxac]=v;
//					 bool enough_space=res.add_solution(maxno, m_path);
//					
//					 #ifdef VIEW_PROGRESS
//					 if(enough_space){
//						stringstream sstr("");
//						res.print_last_sol(sstr);
//						LOG_INFO(sstr.str());
//					 }else{
//							  LOG_ERROR("not enough space to store solutions");
//						//  //*** if lb==ub EXIT
//						}
//					#endif
//				#endif
//			}
//		l_bb.erase_bit(v);
//		continue;
//		}
//
//		//approx. coloring (generates child list of nodes in LISTA_L(maxac))
//		paint_sat_R_sel(maxac);
//	
//		//cuts if there are no child nodes of v
//		if(LISTA_L(maxac).index<0){
//			l_bb.erase_bit(v);
//			continue;
//		}
//				
/////////////////////////////////////////////////////////
//// CANDIDATE EXPANSION
//
//		//sets path
//		m_path[maxac]=v;
//				
//		//Generacion de nuevos nodos
//		expand_sat_R_sel(maxac+1,LISTA_BB(maxac),LISTA_L(maxac));  
//
//		//////////////////////////////////////////////
//		// BACKTRACK (does not delete v from path since it will be overwritten in the same level of search tree)
//		l_bb.erase_bit(v); 
//	}// next node
//}

inline
bool CliqueAll::expand_sat_R_sel (int maxac, bitarray& l_bb , nodelist_t& l_v){
////////////////////////
// recursive search algorithm
		
	int v;
//main loop
	res.inc_number_of_steps();

	while(l_v.index>=0){
			
		//Estrategias
		v=l_v.nodos[l_v.index--];
	
		//ROOT CUT
#ifdef STRONG_ROOT_COLORING
		if(maxac==0){
			if( (m_lcol[maxac][v]+maxac)< /*=*/maxno ){
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
					res.set_LB(maxno);
					res.clear_all_solutions();
					m_path[maxac]=v;
					res.add_solution(maxno, m_path);
				
					#ifdef VIEW_PROGRESS
						stringstream sstr("");
						res.print_first_sol(sstr);
						LOG_INFO(sstr.str());
					#endif
					
				#endif				
			}else if(maxac==(maxno-1)){
				//new maximum solution of same size
				#ifdef STORE_SOLUTION
					 m_path[maxac]=v;
					 bool enough_space=res.add_solution(maxno, m_path);
					
					 #ifdef VIEW_PROGRESS
					 if(enough_space){
						stringstream sstr("");
						res.print_last_sol(sstr);
						LOG_INFO(sstr.str());
					 }else{
							LOG_ERROR("not enough space to store solutions"<<" [lb:"<<maxno<<" ub:"<<res.get_upper_bound()<<"]");
							//if(maxno==res.get_upper_bound()) return true;	
							return true;
						}
					#endif
				#endif
			}
		l_bb.erase_bit(v);
		continue;
		}

		//approx. coloring (generates child list of nodes in LISTA_L(maxac))
		paint_sat_R_sel(maxac);
	
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
		if(expand_sat_R_sel(maxac+1,LISTA_BB(maxac),LISTA_L(maxac))==true)
					return true;  

		//////////////////////////////////////////////
		// BACKTRACK (does not delete v from path since it will be overwritten in the same level of search tree)
		l_bb.erase_bit(v); 
	}// next node

	return false;
}


inline
void CliqueAll::paint_sat_R_sel(int depth){
///////////////////
// Infrachromatic filter + reocloring filter in selective coloring. 
// Applied to ALL vertices with label kmin 
// (recoloring if possible, comes first, to enlarge color subsets as much as possible before they are checked for inconsistency)
// 
// date: 04/09/15
// last_update: 04/09/15
//
// COMMENTS: attemtps to filter vertices with label kmin until first failure.
//			 To attempt to filter all vertices, active while loop (1) (CURRENT STATE ON--MAXIMUM EFFICIENCY)

	int col=1, kmin=maxno-depth-1, v=EMPTY_ELEM, nBB=EMPTY_ELEM;  
	LISTA_L(depth).index=EMPTY_ELEM;
		
	//copies list of vertices to color and stores size for fast empty check 
	int pc=(m_unsel=LISTA_BB(depth)).popcn64();
	//if(pc<kmin){return;}								//CUT based on populaton size (***possibly remove***)
		
	
	////////////////////////////
	//color up to kmin-1
	while(true){
		if(col>=kmin) break;											//note kmin=0 is not cut
		m_colsets[col]=m_unsel;
		m_colsets[col].init_scan(bbo::NON_DESTRUCTIVE);
		while(1){
			v=m_colsets[col].next_bit(nBB, m_unsel);
			if(v==EMPTY_ELEM) break;
			if((--pc)==0){ return;}  //cut, kmin not reached
						
			//color as set difference
			m_colsets[col].erase_block(nBB,g->get_neighbors(v));
		}
		col++;
	}

/////////////////////////////////////////////
// RECOLOR + INFRACHROM FILTERING (col>=kmin always here)
	if(kmin>=3){
		m_forbidden.erase_bit(1, kmin);	
		m_unsel.init_scan(bbo::NON_DESTRUCTIVE);
		while(1){											//REMOVE LOOP to implement filter only up to first failure: when ON attempts all colors
next_v:		v=m_unsel.next_bit();
			if(v==EMPTY_ELEM) break;						

			//recolor v if possible
			for(int recol=1; recol<(kmin-1); recol++){		//loop to find initial color seed
				//filters out forbidden colors
				if(m_forbidden.is_bit(recol)) continue;
				//check if color is valid for swapping (0-1 neighbors)	
				int vswap;
				int pc_swap=m_colsets[recol].single_disjoint(g->get_neighbors(v), vswap);
				if(pc_swap==1){	//color class found
					/////////////////////////////////////////////
					//searches for new color set j>recol
					for(int j=recol+1; j<kmin; j++){
						//filters out forbidden colors
						if(m_forbidden.is_bit(j)) continue;
						if( m_colsets[j].is_disjoint(g->get_neighbors(vswap)) ){
							//updates color subsets
							if((--pc)==0){ return;}
							else{
								//LOG_INFO("RECOLORING FOUND");
								RECOLOR_SWAP(v,vswap,recol,j);
								goto next_v;
							}
						}else if( m_colsets[j].is_disjoint(g->get_neighbors(v),g->get_neighbors(vswap))){
							//updates inconsistent color set with (recol, j)
							if((--pc)==0){ return;}
							else{
								//LOG_INFO("SAT INCONSISTENT SUBSET FOUND");
								INFRA_CHROM(v,recol,j);
								goto next_v;
							}
						}
					}//end of search for new color set j>recol
					/////////////////////////////////////////////
					//searches for new color set j<recol
					for(int j=1; j<recol; j++){
						//filters out forbidden colors
						if(m_forbidden.is_bit(j)) continue;
						if( m_colsets[j].is_disjoint(g->get_neighbors(vswap)) ){
							//updates color subsets
							if((--pc)==0){ return;}
							else{
								//LOG_INFO("RECOLORING FOUND");
								RECOLOR_SWAP(v,vswap,recol,j);
								goto next_v;
							}
						}else if( m_colsets[j].is_disjoint(g->get_neighbors(v),g->get_neighbors(vswap))){
							//updates inconsistent color set with (recol, j)
							if((--pc)==0){ return;}
							else{
								//LOG_INFO("SAT INCONSISTENT SUBSET FOUND");
								INFRA_CHROM(v,recol,j);
								goto next_v;
							}
						}
					}//end of search for new color set j>recol
				}else if(pc_swap==0){ 
					if((--pc)==0){ return;}
					else{
						//LOG_INFO("SIMPLE RECOLORING FOUND");
						RECOLOR_SIMPLE_SWAP(v,recol);
						goto next_v;
					}
				}
			}//next candidate for swap color seed
		}//next attemp at vertex pruning
	}
// END OF INFRACHROM FILTERING
///////////////////////////////////
	//update candidate list and sort by color
	m_unsel.init_scan(bbo::DESTRUCTIVE);
	while(1){
		v=m_unsel.next_bit_del();
		if(v==EMPTY_ELEM)	return;
		
		//add to candidate list
		LISTA_L(depth).nodos[++LISTA_L(depth).index]=v;
	}
 }
 
#endif


