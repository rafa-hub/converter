//init_color_ub.h: header for a specialization of InitColor to determine initial UBs for vertices in ugraphs
//
//date of creation: 15/09/15
//last update: 15/09/15
//author: pablo san segundo

#ifndef __INIT_COLOR_UB_H__
#define __INIT_COLOR_UB_H__

#include "init_color.h"
#include "graph/graph.h"
#include "utils/logger.h"
#include "utils/common.h"
#include "bitscan/bbalg.h"
#include <vector>
#include <algorithm>
#include "clique/infra_tools.h"
#include "clique/infra_tools_plus.h"

using namespace std;

typedef  vector<bitarray* > v_bb;
typedef vector<int> vint;

///////////////////////////
//
// InitColorUB class
// (only for ugraph )
//
////////////////////////////
class InitColorUB: public InitColor<ugraph>{
public:
	InitColorUB				(ugraph& g):InitColor<ugraph>(g), m_colsets(NULL), iop(&g){}
	virtual	~InitColorUB	(){clear_color_sets();}

	int Compute_trivial_UB	(int []);			//driver to compute UB in all vertices trivially: size of subgraphs (bounded by max graph deg+1)
	int Compute_incUB		(int []);			//driver to compute UB in all vertices, based on incremental, simple linear bound
	int Compute_UB			(int []);			//driver to compute UB in all vertices, based on UB of incremental subgraphs
	int Compute_UB_last		(int []);			//driver to compute a tighter UB in all vertices, based on UB of last vertex in incremental subgraphs

	int eval_init_order		();					//returns minimum value of paint_XRN_last for all vertices

	int Compute_UB_enhanced_last	(int []);
	int Compute_UB_enhanced_last	(int [], bitarray& bbs);				//oriented bounds for subgraph bbs (10/11/16)

private:
	int incUB					(bitarray& bb, const int []);				//oriented UB based on neighborhood
	int paint_XRN				(const bitarray& bb);						//oriented UB based on infra-coloring
	int paint_XRN_last			(const bitarray& bb);						//oriented UB based on infra-coloring of neighbor oriented subproblem (tighter than paint_XRN)	

//////////////////////
// even tighter bounds (6/16): SEQ + RECOL + MODERN INFRA-CHROM

	int paint_R					(const bitarray& bb);		
	int paint_R_last			(const bitarray& bb);	//oriented UB for the new tighter bounds
	
	int init_color_sets(int nCol);					
	void clear_color_sets();

/////////
//data members
	bitarray* m_colsets;	
	//InfraOp<ugraph,bitarray> iop;							  //for the new tight experimental UBs
	InfraOpPlusMaxConf<ugraph,bitarray> iop;				  //for the new tight experimental UBs
};

inline
void InitColorUB::clear_color_sets(){
	if(m_colsets) {
		delete [] m_colsets; 
		m_colsets=NULL;
	}
	m_colsets=NULL;

	iop.clear();
}

inline
int InitColorUB::init_color_sets(int nCol){
//////////////
// Assigns space for nCol+1 bitarrays (bitarray 0 will not be used)
// RETURNS 0 if succesful, -1 otherwise
	clear_color_sets();
	
	try{
		m_colsets= new bitarray[nCol+1];		//collor 0 will not be used
		int nV=g.number_of_vertices();
		for(int i=0; i<nCol+1; i++){		
			m_colsets[i].init(nV);
		}

		//extension for infrachrom-operations (21/6/16)
		iop.init(nCol);
	}
	catch(...){
		LOG_ERROR("InitColorUB::init_color_sets failed when assigining memory to color sets");
		return -1;
	}
	return 0;
}

inline
int InitColorUB::Compute_UB(int v_col[]){
/////////////////////
// Driver for initial upper bound: min(incUB, color+infra-chrom+recolor) ç
// in natural order (v1=1 ,etc.)
// date: 15/09/15
// author: pss
// 
// Iterates over root subproblems in natural order and determines a tight upper 
// bound for all vertices
//
// RETURNS 0 if OK or -1 if ERROR

	int inc; int sat;

	//initial values for set of subproblems m_sel and color of first vertex
	m_sel.erase_bit();		
	m_sel.set_bit(0);		
	v_col[0]=1;
	init_color_sets(g.number_of_vertices());					//*** TODO, reduce space

	//main loop iterating over all vertices except {0}
	for(int v=1; v<g.number_of_vertices(); v++){
		m_sel.set_bit(v);
		inc=incUB(m_sel,v_col); 
		sat=paint_XRN(m_sel);
		v_col[v]=min<int>(inc, sat);

							
		////monitoring
		//stringstrem sstr("computing UB: ");
		//sstr<<v; 
		//LOG_INFO(sstr.str()); 
	}
	return 0;
}

