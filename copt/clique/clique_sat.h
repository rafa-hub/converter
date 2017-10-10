////////////////////////////////
// clique_sat.h: interface for CliqueSAT class which contains MaxSAT based bounds
//				  
// initial date:04/09/15
// last update: 
// author: pablo san segundo


#ifndef  __CLIQUE_SAT_H__
#define  __CLIQUE_SAT_H__

#include "clique.h"
#include "../init_color_ub.h"
#include "../amts/amts_exec.h"

#include "../common/common_macros.h"
#include "../setup.h"

class CliqueSat: public Clique<ugraph>{
	
public:
	enum return_t {ROOT_NODE=-1, CONTINUE_SEARCH_IN_BRANCH=0};
	CliqueSat(ugraph* g, param_t p)				:Clique<ugraph>(g, p){};
	CliqueSat(param_t p)						:Clique<ugraph>(p){};
	virtual ~CliqueSat(){}
	
	virtual int init_bitarrays();		

	//paint
	void paint_sat(int depth);
	void paint_sat_sel(int depth);
	void paint_sat_R(int depth);
	void paint_sat_R_sel(int depth);
	void paint_sat_R_sel_seq(int depth);							//recolor+infra-chrom sequentially

	//attempt are recoloring during building of Ckmin-1 subsets
	void paint_sat_R_seq_sel_N_kmin(int depth);										

	//search procedures
	void expand_sat (int maxac, bitarray& l_bb , nodelist_t& l_v);
	void expand_sat_sel (int maxac, bitarray& l_bb , nodelist_t& l_v);
	void expand_sat_R (int maxac, bitarray& l_bb , nodelist_t& l_v);
	void expand_sat_R_sel (int maxac, bitarray& l_bb , nodelist_t& l_v);
	void expand_sat_R_sel_seq (int maxac, bitarray& l_bb , nodelist_t& l_v);

	
	//test framework
	virtual	int set_up();
	virtual	void run();	
	virtual	void tear_down(){ Clique<ugraph>::tear_down();}

private:

////////////////////////
//auxiliary data structures
	bitarray m_forbidden;
};

int CliqueSat::init_bitarrays(){

	Clique<ugraph>::init_bitarrays();

	//empty set
	m_forbidden.init(m_size);
	return 0;
}

inline
int CliqueSat::set_up(){
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
	case BBMCX:
	case BBMCX_L:
	case BBMCXR:
	case BBMCXR_L:
	case BBMCXR_L_SEQ:
	break;
	default:
		LOG_ERROR("CliqueSat::setup unknown algorithm");
		return -1;
	}
	
	//actual set_up
	if(param.unrolled){
		LOG_ERROR("CliqueSat::setup unrolled variant not defined");
		return -1;
	}else{
		
		if( (sol=set_up_non_unrolled(info))>0 ){
				res.set_UB(sol);
		}else{
			//Trivial solution not found 
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
				c.Compute_incUB	(m_lcol[0]);	
				
#endif				
				//output to screen (TODO-LOGGER)
				stringstream sstr("");
				for(int i=0; i<m_size; i++){
					sstr<<m_lcol[0][i]<<" ";
				}
				sstr<<endl;
				LOG_INFO(sstr.str());
												
				//c.Compute_UB_last(m_lcol[0]);
				////output to screen (TODO-LOGGER)
				//sstr.str("");
				//sstr.clear();
				//for(int i=0; i<m_size; i++){
				//	sstr<<m_lcol[0][i]<<" ";
				//}
				//sstr<<endl;
				//LOG_INFO(sstr.str());

			}						
		
			//LB: using amts
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
	
	//trivial solution found 
	if(sol>0){
		LOG_INFO("[w="<<sol<<"]"<<" TRIVIALLY SOLVED DURING PRECOMPUTATION");
	}
	
	return sol;
}

