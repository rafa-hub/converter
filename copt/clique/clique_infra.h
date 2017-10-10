////////////////////////////////
// clique_infra.h: interface for the CliqueInfra class which contains different implementations of an enhanced
//				   infra-chromatic filter for bit-parallel, non-iterative MCP algorithms 
//
// initial date:2/08/16
// last update: 28/09/16
// author: pablo san segundo


#ifndef  __CLIQUE_INFRA_H__
#define  __CLIQUE_INFRA_H__

#include "clique.h"
#include "../init_color_ub.h"
#include "../amts/amts_exec.h"
#include "bitscan/bbalg.h"
#include "infra_tools.h"
#include "../common/common_macros.h"

using namespace com;												//for common types (here bb_t)

class CliqueInfra: public Clique<ugraph>{
////////////////////////
// data structure for infra-chrom operations
	InfraOp<ugraph,bitarray> iop;
			
public:
	
	CliqueInfra(ugraph* g, param_t p)			:Clique<ugraph>(g, p)  {};
	CliqueInfra(param_t p)						:Clique<ugraph>(p) {};
	virtual ~CliqueInfra(){}
	void set_param(param_t p)					{Clique<ugraph>::clear_all(); CLQParam::set_param(p);}	
	

	virtual int init_color_sets();	
	virtual void clear_color_sets();

////////////
//coloring
	
	//cut over full coloring
	void paint_OS	(int depth);													//attempts at selective one-shot cut
	void paint_R	(int depth, int v=EMPTY_ELEM);	
		
	//cut over kmin									
	void paint_R_kmin				(int depth);									//infra-chrom bounding around kmin
	void paint_R_kmin_sel			(int depth);									//idem but in the selective framework
	void paint_R_kmin_comb			(int depth, int v=EMPTY_ELEM);					
	void paint_R_kmin_comb_inc		(int depth, int v=EMPTY_ELEM);					//experimental ***not enough tests
	
							
	
	//drivers
	void expand_OS			(int maxac, bitarray& l_bb , nodelist_t& l_v);	//one shot
	void expand_R			(int maxac, bitarray& l_bb , nodelist_t& l_v);	//infrachrom at the end
	void expand_R_kmin		(int maxac, bitarray& l_bb , nodelist_t& l_v);	//infrachrom cut up to kmin (currently main variant)
	void expand_R_kmin_sel	(int maxac, bitarray& l_bb , nodelist_t& l_v);	//selective framework  (currently main variant)
	void expand_R_kmin_comb	(int maxac, bitarray& l_bb , nodelist_t& l_v);	//combined end-kmin infrachrom
	
			
	//test framework
	virtual	int set_up();
	virtual	void run();	
	virtual	void tear_down(){ Clique<ugraph>::tear_down();}
};


inline
void CliqueInfra::clear_color_sets (){
	iop.clear();
}

inline
int CliqueInfra::init_color_sets(){

	clear_color_sets();

	try{
		iop.set_graph(g);
		if(iop.init(m_alloc+1)==-1){
			runtime_error r("error allocating infra-chrom ColorSets");		//***check this exception is caught below
			throw r;
		}
		
	}catch(exception& e){
		throw;
	}

	return 0;
}

inline
int CliqueInfra::set_up(){
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
	case BBMC_OS:
	case BBMCR:
	case BBMCR_KMIN:
	case BBMCRL_KMIN:
	case BBMCR_KMIN_COMB:
	break;
	default:
		LOG_ERROR("CliqueInfra::setup unknown algorithm");
		return -1;
	}
	
	//actual set_up
	if(param.unrolled){
		LOG_ERROR("CliqueInfra::setup unrolled variant not defined");
		return -1;
	}else{
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
	
	//trivial solution
	if(sol>0){
		LOG_INFO("[w="<<sol<<"]"<<" TRIVIALLY SOLVED DURING PRECOMPUTATION");
	}

	return sol;
}

inline
void CliqueInfra::run(){
	//algorithm
	if(param.unrolled){
		LOG_ERROR("CliqueInfra::run: unrolling not defined for this familiy of algorithms");
		return;
	}else{
		res.tic();
		switch(param.alg){
		case BBMCR_KMIN:
			expand_R_kmin(0, Clique<ugraph>::m_bbroot, Clique<ugraph>::m_lroot);
			break;
		case BBMCR_KMIN_COMB:
			expand_R_kmin_comb(0, Clique<ugraph>::m_bbroot, Clique<ugraph>::m_lroot);
			break;
		case BBMCRL_KMIN:
			Clique<ugraph>::m_bbroot.print(); cout<<endl;
			Clique<ugraph>::m_lroot.print(cout, true); cout<<endl;
			cout<<"lb: "<<maxno<<endl;
			expand_R_kmin_sel(0, Clique<ugraph>::m_bbroot, Clique<ugraph>::m_lroot);
			break;
		case BBMC_OS:
			expand_OS(0, Clique<ugraph>::m_bbroot, Clique<ugraph>::m_lroot);
			break;
		case BBMCR:
			expand_R(0, Clique<ugraph>::m_bbroot, Clique<ugraph>::m_lroot);
			break;
		default:
			LOG_ERROR("CliqueInfra::run-non_unrolled:unknown clique algorithm");
		}

		res.toc();
	}

	//read solution
	res.set_UB(maxno);
	LOG_INFO("[w:"<<res.get_upper_bound()<<","<<res.get_user_time()<<"s]");
}	

inline
void CliqueInfra::expand_OS (int maxac, bitarray& l_bb , nodelist_t& l_v){
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
		paint_OS(maxac);
		
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
		expand_OS(maxac+1,LISTA_BB(maxac),LISTA_L(maxac));  

		//////////////////////////////////////////////
		// BACKTRACK (does not delete v from path since it will be overwritten in the same level of search tree)
		l_bb.erase_bit(v); 

	}// next node

return;
}

inline
void CliqueInfra::expand_R (int maxac, bitarray& l_bb , nodelist_t& l_v){
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


		/*if(m_lcol[0][v]-maxac<m_lcol[maxac][v]){
			LOG_INFO("ROOT BOUND IS BETTER:"<<m_lcol[0][v]-maxac<<":"<<m_lcol[maxac][v]);

		}*/
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
		paint_R(maxac, v);
		
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
		expand_R(maxac+1,LISTA_BB(maxac),LISTA_L(maxac));  

		//////////////////////////////////////////////
		// BACKTRACK (does not delete v from path since it will be overwritten in the same level of search tree)
		l_bb.erase_bit(v); 

	}// next node

}

inline
void CliqueInfra::expand_R_kmin (int maxac, bitarray& l_bb , nodelist_t& l_v){
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

	//	cout<<"v: "<<v<<"depth: "<<maxac<<endl;
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
		paint_R_kmin(maxac);

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
		expand_R_kmin(maxac+1,LISTA_BB(maxac),LISTA_L(maxac));  

		//////////////////////////////////////////////
		// BACKTRACK (does not delete v from path since it will be overwritten in the same level of search tree)
		l_bb.erase_bit(v); 

	}// next node

}

