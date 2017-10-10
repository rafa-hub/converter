////////////////////////////////
// ub_weighted_clique.h: interface for the UBWC class which contains heuristics
//                       to determine and upperbound for teh MWCP 
//
// initial date:11/08/17
// last update: 11/08/17
// author: pablo san segundo

#ifndef  __UB_WEIGHTED_CLIQUE_H__
#define  __UB_WEIGHTED_CLIQUE_H__

#include "../clique.h"
#include "../../../bitscan/bbalg.h"
#include "../../../utils/common.h"
#include "../../../graph/algorithms/graph_map.h"

using namespace com;												//for common types 
typedef vector<int> vint;

///////////////
//switches
//#define COVER					//uses some form of cover once a node is found prunable, to prune other nodes (DEFAULT-ON)
#ifdef COVER
//#define COVER_BY_COLORING		//color cover vs. relaxed
#endif

class UBWC{
////////////////////////
// upper bound heuristics for weighted clique
	enum top_t {NONE=0, TOP1, TOP2, GAP, TOP_OVERLAP, ISET_FREE, NEW_CLAUSE_WEIGHT};
	static const int SIZE_3_ISET = CLQ_MAXINT-1;		
public:
	static  int greedy_lb (const ugraph& g, const bitarray& bbsg);	
	static  int UB_sum	  (const ugraph& g, bitarray& bbsg /* not constant to iterate*/, int KMIN=CLQ_MAXINT);
	static  int UB_col	  (const ugraph& g, const bitarray& bbsg, int KMIN=CLQ_MAXINT);								/* color bound up to KMIN */
private:
	ugraph* g;				/*the one and only weighted graph (may be NULL as required by framework)*/		
	int** tw;				/*[COL][k=0,1] top-k weights for each clause, top-0 is the weight of the clause */
	int** fn;				/*[COL][3] first nodes of clauses (up to a maximum of 3)*/
	bba_t<bitarray> db;		/*[COL][NODES]*/
	int m_NV;						/* required because of set_graph(...) by tyhe framework*/
	com::stack_t<int> nodes;		/*[NODES] auxiliary node stack */
	com::stack_t<int> colors;		/*[COLORS] auxiliary node stack */
	com::stack_t<int> node_cover;	/*[NODES] auxiliary node stack for overlap filter */
	bitarray sel;					/* auxiliary bitset for coloring*/

//////////
// variable weights and color sets
	int** cw;				/*[COL][k=0,1,2] variable weights for each node (initially original weights) */
	int NB_COL;
	int FIRST_NEW_COL;		/*pointer to the first new color set*/
	int GAP_SIZE;
public:
	bitarray unsel;			/* stores candidate nodes */

public:
	UBWC(ugraph* g=NULL)			:g(g), tw(NULL), fn(NULL), cw(NULL), NB_COL(1), FIRST_NEW_COL(CLQ_MAXINT), GAP_SIZE(0){ if(g) m_NV=g->number_of_vertices();};
	virtual ~UBWC()					{clear();}
	void set_graph(ugraph* g);		
		
	const ugraph& get_graph() const	{return *g;}
	void init();
	void clear();
	bba_t<bitarray>& get_db()		{return db;}
	bitarray& get_unsel()			{return unsel;}
	int* get_tk(int col)			{return tw[col];}
	int number_of_colors()			{return NB_COL;}
	int get_gap()					{return GAP;}
	
////////////
// main procedures	
	 int paint			(const bitarray& bbsg, const int KMIN, int& nb_col, com::stack_t<int>& cand);			/* SEQ coloring of subgraph <= KMIN (g must be ordered non-increasing weights)*/
	 int paint_and_tk	(const bitarray& bbsg, const int KMIN, int& nb_col, com::stack_t<int>& cand);			/* SEQ + update top_k (g must be ordered non-increasing weights)*/
	 int paint_and_map	(const bitarray& bbsg, const int KMIN, const GraphMap& gm, UBWC& dest, int& nb_col);	/* for tests-computes color sets for weighted graph */
	 int paint_and_map_OPT (const bitarray& bbsg, const int KMIN, const GraphMap& gm, UBWC& dest, int& nb_col);	/*reference*/	

	 int set_tk			(int col, int type_k, int val) {tw[col][type_k]=val;}	
	 int set_tk			(int col_ini, int col_end);																/* determines top_k and caches first 3 nodes of each color set (g must be ordered non-increasing weights) */
	 void update_tk		(int col);
	 void update_CW		(int col);
/////////
//I/O
	ostream& print_tk	(int col_ini, int col_end, ostream& o=cout);
	ostream& print_db	(int col_ini, int col_end, ostream& o=cout);	/* includes weight info*/

//////////////
//covering when nodes ordered by weights (currently deprecated)
	int cover						(int& gap, int nb_col);													/* two types(overlap and default) + relaxed cover */
	top_t find_gap					(int v, int& gap);	
	top_t find_top_k				(int v, int col);	
	top_t find_top_k_overlap		(int v, int col_ini, int col_end);	

/////////////
// covering using 2 graph isomorphisms (current reference)
	int cover_with_overlap			(int nb_col, UBWC& dest, const GraphMap& gm);	
	int cover						(int& gap, int nb_col, UBWC& dest, const GraphMap& gm);	
	top_t find_top_k_shared			(int v, int col);														/* simplified variant of find_top_k for shared covering*/	
	top_t find_top_k_shared_with_swap (int v, int col);

/////////////
// covering with variable color sets (uses global-thus variable- NB_COL)
	int paint_and_map_CW		(const bitarray& bbsg, const int KMIN, const GraphMap& gm, UBWC& dest);		
	int paint_and_map_CW_3S		(const bitarray& bbsg, const int KMIN, const GraphMap& gm, UBWC& dest);		/* with SIZE 3 information (1) */
	int paint_and_map_CW_dyn	(const bitarray& bbsg, const int KMIN, const GraphMap& gm, UBWC& dest);	    /* DEVELOPING-currently BUGGY! */


	int cover_CW				(/*int& gap,*/ UBWC& dest, const GraphMap& gm);
	int cover_CW_2R				(/*int& gap,*/ UBWC& dest, const GraphMap& gm);
	int cover_CW_3S				(/*int& gap,*/ UBWC& dest, const GraphMap& gm);
	int cover_CW_3S_2R			(/*int& gap,*/ UBWC& dest, const GraphMap& gm);
	top_t find_top_k_CW			(int v, int col);				/* deprecated */
	top_t find_top_k_CW_OPT		(int v, int col);				/* deprecated */
	top_t find_top_k_CW_OPT_T2split		(int v, int col);		/* reference */
	top_t find_top_k_CW_OPT_T2split_3S	(int v, int col);		/* reference */								/* with SIZE 3 information, combined with (1) (TODO-CHECK PROPERLY)*/

	top_t find_top_k_CW_OPT_T2split		(int v, int col, int NEW_COL_POINTER /* pos of a fresh ISET split*/);	/* to be used during paint */


	top_t find_top_k_overlap_CW			(int v);	
	top_t find_top_k_overlap_CW_plus	(int v);					/* to TEST properly */
	int cover_with_overlap_CW			(UBWC& dest, const GraphMap& gm);	

////////////
// covering using weight ordering
	int cover_CW_Ord				(/*int& gap,*/ UBWC& dest, const GraphMap& gm);
	int cover_with_overlap_CW_Ord	(UBWC& dest, const GraphMap& gm);	

////////////
// covering  with residual weights for ISETS -BUGGY!
	int cover_CW_resw						(/*int& gap,*/ UBWC& dest, const GraphMap& gm);
	top_t find_top_k_CW_OPT_T2split_resw	(int v, int col, int& wv, com::stack_t<int>& c_stack /*, com::stack_t<int>& n_stack*/);			/* reference */

///////////
// gap related
	int increase_gap();				/* attempts to reduce GAP_SIZE (DEVELOPING-NOT WORKING)*/
};

inline
void UBWC::set_graph(ugraph* g) {
	clear(); 
	if(g==NULL){
		LOG_ERROR("UBWC::set_graph()-setting invalid graph");
		return;
	}
	this->g=g;
	m_NV=g->number_of_vertices(); 	
}

inline
int UBWC::cover (int& gap, int nb_col){
/////////////////////
// Computes relaxed cover of nodes in unsel
//
// RETURNS:
//
// COMMENTS: Only for graphs ordered by non-increasing weights!
	
	top_t res;
	com::stack_t<int> nodes(g->number_of_vertices());		/** TODO-Optimize allocation! */
	unsel.init_scan(bbo::DESTRUCTIVE);
	while(true){
		int v=unsel.next_bit_del();					
		if(v==EMPTY_ELEM) break;
		
		//attemtp to cover v in full with gap
		if(g->get_wv(v)<=gap){
			res=find_gap(v,gap);					/* 1.relaxed cover GAP, 2.updates gap */
			if(res==GAP)
				continue;
		}

#ifdef OVERLAP
		//cover with overlap
		res=find_top_k_overlap(v,1,nb_col);
#else
		//attempt cover v in full with TOP_2 and, in case not possible, TOP_1
		for(int c=1; c<=nb_col; c++){
			res=find_top_k(v,c);					/* 1.relaxed cover TOP_1, 2.updates tw */
			if(res!=NONE) break;
		}
#endif

		if(res==NONE)
			nodes.push(v);

	}//next vertex
	
	//reset unsel with remaining nodes-TESTS
	for(int i=0; i<nodes.size(); i++){
		unsel.set_bit(nodes.get_elem(i));
	}

	return 0;
}

//inline
//int UBWC::cover (int& gap, int nb_col, UBWC& dest, const GraphMap& gm){
//////////////////////
//// covers nodes in 'unset' in current space using ISETS computed in 'dest' space
////
//// COMMENTS: Could be an important contribution!
//	
//	top_t res; 
//	int nBB=0;
//	unsel.init_scan(bbo::NON_DESTRUCTIVE);
//	while(true){
//		int v=unsel.next_bit(nBB);
//		if(v==EMPTY_ELEM) break;
//		int wv=g->get_wv(v);
//
//		//attemtp to cover v in full with gap
//		res=find_gap(v,gap);															/* 1.relaxed cover GAP, 2.updates gap */
//		if(res==GAP){
//			unsel.erase_bit(v);	
//			continue;
//		}	
//
//		//attempt cover v in full with TOP_2 and, in case not possible, TOP_1
//		for(int c=1; c<=nb_col; c++){
//			res=dest.find_top_k_shared_tests(gm.map_l2r(v),c);						/* 1.updates tw */
//			if(res!=NONE){
//				unsel.erase_bit(v);	
///////////////////////////////////////////////////////			
////relaxed cover: optimized by blocks-CHECK!
//				//int MAX_BB=sel.number_of_bitblocks();
//				//if(res==TOP1){		
//				//	for(int i=nBB; i<MAX_BB; i++){
//				//		sel.get_bitboard(i)=unsel.get_bitboard(i)& ~(g->get_neighbors(v).get_bitboard(i) | g->get_neighbors(gm.map_r2l(dest.fn[c][0])).get_bitboard(i) );
//				//	}
//				//}else if(res==TOP2){	
//				//	for(int i=nBB; i<MAX_BB; i++){
//				//		sel.get_bitboard(i)=unsel.get_bitboard(i)& ~( g->get_neighbors(v).get_bitboard(i) |
//				//							g->get_neighbors(gm.map_r2l(dest.fn[c][0])).get_bitboard(i)  |
//				//							g->get_neighbors(gm.map_r2l(dest.fn[c][1])).get_bitboard(i)    );
//				//	}
//				//}
//
//				////manual scanning configuration (must be NON DESTRUCTIVE)
//				//int w=0;
//				//sel.set_bbindex(nBB);
//				//sel.set_posbit(WMOD(v));	/* forced!-scan after v */
//				//while(true){
//				//	int u=sel.next_bit();
//				//	if(u==EMPTY_ELEM) break;
//
//				//	w+=g->get_wv(u);
//				//	if(w<=wv){
//				//		unsel.erase_bit(u);	
//				//		/*if(res==TOP1)
//				//			LOG_INFO("TOP-1_COVER:"<<u);
//				//		else
//				//			LOG_INFO("TOP-2_COVER:"<<u);*/
//				//	}
//				//	else w-=g->get_wv(u);
//				//}
//
////END of relaxed cover
///////////////////////////////////////////////////////////			
//				break;
//			}//end of conditional cover
//		}//next color to test
//	}//next vertex
//		
//	return 0;
//}

inline
int UBWC::cover_with_overlap (int nb_col, UBWC& dest, const GraphMap& gm){
////////////////////
// covers nodes in 'unset' in current space using ISETS computed in 'dest' space
//
// COMMENTS: Could be an important contribution!
	
	top_t res; 
	unsel.init_scan(bbo::NON_DESTRUCTIVE);
	while(true){
		int v=unsel.next_bit();
		if(v==EMPTY_ELEM) break;
		int wv=g->get_wv(v);
		
		//cover with overlap
		res=dest.find_top_k_overlap(gm.map_l2r(v),1,nb_col);							/* TODO- CHECK! */	
		if(res!=NONE){
			//LOG_INFO("OVERLAP:"<<v);
			unsel.erase_bit(v);	
		}
	}//next vertex
		
	return 0;
}

inline
int UBWC::cover_with_overlap_CW (UBWC& dest, const GraphMap& gm){
////////////////////
// covers nodes in 'unset' in current space using ISETS computed in 'dest' space
//
// COMMENTS: Could be an important contribution!
	
	top_t res; 
	unsel.init_scan(bbo::NON_DESTRUCTIVE);
	while(true){
		int v=unsel.next_bit();
		if(v==EMPTY_ELEM) break;
				
		res=dest.find_top_k_overlap_CW(gm.map_l2r(v));							/* TODO- CHECK! */	
		if(res!=NONE){
			//LOG_INFO("OVERLAP:"<<v);
			unsel.erase_bit(v);	
		}
	}//next vertex
		
	return 0;
}

inline
int UBWC::cover_with_overlap_CW_Ord (UBWC& dest, const GraphMap& gm){
////////////////////
// covers_with_overlap using weighted order (less weight first)
//
		
	top_t res; 

	gm.map_l2r(unsel,dest.unsel);
	
//	dest.unsel.init_scan(bbo::NON_DESTRUCTIVE);
	dest.unsel.init_scan(bbo::NON_DESTRUCTIVE_REVERSE);

	while(true){
	//	int v=dest.unsel.next_bit();
		int v=dest.unsel.previous_bit();
		if(v==EMPTY_ELEM) break;
				
		res=dest.find_top_k_overlap_CW(/*gm.map_l2r(v)*/v);									
	//	res=dest.find_top_k_overlap_CW_plus(/*gm.map_l2r(v)*/v);							/* TODO- CHECK! */	
		if(res!=NONE){
			//LOG_INFO("OVERLAP:"<<v);
			dest.unsel.erase_bit(v);	
		}
	}//next vertex

	gm.map_r2l(unsel,dest.unsel);
		
	return 0;
}