inline
int InitColorUB::Compute_trivial_UB(int v_col[]){
/////////////////////
// Driver for trivial initial upper bound: size of subgraphs bounded by maximun graph degree
// date: 17/03/17
// author: pss
// 
// RETURNS 0 if OK or -1 if ERROR
//
// COMMENTS: the reference trivial bound
		
	//initial values for set of subproblems m_sel and color of first vertex
	v_col[0]=1;
	int max_deg_ub=g.max_degree_of_graph()+1;
	for(int v=1; v<g.number_of_vertices(); v++){
		v_col[v]=min<int>(v+1,max_deg_ub);
	}

	return 0;
}


inline
int InitColorUB::Compute_incUB(int v_col[]){
/////////////////////
// Driver for LINEAR initial upper bound: min(incUB) in natural order (v1=1 ,etc.)
// date: 27/01/17
// author: pss
// 
// Iterates over root subproblems in natural order and determines a tight upper 
// bound for all vertices
//
// RETURNS 0 if OK or -1 if ERROR

	int inc; int sat;

	//initial values for set of subproblems m_sel and color of first vertex
	m_sel.erase_bit();		
	m_sel.set_bit(0);		
	v_col[0]=1;
	//init_color_sets(g.number_of_vertices());					//*** TODO, reduce space

	//main loop iterating over all vertices except {0}
	for(int v=1; v<g.number_of_vertices(); v++){
		m_sel.set_bit(v);
		inc=incUB(m_sel,v_col); 
	
							
		////monitoring
		//stringstrem sstr("computing UB: ");
		//sstr<<v; 
		//LOG_INFO(sstr.str()); 
	}
	return 0;
}

inline
int InitColorUB::Compute_UB_last (int v_col[]){
	int inc; int sat;

	//initial values for set of subproblems m_sel and color of first vertex
	m_sel.erase_bit();		
	m_sel.set_bit(0);		
	v_col[0]=1;
	init_color_sets(g.number_of_vertices());					//*** TODO, reduce space

	//main loop iterating over all vertices except {0}
	for(int v=1; v<g.number_of_vertices(); v++){
		m_sel.set_bit(v);
		inc=incUB(m_sel,v_col); 
		sat=paint_XRN_last(m_sel);								//*** sat cannot be -1, not checked
		v_col[v]=min<int>(inc, sat);
							
		////monitoring
		//stringstrem sstr("computing UB: ");
		//sstr<<v; 
		//LOG_INFO(sstr.str()); 
	}
	return 0;

}

inline
int InitColorUB::Compute_UB_enhanced_last (int v_col[]){
	int inc; int sat;

	//initial values for set of subproblems m_sel and color of first vertex
	m_sel.erase_bit();		
	m_sel.set_bit(0);		
	v_col[0]=1;
	init_color_sets(g.number_of_vertices());					//*** TODO, reduce space

	//main loop iterating over all vertices except {0}
	for(int v=1; v<g.number_of_vertices(); v++){
		m_sel.set_bit(v);
		inc=incUB(m_sel,v_col); 
		sat=paint_R_last(m_sel);								
		v_col[v]=min<int>(inc, sat);
							
		//monitoring
		//LOG_INFO(v<<":"<<inc<<":"<<sat);
	}
	return 0;
}

inline
int InitColorUB::Compute_UB_enhanced_last (int v_col[], bitarray& bbs){
//oriented bounds for subgraph bbs only (10/11/16)
//
// REMARKS
// 1.bbs should not be empty!

	int inc; int sat;

	//*** ASSERT NO EMPTY

	//initial values for set of subproblems m_sel and color of first vertex
	m_sel.erase_bit();
	int first_v=bbs.lsbn64();
	m_sel.set_bit(first_v);		
	v_col[first_v]=1;
	init_color_sets(bbs.popcn64());							//*** TODO, reduce space

	//main loop iterating over all vertices 
	bbs.erase_bit(first_v);
	bbs.init_scan(bbo::NON_DESTRUCTIVE);
	while(true){
		int v=bbs.next_bit();
		if(v==EMPTY_ELEM) break;
		
		m_sel.set_bit(v);
		inc=incUB(m_sel,v_col); 
		sat=paint_R_last(m_sel);								
		v_col[v]=min<int>(inc, sat);
							
		//monitoring
		//LOG_INFO(v<<":"<<inc<<":"<<sat);
	}

	bbs.set_bit(first_v);
	return 0;
}