inline
void CliqueInfra::expand_R_kmin_comb (int maxac, bitarray& l_bb , nodelist_t& l_v){
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

	//	cout<<"v: "<<v<<"depth: "<<maxac<<endl;
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
		paint_R_kmin_comb(maxac, v);
		//*** TO TEST PROPERLY paint_R_kmin_comb_inc(maxac, v);

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
		expand_R_kmin_comb(maxac+1,LISTA_BB(maxac),LISTA_L(maxac));  

		//////////////////////////////////////////////
		// BACKTRACK (does not delete v from path since it will be overwritten in the same level of search tree)
		l_bb.erase_bit(v); 

	}// next node

}

inline
void CliqueInfra::expand_R_kmin_sel (int maxac, bitarray& l_bb , nodelist_t& l_v){
////////////////////////
// recursive search algorithm

	int v;
	res.inc_number_of_steps();

	//main loop
	while(l_v.index>=0){
			
		//Estrategias
		v=l_v.nodos[l_v.index--];
		if(v==8){
			cout<<v<<":"<<maxac<<endl;
			cin.get();
			
		}

		if(v==7 && m_path[0]==8){
			cout<<8<<":"<<7<<":"<<maxac<<endl;
			cin.get();
			
		}

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
		
		if(v==8){
			cout<<v<<":"<<maxac<<":"<<maxno<<endl;
			
			LISTA_BB(maxac).print();
			cin.get();
		}

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
		paint_R_kmin_sel(maxac);


		//cuts if there are no child nodes of v
		if(LISTA_L(maxac).index<0){
			l_bb.erase_bit(v);
			continue;
		}

		if(v==8){
			cout<<v<<":"<<maxac<<endl;
			LISTA_L(maxac).print(cout, true);
			cin.get();
		}
				
///////////////////////////////////////////////////////
// CANDIDATE EXPANSION

		//sets path
		m_path[maxac]=v;
				
		//Generacion de nuevos nodos
		expand_R_kmin_sel(maxac+1,LISTA_BB(maxac),LISTA_L(maxac));  

		//////////////////////////////////////////////
		// BACKTRACK (does not delete v from path since it will be overwritten in the same level of search tree)
		l_bb.erase_bit(v); 

	}// next node

}


inline
void CliqueInfra::paint_OS(int depth){
//////////////////////////
// Greedy SEQ base coloring
// date of creation: 02/04/16
																															
	int cmax=1; int kmin=maxno-depth; int nBB=EMPTY_ELEM; int v=EMPTY_ELEM; 
	LISTA_L(depth).index=EMPTY_ELEM;								
	const int DEPTHPLUS1=depth+1;
	bool iscol;

	//main loop: sizes of color sets are not updated on the fly
	int pc=(m_unsel=LISTA_BB(depth)).popcn64();
	while(true){
		bitarray& bbcmax=iop.m_colSets[cmax].bb;
		for(int i=0; i<iop.NB_OF_BB_NODES; i++){
			bbcmax.get_bitboard(i)=m_unsel.get_bitboard(i);
		}

		bbcmax.init_scan(bbo::NON_DESTRUCTIVE);
		while(true){
			v=bbcmax.next_bit(nBB, m_unsel);
			if(v==EMPTY_ELEM) break;
			if(cmax>=kmin){													//note this includes kmin=cmax color class which may be filtered later
				LISTA_L(depth).nodos[++LISTA_L(depth).index]=v;
			}

			//label for every vertex, required for logical inferences
			m_lcol[DEPTHPLUS1][v]=cmax;	
			iop.node_iset_no[v]=cmax;

			if((--pc)==0){
				goto outer;
			}
			//actual coloring
			bitarray& nbor=g->get_neighbors(v);
			for(int i=nBB; i<iop.NB_OF_BB_NODES; i++){
				bbcmax.get_bitboard(i)&= ~nbor.get_bitboard(i);
			}
		}
		
		cmax++;
	}//next color

////////////////////////
// Attempts at one-color infra-chrom coloring
outer:	
	if(cmax==kmin){
		iop.set_color_nb(cmax);
		iop.update_color_sizes(cmax);
		iop.set_node_state_active(LISTA_BB(depth));
		if(iop.filter()){  //CUT: erases candidate list
			//LOG_INFO("ONE_SHOT_CUT");
			//res.inc_counter(0,1);
			LISTA_L(depth).index=EMPTY_ELEM;		
		}
	}
 }