inline
int UBWC::cover (int& gap, int nb_col, UBWC& dest, const GraphMap& gm){
/////////////////////
// covers nodes in 'unset' in current space using ISETS computed in 'dest' space
//
// COMMENTS: Could be an important contribution!
	
	top_t res; 
	int nBB=0;
	unsel.init_scan(bbo::NON_DESTRUCTIVE);
	while(true){
		int v=unsel.next_bit(nBB);
		if(v==EMPTY_ELEM) break;
		int wv=g->get_wv(v);

		//attemtp to cover v in full with gap: -IMPROBABLE
		if(wv<=gap){
			unsel.erase_bit(v);
			gap-=wv;
//////////////////////
//relaxed cover for GAP

			int MAX_BB=sel.number_of_bitblocks();
			for(int i=nBB /*0*/; i<MAX_BB; i++){
				sel.get_bitboard(i)=unsel.get_bitboard(i)& ~(g->get_neighbors(v).get_bitboard(i) );
			}
			int w=0;
			sel.set_bbindex(nBB);
			sel.set_posbit(WMOD(v));	/* forced!-scan after v */
			while(true){
				int u=sel.next_bit();
				if(u==EMPTY_ELEM) break;

				w+=g->get_wv(u);
				if(w<=wv){
					unsel.erase_bit(u);	
					LOG_INFO("GAP_COVER:"<<u);
					cin.get();
				}
				else w-=g->get_wv(u);
			}//next vertex to cover
			//////////////////////////
			continue;
		}		
	
		//attempt cover v in full with TOP_2 and, in case not possible, TOP_1
		for(int c=1; c<=nb_col; c++){
			res=dest.find_top_k_shared(gm.map_l2r(v),c);								/* 1.updates tw */
		//	res=dest.find_top_k_shared_with_swap(gm.map_l2r(v),c);						/* 1.updates tw */
			if(res!=NONE){
				unsel.erase_bit(v);	
/////////////////////////////////////////////////////			
//relaxed cover: optimized by blocks-CHECK!
#ifdef COVER
				int MAX_BB=sel.number_of_bitblocks();
				if(res==TOP1){		
					for(int i=nBB /*0*/; i<MAX_BB; i++){
						sel.get_bitboard(i)=unsel.get_bitboard(i)& ~(g->get_neighbors(v).get_bitboard(i) | g->get_neighbors(gm.map_r2l(dest.fn[c][0])).get_bitboard(i) );
					}
				}else if(res==TOP2){	
					for(int i=nBB /*0*/; i<MAX_BB; i++){
						sel.get_bitboard(i)=unsel.get_bitboard(i)& ~( g->get_neighbors(v).get_bitboard(i) |
											g->get_neighbors(gm.map_r2l(dest.fn[c][0])).get_bitboard(i)  |
											g->get_neighbors(gm.map_r2l(dest.fn[c][1])).get_bitboard(i)    );
					}
				}

				//manual scanning configuration (must be NON DESTRUCTIVE)
				int w=0;
				sel.set_bbindex(nBB);
				sel.set_posbit(WMOD(v));	/* scan after v */
				/*sel.set_bbindex(0);
				sel.set_posbit(MASK_LIM);	*/
				while(true){
					int u=sel.next_bit();
					if(u==EMPTY_ELEM) break;
					
#ifdef COVER_BY_COLORING
					//color cover
					if(g->get_wv(u)>wv) continue;
					//LOG_INFO("TOP-1_COVER:"<<u<<" by "<<v);
					unsel.erase_bit(u);
					sel.erase_block(nBB,g->get_neighbors(u));
#else 

					//relaxed cover
					w+=g->get_wv(u);
					if(w<=wv){
						unsel.erase_bit(u);	
						/*if(res==TOP1)
							LOG_INFO("TOP-1_COVER:"<<u);
						else
							LOG_INFO("TOP-2_COVER:"<<u);*/
					}
					else w-=g->get_wv(u);

#endif //COVER BY COLORING

				}
#endif //COVER


//END of relaxed cover
/////////////////////////////////////////////////////////			
				break;
			}//end of conditional cover
		}//next color to test
	}//next vertex
		
	return 0;
}

inline
int UBWC::cover_CW (/*int& gap,*/ UBWC& dest, const GraphMap& gm){
/////////////////////
// covers nodes in 'unset' in current space using ISETS computed in 'dest' space
//
// COMMENTS: Could be an important contribution!
	
	
	top_t res; 
	int nBB=0;
	unsel.init_scan(bbo::NON_DESTRUCTIVE);
	while(true){
		int v=unsel.next_bit(nBB);
		if(v==EMPTY_ELEM) break;
		int wv=g->get_wv(v);

		//attemtp to cover v in full with gap: -VERY IMPROBABLE
//		if(wv<=dest.GAP_SIZE){
//			unsel.erase_bit(v);	
//			dest.GAP_SIZE-=wv;
//			LOG_INFO("GAP COVERED:"<<v);
//	//		cin.get();
////////////////////////
////**TODO-relaxed cover for GAP
//			//LOG_INFO("GAP TO BE COVERED");
//			//int MAX_BB=sel.number_of_bitblocks();
//			//for(int i=nBB /*0*/; i<MAX_BB; i++){
//			//	sel.get_bitboard(i)=unsel.get_bitboard(i)& ~(g->get_neighbors(v).get_bitboard(i) );
//			//}
//			//int w=0;
//			//sel.set_bbindex(nBB);
//			//sel.set_posbit(WMOD(v));	/* forced!-scan after v */
//			//while(true){
//			//	int u=sel.next_bit();
//			//	if(u==EMPTY_ELEM) break;
//
//			//	w+=g->get_wv(u);
//			//	if(w<=wv){
//			//		unsel.erase_bit(u);	
//			//		LOG_INFO("GAP_COVER:"<<u);
//			//		cin.get();
//			//	}
//			//	else w-=g->get_wv(u);
//			//}//next vertex to cover
//			////////////////////////////
//			
//			continue;
//		}		
		
		//attempt cover v in full with TOP_2 and, in case not possible, TOP_1
		for(int c=1; c<=dest.NB_COL; c++){
		//	res=dest.find_top_k_CW(gm.map_l2r(v),c);									/* may create new color sets */		
		//	res=dest.find_top_k_CW_OPT(gm.map_l2r(v),c);									/* may create new color sets */		
			res=dest.find_top_k_CW_OPT_T2split(gm.map_l2r(v),c);									/* may create new color sets */		
		//	res=dest.find_top_k_CW_OPT_T2split_3S(gm.map_l2r(v),c);									/* may create new color sets */		
		
			if(res!=NONE){
				unsel.erase_bit(v);				
				break;
			}//end of conditional cover
			
		}//next color to test
		
	}//next vertex

	return 0;
}

inline
int UBWC::cover_CW_resw ( UBWC& dest, const GraphMap& gm){

	int wv=0, nBB=0;
	top_t res; 
	unsel.init_scan(bbo::NON_DESTRUCTIVE);
	while(true){
		int v=unsel.next_bit(nBB);
		if(v==EMPTY_ELEM) break;
		wv=g->get_wv(v);
		colors.erase();
		//nodes.erase();

		/**** wv<=GAP_SIZE ***/
		/*if(wv<=dest.GAP_SIZE){
			unsel.erase_bit(v);	
			dest.GAP_SIZE-=wv;
			LOG_INFO("GAP COVERED:"<<v);
			cin.get();

		}*/	
	
		//attempt cover v in full with TOP_2 and, in case not possible, TOP_1
		for(int c=1; c<=dest.NB_COL; c++){
			res=dest.find_top_k_CW_OPT_T2split_resw(gm.map_l2r(v) ,c, wv /* residual weight */, colors/*,nodes*/);											/* may create new color sets */		
				
			if(res!=NONE){

				//**TODO-update colors
				for(int i=0; i<colors.pt; i++){
					int col=colors.get_elem(i);
					dest.fn[col][2]=dest.fn[col][1];
					dest.cw[col][2]=dest.cw[col][1];
				
					////option-A
					//dest.fn[col][1]=v;
					//dest.cw[col][1]=dest.cw[c][0];

					//option-B
					dest.fn[col][1]=dest.fn[col][0];
					dest.cw[col][1]=dest.cw[col][0];
					dest.fn[c][0]=v;					/* weight is the same */
									
				}
				
				unsel.erase_bit(v);				
				break;
			}//end of conditional cover			
		}//next color 	

	}//next vertex

	return 0;
}


inline
int UBWC::cover_CW_2R (/*int& gap,*/ UBWC& dest, const GraphMap& gm){
/////////////////////
// covers nodes in 'unset' in current space using ISETS computed in 'dest' space
// double round of pruning (reasonable compromise but no more)
//
// COMMENTS: Could be an important contribution!


	nodes.erase(); bool is_pruned=false;
	top_t res; 
	int nBB=0;
	unsel.init_scan(bbo::NON_DESTRUCTIVE);
	while(true){
		int v=unsel.next_bit(nBB);
		if(v==EMPTY_ELEM) break;
		int wv=g->get_wv(v);

		//attemtp to cover v in full with gap: -VERY IMPROBABLE
		if(wv<=dest.GAP_SIZE){
			unsel.erase_bit(v);	
			dest.GAP_SIZE-=wv;
			LOG_INFO("GAP COVERED:"<<v);
	//		cin.get();
//////////////////////
//**TODO-relaxed cover for GAP
			//LOG_INFO("GAP TO BE COVERED");
			//int MAX_BB=sel.number_of_bitblocks();
			//for(int i=nBB /*0*/; i<MAX_BB; i++){
			//	sel.get_bitboard(i)=unsel.get_bitboard(i)& ~(g->get_neighbors(v).get_bitboard(i) );
			//}
			//int w=0;
			//sel.set_bbindex(nBB);
			//sel.set_posbit(WMOD(v));	/* forced!-scan after v */
			//while(true){
			//	int u=sel.next_bit();
			//	if(u==EMPTY_ELEM) break;

			//	w+=g->get_wv(u);
			//	if(w<=wv){
			//		unsel.erase_bit(u);	
			//		LOG_INFO("GAP_COVER:"<<u);
			//		cin.get();
			//	}
			//	else w-=g->get_wv(u);
			//}//next vertex to cover
			////////////////////////////
			
			continue;
		}		
	
		//is_pruned=false;
		//attempt cover v in full with TOP_2 and, in case not possible, TOP_1
		for(int c=1; c<=dest.NB_COL; c++){
		//	res=dest.find_top_k_CW(gm.map_l2r(v),c);									/* may create new color sets */		
		//	res=dest.find_top_k_CW_OPT(gm.map_l2r(v),c);									/* may create new color sets */		
			res=dest.find_top_k_CW_OPT_T2split(gm.map_l2r(v),c);									/* may create new color sets */		
		//	res=dest.find_top_k_CW_OPT_T2split_3S(gm.map_l2r(v),c);									/* may create new color sets */		
		
			if(res!=NONE){
				unsel.erase_bit(v);				
				is_pruned=true;
				break;
			}//end of conditional cover
			
		}//next color to test

		if(!is_pruned)
			nodes.push(v);
	}//next vertex

	//SECOND ROUND-Does not comp
	for(int i=0; i<nodes.pt; i++){
		for(int c=dest.FIRST_NEW_COL; c<=dest.NB_COL; c++){
			int v=nodes.get_elem(i);
			res=dest.find_top_k_CW_OPT_T2split(gm.map_l2r(v),c);									/* may create new color sets */		
		
			if(res!=NONE){
			//	LOG_INFO("VERTEX PRUNED IN SECOND ROUND");
				unsel.erase_bit(v);	
				break;
			}//end of conditional cover
			
		}//next color to test
	}

		
	return 0;
}

inline
int UBWC::cover_CW_3S (/*int& gap,*/ UBWC& dest, const GraphMap& gm){
/////////////////////
// covers nodes in 'unset' in current space using ISETS computed in 'dest' space
//
// COMMENTS: Could be an important contribution!
	
	top_t res; 
	int nBB=0;
	unsel.init_scan(bbo::NON_DESTRUCTIVE);
	while(true){
		int v=unsel.next_bit(nBB);
		if(v==EMPTY_ELEM) break;
		
		//attemtp to cover v in full with gap: -VERY IMPROBABLE
		int wv=g->get_wv(v);
		if(wv<=dest.GAP_SIZE){
			unsel.erase_bit(v);	
			dest.GAP_SIZE-=wv;
			LOG_INFO("GAP COVERED:"<<v);
	//		cin.get();
//////////////////////
//**TODO-relaxed cover for GAP
			//LOG_INFO("GAP TO BE COVERED");
			//int MAX_BB=sel.number_of_bitblocks();
			//for(int i=nBB /*0*/; i<MAX_BB; i++){
			//	sel.get_bitboard(i)=unsel.get_bitboard(i)& ~(g->get_neighbors(v).get_bitboard(i) );
			//}
			//int w=0;
			//sel.set_bbindex(nBB);
			//sel.set_posbit(WMOD(v));	/* forced!-scan after v */
			//while(true){
			//	int u=sel.next_bit();
			//	if(u==EMPTY_ELEM) break;

			//	w+=g->get_wv(u);
			//	if(w<=wv){
			//		unsel.erase_bit(u);	
			//		LOG_INFO("GAP_COVER:"<<u);
			//		cin.get();
			//	}
			//	else w-=g->get_wv(u);
			//}//next vertex to cover
			////////////////////////////

			continue;
		}		
	
		//attempt cover v in full with TOP_2 and, in case not possible, TOP_1
	//	is_pruned=false;
		for(int c=1; c<=dest.NB_COL; c++){
			res=dest.find_top_k_CW_OPT_T2split_3S(gm.map_l2r(v),c);									/* may create new color sets */		
		
			if(res!=NONE){
				unsel.erase_bit(v);									
				break;
			}//end of conditional cover
		}//next color to test
	}//next vertex
	
				
	return 0;
}

inline
int UBWC::cover_CW_3S_2R (/*int& gap,*/ UBWC& dest, const GraphMap& gm){
///////////////////////////
// covers nodes in 'unset' in current space using ISETS computed in 'dest' space
// double round of pruning (reasonable compromise but no more)
//
// COMMENTS: Could be an important contribution!
	nodes.erase(); 
	bool is_pruned=false;
	top_t res; 
	int nBB=0;
	unsel.init_scan(bbo::NON_DESTRUCTIVE);
	while(true){
		int v=unsel.next_bit(nBB);
		if(v==EMPTY_ELEM) break;
		
		//attemtp to cover v in full with gap: -VERY IMPROBABLE
		int wv=g->get_wv(v);
		if(wv<=dest.GAP_SIZE){
			unsel.erase_bit(v);	
			dest.GAP_SIZE-=wv;
			LOG_INFO("GAP COVERED:"<<v);
	//		cin.get();
//////////////////////
//**TODO-relaxed cover for GAP
			//LOG_INFO("GAP TO BE COVERED");
			//int MAX_BB=sel.number_of_bitblocks();
			//for(int i=nBB /*0*/; i<MAX_BB; i++){
			//	sel.get_bitboard(i)=unsel.get_bitboard(i)& ~(g->get_neighbors(v).get_bitboard(i) );
			//}
			//int w=0;
			//sel.set_bbindex(nBB);
			//sel.set_posbit(WMOD(v));	/* forced!-scan after v */
			//while(true){
			//	int u=sel.next_bit();
			//	if(u==EMPTY_ELEM) break;

			//	w+=g->get_wv(u);
			//	if(w<=wv){
			//		unsel.erase_bit(u);	
			//		LOG_INFO("GAP_COVER:"<<u);
			//		cin.get();
			//	}
			//	else w-=g->get_wv(u);
			//}//next vertex to cover
			////////////////////////////

			continue;
		}		
	
		//attempt cover v in full with TOP_2 and, in case not possible, TOP_1
	//	is_pruned=false;
		for(int c=1; c<=dest.NB_COL; c++){
			res=dest.find_top_k_CW_OPT_T2split_3S(gm.map_l2r(v),c);									/* may create new color sets */		
		
			if(res!=NONE){
				unsel.erase_bit(v);									
				is_pruned=true;
				break;
			}//end of conditional cover
		}//next color to test

		//SECOND ROUND
		if(!is_pruned)
			nodes.push(v);

	}//next vertex
	

	//SECOND ROUND-(does not compensate completely)
	for(int i=0; i<nodes.pt; i++){
		for(int c=dest.FIRST_NEW_COL; c<=dest.NB_COL; c++){
			int v=nodes.get_elem(i);
			res=dest.find_top_k_CW_OPT_T2split_3S(gm.map_l2r(v),c);									/* may create new color sets */		
		
			if(res!=NONE){
			//	LOG_INFO("VERTEX PRUNED IN SECOND ROUND");
				unsel.erase_bit(v);	
				break;
			}//end of conditional cover
			
		}//next color to test
	}

		
	return 0;
}