inline
int InitColorUB::incUB(bitarray& bb, const int v_col []){
///////////////
//  Considers positional neihgbors to determine an incremental upper bound
//	RETURNS: 1+min(c(N(w)),|N(w)|) where w is the last vertex in the set 
//  (so N(w) is the neighbor set of preceding vertices)
//
//  Observations: The return value is an upper bound for w(bb)
// 
//  initial date: 28/7/15
//  last update:  28/7/15

	int w=bb.msbn64();
	if(w==EMPTY_ELEM) return w;					//should not occurr
	
	int col=0; int v=EMPTY_ELEM; int pc=0;
	bb.init_scan(bbo::NON_DESTRUCTIVE);
	while(true){
		v=bb.next_bit();
		if(v==w) break;

		if(!g.get_neighbors(v).is_bit(w))		//filters neighbors vertice of input vertex w
			continue;
		col=max<int>(col, v_col[v]);
		pc++;
	}
	
	//minimum 
	return (min<int>(col, pc)+1);	
}

inline
int InitColorUB::paint_XRN (const bitarray& bb){
///////////////// 
// Determines an infra-chromatic + recoloring bound for vertex set bb
// Assumes color sets have been initialized
	
	int col=1;  int nBB=EMPTY_ELEM; int v=EMPTY_ELEM;
	int pc=(m_unsel=bb).popcn64();
	bool color_used=false;

	bitarray m_forbidden(g.number_of_vertices());			//forbidden colors for inferences
			
	//main loop
	while(1){
		//sets candidates nodes for coloring from unsel
		m_colsets[col]=m_unsel;
		m_colsets[col].init_scan(bbo::NON_DESTRUCTIVE);
		while(1){
next_v:		v=m_colsets[col].next_bit(nBB, m_unsel);
			if(v==EMPTY_ELEM){
				break;
			}
///////////////////////////////////////
// Infrachrom + recoloring filter 
			if( (col>=3) ){

				for(int recol=1; recol<(col-1); recol++){		//loop to find initial color seed
					//filters out forbidden colors
					if(m_forbidden.is_bit(recol)) continue;
					
					//check if color is valid for swapping (0-1 neighbors)	
					int vswap;
					int pc_swap=m_colsets[recol].single_disjoint(g.get_neighbors(v), vswap);
							
					//analysis
					if(pc_swap==1){	//candidate to first inconsistent color subset found
						
						/////////////////////////////////////////////
						//searches for new color set j>recol
						for(int j=recol+1; j<col; j++){
							

							//filters out forbidden colors
							if(m_forbidden.is_bit(j)) continue;

						
							if( m_colsets[j].is_disjoint(g.get_neighbors(vswap)) ){
								
								//updates color subsets
								if((--pc)==0){ return ((color_used)? col : col-1); }
								else{
									//LOG_INFO("RECOLORING FOUND");
									m_colsets[j].set_bit(vswap);
									m_colsets[recol].set_bit(v);
									m_colsets[recol].erase_bit(vswap);
									goto next_v;
								}


							}else if( m_colsets[j].is_disjoint(g.get_neighbors(v),g.get_neighbors(vswap))){

								//updates inconsistent color set with (recol, j)
								if((--pc)==0){ return ((color_used)? col : col-1); }
								else{
									//LOG_INFO("SAT INCONSISTENT SUBSET FOUND");
									m_forbidden.set_bit(j);
									m_forbidden.set_bit(recol);
									goto next_v;
								}
						
							}
						}//end of search for new color set j>recol

						///////////////////////////////////////////
						//searches for new color set j<recol
						for(int j=1; j<recol; j++){
							

							//filters out forbidden colors
							if(m_forbidden.is_bit(j)) continue;

						
							if( m_colsets[j].is_disjoint(g.get_neighbors(vswap)) ){
								
								//updates color subsets
								if((--pc)==0){ return ((color_used)? col : col-1); }
								else{
									//LOG_INFO("RECOLORING FOUND");
									m_colsets[j].set_bit(vswap);
									m_colsets[recol].set_bit(v);
									m_colsets[recol].erase_bit(vswap);
									goto next_v;
								}


							}else if( m_colsets[j].is_disjoint(g.get_neighbors(v),g.get_neighbors(vswap))){

								//updates inconsistent color set with (recol, j)
								if((--pc)==0){ return ((color_used)? col : col-1); }
								else{
									//LOG_INFO("SAT INCONSISTENT SUBSET FOUND");
									m_forbidden.set_bit(j);
									m_forbidden.set_bit(recol);
									goto next_v;
								}
						
							}
						}//end of search for new color set j<recol
					
					}else if(pc_swap==0){ 
						if((--pc)==0){	return ((color_used)? col : col-1);	}
						else{
							//LOG_INFO("SIMPLE RECOLORING FOUND");
							m_colsets[recol].set_bit(v);
							goto next_v;
						}
					}

				}//next first candidate color subset
			}//end of infrachrom+recoloring filter

// End of Infrachrom + recoloring filter
///////////////////////////////////////
		
			if((--pc)==0)
						return col;			//the last vertex is not filtered so there is NO correction of color here on return

			//main coloring computation
			m_colsets[col].erase_block(nBB,g.get_neighbors(v));
			color_used=true;
	
		}//next vertex
		
		col++;
		color_used=false;
		
	}//next color

	return col;		//should not reach here
}