inline
void CliqueInfra::paint_R(int depth, int v_node){
//////////////////////////
// greedy SEQ with recoloring and one-shot infrachrom pruning if the size of the coloring is critical (LB+1)
// date of creation: 02/04/16
//
																															
	int cmax=1; int kmin=maxno-depth; 
	int nBB, v, vswap; 
	const int KMIN_MINUS_ONE=kmin-1; 
	const int NB_OF_BB_NODES_MINUS_ONE=iop.NB_OF_BB_NODES-1;
	LISTA_L(depth).index=EMPTY_ELEM;								
	const int DEPTHPLUS1=depth+1;
	
	
	
	//main loop: sizes of color sets are not updated on the fly
	int pc=(m_unsel=LISTA_BB(depth)).popcn64();
	while(true){
		bitarray& bbcmax=iop.m_colSets[cmax].bb;
		for(int i=0; i<iop.NB_OF_BB_NODES; i++){
			bbcmax.get_bitboard(i)=m_unsel.get_bitboard(i);
		}

		bbcmax.init_scan(bbo::NON_DESTRUCTIVE);
		
		while(1){
next_v:		v=bbcmax.next_bit(nBB, m_unsel);
			if(v==EMPTY_ELEM) 
						break;

//////////////
// recoloring
			if( (cmax>=kmin) && (kmin>=3 /* at least two color classes below*/)  /*&& first_color*/){
			//if( cmax>=3){
				for(int recol=1; recol<KMIN_MINUS_ONE; recol++){		//loop to find initial color seed
				//for(int recol=1; recol<cmax-1; recol++){		//loop to find initial color seed

					//check if color is valid for swapping (0-1 neighbors)	
					int pc_swap=iop.m_colSets[recol].bb.single_disjoint(0, NB_OF_BB_NODES_MINUS_ONE,g->get_neighbors(v), vswap);

					//analysis
					if(pc_swap==1){	//color class found

						//Busca swap con vertice ya coloreado
						for(int j=recol+1; j<kmin; j++){
					   //  for(int j=recol+1; j<cmax; j++){
							//for(int j=kmin-1/*1 reverse directon; j>/*=1*/recol; j--){
							//if(j==recol) continue;

							if( iop.m_colSets[j].bb.is_disjoint(0, NB_OF_BB_NODES_MINUS_ONE,g->get_neighbors(vswap))){

								//swap action
								iop.m_colSets[j].bb.set_bit(vswap);
								iop.m_colSets[recol].bb.set_bit(v);
								iop.m_colSets[recol].bb.erase_bit(vswap);
								iop.m_colSets[cmax].bb.erase_bit(v);
								m_lcol[DEPTHPLUS1][vswap]=j; 
								m_lcol[DEPTHPLUS1][v]=recol;
								iop.node_iset_no[vswap]=j;
								iop.node_iset_no[v]=recol;

								//empty check of unsel in case vertex swapped is the last one
								if((--pc)==0){
									goto outer;
								}else goto next_v;
							}
						}
					}else if(pc_swap==0){					//disjoint color class found (previous swap necessary)
						iop.m_colSets[recol].bb.set_bit(v);
						iop.m_colSets[cmax].bb.erase_bit(v);
						m_lcol[DEPTHPLUS1][v]=recol;
						iop.node_iset_no[v]=recol;
						if((--pc)==0){
							goto outer;
						}else goto next_v;
					}
				}//next candidate for swap color seed
			}
/////////////////////////////
			if(cmax>=kmin){												//note this includes kmin==cmax color class which may be filtered later
				LISTA_L(depth).nodos[++LISTA_L(depth).index]=v;
				m_lcol[DEPTHPLUS1][v]=cmax;	
			}

			//stores all color labels for infra-chrom cut
			iop.node_iset_no[v]=cmax;

			if((--pc)==0){
				goto outer;
			}
			//actual coloring
			bitarray& nbor=g->get_neighbors(v);
			for(int i=nBB; i<iop.NB_OF_BB_NODES; i++){
				bbcmax.get_bitboard(i)&= ~nbor.get_bitboard(i);
			}
			//iop.m_colSets[cmax].bb.erase_block(nBB,g->get_neighbors(v));
		}
		
		cmax++;
	}//next color

////////////////////////
// Infrachrom attempt over the full colored set
outer:	
	int nCol=cmax;
	if(cmax>1){
		if(iop.m_colSets[cmax].bb.is_empty()){ nCol--; }

		iop.set_color_nb(nCol);
		iop.update_color_sizes(nCol);
			
		iop.set_node_state_active(LISTA_BB(depth));	
		if(nCol==kmin){
			if(iop.filter()){
				//res.inc_counter(0,1);
				LISTA_L(depth).index=EMPTY_ELEM;
				return;
			}
		}

		int nb_conf=iop.init_maxsatz(v_node, kmin);
		if(nb_conf==EMPTY_ELEM){
			//res.inc_counter(0,1);
			LISTA_L(depth).index=EMPTY_ELEM;
		}
	}

	return;
 }

 
