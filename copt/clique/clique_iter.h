////////////////////////////////
// clique_iter.h: interface for CliqueIter class with the new reverse control flow
//				  when branching (aka russian dolls). Includes now infrachromatic+recoloring filter
//				  in the standard framework. 
//				  Seems specially well suited for non structured graphs (uniform random), because it can find good solutions quickly.	
//
// initial date: 22/05/15
// last update: 22/05/15
// author: pablo san segundo

#ifndef  __CLIQUE_ITER_H__
#define  __CLIQUE_ITER_H__

#include "clique.h"
#include "../init_color_ub.h"
#include "../amts/amts_exec.h"
#include "../common/common_macros.h"

class CliqueIter: public Clique<ugraph>{
	
public:
	enum return_t {ROOT_NODE=-1, CONTINUE_SEARCH_IN_BRANCH=0};
	CliqueIter(ugraph* g, param_t p)				:Clique<ugraph>(g, p){};
	CliqueIter(param_t p)							:Clique<ugraph>(p){};
	virtual ~CliqueIter(){}

	virtual int init_bitarrays();											

	int paint_iter(int depth);
	int paint_iter_sel(int depth);
	int paint_iter_sat(int depth);
	int paint_iter_sat_R(int depth);
	int paint_iter_sat_sel(int depth);
	int paint_iter_sat_R_sel(int depth);
	int paint_iter_sat_R_sel_seq(int depth);

	//for enlarge implementations
	int paint_iter_ROOT(bitarray& bbrem, int inc_sol);

	//tested search procedures
	return_t expand_iter_root (int maxac, bitarray& l_bb , nodelist_t& l_v);
	return_t expand_iter_root_sel (int maxac, bitarray& l_bb , nodelist_t& l_v);
	return_t expand_iter_root_sat (int maxac, bitarray& l_bb , nodelist_t& l_v);
	return_t expand_iter_root_sat_R (int maxac, bitarray& l_bb , nodelist_t& l_v);
	return_t expand_iter_root_sat_sel (int maxac, bitarray& l_bb , nodelist_t& l_v);
	return_t expand_iter_root_sat_R_sel (int maxac, bitarray& l_bb , nodelist_t& l_v);
	return_t expand_iter_root_sat_R_sel_seq (int maxac, bitarray& l_bb , nodelist_t& l_v);
	
	//attempts to find a new solution from remaining vertices at the root node
	return_t expand_iter_root_enlarge (int maxac, bitarray& l_bb , nodelist_t& l_v);
	return_t expand_iter_root_enlarge_sat_R (int maxac, bitarray& l_bb , nodelist_t& l_v);
	
		
	//test framework
	virtual	int set_up();
	virtual	void run();	
	virtual	void tear_down(){ Clique<ugraph>::tear_down();}

private:
	//auxiliary functions
	inline int incUB(bitarray&, int* labels);
	inline int incUB(bitarray&, int v, int* labels);

	int is_enlargeable(int cq_size, int* cq, int vref);
	int is_enlargeable(int cq_size, int* cq, const bitarray& bbcrem);

	//auxiliary data structures
	bitarray m_forbidden;
	bitarray m_remaining;
	bool is_enlarge_succesful;
};

int CliqueIter::init_bitarrays(){

	Clique<ugraph>::init_bitarrays();

	//empty set
	m_bbroot.erase_bit();
	m_forbidden.init(m_size);
	m_remaining.init(m_size);			
	m_remaining.set_bit(0, m_size-1);				//all remaining
	is_enlarge_succesful=false;
	return 0;
}

inline
int CliqueIter::set_up(){
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
	case BBMCIT:
	case BBMCIT_L:
	case BBMCITX:
	case BBMCITXR:
	case BBMCITX_L:
	case BBMCITXR_L:
	case BBMCITXR_L_SEQ:

	case BBMC_Iter_Root_Enlarge:
	case BBMC_Iter_Root_Enlarge_Sat_R:
		break;
	default:
		LOG_ERROR("CliqueIter::setup unknown algorithm");
		return -1;
	}
	
	//actual set_up
	if(param.unrolled){
		LOG_ERROR("CliqueIter::setup unrolled variant undefined");
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
	
	//solution found
	if(sol>0){
		LOG_INFO("[w="<<sol<<"]"<<" TRIVIALLY SOLVED DURING PRECOMPUTATION");
	}

	return sol;
}

inline
void CliqueIter::run(){
	//algorithm
	if(param.unrolled){
		LOG_ERROR("CliqueIter::run: unrolling not defined for this familiy of algorithms");
		return;
	}else{
		res.tic();
		switch(param.alg){
		case BBMCIT:
			expand_iter_root(0, Clique<ugraph>::m_bbroot, Clique<ugraph>::m_lroot);
		case BBMCIT_L:
			expand_iter_root_sel(0, Clique<ugraph>::m_bbroot, Clique<ugraph>::m_lroot);
			break;
		case BBMCITX:
			expand_iter_root_sat(0, Clique<ugraph>::m_bbroot, Clique<ugraph>::m_lroot);
			break;
		case BBMCITX_L:
			expand_iter_root_sat_sel(0, Clique<ugraph>::m_bbroot, Clique<ugraph>::m_lroot);
			break;
		case BBMCITXR_L:
			expand_iter_root_sat_R_sel(0, Clique<ugraph>::m_bbroot, Clique<ugraph>::m_lroot);
			break;
		case BBMCITXR_L_SEQ:
			expand_iter_root_sat_R_sel_seq(0, Clique<ugraph>::m_bbroot, Clique<ugraph>::m_lroot);
			break;
		case BBMCITXR:
			expand_iter_root_sat_R(0, Clique<ugraph>::m_bbroot, Clique<ugraph>::m_lroot);
			break;
		case BBMC_Iter_Root_Enlarge:
			expand_iter_root_enlarge(0, Clique<ugraph>::m_bbroot, Clique<ugraph>::m_lroot);
			break;
		case BBMC_Iter_Root_Enlarge_Sat_R:
			expand_iter_root_enlarge_sat_R(0, Clique<ugraph>::m_bbroot, Clique<ugraph>::m_lroot);
			break;
		default:
			LOG_ERROR("CliqueIter::run-non_unrolled:unknown clique algorithm");
		}

		res.toc();
	}

	//read solution
	res.set_UB(maxno);
	LOG_INFO("[w:"<<res.get_upper_bound()<<","<<res.get_user_time()<<"s]");
}	