inline
void CliqueSat::run(){
	//algorithm
	if(param.unrolled){
		LOG_ERROR("CliqueSat::run: unrolling not defined for this familiy of algorithms");
		return;
	}else{
		res.tic();
		switch(param.alg){
		case BBMCX:
			expand_sat(0, Clique<ugraph>::m_bbroot, Clique<ugraph>::m_lroot);
			break;
		case BBMCX_L:
			expand_sat_sel(0, Clique<ugraph>::m_bbroot, Clique<ugraph>::m_lroot);
			break;
		case BBMCXR:
			expand_sat_R(0, Clique<ugraph>::m_bbroot, Clique<ugraph>::m_lroot);
			break;
		case BBMCXR_L:
			expand_sat_R_sel(0, Clique<ugraph>::m_bbroot, Clique<ugraph>::m_lroot);
			break;
		case BBMCXR_L_SEQ:
			expand_sat_R_sel_seq(0, Clique<ugraph>::m_bbroot, Clique<ugraph>::m_lroot);
			break;
		default:
			LOG_ERROR("CliqueSat::run-non_unrolled:unknown clique algorithm");
		}

		res.toc();
	}

	//read solution
	res.set_UB(maxno);
	LOG_INFO("[w:"<<res.get_upper_bound()<<","<<res.get_user_time()<<"s]");
}	

inline
void CliqueSat::expand_sat (int maxac, bitarray& l_bb , nodelist_t& l_v){
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
		}else if( (m_lcol[maxac][v]+maxac)<=maxno ){
			break;
		}
#else
		if( (m_lcol[maxac][v]+maxac)<=maxno ){
			break;
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
		paint_sat(maxac);

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
		expand_sat(maxac+1,LISTA_BB(maxac),LISTA_L(maxac));  

		//////////////////////////////////////////////
		// BACKTRACK (does not delete v from path since it will be overwritten in the same level of search tree)
		l_bb.erase_bit(v); 

	}// next node

return;
}

inline
void CliqueSat::expand_sat_R (int maxac, bitarray& l_bb , nodelist_t& l_v){
////////////////////////
// recursive search algorithm infra-chrom + recoloring in the non selective framework

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
		}else if( (m_lcol[maxac][v]+maxac)<=maxno ){
			break;
		}
#else
		if( (m_lcol[maxac][v]+maxac)<=maxno ){
			break;
		}
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
		paint_sat_R(maxac);

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
		expand_sat_R(maxac+1,LISTA_BB(maxac),LISTA_L(maxac));  

		//////////////////////////////////////////////
		// BACKTRACK (does not delete v from path since it will be overwritten in the same level of search tree)
		l_bb.erase_bit(v); 

	}// next node

return;
}



inline
void CliqueSat::expand_sat_R_sel_seq (int maxac, bitarray& l_bb , nodelist_t& l_v){
////////////////////////
// recursive search algorithm infra-chrom + recoloring in the selective framework

	int v;
	res.inc_number_of_steps();
	
	//main loop
	while(l_v.index>=0){
			
		//Estrategias
		v=l_v.nodos[l_v.index--];

		//ROOT CUT
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
		paint_sat_R_sel_seq(maxac);
		//paint_sat_R_seq_sel_N_kmin(maxac);

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
		expand_sat_R_sel_seq(maxac+1,LISTA_BB(maxac),LISTA_L(maxac));  

		//////////////////////////////////////////////
		// BACKTRACK (does not delete v from path since it will be overwritten in the same level of search tree)
		l_bb.erase_bit(v); 

	}// next node

return;
}

inline
void CliqueSat::expand_sat_R_sel (int maxac, bitarray& l_bb , nodelist_t& l_v){
////////////////////////
// recursive search algorithm infra-chrom + recoloring in the selective framework

	int v;
	res.inc_number_of_steps();
	
	//main loop
	while(l_v.index>=0){
			
		//Estrategias
		v=l_v.nodos[l_v.index--];

		//ROOT CUT
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
		expand_sat_R_sel(maxac+1,LISTA_BB(maxac),LISTA_L(maxac));  

		//////////////////////////////////////////////
		// BACKTRACK (does not delete v from path since it will be overwritten in the same level of search tree)
		l_bb.erase_bit(v); 

	}// next node

return;
}

inline
void CliqueSat::expand_sat_sel (int maxac, bitarray& l_bb , nodelist_t& l_v){
////////////////////////
// Infrachromatic search in the selective color framework 
// Moreover, filter is applied SELECTIVELY to vertices with color number >= kmin UNTIL first failure

	int v;
	res.inc_number_of_steps();

	//main loop
	while(l_v.index>=0){
			
		//Estrategias
		v=l_v.nodos[l_v.index--];

		//ROOT CUT
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
		paint_sat_sel(maxac);

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
		expand_sat_sel(maxac+1,LISTA_BB(maxac),LISTA_L(maxac));  

		//////////////////////////////////////////////
		// BACKTRACK (does not delete v from path since it will be overwritten in the same level of search tree)
		l_bb.erase_bit(v); 

	}// next node

return;
}


