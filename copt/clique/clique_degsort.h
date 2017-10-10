////////////////////////////////
// clique_degsort.h: interface for CliquDegSort class which contains algorithms which branch on degree
//					 (currently, only for non sparse undirected graphs)
//				  
// initial date:05/10/15
// last update: 
// author: pablo san segundo


#ifndef  __CLIQUE_DEGSORT_H__
#define  __CLIQUE_DEGSORT_H__

#include "clique.h"
#include "../init_color_ub.h"
#include "graph/algorithms/degree_ugraph.h"

class CliqueDegSort: public Clique<ugraph>{
private:
	static const int LIMIT_DEG_BRANCH=10;			//depth limit to degree branching strategy
public:
		
	CliqueDegSort(ugraph* g, param_t p)				:Clique<ugraph>(g, p), pdeg(NULL){}
	CliqueDegSort(param_t p)						:Clique<ugraph>(p), pdeg(NULL){}
	virtual ~CliqueDegSort()						{clear_deg();}
	
	virtual int init_bitarrays();		

	//paint
	void paint_degsort(int depth);	
	void paint_degsort_extended(int depth);									//includes all vertices in subproblem to evaluate degree
	void paint_degsort_sat_R_sel_N(int depth);
	void paint_degsort_extended_sat_R_sel_N(int depth);						//includes all vertices in subproblem to evaluate degree
	
		
	//search procedures
	void expand_degsort (int maxac, bitarray& l_bb , nodelist_t& l_v);
	void expand_degsort_sat_R_sel_N (int maxac, bitarray& l_bb , nodelist_t& l_v);
	

	//test framework
	virtual	int set_up();
	virtual	void run();	
	virtual	void tear_down(){ clear_deg(); Clique<ugraph>::tear_down();}

	//auxiliary member functions to manage DegUg object
	void init_deg() {clear_deg(); pdeg=new DegUg(*g);}
	void clear_deg() {if(pdeg){delete pdeg; pdeg=NULL;}}

private:

	
//auxiliary data structures 
	bitarray m_forbidden;							//for BBMCX type of algorithms
	DegUg* pdeg;									//to compute degree sort in many nodes
	
};

int CliqueDegSort::init_bitarrays(){
//////////
// called from set_up()

	Clique<ugraph>::init_bitarrays();
	
	//particular inits
	init_deg();

	//empty set
	m_forbidden.init(m_size);
	return 0;
}

inline
int CliqueDegSort::set_up(){
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
	case BBMCD:
	case BBMCD_XR_LN:
	
		break;
	default:
		LOG_ERROR("CliqueDegSort::setup unknown algorithm");
		return -1;
	}
	
	//actual set_up
	if(param.unrolled){
		LOG_ERROR("CliqueDegSort::setup unrolled variant not defined");
		return -1;
	}else{
		if( (sol=set_up_non_unrolled(info))>0 ){
				res.set_UB(sol);
		}else{//Trivial solution not found 

			//*** additional extra-initialization non_unrolled case ***

			//computes strong initial color bounds
			InitColorUB c(*g);
			c.Compute_UB_last(m_lcol[0]);
			for(int i=0; i<m_size; i++){
			cout<<m_lcol[0][i]<<" ";
			}
			cout<<endl;

		}
	
	}	
	
	//violencia
	if(sol>0){
		LOG_INFO("[w="<<sol<<"]"<<" TRIVIALLY SOLVED DURING PRECOMPUTATION");
	}

	return sol;
}

inline
void CliqueDegSort::run(){
	//algorithm
	if(param.unrolled){
		LOG_ERROR("CliqueDegSort::run: unrolling not defined for this familiy of algorithms");
		return;
	}else{
		res.tic();
		switch(param.alg){
		case BBMCD:
			expand_degsort(0, Clique<ugraph>::m_bbroot, Clique<ugraph>::m_lroot);
			break;
		case BBMCD_XR_LN:
			expand_degsort_sat_R_sel_N(0, Clique<ugraph>::m_bbroot, Clique<ugraph>::m_lroot);
			break;
		default:
			LOG_ERROR("CliqueDegSort::run-non_unrolled:unknown clique algorithm");
		}

		res.toc();
	}

	//read solution
	res.set_UB(maxno);
	LOG_INFO("[w:"<<res.get_upper_bound()<<","<<res.get_user_time()<<"s]");
}	