inline
int UBWC::cover_CW_Ord (/*int& gap,*/ UBWC& dest, const GraphMap& gm){
/////////////////////
// CW cover but using weight ordering (lowest weight first to fit better) to present nodes
//
// COMMENTS: reduces nodes, but time is slightly worse because of mappings

	top_t res; 
	int nBB=0;
	
	gm.map_l2r(unsel,dest.unsel);
	
	dest.unsel.init_scan(bbo::NON_DESTRUCTIVE);
//	dest.unsel.init_scan(bbo::NON_DESTRUCTIVE_REVERSE);
	while(true){
		int v=dest.unsel.next_bit(nBB);
	//	int v=dest.unsel.previous_bit(nBB);
		if(v==EMPTY_ELEM) break;
		int wv=dest.g->get_wv(v);

		//attemtp to cover v in full with gap: -DOES NOT OCCURR (CHECK*)
		if(wv<=dest.GAP_SIZE){
			dest.unsel.erase_bit(v);	
			LOG_INFO("GAP COVERED:"<<v<<" GSIZE:"<<dest.GAP_SIZE);
			dest.GAP_SIZE-=wv;
		//	cin.get();

//////////////////////
//**TODO-relaxed cover for GAP
			//LOG_INFO("GAP TO BE COVERED");
			//int MAX_BB=sel.number_of_bitblocks();
			//for(int i=nBB /*0*/; i<MAX_BB; i++){
			//	sel.get_bitboard(i)=unsel.get_bitboard(i)& ~(g->get_neighbors(v).get_bitboard(i) );
			//}
			//int w=0;
			//sel.set_bbindex(nBB);
			//sel.set_posbit(WMOD(v));	/* forced!-scan after v */
			//while(true){
			//	int u=sel.next_bit();
			//	if(u==EMPTY_ELEM) break;

			//	w+=g->get_wv(u);
			//	if(w<=wv){
			//		unsel.erase_bit(u);	
			//		LOG_INFO("GAP_COVER:"<<u);
			//		cin.get();
			//	}
			//	else w-=g->get_wv(u);
			//}//next vertex to cover
			////////////////////////////


			continue;
		}		
	
		//attempt cover v in full with TOP_2 and, in case not possible, TOP_1
		for(int c=1; c<=dest.NB_COL; c++){
		//	res=dest.find_top_k_CW(/*gm.map_l2r(v)*/v,c);									/* may create new color sets */		
			res=dest.find_top_k_CW_OPT_T2split(/*gm.map_l2r(v)*/v,c);		
			if(res!=NONE){
				dest.unsel.erase_bit(v);	
				
				//*** no cover of any kind of additional nodes

				break;
			}//end of conditional cover
		}//next color to test
	}//next vertex
	

	gm.map_r2l(unsel,dest.unsel);

	return 0;
}

inline
ostream& UBWC::print_db(int col_ini, int col_end, ostream& o){
//////////////
// formats bitstring color sets of db to include colors inside the range
// (both ends included)
	o<<"-----------------"<<endl;
	for(int c=col_ini; c<=col_end; c++){
		o<<c<<".";
		bitarray& bb=db.pbb[c];
		bb.init_scan(bbo::NON_DESTRUCTIVE);
		while(true){
			int v=bb.next_bit();
			if(v==EMPTY_ELEM) break;

			o<<v<<"("<<g->get_wv(v)<<")"<<" ";
		}
		o<<endl;
	}
	o<<"-----------------"<<endl;
	return o;
}

inline
int UBWC::paint_and_map	(const bitarray& bbsg, const int KMIN, const GraphMap& gm, UBWC& dest, int& nb_col){
///////////////////
// Paints subgraph bbsg and maps result to a dest.db ordered by non-decreasing weights
//
// Once a vertex is found which exceeds KMIN in col, col becomes last_col: a maximal IS 
// such that the total ub does not exceed KMIN
//
// RETURNS: gap=KMIN-ub (MUST be >0): number of colors used, UNSEL contains unfiltered nodes
//
// COMMENTS: 
// 1-dest must be correctly allocated
// 2-dest db[0] is not assigned (can have any spurious value); colors are 1-based
// 3-GraphMap gm assumes always left is caller, right is dest
// 4-computes only the weight of clauses, no top_1 or top_2 weights
	
	int nBB=EMPTY_ELEM, v=EMPTY_ELEM, ub=0;
	int pc= (unsel=bbsg).popcn64();
	nodes.erase();

	if(KMIN==1){
		LOG_INFO("UBWC::paint_and_map-KMIN 1");
	}
	nb_col=1; 
	while(true){
		sel=unsel;	
		sel.init_scan(bbo::DESTRUCTIVE);
		dest.db.erase_bit(nb_col);
		dest.tw[nb_col][0]=0;													/* initial weight of empty clause =0*/
		while(true){
			v=sel.next_bit_del(nBB,unsel);
			if(v==EMPTY_ELEM)
				break;
//////////////////////////////////
//mapping
			int wv=g->get_wv(v);
			if(wv>dest.tw[nb_col][0]){
				int inc=wv-dest.tw[nb_col][0];
				if((ub+inc)<=KMIN){
					dest.db.set_bit(nb_col, gm.map_l2r(v)/*,has_maxw /* TODO- do not need it here */);				/*TODO-check it is the first bit of nb_col*/			
					ub+=inc;
					dest.tw[nb_col][0]=wv;					
				}else{
					nodes.push(v);													/* stores rejected candidates in a stack, cannot be part of unsel */
					if((--pc)==0){
						if(dest.tw[nb_col][0]==0) nb_col--;							/* last color nb_col might be empty- TODO: might be unnecessary since ub<=KMIN */
						for(int i=0; i<nodes.pt; i++)
							unsel.set_bit(nodes.get_elem(i));
						/*if(ub>KMIN){
							LOG_ERROR("UBWC::paint_and_map()-bizarre ub > KMIN");
						}*/
						return KMIN-ub;	/*should be >=ub*/	
					}
					continue;
				}
			}else{
				dest.db.set_bit(nb_col, gm.map_l2r(v)/*,has_maxw*/);					/* current(l) 2 dest(r) */
			}
					
			if((--pc)==0){
				if(dest.tw[nb_col][0]==0) nb_col--;									/* last color nb_col might be empty- TODO: might be unnecessary since ub<=KMIN */
				for(int i=0; i<nodes.pt; i++)
					unsel.set_bit(nodes.get_elem(i));
				
				/*if(ub>KMIN){
					LOG_ERROR("UBWC::paint_and_map()-bizarre ub > KMIN");
				}*/
				return KMIN-ub;	/*should be >=ub*/	
			}
			sel.erase_block(nBB,g->get_neighbors(v));								/* coloring !*/		
		}//next vertex to enlarge current color set
		
	
		if(ub==KMIN){																/* normal-exit no more gap for new colors, with remaining nodes in unsel */
			for(int i=0; i<nodes.pt; i++)
				unsel.set_bit(nodes.get_elem(i));	
			return KMIN-ub;
		}
		nb_col++;
	}//next color set to build
}

inline
int UBWC::paint_and_map_OPT	(const bitarray& bbsg, const int KMIN, const GraphMap& gm, UBWC& dest, int& nb_col){
///////////////////
// paint and map optimized: updates tk on the fly. Does not store color sets

	int nBB=EMPTY_ELEM, v=EMPTY_ELEM, ub=0;
	int pc= (unsel=bbsg).popcn64();
	nodes.erase();

	//if(KMIN==1){
	//	LOG_INFO("UBWC::paint_and_map-KMIN 1");
	//}
	nb_col=1; 
	while(true){
		sel=unsel;	
		sel.init_scan(bbo::DESTRUCTIVE);
		dest.tw[nb_col][0]=0;													/* initial weight of empty clause =0*/
		dest.fn[nb_col][0]=CLQ_MAXINT;
		dest.fn[nb_col][1]=CLQ_MAXINT;
		dest.fn[nb_col][2]=CLQ_MAXINT;
		while(true){
			v=sel.next_bit_del(nBB,unsel);
			if(v==EMPTY_ELEM)
				break;
//////////////////////////////////
//mapping
			int vw=gm.map_l2r(v);
			if(vw<dest.fn[nb_col][2]){
				if(vw<dest.fn[nb_col][0]){
					int wv=g->get_wv(v);
					int inc=wv-dest.tw[nb_col][0];
					if((ub+inc)<=KMIN){
						dest.fn[nb_col][2]=dest.fn[nb_col][1];
						dest.fn[nb_col][1]=dest.fn[nb_col][0];
						dest.fn[nb_col][0]=vw;
						ub+=inc;
						dest.tw[nb_col][0]=wv;
					}else{
						nodes.push(v);	
						if((--pc)==0){
							if(dest.tw[nb_col][0]==0) nb_col--;							/* last color nb_col might be empty- TODO: might be unnecessary since ub<=KMIN */
							else dest.update_tk(nb_col);
							for(int i=0; i<nodes.pt; i++)
								unsel.set_bit(nodes.get_elem(i));
							return KMIN-ub;	
						}
						continue;
					}
				}else if(vw<dest.fn[nb_col][1]){
					dest.fn[nb_col][2]=dest.fn[nb_col][1];
					dest.fn[nb_col][1]=vw;
				}else{
					dest.fn[nb_col][2]=vw;
				}
				
				/*else if(vw<dest.fn[nb_col][2]){
					dest.fn[nb_col][2]=vw;
				}*/
			}
			 
			/* else the vertex is colored but not stored */
			if((--pc)==0){
				if(dest.tw[nb_col][0]==0) nb_col--;									/* last color nb_col might be empty- TODO: might be unnecessary since ub<=KMIN */
				else dest.update_tk(nb_col);
				for(int i=0; i<nodes.pt; i++)
					unsel.set_bit(nodes.get_elem(i));
				return KMIN-ub;	/*should be >=ub*/	
			}
			sel.erase_block(nBB,g->get_neighbors(v));								/* coloring !*/		
		}//next vertex to enlarge current color set
		
		dest.update_tk(nb_col);	
		if(ub==KMIN){																/* normal-exit no more gap for new colors, with remaining nodes in unsel */
			for(int i=0; i<nodes.pt; i++)
				unsel.set_bit(nodes.get_elem(i));	
			return KMIN-ub;
		}
		nb_col++;
	}//next color set to build
}

inline
int UBWC::paint_and_map_CW (const bitarray& bbsg, const int KMIN, const GraphMap& gm, UBWC& dest){
///////////////////
// paint and map optimized, but computes (node, weight)-not top-k values- for the first 3 nodes of each color set 
// in dest

	int nBB=EMPTY_ELEM, v=EMPTY_ELEM, ub=0;
	int pc= (unsel=bbsg).popcn64();
	nodes.erase();
		
	dest.NB_COL=1; 
	dest.FIRST_NEW_COL=CLQ_MAXINT;
	while(true){
		sel=unsel;	
		sel.init_scan(bbo::DESTRUCTIVE);
		dest.cw[dest.NB_COL][0]=0;													/* initial weight of empty clause =0*/
		dest.fn[dest.NB_COL][0]=CLQ_MAXINT;
		dest.fn[dest.NB_COL][1]=CLQ_MAXINT;
		dest.fn[dest.NB_COL][2]=CLQ_MAXINT;
		while(true){
			v=sel.next_bit_del(nBB,unsel);
			if(v==EMPTY_ELEM)
				break;
//////////////////////////////////
//mapping
			int vw=gm.map_l2r(v);
			if(vw<dest.fn[dest.NB_COL][2]){
				if(vw<dest.fn[dest.NB_COL][0]){
					int wv=g->get_wv(v);
					int inc=wv-dest.cw[dest.NB_COL][0];
					if((ub+inc)<=KMIN){
						dest.fn[dest.NB_COL][2]=dest.fn[dest.NB_COL][1];
						dest.fn[dest.NB_COL][1]=dest.fn[dest.NB_COL][0];
						dest.fn[dest.NB_COL][0]=vw;
						ub+=inc;
						dest.cw[dest.NB_COL][0]=wv;
					}else{
						nodes.push(v);	
						if((--pc)==0){
							if(dest.cw[dest.NB_COL][0]==0) dest.NB_COL--;							/* last color nb_col might be empty- TODO: might be unnecessary since ub<=KMIN */
							else dest.update_CW(dest.NB_COL);
							for(int i=0; i<nodes.pt; i++)
								unsel.set_bit(nodes.get_elem(i));
							dest.FIRST_NEW_COL=dest.NB_COL+1;
							return (dest.GAP_SIZE=KMIN-ub);	
						}
						continue;
					}
				}else if(vw<dest.fn[dest.NB_COL][1]){
					dest.fn[dest.NB_COL][2]=dest.fn[dest.NB_COL][1];
					dest.fn[dest.NB_COL][1]=vw;
				}else{
					dest.fn[dest.NB_COL][2]=vw;
				}				
			}
			 
			/* else the vertex is colored but not stored */
			if((--pc)==0){
				if(dest.cw[dest.NB_COL][0]==0) dest.NB_COL--;									/* last color nb_col might be empty- TODO: might be unnecessary since ub<=KMIN */
				else dest.update_CW(dest.NB_COL);
				for(int i=0; i<nodes.pt; i++)
					unsel.set_bit(nodes.get_elem(i));
				dest.FIRST_NEW_COL=dest.NB_COL+1;
				return (dest.GAP_SIZE=KMIN-ub);	
			}
			sel.erase_block(nBB,g->get_neighbors(v));								/* coloring !*/		
		}//next vertex to enlarge current color set
		
		dest.update_CW(dest.NB_COL);	
		if(ub==KMIN){																/* normal-exit no more gap for new colors, with remaining nodes in unsel */
			for(int i=0; i<nodes.pt; i++)
				unsel.set_bit(nodes.get_elem(i));	
			dest.FIRST_NEW_COL=dest.NB_COL+1;
			return (dest.GAP_SIZE=KMIN-ub);	
		}
		dest.NB_COL++;
	}//next color set to build
}


