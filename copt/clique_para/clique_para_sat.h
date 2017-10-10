////////////////////////////////
// clique_para_sat.h: interface for CliqueParaSAT class which implements multicore MaxSAT based variants
//				  
// initial date:17/11/15
// last update: 
// author: pablo san segundo


#ifndef  __CLIQUE_PARA_SAT_H__
#define  __CLIQUE_PARA_SAT_H__

#include "clique_para.h"
#include "../init_color_ub.h"
#include "../amts/amts_exec.h"
#include "../common/common_macros.h"			


class CliqueParaSat: public CliquePara<ugraph>{
	
typedef void (CliqueParaSat::*func_sat)(int, bitarray&, nodelist_t&);

public:
	enum return_t {ROOT_NODE=-1, CONTINUE_SEARCH_IN_BRANCH=0};
	CliqueParaSat(ugraph* g, param_t p)				:CliquePara<ugraph>(g, p){};
	CliqueParaSat(param_t p)						:CliquePara<ugraph>(p){};
	virtual ~CliqueParaSat(){}
	
	virtual int init_bitarrays();		

	//paint
	void paint_sat(int depth);
	void paint_sat_R_sel(int depth);
	
	//search procedures
	void expand_sat (int maxac, bitarray& l_bb , nodelist_t& l_v);
	void expand_sat_R_sel (int maxac, bitarray& l_bb , nodelist_t& l_v);
	void initial_expand	(func_sat);
	
	//test framework
	virtual	int set_up();
	virtual	void run();	
	virtual	void tear_down(){ clear_forbidden(); CliquePara<ugraph>::tear_down();}

private:
////////////////////////
//auxiliary data structures

	int init_forbidden();
	void clear_forbidden();

	bitarray* m_forbidden;								//[THREAD]
};