inline
void CliqueInfra::paint_R_kmin(int depth){
//////////////////////////
// Tomita greedy SEQ with recoloring and later infra-chrom
// date of creation: 02/04/16
//

	
	int cmax=1; int kmin=maxno-depth; int nBB=EMPTY_ELEM; int v=EMPTY_ELEM; 
	LISTA_L(depth).index=EMPTY_ELEM;								
	const int DEPTHPLUS1=depth+1;
	const int NB_OF_BB_NODES_MINUS_ONE=iop.NB_OF_BB_NODES-1;
	const int KMIN_MINUS_ONE=kmin-1;
	
		
	//colors up to kmin without updating color set sizes
	int pc=(m_unsel=LISTA_BB(depth)).popcn64();
	while(true){
		bitarray& bbcmax=iop.m_colSets[cmax].bb;
		for(int i=0; i<iop.NB_OF_BB_NODES; i++){
			bbcmax.get_bitboard(i)=m_unsel.get_bitboard(i);
		}

		bbcmax.init_scan(bbo::NON_DESTRUCTIVE);
		while(true){
			v=bbcmax.next_bit(nBB, m_unsel);
			if(v==EMPTY_ELEM) break;
			

			//stores color label
			iop.node_iset_no[v]=cmax;
			m_lcol[DEPTHPLUS1][v]=cmax;
		
			if((--pc)==0){
				if(cmax==1){
					int v=LISTA_BB(depth).lsbn64();
					LISTA_L(depth).nodos[++LISTA_L(depth).index]=v;
					m_lcol[DEPTHPLUS1][v]=1;
				}
				 return;
			}
			//actual coloring
			bitarray& nbor=g->get_neighbors(v);
			for(int i=nBB; i<iop.NB_OF_BB_NODES; i++){
				bbcmax.get_bitboard(i)&= ~nbor.get_bitboard(i);
			}
		
		}
			
		//colors up to kmin
		if(++cmax>=kmin)
			break;
	}//next color


////////////////////////
//Recoloring (re-number)
//	
	if(kmin>=3) {
		m_unsel.init_scan(bbo::NON_DESTRUCTIVE);
		while(true){
next_v:		int v=m_unsel.next_bit();
			if(v==EMPTY_ELEM) break;
			for(int recol=1; recol<KMIN_MINUS_ONE; recol++){		//loop to find initial color seed

				//check if color is valid for swapping (0-1 neighbors)	
				int vswap;
				int pc_swap=iop.m_colSets[recol].bb.single_disjoint(0, NB_OF_BB_NODES_MINUS_ONE, g->get_neighbors(v), vswap);

				//analysis
				if(pc_swap==1){	//color class found

					//Busca swap con vertice ya coloreado
					for(int j=recol+1; j<(kmin); j++){
						//for(int j=kmin-1/*1 reverse directon; j>/*=1*/recol; j--){
						if(j==recol) continue;

						if( iop.m_colSets[j].bb.is_disjoint(0, NB_OF_BB_NODES_MINUS_ONE,g->get_neighbors(vswap))){

							//swap action
							iop.m_colSets[j].bb.set_bit(vswap);
							iop.m_colSets[recol].bb.set_bit(v);
							iop.m_colSets[recol].bb.erase_bit(vswap);
							iop.m_colSets[cmax].bb.erase_bit(v);
							m_lcol[DEPTHPLUS1][vswap]=j;
							m_lcol[DEPTHPLUS1][v]=recol;
							iop.node_iset_no[vswap]=j;
							iop.node_iset_no[v]=recol;
						
							m_unsel.erase_bit(v);

							//empty check of unsel in case vertex swapped is the last one
							if((--pc)==0){
								return;
							}else goto next_v;
						}
					}
				}else if(pc_swap==0){					//disjoint color class found (previous swap necessary)
					iop.m_colSets[recol].bb.set_bit(v);
					m_lcol[DEPTHPLUS1][v]=recol;
					iop.node_iset_no[v]=recol;

					m_unsel.erase_bit(v);
					if((--pc)==0){
						return;
					}else goto next_v;
				}

			}//next candidate for swap color seed
		}
	}


//////////////////
//attempt with vertices above kmin
	int nBBv;	bool iscol;
	iop.m_colSets[cmax].erase_bit();
	v=m_unsel.lsbn64();
	m_unsel.erase_bit(v);

	m_lcol[DEPTHPLUS1][v]=cmax;
	iop.update_color_sizes(cmax-1);
	ERASE(LISTA_BB(depth), m_unsel, m_sel);
	iop.set_node_state_active(m_sel);	
	iop.set_color_nb(cmax-1);
	iop.init_inc_maxsatz();

	//new vertex in singleton cmax
	iop.set_color_nb(cmax);
	iop.m_colSets[cmax].push(v);
	iop.node_iset_no[v]=cmax;
	iop.color_unit_stack.push(cmax);
	iop.bb_node_state_active.is_bit(v);
	iop.color_state_active.set_bit(cmax);
	
	
	if(iop.inc_maxsatz(v)!=EMPTY_ELEM){		//END INFRA-CHROM CUT
		LISTA_L(depth).nodos[++LISTA_L(depth).index]=v;
		if((--pc)==0){
			iop.reset_enlarged_isets();
			return;
		}
				
		//Tomita normal coloring and finish
		m_unsel.init_scan(bbo::DESTRUCTIVE);
		while(true){
			int v=m_unsel.next_bit_del(nBBv);
			iscol=false;
			for(int col=cmax; col<=cmax; col++){
				iscol=g->get_neighbors(v).is_disjoint(0,nBBv, iop.m_colSets[col].bb);			//**TODO optimize? note: the reasoning does not include added nodes

				//color found for vertex
				if(iscol){
					iop.m_colSets[col].push(v);					//adds vertex and updates bitstring
					//iop.bb_node_state_active.set_bit(v);
					//iop.color_state_active.set_bit(col);
					iop.node_iset_no[v]=col;
					m_lcol[DEPTHPLUS1][v]=col;
					iscol=true;
					break;
				}
			}

			//color not found: new color
			if(!iscol){
				iop.m_colSets[++cmax].erase_bit();		//clears color (previous use in same level)
				iop.m_colSets[cmax].push(v);
				//iop.color_state_active.set_bit(cmax);
				//iop.bb_node_state_active.set_bit(v);
				iop.node_iset_no[v]=cmax;
				//iop.set_color_nb(cmax);
				m_lcol[DEPTHPLUS1][v]=cmax;
				iop.color_unit_stack.push(cmax);
			}
						
			LISTA_L(depth).nodos[++LISTA_L(depth).index]=v;			
			if((--pc)==0){
				iop.reset_enlarged_isets();
				return;		//SUCCESS
			}
		}//pick next vertex to col


	}else{
		if((--pc)==0){
			iop.reset_enlarged_isets();
			return;
		}
		//color a la Tomita
		bool cut_active=true;
		m_unsel.init_scan(bbo::DESTRUCTIVE);
		while(true){
			int v=m_unsel.next_bit_del(nBBv);
			if(v==EMPTY_ELEM){
				iop.reset_enlarged_isets();
				break;
			}			

			//inner loop: search for SEQ color for v
			iscol=false;
			for(int col=cmax; col<=cmax; col++){
				iscol=g->get_neighbors(v).is_disjoint(0,nBBv, iop.m_colSets[col].bb);			//**TODO optimize? note: the reasoning does not include added nodes

				//color found for vertex
				if(iscol){
					iop.m_colSets[col].push(v);					//adds vertex and updates bitstring
					iop.bb_node_state_active.set_bit(v);
					iop.color_state_active.set_bit(col);
					iop.node_iset_no[v]=col;
					m_lcol[DEPTHPLUS1][v]=col;
					iscol=true;
					break;
				}
			}

			//color not found: new color
			if(!iscol){
				iop.m_colSets[++cmax].erase_bit();		//clears color (previous use in same level)
				iop.m_colSets[cmax].push(v);
				iop.color_state_active.set_bit(cmax);
				iop.bb_node_state_active.set_bit(v);
				iop.node_iset_no[v]=cmax;
				iop.set_color_nb(cmax);
				m_lcol[DEPTHPLUS1][v]=cmax;
				iop.color_unit_stack.push(cmax);
			}

			//attemtp to cut for the first kmin vertices
			if(cut_active){
				if(iop.inc_maxsatz(v)!=EMPTY_ELEM){
					cut_active=false;
					LISTA_L(depth).nodos[++LISTA_L(depth).index]=v;
				}
			}else{
				LISTA_L(depth).nodos[++LISTA_L(depth).index]=v;
			}
						
			if((--pc)==0){
				iop.reset_enlarged_isets();
				return;		//SUCCESS
			}
		}//pick next vertex to col
	}

/////////////////
//** Final attempt to reduce the last colors: 
//** if(succesful) LISTA_L(depth).index=EMPTY_ELEM
// note we cannot consider the full coloring so this last step might be not be useful 
// at all
	
	

 } 