inline
CliqueIter::return_t CliqueIter::expand_iter_root (int maxac, bitarray& l_bb , nodelist_t& l_v){
//////////////////////
//	initial date: 28/08/15
//	author: pss
//	New recursive search procedure with specific bound analysis at the root node. Sepecifically:
//  1-Evaluation of incUB in all nodes
//  2-Update of colors of level k when branching 
//  3-Update of colors of pruned vertices 
//	4-Use Smax at the root node as possible bound  (not done)
//	5-When new solution found return to ROOT_LEVEL
//
// An alternative configuration is that bound fixes occurr only at the root node

	int v; int col;
	res.inc_number_of_steps();

//main loop
	int vIndex=0;
	while(vIndex<=l_v.index){
			
		//Estrategias
		v=l_v.nodos[vIndex++];

#ifdef ROOT_VERTEX_PROGRESS
		if(maxac==0)
			cout<<"root vertex: "<<v<<endl;
#endif		
		//CUT by color (since [Konc & Janecic, 2007] this is of limited use (only for first branch))
		if( (m_lcol[maxac][v]+maxac)<=maxno ){
			l_bb.set_bit(v);
			continue;
		}

/////////////////////////////////
// CHILD NODE GENERATION
		
		//Node generation by masking
		AND(g->get_neighbors(v), l_bb, LISTA_BB(maxac));					//optimized when the second argument is the bitset with higher population
		
/////////////////////
//CUT based on previous color labels
		if(maxac==0){
			col=incUB(LISTA_BB(maxac),v, m_lcol[maxac]);
			if(m_lcol[maxac][v]>col+1){
				//updates colors with new lower bound
				m_lcol[maxac][v]=col+1;
				//checks if pruning is possible
				if( (m_lcol[maxac][v]+maxac)<=maxno){
					l_bb.set_bit(v);
					cout<<"incUB-CUT:"<<v<<":"<<maxac<<endl;
					continue;									//next vertex after v
				}
			}
			//upper bound can never be greater than Smax (at the moment does not make a difference)
			m_lcol[maxac][v]=min<int>(m_lcol[maxac][v],maxno);
		}
////////////////////////////

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

				//update color at root node with new solution and return to root node
				m_lcol[0][m_path[0]]=maxno;
				return ROOT_NODE;
			}
		
		l_bb.set_bit(v);
		continue;
		}
		
		//approx. coloring (generates child list of nodes in LISTA_L(maxac))
		col=paint_iter(maxac);
		
		//cuts if there are no child nodes of v
		if(LISTA_L(maxac).index<0){
			//update color threshold
			if(maxac==0){
				if(m_lcol[maxac][v]>col+1){
					m_lcol[maxac][v]=col+1;

				}
			}
			l_bb.set_bit(v); 
			continue;		
		}


		//root update of colors
		if(maxac==0){
			if(m_lcol[maxac][v]>col+1){
					m_lcol[maxac][v]=col+1;
			}
		}

		
		
				
///////////////////////////////////////////////////////
// CANDIDATE EXPANSION

		//sets path
		m_path[maxac]=v;
				
		//Generacion de nuevos nodos
		if(expand_iter_root(maxac+1,LISTA_BB(maxac),LISTA_L(maxac))==ROOT_NODE ){
			if(maxac!=0)
				return ROOT_NODE;
		}

		//////////////////////////////////////////////
		// BACKTRACK: adds the examined vertex
		
		l_bb.set_bit(v);
	}// next node

return CONTINUE_SEARCH_IN_BRANCH;
}

inline
CliqueIter::return_t CliqueIter::expand_iter_root_enlarge (int maxac, bitarray& l_bb , nodelist_t& l_v){
//////////////////////
//	initial date: 28/08/15
//	author: pss
//	New recursive search procedure with specific bound analysis at the root node. Sepecifically:
//  1-Evaluation of incUB in all nodes
//  2-Update of colors of level k when branching 
//  3-Update of colors of pruned vertices 
//	4-Use Smax at the root node as possible bound  (not done)
//	5-When new solution found return to ROOT_LEVEL
//
// An alternative configuration is that bound fixes occurr only at the root node

	int v; int col;
	res.inc_number_of_steps();

//main loop
	int vIndex=0;
	while(vIndex<=l_v.index){
			
		//Estrategias
		v=l_v.nodos[vIndex++];
		if(maxac==0){
			m_remaining.erase_bit(v);				//removes from remaining set
		}
				
		//CUT by color (since [Konc & Janecic, 2007] this is of limited use (only for first branch))
		if( (m_lcol[maxac][v]+maxac)<=maxno ){
			l_bb.set_bit(v);
			continue;
		}

/////////////////////////////////
// CHILD NODE GENERATION
		
		//Node generation by masking
		AND(g->get_neighbors(v), l_bb, LISTA_BB(maxac));		//optimized when the second argument is the bitset with higher population
		
/////////////////////
//CUT based on previous color labels
		if(maxac==0){
			col=incUB(LISTA_BB(maxac),v, m_lcol[maxac]);
			if(m_lcol[maxac][v]>col+1){
				//updates colors with new lower bound
				m_lcol[maxac][v]=col+1;
				//checks if pruning is possible
				if( (m_lcol[maxac][v]+maxac)<=maxno){
					l_bb.set_bit(v);
					cout<<"incUB-CUT:"<<v<<":"<<maxac<<endl;
					continue;									//next vertex after v
				}
			}
			//upper bound can never be greater than Smax (at the moment does not make a difference)
			m_lcol[maxac][v]=min<int>(m_lcol[maxac][v],maxno);
		}
////////////////////////////

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


				//show if it can be enlarged at ROOT NODE
				int inc=is_enlargeable(maxno, m_path, m_remaining);
				if(inc>0){
					maxno+=inc;											//must be updated before the call to paint_iter_ROOT
					stringstream sstr("");
					sstr<<"INCREMENTING SOLUTION BY "<<inc<<" VERTICES";
					LOG_INFO(sstr.str());
					if(paint_iter_ROOT(m_remaining, inc)==EMPTY_ELEM){
						LOG_INFO("TRIVIALLY SOLVED IN  paint_iter_ROOT*****************");
						//**EXIT SEARCH-APPARENTLY DOES NOT HAPPEN IN PRACTICE***
					}
					is_enlarge_succesful=true;		//control flag for backtracking 
				}

				//update color at root node with new solution and return to root node
				m_lcol[0][m_path[0]]=maxno;
				return ROOT_NODE;
			}
		
		l_bb.set_bit(v);
		continue;
		}
		
		//approx. coloring (generates child list of nodes in LISTA_L(maxac))
		col=paint_iter(maxac);
		
		//cuts if there are no child nodes of v
		if(LISTA_L(maxac).index<0){
			//update color threshold
			if(maxac==0){
				if(m_lcol[maxac][v]>col+1){
					m_lcol[maxac][v]=col+1;

				}
			}
			l_bb.set_bit(v); 
			continue;		
		}


		//root update of colors
		if(maxac==0){
			if(m_lcol[maxac][v]>col+1){
					m_lcol[maxac][v]=col+1;
			}
		}
		
				
///////////////////////////////////////////////////////
// CANDIDATE EXPANSION

		//sets path
		m_path[maxac]=v;
				
		//Generacion de nuevos nodos
		if(expand_iter_root_enlarge(maxac+1,LISTA_BB(maxac),LISTA_L(maxac))==ROOT_NODE ){
			if(maxac!=0)
				return ROOT_NODE;
		}

		//////////////////////////////////////////////
		// BACKTRACK: adds the examined vertex
		
		l_bb.set_bit(v);

		if(is_enlarge_succesful){
			vIndex=0;
			is_enlarge_succesful=false;
		}
	}// next node

return CONTINUE_SEARCH_IN_BRANCH;
}