int CliqueParaSat::init_forbidden(){
	
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

void CliqueParaSat::clear_forbidden(){
	if(m_forbidden !=NULL){
		delete [] m_forbidden;
	}
	m_forbidden=NULL;
}

int CliqueParaSat::init_bitarrays(){

	CliquePara<ugraph>::init_bitarrays();
	
	//init member structures
	init_forbidden();
	return 0;
}

inline
int CliqueParaSat::set_up(){
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
	case BBMCX:
	case BBMCXR_L:
		break;
	default:
		LOG_ERROR("CliqueParaSat::setup unknown algorithm");
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
void CliqueParaSat::run(){
	//algorithm
	if(!param.unrolled){
		LOG_ERROR("CliqueParaSat::run: unrolling not defined for this familiy of algorithms");
		return;
	}else{
		res.tic();
		
		switch(param.alg){
		case BBMCX:
			initial_expand(&CliqueParaSat::expand_sat);
			break;
		case BBMCXR_L:
			initial_expand(&CliqueParaSat::expand_sat_R_sel);
			break;
		default:
			LOG_ERROR("CliqueParaSat::run-non_unrolled:unknown clique algorithm");
		}

		res.toc();
	}

	//read solution
	res.set_UB(maxno);
	LOG_INFO("[w:"<<res.get_upper_bound()<<","<<res.get_user_time()<<"s]");
}	

inline
void CliqueParaSat::expand_sat (int maxac, bitarray& l_bb , nodelist_t& l_v){
////////////////////////
// recursive search algorithm

	int v;
	int ID=omp_get_thread_num();
	res.inc_number_of_steps();

	//main loop
	while(l_v.index>=0){
			
		//Estrategias
		v=l_v.nodos[l_v.index--];

		//CUT by color (since [Konc & Janecic, 2007] this is of limited use (only for first branch))
#ifdef STRONG_ROOT_COLORING
		if(maxac==1){
			if( (m_lcol[ID][maxac][v]+maxac)<=maxno )
				continue;
		}else if( (m_lcol[ID][maxac][v]+maxac)<=maxno ){
			break;
		}
#else
		if( (m_lcol[ID][maxac][v]+maxac)<=maxno ){
			break;
		}
#endif

/////////////////////////////////
// CHILD NODE GENERATION
		
		//Node generation by masking
		AND(g->get_neighbors(v), l_bb, LISTA_BB(ID,maxac));		//optimized when place second the bitset with higher population
		
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
		l_bb.erase_bit(v);
		continue;
		}
		//approx. coloring (generates child list of nodes in LISTA_L(maxac))
		paint_sat(maxac);

		//cuts if there are no child nodes of v
		if(LISTA_L(ID,maxac).index<0){
			l_bb.erase_bit(v);
			continue;
		}
				
///////////////////////////////////////////////////////
// CANDIDATE EXPANSION

		//sets path
		m_path[ID][maxac]=v;
				
		//Generacion de nuevos nodos
		expand_sat(maxac+1,LISTA_BB(ID,maxac),LISTA_L(ID,maxac));  

		//////////////////////////////////////////////
		// BACKTRACK (does not delete v from path since it will be overwritten in the same level of search tree)
		l_bb.erase_bit(v); 

	}// next node

return;
}

inline
void CliqueParaSat::expand_sat_R_sel (int maxac, bitarray& l_bb , nodelist_t& l_v){
////////////////////////
// recursive search algorithm infra-chrom + recoloring in the selective framework

	int v;
	int ID=omp_get_thread_num();
	res.inc_number_of_steps();
	
	//main loop
	while(l_v.index>=0){
			
		//Estrategias
		v=l_v.nodos[l_v.index--];
		

		//ROOT CUT
#ifdef STRONG_ROOT_COLORING
		if(maxac==1){
			
			if( (m_lcol[ID][maxac][v]+maxac)<=maxno ){
				//cout<<"ID("<<ID<<")"<<"ROOT CUT:"<<v<<endl;
				continue;
			}
		}

#endif

/////////////////////////////////
// CHILD NODE GENERATION
		
		//Node generation by masking
		AND(g->get_neighbors(v), l_bb, LISTA_BB(ID,maxac));		//optimized when place second the bitset with higher population
		
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
		l_bb.erase_bit(v);
		continue;
		}
		//approx. coloring (generates child list of nodes in LISTA_L(maxac))
		paint_sat_R_sel(maxac);

		//cuts if there are no child nodes of v
		if(LISTA_L(ID,maxac).index<0){
			l_bb.erase_bit(v);
			continue;
		}
				
///////////////////////////////////////////////////////
// CANDIDATE EXPANSION

		//sets path
		m_path[ID][maxac]=v;
				
		//Generacion de nuevos nodos
		expand_sat_R_sel(maxac+1,LISTA_BB(ID,maxac),LISTA_L(ID,maxac));  

		//////////////////////////////////////////////
		// BACKTRACK (does not delete v from path since it will be overwritten in the same level of search tree)
		l_bb.erase_bit(v); 

	}// next node

return;
}