inline
void CliqueInfra::paint_R_kmin_comb(int depth, int v_expanded){
//////////////////////////
// Tomita greedy SEQ with recoloring and later infra-chrom
// date of creation: 02/04/16
//

	
	int cmax=1; int kmin=maxno-depth; int nBB=EMPTY_ELEM; int v=EMPTY_ELEM; 
	LISTA_L(depth).index=EMPTY_ELEM;								
	const int DEPTHPLUS1=depth+1;
	const int NB_OF_BB_NODES_MINUS_ONE=iop.NB_OF_BB_NODES-1;
	const int KMIN_MINUS_ONE=kmin-1;
	
		
	//colors up to kmin without updating color set sizes
	int pc=(m_unsel=LISTA_BB(depth)).popcn64();
	while(true){
		bitarray& bbcmax=iop.m_colSets[cmax].bb;
		for(int i=0; i<iop.NB_OF_BB_NODES; i++){
			bbcmax.get_bitboard(i)=m_unsel.get_bitboard(i);
		}

		bbcmax.init_scan(bbo::NON_DESTRUCTIVE);
		while(true){
			v=bbcmax.next_bit(nBB, m_unsel);
			if(v==EMPTY_ELEM) break;
			

			//stores color label
			iop.node_iset_no[v]=cmax;
			m_lcol[DEPTHPLUS1][v]=cmax;
		
			if((--pc)==0){
				if(cmax==1){
					int v=LISTA_BB(depth).lsbn64();
					LISTA_L(depth).nodos[++LISTA_L(depth).index]=v;
					m_lcol[DEPTHPLUS1][v]=1;
				}
				 return;
			}
			//actual coloring
			bitarray& nbor=g->get_neighbors(v);
			for(int i=nBB; i<iop.NB_OF_BB_NODES; i++){
				bbcmax.get_bitboard(i)&= ~nbor.get_bitboard(i);
			}
		
		}
			
		//colors up to kmin
		if(++cmax>=kmin)
			break;
	}//next color


////////////////////////
//Recoloring (re-number)
//	
	if(kmin>=3) {
		m_unsel.init_scan(bbo::NON_DESTRUCTIVE);
		while(true){
next_v:		int v=m_unsel.next_bit();
			if(v==EMPTY_ELEM) break;
			for(int recol=1; recol<KMIN_MINUS_ONE; recol++){		//loop to find initial color seed

				//check if color is valid for swapping (0-1 neighbors)	
				int vswap;
				int pc_swap=iop.m_colSets[recol].bb.single_disjoint(0, NB_OF_BB_NODES_MINUS_ONE, g->get_neighbors(v), vswap);

				//analysis
				if(pc_swap==1){	//color class found

					//Busca swap con vertice ya coloreado
					for(int j=recol+1; j<(kmin); j++){
						//for(int j=kmin-1/*1 reverse directon; j>/*=1*/recol; j--){
						if(j==recol) continue;

						if( iop.m_colSets[j].bb.is_disjoint(0, NB_OF_BB_NODES_MINUS_ONE,g->get_neighbors(vswap))){

							//swap action
							iop.m_colSets[j].bb.set_bit(vswap);
							iop.m_colSets[recol].bb.set_bit(v);
							iop.m_colSets[recol].bb.erase_bit(vswap);
							iop.m_colSets[cmax].bb.erase_bit(v);
							m_lcol[DEPTHPLUS1][vswap]=j;
							m_lcol[DEPTHPLUS1][v]=recol;
							iop.node_iset_no[vswap]=j;
							iop.node_iset_no[v]=recol;
						
							m_unsel.erase_bit(v);

							//empty check of unsel in case vertex swapped is the last one
							if((--pc)==0){
								return;
							}else goto next_v;
						}
					}
				}else if(pc_swap==0){					//disjoint color class found (previous swap necessary)
					iop.m_colSets[recol].bb.set_bit(v);
					m_lcol[DEPTHPLUS1][v]=recol;
					iop.node_iset_no[v]=recol;

					m_unsel.erase_bit(v);
					if((--pc)==0){
						return;
					}else goto next_v;
				}

			}//next candidate for swap color seed
		}
	}


//////////////////
//attempt with vertices above kmin

	int nBBv;	bool iscol;
	iop.m_colSets[cmax].erase_bit();
	v=m_unsel.lsbn64();
	m_unsel.erase_bit(v);

	m_lcol[DEPTHPLUS1][v]=cmax;
	iop.update_color_sizes(cmax-1);
	ERASE(LISTA_BB(depth), m_unsel, m_sel);
	iop.set_node_state_active(m_sel);	
	iop.set_color_nb(cmax-1);
	iop.init_inc_maxsatz();

	//new vertex in singleton cmax
	iop.set_color_nb(cmax);
	iop.m_colSets[cmax].push(v);
	iop.node_iset_no[v]=cmax;
	iop.color_unit_stack.push(cmax);
	iop.bb_node_state_active.set_bit(v);
	iop.color_state_active.set_bit(cmax);
	
	
	if(iop.inc_maxsatz(v)!=EMPTY_ELEM){		//END INFRA-CHROM CUT
		LISTA_L(depth).nodos[++LISTA_L(depth).index]=v;
		if((--pc)==0){
			iop.reset_enlarged_isets();
			goto outer;
		}
				
		//Tomita normal coloring and finish
		m_unsel.init_scan(bbo::DESTRUCTIVE);
		while(true){
			int v=m_unsel.next_bit_del(nBBv);
			iscol=false;
			for(int col=cmax; col<=cmax; col++){
				iscol=g->get_neighbors(v).is_disjoint(0,nBBv, iop.m_colSets[col].bb);			//**TODO optimize? note: the reasoning does not include added nodes

				//color found for vertex
				if(iscol){
					iop.m_colSets[col].push(v);					//adds vertex and updates bitstring
					iop.bb_node_state_active.set_bit(v);
					//iop.color_state_active.set_bit(col);
					iop.node_iset_no[v]=col;
					m_lcol[DEPTHPLUS1][v]=col;
					iscol=true;
					break;
				}
			}

			//color not found: new color
			if(!iscol){
				iop.m_colSets[++cmax].erase_bit();		//clears color (previous use in same level)
				iop.m_colSets[cmax].push(v);
				iop.color_state_active.set_bit(cmax);
				iop.bb_node_state_active.set_bit(v);
				iop.node_iset_no[v]=cmax;
				//iop.set_color_nb(cmax);
				m_lcol[DEPTHPLUS1][v]=cmax;
				iop.color_unit_stack.push(cmax);
			}
						
			LISTA_L(depth).nodos[++LISTA_L(depth).index]=v;			
			if((--pc)==0){
				iop.reset_enlarged_isets();
				goto outer;		//SUCCESS
			}
		}//pick next vertex to col


	}else{
		if((--pc)==0){
			iop.reset_enlarged_isets();
			goto outer;
		}
		//color a la Tomita
		bool cut_active=true;
		m_unsel.init_scan(bbo::DESTRUCTIVE);
		while(true){
			int v=m_unsel.next_bit_del(nBBv);
			if(v==EMPTY_ELEM){
				iop.reset_enlarged_isets();
				goto outer;
			}			

			//inner loop: search for SEQ color for v
			iscol=false;
			for(int col=cmax; col<=cmax; col++){
				iscol=g->get_neighbors(v).is_disjoint(0,nBBv, iop.m_colSets[col].bb);			//**TODO optimize? note: the reasoning does not include added nodes

				//color found for vertex
				if(iscol){
					iop.m_colSets[col].push(v);					//adds vertex and updates bitstring
					iop.bb_node_state_active.set_bit(v);
					iop.color_state_active.set_bit(col);
					iop.node_iset_no[v]=col;
					m_lcol[DEPTHPLUS1][v]=col;
					iscol=true;
					break;
				}
			}

			//color not found: new color
			if(!iscol){
				iop.m_colSets[++cmax].erase_bit();		//clears color (previous use in same level)
				iop.m_colSets[cmax].push(v);
				iop.color_state_active.set_bit(cmax);
				iop.bb_node_state_active.set_bit(v);
				iop.node_iset_no[v]=cmax;
				iop.set_color_nb(cmax);
				m_lcol[DEPTHPLUS1][v]=cmax;
				iop.color_unit_stack.push(cmax);
			}

			//attemtp to cut for the first kmin vertices
			if(cut_active){
				if(iop.inc_maxsatz(v)!=EMPTY_ELEM){
					cut_active=false;
					LISTA_L(depth).nodos[++LISTA_L(depth).index]=v;
				}
			}else{
				LISTA_L(depth).nodos[++LISTA_L(depth).index]=v;
			}
						
			if((--pc)==0){
				iop.reset_enlarged_isets();
				goto outer;		//SUCCESS
			}
		}//pick next vertex to col
	}

/////////////////
// Final attempt to reduce the last colors: 

outer: //infrachrom in full

int nCol=cmax;
if((nCol-kmin)<20 /* max 64 conflicts*/){

	//***if(iop.m_colSets[cmax].bb.is_empty()){ nCol--; }
	iop.set_color_nb(nCol);
	iop.bb_node_state_active.get_bitboard(iop.NB_OF_BB_NODES)=0;			//may contain added nodes!!
	//iop.update_color_sizes(nCol);	
	//iop.set_node_state_active(LISTA_BB(depth));	
		
	if(nCol==kmin){
		if(iop.filter()==true){
			//res.inc_counter(0,1);
			LISTA_L(depth).index=EMPTY_ELEM;
			return;
		}
	}
	
	//***TODO attempt lighter maxsatz

	int nb_conf=iop.init_maxsatz(v_expanded, kmin);
	if(nb_conf==EMPTY_ELEM){
		//LOG_INFO("CUT NORMAL INFRACHROM");
		//res.inc_counter(0,1);
		LISTA_L(depth).index=EMPTY_ELEM;
	}

}else{
	//LOG_INFO("LOOSE COLORING:"<<nCol<<":"<<kmin);
}


return;
}