inline
CliqueIter::return_t CliqueIter::expand_iter_root_sat (int maxac, bitarray& l_bb , nodelist_t& l_v){
//////////////////////
//	initial date: 28/08/15
//	author: pss
//	New recursive search procedure with specific bound analysis at the root node. Sepecifically:
//  1-Evaluation of incUB in all nodes
//  2-Update of colors of level k when branching 
//  3-Update of colors of pruned vertices 
//	4-Use Smax at the root node as possible bound  (not done)
//	5-When new solution found return to ROOT_LEVEL
//
// An alternative configuration is that bound fixes occurr only at the root node

	int v; int col;
	res.inc_number_of_steps();

//main loop
	int vIndex=0;
	while(vIndex<=l_v.index){
			
		//Estrategias
		v=l_v.nodos[vIndex++];

#ifdef ROOT_VERTEX_PROGRESS
		if(maxac==0)
			cout<<"root vertex: "<<v<<endl;
#endif		
		//CUT by color (since [Konc & Janecic, 2007] this is of limited use (only for first branch))
		if( (m_lcol[maxac][v]+maxac)<=maxno ){
			l_bb.set_bit(v);
			continue;
		}

/////////////////////////////////
// CHILD NODE GENERATION
		
		//Node generation by masking
		AND(g->get_neighbors(v), l_bb, LISTA_BB(maxac));		//optimized when the second argument is the bitset with higher population
		
/////////////////////
//CUT based on previous color labels
		if(maxac==0){
			col=incUB(LISTA_BB(maxac),v, m_lcol[maxac]);
			if(m_lcol[maxac][v]>col+1){
				//updates colors with new lower bound
				m_lcol[maxac][v]=col+1;
				//checks if pruning is possible
				if( (m_lcol[maxac][v]+maxac)<=maxno){
					l_bb.set_bit(v);
					cout<<"incUB-CUT:"<<maxac<<endl;
					continue;								//next vertex after v
				}
			}
		}
////////////////////////////

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

				//update color at root node and return to root node
				m_lcol[0][m_path[0]]=maxno;
				return ROOT_NODE;
			}
		
		l_bb.set_bit(v);
		continue;
		}
		
		//approx. coloring (generates child list of nodes in LISTA_L(maxac))
		col=paint_iter_sat(maxac);
		
		//cuts if there are no child nodes of v
		if(LISTA_L(maxac).index<0){
			//update color threshold
			if(maxac==0){
				if(m_lcol[maxac][v]>col+1){
					m_lcol[maxac][v]=col+1;

				}
			}
			l_bb.set_bit(v); 
			continue;		
		}


		//root update of colors
		if(maxac==0){
			if(m_lcol[maxac][v]>col+1){
					m_lcol[maxac][v]=col+1;
			}
		}
		
				
///////////////////////////////////////////////////////
// CANDIDATE EXPANSION

		//sets path
		m_path[maxac]=v;
				
		//Generacion de nuevos nodos
		if(expand_iter_root_sat(maxac+1,LISTA_BB(maxac),LISTA_L(maxac))==ROOT_NODE ){
			if(maxac!=0)
				return ROOT_NODE;
		}

		//////////////////////////////////////////////
		// BACKTRACK: adds the examined vertex
		
		l_bb.set_bit(v);
	}// next node

return CONTINUE_SEARCH_IN_BRANCH;
}

inline
CliqueIter::return_t CliqueIter::expand_iter_root_sat_R_sel (int maxac, bitarray& l_bb , nodelist_t& l_v){
//////////////////////
//	initial date: 28/08/15
//	author: pss
//	New recursive search procedure with specific bound analysis at the root node. Sepecifically:
//  1-Evaluation of incUB in all nodes
//  2-Update of colors of level k when branching 
//  3-Update of colors of pruned vertices 
//	4-Use Smax at the root node as possible bound  (not done)
//	5-When new solution found return to ROOT_LEVEL
//
// An alternative configuration is that bound fixes occurr only at the root node

	int v; int col;
	res.inc_number_of_steps();

//main loop
	int vIndex=0;
	while(vIndex<=l_v.index){
			
		//Estrategias
		v=l_v.nodos[vIndex++];

#ifdef STRONG_ROOT_COLORING
		if(maxac==0){
			if((m_lcol[maxac][v]+maxac)<=maxno ){
				l_bb.set_bit(v);
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
		AND(g->get_neighbors(v), l_bb, LISTA_BB(maxac));		
		
/////////////////////
//CUT based on previous color labels
		//if(maxac==0){
		//	col=incUB(LISTA_BB(maxac),v, m_lcol[maxac]);
		//	if(m_lcol[maxac][v]>col+1){
		//		//updates colors with new lower bound
		//		m_lcol[maxac][v]=col+1;
		//		//checks if pruning is possible
		//		if( (m_lcol[maxac][v]+maxac)<=maxno){
		//			l_bb.set_bit(v);
		//			cout<<"incUB-CUT:"<<maxac<<endl;
		//			continue;								//next vertex after v
		//		}
		//	}
		//}
////////////////////////////

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

				//update color at root node and return to root node
				m_lcol[0][m_path[0]]=maxno;
				return ROOT_NODE;
			}
		
		l_bb.set_bit(v);
		continue;
		}
		
		//approx. coloring (generates child list of nodes in LISTA_L(maxac))
		col=paint_iter_sat_R_sel(maxac);
	
			
		//cuts if there are no child nodes of v
		if(LISTA_L(maxac).index<0){
			//update color threshold
		/*	if(maxac==0){
				if(m_lcol[maxac][v]>col+1){
					m_lcol[maxac][v]=col+1;

				}
			}*/
			l_bb.set_bit(v); 
			continue;		
		}


		//root update of colors
		/*if(maxac==0){
			if(m_lcol[maxac][v]>col+1){
					m_lcol[maxac][v]=col+1;
			}
		}*/
		
				
///////////////////////////////////////////////////////
// CANDIDATE EXPANSION

		//sets path
		m_path[maxac]=v;
				
		//Generacion de nuevos nodos
		if(expand_iter_root_sat_R_sel(maxac+1,LISTA_BB(maxac),LISTA_L(maxac))==ROOT_NODE ){
			if(maxac!=0)
				return ROOT_NODE;
		}

		//////////////////////////////////////////////
		// BACKTRACK: adds the examined vertex
		
		l_bb.set_bit(v);
	}// next node

return CONTINUE_SEARCH_IN_BRANCH;
}