inline
void CliqueSat::paint_sat(int depth){
///////////////////
// Applies infra-chromatic filter to all candidate vertices 
// Searches for second inconsistent color subset both over and below the first color subset

	int col=1, kmin=maxno-depth, v=EMPTY_ELEM, nBB=EMPTY_ELEM;  
	LISTA_L(depth).index=EMPTY_ELEM;
	const int DEPTH_PLUS1=depth+1; 

	//copies list of vertices to color and stores size for fast empty check 
	int pc=(m_unsel=LISTA_BB(depth)).popcn64();
	//if(pc<kmin){return;}								//CUT based on populaton size (***possibly remove***)
				
	////////////////////////////
	//color up to kmin-1
	while(1){
		if(col>=kmin) break;							//exti condition
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
// INFRACHROM FILTERING (col>=kmin always here)
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
						if( m_colsets[j].is_disjoint(g->get_neighbors(v),g->get_neighbors(vswap))){
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
						if( m_colsets[j].is_disjoint(g->get_neighbors(v),g->get_neighbors(vswap))){
							//updates inconsistent color set with (recol, j)
							if((--pc)==0){ return;}
							else{
								//LOG_INFO("SAT INCONSISTENT SUBSET FOUND");
								INFRA_CHROM(v,recol,j);
								goto next_v;
							}
						}
					}//end of search for new color set j>recol
				}
			}//next candidate for swap color seed
		}//next attemp at vertex pruning
	}
// END OF INFRACHROM FILTERING
///////////////////////////////////
	//update candidate list and sort by color
	while(1){
		m_sel=m_unsel;
		m_sel.init_scan(bbo::DESTRUCTIVE);
		while(1){
			v=m_sel.next_bit_del(nBB,m_unsel);
			if(v==EMPTY_ELEM) break;						
			
			LISTA_L(depth).nodos[++LISTA_L(depth).index]=v;
			m_lcol[DEPTH_PLUS1][v]=col;			
			

			if((--pc)==0){ return ;}											
			m_sel.erase_block(nBB,g->get_neighbors(v));
		}
		col++;
	}
 }