inline
void CliqueDegSort::expand_degsort (int maxac, bitarray& l_bb , nodelist_t& l_v){
////////////////////////
// recursive search algorithm - selective framework to branch on degree using the "tail" of
// the color sets 
// reference: BBMCL variant 

	int v;
//main loop
	res.inc_number_of_steps();

	while(l_v.index>=0){
			
		//Estrategias
		v=l_v.nodos[l_v.index--];
		/*ofstream f("kk.txt", ios::app);
		f<<"v:"<<v<<" d:"<<maxac<<endl;
		f.close();*/

		//ROOT CUT
#ifdef STRONG_ROOT_COLORING
		if(maxac==0){
			if((m_lcol[maxac][v]+maxac)<=maxno )
				continue;
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
		paint_degsort(maxac);

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
		expand_degsort(maxac+1,LISTA_BB(maxac),LISTA_L(maxac));  

		//////////////////////////////////////////////
		// BACKTRACK (does not delete v from path since it will be overwritten in the same level of search tree)
		l_bb.erase_bit(v); 
	}// next node

return;
}


inline
void CliqueDegSort::paint_degsort(int depth){
///////////////////
// paint based on selective coloring

	
	int col=0/* important*/, kmin=maxno-depth, nBB=EMPTY_ELEM, v=EMPTY_ELEM; 
	LISTA_L(depth).index=EMPTY_ELEM;

	//copies list of vertices to color (does not count population)
	 m_unsel=LISTA_BB(depth);

	 while(1){
		col++;
		//exit once kmin-1 colors are assigned (first copy remaining vertices as candidates)
		if(col>=kmin){
			
			
			int * first=&LISTA_L(depth).nodos[LISTA_L(depth).index+1];
			m_unsel.init_scan(bbo::DESTRUCTIVE);
			while(1){
				if((v=m_unsel.next_bit_del())==EMPTY_ELEM)
							break; 
				LISTA_L(depth).nodos[++LISTA_L(depth).index]=v;
			}

			int * last=&LISTA_L(depth).nodos[LISTA_L(depth).index+1];		//adds one to include last vertex
			//sort by degree from first to last
			if(depth>0 && depth<=LIMIT_DEG_BRANCH){
				pdeg->degree_sort(first, last);
			}
			

			return;
		}

		//copies to sel
		m_sel=m_unsel;
		m_sel.init_scan(bbo::DESTRUCTIVE);
		while(1){
			v=m_sel.next_bit_del(nBB,m_unsel);
			if(v==EMPTY_ELEM)
	  					break;
			
			//coloring
			m_sel.erase_block(nBB,g->get_neighbors(v));
	
		}
	 }//next color
 }

inline
void CliqueDegSort::paint_degsort_extended(int depth){
///////////////////
// extends degree evaluation branching-on-degree to include all members of the current subgraph
// and not just the tail of the coloring
		
	int col=0/* important*/, kmin=maxno-depth, nBB=EMPTY_ELEM, v=EMPTY_ELEM; 
	LISTA_L(depth).index=EMPTY_ELEM;

	//copies list of vertices to color (does not count population)
	 m_unsel=LISTA_BB(depth);

	 while(1){
		col++;
		//exit once kmin-1 colors are assigned (first copy remaining vertices as candidates)
		if(col>=kmin){
			int *from=&LISTA_L(depth).nodos[0];
			int * first=&LISTA_L(depth).nodos[LISTA_L(depth).index+1];
			m_unsel.init_scan(bbo::DESTRUCTIVE);
			while(1){
				if((v=m_unsel.next_bit_del())==EMPTY_ELEM)
							break; 
				LISTA_L(depth).nodos[++LISTA_L(depth).index]=v;
			}

			int * last=&LISTA_L(depth).nodos[LISTA_L(depth).index+1];		//adds one to include last vertex
			
			//sort by degree from first to last
			if(depth>0 && depth<=LIMIT_DEG_BRANCH){
				pdeg->degree_sort(first, last, from);
			}
			return;
		}

		//copies to sel
		m_sel=m_unsel;
		m_sel.init_scan(bbo::DESTRUCTIVE);
		while(1){
			v=m_sel.next_bit_del(nBB,m_unsel);
			if(v==EMPTY_ELEM)
	  					break;
			
			//coloring
			m_sel.erase_block(nBB,g->get_neighbors(v));
	
		}
	 }//next color

}



inline
void CliqueDegSort::expand_degsort_sat_R_sel_N (int maxac, bitarray& l_bb , nodelist_t& l_v){
////////////////////////
// recursive search algorithm infra-chrom + recoloring in the selective framework

	int v;
	res.inc_number_of_steps();
	
	//main loop
	while(l_v.index>=0){
			
		//Estrategias
		v=l_v.nodos[l_v.index--];

#ifdef STRONG_ROOT_COLORING
		if(maxac==0){
			if((m_lcol[maxac][v]+maxac)<=maxno )
				continue;
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
		paint_degsort_extended_sat_R_sel_N(maxac);
		//paint_degsort_sat_R_sel_N(maxac);

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
		expand_degsort_sat_R_sel_N(maxac+1,LISTA_BB(maxac),LISTA_L(maxac));  

		//////////////////////////////////////////////
		// BACKTRACK (does not delete v from path since it will be overwritten in the same level of search tree)
		l_bb.erase_bit(v); 

	}// next node

return;
}

void CliqueDegSort::paint_degsort_sat_R_sel_N(int depth){
//////////////////
// Infrachromatic filter + reocloring filter in selective coloring. 
// Applied selectively vertices with label kmin UNTIL first failure
// (recoloring if possible, comes first, to enlarge color subsets as much as possible before they are checked for inconsistency)
// 
// date: 04/09/15
// last_update: 04/09/15
//
// COMMENTS: attemtps to filter vertices with label kmin until first failure.
//			 To attempt to filter all vertices, active while loop (1) (CURRENT STATE ON--MAXIMUM EFFICIENCY)

	int col=0/* important*/, kmin=maxno-depth, v=EMPTY_ELEM, nBB=EMPTY_ELEM;  
	LISTA_L(depth).index=EMPTY_ELEM;

	//copies list of vertices to color and stores size for fast empty check 
	int pc=(m_unsel=LISTA_BB(depth)).popcn64();
	if(pc<kmin){return;}							//CUT based on populaton size (***possibly remove***)
	
	
	while(1){
		col++;
		//threshhold: either recolor or copy to selection list
		//needs to be placed here to catch col=1 and Kmin=0 or Kmin=1 where no recoloring should be done
		if(col>=kmin){	
			///////////////////////////////////////////
			// RECOLORING ATTEMPT
			if(kmin>=3){

				//cleans forbidden color sets (already used for inconsisten subsets)
				m_forbidden.erase_bit(1, kmin);	
	
				m_unsel.init_scan(bbo::NON_DESTRUCTIVE);
				while(1){														//REMOVE LOOP to implement filter only up to first failure: when ON attempts all colors
next_v:				v=m_unsel.next_bit();
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


								if( m_colsets[j].is_disjoint(g->get_neighbors(vswap)) ){

									//updates color subsets
									if((--pc)==0){ return;}
									else{
										//LOG_INFO("RECOLORING FOUND");
										m_unsel.erase_bit(v);
										m_colsets[j].set_bit(vswap);
										m_colsets[recol].set_bit(v);
										m_colsets[recol].erase_bit(vswap);
										goto next_v;
									}


								}else if( m_colsets[j].is_disjoint(g->get_neighbors(v),g->get_neighbors(vswap))){

									//updates inconsistent color set with (recol, j)
									if((--pc)==0){ return;}
									else{
										//LOG_INFO("SAT INCONSISTENT SUBSET FOUND");
										m_unsel.erase_bit(v);
										m_forbidden.set_bit(j);
										m_forbidden.set_bit(recol);
										goto next_v;
									}

								}
							}//end of search for new color set j>recol


							/////////////////////////////////////////////
							//searches for new color set j>recol
							for(int j=1; j<recol; j++){


								//filters out forbidden colors
								if(m_forbidden.is_bit(j)) continue;


								if( m_colsets[j].is_disjoint(g->get_neighbors(vswap)) ){

									//updates color subsets
									if((--pc)==0){ return;}
									else{
										//LOG_INFO("RECOLORING FOUND");
										m_unsel.erase_bit(v);
										m_colsets[j].set_bit(vswap);
										m_colsets[recol].set_bit(v);
										m_colsets[recol].erase_bit(vswap);
										goto next_v;
									}


								}else if( m_colsets[j].is_disjoint(g->get_neighbors(v),g->get_neighbors(vswap))){

									//updates inconsistent color set with (recol, j)
									if((--pc)==0){ return;}
									else{
										//LOG_INFO("SAT INCONSISTENT SUBSET FOUND");
										m_unsel.erase_bit(v);
										m_forbidden.set_bit(j);
										m_forbidden.set_bit(recol);
										goto next_v;
									}

								}
							}//end of search for new color set j>recol

						}else if(pc_swap==0){ 
							if((--pc)==0){	return;	}
							else{
								//LOG_INFO("SIMPLE RECOLORING FOUND");
								m_unsel.erase_bit(v);
								m_colsets[recol].set_bit(v);
								goto next_v;
							}
						}

					}//next candidate for swap color seed
				}//next attemp at vertex pruning

			}//end filter

			// END RECOLORING ATTEMPT
			//////////////////////////////////////////

			//copy remaining unlabeled vertices to candidate set and EXIT
			m_unsel.init_scan(bbo::DESTRUCTIVE);
			
			int * first=&LISTA_L(depth).nodos[LISTA_L(depth).index+1];
			while(1){
				v=m_unsel.next_bit_del();
				if(v==EMPTY_ELEM) 
							break;

				//add to candidate list
				LISTA_L(depth).nodos[++LISTA_L(depth).index]=v;
			}

			int * last=&LISTA_L(depth).nodos[LISTA_L(depth).index+1];		//adds one to include last vertex
			
			//sort by degree from first to last
			if(depth>0 && depth<=LIMIT_DEG_BRANCH){
				pdeg->degree_sort(first, last);
			}
			return;



		}//end col>=kmin

		//computes color set (color number col)
		m_colsets[col]=m_unsel;
		m_colsets[col].init_scan(bbo::NON_DESTRUCTIVE);
		bool is_empty_color_set=true;						 //*** comment ***
		while(1){
			v=m_colsets[col].next_bit(nBB, m_unsel);
			if(v==EMPTY_ELEM){
				if(is_empty_color_set) return;		
				break;
			}

			//color as set difference
			m_colsets[col].erase_block(nBB,g->get_neighbors(v));
			is_empty_color_set=false;
		}

	}//next color
}