inline
CliqueIter::return_t CliqueIter::expand_iter_root_sat_sel (int maxac, bitarray& l_bb , nodelist_t& l_v){
//////////////////////
//	initial date: 28/08/15
//	author: pss
//	New recursive search procedure with specific bound analysis at the root node. Sepecifically:
//  1-Evaluation of incUB in all nodes
//  2-Update of colors of level k when branching 
//  3-Update of colors of pruned vertices 
//	4-Use Smax at the root node as possible bound  (not done)
//	5-When new solution found return to ROOT_LEVEL
//
// An alternative configuration is that bound fixes occurr only at the root node

	int v; int col;
	res.inc_number_of_steps();

//main loop
	int vIndex=0;
	while(vIndex<=l_v.index){
			
		//Estrategias
		v=l_v.nodos[vIndex++];

#ifdef STRONG_ROOT_COLORING
		if(maxac==0){
			if((m_lcol[maxac][v]+maxac)<=maxno ){
				l_bb.set_bit(v);
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
		AND(g->get_neighbors(v), l_bb, LISTA_BB(maxac));
		
		
/////////////////////
//CUT based on previous color labels
		//if(maxac==0){
		//	col=incUB(LISTA_BB(maxac),v, m_lcol[maxac]);
		//	if(m_lcol[maxac][v]>col+1){
		//		//updates colors with new lower bound
		//		m_lcol[maxac][v]=col+1;
		//		//checks if pruning is possible
		//		if( (m_lcol[maxac][v]+maxac)<=maxno){
		//			l_bb.set_bit(v);
		//			cout<<"incUB-CUT:"<<maxac<<endl;
		//			continue;								//next vertex after v
		//		}
		//	}
		//}
////////////////////////////

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

				//update color at root node and return to root node
				m_lcol[0][m_path[0]]=maxno;
				return ROOT_NODE;
			}
		
		l_bb.set_bit(v);
		continue;
		}
		
		//approx. coloring (generates child list of nodes in LISTA_L(maxac))
		col=paint_iter_sat_sel(maxac);
	
			
		//cuts if there are no child nodes of v
		if(LISTA_L(maxac).index<0){
			//update color threshold
		/*	if(maxac==0){
				if(m_lcol[maxac][v]>col+1){
					m_lcol[maxac][v]=col+1;

				}
			}*/
			l_bb.set_bit(v); 
			continue;		
		}


		//root update of colors
		/*if(maxac==0){
			if(m_lcol[maxac][v]>col+1){
					m_lcol[maxac][v]=col+1;
			}
		}*/
		
				
///////////////////////////////////////////////////////
// CANDIDATE EXPANSION

		//sets path
		m_path[maxac]=v;
				
		//Generacion de nuevos nodos
		if(expand_iter_root_sat_sel(maxac+1,LISTA_BB(maxac),LISTA_L(maxac))==ROOT_NODE ){
			if(maxac!=0)
				return ROOT_NODE;
		}

		//////////////////////////////////////////////
		// BACKTRACK: adds the examined vertex
		
		l_bb.set_bit(v);
	}// next node

return CONTINUE_SEARCH_IN_BRANCH;
}


inline
CliqueIter::return_t CliqueIter::expand_iter_root_sel (int maxac, bitarray& l_bb , nodelist_t& l_v){
//////////////////////
//	initial date: 28/08/15
//	author: pss
//	New recursive search procedure with specific bound analysis at the root node. Sepecifically:
//  1-Evaluation of incUB in all nodes
//  2-Update of colors of level k when branching 
//  3-Update of colors of pruned vertices 
//	4-Use Smax at the root node as possible bound  (not done)
//	5-When new solution found return to ROOT_LEVEL
//
// An alternative configuration is that bound fixes occurr only at the root node

	int v; int col;
	res.inc_number_of_steps();

//main loop
	int vIndex=0;
	while(vIndex<=l_v.index){
			
		//Estrategias
		v=l_v.nodos[vIndex++];

#ifdef STRONG_ROOT_COLORING
		if(maxac==0){
			if((m_lcol[maxac][v]+maxac)<=maxno ){
				l_bb.set_bit(v);
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
		AND(g->get_neighbors(v), l_bb, LISTA_BB(maxac));
		LISTA_BB(maxac).print();
		
/////////////////////
//CUT based on previous color labels
		//if(maxac==0){
		//	col=incUB(LISTA_BB(maxac),v, m_lcol[maxac]);
		//	if(m_lcol[maxac][v]>col+1){
		//		//updates colors with new lower bound
		//		m_lcol[maxac][v]=col+1;
		//		//checks if pruning is possible
		//		if( (m_lcol[maxac][v]+maxac)<=maxno){
		//			l_bb.set_bit(v);
		//			cout<<"incUB-CUT:"<<maxac<<endl;
		//			continue;								//next vertex after v
		//		}
		//	}
		//}
////////////////////////////

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

				//update color at root node and return to root node
				m_lcol[0][m_path[0]]=maxno;
				return ROOT_NODE;
			}
		
		l_bb.set_bit(v);
		continue;
		}
		
		//approx. coloring (generates child list of nodes in LISTA_L(maxac))
		col=paint_iter_sel(maxac);
	
			
		//cuts if there are no child nodes of v
		if(LISTA_L(maxac).index<0){
			//update color threshold
		/*	if(maxac==0){
				if(m_lcol[maxac][v]>col+1){
					m_lcol[maxac][v]=col+1;

				}
			}*/
			l_bb.set_bit(v); 
			continue;		
		}


		//root update of colors
		/*if(maxac==0){
			if(m_lcol[maxac][v]>col+1){
					m_lcol[maxac][v]=col+1;
			}
		}*/
		
				
///////////////////////////////////////////////////////
// CANDIDATE EXPANSION

		//sets path
		m_path[maxac]=v;
				
		//Generacion de nuevos nodos
		if(expand_iter_root_sel(maxac+1,LISTA_BB(maxac),LISTA_L(maxac))==ROOT_NODE ){
			if(maxac!=0)
				return ROOT_NODE;
		}

		//////////////////////////////////////////////
		// BACKTRACK: adds the examined vertex
		
		l_bb.set_bit(v);
	}// next node

return CONTINUE_SEARCH_IN_BRANCH;
}

inline
CliqueIter::return_t CliqueIter::expand_iter_root_sat_R_sel_seq (int maxac, bitarray& l_bb , nodelist_t& l_v){
//////////////////////
//	initial date: 28/08/15
//	author: pss
//	New recursive search procedure with specific bound analysis at the root node. Sepecifically:
//  1-Evaluation of incUB in all nodes
//  2-Update of colors of level k when branching 
//  3-Update of colors of pruned vertices 
//	4-Use Smax at the root node as possible bound  (not done)
//	5-When new solution found return to ROOT_LEVEL
//
// An alternative configuration is that bound fixes occurr only at the root node

	int v; int col;
	res.inc_number_of_steps();

//main loop
	int vIndex=0;
	while(vIndex<=l_v.index){
			
		//Estrategias
		v=l_v.nodos[vIndex++];

#ifdef STRONG_ROOT_COLORING
		if(maxac==0){
			if((m_lcol[maxac][v]+maxac)<=maxno ){
				l_bb.set_bit(v);
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
		AND(g->get_neighbors(v), l_bb, LISTA_BB(maxac));		//optimized when the second argument is the bitset with higher population
		
/////////////////////
//CUT based on previous color labels
		//if(maxac==0){
		//	col=incUB(LISTA_BB(maxac),v, m_lcol[maxac]);
		//	if(m_lcol[maxac][v]>col+1){
		//		//updates colors with new lower bound
		//		m_lcol[maxac][v]=col+1;
		//		//checks if pruning is possible
		//		if( (m_lcol[maxac][v]+maxac)<=maxno){
		//			l_bb.set_bit(v);
		//			cout<<"incUB-CUT:"<<maxac<<endl;
		//			continue;								//next vertex after v
		//		}
		//	}
		//}
////////////////////////////

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

				//update color at root node and return to root node
				m_lcol[0][m_path[0]]=maxno;
				return ROOT_NODE;
			}
		
		l_bb.set_bit(v);
		continue;
		}
		
		//approx. coloring (generates child list of nodes in LISTA_L(maxac))
		col=paint_iter_sat_R_sel_seq(maxac);
		
		
		//cuts if there are no child nodes of v
		if(LISTA_L(maxac).index<0){
			//update color threshold
			/*if(maxac==0){
				if(m_lcol[maxac][v]>col+1){
					m_lcol[maxac][v]=col+1;

				}
			}*/
			l_bb.set_bit(v); 
			continue;		
		}


		//root update of colors
		/*if(maxac==0){
			if(m_lcol[maxac][v]>col+1){
					m_lcol[maxac][v]=col+1;
			}
		}*/
		
				
///////////////////////////////////////////////////////
// CANDIDATE EXPANSION

		//sets path
		m_path[maxac]=v;
				
		//Generacion de nuevos nodos
		if(expand_iter_root_sat_R_sel_seq(maxac+1,LISTA_BB(maxac),LISTA_L(maxac))==ROOT_NODE ){
			if(maxac!=0)
				return ROOT_NODE;
		}

		//////////////////////////////////////////////
		// BACKTRACK: adds the examined vertex
		
		l_bb.set_bit(v);
	}// next node

return CONTINUE_SEARCH_IN_BRANCH;
}