inline
int UBWC::paint_and_map_CW_dyn (const bitarray& bbsg, const int KMIN, const GraphMap& gm, UBWC& dest){
///////////////////
// paint and map CW inclyding dynamic reshuffling of nodes before assigned to an ISET
// 
// COMMENTS: Just before a node is selected, filter is attempted with previous ISETS and any move
// (including a new color) that allows to insert the node in a previous ISET WITHOUT INCREMENTING
// THE TOTAL BOUND is made
//
// TODO-BUGGY WHEN FILTERING!

	int nBB=EMPTY_ELEM, v=EMPTY_ELEM, ub=0;
	int pc= (unsel=bbsg).popcn64();
	nodes.erase();
		
	dest.NB_COL=1; 
	dest.FIRST_NEW_COL=CLQ_MAXINT;
	while(true){
		sel=unsel;	
		sel.init_scan(bbo::DESTRUCTIVE);
		dest.cw[dest.NB_COL][0]=0;													/* initial weight of empty clause =0*/
		dest.fn[dest.NB_COL][0]=CLQ_MAXINT;
		dest.fn[dest.NB_COL][1]=CLQ_MAXINT;
		dest.fn[dest.NB_COL][2]=CLQ_MAXINT;
		bool first_vertex=true;
		while(true){	
/*next_v:	*/	v=sel.next_bit_del(nBB,unsel);
			if(v==EMPTY_ELEM){
				break;
			}
//////////////////////////////////
//mapping
			int vw=gm.map_l2r(v);
				
//////////////////////////
// first vertex in the color set- attempt RECOLOR
// ***** TODO-BUGGY!

			//if(first_vertex){
			//	//attempt to filter in previous colors
			//	top_t res=NONE;
			//	for(int c=1; c<dest.NB_COL /* exclude this color*/; c++){
			//		res=dest.find_top_k_CW_OPT_T2split(vw,c, dest.NB_COL);
			//		if(res!=NONE){																	/* split succesful */
			//		//	LOG_INFO("PAINT CUT");
			//		//	cin.get();
			//			if((--pc)==0){
			//				if(dest.cw[dest.NB_COL][0]==0){
			//				//	LOG_INFO("KKKKKKKKKKKKKKKKKKK");
			//					dest.NB_COL--;														/* last color nb_col might be empty- TODO: might be unnecessary since ub<=KMIN */
			//				}
			//				else dest.update_CW(dest.NB_COL);
			//				for(int i=0; i<nodes.pt; i++)
			//					unsel.set_bit(nodes.get_elem(i));
			//				dest.FIRST_NEW_COL=dest.NB_COL+1;
			//				return (dest.GAP_SIZE=KMIN-ub);	
			//			}
			//			//new open color after split
			//			if(res!=ISET_FREE){
			//				dest.NB_COL++;
			//				dest.cw[dest.NB_COL][0]=0;												/* initial weight of empty clause =0*/
			//				dest.fn[dest.NB_COL][0]=CLQ_MAXINT;
			//				dest.fn[dest.NB_COL][1]=CLQ_MAXINT;
			//				dest.fn[dest.NB_COL][2]=CLQ_MAXINT;
			//			}
			//			break;
			//		//	goto next_v;  /* MUST keep  first_vertex flag ON */
			//		}
			//	}//next color attemtp to filter first vertex
			//	if(res!=NONE) continue;																/*SUCCESS, go to next filtered vertex */
			//}

//////////////////////////////////////////

			if(vw<dest.fn[dest.NB_COL][2]){
				if(vw<dest.fn[dest.NB_COL][0]){
					int wv=g->get_wv(v);
					int inc=wv-dest.cw[dest.NB_COL][0];
					if((ub+inc)<=KMIN){


						//*** TODO- filter (more difficult since  dest.NB_COL is already occupied)
						//					possibly design a filter which substitutes ISETS, and does not split

						dest.fn[dest.NB_COL][2]=dest.fn[dest.NB_COL][1];
						dest.fn[dest.NB_COL][1]=dest.fn[dest.NB_COL][0];
						dest.fn[dest.NB_COL][0]=vw;
						ub+=inc;
						dest.cw[dest.NB_COL][0]=wv;
					}else{
						nodes.push(v);	/* could not insert: GAP CONSTRAINT*/
						if((--pc)==0){
							if(dest.cw[dest.NB_COL][0]==0) dest.NB_COL--;							/* last color nb_col might be empty- TODO: might be unnecessary since ub<=KMIN */
							else dest.update_CW(dest.NB_COL);
							for(int i=0; i<nodes.pt; i++)
								unsel.set_bit(nodes.get_elem(i));
							dest.FIRST_NEW_COL=dest.NB_COL+1;
							return (dest.GAP_SIZE=KMIN-ub);	
						}
						continue;
					}
				}else if(vw<dest.fn[dest.NB_COL][1]){
					dest.fn[dest.NB_COL][2]=dest.fn[dest.NB_COL][1];
					dest.fn[dest.NB_COL][1]=vw;
				}else{
					dest.fn[dest.NB_COL][2]=vw;
				}				
			}
			 
			/* else the vertex is colored but not stored */
			if((--pc)==0){
				if(dest.cw[dest.NB_COL][0]==0) dest.NB_COL--;									/* last color nb_col might be empty- TODO: might be unnecessary since ub<=KMIN */
				else dest.update_CW(dest.NB_COL);
				for(int i=0; i<nodes.pt; i++)
					unsel.set_bit(nodes.get_elem(i));
				dest.FIRST_NEW_COL=dest.NB_COL+1;
				return (dest.GAP_SIZE=KMIN-ub);	
			}
//////////////////////////
// coloring
			//updates flag
			if(first_vertex==true)
					first_vertex=false;
			sel.erase_block(nBB,g->get_neighbors(v));								/* coloring !*/		
		}//next vertex to enlarge current color set
		
		dest.update_CW(dest.NB_COL);	
		if(ub==KMIN){																/* normal-exit no more gap for new colors, with remaining nodes in unsel */
			for(int i=0; i<nodes.pt; i++)
				unsel.set_bit(nodes.get_elem(i));	
			dest.FIRST_NEW_COL=dest.NB_COL+1;
			return (dest.GAP_SIZE=KMIN-ub);	
		}
		dest.NB_COL++;
	}//next color set to build
}

inline
int UBWC::paint_and_map_CW_3S (const bitarray& bbsg, const int KMIN, const GraphMap& gm, UBWC& dest){
///////////////////
// paint and map optimized but also informs for all ISETS if theyhave SIZE 3 or more

	int nBB=EMPTY_ELEM, v=EMPTY_ELEM, ub=0;
	int pc= (unsel=bbsg).popcn64();
	nodes.erase();
		
	dest.NB_COL=1; 
	dest.FIRST_NEW_COL=CLQ_MAXINT;
	while(true){
		sel=unsel;	
		sel.init_scan(bbo::DESTRUCTIVE);
		dest.cw[dest.NB_COL][0]=0;													/* initial weight of empty clause =0*/
		dest.fn[dest.NB_COL][0]=CLQ_MAXINT;
		dest.fn[dest.NB_COL][1]=CLQ_MAXINT;
		dest.fn[dest.NB_COL][2]=CLQ_MAXINT;
		dest.fn[dest.NB_COL][3]=SIZE_3_ISET;										/* default value of ISET */
		while(true){
			v=sel.next_bit_del(nBB,unsel);
			if(v==EMPTY_ELEM)
				break;
//////////////////////////////////
//mapping
			int vw=gm.map_l2r(v);
			if(vw<dest.fn[dest.NB_COL][2]){
				if(vw<dest.fn[dest.NB_COL][0]){
					int wv=g->get_wv(v);
					int inc=wv-dest.cw[dest.NB_COL][0];

					if((ub+inc)<=KMIN){
						//update SIZE_3															/* MUST BE HERE */
						if(dest.fn[dest.NB_COL][2]!=CLQ_MAXINT)
								dest.fn[dest.NB_COL][3]=CLQ_MAXINT;


						dest.fn[dest.NB_COL][2]=dest.fn[dest.NB_COL][1];
						dest.fn[dest.NB_COL][1]=dest.fn[dest.NB_COL][0];
						dest.fn[dest.NB_COL][0]=vw;
						ub+=inc;
						dest.cw[dest.NB_COL][0]=wv;

						

					}else{
						nodes.push(v);	
						if((--pc)==0){
							if(dest.cw[dest.NB_COL][0]==0) dest.NB_COL--;							/* last color nb_col might be empty- TODO: might be unnecessary since ub<=KMIN */
							else dest.update_CW(dest.NB_COL);
							for(int i=0; i<nodes.pt; i++)
								unsel.set_bit(nodes.get_elem(i));
							dest.FIRST_NEW_COL=dest.NB_COL+1;
							return (dest.GAP_SIZE=KMIN-ub);	
						}
						continue;
					}

					


				}else if(vw<dest.fn[dest.NB_COL][1]){

					//update SIZE_3
					if(dest.fn[dest.NB_COL][2]!=CLQ_MAXINT)
								dest.fn[dest.NB_COL][3]=CLQ_MAXINT;

					dest.fn[dest.NB_COL][2]=dest.fn[dest.NB_COL][1];
					dest.fn[dest.NB_COL][1]=vw;

					

				}else{
					//update SIZE_3
					if(dest.fn[dest.NB_COL][2]!=CLQ_MAXINT)
								dest.fn[dest.NB_COL][3]=CLQ_MAXINT;


					dest.fn[dest.NB_COL][2]=vw;

					//update SIZE_3
				}				
			}else{	/* SIZE_3 off for the rest of nodes */
				dest.fn[dest.NB_COL][3]=CLQ_MAXINT;
			}
			 
			/* else the vertex is colored but not stored */
			if((--pc)==0){
				if(dest.cw[dest.NB_COL][0]==0) dest.NB_COL--;									/* last color nb_col might be empty- TODO: might be unnecessary since ub<=KMIN */
				else dest.update_CW(dest.NB_COL);
				for(int i=0; i<nodes.pt; i++)
					unsel.set_bit(nodes.get_elem(i));
				dest.FIRST_NEW_COL=dest.NB_COL+1;
				return (dest.GAP_SIZE=KMIN-ub);	
			}
			sel.erase_block(nBB,g->get_neighbors(v));								/* coloring !*/		
		}//next vertex to enlarge current color set
		
		dest.update_CW(dest.NB_COL);	
		if(ub==KMIN){																/* normal-exit no more gap for new colors, with remaining nodes in unsel */
			for(int i=0; i<nodes.pt; i++)
				unsel.set_bit(nodes.get_elem(i));	
			dest.FIRST_NEW_COL=dest.NB_COL+1;
			return (dest.GAP_SIZE=KMIN-ub);	
		}
		dest.NB_COL++;
	}//next color set to build
}

inline
int UBWC::increase_gap(){
///////////////////
// for a given color_db, attempts to reduce GAP_SIZE
// of the ISETs derived by painting functions by finding and ISET 
// of the representative nodes
//
// RETURNS-increment in GAP_SIZE
//
// COMMENTS
// 1.Apply after painting and before covering
//
// **TODO-DEVELOPING!!!! AND BUGGY-CHECK

	//int col=1;
	//nodes.erase();
	//colors.erase();
	//int GAP_SIZE_NEW=GAP_SIZE;

	////determine first ISET which is not a singleton
	//for(col=1; col<=NB_COL; col++){
	//	if(fn[col][1]==CLQ_MAXINT) continue;
	//	nodes.push(fn[col][0]);
	//	colors.push(col);
	//}

	////determine ISET
	//if(col<NB_COL){
	//	for(int c=col+1; c<=NB_COL; c++){
	//		if(fn[c][1]==CLQ_MAXINT) continue;

	//		//is indep with nodes
	//		bool is_indep=true;
	//		for(int i=0; i<nodes.pt; i++){
	//			int w= nodes.get_elem(i);
	//			if(g->is_edge(w,fn[c][0])){
	//				is_indep=false;
	//				break;
	//			}
	//		}

	//		if(is_indep){
	//			nodes.push(fn[c][0]);
	//			colors.push(c);
	//		}
	//	
	//	}
	//}


	////Evaluate ISET: min( SUM diff weight first two nodes of combinations (nodes.size/nodes.size()-1))
	////vector<int> gap_inc;

	////APPROXIMATION A**** TODO-DO IT PROPERLY
	////int* p=std::max_element(nodes.stack,nodes.stack+nodes.pt); 
	////for(int i=0; i<colors.pt-2; i++){
	////	//if(nodes.get_elem(i)==*p) continue;
	////	GAP_SIZE_NEW+=(cw[fn[colors.get_elem(i)][0]]-cw[fn[colors.get_elem(i)][1]]);
	////}
	////	

	////APPROXIMATION B**** SHOULD WORK!
	//if(nodes.pt>=2)
	//	GAP_SIZE_NEW+=std::min<int>(cw[fn[colors.get_elem(0)][0]]-cw[fn[colors.get_elem(0)][1]],cw[fn[colors.get_elem(1)][0]]-cw[fn[colors.get_elem(1)][1]]);

	//int res=GAP_SIZE_NEW-GAP_SIZE;
	//GAP_SIZE=GAP_SIZE_NEW;
	//return res;

	return 0;
}


inline
void UBWC::update_tk(int nb_col){
	if(fn[nb_col][1]!=CLQ_MAXINT){
		tw[nb_col][1]=tw[nb_col][0]-g->get_wv(fn[nb_col][1]);
		if(fn[nb_col][2]!=CLQ_MAXINT){
			tw[nb_col][2]=tw[nb_col][0]-g->get_wv(fn[nb_col][2]);
		}else tw[nb_col][2]=0;
	}else{
		tw[nb_col][1]=0;
		tw[nb_col][2]=0;
	}	
}

inline
void UBWC::update_CW(int nb_col){
/////////////
// invalid cw slots are filled with CLQ_MAX_INT

	if(fn[nb_col][1]!=CLQ_MAXINT){
		cw[nb_col][1]=g->get_wv(fn[nb_col][1]);
		if(fn[nb_col][2]!=CLQ_MAXINT)
			cw[nb_col][2]=g->get_wv(fn[nb_col][2]);
		else cw[nb_col][2]=CLQ_MAXINT;
	}else{
		cw[nb_col][1]=CLQ_MAXINT;
		cw[nb_col][2]=CLQ_MAXINT;
	}
}

inline
int UBWC::set_tk (int col_ini, int col_end){
/////////////////////////
// determines top_k from db within range  (both ends included)
// also caches first three nodes of color sets
//
// RETURNS 0-ok, -1 Error
//
// COMMENTS:
// 1. Only for graphs ordered according to non-increasing weights!
// 2. Assumes colors are NEVER empty (ASSERT ACTIVE)

	for(int c=col_ini; c<=col_end; c++){
		int nb_read=first_k_bits(3, db.pbb[c], fn[c]);  
		tw[c][0]=g->get_wv(fn[c][0]);								/* should exist:col should not be empty */
		if(nb_read==3){
			tw[c][1]=tw[c][0]-g->get_wv(fn[c][1]);
			tw[c][2]=tw[c][0]-g->get_wv(fn[c][2]);
			continue;
		}else {
			tw[c][2]=0; 
			(nb_read==2)? tw[c][1]=tw[c][0]-g->get_wv(fn[c][1]) : tw[c][1]=0;			
		}

		//assert-remove in RELEASE! 
		if(fn[c][0]==EMPTY_ELEM){
			LOG_ERROR(" UBWC::set_tk ()-bizarre empty col");
			return -1;
		}
	}
	return 0;
}

