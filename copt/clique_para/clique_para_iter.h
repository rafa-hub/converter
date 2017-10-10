////////////////////////////////
// clique_para_iter.h: interface for CliqueParaIter class which implements multicore Russian Doll variants
// currently the classical splitting in reverse order at the root node does not seem to be working here
//				  
// initial date:17/11/15
// last update: 
// author: pablo san segundo


#ifndef  __CLIQUE_PARA_ITER_H__
#define  __CLIQUE_PARA_ITER_H__

#include "clique_para.h"
#include "../init_color_ub.h"
#include "../amts/amts_exec.h"
#include "../common/common_macros.h"			


class CliqueParaIter: public CliquePara<ugraph>{
	
	enum return_t {ROOT_NODE=-1, CONTINUE_SEARCH_IN_BRANCH=0};
typedef return_t (CliqueParaIter::*func_sat)(int, bitarray&, nodelist_t&);

public:
	CliqueParaIter(ugraph* g, param_t p)				:CliquePara<ugraph>(g, p){};
	CliqueParaIter(param_t p)						:CliquePara<ugraph>(p){};
	virtual ~CliqueParaIter(){}
	
	virtual int init_bitarrays();		

	//paint
	int paint_iter(int depth);
	int paint_iter_sat(int depth);
	int paint_iter_sat_R_sel(int depth);
	
	//search procedures
	return_t expand_iter (int maxac, bitarray& l_bb , nodelist_t& l_v);
	return_t expand_iter_sat (int maxac, bitarray& l_bb , nodelist_t& l_v);
	return_t expand_iter_sat_R_sel (int maxac, bitarray& l_bb , nodelist_t& l_v);
	void initial_expand	(func_sat);
	
	//test framework
	virtual	int set_up();
	virtual	void run();	
	virtual	void tear_down(){ clear_forbidden(); CliquePara<ugraph>::tear_down();}

private:
////////////////////////
//auxiliary data structures

	inline int incUB(bitarray&, int v, int* labels);

	int init_forbidden();
	void clear_forbidden();

	bitarray* m_forbidden;								//[THREAD]
};