inline
CliqueIter::return_t CliqueIter::expand_iter_root_sat_R (int maxac, bitarray& l_bb , nodelist_t& l_v){
//////////////////////
//	initial date: 28/08/15
//	author: pss
//	New recursive search procedure with specific bound analysis at the root node. Sepecifically:
//  1-Evaluation of incUB in all nodes
//  2-Update of colors of level k when branching 
//  3-Update of colors of pruned vertices 
//	4-Use Smax at the root node as possible bound  (not done)
//	5-When new solution found return to ROOT_LEVEL
//
// An alternative configuration is that bound fixes occurr only at the root node

	int v; int col;
	res.inc_number_of_steps();

//main loop
	int vIndex=0;
	while(vIndex<=l_v.index){
			
		//Estrategias
		v=l_v.nodos[vIndex++];

#ifdef ROOT_VERTEX_PROGRESS
		if(maxac==0)
			cout<<"root vertex: "<<v<<endl;
#endif		
		//CUT by color (since [Konc & Janecic, 2007] this is of limited use (only for first branch))
		if( (m_lcol[maxac][v]+maxac)<=maxno ){
			l_bb.set_bit(v);
			continue;
		}

/////////////////////////////////
// CHILD NODE GENERATION
		
		//Node generation by masking
		AND(g->get_neighbors(v), l_bb, LISTA_BB(maxac));		//optimized when the second argument is the bitset with higher population
		
/////////////////////
//CUT based on previous color labels
		//if(maxac==0){
		//	col=incUB(LISTA_BB(maxac),v, m_lcol[maxac]);
		//	if(m_lcol[maxac][v]>col+1){
		//		//updates colors with new lower bound
		//		m_lcol[maxac][v]=col+1;
		//		//checks if pruning is possible
		//		if( (m_lcol[maxac][v]+maxac)<=maxno){
		//			l_bb.set_bit(v);
		//			cout<<"incUB-CUT:"<<v<<":"<<maxac<<endl;
		//			continue;								//next vertex after v
		//		}
		//	}
		//	//upper bound can never be greater than Smax (at the moment does not make a difference)
		//	m_lcol[maxac][v]=min<int>(m_lcol[maxac][v],maxno);
		//}
////////////////////////////

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

				//update color at root node and return to root node
				m_lcol[0][m_path[0]]=maxno;
				return ROOT_NODE;
			}
		
		l_bb.set_bit(v);
		continue;
		}
		
		//approx. coloring (generates child list of nodes in LISTA_L(maxac))
		col=paint_iter_sat_R(maxac);
		
		//cuts if there are no child nodes of v
		if(LISTA_L(maxac).index<0){
			//update color threshold
			/*if(maxac==0){
				if(m_lcol[maxac][v]>col+1){
					m_lcol[maxac][v]=col+1;

				}
			}*/
			l_bb.set_bit(v); 
			continue;		
		}

		//root update of colors
		/*if(maxac==0){
			if(m_lcol[maxac][v]>col+1){
					m_lcol[maxac][v]=col+1;
			}
		}*/
		
				
///////////////////////////////////////////////////////
// CANDIDATE EXPANSION

		//sets path
		m_path[maxac]=v;
				
		//Generacion de nuevos nodos
		if(expand_iter_root_sat_R(maxac+1,LISTA_BB(maxac),LISTA_L(maxac))==ROOT_NODE ){
			if(maxac!=0)
				return ROOT_NODE;
		}

		//////////////////////////////////////////////
		// BACKTRACK: adds the examined vertex
		
		l_bb.set_bit(v);
	}// next node

return CONTINUE_SEARCH_IN_BRANCH;
}



inline
CliqueIter::return_t CliqueIter::expand_iter_root_enlarge_sat_R (int maxac, bitarray& l_bb , nodelist_t& l_v){
//////////////////////
//	initial date: 28/08/15
//	author: pss
//	New recursive search procedure with specific bound analysis at the root node. Specifically:
//  1-Evaluation of incUB in all nodes
//  2-Update of colors of level k when branching 
//  3-Update of colors of pruned vertices 
//	4-Use Smax at the root node as possible bound  (not done)
//	5-When new solution found return to ROOT_LEVEL
//
// An alternative configuration is that bound fixes occurr only at the root node

	int v; int col;
	res.inc_number_of_steps();

//main loop
	int vIndex=0;
	while(vIndex<=l_v.index){
			
		//Estrategias
		v=l_v.nodos[vIndex++];
		if(maxac==0){
			m_remaining.erase_bit(v);				//removes from remaining set
#ifdef ROOT_VERTEX_PROGRESS
			cout<<"root vertex: "<<v<<endl;
#endif
		}
				
		//CUT by color (since [Konc & Janecic, 2007] this is of limited use (only for first branch))
		if( (m_lcol[maxac][v]+maxac)<=maxno ){
			l_bb.set_bit(v);
			continue;
		}

/////////////////////////////////
// CHILD NODE GENERATION
		
		//Node generation by masking
		AND(g->get_neighbors(v), l_bb, LISTA_BB(maxac));		//optimized when the second argument is the bitset with higher population
		
/////////////////////
//CUT based on previous color labels
		if(maxac==0){
			col=incUB(LISTA_BB(maxac),v, m_lcol[maxac]);
			if(m_lcol[maxac][v]>col+1){
				//updates colors with new lower bound
				m_lcol[maxac][v]=col+1;
				//checks if pruning is possible
				if( (m_lcol[maxac][v]+maxac)<=maxno){
					l_bb.set_bit(v);
					cout<<"incUB-CUT:"<<v<<":"<<maxac<<endl;
					continue;									//next vertex after v
				}
			}
			//upper bound can never be greater than Smax (at the moment does not make a difference)
			m_lcol[maxac][v]=min<int>(m_lcol[maxac][v],maxno);
		}
////////////////////////////

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

				//show if it can be enlarged at ROOT NODE
				int inc=is_enlargeable(maxno, m_path, m_remaining);
				if(inc>0){
					maxno+=inc;											//must be updated before the call to paint_iter_ROOT
					stringstream sstr("");
					sstr<<"INCREMENTING SOLUTION BY "<<inc<<" VERTICES";
					LOG_INFO(sstr.str());
					if(paint_iter_ROOT(m_remaining, inc)==EMPTY_ELEM){
						LOG_INFO("TRIVIALLY SOLVED IN  paint_iter_ROOT*****************");
						//**EXIT SEARCH-APPARENTLY DOES NOT HAPPEN IN PRACTICE***
					}
					is_enlarge_succesful=true;		//control flag for backtracking 
				}

				//update color at root node and return to root node
				m_lcol[0][m_path[0]]=maxno;
				return ROOT_NODE;
			}
		
		l_bb.set_bit(v);
		continue;
		}
		
		//approx. coloring (generates child list of nodes in LISTA_L(maxac))
		col=paint_iter_sat_R(maxac);
		
		
		//cuts if there are no child nodes of v
		if(LISTA_L(maxac).index<0){
			//update color threshold
			if(maxac==0){
				if(m_lcol[maxac][v]>col+1){
					m_lcol[maxac][v]=col+1;

				}
			}
			l_bb.set_bit(v); 
			continue;		
		}


		//root update of colors
		if(maxac==0){
			if(m_lcol[maxac][v]>col+1){
					m_lcol[maxac][v]=col+1;
			}
		}
		
				