inline
void CliqueSat::paint_sat_R(int depth){
///////////////////
// Applies infra-chromatic + recoloring filter to all candidate vertices
// (recoloring if possible, comes first, to enlarge color subsets as much as possible before they are checked for inconsistency)
// Searches for second inconsistent color subset both over and below the first color subset

	int col=1, kmin=maxno-depth, v=EMPTY_ELEM, nBB=EMPTY_ELEM;  
	LISTA_L(depth).index=EMPTY_ELEM;
	const int DEPTH_PLUS1=depth+1; 

	//copies list of vertices to color and stores size for fast empty check 
	int pc=(m_unsel=LISTA_BB(depth)).popcn64();
	//if(pc<kmin){return;}								//CUT based on populaton size (***possibly remove***)
				
	////////////////////////////
	//color up to kmin-1
	while(1){
		if(col>=kmin) break;							//exit condition
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
	while(1){
		m_sel=m_unsel;
		m_sel.init_scan(bbo::DESTRUCTIVE);
		while(1){
			v=m_sel.next_bit_del(nBB,m_unsel);
			if(v==EMPTY_ELEM) break;						
			
			LISTA_L(depth).nodos[++LISTA_L(depth).index]=v;
			m_lcol[DEPTH_PLUS1][v]=col;			

			if((--pc)==0){ return ;}											
			m_sel.erase_block(nBB,g->get_neighbors(v));
		}
		col++;
	}
 }
 
void CliqueSat::paint_sat_R_sel(int depth){
//////////////////
// Infrachromatic filter + reocloring filter in selective coloring. 
// Applied to ALL vertices with label kmin 
// (recoloring if possible, comes first, to enlarge color subsets as much as possible before they are checked for inconsistency)
// 
// date: 04/09/15
// last_update: 04/09/15
//
// COMMENTS: attemtps to filter vertices with label kmin until first failure.
//			 To attempt to filter all vertices, active while loop (1) (CURRENT STATE ON--MAXIMUM EFFICIENCY)

	int col=1, kmin=maxno-depth, v=EMPTY_ELEM, nBB=EMPTY_ELEM;  
	LISTA_L(depth).index=EMPTY_ELEM;

	//copies list of vertices to color and stores size for fast empty check 
	int pc=(m_unsel=LISTA_BB(depth)).popcn64();
	//if(pc<kmin){return;}								//CUT based on populaton size (***possibly remove***)
				
	////////////////////////////
	//color up to kmin-1
	while(1){
		if(col>=kmin) break;							//exti condition
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


void CliqueSat::paint_sat_sel(int depth){
//////////////////
// Infrachromatic filter + reocloring filter in selective coloring. 
// Applied to ALL vertices with label kmin 
// (recoloring if possible, comes first, to enlarge color subsets as much as possible before they are checked for inconsistency)
// 
// date: 04/09/15
// last_update: 04/09/15
//
// COMMENTS: attemtps to filter vertices with label kmin until first failure.
//			 To attempt to filter all vertices, active while loop (1) (CURRENT STATE ON--MAXIMUM EFFICIENCY)
	
	int col=1, kmin=maxno-depth, v=EMPTY_ELEM, nBB=EMPTY_ELEM;  
	LISTA_L(depth).index=EMPTY_ELEM;
	
	//copies list of vertices to color and stores size for fast empty check 
	int pc=(m_unsel=LISTA_BB(depth)).popcn64();
	//if(pc<kmin){return;}								//CUT based on populaton size (***possibly remove***)

	////////////////////////////
	//color up to kmin-1
	while(1){
		if(col>=kmin) break;							//exti condition
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
// INFRACHROM FILTERING (col>=kmin always here)
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
						if( m_colsets[j].is_disjoint(g->get_neighbors(v),g->get_neighbors(vswap))){
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
						if( m_colsets[j].is_disjoint(g->get_neighbors(v),g->get_neighbors(vswap))){
							//updates inconsistent color set with (recol, j)
							if((--pc)==0){ return;}
							else{
								//LOG_INFO("SAT INCONSISTENT SUBSET FOUND");
								INFRA_CHROM(v,recol,j);
								goto next_v;
							}
						}
					}//end of search for new color set j>recol
				}
			}//next candidate for swap color seed
		}//next attemp at vertex pruning
	}
// END OF INFRACHROM FILTERING
///////////////////////////////////
	//add unfiltered vertices to candidate list and exit
	m_unsel.init_scan(bbo::DESTRUCTIVE);
	while(1){
		v=m_unsel.next_bit_del();
		if(v==EMPTY_ELEM)	return;
		
		//add to candidate list
		LISTA_L(depth).nodos[++LISTA_L(depth).index]=v;
	}
}