inline
void CliqueDegSort::paint_degsort_extended_sat_R_sel_N(int depth){
////////////
//extends degree evaluation to all vertices in the current subgraph,
// not just the "tail" of the coloring

	int col=0/* important*/, kmin=maxno-depth, v=EMPTY_ELEM, nBB=EMPTY_ELEM;  
	LISTA_L(depth).index=EMPTY_ELEM;

	//copies list of vertices to color and stores size for fast empty check 
	int pc=(m_unsel=LISTA_BB(depth)).popcn64();
	if(pc<kmin){return;}							//CUT based on populaton size (***possibly remove***)
	
	
	while(1){
		col++;
		//threshhold: either recolor or copy to selection list
		//needs to be placed here to catch col=1 and Kmin=0 or Kmin=1 where no recoloring should be done
		if(col>=kmin){	
			///////////////////////////////////////////
			// RECOLORING ATTEMPT
			if(kmin>=3){

				//cleans forbidden color sets (already used for inconsisten subsets)
				m_forbidden.erase_bit(1, kmin);	
	
				m_unsel.init_scan(bbo::NON_DESTRUCTIVE);
				while(1){														//REMOVE LOOP to implement filter only up to first failure: when ON attempts all colors
next_v:				v=m_unsel.next_bit();
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


								if( m_colsets[j].is_disjoint(g->get_neighbors(vswap)) ){

									//updates color subsets
									if((--pc)==0){ return;}
									else{
										//LOG_INFO("RECOLORING FOUND");
										m_unsel.erase_bit(v);
										m_colsets[j].set_bit(vswap);
										m_colsets[recol].set_bit(v);
										m_colsets[recol].erase_bit(vswap);
										goto next_v;
									}


								}else if( m_colsets[j].is_disjoint(g->get_neighbors(v),g->get_neighbors(vswap))){

									//updates inconsistent color set with (recol, j)
									if((--pc)==0){ return;}
									else{
										//LOG_INFO("SAT INCONSISTENT SUBSET FOUND");
										m_unsel.erase_bit(v);
										m_forbidden.set_bit(j);
										m_forbidden.set_bit(recol);
										goto next_v;
									}

								}
							}//end of search for new color set j>recol


							/////////////////////////////////////////////
							//searches for new color set j>recol
							for(int j=1; j<recol; j++){


								//filters out forbidden colors
								if(m_forbidden.is_bit(j)) continue;


								if( m_colsets[j].is_disjoint(g->get_neighbors(vswap)) ){

									//updates color subsets
									if((--pc)==0){ return;}
									else{
										//LOG_INFO("RECOLORING FOUND");
										m_unsel.erase_bit(v);
										m_colsets[j].set_bit(vswap);
										m_colsets[recol].set_bit(v);
										m_colsets[recol].erase_bit(vswap);
										goto next_v;
									}


								}else if( m_colsets[j].is_disjoint(g->get_neighbors(v),g->get_neighbors(vswap))){

									//updates inconsistent color set with (recol, j)
									if((--pc)==0){ return;}
									else{
										//LOG_INFO("SAT INCONSISTENT SUBSET FOUND");
										m_unsel.erase_bit(v);
										m_forbidden.set_bit(j);
										m_forbidden.set_bit(recol);
										goto next_v;
									}

								}
							}//end of search for new color set j>recol

						}else if(pc_swap==0){ 
							if((--pc)==0){	return;	}
							else{
								//LOG_INFO("SIMPLE RECOLORING FOUND");
								m_unsel.erase_bit(v);
								m_colsets[recol].set_bit(v);
								goto next_v;
							}
						}

					}//next candidate for swap color seed
				}//next attemp at vertex pruning

			}//end filter

			// END RECOLORING ATTEMPT
			//////////////////////////////////////////

			//copy remaining unlabeled vertices to candidate set and EXIT
			m_unsel.init_scan(bbo::DESTRUCTIVE);
			int *from=&LISTA_L(depth).nodos[0];
			int * first=&LISTA_L(depth).nodos[LISTA_L(depth).index+1];
			while(1){
				v=m_unsel.next_bit_del();
				if(v==EMPTY_ELEM) 
							break;

				//add to candidate list
				LISTA_L(depth).nodos[++LISTA_L(depth).index]=v;
			}

			int * last=&LISTA_L(depth).nodos[LISTA_L(depth).index+1];		//adds one to include last vertex
			
			//sort by degree from first to last
			if(depth>0 && depth<=LIMIT_DEG_BRANCH){
				pdeg->degree_sort(first, last, from);
			}
			return;



		}//end col>=kmin

		//computes color set (color number col)
		m_colsets[col]=m_unsel;
		m_colsets[col].init_scan(bbo::NON_DESTRUCTIVE);
		bool is_empty_color_set=true;						 //*** comment ***
		while(1){
			v=m_colsets[col].next_bit(nBB, m_unsel);
			if(v==EMPTY_ELEM){
				if(is_empty_color_set) return;		
				break;
			}

			//color as set difference
			m_colsets[col].erase_block(nBB,g->get_neighbors(v));
			is_empty_color_set=false;
		}

	}//next color
}

#endif