inline
int UBWC::paint_and_tk(const bitarray& bbsg, const int KMIN, int& nb_col, com::stack_t<int>& cand){
///////////////////
// SEQ partial coloring of subgraph bbsg up to and including THRESHOLD KMIN. 
// Also collects top_k information, and caches the first 3 nodes of each color set
//
// RETURNS UB of the partial coloring (UB<=KMIN!)
//
// COMMENTS: Only for graphs ordered according to non-increasing weights!

	unsel=bbsg;
	int nBB=0, count_3=1, ub=0, v=EMPTY_ELEM, pc=unsel.popcn64();
	cand.erase();

	//assert not_empty
	if(pc==0){
		LOG_INFO("UBWC::paint_and_tk()-empty subgraph");
		nb_col=0;
		return 0;
	}
		
	nb_col=1;
	while(true){ 
		db.pbb[nb_col]=unsel;
		count_3=1;
		db.pbb[nb_col].init_scan(bbo::NON_DESTRUCTIVE);
		tw[nb_col][0]=0; tw[nb_col][1]=0; tw[nb_col][2]=0;
		while(true){
			v=db.pbb[nb_col].next_bit(nBB,unsel);
			if(v==EMPTY_ELEM)
				break;
			//updates top_weights and weight of partial coloring (w_col)
			if(count_3<=3 && count_3>=1){
				int wv=g->get_wv(v);
				if(count_3==1){
					ub+=wv;
					if(ub>KMIN){													/*checks if node is valid*/
						//restores data		
						db.pbb[nb_col].erase_bit(v);								/* removes from color set */
						cand.push(v);
						ub-=wv;
						if((--pc)==0){
							if(count_3==1) nb_col--;								/* restores data when last color is empty (perhaps unnecessary) */
							//if(ub>KMIN){											/* ASSERT */
							//	LOG_ERROR("UBWC::paint()-bizarre ub < kmin");
							//}
							return ub;
						}
						count_3=1;
						continue;												
					}//end of check- valid node
					fn[nb_col][0]=v;
					tw[nb_col][0]=wv;	
					count_3++;
				}else if(count_3==2){
					fn[nb_col][1]=v;
					tw[nb_col][1]=tw[nb_col][0]-wv;
					count_3++;
				}else if(count_3==3){
					fn[nb_col][2]=v;
					tw[nb_col][2]=tw[nb_col][0]-wv;
					count_3++;
				}
			}
			//upper bound is not greater than KMIN
			if((--pc)==0){
				//if(count_3==1) nb_col--;
				if(ub>KMIN || count_3==1){
					LOG_ERROR("UBWC::paint_and_tk()-bizarre ub < kmin");
				}
				return ub;	/*should be >=ub*/												
			}			
			db.pbb[nb_col].erase_block(nBB,g->get_neighbors(v));		/* paints!*/
		}//next node of current color
		if(ub==KMIN){ 
			//OPTIONAL-copies cand to unsel 
			/*for(int i=0; i<cand.pt; i++)
				unsel.set_bit(cand.get_elem(i));*/
			return ub;
		}
		nb_col++;
	}//next color
	
	LOG_ERROR("UBWC::paint_and_tk()-bizarre line of code reached");
	return ub;	/* should not reach here*/
}

inline
int UBWC::paint(const bitarray& bbsg, const int KMIN, int& nb_col, com::stack_t<int>& cand){
///////////////////
// SEQ partial coloring of subgraph bbsg up to
// and including THRESHOLD KMIN. 
//
// RETURNS UB of the partial coloring (UB<=KMIN!)
//
// COMMENTS: Only for graphs ordered according to non-increasing weights!
	
	unsel=bbsg;
	int nBB=0, ub=0, v=EMPTY_ELEM, pc=unsel.popcn64();
	cand.erase();

	//assert not_empty
	if(pc==0){
		LOG_INFO("UBWC::paint()-empty subgraph");
		nb_col=0;
		return 0;
	}
	
	nb_col=1;
	bool first_vertex;
	while(true){ 
		db.pbb[nb_col]=unsel;
		db.pbb[nb_col].init_scan(bbo::NON_DESTRUCTIVE);
		first_vertex=true;
		while(true){
			v=db.pbb[nb_col].next_bit(nBB,unsel);
			if(v==EMPTY_ELEM)
				break;
			if(first_vertex){											/* first vertex analysis */	
				ub+=g->get_wv(v);
				if(ub>KMIN){
					//restore
					db.pbb[nb_col].erase_bit(v);								/* removes from color set */
					cand.push(v);												/* stores rejected candidates in a stack, cannot be part of unsel */
					ub-=g->get_wv(v);
					if((--pc)==0){
						if(first_vertex) nb_col--;								/* restores data when last color is empty (perhaps unnecessary) */
						//if(ub>KMIN){											/* ASSERT */
						//	LOG_ERROR("UBWC::paint()-bizarre ub < kmin");
						//}
						return ub;
					}
					continue;											/*next first vertex candidate */	
				}else first_vertex=false;		
			}
			//upper bound is not greater than KMIN
			if((--pc)==0){
				//if(first_vertex) nb_col--;							/* restores data when last color is empty (perhaps unnecessary) */
				if(ub>KMIN || first_vertex){											/* ASSERT */
					LOG_ERROR("UBWC::paint()-bizarre ub");
				}
				return ub;												/* KMIN should be >=ub*/												
			}			
			db.pbb[nb_col].erase_block(nBB,g->get_neighbors(v));			/* paints!*/
		}//next node of current color

		if(ub==KMIN){													/* last color equals gap, cannot produce new color sets */
			//OPTIONAL-copies cand to unsel 
			/*for(int i=0; i<cand.pt; i++)
				unsel.set_bit(cand.get_elem(i));*/
			return ub;
		}
		nb_col++;
	}//next color
	
	LOG_ERROR("UBWC::paint()-bizarre line of code reached");
	return ub;	/* should not reach here*/
}

inline
int UBWC::greedy_lb (const ugraph& g, const bitarray& bbsg){
	int lb=0;
	bitarray bb(bbsg);
	bb.init_scan(bbo::DESTRUCTIVE);
	while(true){
		int v=bb.next_bit_del();
		if (v==EMPTY_ELEM) break;
		lb+=g.get_wv(v);
		bb&=g.get_neighbors(v);		
	}

	return lb;
}

inline
int UBWC::UB_sum  (const ugraph& g, bitarray& bbsg, int KMIN){
////////////////////////////
// trivial greedy bound of subgraph bbsg- sum of weights of nodes
// with early exit
//
// RETURNS UB (DEFAULT value for KMIN) or a close value to KMIN when ub>=KMIN

	int ub=0;
	bbsg.init_scan(bbo::NON_DESTRUCTIVE);
	while(ub<KMIN){
		int v=bbsg.next_bit();
		if (v==EMPTY_ELEM) break;
		ub+=g.get_wv(v);
	}

	return ub;
}

inline
int UBWC::UB_col  (const ugraph& g, const bitarray& bbsg, int KMIN){
////////////////////////////
// greedy bound as sum of maximum weight of ISETS with early EXIT if
// bound is greater than KMIN
//
// RETURNS UB or a close value to KMIN when ub>=KMIN

	int ub=0, wvISET=0, nBB=EMPTY_ELEM, v=EMPTY_ELEM;
	bitarray unsel(bbsg);
	bitarray sel(g.number_of_vertices());
	int pc=unsel.popcn64();

	if(pc==0) return 0;						/* MUST BE */
	
	while(true){ 
		sel=unsel;
		sel.init_scan(bbo::DESTRUCTIVE);
		wvISET=0;
		while(true){
			v=sel.next_bit_del(nBB,unsel);
			if(v==EMPTY_ELEM)
				break;
			int wv=g.get_wv(v);
			if(wvISET<wv) wvISET=wv;
			if((--pc)==0){
				ub+=wvISET;
				if(ub>=KMIN) return ub;
			}
			sel.erase_block(nBB,g.get_neighbors(v));
		}//next node
		ub+=wvISET;
		if(ub>=KMIN) return ub;
	}//next ISET

	LOG_ERROR("UBWC::UB_col()-bizarre exit");
	return ub;								/* should not reach here*/
}


inline
void UBWC::clear(){
	//const int NV=g->number_of_vertices();
	if(tw){
		for(int i=0; i<m_NV; i++){
			delete [] tw[i];
		}
		delete [] tw;
	}

	if(fn){
		for(int i=0; i<m_NV; i++){
			delete [] fn[i];
		}
		delete [] fn;
	}
	tw=NULL;				/* MUST BE */
	fn=NULL;				/* MUST BE */
	nodes.erase();
	node_cover.erase();
	colors.erase();

	//**TODO-clear info of unsel, sel?
}

inline
void UBWC::init(){
	clear();
	//const int NV=g->number_of_vertices();
	
	try{
		unsel.init(m_NV);
		sel.init(m_NV);
		
		db.init(m_NV+1,m_NV);					/* max allocation! */
		tw= new int*[m_NV];
		for(int i=0; i<m_NV; i++){
			tw[i]=new int[3];
		}

		fn= new int*[m_NV];
		for(int i=0; i<m_NV; i++){
			fn[i]=new int[4];					/* SLOT [3] constains SIZE_3 or CLQ_MAXINT */
		}

		cw= new int*[m_NV];
		for(int i=0; i<m_NV; i++){
			cw[i]=new int[3];
		}

		nodes.init(m_NV);
		node_cover.init(m_NV);
		colors.init(m_NV);			/* TODO-optimize color space */
		
	}catch(exception &e){
		e.what();
	}
}

inline
UBWC::top_t UBWC::find_gap(int v, int& gap){
///////////////////////////
// Full cover by gap (does not depend on db_color)
// Also updates gap
//
// RETURNS GAP or NONE
//
// COMMENTS: Only for graphs ordered by non-increasing weights!
	
	//**POSSIBLY-ASSERT gap is not 0

	int wv=g->get_wv(v);
	if(wv<=gap){								/* v fully covered by gap */

		//relaxed cover-***TODO OPTIMIZE
		sel=unsel;
		sel.erase_bit(g->get_neighbors(v));
		int w=0;
		//sel.init_scan(bbo::DESTRUCTIVE_REVERSE);       /** in reverse */
		sel.init_scan(bbo::DESTRUCTIVE);       /**  */
		while(true){
		//	int u=sel.previous_bit_del();
			int u=sel.next_bit_del();
			if(u==EMPTY_ELEM) break;
			
			w+=g->get_wv(u);
			if(w<=wv){
				//LOG_INFO("GAP_COVER:"<<u<<":"<<wv);
				unsel.erase_bit(u);	
			}
			else w-=g->get_wv(u);
		}

		//updates gap and returns
		gap-=wv;
		return GAP;
	}

	return NONE;
}

inline
UBWC::top_t UBWC::find_top_k(int v, int col) {
/////////////////
// Determines top_1 or top_2 for the given vertex, col and gap
// Updates gap, and top_k
//
// Returns NONE, TOP1 or TOP2
//
// COMMENTS: col MUST NOT BE EMPTY

	int wv=g->get_wv(v);

	//attempt top_2 and exit
	if(fn[col][1]!=EMPTY_ELEM && wv<=tw[col][2]){			
		if(!g->get_neighbors(v).is_bit(fn[col][1]) && !g->get_neighbors(v).is_bit(fn[col][0])){			/* test for top_2*/
			tw[col][2]-=wv;
			tw[col][1]= min<int>(tw[col][1],tw[col][0]-wv);								/** TODO- check non negative values */
			
			/**cover? */

			LOG_INFO("TOP2:"<<v<<":"<<col);
			return TOP2;
		}
	}

	//attempt top_1 and exit
	if(wv<=tw[col][1] && !g->get_neighbors(v).is_bit(fn[col][0])){	
		tw[col][1]-=wv;	
		tw[col][2]=max<int>(tw[col][2]-wv, 0);

		//relaxed cover
		sel=unsel;
		sel.erase_bit_joint(g->get_neighbors(fn[col][0]),g->get_neighbors(v));
		int w=0;
		//sel.init_scan(bbo::DESTRUCTIVE_REVERSE);  /** in reverse */
		sel.init_scan(bbo::DESTRUCTIVE); 
		while(true){
		//	int u=sel.previous_bit_del();
			int u=sel.next_bit_del();
			if(u==EMPTY_ELEM) break;

			w+=g->get_wv(u);
			if(w<=wv){
				unsel.erase_bit(u);	
				LOG_INFO("TOP-1_COVER:"<<u<<":"<<col);
			}
			else w-=g->get_wv(u);
		}
		
		LOG_INFO("TOP1:"<<v<<":"<<col);
		return TOP1;
	}
	return NONE;
}


inline
UBWC::top_t UBWC::find_top_k_shared(int v, int col) {
/////////////////
// Determines top_1 or top_2 for the given pair (v,col) 
// Also updates top_k accordingly
//
// Returns NONE, TOP1 or TOP2
//
// COMMENTS: 
// 1.col MUST NOT BE EMPTY 
// 2.there may be a big number of calls where the weight of v is greater than
//   the weight of the color set col (i.e. wv>tw[col][0]). In this case NO covering is possible

	int wv=g->get_wv(v);
			
	//attempt TOP-K-2 and exit
	if( wv<=tw[col][2] && !g->get_neighbors(v).is_bit(fn[col][1]) &&   /* fn[col][1] must exist to meet the first cond*/
						  !g->get_neighbors(v).is_bit(fn[col][0])		){				
			tw[col][2]-= wv;			
			if( (g->get_wv(fn[col][1])-wv) < g->get_wv(fn[col][2]))					/* top-k-1 will not change in some cases*/
						tw[col][1]=max<int>(tw[col][1]-wv, 0);

			//attempt 2		
			//tw[col][1]= min<int>(tw[col][1],tw[col][0]-wv);	

			//attempt 3 (apparently working)
			//tw[col][1]=max<int>(tw[col][1]-wv, 0);	
			
			//LOG_INFO("TOP2:"<<v<<":"<<col);
			return TOP2;
	}

	//attempt TOP-K-1 and exit
	if( wv<=tw[col][1] && !g->get_neighbors(v).is_bit(fn[col][0]) ){	
		tw[col][1]-=wv;	
		tw[col][2]=max<int>(tw[col][2]-wv, 0);
									
		//LOG_INFO("TOP1:"<<v<<":"<<col);
		return TOP1;
	}
	
	return NONE;
}

inline
UBWC::top_t UBWC::find_top_k_shared_with_swap(int v, int col) {
/////////////////
// find_top_k_shared but swapping nodes to maximize tw 
//
// TODO-devekoping (28/8/17)

	int wv=g->get_wv(v);
			
	//attempt TOP-K-2 and exit
	if( wv<=tw[col][2] && !g->get_neighbors(v).is_bit(fn[col][1]) &&   /* fn[col][1] must exist to meet the first cond*/
						  !g->get_neighbors(v).is_bit(fn[col][0])		){				
			

			//if( (g->get_wv(fn[col][1])-wv) < g->get_wv(fn[col][2])){					/* top-k-1 will not change in some cases*/
				//swap	
				int wv1=g->get_wv(fn[col][0])-g->get_wv(fn[col][2]);
				int wv2=g->get_wv(fn[col][1])-g->get_wv(fn[col][2]);
				int wv3=g->get_wv(fn[col][2]);
				if(wv>wv2){
					fn[col][2]=fn[col][1]; wv3=wv2;
					fn[col][1]=wv; wv2=wv;
				}else if(wv>wv3){
					fn[col][2]=wv; wv2=wv;
				}

			//	tw[col][0]=wv1;
				tw[col][1]=wv1-wv2;
				tw[col][2]=wv2-wv3;
			//}else{
			//	tw[col][2]-= wv;
			//}

							  
							  
			//tw[col][2]-= wv;			
			//if( (g->get_wv(fn[col][1])-wv) < g->get_wv(fn[col][2]))					/* top-k-1 will not change in some cases*/
			//			tw[col][1]=max<int>(tw[col][1]-wv, 0);

			//attempt 2		
			//tw[col][1]= min<int>(tw[col][1],tw[col][0]-wv);	

			//attempt 3 (apparently working)
			//tw[col][1]=max<int>(tw[col][1]-wv, 0);	
			
			//LOG_INFO("TOP2:"<<v<<":"<<col);
			return TOP2;
	}

	//attempt TOP-K-1 and exit
	if( wv<=tw[col][1] && !g->get_neighbors(v).is_bit(fn[col][0]) ){

		tw[col][1]-=wv;	
		tw[col][2]=max<int>(tw[col][2]-wv, 0);


		//int wv1=g->get_wv(fn[col][0])-g->get_wv(fn[col][1]);
		//int wv2=g->get_wv(fn[col][1]);
		//int wv3=g->get_wv(fn[col][2]);
		//if(wv>wv1){
		//	fn[col][2]=fn[col][1]; wv3=wv2;
		//	fn[col][1]=wv; wv2=wv;
		////	tw[col][0]=wv1;
		//	tw[col][1]=wv1-wv2;
		//	tw[col][2]=wv2-wv3;
		//}else{




		//tw[col][1]-=wv;	
		//tw[col][2]=max<int>(tw[col][2]-wv, 0);

		//}
									
		//LOG_INFO("TOP1:"<<v<<":"<<col);
		return TOP1;
	}
	
	return NONE;
}