void CliqueSat::paint_sat_R_seq_sel_N_kmin(int depth){
//////////////////
// Infrachromatic filter + recoloring filter in selective coloring implemented more cleanly 
// WITH RECOL BELOW KMIN INCLUDED
// May be applied selectively to vertices with label kmin UNTIL first failure or NOT (check the current implementation)
// Recoloring comes first so that color sets are as large as possible before applying infra-chrom
//
// RESULTS: would seem to perform worse in random high dense structures than when not recoloring below kmin
// 
// date: 06/11/15
// last_update: 06/11/15

	int col=0/* important*/, kmin=maxno-depth, v=EMPTY_ELEM, nBB=EMPTY_ELEM;  
	LISTA_L(depth).index=EMPTY_ELEM;

	//copies list of vertices to color and stores size for fast empty check 
	int pc=(m_unsel=LISTA_BB(depth)).popcn64();
	if(pc<kmin){return;}							//CUT based on populaton size (***possibly remove***)
	
		
	////////////////////////////
	//color up to kmin-1
	while(1){
		col++;
		//exit condition
		if(col>=kmin) break;

		m_colsets[col]=m_unsel;
		m_colsets[col].init_scan(bbo::NON_DESTRUCTIVE);
		while(1){
next_v:		v=m_colsets[col].next_bit(nBB, m_unsel);
			if(v==EMPTY_ELEM){
				break;
			}

			/////////////////////////
			//recolor vertex up to (and including) col if possible
			if(col>2){
				for(int recol=1; recol<(col-1); recol++){		//loop to find initial color seed
				
					//check if color is valid for swapping (0-1 neighbors)	
					int vswap;
					int pc_swap=m_colsets[recol].single_disjoint(g->get_neighbors(v), vswap);

					//analysis
					if(pc_swap==1){	//color class found

						/////////////////////////////////////////////
						//searches for new color set j>recol
						for(int j=recol+1; j<col; j++){

							if( m_colsets[j].is_disjoint(g->get_neighbors(vswap)) ){

								//updates color subsets
								if((--pc)==0){ return;}
								else{
									//LOG_INFO("RECOLORING BELOW KMIN FOUND");
									m_unsel.erase_bit(v);
									m_colsets[j].set_bit(vswap);
									m_colsets[recol].set_bit(v);
									m_colsets[recol].erase_bit(vswap);
									goto next_v;
								}

							}
						}//end of search for new color set j>recol


						/////////////////////////////////////////////
						//searches for new color set j<recol
						for(int j=1; j<recol; j++){

							if( m_colsets[j].is_disjoint(g->get_neighbors(vswap)) ){

								//updates color subsets
								if((--pc)==0){ return;}
								else{
									//LOG_INFO("RECOLORING BELOW KMIN FOUND");
									m_unsel.erase_bit(v);
									m_colsets[j].set_bit(vswap);
									m_colsets[recol].set_bit(v);
									m_colsets[recol].erase_bit(vswap);
									goto next_v;
								}

							}
						}//end of search for new color set j<recol
	

					}else if(pc_swap==0){ 
						if((--pc)==0){ return;}
						else{
							//LOG_INFO("SIMPLE RECOLORING BELOW KMIN FOUND");
							m_unsel.erase_bit(v);
							m_colsets[recol].set_bit(v);
							goto next_v;
						}
					}
		
				}
			}

			//end recoloring
			//////////////////////////////

			if((--pc)==0){ return;}  //cut, kmin not reached
			
			
			//color as set difference
			m_colsets[col].erase_block(nBB,g->get_neighbors(v));
		}
	}

	///////////////////////////////////////////
	// FILTERING (here col>=kmin)
	if(kmin>=3){


		///////////////////////////////////////////
		// RECOLORING ATTEMPT
		m_unsel.init_scan(bbo::NON_DESTRUCTIVE);
		while(1){														//REMOVE LOOP to implement filter only up to first failure: when ON attempts all colors
next_recol: v=m_unsel.next_bit();
			if(v==EMPTY_ELEM) break;		//or RETURN

			//recolor v if possible
			for(int recol=1; recol<(kmin-1); recol++){		//loop to find initial color seed


				//check if color is valid for swapping (0-1 neighbors)	
				int vswap;
				int pc_swap=m_colsets[recol].single_disjoint(g->get_neighbors(v), vswap);

				//analysis
				if(pc_swap==1){	//color class found

					/////////////////////////////////////////////
					//searches for new color set j>recol
					for(int j=recol+1; j<kmin; j++){

						if( m_colsets[j].is_disjoint(g->get_neighbors(vswap)) ){

							//updates color subsets
							if((--pc)==0){ return;}
							else{
								//LOG_INFO("RECOLORING FOUND");
								m_unsel.erase_bit(v);
								m_colsets[j].set_bit(vswap);
								m_colsets[recol].set_bit(v);
								m_colsets[recol].erase_bit(vswap);
								goto next_recol;
							}

						}
					}//end of search for new color set j>recol


					/////////////////////////////////////////////
					//searches for new color set j<recol
					for(int j=1; j<recol; j++){

						if( m_colsets[j].is_disjoint(g->get_neighbors(vswap)) ){

							//updates color subsets
							if((--pc)==0){ return;}
							else{
								//LOG_INFO("RECOLORING ABOVE KMIN J<RECOL FOUND");
								m_unsel.erase_bit(v);
								m_colsets[j].set_bit(vswap);
								m_colsets[recol].set_bit(v);
								m_colsets[recol].erase_bit(vswap);
								goto next_recol;
							}

						}
					}//end of search for new color set j<recol

				}else if(pc_swap==0){ 
					if((--pc)==0){ return;}
					else{
						//LOG_INFO("SIMPLE RECOLORING FOUND");
						m_unsel.erase_bit(v);
						m_colsets[recol].set_bit(v);
						goto next_recol;
					}
				}

			}//next candidate for swap color seed
		}//next attemp at vertex pruning


		///////////////////////////////////////////
		// INFRA-CHROM ATTEMPT
		m_forbidden.erase_bit(1, kmin);	

		m_unsel.init_scan(bbo::NON_DESTRUCTIVE);
		while(1){														//REMOVE LOOP to implement filter only up to first failure: when ON attempts all colors
next_infra: v=m_unsel.next_bit();
			if(v==EMPTY_ELEM) break;		//or RETURN

			//recolor v if possible
			for(int recol=1; recol<(kmin-1); recol++){		//loop to find initial color seed

				if(m_forbidden.is_bit(recol)) continue;

				//check if color is valid for swapping (0-1 neighbors)	
				int vswap;
				int pc_swap=m_colsets[recol].single_disjoint(g->get_neighbors(v), vswap);

				//analysis
				if(pc_swap==1){	//color class found

					/////////////////////////////////////////////
					//searches for new color set j>recol
					for(int j=recol+1; j<kmin; j++){


						//filters out forbidden colors
						//if(m_forbidden.is_bit(j)) continue;

						if( m_colsets[j].is_disjoint(g->get_neighbors(v),g->get_neighbors(vswap))){

							//updates inconsistent color set with (recol, j)
							if((--pc)==0){ return;}
							else{
								//LOG_INFO("SAT INCONSISTENT SUBSET FOUND");
								m_unsel.erase_bit(v);
								m_forbidden.set_bit(j);
								m_forbidden.set_bit(recol);
								goto next_infra;
							}

						}
					}//end of search for new color set j>recol


					/////////////////////////////////////////////
					//searches for new color set j<recol
					for(int j=1; j<recol; j++){


						//filters out forbidden colors
						//if(m_forbidden.is_bit(j)) continue;

						if( m_colsets[j].is_disjoint(g->get_neighbors(v),g->get_neighbors(vswap))){

							//updates inconsistent color set with (recol, j)
							if((--pc)==0){ return;}
							else{
								//LOG_INFO("SAT INCONSISTENT SUBSET FOUND");
								m_unsel.erase_bit(v);
								m_forbidden.set_bit(j);
								m_forbidden.set_bit(recol);
								goto next_infra;
							}

						}
					}//end of search for new color set j>recol

				}

			}//next candidate for swap color seed
		}//next attemp at vertex pruning

	}
	
	//END FILTERING
	//////////////////////////////////////////
	
	//add unfiltered vertices to candidate list and exit
	m_unsel.init_scan(bbo::DESTRUCTIVE);
	while(1){
		v=m_unsel.next_bit_del();
		if(v==EMPTY_ELEM)	return;
		
		//add to candidate list
		LISTA_L(depth).nodos[++LISTA_L(depth).index]=v;
	}
}