///////////////////////////////////////////////////////
// CANDIDATE EXPANSION

		//sets path
		m_path[maxac]=v;
				
		//Generacion de nuevos nodos
		if(expand_iter_root_enlarge_sat_R(maxac+1,LISTA_BB(maxac),LISTA_L(maxac))==ROOT_NODE ){
			if(maxac!=0)
				return ROOT_NODE;
		}

		//////////////////////////////////////////////
		// BACKTRACK: adds the examined vertex
		
		l_bb.set_bit(v);

		
		if(is_enlarge_succesful){
			vIndex=0;
			is_enlarge_succesful=false;
		}
	}// next node

return CONTINUE_SEARCH_IN_BRANCH;
}

inline
int CliqueIter::paint_iter(int depth){
///////////////////
// Sequential greedy independent set vertex coloring which prunes the search tree
// Compared to classical paint, here candidate vertices are deleted from the bitset to be painted (LISTA_BB(depth))
//
// RETURNS: number of colors(>=1) or EMPTY_ELEM (-1) if error


	int col=1; int kmin=maxno-depth; int nBB=EMPTY_ELEM; int v=EMPTY_ELEM;		
	LISTA_L(depth).index=EMPTY_ELEM;												//cleans the set fo candidate vertices
	const int DEPTH_PLUS1=depth+1;
	
	//copies list of vertices to color and stores size for fast empty check 
	int pc= (m_unsel=LISTA_BB(depth)).popcn64();
		
	while(true){ 
		m_sel=m_unsel;
		m_sel.init_scan(bbo::DESTRUCTIVE);
		while(true){
			v=m_sel.next_bit_del(nBB,m_unsel);
			if(v==EMPTY_ELEM)
							break;
			if(col>=kmin){  
				LISTA_L(depth).nodos[++LISTA_L(depth).index]=v;
				LISTA_BB(depth).erase_bit(v);			        //erases candidate vertices, critical
			}

			//add label always
			m_lcol[DEPTH_PLUS1][v]=col;	
					

			if((--pc)==0){
				 return col;
			}
			m_sel.erase_block(nBB,g->get_neighbors(v));
		}				
	col++;
	}

return EMPTY_ELEM;				//should not reach here
 }

inline
int CliqueIter::paint_iter_sel(int depth){
/////////////////
// selective pruning (up to kmin) 
// Added to try to understand the new iterative oriented flow (23/6/16)

int col=1, kmin=maxno-depth, v=EMPTY_ELEM, nBB=EMPTY_ELEM;  
	LISTA_L(depth).index=EMPTY_ELEM;
	const int DEPTH_PLUS1=depth+1; 

	//copies list of vertices to color and stores size for fast empty check 
	int pc=(m_unsel=LISTA_BB(depth)).popcn64();

	////////////////////////////
	//color up to kmin-1
	while(1){
		if(col>=kmin) break;							//exit condition
		m_colsets[col]=m_unsel;
		m_colsets[col].init_scan(bbo::NON_DESTRUCTIVE);
		while(1){
			v=m_colsets[col].next_bit(nBB, m_unsel);
			if(v==EMPTY_ELEM) break;
			if((--pc)==0){ return col;}  //cut, kmin not reached

			//color as set difference
			m_colsets[col].erase_block(nBB,g->get_neighbors(v));
		}
		col++;
	}

	//update candidate list and sort by color
	cout<<endl;
	m_unsel.init_scan(bbo::DESTRUCTIVE);
	while(1){
		v=m_unsel.next_bit_del();
		if(v==EMPTY_ELEM) return (CLQ_MAXINT);

		LISTA_L(depth).nodos[++LISTA_L(depth).index]=v;
		cout<<"["<<v<<",";
		LISTA_BB(depth).erase_bit(v);			        
	}
	cout<<"]"<<endl;

	return EMPTY_ELEM;			//should not reach here
}

inline
int CliqueIter::paint_iter_ROOT(bitarray& bb_unexplored, int inc_sol){
/////////////////////
// Initial considerations: 1-inc_sol vertices from bbrem that can enlarhe the current clique soluton in maxno (MUST BE >0)
//						   2-it is assumed that maxno is already updated with the new value (*** EXPERIMENTAL ***)
//
// Contents: paints bbrem with inc_sol colors and updates data structures conveniently
// 
// RETURNS: EMPTY_ELEM if no candidates remaining: solution to MCP found, inc_sol otherwise
//
// Other considerations: At the moment, the inc_sol vertices from bbrem which ENLARGE the current best clique
//						 are not considered in any way (they may be filtered or not)
		
	int col=1; int kmin=inc_sol; int nBB=EMPTY_ELEM; int v=EMPTY_ELEM;
	m_lroot.index=EMPTY_ELEM;
		
	//copies list of vertices to color and stores size for fast empty check 
	int pc= bb_unexplored.popcn64();

	//consistency check (*** remove ***)
	if(pc<inc_sol){
		LOG_ERROR("paint_iter_ROOT(): INCONSISTENT NUMBER OF REMAINING VERTICES");
		return EMPTY_ELEM;			
	}
		
	while(true){ 
		m_sel=bb_unexplored;
		m_sel.init_scan(bbo::DESTRUCTIVE);
		while(true){
			v=m_sel.next_bit_del(nBB,bb_unexplored);
			if(v==EMPTY_ELEM) break;
			if(col<=inc_sol){  
				//adds filtered vertices (and color labels) to problem set
				m_bbroot.set_bit(v);					
				m_lcol[0][v]=maxno;					//assumed updated (maximum color possible)	
			}

			if( (--pc)==0 ){
					return EMPTY_ELEM;		//solution to MCP found!	
			} 
			m_sel.erase_block(nBB,g->get_neighbors(v));
		}				
	col++;
	if(col>inc_sol) break;		 
	}

	//add remaining unexplored vertices to list of candidates
	bb_unexplored.init_scan(bbo::NON_DESTRUCTIVE);
	int i=maxno;
	while(true){
		v=bb_unexplored.next_bit();
		if(v==EMPTY_ELEM)
					break;
		m_lroot.nodos[++m_lroot.index]=v;
		m_lcol[0][v]=i++;					//silly but legal upper bound		
	}

return inc_sol;								
}