inline
void CliqueParaSat::paint_sat(int depth){
///////////////////
// Applies infra-chromatic filter to all candidate vertices 
// Searches for second inconsistent color subset both over and below the first color subset

	int ID=omp_get_thread_num();
	int col=1, kmin=maxno-depth, v=EMPTY_ELEM, nBB=EMPTY_ELEM;  
	LISTA_L(ID,depth).index=EMPTY_ELEM;
	const int DEPTH_PLUS1=depth+1; 

	//copies list of vertices to color and stores size for fast empty check 
	int pc=(m_unsel[ID]=LISTA_BB(ID,depth)).popcn64();
	//if(pc<kmin){return;}								//CUT based on populaton size (***possibly remove***)
				
	////////////////////////////
	//color up to kmin-1
	while(1){
		if(col>=kmin) break;							//exti condition
		m_colsets[ID][col]=m_unsel[ID];
		m_colsets[ID][col].init_scan(bbo::NON_DESTRUCTIVE);
		while(1){
			v=m_colsets[ID][col].next_bit(nBB, m_unsel[ID]);
			if(v==EMPTY_ELEM) break;
			if((--pc)==0){ return;}  //cut, kmin not reached
						
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

			//recolor v if possible
			for(int recol=1; recol<(kmin-1); recol++){		//loop to find initial color seed
				//filters out forbidden colors
				if(m_forbidden[ID].is_bit(recol)) continue;
				//check if color is valid for swapping (0-1 neighbors)	
				int vswap;
				int pc_swap=m_colsets[ID][recol].single_disjoint(g->get_neighbors(v), vswap);
				if(pc_swap==1){	//color class found
					/////////////////////////////////////////////
					//searches for new color set j>recol
					for(int j=recol+1; j<kmin; j++){
						//filters out forbidden colors
						if(m_forbidden[ID].is_bit(j)) continue;
						if( m_colsets[ID][j].is_disjoint(g->get_neighbors(v),g->get_neighbors(vswap))){
							//updates inconsistent color set with (recol, j)
							if((--pc)==0){ return;}
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
							if((--pc)==0){ return;}
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
	//update candidate list and sort by color
	while(1){
		m_sel[ID]=m_unsel[ID];
		m_sel[ID].init_scan(bbo::DESTRUCTIVE);
		while(1){
			v=m_sel[ID].next_bit_del(nBB,m_unsel[ID]);
			if(v==EMPTY_ELEM) break;						
			
			LISTA_L(ID,depth).nodos[++LISTA_L(ID,depth).index]=v;
			m_lcol[ID][DEPTH_PLUS1][v]=col;			

			if((--pc)==0){ return ;}											
			m_sel[ID].erase_block(nBB,g->get_neighbors(v));
		}
		col++;
	}
 }
 
void CliqueParaSat::paint_sat_R_sel(int depth){
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

	int ID=omp_get_thread_num();
	int col=1, kmin=maxno-depth, v=EMPTY_ELEM, nBB=EMPTY_ELEM;  
	LISTA_L(ID,depth).index=EMPTY_ELEM;

	//copies list of vertices to color and stores size for fast empty check 
	int pc=(m_unsel[ID]=LISTA_BB(ID,depth)).popcn64();
	//if(pc<kmin){return;}								//CUT based on populaton size (***possibly remove***)
				
	////////////////////////////
	//color up to kmin-1
	while(1){
		if(col>=kmin) break;							//exti condition
		m_colsets[ID][col]=m_unsel[ID];
		m_colsets[ID][col].init_scan(bbo::NON_DESTRUCTIVE);
		while(1){
			v=m_colsets[ID][col].next_bit(nBB, m_unsel[ID]);
			if(v==EMPTY_ELEM) break;
			if((--pc)==0){ return;}  //cut, kmin not reached
						
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
					//searches for new color set j>recol
					for(int j=recol+1; j<kmin; j++){
						//filters out forbidden colors
						if(m_forbidden[ID].is_bit(j)) continue;
						if( m_colsets[ID][j].is_disjoint(g->get_neighbors(vswap)) ){
							//updates color subsets
							if((--pc)==0){ return;}
							else{
								//LOG_INFO("RECOLORING FOUND");
								RECOLOR_SWAP_PARA(v,vswap,recol,j);
								goto next_v;
							}
						}else if( m_colsets[ID][j].is_disjoint(g->get_neighbors(v),g->get_neighbors(vswap))){
							//updates inconsistent color set with (recol, j)
							if((--pc)==0){ return;}
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
							if((--pc)==0){ return;}
							else{
								//LOG_INFO("RECOLORING FOUND");
								RECOLOR_SWAP_PARA(v,vswap,recol,j);
								goto next_v;
							}
						}else if( m_colsets[ID][j].is_disjoint(g->get_neighbors(v),g->get_neighbors(vswap))){
							//updates inconsistent color set with (recol, j)
							if((--pc)==0){ return;}
							else{
								//LOG_INFO("SAT INCONSISTENT SUBSET FOUND");
								INFRA_CHROM_PARA(v,recol,j);
								goto next_v;
							}
						}
					}//end of search for new color set j>recol
				}else if(pc_swap==0){ 
					if((--pc)==0){ return;}
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
		if(v==EMPTY_ELEM)	return;
		
		//add to candidate list
		LISTA_L(ID,depth).nodos[++LISTA_L(ID,depth).index]=v;
	}
}

void CliqueParaSat::initial_expand(func_sat f){
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

			LISTA_BB(ID,0).init_scan(bbo::NON_DESTRUCTIVE);
			int i=1;
			while(true){
				int w=LISTA_BB(ID,0).next_bit();
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