inline
UBWC::top_t UBWC::find_top_k_CW(int v, int col) {
/////////////////
// find_top_k_shared but swapping nodes to maximize tw 
//
// TODO-WRITE IN TERMS OF LOOPING THROUGH fn array 03/09/17)
//
// COMMENTS
// 1.A la Li (first and obvious attempt)


	int wv=g->get_wv(v);

	////RULE-early first node of clause change detection (working!, partly repeated in new color sets rule below)
	//if(wv>cw[col][0] && (wv-cw[col][0])<=GAP_SIZE && fn[col][2]==CLQ_MAXINT && fn[col][1]!=CLQ_MAXINT && !g->get_neighbors(v).is_bit(fn[col][1])  && !g->get_neighbors(v).is_bit(fn[col][0]) ){
	//	//LOG_INFO("UBWC::find_top_k_CW()-Possible SWAP: "<<"["<<v<<","<<wv<<"]"<<"\tgs:"<<"["<<wv-cw[col][0]<<","<<GAP_SIZE<<"]"<<" col:"<<col);
	//	//cin.get();

	//	////if(fn[col][1]!=CLQ_MAXINT){
	//	//	fn[col][2]=CLQ_MAXINT; cw[col][2]=CLQ_MAXINT;
	//	////}
	//	fn[col][2]=fn[col][1]; cw[col][2]=cw[col][1];
	//	fn[col][1]=fn[col][0]; cw[col][1]=cw[col][0];
	//	fn[col][0]=v; cw[col][0]=wv;
	//	GAP_SIZE-= (wv-cw[col][0]);
	//	return NEW_CLAUSE_WEIGHT;
	//}

	//original color sets of size 2- CHECK** NOT CLEAR AT ALL IF IMPROVES PERFORMANCE (number of steps reduces slightly)
	//if( col<FIRST_NEW_COL && fn[col][2]==CLQ_MAXINT && fn[col][1]!=CLQ_MAXINT && 
	//						  !g->get_neighbors(v).is_bit(fn[col][1]) &&
	//						  !g->get_neighbors(v).is_bit(fn[col][0])			){


	//	int diff=wv-cw[col][0];
	//	if(diff<=0){
	//		if(wv>cw[col][1]){
	//			fn[col][2]=fn[col][1]; cw[col][2]=cw[col][1];
	//			fn[col][1]=v; cw[col][1]=wv;
	//		}/*else{
	//			fn[col][2]=v; cw[col][2]=wv;			// there is no real need to add the node: the color set remains with two nodes, no top-k-2 though 
	//		}*/
	//		
	//		return ISET_FREE;
	//	}else if(diff<=GAP_SIZE /*diff MUST BE >0*/){
	//		//LOG_INFO("GAP:"<<wv-cw[col][0]<<"real GAP:"<<GAP_SIZE);
	//		//LOG_INFO("CUT-GAP:"<<wv-cw[col][0]<<" real GAP:"<<GAP_SIZE<<" col: "<<col;);
	//		GAP_SIZE-=diff;
	//		fn[col][2]=fn[col][1]; cw[col][2]=cw[col][1];
	//		fn[col][1]=fn[col][0]; cw[col][1]=cw[col][0];
	//		fn[col][0]=v; cw[col][0]=wv;
	//		
	//		return ISET_FREE;
	//	}

	//}
	
	//RULE new color sets (all have size 2)
	if( col>=FIRST_NEW_COL && fn[col][2]==CLQ_MAXINT && 
							  !g->get_neighbors(v).is_bit(fn[col][1]) &&
							  !g->get_neighbors(v).is_bit(fn[col][0])			){
	//	LOG_ERROR("INDEPENDENT SET FOUND:"<<v<<" col:"<<col<<" cw:"<<cw[col][0]<<","<<cw[col][1]<<" wv:"<<wv);
		//cin.get();
		int diff=wv-cw[col][0];
		if(diff<=0){
			if(wv>cw[col][1]){
				fn[col][2]=fn[col][1]; cw[col][2]=cw[col][1];
				fn[col][1]=v; cw[col][1]=wv;
			}
			//LOG_ERROR("ISET_FREE:"<<v);
			//cin.get();
			return ISET_FREE;
		}else if(diff<=GAP_SIZE /*diff MUST BE >0*/){
			//LOG_INFO("GAP:"<<wv-cw[col][0]<<"real GAP:"<<GAP_SIZE);
			//LOG_INFO("CUT-GAP:"<<wv-cw[col][0]<<" real GAP:"<<GAP_SIZE<<" col: "<<col;);
			GAP_SIZE-=diff;
			fn[col][2]=fn[col][1]; cw[col][2]=cw[col][1];
			fn[col][1]=fn[col][0]; cw[col][1]=cw[col][0];
			fn[col][0]=v; cw[col][0]=wv;
			
			return ISET_FREE;

			//cin.get();
		}
	}
					
	//RULE TOP-K2 and exit
	if(wv<=(cw[col][0]-cw[col][2]) && !g->get_neighbors(v).is_bit(fn[col][1]) &&   /* fn[col][1] must exist to meet the first cond*/
									  !g->get_neighbors(v).is_bit(fn[col][0])		){				
			//substitute clause
			cw[col][0]-=cw[col][2];
			cw[col][1]-=cw[col][2];
			if(wv>cw[col][1]){
				fn[col][2]=fn[col][1]; cw[col][2]=cw[col][1];
				fn[col][1]=v; cw[col][1]=wv;
			}else{
				fn[col][2]=v; cw[col][2]=wv;
			}			 	
			
			//LOG_INFO("TOP2:"<<v<<":"<<col);
			return TOP2;
	}

	//RULE TOP-K1 and exit
	if( wv<=(cw[col][0]-cw[col][1]) && !g->get_neighbors(v).is_bit(fn[col][0]) ){

		//new clause with 2 nodes
		NB_COL++; 
		fn[NB_COL][0]=fn[col][0];		/*NB_COL MUST BE >1*/
		/*if(fn[col][0]>m_NV){
			LOG_ERROR("bizarre node");
			cin.get();
		}*/
		cw[NB_COL][0]=cw[col][0]-cw[col][1];
		fn[NB_COL][1]=v;
		cw[NB_COL][1]=wv;
		fn[NB_COL][2]=CLQ_MAXINT;
		cw[NB_COL][2]=CLQ_MAXINT;

		//update existing clause
		cw[col][0]=cw[col][1];
											
		//LOG_INFO("TOP1:"<<v<<":"<<col);
		return TOP1;
	}
	
	return NONE;
}


inline
UBWC::top_t UBWC::find_top_k_CW_OPT(int v, int col) {
/////////////////
// find_top_k_shared but swapping nodes to maximize tw 
//
// TODO-WRITE IN TERMS OF LOOPING THROUGH fn array 03/09/17)
//
// COMMENTS
// 1.A la Li (first and obvious attempt)
	int wv=g->get_wv(v), diff=0, POINTER_NBOR=EMPTY_ELEM;
	 
	for(int i=0; i<3; i++){
		if(fn[col][i]==CLQ_MAXINT) break;
		if(g->get_neighbors(v).is_bit(fn[col][i])){
			POINTER_NBOR=i;
			break;
		}
	}
	
	//RULE-0 All antineighbors
	if(POINTER_NBOR==EMPTY_ELEM && fn[col][2]==CLQ_MAXINT){											 /* all non-neighbors  of v, MUST be an ISET of size 2 (**TODO- or KNOW its SIZE is 3 ) */
		//LOG_INFO("ISET FOUND:"<<col<<","<<fn[col][0]<<","<<fn[col][1]<<","<<fn[col][2]);
		//substitute clause
		diff=wv-cw[col][0];
		if(diff<0){
			if(fn[col][1]!=CLQ_MAXINT){
				if(wv>cw[col][1]){
					fn[col][2]=fn[col][1]; cw[col][2]=cw[col][1];
					fn[col][1]=v; cw[col][1]=wv;
				}else{
					fn[col][2]=v; cw[col][2]=wv;
				}
			}else{
				fn[col][1]=v; cw[col][1]=wv;
			}
			return ISET_FREE;
		}else if(diff<=GAP_SIZE){
			//LOG_INFO("GAP:"<<wv-cw[col][0]<<"real GAP:"<<GAP_SIZE);
			//LOG_INFO("CUT-GAP:"<<wv-cw[col][0]<<" real GAP:"<<GAP_SIZE<<" col: "<<col;);
			GAP_SIZE-=diff;
			if(fn[col][1]!=CLQ_MAXINT){
				fn[col][2]=fn[col][1]; cw[col][2]=cw[col][1];
			}else{
				fn[col][2]=CLQ_MAXINT; cw[col][2]=CLQ_MAXINT;
			}
			
			fn[col][1]=fn[col][0]; cw[col][1]=cw[col][0];
			fn[col][0]=v; cw[col][0]=wv;
			return ISET_FREE;
		}
			
		return NONE;
	}//end POINTER=0	
	

	//RULE-II (TOP-2-K)
	diff=cw[col][0]-cw[col][2];
	if((POINTER_NBOR==2) && wv<= (diff + GAP_SIZE)){					 
		//substitute clause
		cw[col][0]-=cw[col][2];
		cw[col][1]-=cw[col][2];
		if(wv>cw[col][1]){
			fn[col][2]=fn[col][1]; cw[col][2]=cw[col][1];
			fn[col][1]=v; cw[col][1]=wv;
		}else{
			fn[col][2]=v; cw[col][2]=wv;
		}	

		if(wv>diff)
			GAP_SIZE-=(wv-diff);

		//LOG_INFO("TOP2:"<<v<<":"<<col);
		return TOP2;
	}

	//RULE-I (TOP-1-K)
	diff=cw[col][0]-cw[col][1];
	if((POINTER_NBOR==1) && wv<=(diff + GAP_SIZE)){			
		
		//I.add new clause with 2 nodes
		NB_COL++; 
		fn[NB_COL][0]=fn[col][0];		/*NB_COL MUST BE >1*/
		/*if(fn[col][0]>m_NV){
		LOG_ERROR("bizarre node");
		cin.get();
		}*/
		cw[NB_COL][0]=cw[col][0]-cw[col][1];
		fn[NB_COL][1]=v;
		cw[NB_COL][1]=wv;
		fn[NB_COL][2]=CLQ_MAXINT;
		cw[NB_COL][2]=CLQ_MAXINT;

		//II.update existing clause
		cw[col][0]=cw[col][1];

		if(wv>diff)
			GAP_SIZE-=(wv-diff);

		//LOG_INFO("TOP1:"<<v<<":"<<col);
		return TOP1;
	}

	return NONE;
}

inline
UBWC::top_t UBWC::find_top_k_CW_OPT_T2split(int v, int col) {
/////////////////
// find_top_k_shared but swapping nodes to maximize tw 
//
// TODO-WRITE IN TERMS OF LOOPING THROUGH fn array 03/09/17)
//
// COMMENTS
// 1.A la Li (first and obvious attempt)
	int wv=g->get_wv(v), diff=0, POINTER_NBOR=EMPTY_ELEM;
	bitarray& bbnv=g->get_neighbors(v);
	 
	for(int i=0; i<3; i++){
		if(fn[col][i]==CLQ_MAXINT) break;
	//	if(g->get_neighbors(v).is_bit(fn[col][i])){
		if(g->is_edge(v,fn[col][i])){
			POINTER_NBOR=i;
			break;
		}
	}
		
	//RULE-0 All anti-neighbors
	if(POINTER_NBOR==EMPTY_ELEM  /*&&  fn[col][2]==CLQ_MAXINT*/){												/* all non-neighbors  of v, MUST BE an ISET of size 2 (***TODO-or KNOW the size is 3 ) */
		if(fn[col][2]==CLQ_MAXINT){
			//LOG_INFO("ISET FOUND:"<<col<<","<<fn[col][0]<<","<<fn[col][1]<<","<<fn[col][2]);
			//substitute clause
			diff=wv-cw[col][0];
			if(diff<0){
				if(fn[col][1]!=CLQ_MAXINT){
					if(wv>cw[col][1]){
						fn[col][2]=fn[col][1]; cw[col][2]=cw[col][1];
						fn[col][1]=v; cw[col][1]=wv;
					}else{
						fn[col][2]=v; cw[col][2]=wv;
					}
				}else{
					fn[col][1]=v; cw[col][1]=wv;
				}
				return ISET_FREE;
			}else if(diff<=GAP_SIZE){
				//LOG_INFO("GAP:"<<wv-cw[col][0]<<"real GAP:"<<GAP_SIZE);
				//LOG_INFO("CUT-GAP:"<<wv-cw[col][0]<<" real GAP:"<<GAP_SIZE<<" col: "<<col;);
				GAP_SIZE-=diff;
				if(fn[col][1]!=CLQ_MAXINT){
					fn[col][2]=fn[col][1]; cw[col][2]=cw[col][1];
				}else{
					fn[col][2]=CLQ_MAXINT; cw[col][2]=CLQ_MAXINT;
				}

				fn[col][1]=fn[col][0]; cw[col][1]=cw[col][0];
				fn[col][0]=v; cw[col][0]=wv;
				return ISET_FREE;
			}

		//	LOG_INFO("VERTEX TO APPLY RESIDUAL WEIGHT:"<<v<<","<<wv<<","<<cw[col][0]);
		//	cin.get();
			POINTER_NBOR=1;	

		}else{
			POINTER_NBOR=2;  /* fn[col][2]!=CLQ_MAXINT */

			//update POINTER to the best possible place
			//(fn[col][2]!=CLQ_MAXINT)? POINTER_NBOR=2: POINTER_NBOR=1;	 	
			//return NONE;
		}		
	}
	

	//RULE-II (TOP-2-K)
	diff=cw[col][0]-cw[col][2];
	if((POINTER_NBOR==2) && wv<= (diff + GAP_SIZE)){	

		//new clause
		NB_COL++; 
		int wn0=diff;
		int wn1=cw[col][1]-cw[col][2];
		if(wv>wn0){
		//	LOG_INFO("Top-2 split GT new weight of clause");
			fn[NB_COL][0]=v;
			cw[NB_COL][0]=wv;
			fn[NB_COL][1]=fn[col][0];		
			cw[NB_COL][1]=wn0;
			fn[NB_COL][2]=fn[col][1];
			cw[NB_COL][2]=wn1;
		}else if(wv>wn1){
			fn[NB_COL][0]=fn[col][0];		
			cw[NB_COL][0]=wn0;
			fn[NB_COL][1]=v;
			cw[NB_COL][1]=wv;
			fn[NB_COL][2]=fn[col][1];
			cw[NB_COL][2]=wn1;
		}else{
			fn[NB_COL][0]=fn[col][0];		
			cw[NB_COL][0]=wn0;
			fn[NB_COL][1]=fn[col][1];
			cw[NB_COL][1]=wn1;
			fn[NB_COL][2]=v;
			cw[NB_COL][2]=wv;
		}

		//old clause
		cw[col][0]=cw[col][2];
		cw[col][1]=cw[col][2];

		//swap weights of middle node (new ISET->old ISET)-TODO CHECK*** (18/9/17)
		int wsap=cw[NB_COL][1]-cw[NB_COL][2];
		if(wsap>0){
		//	LOG_INFO("WEIGHT TRANSFER");
			int v0=fn[col][0]; int wv0=cw[col][0];
			fn[col][0]=fn[col][1];
			cw[col][0]=cw[col][1]+wsap;

			fn[col][1]=v0;
			cw[col][1]=wv0;
			
			cw[NB_COL][1]=cw[NB_COL][2];
		}
				
		if(wv>diff)
			GAP_SIZE-=(wv-diff);

		//LOG_INFO("TOP2:"<<v<<":"<<col);
		return TOP2;
	}

	//RULE-I (TOP-1-K)
	diff=cw[col][0]-cw[col][1];
	if((POINTER_NBOR==1) && wv<=(diff + GAP_SIZE)){			
		
		//I.add new clause with 2 nodes
		NB_COL++; 

		if(wv<diff){
			fn[NB_COL][0]=fn[col][0];					/*NB_COL MUST BE >1*/
			cw[NB_COL][0]=diff;
			fn[NB_COL][1]=v;
			cw[NB_COL][1]=wv;
		}else{											/* wv==diff swap nodes also; looks better to change first vertex if possible */
			fn[NB_COL][1]=fn[col][0];					
			cw[NB_COL][1]=diff;
			fn[NB_COL][0]=v;
			cw[NB_COL][0]=wv;
		}
	
		fn[NB_COL][2]=CLQ_MAXINT;
		cw[NB_COL][2]=CLQ_MAXINT;

		
		//II.update existing clause
		cw[col][0]=cw[col][1];


		//swap weights (new->old clause)			/* Does not seem to improve */
		//int wsap=cw[NB_COL][0]-cw[NB_COL][1];
		//if(wsap>0){
		//	//LOG_INFO("WEIGHT TRANSFER TOP-1");
		//	cw[col][0]=cw[col][1]+wsap;
		//	cw[NB_COL][0]=cw[NB_COL][1];
		//}
				

		if(wv>diff)
			GAP_SIZE-=(wv-diff);

		//LOG_INFO("TOP1:"<<v<<":"<<col);
		return TOP1;
	}

	return NONE;
}