int CliqueIter::paint_iter_sat_R_sel_seq(int depth){
////////////////////
//  recoloring followed by infra-chromatic filter in selective framework
//
// REMARKS: the control flow is slightly changed (looks simpler)

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
			if((--pc)==0){ return col;}  //cut, kmin not reached
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
							if((--pc)==0){ return col-1;}
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
							if((--pc)==0){ return col-1;}
							else{
								//LOG_INFO("RECOLORING FOUND");
								RECOLOR_SWAP(v,vswap,recol,j);
								goto next_recol;
								}
						}
					}//end of search for new color set j<recol
				}else if(pc_swap==0){ 
					if((--pc)==0){ return col-1;}
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
							if((--pc)==0){ return col-1;}
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
							if((--pc)==0){ return col-1;}
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
	
	//update candidate list and sort by color
	m_unsel.init_scan(bbo::DESTRUCTIVE);
	while(1){
		v=m_unsel.next_bit_del();
		if(v==EMPTY_ELEM) return (CLQ_MAXINT);

		LISTA_L(depth).nodos[++LISTA_L(depth).index]=v;
		LISTA_BB(depth).erase_bit(v);			        
	}

	return EMPTY_ELEM;										//should not reach here
}

int CliqueIter::paint_iter_sat_R_sel(int depth){
////////////////////
//  Combined recolorong+infra-chrom filter
//
// REMARKS: the control flow is slightly changed (looks simpler)

	
	int col=1, kmin=maxno-depth, v=EMPTY_ELEM, nBB=EMPTY_ELEM;  
	LISTA_L(depth).index=EMPTY_ELEM;
	const int DEPTH_PLUS1=depth+1; 

	//copies list of vertices to color and stores size for fast empty check 
	int pc=(m_unsel=LISTA_BB(depth)).popcn64();
				
	////////////////////////////
	//color up to kmin-1
	while(1){
		if(col>=kmin) break;							//exti condition
		m_colsets[col]=m_unsel;
		m_colsets[col].init_scan(bbo::NON_DESTRUCTIVE);
		while(1){
			v=m_colsets[col].next_bit(nBB, m_unsel);
			if(v==EMPTY_ELEM) break;
			if((--pc)==0){ return col;}  //cut, kmin not reached
						
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
					//searches for new color set j>recol (for(int j=recol+1; j<kmin; j++) comprises the second loop)
					for(int j=recol+1; j<kmin; j++){
						//filters out forbidden colors
						if(m_forbidden.is_bit(j)) continue;
						if( m_colsets[j].is_disjoint(g->get_neighbors(vswap)) ){
							//updates color subsets
							if((--pc)==0){ return col-1;}
							else{
								//LOG_INFO("RECOLORING FOUND");
								RECOLOR_SWAP(v,vswap,recol,j);
								goto next_v;
							}
						}else if( m_colsets[j].is_disjoint(g->get_neighbors(v),g->get_neighbors(vswap))){
							//updates inconsistent color set with (recol, j)
							if((--pc)==0){ return col-1;}
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
							if((--pc)==0){ return col-1;}
							else{
								//LOG_INFO("RECOLORING FOUND");
								RECOLOR_SWAP(v,vswap,recol,j);
								goto next_v;
							}
						}else if( m_colsets[j].is_disjoint(g->get_neighbors(v),g->get_neighbors(vswap))){
							//updates inconsistent color set with (recol, j)
							if((--pc)==0){ return col-1;}
							else{
								//LOG_INFO("SAT INCONSISTENT SUBSET FOUND");
								INFRA_CHROM(v,recol,j);
								goto next_v;
							}
						}
					}//end of search for new color set j>recol
				}else if(pc_swap==0){ 
					if((--pc)==0){ return col-1;}
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
		if(v==EMPTY_ELEM) return (CLQ_MAXINT);

		LISTA_L(depth).nodos[++LISTA_L(depth).index]=v;
		LISTA_BB(depth).erase_bit(v);			        
	}

return EMPTY_ELEM;											//should not reach here
}


int  CliqueIter::paint_iter_sat(int depth){
//////////////////////
// date: 6/9/15
// last update: 06/9/15
//
// infrachromatic in standard framework (attempts to filter all vertices for color subsets above kmin)
// switch first_color available (currently OFF)
// 
// RETURN: color label, or EMPTY_ELEM if error
//	
	int col=1, kmin=maxno-depth, v=EMPTY_ELEM, nBB=EMPTY_ELEM;  
	LISTA_L(depth).index=EMPTY_ELEM;
	const int DEPTH_PLUS1=depth+1; 

	//copies list of vertices to color and stores size for fast empty check 
	int pc=(m_unsel=LISTA_BB(depth)).popcn64();

	////////////////////////////
	//color up to kmin-1
	while(1){
		if(col>=kmin) break;							//exti condition
		m_colsets[col]=m_unsel;
		m_colsets[col].init_scan(bbo::NON_DESTRUCTIVE);
		while(1){
			v=m_colsets[col].next_bit(nBB, m_unsel);
			if(v==EMPTY_ELEM) break;
			if((--pc)==0){ return col;}  //cut, kmin not reached

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
			for(int recol=1; recol<(kmin-1); recol++){		//loop to find initial color seed
				//filters out forbidden colors
				if(m_forbidden.is_bit(recol)) continue;
				//check if color is valid for swapping (0-1 neighbors)	
				int vswap;
				int pc_swap=m_colsets[recol].single_disjoint(g->get_neighbors(v), vswap);
				if(pc_swap==1){	//color class found
					/////////////////////////////////////////////
					//searches for new color set j>recol (for(int j=recol+1; j<kmin; j++) comprises the second loop)
					for(int j=recol+1; j<kmin; j++){
						//filters out forbidden colors
						if(m_forbidden.is_bit(j)|| j==recol) continue;
						if( m_colsets[j].is_disjoint(g->get_neighbors(v),g->get_neighbors(vswap))){
							//updates inconsistent color set with (recol, j)
							if((--pc)==0){ return col-1;}
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
							if((--pc)==0){ return col-1;}
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

	//update candidate list sorted by non decreasing color number
	while(1){
		m_sel=m_unsel;
		m_sel.init_scan(bbo::DESTRUCTIVE);
		while(1){
			v=m_sel.next_bit_del(nBB,m_unsel);
			if(v==EMPTY_ELEM) break;						

			LISTA_L(depth).nodos[++LISTA_L(depth).index]=v;
			LISTA_BB(depth).erase_bit(v);	
			m_lcol[DEPTH_PLUS1][v]=col;			

			if((--pc)==0){ return col;}											
			m_sel.erase_block(nBB,g->get_neighbors(v));
		}
		col++;
	}

	return EMPTY_ELEM;			//should not reach here
}

int  CliqueIter::paint_iter_sat_sel(int depth){
//////////////////////
// date: 6/9/15
// last update: 06/9/15
//
// infrachromatic in standard framework (attempts to filter all vertices for color subsets above kmin)
// switch first_color available (currently OFF)
// 
// RETURN: color label, or EMPTY_ELEM if error
//	
	int col=1, kmin=maxno-depth, v=EMPTY_ELEM, nBB=EMPTY_ELEM;  
	LISTA_L(depth).index=EMPTY_ELEM;
	const int DEPTH_PLUS1=depth+1; 

	//copies list of vertices to color and stores size for fast empty check 
	int pc=(m_unsel=LISTA_BB(depth)).popcn64();

	////////////////////////////
	//color up to kmin-1
	while(1){
		if(col>=kmin) break;							//exit condition
		m_colsets[col]=m_unsel;
		m_colsets[col].init_scan(bbo::NON_DESTRUCTIVE);
		while(1){
			v=m_colsets[col].next_bit(nBB, m_unsel);
			if(v==EMPTY_ELEM) break;
			if((--pc)==0){ return col;}  //cut, kmin not reached

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
			for(int recol=1; recol<(kmin-1); recol++){		//loop to find initial color seed
				//filters out forbidden colors
				if(m_forbidden.is_bit(recol)) continue;
				//check if color is valid for swapping (0-1 neighbors)	
				int vswap;
				int pc_swap=m_colsets[recol].single_disjoint(g->get_neighbors(v), vswap);
				if(pc_swap==1){	//color class found
					/////////////////////////////////////////////
					//searches for new color set j>recol (for(int j=recol+1; j<kmin; j++) comprises the second loop)
					for(int j=recol+1; j<kmin; j++){
						//filters out forbidden colors
						if(m_forbidden.is_bit(j)|| j==recol) continue;
						if( m_colsets[j].is_disjoint(g->get_neighbors(v),g->get_neighbors(vswap))){
							//updates inconsistent color set with (recol, j)
							if((--pc)==0){ return col-1;}
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
							if((--pc)==0){ return col-1;}
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
	m_unsel.init_scan(bbo::DESTRUCTIVE);
	while(1){
		v=m_unsel.next_bit_del();
		if(v==EMPTY_ELEM) return (CLQ_MAXINT);

		LISTA_L(depth).nodos[++LISTA_L(depth).index]=v;
		LISTA_BB(depth).erase_bit(v);			        
	}

	return EMPTY_ELEM;			//should not reach here
}






int  CliqueIter::paint_iter_sat_R(int depth){
//////////////////////
// date: 6/9/15
// last update: 06/9/15
//
// infrachromatic + recoloring in standard framework (attempts to filter all vertices for color subsets above kmin)
// switch first_color available (currently OFF)
// 
// RETURN: color label, or EMPTY_ELEM if error
//

	int col=1, kmin=maxno-depth, v=EMPTY_ELEM, nBB=EMPTY_ELEM;  
	LISTA_L(depth).index=EMPTY_ELEM;
	const int DEPTH_PLUS1=depth+1; 

	//copies list of vertices to color and stores size for fast empty check 
	int pc=(m_unsel=LISTA_BB(depth)).popcn64();
				
	////////////////////////////
	//color up to kmin-1
	while(1){
		if(col>=kmin) break;							//exti condition
		m_colsets[col]=m_unsel;
		m_colsets[col].init_scan(bbo::NON_DESTRUCTIVE);
		while(1){
			v=m_colsets[col].next_bit(nBB, m_unsel);
			if(v==EMPTY_ELEM) break;
			if((--pc)==0){ return col;}  //cut, kmin not reached
						
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
					//searches for new color set j>recol (for(int j=recol+1; j<kmin; j++) comprises the second loop)
					for(int j=recol+1; j<kmin; j++){
						//filters out forbidden colors
						if(m_forbidden.is_bit(j)) continue;
						if( m_colsets[j].is_disjoint(g->get_neighbors(vswap)) ){
							//updates color subsets
							if((--pc)==0){ return col-1;}
							else{
								//LOG_INFO("RECOLORING FOUND");
								RECOLOR_SWAP(v,vswap,recol,j);
								goto next_v;
							}
						}else if( m_colsets[j].is_disjoint(g->get_neighbors(v),g->get_neighbors(vswap))){
							//updates inconsistent color set with (recol, j)
							if((--pc)==0){ return col-1;}
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
							if((--pc)==0){ return col-1;}
							else{
								//LOG_INFO("RECOLORING FOUND");
								RECOLOR_SWAP(v,vswap,recol,j);
								goto next_v;
							}
						}else if( m_colsets[j].is_disjoint(g->get_neighbors(v),g->get_neighbors(vswap))){
							//updates inconsistent color set with (recol, j)
							if((--pc)==0){ return col-1;}
							else{
								//LOG_INFO("SAT INCONSISTENT SUBSET FOUND");
								INFRA_CHROM(v,recol,j);
								goto next_v;
							}
						}
					}//end of search for new color set j>recol
				}else if(pc_swap==0){ 
					if((--pc)==0){ return col-1;}
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
			LISTA_BB(depth).erase_bit(v);	
			m_lcol[DEPTH_PLUS1][v]=col;			

			if((--pc)==0){ return col;}											
			m_sel.erase_block(nBB,g->get_neighbors(v));
		}
		col++;
	}

return EMPTY_ELEM;										//should not reach here
}

inline
int CliqueIter::incUB(bitarray& bb, int* labels){
///////////////
//  For a given subgraph bb, returns the maximum color label assigned to any of its vertices
//  (an upper bound for maximum clique)
// 
//  initial date: 12/06/15
//  last update: 12/06/15

	int col=0; int v=EMPTY_ELEM;
	bb.init_scan(bbo::NON_DESTRUCTIVE);
	while(true){
		v=bb.next_bit();
		if(v==EMPTY_ELEM)
			break;
		col=max<int>(col, labels[v]);
	}
	
	return col;  //0 if empty bb
}

inline
int CliqueIter::incUB(bitarray& bb, int w, int* labels){
///////////////
//  Considers positional neihgbors to determine an incremental upper bound
//	Returns min(c(N(w))+1,|N(w)+1|) for subset of vertices < w 
// 
//  initial date: 28/7/15
//  last update:  28/7/15
	
	int col=0; int v=EMPTY_ELEM; int pc=0;
	bb.init_scan(bbo::NON_DESTRUCTIVE);
	while(true){
		v=bb.next_bit();
		if(v>=w || v==EMPTY_ELEM )			
			break;
		if(!g->get_neighbors(v).is_bit(w))			//filters neighbors vertice of input vertex w
			continue;

		col=max<int>(col, labels[v]);
		pc++;
	}
	
	//minimum 
	//return min<int>(col+1, pc+1);
	return min<int>(col, pc);	
}



int CliqueIter::is_enlargeable(int cq_size, int* cq, int vref){
//////////////////
// determines if any vertex above vref can enlarge clique in cq
	int inc_size=0;
	bitarray bbcand(m_size);
	bbcand.set_bit(0, m_size-1);
	//determine candidates to enlarge cq
	for(int i=0; i<cq_size; i++){
		bbcand&=g->get_neighbors(cq[i]);
	}

	bbcand.init_scan(bbo::NON_DESTRUCTIVE);
	while(true){
		int v=bbcand.next_bit();
		if(v==EMPTY_ELEM) break;
		inc_size ++;
		bbcand&=g->get_neighbors(v);
	}
	
	return inc_size;
}

int CliqueIter::is_enlargeable(int cq_size, int* cq, const bitarray& bbrem){
//////////////////
// determines greedily if vertices from bbrem set can enlarge clique in cq
//
// RETURNS: the increment in the size of cq

	int inc_size=0;
	bitarray bbcand(bbrem);
	
	//determine candidates to enlarge cq
	for(int i=0; i<cq_size; i++){
		bbcand&=g->get_neighbors(cq[i]);
		//*** is_empty check to optimize? ***
	}

	bbcand.init_scan(bbo::NON_DESTRUCTIVE);
	while(true){
		int v=bbcand.next_bit();
		if(v==EMPTY_ELEM) break;
		inc_size ++;
		bbcand&=g->get_neighbors(v);
	}
	
	return inc_size;
}


#endif