inline
void CliqueInfra::paint_R_kmin_comb_inc(int depth, int v_expanded){
//////////////////////////
// Tomita greedy SEQ with recoloring and later infra-chrom
// date of creation: 02/04/16
//

	
	int cmax=1; int kmin=maxno-depth; int nBB=EMPTY_ELEM; int v=EMPTY_ELEM; 
	LISTA_L(depth).index=EMPTY_ELEM;								
	const int DEPTHPLUS1=depth+1;
	const int NB_OF_BB_NODES_MINUS_ONE=iop.NB_OF_BB_NODES-1;
	const int KMIN_MINUS_ONE=kmin-1;
	
		
	//colors up to kmin without updating color set sizes
	int pc=(m_unsel=LISTA_BB(depth)).popcn64();
	while(true){
		bitarray& bbcmax=iop.m_colSets[cmax].bb;
		for(int i=0; i<iop.NB_OF_BB_NODES; i++){
			bbcmax.get_bitboard(i)=m_unsel.get_bitboard(i);
		}

		bbcmax.init_scan(bbo::NON_DESTRUCTIVE);
		while(true){
			v=bbcmax.next_bit(nBB, m_unsel);
			if(v==EMPTY_ELEM) break;
			

			//stores color label
			iop.node_iset_no[v]=cmax;
		//	m_lcol[DEPTHPLUS1][v]=cmax;
		
			if((--pc)==0){
				if(cmax==1){
					int v=LISTA_BB(depth).lsbn64();
					LISTA_L(depth).nodos[++LISTA_L(depth).index]=v;
					m_lcol[DEPTHPLUS1][v]=1;
				}
				 return;
			}
			//actual coloring
			bitarray& nbor=g->get_neighbors(v);
			for(int i=nBB; i<iop.NB_OF_BB_NODES; i++){
				bbcmax.get_bitboard(i)&= ~nbor.get_bitboard(i);
			}
		
		}
			
		//colors up to kmin
		if(++cmax>=kmin)
			break;
	}//next color


////////////////////////
//Recoloring (re-number)
//	
	if(kmin>=3) {
		m_unsel.init_scan(bbo::NON_DESTRUCTIVE);
		while(true){
next_v:		int v=m_unsel.next_bit();
			if(v==EMPTY_ELEM) break;
			for(int recol=1; recol<KMIN_MINUS_ONE; recol++){		//loop to find initial color seed

				//check if color is valid for swapping (0-1 neighbors)	
				int vswap;
				int pc_swap=iop.m_colSets[recol].bb.single_disjoint(0, NB_OF_BB_NODES_MINUS_ONE, g->get_neighbors(v), vswap);

				//analysis
				if(pc_swap==1){	//color class found

					//Busca swap con vertice ya coloreado
					for(int j=recol+1; j<(kmin); j++){
						//for(int j=kmin-1/*1 reverse directon; j>/*=1*/recol; j--){
						if(j==recol) continue;

						if( iop.m_colSets[j].bb.is_disjoint(0, NB_OF_BB_NODES_MINUS_ONE,g->get_neighbors(vswap))){

							//swap action
							iop.m_colSets[j].bb.set_bit(vswap);
							iop.m_colSets[recol].bb.set_bit(v);
							iop.m_colSets[recol].bb.erase_bit(vswap);
							iop.m_colSets[cmax].bb.erase_bit(v);
							m_lcol[DEPTHPLUS1][vswap]=j;
							m_lcol[DEPTHPLUS1][v]=recol;
							iop.node_iset_no[vswap]=j;
							iop.node_iset_no[v]=recol;
						
							m_unsel.erase_bit(v);

							//empty check of unsel in case vertex swapped is the last one
							if((--pc)==0){
								return;
							}else goto next_v;
						}
					}
				}else if(pc_swap==0){					//disjoint color class found (previous swap necessary)
					iop.m_colSets[recol].bb.set_bit(v);
					m_lcol[DEPTHPLUS1][v]=recol;
					iop.node_iset_no[v]=recol;

					m_unsel.erase_bit(v);
					if((--pc)==0){
						return;
					}else goto next_v;
				}

			}//next candidate for swap color seed
		}
	}


//////////////////
//attempt with vertices above kmin

	int nBBv;	bool iscol; 
	iop.m_colSets[cmax].erase_bit();
	v=m_unsel.lsbn64();
	m_unsel.erase_bit(v);

	m_lcol[DEPTHPLUS1][v]=cmax;
	iop.update_color_sizes(cmax-1);
	ERASE(LISTA_BB(depth), m_unsel, m_sel);
	iop.set_node_state_active(m_sel);	
	iop.set_color_nb(cmax-1);
	iop.init_inc_maxsatz();

	//new vertex in singleton cmax
	iop.set_color_nb(cmax);
	iop.m_colSets[cmax].push(v);
	iop.node_iset_no[v]=cmax;
	iop.color_unit_stack.push(cmax);
	iop.bb_node_state_active.set_bit(v);
	iop.color_state_active.set_bit(cmax);
		
	int w=v;
	if(iop.inc_maxsatz(v)!=EMPTY_ELEM){		//END INFRA-CHROM CUT
		goto outer;
	}else{
		if((--pc)==0){
			iop.reset_enlarged_isets();
			return;				//SUCCESS
		}
		//color a la Tomita
		bool cut_active=true;
		m_unsel.init_scan(bbo::DESTRUCTIVE);
		while(true){
			w=m_unsel.next_bit_del(nBBv);
			if(w==EMPTY_ELEM){
				iop.reset_enlarged_isets();
				goto outer;
			}			

			//inner loop: search for SEQ color for v
			iscol=false;
			for(int col=cmax; col<=cmax; col++){
				iscol=g->get_neighbors(w).is_disjoint(0,nBBv, iop.m_colSets[col].bb);			//**TODO optimize? note: the reasoning does not include added nodes

				//color found for vertex
				if(iscol){
					iop.m_colSets[col].push(w);					//adds vertex and updates bitstring
					iop.bb_node_state_active.set_bit(w);
					iop.color_state_active.set_bit(col);
					iop.node_iset_no[w]=col;
					m_lcol[DEPTHPLUS1][w]=col;
					iscol=true;
					break;
				}
			}

			//color not found: new color
			if(!iscol){
				iop.m_colSets[++cmax].erase_bit();		//clears color (previous use in same level)
				iop.m_colSets[cmax].push(w);
				iop.color_state_active.set_bit(cmax);
				iop.bb_node_state_active.set_bit(w);
				iop.node_iset_no[w]=cmax;
				iop.set_color_nb(cmax);
				m_lcol[DEPTHPLUS1][w]=cmax;
				iop.color_unit_stack.push(cmax);
			}

			//attemtp to cut for the first kmin vertices
			if(iop.inc_maxsatz(w)!=EMPTY_ELEM){
				goto outer;
				//LISTA_L(depth).nodos[++LISTA_L(depth).index]=v;
				
			}
						
			if((--pc)==0){
				iop.reset_enlarged_isets();
				return;		//SUCCESS
			}
		}//pick next vertex to col
	}

/////////////////
// Final attempt to reduce the last colors: 

outer: //infrachrom in full for the rest of nodes
	
	iop.reset_enlarged_isets();			/*forced*/
	iop.set_color_nb(cmax);	
	if(iop.m_colSets[cmax].bb.is_empty()){
		LOG_ERROR("EMPTY_COLOR_SET");
	}
//	iop.bb_node_state_active.get_bitboard(iop.NB_OF_BB_NODES)=0;	
	//treatment of last vertex, which has not been cut
	if(w==EMPTY_ELEM){
		LOG_ERROR("LAST VERTEX UNKNWON: BIZARRE");
	}
	int nb_conf=iop.init_maxsatz(kmin-1);
	if(nb_conf!=EMPTY_ELEM){
		//LOG_INFO("CUT NORMAL INFRACHROM");
		//res.inc_counter(0,1);
		LISTA_L(depth).nodos[++LISTA_L(depth).index]=w;
	}
	if((--pc)==0){return;}

	
	//Tomita normal coloring and finish
	m_unsel.init_scan(bbo::DESTRUCTIVE);
	while(true){
		int v=m_unsel.next_bit_del(nBBv);
		iscol=false;
		for(int col=cmax; col<=cmax; col++){
			iscol=g->get_neighbors(v).is_disjoint(0,nBBv, iop.m_colSets[col].bb);			//**TODO optimize? note: the reasoning does not include added nodes

			//color found for vertex
			if(iscol){
				iop.m_colSets[col].push(v);					//adds vertex and updates bitstring
				iop.bb_node_state_active.set_bit(v);
				//iop.color_state_active.set_bit(col);
				iop.node_iset_no[v]=col;
				m_lcol[DEPTHPLUS1][v]=col;
				iscol=true;
				break;
			}
		}

		//color not found: new color
		if(!iscol){
			iop.m_colSets[++cmax].erase_bit();		//clears color (previous use in same level)
			iop.m_colSets[cmax].push(v);
			iop.color_state_active.set_bit(cmax);
			iop.bb_node_state_active.set_bit(v);
			iop.node_iset_no[v]=cmax;
			iop.set_color_nb(cmax);
			m_lcol[DEPTHPLUS1][v]=cmax;
			//iop.color_unit_stack.push(cmax);
		}



		//iop.bb_node_state_active.get_bitboard(iop.NB_OF_BB_NODES)=0;
		//iop.bb_node_state_active.print(); cout<<endl;
		//cout<<"--------------------------------------"<<endl;
		//int size=0;
		//for(int i=1; i<=cmax; i++){
		//	iop.m_colSets[i].bb.print();
		//	size+=iop.m_colSets[i].get_size();
		//	cout<<endl;
		//}
	
		//if(size!=iop.bb_node_state_active.popcn64()){
		//	LOG_ERROR(size<<":"<<iop.bb_node_state_active.popcn64());
		//}
		//		
		//cout<<"--------------------------------------"<<endl;


		int nb_conf=iop.init_maxsatz(kmin-1);			//**reuse color unit stack
		if(nb_conf!=EMPTY_ELEM){
			//LOG_INFO("CUT NORMAL INFRACHROM");
			//res.inc_counter(0,1);
			LISTA_L(depth).nodos[++LISTA_L(depth).index]=v;
		}



		//LISTA_L(depth).nodos[++LISTA_L(depth).index]=v;			
		if((--pc)==0){
			return;		//SUCCESS
		}
	}//pick next vertex to col



//int nCol=cmax;
//if((nCol-kmin)<20 /* max 64 conflicts*/){
//
//	//***if(iop.m_colSets[cmax].bb.is_empty()){ nCol--; }
//	iop.set_color_nb(nCol);
//	iop.bb_node_state_active.get_bitboard(iop.NB_OF_BB_NODES)=0;			//may contain added nodes!!
//	//iop.update_color_sizes(nCol);	
//	//iop.set_node_state_active(LISTA_BB(depth));	
//		
//	if(nCol==kmin){
//		if(iop.filter()==true){
//			//res.inc_counter(0,1);
//			LISTA_L(depth).index=EMPTY_ELEM;
//			return;
//		}
//	}
//	
//	//***TODO attempt lighter maxsatz
//
//	int nb_conf=iop.init_maxsatz(v_expanded, kmin);
//	if(nb_conf==EMPTY_ELEM){
//		//LOG_INFO("CUT NORMAL INFRACHROM");
//		//res.inc_counter(0,1);
//		LISTA_L(depth).index=EMPTY_ELEM;
//	}
//
//}else{
//	//LOG_INFO("LOOSE COLORING:"<<nCol<<":"<<kmin);
//}

return;
}


 inline