inline
UBWC::top_t UBWC::find_top_k_CW_OPT_T2split(int v, int col, int NEW_COL_POINTER) {
/////////////////
// attempt to cover v by col completely (will not change the bound of color_db)
// a fresh clause produced by splitting will be written in NEW_COL_POINTER
//
// COMMENTS
// 1.to be used during coloring
// 2. NEW_COL_POINTER corresponds with the current color being built durint paint,
//    which should be empty
//
// BUG-sanr400_0.7.clq!! TODO*

	int wv=g->get_wv(v), diff=0, POINTER_NBOR=EMPTY_ELEM;
	 
	for(int i=0; i<3; i++){
		if(fn[col][i]==CLQ_MAXINT) break;
	//	if(g->get_neighbors(v).is_bit(fn[col][i])){
		if(g->is_edge(v,fn[col][i])){
			POINTER_NBOR=i;
			break;
		}
	}
		
	//RULE-0 All anti-neighbors
	if(POINTER_NBOR==EMPTY_ELEM  /*&&  fn[col][2]==CLQ_MAXINT*/){												/* all non-neighbors  of v, MUST BE an ISET of size 2 (***TODO-or KNOW the size is 3 ) */
		if(fn[col][2]==CLQ_MAXINT){
			//LOG_INFO("ISET FOUND:"<<col<<","<<fn[col][0]<<","<<fn[col][1]<<","<<fn[col][2]);
			//substitute clause
			if((wv-cw[col][0])<0){
				if(fn[col][1]!=CLQ_MAXINT){
					if(wv>cw[col][1]){
						fn[col][2]=fn[col][1]; cw[col][2]=cw[col][1];
						fn[col][1]=v; cw[col][1]=wv;
					}else{
						fn[col][2]=v; cw[col][2]=wv;
					}
				}else{
					fn[col][1]=v; cw[col][1]=wv;
				}
				return ISET_FREE;
			}

		//	LOG_INFO("VERTEX TO APPLY RESIDUAL WEIGHT:"<<v<<","<<wv<<","<<cw[col][0]);
		//	cin.get();
			POINTER_NBOR=1;	
			//return NONE;

		}else{
			POINTER_NBOR=2;  /* fn[col][2]!=CLQ_MAXINT */

			//update POINTER to the best possible place
			//(fn[col][2]!=CLQ_MAXINT)? POINTER_NBOR=2: POINTER_NBOR=1;	 	
			//return NONE;
		}		
	}
	

	////RULE-II (TOP-2-K)
	//diff=cw[col][0]-cw[col][2];
	//if((POINTER_NBOR==2) && wv<= diff){	

	//	//new clause
	//	NB_COL=NEW_COL_POINTER;
	//	int wn0=diff;
	//	int wn1=cw[col][1]-cw[col][2];
	///*	if(wv>wn0){
	//	//	LOG_INFO("Top-2 split GT new weight of clause");
	//		fn[NB_COL][0]=v;
	//		cw[NB_COL][0]=wv;
	//		fn[NB_COL][1]=fn[col][0];		
	//		cw[NB_COL][1]=wn0;
	//		fn[NB_COL][2]=fn[col][1];
	//		cw[NB_COL][2]=wn1;
	//	}else*/ if(wv>wn1){
	//		fn[NB_COL][0]=fn[col][0];		
	//		cw[NB_COL][0]=wn0;
	//		fn[NB_COL][1]=v;
	//		cw[NB_COL][1]=wv;
	//		fn[NB_COL][2]=fn[col][1];
	//		cw[NB_COL][2]=wn1;
	//	}else{
	//		fn[NB_COL][0]=fn[col][0];		
	//		cw[NB_COL][0]=wn0;
	//		fn[NB_COL][1]=fn[col][1];
	//		cw[NB_COL][1]=wn1;
	//		fn[NB_COL][2]=v;
	//		cw[NB_COL][2]=wv;
	//	}

	//	//old clause
	//	cw[col][0]=cw[col][2];
	//	cw[col][1]=cw[col][2];

	//	////swap weights of middle node (new ISET->old ISET) -*** BUGGY! san200_0.7_2
	//	//int wsap=cw[NB_COL][1]-cw[NB_COL][2];
	//	//if(wsap>0){
	//	////	LOG_INFO("WEIGHT TRANSFER");
	//	//	int v0=fn[col][0]; int wv0=cw[col][0];
	//	//	fn[col][0]=fn[col][1];
	//	//	cw[col][0]=cw[col][1]+wsap;

	//	//	fn[col][1]=v0;
	//	//	cw[col][1]=wv0;
	//	//	
	//	//	cw[NB_COL][1]=cw[NB_COL][2];
	//	//}
	//	
	//	//LOG_INFO("TOP2:"<<v<<":"<<col);
	//	return TOP2;
	//}

	//RULE-I (TOP-1-K)
	diff=cw[col][0]-cw[col][1];
	if((POINTER_NBOR==1) && wv<=diff){			
		
		//I.add new clause with 2 nodes
		NB_COL=NEW_COL_POINTER;
		
	//	if(wv<diff){
			fn[NB_COL][0]=fn[col][0];					/*NB_COL MUST BE >1*/
			cw[NB_COL][0]=diff;
			fn[NB_COL][1]=v;
			cw[NB_COL][1]=wv;
/*		}else{											
			fn[NB_COL][1]=fn[col][0];					
			cw[NB_COL][1]=diff;
			fn[NB_COL][0]=v;
			cw[NB_COL][0]=wv;
		}*/
	
		fn[NB_COL][2]=CLQ_MAXINT;
		cw[NB_COL][2]=CLQ_MAXINT;
				
		//II.update existing clause
		cw[col][0]=cw[col][1];
							

		//LOG_INFO("TOP1:"<<v<<":"<<col);
		return TOP1;
	}

	return NONE;
}

inline
UBWC::top_t UBWC::find_top_k_CW_OPT_T2split_resw(int v, int col, int& wv, com::stack_t<int>& c_stack) {
/////////////////
// find_top_k_shared but swapping nodes to maximize tw 
//
// TODO-WRITE IN TERMS OF LOOPING THROUGH fn array 03/09/17)
//
// COMMENTS
// 1.A la Li (first and obvious attempt)
	int /*wv=g->get_wv(v),*/ diff=0, POINTER_NBOR=EMPTY_ELEM;
	//bitarray& bbnv=g->get_neighbors(v);
	for(int i=0; i<3; i++){
		if(fn[col][i]==CLQ_MAXINT) break;
		if(g->is_edge(v,fn[col][i])){
		//if(bbnv.is_bit(fn[col][i])){
			POINTER_NBOR=i;
			break;
		}
	}
		
	//RULE-0 All anti-neighbors only for 2node ISETS
	if(POINTER_NBOR==EMPTY_ELEM  /*&&  fn[col][2]==CLQ_MAXINT*/){												/* all non-neighbors  of v, MUST BE an ISET of size 2 (***TODO-or KNOW the size is 3 ) */
		if(fn[col][2]==CLQ_MAXINT){
			//LOG_INFO("ISET FOUND:"<<col<<","<<fn[col][0]<<","<<fn[col][1]<<","<<fn[col][2]);
			//substitute clause
			diff=wv-cw[col][0];
			if(diff<0){
				if(fn[col][1]!=CLQ_MAXINT){
					if(wv>cw[col][1]){
						fn[col][2]=fn[col][1]; cw[col][2]=cw[col][1];
						fn[col][1]=v; cw[col][1]=wv;
					}else{
						fn[col][2]=v; cw[col][2]=wv;
					}
				}else{
					fn[col][1]=v; cw[col][1]=wv;
				}

				//NOTE:LAST NODE OF INFERENCET-DOES NOT GO TO STACKS
				
				return ISET_FREE;
			}else if(diff<=GAP_SIZE){
				//LOG_INFO("GAP:"<<wv-cw[col][0]<<"real GAP:"<<GAP_SIZE);
				//LOG_INFO("CUT-GAP:"<<wv-cw[col][0]<<" real GAP:"<<GAP_SIZE<<" col: "<<col;);
				GAP_SIZE-=diff;
				if(fn[col][1]!=CLQ_MAXINT){
					fn[col][2]=fn[col][1]; cw[col][2]=cw[col][1];
				}else{
					fn[col][2]=CLQ_MAXINT; cw[col][2]=CLQ_MAXINT;
				}

				fn[col][1]=fn[col][0]; cw[col][1]=cw[col][0];
				fn[col][0]=v; cw[col][0]=wv;
				return ISET_FREE;
			}

			//RESIDUAL WEIGHT UPDATE-NOT ENOUGH TO CUT
			c_stack.push(col);
			//LOG_INFO("VERTEX TO APPLY RESIDUAL WEIGHT RW:"<<v<<","<<wv<<","<<wv-cw[col][0]);
			wv-=cw[col][0];		
			return NONE;
		}else{
			POINTER_NBOR=2;  /* fn[col][2]!=CLQ_MAXINT */

			//update POINTER to the best possible place
			//(fn[col][2]!=CLQ_MAXINT)? POINTER_NBOR=2: POINTER_NBOR=1;	 	
			//return NONE;
		}		
	}
	

	//RULE-II (TOP-2-K)
	diff=cw[col][0]-cw[col][2];
	if((POINTER_NBOR==2) && wv<= (diff + GAP_SIZE)){	

		//new clause
		NB_COL++; 
		int wn0=diff;
		int wn1=cw[col][1]-cw[col][2];
		if(wv>wn0){
		//	LOG_INFO("Top-2 split GT new weight of clause");
			fn[NB_COL][0]=v;
			cw[NB_COL][0]=wv;
			fn[NB_COL][1]=fn[col][0];		
			cw[NB_COL][1]=wn0;
			fn[NB_COL][2]=fn[col][1];
			cw[NB_COL][2]=wn1;
		}else if(wv>wn1){
			fn[NB_COL][0]=fn[col][0];		
			cw[NB_COL][0]=wn0;
			fn[NB_COL][1]=v;
			cw[NB_COL][1]=wv;
			fn[NB_COL][2]=fn[col][1];
			cw[NB_COL][2]=wn1;
		}else{
			fn[NB_COL][0]=fn[col][0];		
			cw[NB_COL][0]=wn0;
			fn[NB_COL][1]=fn[col][1];
			cw[NB_COL][1]=wn1;
			fn[NB_COL][2]=v;
			cw[NB_COL][2]=wv;
		}

		//old clause
		cw[col][0]=cw[col][2];
		cw[col][1]=cw[col][2];

		//swap weights of middle node (new ISET->old ISET)
		int wsap=cw[NB_COL][1]-cw[NB_COL][2];
		if(wsap>0){
		//	LOG_INFO("WEIGHT TRANSFER");
			int v0=fn[col][0]; int wv0=cw[col][0];
			fn[col][0]=fn[col][1];
			cw[col][0]=cw[col][1]+wsap;

			fn[col][1]=v0;
			cw[col][1]=wv0;
			
			cw[NB_COL][1]=cw[NB_COL][2];
		}
				
		if(wv>diff)
			GAP_SIZE-=(wv-diff);

		//LOG_INFO("TOP2:"<<v<<":"<<col);
		return TOP2;
	}

	//RULE-I (TOP-1-K)
	diff=cw[col][0]-cw[col][1];
	if((POINTER_NBOR==1) && wv<=(diff + GAP_SIZE)){			
		
		//I.add new clause with 2 nodes
		NB_COL++; 

		if(wv<diff){
			fn[NB_COL][0]=fn[col][0];					/*NB_COL MUST BE >1*/
			cw[NB_COL][0]=diff;
			fn[NB_COL][1]=v;
			cw[NB_COL][1]=wv;
		}else{											/* wv==diff swap nodes also; looks better to change first vertex if possible */
			fn[NB_COL][1]=fn[col][0];					
			cw[NB_COL][1]=diff;
			fn[NB_COL][0]=v;
			cw[NB_COL][0]=wv;
		}
	
		fn[NB_COL][2]=CLQ_MAXINT;
		cw[NB_COL][2]=CLQ_MAXINT;

		
		//II.update existing clause
		cw[col][0]=cw[col][1];


		//swap weights (new->old clause)			/* Does not seem to improve */
		//int wsap=cw[NB_COL][0]-cw[NB_COL][1];
		//if(wsap>0){
		//	//LOG_INFO("WEIGHT TRANSFER TOP-1");
		//	cw[col][0]=cw[col][1]+wsap;
		//	cw[NB_COL][0]=cw[NB_COL][1];
		//}
				

		if(wv>diff)
			GAP_SIZE-=(wv-diff);

		//LOG_INFO("TOP1:"<<v<<":"<<col);
		return TOP1;
	}

	return NONE;
}