inline
int InitColorUB::paint_R (const bitarray& bb){
/////////////////////
// recoloring extended to all colors (not only up to kmin)

	int col=1;  int nBB=EMPTY_ELEM; int v=EMPTY_ELEM;
	int pc=(m_unsel=bb).popcn64();
	const int NB_OF_BB_NODES_MINUS_ONE=iop.NB_OF_BB_NODES-1;
				
	//main loop
	while(true){
		//sets candidates nodes for coloring from unsel
		for(int i=0; i<iop.NB_OF_BB_NODES; i++){
			iop.m_colSets[col].bb.get_bitboard(i)=m_unsel.get_bitboard(i);
		}
		//iop.m_colSets[col].bb=m_unsel;
		iop.m_colSets[col].bb.init_scan(bbo::NON_DESTRUCTIVE);
		while(true){
next_v:		v=iop.m_colSets[col].bb.next_bit(nBB, m_unsel);
			if(v==EMPTY_ELEM){
				break;
			}
///////////////////////////////////////
// Infrachrom + recoloring filter 
			if( (col>=3) ){

				for(int recol=1; recol<(col-1); recol++){		//loop to find initial color seed
					
					//check if color is valid for swapping (0-1 neighbors)	
					int vswap;
					int pc_swap=iop.m_colSets[recol].bb.single_disjoint(0,NB_OF_BB_NODES_MINUS_ONE,g.get_neighbors(v), vswap);
							
					//analysis
					if(pc_swap==1){	//candidate to first inconsistent color subset found
						
						/////////////////////////////////////////////
						//searches for new color set j>recol
						for(int j=recol+1; j<col; j++){
													
							if( iop.m_colSets[j].bb.is_disjoint(0,NB_OF_BB_NODES_MINUS_ONE,g.get_neighbors(vswap)) ){
								
								iop.m_colSets[j].bb.set_bit(vswap);
								iop.m_colSets[recol].bb.set_bit(v);
								iop.m_colSets[recol].bb.erase_bit(vswap);
								iop.m_colSets[col].bb.erase_bit(v);
								iop.node_iset_no[vswap]=j;
								iop.node_iset_no[v]=recol;
																
								if((--pc)==0){ 
									if(iop.m_colSets[col].bb.is_empty()) return col-1;
									else return col;	
								}
								else goto next_v;
								
							}
						}//end of search for new color set j>recol
					
					}else if(pc_swap==0){
						iop.m_colSets[recol].bb.set_bit(v);
						iop.m_colSets[col].bb.erase_bit(v);
						iop.node_iset_no[v]=recol;

						if((--pc)==0){	
							if(iop.m_colSets[col].bb.is_empty()) return col-1;
							else return col;
						}else	goto next_v;
					}

				}//next first candidate color subset
			}//end of infrachrom+recoloring filter

// End of Infrachrom + recoloring filter
///////////////////////////////////////

			iop.node_iset_no[v]=col;

			if((--pc)==0){
				if(iop.m_colSets[col].bb.is_empty()) return col-1;
				else return col;			
			}

			//main coloring computation
			bitarray& nbor=g.get_neighbors(v);
			for(int i=nBB; i<iop.NB_OF_BB_NODES; i++){
				iop.m_colSets[col].bb.get_bitboard(i)&= ~nbor.get_bitboard(i);
			}
			//iop.m_colSets[col].bb.erase_block(nBB,g.get_neighbors(v));
						
		}//next vertex
		
		col++;
	}//next color

	return col;		//should not reach here
}