void CliqueInfra::paint_R_kmin_sel(int depth){
//////////////////////////
// Tomita greedy SEQ with recoloring with infra-chrom kmin cut 
// in the selective framework (BBMXRL_KMIN)
// date of creation: 14/09/16
// COMMENTS: 
// main variant currently, and a first clear improvement over prior BBMCX
	
	int cmax=1, kmin=maxno-depth, nBB=EMPTY_ELEM, v=EMPTY_ELEM, vswap; 
	LISTA_L(depth).index=EMPTY_ELEM;								
	const int NB_OF_BB_NODES_MINUS_ONE=iop.NB_OF_BB_NODES-1, KMIN_MINUS_ONE=kmin-1;
			
	//init infrachrom structures
	iop.bb_node_state_active.erase_bit();
	iop.color_state_active.erase_bit();


	int pc=(m_unsel=LISTA_BB(depth)).popcn64();
	while(true){
		bitarray& bbcmax=iop.m_colSets[cmax].bb;
		for(int i=0; i<iop.NB_OF_BB_NODES; i++){
			bbcmax.get_bitboard(i)=m_unsel.get_bitboard(i);
		}

		iop.m_colSets[cmax].size=0;
		bbcmax.init_scan(bbo::NON_DESTRUCTIVE);
		while(true){
			v=bbcmax.next_bit(nBB, m_unsel);
			if(v==EMPTY_ELEM) break;
			
			//stores color label
			iop.node_iset_no[v]=cmax;
			iop.m_colSets[cmax].size++;				//updates size, v is already there
			iop.bb_node_state_active.set_bit(v);

			if((--pc)==0){
				if(cmax==1){
					int v=LISTA_BB(depth).lsbn64();
					LISTA_L(depth).nodos[++LISTA_L(depth).index]=v;
				}
				 return;
			}
			//actual coloring
			bitarray& nbor=g->get_neighbors(v);
			for(int i=nBB; i<iop.NB_OF_BB_NODES; i++){
				bbcmax.get_bitboard(i)&= ~nbor.get_bitboard(i);
			}
	
		}
			
		//colors up to kmin
		if(++cmax>=kmin)
			break;
	}//next color

////////////////////////
//Recoloring (re-number)
//	
	if(kmin>=3) {
		m_unsel.init_scan(bbo::NON_DESTRUCTIVE);
		while(true){
next_v:		int v=m_unsel.next_bit();
			if(v==EMPTY_ELEM) break;
			for(int recol=1; recol<KMIN_MINUS_ONE; recol++){		
				int pc_swap=iop.m_colSets[recol].bb.single_disjoint(0, NB_OF_BB_NODES_MINUS_ONE, g->get_neighbors(v), vswap);
				if(pc_swap==1){
					for(int j=recol+1; j<kmin; j++){
						//for(int j=kmin-1/*1 reverse directon; j>/*=1*/recol; j--){
						if(j==recol) continue;

						if( iop.m_colSets[j].bb.is_disjoint(0, NB_OF_BB_NODES_MINUS_ONE,g->get_neighbors(vswap))){

							//swap action
							iop.m_colSets[j].push(vswap);
							iop.m_colSets[recol].push(v);
							iop.m_colSets[recol].erase_bit(vswap);
							iop.m_colSets[cmax].bb.erase_bit(v);		//no need to update size

							iop.node_iset_no[vswap]=j;
							iop.node_iset_no[v]=recol;
						
							m_unsel.erase_bit(v);
							iop.bb_node_state_active.set_bit(v);

							//empty check of unsel in case vertex swapped is the last one
							if((--pc)==0){
								return;
							}else goto next_v;
						}
					}
				}else if(pc_swap==0){					//disjoint color class found (previous swap necessary)
					iop.m_colSets[cmax].bb.erase_bit(v);
					iop.m_colSets[recol].push(v);
					iop.node_iset_no[v]=recol;
					m_unsel.erase_bit(v);
					iop.bb_node_state_active.set_bit(v);

					if((--pc)==0){
						return;
					}else goto next_v;
				}

			}//next candidate for swap color seed
		}
	}
		


//////////////////
//attempt with vertices above kmin
// BUGGY-detected memory issues (5/10/17)!! S
// Small random 10 graph it did not work in RELEASE but did work in DEBUG mode
// during CliqueInterdiction Tests

	int nBBv;	bool iscol;
	v=m_unsel.lsbn64();
	m_unsel.erase_bit(v);

	
	//init infrachrom
	iop.set_color_nb(cmax-1);
	iop.init_inc_maxsatz();

	//adds vertex v to infrachromx
	iop.m_colSets[cmax].erase_bit();
	iop.m_colSets[cmax].push(v);
	iop.node_iset_no[v]=cmax;
	iop.set_color_nb(cmax);
	iop.color_unit_stack.push(cmax);
	iop.bb_node_state_active.set_bit(v);
	iop.color_state_active.set_bit(cmax);
			
	if(iop.inc_maxsatz(v)!=EMPTY_ELEM){						//p-maxsat for first vertex which could not be pruned
		LISTA_L(depth).nodos[++LISTA_L(depth).index]=v;
		if((--pc)==0){
			iop.reset_enlarged_isets();
			return;
		}
		
		//store as candidates and return
		m_unsel.init_scan(bbo::DESTRUCTIVE);
		while(true){
			int v=m_unsel.next_bit_del();
			if(v==EMPTY_ELEM) return;
			LISTA_L(depth).nodos[++LISTA_L(depth).index]=v;			
		}
	}else{

		if((--pc)==0){
			iop.reset_enlarged_isets();
			return;
		}
		//color a la Tomita-pmaxsat for remaining vertices
		m_unsel.init_scan(bbo::DESTRUCTIVE);				
		while(true){
			int v=m_unsel.next_bit_del(nBBv);
			if(v==EMPTY_ELEM){
				iop.reset_enlarged_isets();
				break;
			}			

			//inner loop: search for SEQ color for v
			iscol=false;
			for(int col=cmax; col<=cmax; col++){
				iscol=(iop.m_colSets[col].bb.get_bitboard(iop.NB_OF_BB_NODES)==0) && g->get_neighbors(v).is_disjoint(0,nBBv, iop.m_colSets[col].bb);			//**TODO optimize? note: the reasoning does not include added nodes

				//color found for vertex
				if(iscol){
					iop.m_colSets[col].push(v);					//adds vertex and updates bitstring
					iop.bb_node_state_active.set_bit(v);
					//iop.color_state_active.set_bit(col);
					iop.node_iset_no[v]=col;
					iscol=true;
					break;
				}
			}

			//color not found: new color
			if(!iscol){
				iop.m_colSets[++cmax].erase_bit();		//clears color (previous use in same level)
				iop.m_colSets[cmax].push(v);
				iop.color_state_active.set_bit(cmax);
				iop.bb_node_state_active.set_bit(v);
				iop.node_iset_no[v]=cmax;
				iop.set_color_nb(cmax);
				iop.color_unit_stack.push(cmax);
			}

			//attemtp to cut for the first kmin vertices
			//if(iop.inc_maxsatz(v)!=EMPTY_ELEM ){					
			if(!iscol && iop.inc_maxsatz(v)!=EMPTY_ELEM){			//possible if inconsistent sets are tagged
				LISTA_L(depth).nodos[++LISTA_L(depth).index]=v;

				//store as candidates in order and return
				m_unsel.init_scan(bbo::DESTRUCTIVE);
				while(true){
					int v=m_unsel.next_bit_del();
					if(v==EMPTY_ELEM){ 
						iop.reset_enlarged_isets();
						return;
					}
					LISTA_L(depth).nodos[++LISTA_L(depth).index]=v;			
				}
			}
						
			if((--pc)==0){
				iop.reset_enlarged_isets();
				return;		//SUCCESS
			}
		}//pick next vertex to col
	}
 } 
#endif