inline
UBWC::top_t UBWC::find_top_k_CW_OPT_T2split_3S(int v, int col) {
/////////////////
// find_top_k_shared but swapping nodes to maximize tw. INCLUDES SIZE_3 info 
//
// TODO-WRITE IN TERMS OF LOOPING THROUGH fn array 03/09/17)
//
// COMMENTS
// 1.The impact of SIZE_3 info seems small

	int wv=g->get_wv(v), diff=0, POINTER_NBOR=EMPTY_ELEM;
	bitarray& bbnv=g->get_neighbors(v);
	
	for(int i=0; i<3; i++){
		if(fn[col][i]==CLQ_MAXINT) break;
		if(bbnv.is_bit(fn[col][i])){
			POINTER_NBOR=i;
			break;
		}
	}
			
	//RULE-0 All anti-neighbors
	if(POINTER_NBOR==EMPTY_ELEM ){															/* all non-neighbors  of v and ISET of size 3  */
		//LOG_INFO("ISET FOUND:"<<col<<","<<fn[col][0]<<","<<fn[col][1]<<","<<fn[col][2]);
			
		if(fn[col][3]==SIZE_3_ISET){
			//substitute clause
			diff=wv-cw[col][0];
			if(diff<0){
				if(fn[col][1]!=CLQ_MAXINT){

					//update SIZE_3															/* MUST BE HERE */
					if(fn[col][2]!=CLQ_MAXINT)
						fn[col][3]=CLQ_MAXINT;

					if(wv>cw[col][1]){
						fn[col][2]=fn[col][1]; cw[col][2]=cw[col][1];
						fn[col][1]=v; cw[col][1]=wv;
					}else if(wv>cw[col][2]){
						fn[col][2]=v; cw[col][2]=wv;
					}

				}else{
					fn[col][1]=v; cw[col][1]=wv;
				}
				return ISET_FREE;
			}else if(diff<=GAP_SIZE){
				//LOG_INFO("GAP:"<<wv-cw[col][0]<<"real GAP:"<<GAP_SIZE);
				//LOG_INFO("CUT-GAP:"<<wv-cw[col][0]<<" real GAP:"<<GAP_SIZE<<" col: "<<col;);
				GAP_SIZE-=diff;

				//update SIZE_3																/* MUST BE HERE */
				if(fn[col][2]!=CLQ_MAXINT)
					fn[col][3]=CLQ_MAXINT;


				if(fn[col][1]!=CLQ_MAXINT){
					fn[col][2]=fn[col][1]; cw[col][2]=cw[col][1];
				}else{
					fn[col][2]=CLQ_MAXINT; cw[col][2]=CLQ_MAXINT;
				}

				fn[col][1]=fn[col][0]; cw[col][1]=cw[col][0];
				fn[col][0]=v; cw[col][0]=wv;
				return ISET_FREE;
			}
		}
			
		return NONE;
	}//end NO-NEIGHBORS case
		

	//RULE-II (TOP-2-K)
	diff=cw[col][0]-cw[col][2];
	if((POINTER_NBOR==2) && wv<= (diff + GAP_SIZE)){	

		//new clause
		NB_COL++; 
		int wn0=diff;
		int wn1=cw[col][1]-cw[col][2];
		if(wv>wn0){
		//	LOG_INFO("Top-2 split GT new weight of clause");
			fn[NB_COL][0]=v;
			cw[NB_COL][0]=wv;
			fn[NB_COL][1]=fn[col][0];		
			cw[NB_COL][1]=wn0;
			fn[NB_COL][2]=fn[col][1];
			cw[NB_COL][2]=wn1;
		}else if(wv>wn1){
			fn[NB_COL][0]=fn[col][0];		
			cw[NB_COL][0]=wn0;
			fn[NB_COL][1]=v;
			cw[NB_COL][1]=wv;
			fn[NB_COL][2]=fn[col][1];
			cw[NB_COL][2]=wn1;
		}else{
			fn[NB_COL][0]=fn[col][0];		
			cw[NB_COL][0]=wn0;
			fn[NB_COL][1]=fn[col][1];
			cw[NB_COL][1]=wn1;
			fn[NB_COL][2]=v;
			cw[NB_COL][2]=wv;
		}

		fn[NB_COL][3]=SIZE_3_ISET;	

		//old clause
		cw[col][0]=cw[col][2];
		cw[col][1]=cw[col][2];

		//swap weights of middle node (new ISET->old ISET)
		int wsap=cw[NB_COL][1]-cw[NB_COL][2];
		if(wsap>0){
		//	LOG_INFO("WEIGHT TRANSFER");
			int v0=fn[col][0]; int wv0=cw[col][0];
			fn[col][0]=fn[col][1];
			cw[col][0]=cw[col][1]+wsap;

			fn[col][1]=v0;
			cw[col][1]=wv0;
			
			cw[NB_COL][1]=cw[NB_COL][2];
		}
				
		if(wv>diff)
			GAP_SIZE-=(wv-diff);

		//LOG_INFO("TOP2:"<<v<<":"<<col);
		return TOP2;
	}

	//RULE-I (TOP-1-K)
	diff=cw[col][0]-cw[col][1];
	if((POINTER_NBOR==1) && wv<=(diff + GAP_SIZE)){			
		
		//I.add new clause with 2 nodes
		NB_COL++; 

		if(wv<diff){
			fn[NB_COL][0]=fn[col][0];					/*NB_COL MUST BE >1*/
			cw[NB_COL][0]=diff;
			fn[NB_COL][1]=v;
			cw[NB_COL][1]=wv;
		}else{											/* wv==diff swap nodes also; looks better to change first vertex if possible */
			fn[NB_COL][1]=fn[col][0];					
			cw[NB_COL][1]=diff;
			fn[NB_COL][0]=v;
			cw[NB_COL][0]=wv;
		}

		fn[NB_COL][2]=CLQ_MAXINT;	
		fn[NB_COL][3]=SIZE_3_ISET;	

		//II.update existing clause
		cw[col][0]=cw[col][1];


		//swap weights (new->old clause)			/* Does not seem to improve */
		//int wsap=cw[NB_COL][0]-cw[NB_COL][1];
		//if(wsap>0){
		//	//LOG_INFO("WEIGHT TRANSFER TOP-1");
		//	cw[col][0]=cw[col][1]+wsap;
		//	cw[NB_COL][0]=cw[NB_COL][1];
		//}
				

		if(wv>diff)
			GAP_SIZE-=(wv-diff);

		//LOG_INFO("TOP1:"<<v<<":"<<col);
		return TOP1;
	}

	return NONE;
}

inline
UBWC::top_t UBWC::find_top_k_overlap (int v, int col_ini, int col_end){
	int wcov=0, wv=g->get_wv(v);
	bool cover_found=false;
	bitarray& bbnv=g->get_neighbors(v);

	//stack_t<int> colors(col_end-col_ini+1);					/* TODO-Optimize allocation! */
	//stack_t<top_t> cover_mode(col_end-col_ini+1);

	colors.erase();

	for(int c=col_ini; c<=col_end; c++){
		if(tw[c][1]==0) continue;
		//attempt top_2
		if(tw[c][2]>0 && !bbnv.is_bit(fn[c][1]) && !bbnv.is_bit(fn[c][0])){
			colors.push(c);
		//	cover_mode.push(TOP2);
		//	nodes.push(c);
			if((wcov+=tw[c][2])>=wv){
				cover_found=true;
				break;
			}
			continue;				/* no TOP1 after TOP2*/
		}

		//attempt top_1
		if(tw[c][1]>0 && !bbnv.is_bit(fn[c][0]) ){
			colors.push(c);
		//	cover_mode.push(TOP1);
		//	nodes.push(c);
			if((wcov+=tw[c][1])>=wv){
				cover_found=true;
				break;
			}
		}

	}//next color

	if(cover_found){

		//update tw: the full tw is taken in each case
		//for(int i=0; i<colors.size(); i++){
		//	top_t res=cover_mode.get_elem(i);
		//	int col=colors.get_elem(i);
		for(int i=0; i<colors.size(); i++){
			int col=colors.get_elem(i);
			cw[col][1]=0;	
			tw[col][2]=0;
			//if(res==TOP1){
			//	tw[col][1]=0;	
			//	tw[col][2]=0;
			//}else if(res==TOP2){
			//	//tw[col][1]= min<int>(tw[col][1],tw[col][0]-tw[col][2]);			/* TODO-CHECK */
			//	tw[col][1]=0;
			//	tw[col][2]=0;
			//}			
		}
		return TOP_OVERLAP;
	}
	return NONE;
}

inline
UBWC::top_t UBWC::find_top_k_overlap_CW (int v){
///////////////////////
// Overlap-reasoning engine- weights of vertices are distributed
// along the ISETS
//
// COMMENTS: 
// 1. the reasoning is limited to ONE-USE for each ISET
// 2. it is applied as a last filter, since it corrupts the ISETS for further use

	int wcov=0, wv=g->get_wv(v);
	colors.erase();
	enum type_t {NON_NEIGHBORS=0, TOP_1, TOP_2};
	const int COLOR_USED=-1;
	bitarray& bbnv=g->get_neighbors(v);
	
	bool cover_found=false;
	for(int c=1; c<=NB_COL; c++){
		if(cw[c][1]==COLOR_USED) continue;

		//all non-neighbors (9/9/17) -TODO** INCOMPLETE
		/*if(fn[c][1]!=CLQ_MAXINT &&  fn[c][2]==CLQ_MAXINT && !g->get_neighbors(v).is_bit(fn[c][1])
			&& !g->get_neighbors(v).is_bit(fn[c][0]) ){*/
		if(fn[c][1]!=CLQ_MAXINT &&  fn[c][2]==CLQ_MAXINT && !g->is_edge(v,fn[c][1]) 
			&& !g->is_edge(v,fn[c][0]) ){	

			//LOG_INFO("OVERLAP ATTEMPT:"<<wv-cw[c][0]);
			int diff=wv-cw[c][0];
			if(diff<0){				
			//	LOG_INFO("UBWC::find_top_k_overlap_CW()-BIZARRE WEIGHT OF NODE");
				cw[c][1]=COLOR_USED;				
				return TOP_OVERLAP;
			}else{
				colors.push(c);
				if( (wcov+=diff)/*+GAP_SIZE */>=wv ){  /* TODO-GAP_SIZE not working HERE CHECK!!! */
					//if(wcov<wv){
					////	LOG_INFO("OVERLAP NON-NEIGHBORS WITH GAP");
					//	GAP_SIZE-=(wv-wcov);
					//}
					cover_found=true;									
					break;
				}
				continue;		/* no TOP1 after TOP2*/
			}	
		}

		//TOP-2
		int tk2=cw[c][0]-cw[c][2];
	//	if(tk2>0 /* CHECK */ && !g->get_neighbors(v).is_bit(fn[c][1]) && !g->get_neighbors(v).is_bit(fn[c][0])){
		if(tk2>0 /* CHECK */ && !g->is_edge(v,fn[c][1]) && !g->is_edge(v,fn[c][0]) ){
			colors.push(c);	
			if((wcov+=tk2)+GAP_SIZE>=wv){
				if(wcov<wv){
					//LOG_INFO("OVERLAP WITH GAP");
					GAP_SIZE-=(wv-wcov);
				}
				cover_found=true;
				break;
			}
			continue;				/* no TOP1 after TOP2*/
		}

		//TOP-1
		int tk1=cw[c][0]-cw[c][1];
	//	if(tk1>0 /* CHECK */ && !g->get_neighbors(v).is_bit(fn[c][0]) ){
		if(tk1>0 /* CHECK */ && !g->is_edge(v,fn[c][0]) ){
			colors.push(c);		
			if((wcov+=tk1)+GAP_SIZE>=wv){
				if(wcov<wv){
					//LOG_INFO("OVERLAP WITH GAP");
					GAP_SIZE-=(wv-wcov);
				}
				cover_found=true;
				break;
			}
		}
	}//next color

	if(cover_found){
		for(int i=0; i<colors.size(); i++){
			int col=colors.get_elem(i);
			cw[col][1]=COLOR_USED;
		//	cw[col][1]=0;	
		//	cw[col][2]=0;			

		}//next color
		return TOP_OVERLAP;
	}
	return NONE;
}

inline
UBWC::top_t UBWC::find_top_k_overlap_CW_plus (int v){
///////////////////////
// Overlap-reasoning engine- weights of vertices are distributed
// along the ISETS ENHANCED
//
// COMMENTS: 
// 1. the reasoning is limited to ONE-USE for each ISET
// 2. it is applied as a last filter, since it corrupts the ISETS for further use

	int wcov=0, wv=g->get_wv(v);
	colors.erase();
	node_cover.erase();
	enum type_t {NON_NEIGHBORS=0, TOP_1, TOP_2};
	const int COLOR_USED=-1;
	bitarray& bbnv=g->get_neighbors(v);
	
	bool cover_found=false;
	for(int c=1; c<=NB_COL; c++){
		if(cw[c][1]==COLOR_USED) continue;

		//all non-neighbors (9/9/17) -TODO** INCOMPLETE
		if(fn[c][1]!=CLQ_MAXINT &&  fn[c][2]==CLQ_MAXINT && !bbnv.is_bit(fn[c][1])
			&& !bbnv.is_bit(fn[c][0]) ){
				//ASSERT-should not occurr often!
				if(wv-cw[c][0]<0){				
					//LOG_INFO("UBWC::find_top_k_overlap_CW()-BIZARRE WEIGHT OF NODE");
					//update clause
					if(wv>cw[c][1]){			
						fn[c][2]=fn[c][1];
						cw[c][2]=cw[c][1];
						fn[c][1]=v;
						cw[c][1]=wv;
					}else{
						fn[c][2]=v;
						cw[c][2]=wv;
					}

					cw[c][1]=COLOR_USED;				
					return TOP_OVERLAP;
				}

				//normal filter				
				int wred=(wv-wcov);
				int diff=wred-cw[c][0];								/* TODO-GAP SIZE*/
				if(diff<=0){   /* v is  covered in full */

					//updates last clause
					if(wred-diff>cw[c][1]){
						fn[c][2]=fn[c][1];
						cw[c][2]=cw[c][1];
						fn[c][1]=v;
						cw[c][1]=wred;
					}else{
						fn[c][2]=v;
						cw[c][2]=wred;
					}

					//LOG_INFO("UBWC::find_top_k_overlap_CW()-COVER_OVERLAP_FOUND NON NEIGHBOR");

					cover_found=true;	/* note the las ISET has not been added to colors */
					break;
				}
				//accumulates weight
				wcov+=cw[c][0];
				colors.push(c);	
				node_cover.push(NON_NEIGHBORS);
				continue;	
		}//end all-neighbors

		
		//TOP-2
		int tk2=cw[c][0]-cw[c][2];
		if(tk2>0 /* CHECK */ && !bbnv.is_bit(fn[c][1]) && !bbnv.is_bit(fn[c][0])){
			colors.push(c);	
			node_cover.push(TOP_1);
			
			if((wcov+=tk2)+GAP_SIZE>=wv){
				if(wcov<wv){
					//LOG_INFO("OVERLAP WITH GAP");
					GAP_SIZE-=(wv-wcov);
				}
				cover_found=true;
				break;
			}
			continue;				/* no TOP1 after TOP2*/


		}

		//TOP-1
		int tk1=cw[c][0]-cw[c][1];
		if(tk1>0 /* CHECK */ && !bbnv.is_bit(fn[c][0]) ){
			colors.push(c);	
			node_cover.push(TOP_2);
			if((wcov+=tk1)+GAP_SIZE>=wv){
				if(wcov<wv){
					//LOG_INFO("OVERLAP WITH GAP");
					GAP_SIZE-=(wv-wcov);
				}
				cover_found=true;
				break;
			}
		}
	}//next color

	if(cover_found){
		for(int i=0; i<colors.size(); i++){
			int c=colors.get_elem(i);
			
			if(node_cover.get_elem(c)==NON_NEIGHBORS ){
				//add filtered vertex to ISET 
				fn[c][2]=fn[c][1];
				cw[c][2]=cw[c][1];
				fn[c][1]=fn[c][0];
				cw[c][1]=cw[c][0];
				fn[c][0]=v;
				/*the weight of the ISET does not change */

			}else{
				cw[c][1]=COLOR_USED;
			}


		//	cw[col][1]=0;	
		//	cw[col][2]=0;			

		}//next color
		return TOP_OVERLAP;
	}
	return NONE;
}

inline
ostream& UBWC::print_tk	(int col_ini, int col_end /*1 based */, ostream& o){
///////////////
// prints top weight up to, and including, last_col
	for(int i=col_ini; i<=col_end; i++){
		o<<i<<"."<<"["<<tw[i][0]<<","<<tw[i][1]<<","<<tw[i][2]<<"] ";			
	}
	return o;
}


#endif