inline
int InitColorUB::paint_XRN_last (const bitarray& bb){
/////////////////
// Determines an oriented infra-chromatic + recoloring bound for LAST VERTEX of set 
// RETURNS: UB (0 if array is empty) 

//
// Assumes color sets have been initialized
	int vlast=bb.msbn64();
	if (vlast==EMPTY_ELEM) return 0;					//empty check
	bitarray neigh(g.number_of_vertices());
	AND(g.get_neighbors(vlast), bb,neigh);				//could be optimized to the block of vlast if bb is oriented
	if(neigh.is_empty()) return 1;

	return (paint_XRN(neigh)+1);
}

inline
int InitColorUB::paint_R_last (const bitarray& bb){
/////////////////
// Determines an oriented infra-chromatic + recoloring bound for LAST VERTEX of set 
// RETURNS: UB (0 if array is empty) 
//
// !!!Filtering in any form after maxsatz analysis is INCOMPLETE-possibly might be subsumed by prior reasoning: Build a test!!! (13/12/16)
//
// **** BUG WHEN USING NEW RLF_CONF IMPROVED SORTING- REMOVE FILTER_NON_ENLARGED UNTIL FURTHER NEWS *****
	
	int vlast=bb.msbn64();
	if (vlast==EMPTY_ELEM) return 0;						//empty check
	bitarray neigh(g.number_of_vertices());
	AND(g.get_neighbors(vlast), bb,neigh);					//could be optimized to the block of vlast if bb is oriented
	if(neigh.is_empty()) return 1;

//	neigh.set_bit(vlast);									//attempt: add last vertex (1)-22/8/16
	int col=iop.InfraOpPlus<ugraph, bitarray>::paint_R(neigh);
//	int col=paint_R(neigh);
		
	/*iop.set_color_nb(col);
	iop.set_node_state_active(neigh);
	iop.update_color_sizes(col);*/

	//int inc=0;
	//if(iop.filter()){
	//	LOG_INFO("PREPROC-FILTER-CUT:"<<vlast);
	//	inc++;
	//}

	//int nb_conf=iop.init_maxsatz_init_ub(vlast, 0);			
	int nb_conf=iop.init_maxsatz(-1, false /* incremental */);		//unnecessary to add node v (13/12/96)
	//int nb_conf=iop.init_maxsatz();
	//cin.get();

		
	/*if(!iop.check_consistency_db(true)){
		LOG_ERROR("INCONSISTENT DB");
	}*/
	
	//new filter for non_enlarged sets-BUGGY CURRENTLY-RLF COND IMPROVED FOR SAN  (27/12/16)
	/*iop.add_node_to_new_color(vlast);							 
	if(iop.InfraOpPlus<ugraph, bitarray>::filter_non_enlarged()){
		LOG_ERROR("PREPROC-FILTER-CUT:"<<vlast);
		nb_conf++;
	}*/

	iop.reset_enlarged_isets();

	//if(nb_conf>0){
	//	LOG_INFO("PREPROC-INFRACUT:"<<vlast<<":"<<nb_conf);
	//}

	return col+1-nb_conf/*-inc*/;									//col - nb_conf if (1)
}

inline
int InitColorUB::eval_init_order (){
//////////////////////
// Evalutes current vertex order using paint_XRN_last
// Changed to p-maxsat criteria (paint_R_last(...)) 29/12/16

	int max=0; int temp;

	//initial values for set of subproblems m_sel and color of first vertex
	m_sel.erase_bit();		
	m_sel.set_bit(0);		
	init_color_sets(g.number_of_vertices());					//*** TODO, reduce space

	//main loop iterating over all vertices except {0}
	for(int v=1; v<g.number_of_vertices(); v++){
		m_sel.set_bit(v);
		//temp=paint_XRN_last(m_sel);								//*** sat cannot be -1, not checked
		temp=paint_R_last(m_sel);									
		if(temp>max){
			max=temp;
		}
	}
	return max;

}

#endif 