void CliqueSat::paint_sat_R_sel_seq(int depth){
//////////////////
// Infrachromatic filter + recoloring filter in selective coloring implemented more cleanly 
// May be applied selectively to vertices with label kmin UNTIL first failure or NOT (check the current implementation)
// Recolorin comes first so that color sets are as large as possible before applying infra-chrom
// 
// date: 04/11/15
// last_update: 04/11/15

	int col=1, kmin=maxno-depth, v=EMPTY_ELEM, nBB=EMPTY_ELEM;  
	LISTA_L(depth).index=EMPTY_ELEM;

	//copies list of vertices to color and stores size for fast empty check 
	int pc=(m_unsel=LISTA_BB(depth)).popcn64();
	//if(pc<kmin){return;}							//CUT based on populaton size (***possibly remove***)
			
	////////////////////////////
	//color up to kmin-1
	while(1){
		//exit condition
		if(col>=kmin) break;
		m_colsets[col]=m_unsel;
		m_colsets[col].init_scan(bbo::NON_DESTRUCTIVE);
		while(1){
			v=m_colsets[col].next_bit(nBB, m_unsel);
			if(v==EMPTY_ELEM){
				break;
			}
			if((--pc)==0){ return;}  //cut, kmin not reached
			//color as set difference
			m_colsets[col].erase_block(nBB,g->get_neighbors(v));
		}
		col++;
	}
///////////////////////////////////////////
// FILTERING (here col>=kmin)
	if(kmin>=3){
		///////////////////////////////////////////
		// RECOLORING ATTEMPT
		m_unsel.init_scan(bbo::NON_DESTRUCTIVE);
		while(1){														//REMOVE LOOP to implement filter only up to first failure: when ON attempts all colors
next_recol: v=m_unsel.next_bit();
			if(v==EMPTY_ELEM) break;		//or RETURN

			//recolor v if possible
			for(int recol=1; recol<(kmin-1); recol++){		//loop to find initial color seed
				//check if color is valid for swapping (0-1 neighbors)	
				int vswap;
				int pc_swap=m_colsets[recol].single_disjoint(g->get_neighbors(v), vswap);
				if(pc_swap==1){	//color class found
					/////////////////////////////////////////////
					//searches for new color set j>recol
					for(int j=recol+1; j<kmin; j++){
						if( m_colsets[j].is_disjoint(g->get_neighbors(vswap)) ){
							//updates color subsets
							if((--pc)==0){ return;}
							else{
								//LOG_INFO("RECOLORING FOUND");
								RECOLOR_SWAP(v,vswap,recol,j);
								goto next_recol;
							}
						}
					}//end of search for new color set j>recol
					/////////////////////////////////////////////
					//searches for new color set j<recol
					for(int j=1; j<recol; j++){
						if( m_colsets[j].is_disjoint(g->get_neighbors(vswap)) ){
							//updates color subsets
							if((--pc)==0){ return;}
							else{
								//LOG_INFO("RECOLORING FOUND");
								RECOLOR_SWAP(v,vswap,recol,j);
								goto next_recol;
								}
						}
					}//end of search for new color set j<recol
				}else if(pc_swap==0){ 
					if((--pc)==0){ return;}
					else{
						//LOG_INFO("SIMPLE RECOLORING FOUND");
						RECOLOR_SIMPLE_SWAP(v,recol);
						goto next_recol;
					}
				}
			}//next candidate for swap color seed
		}//next attemp at vertex pruning
		///////////////////////////////////////////
		// INFRA-CHROM ATTEMPT
		m_forbidden.erase_bit(1, kmin);	
		m_unsel.init_scan(bbo::NON_DESTRUCTIVE);
		while(1){														//REMOVE LOOP to implement filter only up to first failure: when ON attempts all colors
next_infra: v=m_unsel.next_bit();
			if(v==EMPTY_ELEM) break;		//or RETURN
			//recolor v if possible
			for(int recol=1; recol<(kmin-1); recol++){		//loop to find initial color seed
				if(m_forbidden.is_bit(recol)) continue;
				//check if color is valid for swapping (0-1 neighbors)	
				int vswap;
				int pc_swap=m_colsets[recol].single_disjoint(g->get_neighbors(v), vswap);
				//analysis
				if(pc_swap==1){	//color class found
					/////////////////////////////////////////////
					//searches for new color set j>recol
					for(int j=recol+1; j<kmin; j++){
						//filters out forbidden colors
						if(m_forbidden.is_bit(j)) continue;
						if( m_colsets[j].is_disjoint(g->get_neighbors(v),g->get_neighbors(vswap))){
							//updates inconsistent color set with (recol, j)
							if((--pc)==0){ return;}
							else{
								//LOG_INFO("SAT INCONSISTENT SUBSET FOUND");
								INFRA_CHROM(v,recol,j);
								goto next_infra;
							}
						}
					}//end of search for new color set j>recol
					/////////////////////////////////////////////
					//searches for new color set j<recol
					for(int j=1; j<recol; j++){
						//filters out forbidden colors
						if(m_forbidden.is_bit(j)) continue;
						if( m_colsets[j].is_disjoint(g->get_neighbors(v),g->get_neighbors(vswap))){
							//updates inconsistent color set with (recol, j)
							if((--pc)==0){ return;}
							else{
								//LOG_INFO("SAT INCONSISTENT SUBSET FOUND");
								INFRA_CHROM(v,recol,j);
								goto next_infra;
							}
						}
					}//end of search for new color set j>recol
				}
			}//next candidate for swap color seed
		}//next attemp at vertex pruning
	}
//END FILTERING
//////////////////////////////////////////
	//add unfiltered vertices to candidate list and exit
	m_unsel.init_scan(bbo::DESTRUCTIVE);
	while(1){
		v=m_unsel.next_bit_del();
		if(v==EMPTY_ELEM)	return;
		
		//add to candidate list
		LISTA_L(depth).nodos[++LISTA_L(depth).index]=v;
	}
}

#endif