inline
int CliqueParaIter::incUB(bitarray& bb, int w, int* labels){
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

int CliqueParaIter::init_forbidden(){
	
	clear_others();
	try{
		m_forbidden= new bitarray[m_nCores];		
		for(int i=0; i<m_nCores; i++){
				m_forbidden[i].init(m_size);
		}

	}catch(exception& e){
		throw;
	}

	return 0;
}

void CliqueParaIter::clear_forbidden(){
	if(m_forbidden !=NULL){
		delete [] m_forbidden;
	}
	m_forbidden=NULL;
}

int CliqueParaIter::init_bitarrays(){

	CliquePara<ugraph>::init_bitarrays();
	
	//init member structures
	init_forbidden();
	return 0;
}

inline
int CliqueParaIter::set_up(){
//////////////
// allocates memory, evaluates initial bounds and determines 
// initial trivial solutions
//
// RETURN VALUE: -1 Error, 0-ok, >0 trivial solution found

	
	res.clear();
	d.clear();
	res.set_name(g->get_name());

	//setup number of cores (before memory allocation)
	set_cores(param.nThreads);

	//determine allocation info
	search_alloc_t info;							//loads BBMC configuration for search allocation
	info.set(search_alloc_t::ALLOC_COLOR_SETS);		//for recoloring
	switch(param.alg){
	case BBMCIT:
	case BBMCITX:
	case BBMCITXR_L:
		break;
	default:
		LOG_ERROR("CliqueParaIter::setup unknown algorithm");
		return -1;
	}
	
	//actual set_up
	int sol=0;
	if(param.unrolled){
		if((sol=set_up_unrolled(info))>0){ 
			res.set_UB(sol);							
		}else{
			//search step: additional initialization
			
#ifdef STRONG_ROOT_COLORING
			InitColorUB c(*g);
			c.Compute_UB_last(m_lcol[0][0]);

			//copy root color info to all threads
			for(int id=1; id<m_nCores; id++){
				for(int i=0; i<m_size; i++){
					m_lcol[id][0][i]=m_lcol[0][0][i];
				}
			}

			//output to screen (TODO-LOGGER)
			for(int i=0; i<m_size; i++){
				cout<<m_lcol[0][0][i]<<" ";
			}
			cout<<endl;
#endif
#ifdef AMTS_LB			
			//amts initial solution: 
			//****TODO: place in setup and check for TRIVIAL SOLUTION
			AMTSexec a(50, 20000);
			int lb_amts=a.run(*g);
			if(lb_amts>res.get_lower_bound()){
				maxno=lb_amts;
				res.set_LB(lb_amts);
			}
			
#endif		

		}
	}else{
		LOG_ERROR("CliquePara::set_up()-non_unrolled cannot be set to TRUE in multicore execution");
		return -1;
	}


	//trivial solution
	if(sol>0){
		LOG_INFO("[w="<<sol<<"]"<<" TRIVIALLY SOLVED DURING PRECOMPUTATION");
	}

	return sol;
}

inline
void CliqueParaIter::run(){
	//algorithm
	if(!param.unrolled){
		LOG_ERROR("CliqueParaIter::run: unrolling not defined for this familiy of algorithms");
		return;
	}else{
		res.tic();
		
		switch(param.alg){
		case BBMCIT:
			initial_expand(&CliqueParaIter::expand_iter);
			break;
		case BBMCITX:
			initial_expand(&CliqueParaIter::expand_iter_sat);
			break;
		case BBMCITXR_L:
			initial_expand(&CliqueParaIter::expand_iter_sat_R_sel);
			break;
		default:
			LOG_ERROR("CliqueParaIter::run-non_unrolled:unknown clique algorithm");
		}

		res.toc();
	}

	//read solution
	res.set_UB(maxno);
	LOG_INFO("[w:"<<res.get_upper_bound()<<","<<res.get_user_time()<<"s]");
}	

inline
CliqueParaIter::return_t CliqueParaIter::expand_iter (int maxac, bitarray& l_bb , nodelist_t& l_v){
////////////////////////
// recursive search algorithm

	int v; int col;
	int ID=omp_get_thread_num();
	res.inc_number_of_steps();

	//main loop
	int vIndex=0;
	while(vIndex<=l_v.index){
			
		//Estrategias
		v=l_v.nodos[vIndex++];
	
		//CUT by color (since [Konc & Janecic, 2007] this is of limited use (only for first branch))
		if( (m_lcol[ID][maxac][v]+maxac)<=maxno ){
			l_bb.set_bit(v);
			continue;
		}

/////////////////////////////////
// CHILD NODE GENERATION
		
		//Node generation by masking
		AND(g->get_neighbors(v), l_bb, LISTA_BB(ID,maxac));		//optimized when the second argument is the bitset with higher population
		
/////////////////////
//CUT based on previous color labels
		//if(maxac==1){
		//	col=incUB(LISTA_BB(ID,maxac),v, m_lcol[ID][maxac]);
		//	if(m_lcol[ID][maxac][v]>col+1){
		//		//updates colors with new lower bound
		//		m_lcol[ID][maxac][v]=col+1;
		//		//checks if pruning is possible
		//		if( (m_lcol[ID][maxac][v]+maxac)<=maxno){
		//			l_bb.set_bit(v);
		//	//		cout<<"incUB-CUT:"<<v<<":"<<maxac<<endl;
		//			continue;									//next vertex after v
		//		}
		//	}
		//	//upper bound can never be greater than Smax (at the moment does not make a difference)
		//	m_lcol[ID][maxac][v]=min<int>(m_lcol[ID][maxac][v],maxno);
		//}
////////////////////////////

		//Leaf node: updates incumbent if necessary
		if( LISTA_BB(ID,maxac).is_empty()){

//////CRITICAL SECTION///////////	
#pragma omp critical
{
			if(maxac>=maxno){
				maxno=maxac+1;						//NEW GLOBAL OPTIMUM FOUND

				#ifdef STORE_SOLUTION
					res.set_UB(maxno);
					res.clear_all_solutions();
					m_path[ID][maxac]=v;
					res.add_solution(maxno, m_path[ID]);
										
					#ifdef VIEW_PROGRESS
						stringstream sstr("");
						res.print_first_sol(sstr);
						LOG_INFO(sstr.str());
					#endif
						
				
					
				#endif
				
			}
}
//////END OF CRITICAL SECTION///////////

		//repeat evaluation, but outside the critical section
		if(maxac>=maxno){
			//update color at root node with new solution and return to root node
				m_lcol[ID][1][m_path[ID][1]]=maxno;
				return ROOT_NODE;
		}
		
		
		l_bb.set_bit(v);
		continue;
		}
		
		//approx. coloring (generates child list of nodes in LISTA_L(maxac))
		col=paint_iter(maxac);
		
		//cuts if there are no child nodes of v
		if(LISTA_L(ID,maxac).index<0){
			//update color threshold
			if(maxac==1){
				if(m_lcol[ID][maxac][v]>col+1){
					m_lcol[ID][maxac][v]=col+1;

				}
			}
			l_bb.set_bit(v); 
			continue;		
		}


		//root update of colors
		/*if(maxac==1){
			if(m_lcol[ID][maxac][v]>col+1){
					m_lcol[ID][maxac][v]=col+1;
			}
		}*/
	
				
///////////////////////////////////////////////////////
// CANDIDATE EXPANSION

		//sets path
		m_path[ID][maxac]=v;
				
		//Generacion de nuevos nodos
		if(expand_iter(maxac+1,LISTA_BB(ID,maxac),LISTA_L(ID,maxac))==ROOT_NODE ){
			if(maxac!=1)
				return ROOT_NODE;
		}

		//////////////////////////////////////////////
		// BACKTRACK: adds the examined vertex
		
		l_bb.set_bit(v);
	}// next node

return CONTINUE_SEARCH_IN_BRANCH;
}

inline
CliqueParaIter::return_t CliqueParaIter::expand_iter_sat (int maxac, bitarray& l_bb , nodelist_t& l_v){
////////////////////////
// recursive search algorithm

	int v; int col;
	int ID=omp_get_thread_num();
	res.inc_number_of_steps();

	//main loop
	int vIndex=0;
	while(vIndex<=l_v.index){
			
		//Estrategias
		v=l_v.nodos[vIndex++];
	
		//CUT by color (since [Konc & Janecic, 2007] this is of limited use (only for first branch))
		if( (m_lcol[ID][maxac][v]+maxac)<=maxno ){
			l_bb.set_bit(v);
			continue;
		}

/////////////////////////////////
// CHILD NODE GENERATION
		
		//Node generation by masking
		AND(g->get_neighbors(v), l_bb, LISTA_BB(ID,maxac));		//optimized when the second argument is the bitset with higher population
		
/////////////////////
//CUT based on previous color labels
		//if(maxac==1){
		//	col=incUB(LISTA_BB(ID,maxac),v, m_lcol[ID][maxac]);
		//	if(m_lcol[ID][maxac][v]>col+1){
		//		//updates colors with new lower bound
		//		m_lcol[ID][maxac][v]=col+1;
		//		//checks if pruning is possible
		//		if( (m_lcol[ID][maxac][v]+maxac)<=maxno){
		//			l_bb.set_bit(v);
		//	//		cout<<"incUB-CUT:"<<v<<":"<<maxac<<endl;
		//			continue;									//next vertex after v
		//		}
		//	}
		//	//upper bound can never be greater than Smax (at the moment does not make a difference)
		//	m_lcol[ID][maxac][v]=min<int>(m_lcol[ID][maxac][v],maxno);
		//}
////////////////////////////

		//Leaf node: updates incumbent if necessary
		if( LISTA_BB(ID,maxac).is_empty()){

//////CRITICAL SECTION///////////	
#pragma omp critical
{
			if(maxac>=maxno){
				maxno=maxac+1;						//NEW GLOBAL OPTIMUM FOUND

				#ifdef STORE_SOLUTION
					res.set_UB(maxno);
					res.clear_all_solutions();
					m_path[ID][maxac]=v;
					res.add_solution(maxno, m_path[ID]);
										
					#ifdef VIEW_PROGRESS
						stringstream sstr("");
						res.print_first_sol(sstr);
						LOG_INFO(sstr.str());
					#endif
						
				
					
				#endif
				
			}
}
//////END OF CRITICAL SECTION///////////

		//repeat evaluation, but outside the critical section
		if(maxac>=maxno){
			//update color at root node with new solution and return to root node
				m_lcol[ID][1][m_path[ID][1]]=maxno;
				return ROOT_NODE;
		}
		
		
		l_bb.set_bit(v);
		continue;
		}
		
		//approx. coloring (generates child list of nodes in LISTA_L(maxac))
		col=paint_iter_sat(maxac);
		
		//cuts if there are no child nodes of v
		if(LISTA_L(ID,maxac).index<0){
			//update color threshold
			if(maxac==1){
				if(m_lcol[ID][maxac][v]>col+1){
					m_lcol[ID][maxac][v]=col+1;

				}
			}
			l_bb.set_bit(v); 
			continue;		
		}


		//root update of colors
		/*if(maxac==1){
			if(m_lcol[ID][maxac][v]>col+1){
					m_lcol[ID][maxac][v]=col+1;
			}
		}*/
	
				
///////////////////////////////////////////////////////
// CANDIDATE EXPANSION

		//sets path
		m_path[ID][maxac]=v;
				
		//Generacion de nuevos nodos
		if(expand_iter_sat(maxac+1,LISTA_BB(ID,maxac),LISTA_L(ID,maxac))==ROOT_NODE ){
			if(maxac!=1)
				return ROOT_NODE;
		}

		//////////////////////////////////////////////
		// BACKTRACK: adds the examined vertex
		
		l_bb.set_bit(v);
	}// next node

return CONTINUE_SEARCH_IN_BRANCH;
}


inline
CliqueParaIter::return_t CliqueParaIter::expand_iter_sat_R_sel (int maxac, bitarray& l_bb , nodelist_t& l_v){
//////////////////////
//	initial date: 19/11/15
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
	int ID=omp_get_thread_num();
	res.inc_number_of_steps();

	//main loop
	int vIndex=0;
	while(vIndex<=l_v.index){
			
		//Estrategias
		v=l_v.nodos[vIndex++];
	
#ifdef STRONG_ROOT_COLORING
		if(maxac==1){
			if((m_lcol[ID][maxac][v]+maxac)<=maxno ){
				l_bb.set_bit(v);
				continue;
			}
		}
#endif

/////////////////////////////////
// CHILD NODE GENERATION
		
		//Node generation by masking
		AND(g->get_neighbors(v), l_bb, LISTA_BB(ID,maxac));		//optimized when the second argument is the bitset with higher population
		
/////////////////////
//CUT based on previous color labels
		//if(maxac==1){
		//	col=incUB(LISTA_BB(ID,maxac),v, m_lcol[ID][maxac]);
		//	if(m_lcol[ID][maxac][v]>col+1){
		//		//updates colors with new lower bound
		//		m_lcol[ID][maxac][v]=col+1;
		//		//checks if pruning is possible
		//		if( (m_lcol[ID][maxac][v]+maxac)<=maxno){
		//			l_bb.set_bit(v);
		//	//		cout<<"incUB-CUT:"<<v<<":"<<maxac<<endl;
		//			continue;									//next vertex after v
		//		}
		//	}
		//	//upper bound can never be greater than Smax (at the moment does not make a difference)
		//	m_lcol[ID][maxac][v]=min<int>(m_lcol[ID][maxac][v],maxno);
		//}
////////////////////////////

		//Leaf node: updates incumbent if necessary
		if( LISTA_BB(ID,maxac).is_empty()){

//////CRITICAL SECTION///////////	
#pragma omp critical
{
			if(maxac>=maxno){
				maxno=maxac+1;						//NEW GLOBAL OPTIMUM FOUND

				#ifdef STORE_SOLUTION
					res.set_UB(maxno);
					res.clear_all_solutions();
					m_path[ID][maxac]=v;
					res.add_solution(maxno, m_path[ID]);
										
					#ifdef VIEW_PROGRESS
						stringstream sstr("");
						res.print_first_sol(sstr);
						LOG_INFO(sstr.str());
					#endif
						
				
					
				#endif
				
			}
}
//////END OF CRITICAL SECTION///////////

			//repeat evaluation, but outside the critical section
			if(maxac>=maxno){
				//update color at root node with new solution and return to root node
				m_lcol[ID][1][m_path[ID][1]]=maxno;
				return ROOT_NODE;
			}
		
		
		l_bb.set_bit(v);
		continue;
		}
		
		//approx. coloring (generates child list of nodes in LISTA_L(maxac))
		col=paint_iter_sat_R_sel(maxac);
		
		//cuts if there are no child nodes of v
		if(LISTA_L(ID,maxac).index<0){
			//update color threshold
			/*if(maxac==1){
				if(m_lcol[ID][maxac][v]>col+1){
					m_lcol[ID][maxac][v]=col+1;

				}
			}*/
			l_bb.set_bit(v); 
			continue;		
		}


		//root update of colors
		/*if(maxac==1){
			if(m_lcol[ID][maxac][v]>col+1){
					m_lcol[ID][maxac][v]=col+1;
			}
		}*/
	
				
///////////////////////////////////////////////////////
// CANDIDATE EXPANSION

		//sets path
		m_path[ID][maxac]=v;
				
		//Generacion de nuevos nodos
		if(expand_iter_sat_R_sel(maxac+1,LISTA_BB(ID,maxac),LISTA_L(ID,maxac))==ROOT_NODE ){
			if(maxac!=1)
				return ROOT_NODE;
		}

		//////////////////////////////////////////////
		// BACKTRACK: adds the examined vertex
		
		l_bb.set_bit(v);
	}// next node

return CONTINUE_SEARCH_IN_BRANCH;
}

inline
int CliqueParaIter::paint_iter(int depth){
///////////////////
// Applies infra-chromatic filter to all candidate vertices 
// Searches for second inconsistent color subset both over and below the first color subset

	int ID=omp_get_thread_num();
	int col=1, kmin=maxno-depth, v=EMPTY_ELEM, nBB=EMPTY_ELEM;  
	LISTA_L(ID,depth).index=EMPTY_ELEM;
	const int DEPTH_PLUS1=depth+1; 

	//copies list of vertices to color and stores size for fast empty check 
	int pc=(m_unsel[ID]=LISTA_BB(ID,depth)).popcn64();
	

	while(true){ 
		m_sel[ID]=m_unsel[ID];
		m_sel[ID].init_scan(bbo::DESTRUCTIVE);
		while(true){
			v=m_sel[ID].next_bit_del(nBB,m_unsel[ID]);
			if(v==EMPTY_ELEM)
							break;
			if(col>=kmin){  
				LISTA_L(ID,depth).nodos[++LISTA_L(ID,depth).index]=v;
				LISTA_BB(ID,depth).erase_bit(v);			        //erases candidate vertices, critical
			}

			//add label always
			m_lcol[ID][DEPTH_PLUS1][v]=col;	
					

			if((--pc)==0){
				 return col;
			}
			m_sel[ID].erase_block(nBB,g->get_neighbors(v));
		}				
	col++;
	}

return EMPTY_ELEM;  //should not reach here
}
 
inline
int CliqueParaIter::paint_iter_sat(int depth){
//////////////////////
// date: 6/9/15
// last update: 06/9/15
//
// infrachromatic in standard framework (attempts to filter all vertices for color subsets above kmin)
// switch first_color available (currently OFF)
// 
// RETURN: color label, or EMPTY_ELEM if error
//	
	int ID=omp_get_thread_num();
	int col=1, kmin=maxno-depth, v=EMPTY_ELEM, nBB=EMPTY_ELEM;  
	LISTA_L(ID,depth).index=EMPTY_ELEM;
	const int DEPTH_PLUS1=depth+1; 

	//copies list of vertices to color and stores size for fast empty check 
	int pc=(m_unsel[ID]=LISTA_BB(ID,depth)).popcn64();

	////////////////////////////
	//color up to kmin-1
	while(1){
		if(col>=kmin) break;							//exti condition
		m_colsets[ID][col]=m_unsel[ID];
		m_colsets[ID][col].init_scan(bbo::NON_DESTRUCTIVE);
		while(1){
			v=m_colsets[ID][col].next_bit(nBB, m_unsel[ID]);
			if(v==EMPTY_ELEM) break;
			if((--pc)==0){ return col;}  //cut, kmin not reached

			//color as set difference
			m_colsets[ID][col].erase_block(nBB,g->get_neighbors(v));
		}
		col++;
	}
/////////////////////////////////////////////
// INFRACHROM FILTERING (col>=kmin always here)
	if(kmin>=3){
		m_forbidden[ID].erase_bit(1, kmin);	
		m_unsel[ID].init_scan(bbo::NON_DESTRUCTIVE);
		while(1){											//REMOVE LOOP to implement filter only up to first failure: when ON attempts all colors
next_v:		v=m_unsel[ID].next_bit();
			if(v==EMPTY_ELEM) break;						
			for(int recol=1; recol<(kmin-1); recol++){		//loop to find initial color seed
				//filters out forbidden colors
				if(m_forbidden[ID].is_bit(recol)) continue;
				//check if color is valid for swapping (0-1 neighbors)	
				int vswap;
				int pc_swap=m_colsets[ID][recol].single_disjoint(g->get_neighbors(v), vswap);
				if(pc_swap==1){	//color class found
					/////////////////////////////////////////////
					//searches for new color set j>recol (for(int j=recol+1; j<kmin; j++) comprises the second loop)
					for(int j=recol+1; j<kmin; j++){
						//filters out forbidden colors
						if(m_forbidden[ID].is_bit(j)|| j==recol) continue;
						if( m_colsets[ID][j].is_disjoint(g->get_neighbors(v),g->get_neighbors(vswap))){
							//updates inconsistent color set with (recol, j)
							if((--pc)==0){ return col-1;}
							else{
								//LOG_INFO("SAT INCONSISTENT SUBSET FOUND");
								INFRA_CHROM_PARA(v,recol,j);
								goto next_v;
							}
						}
					}//end of search for new color set j>recol
					/////////////////////////////////////////////
					//searches for new color set j<recol
					for(int j=1; j<recol; j++){
						//filters out forbidden colors
						if(m_forbidden[ID].is_bit(j)) continue;
						if( m_colsets[ID][j].is_disjoint(g->get_neighbors(v),g->get_neighbors(vswap))){
							//updates inconsistent color set with (recol, j)
							if((--pc)==0){ return col-1;}
							else{
								//LOG_INFO("SAT INCONSISTENT SUBSET FOUND");
								INFRA_CHROM_PARA(v,recol,j);
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
		m_sel[ID]=m_unsel[ID];
		m_sel[ID].init_scan(bbo::DESTRUCTIVE);
		while(1){
			v=m_sel[ID].next_bit_del(nBB,m_unsel[ID]);
			if(v==EMPTY_ELEM) break;						

			LISTA_L(ID,depth).nodos[++LISTA_L(ID,depth).index]=v;
			LISTA_BB(ID,depth).erase_bit(v);	
			m_lcol[ID][DEPTH_PLUS1][v]=col;			

			if((--pc)==0){ return col;}											
			m_sel[ID].erase_block(nBB,g->get_neighbors(v));
		}
		col++;
	}

	return EMPTY_ELEM;			//should not reach here

}

int CliqueParaIter::paint_iter_sat_R_sel(int depth){
////////////////////
//  Combined recolorong+infra-chrom filter
//
// REMARKS: the control flow is slightly changed (looks simpler)
	
	int ID=omp_get_thread_num();
	int col=1, kmin=maxno-depth, v=EMPTY_ELEM, nBB=EMPTY_ELEM;  
	LISTA_L(ID,depth).index=EMPTY_ELEM;
	const int DEPTH_PLUS1=depth+1; 

	//copies list of vertices to color and stores size for fast empty check 
	int pc=(m_unsel[ID]=LISTA_BB(ID,depth)).popcn64();
				
	////////////////////////////
	//color up to kmin-1
	while(1){
		if(col>=kmin) break;							//exti condition
		m_colsets[ID][col]=m_unsel[ID];
		m_colsets[ID][col].init_scan(bbo::NON_DESTRUCTIVE);
		while(1){
			v=m_colsets[ID][col].next_bit(nBB, m_unsel[ID]);
			if(v==EMPTY_ELEM) break;
			if((--pc)==0){ return col;}  //cut, kmin not reached
						
			//color as set difference
			m_colsets[ID][col].erase_block(nBB,g->get_neighbors(v));
		}
		col++;
	}

/////////////////////////////////////////////
// RECOLOR + INFRACHROM FILTERING (col>=kmin always here)
	if(kmin>=3){
		m_forbidden[ID].erase_bit(1, kmin);	
		m_unsel[ID].init_scan(bbo::NON_DESTRUCTIVE);
		while(1){											//REMOVE LOOP to implement filter only up to first failure: when ON attempts all colors
next_v:		v=m_unsel[ID].next_bit();
			if(v==EMPTY_ELEM) break;						

			//recolor v if possible
			for(int recol=1; recol<(kmin-1); recol++){		//loop to find initial color seed
				//filters out forbidden colors
				if(m_forbidden[ID].is_bit(recol)) continue;
				//check if color is valid for swapping (0-1 neighbors)	
				int vswap;
				int pc_swap=m_colsets[ID][recol].single_disjoint(g->get_neighbors(v), vswap);
				if(pc_swap==1){	//color class found
					/////////////////////////////////////////////
					//searches for new color set j>recol (for(int j=recol+1; j<kmin; j++) comprises the second loop)
					for(int j=recol+1; j<kmin; j++){
						//filters out forbidden colors
						if(m_forbidden[ID].is_bit(j)) continue;
						if( m_colsets[ID][j].is_disjoint(g->get_neighbors(vswap)) ){
							//updates color subsets
							if((--pc)==0){ return col-1;}
							else{
								//LOG_INFO("RECOLORING FOUND");
								RECOLOR_SWAP_PARA(v,vswap,recol,j);
								goto next_v;
							}
						}else if( m_colsets[ID][j].is_disjoint(g->get_neighbors(v),g->get_neighbors(vswap))){
							//updates inconsistent color set with (recol, j)
							if((--pc)==0){ return col-1;}
							else{
								//LOG_INFO("SAT INCONSISTENT SUBSET FOUND");
								INFRA_CHROM_PARA(v,recol,j);
								goto next_v;
							}
						}
					}//end of search for new color set j>recol
					/////////////////////////////////////////////
					//searches for new color set j<recol
					for(int j=1; j<recol; j++){
						//filters out forbidden colors
						if(m_forbidden[ID].is_bit(j)) continue;
						if( m_colsets[ID][j].is_disjoint(g->get_neighbors(vswap)) ){
							//updates color subsets
							if((--pc)==0){ return col-1;}
							else{
								//LOG_INFO("RECOLORING FOUND");
								RECOLOR_SWAP_PARA(v,vswap,recol,j);
								goto next_v;
							}
						}else if( m_colsets[ID][j].is_disjoint(g->get_neighbors(v),g->get_neighbors(vswap))){
							//updates inconsistent color set with (recol, j)
							if((--pc)==0){ return col-1;}
							else{
								//LOG_INFO("SAT INCONSISTENT SUBSET FOUND");
								INFRA_CHROM_PARA(v,recol,j);
								goto next_v;
							}
						}
					}//end of search for new color set j>recol
				}else if(pc_swap==0){ 
					if((--pc)==0){ return col-1;}
					else{
						//LOG_INFO("SIMPLE RECOLORING FOUND");
						RECOLOR_SIMPLE_SWAP_PARA(v,recol);
						goto next_v;
					}
				}
			}//next candidate for swap color seed
		}//next attemp at vertex pruning
	}
// END OF INFRACHROM FILTERING
///////////////////////////////////
	//update candidate list and sort by color
	m_unsel[ID].init_scan(bbo::DESTRUCTIVE);
	while(1){
		v=m_unsel[ID].next_bit_del();
		if(v==EMPTY_ELEM) return (CLQ_MAXINT);

		LISTA_L(ID,depth).nodos[++LISTA_L(ID,depth).index]=v;
		LISTA_BB(ID,depth).erase_bit(v);			        
	}

return EMPTY_ELEM;											//should not reach here
}

void CliqueParaIter::initial_expand(func_sat f){
////////////////////
// Unrolling of first level
//
// TODO: simple coloring of each subproblem with vertices sorted in min-width order

	//InitOrder<T> o(*g);
	//InitColor<T> cinit(*g);
	res.inc_number_of_steps();

	//set cores
	omp_set_num_threads(m_nCores);
			
	//Loop over neighbor set subproblems
	#pragma omp parallel for schedule(dynamic)  default(shared)  /*private(f)*/ /*shared(cout,f)*/ //firstprivate(o/*,cinit*/)
	for(int v=m_size-1; v>=maxno; v--){
		int ID=omp_get_thread_num(); 
		LISTA_BB(ID,0).init_bit(v,g->get_neighbors(v));
				
		//CUT related to size: possibly remove?
		int pc=LISTA_BB(ID,0).popcn64();
		if(pc>=maxno){
			
			//order LISTA_BB by root ordering strategy and simple initial coloring
			LISTA_L(ID,0).index=EMPTY_ELEM;

			//vint new_ord=o.create_new_order(param.init_order, LISTA_BB(ID,0), (param.init_order==NONE)? PLACE_FL : PLACE_LF);
			//if(new_ord.empty()) return;
			//int gdeg=g->max_degree_of_subgraph(LISTA_BB(ID,0));
			//for(int i=0; i<new_ord.size(); i++){
			//	LISTA_L(ID,0).nodos[++LISTA_L(ID,0).index]=new_ord[i];
			//	m_lcol[ID][1][new_ord[i]]=(i<=gdeg)? i+1 : gdeg+1;					//simple initial coloring
			//}

			LISTA_BB(ID,0).init_scan(bbo::DESTRUCTIVE);								//MUST BE DESTRCUTIVE: BitString with candidate set must be empty
			int i=1;
			while(true){
				int w=LISTA_BB(ID,0).next_bit_del();
				if(w==EMPTY_ELEM) break;
				LISTA_L(ID,0).nodos[++LISTA_L(ID,0).index]=w;
				int root_col=m_lcol[ID][0][v]-1;
				if(root_col>1){
					m_lcol[ID][1][w]=min<int>(i,m_lcol[ID][0][v]-1);
				}else m_lcol[ID][1][w]=i;
				i++;
			}

			//Search
			m_path[ID][0]=v;
			
#pragma omp critical
{
#ifdef ROOT_VERTEX_PROGRESS
		cout<<"ID:"<<ID<<" root vertex: "<<v<<" w:"<<maxno<<endl;
#endif
}
			//NP step
			(this->*f)(1,LISTA_BB(ID,0),LISTA_L(ID,0));
		}
	}
}

#endif
