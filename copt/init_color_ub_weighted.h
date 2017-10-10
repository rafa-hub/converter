//init_color_ub_weighted.h: header for a specialization InitColorUBW for InitColor to determine initial UBs for vertices
//							in weighted graphs
//
//date of creation: 10/10/16
//last update: 10/10/16
//author: pss

#ifndef __INIT_COLOR_UB_WEIGHTED_H__
#define __INIT_COLOR_UB_WEIGHTED_H__

#include "init_color.h"
#include "graph/graph.h"
#include "utils/logger.h"
#include "utils/common.h"
#include "bitscan/bbalg.h"
#include <vector>
#include <algorithm>
#include "clique/infra_tools.h"

using namespace std;

typedef  vector<bitarray* > v_bb;
typedef vector<int> vint;

///////////////////////////
//
// InitColorUBW class
// (only for ugraph )
//
////////////////////////////
class InitColorUBW: public InitColor<ugraph>{
public:
	InitColorUBW			(ugraph& g):InitColor<ugraph>(g) /*, m_colsets(NULL) */{}
	virtual	~InitColorUBW	(){/*clear_color_sets();*/}

	int Compute_UB_last		(int []);			//driver to compute a tighter UB in all vertices, based on UB of last vertex in incremental subgraphs
		
private:
	int incUB				(bitarray& bb, const int []);		//oriented UB based on neighborhood
	int paint				(const bitarray& bb);				//simple color bound generalized to weights
	int paint_last			(const bitarray& bb);				//simple color bound generalized to weights

//	int paint_XRN			(const bitarray& bb);				//oriented UB based on infra-coloring
//	int paint_XRN_last		(const bitarray& bb);				//oriented UB based on infra-coloring of neighbor oriented subproblem (tighter than paint_XRN)	
			
	//int init_color_sets		(int nCol /* bound on number of colors */);					
	//void clear_color_sets	();

/////////
//data members
	//bitarray* m_colsets;											//for (my) infra-chrom coloring
};

//inline
//void InitColorUBW::clear_color_sets(){
//	if(m_colsets) {
//		delete [] m_colsets; 
//		m_colsets=NULL;
//	}
//	m_colsets=NULL;
//}
//
//inline
//int InitColorUBW::init_color_sets(int nCol){
////////////////
//// Assigns space for nCol+1 bitarrays (bitarray 0 will not be used)
//// RETURNS 0 if succesful, -1 otherwise
//	clear_color_sets();
//	
//	try{
//		m_colsets= new bitarray[nCol+1];		//collor 0 will not be used
//		int nV=g.number_of_vertices();
//		for(int i=0; i<nCol+1; i++)		
//				m_colsets[i].init(nV);
//	}
//	catch(...){
//		LOG_ERROR("InitColorUBW::init_color_sets failed when assigining memory to color sets");
//		return -1;
//	}
//	return 0;
//}

inline
int InitColorUBW::Compute_UB_last (int v_col[]){
	int inc, col/*, sat*/;

	//initial values for set of subproblems m_sel and color of first vertex
	m_sel.erase_bit();		
	m_sel.set_bit(0);		
	v_col[0]=g.get_wv(0);					
	//init_color_sets(g.number_of_vertices());				

	//main loop iterating over all vertices except {0}
	const int NV=g.number_of_vertices();
	for(int v=1; v<NV; v++){
		m_sel.set_bit(v);
		inc=incUB(m_sel,v_col); 
	//	cout<<"iteration vertex :"<<v<<endl;
		col=paint_last(m_sel);
	//	sat=paint_XRN_last(m_sel);								//*** sat cannot be -1, not checked
		v_col[v]=min<int>(inc, col);
			
		
		////monitoring
		//stringstrem sstr("computing UB: ");
		//sstr<<v; 
		//LOG_INFO(sstr.str()); 
	}
	return 0;
}

inline
int InitColorUBW::incUB(bitarray& bb, const int v_col []){
///////////////
//  Considers positional neihgbors to determine an incremental upper bound (extension to weighted case)
//	RETURNS: w(v)+min(max bound in N(v), sum of weights of N(v)) where v is 
//			 the last vertex in the set 
//  (N(v): neighbor set of preceding vertices)
//
//  Observations: The return value is an oriented upper bound for set bb (because it is a bound for the last
//				  vertex of the set)
// 
//  initial date: 10/10/16
//  last update:  10/10/16

	int w=bb.msbn64();
	if(w==EMPTY_ELEM){
		LOG_ERROR("incUB:bizarre empty set found");
		return EMPTY_ELEM;							
	}
	
	//incremental bound: maximum bound of 
	int inc=0; int v=EMPTY_ELEM; int pcw=0;
	bb.init_scan(bbo::NON_DESTRUCTIVE);
	while(true){
		v=bb.next_bit();
		if(v==w) break;

		if(!g.get_neighbors(v).is_bit(w))		//filters neighbors vertices of input vertex w
			continue;
		inc=max<int>(inc, v_col[v]);
		pcw+=g.get_wv(v);
	}
	
	//adds weights
	return (min<int>(inc, pcw)+g.get_wv(w));	
}

//inline
//int InitColorUBW::paint_XRN (const bitarray& bb){
/////////////////// 
//// Determines an infra-chromatic + recoloring bound for vertex set bb
//// Assumes color sets have been initialized
//	
//	int col=1;  int nBB=EMPTY_ELEM; int v=EMPTY_ELEM;
//	int pc=(m_unsel=bb).popcn64();
//	bool color_used=false;
//
//	bitarray m_forbidden(g.number_of_vertices());			//forbidden colors for inferences
//			
//	//main loop
//	while(1){
//		//sets candidates nodes for coloring from unsel
//		m_colsets[col]=m_unsel;
//		m_colsets[col].init_scan(bbo::NON_DESTRUCTIVE);
//		while(1){
//next_v:		v=m_colsets[col].next_bit(nBB, m_unsel);
//			if(v==EMPTY_ELEM){
//				break;
//			}
/////////////////////////////////////////
//// Infrachrom + recoloring filter 
//			if( (col>=3) ){
//
//				for(int recol=1; recol<(col-1); recol++){		//loop to find initial color seed
//					//filters out forbidden colors
//					if(m_forbidden.is_bit(recol)) continue;
//					
//					//check if color is valid for swapping (0-1 neighbors)	
//					int vswap;
//					int pc_swap=m_colsets[recol].single_disjoint(g.get_neighbors(v), vswap);
//							
//					//analysis
//					if(pc_swap==1){	//candidate to first inconsistent color subset found
//						
//						/////////////////////////////////////////////
//						//searches for new color set j>recol
//						for(int j=recol+1; j<col; j++){
//							
//
//							//filters out forbidden colors
//							if(m_forbidden.is_bit(j)) continue;
//
//						
//							if( m_colsets[j].is_disjoint(g.get_neighbors(vswap)) ){
//								
//								//updates color subsets
//								if((--pc)==0){ return ((color_used)? col : col-1); }
//								else{
//									//LOG_INFO("RECOLORING FOUND");
//									m_colsets[j].set_bit(vswap);
//									m_colsets[recol].set_bit(v);
//									m_colsets[recol].erase_bit(vswap);
//									goto next_v;
//								}
//
//
//							}else if( m_colsets[j].is_disjoint(g.get_neighbors(v),g.get_neighbors(vswap))){
//
//								//updates inconsistent color set with (recol, j)
//								if((--pc)==0){ return ((color_used)? col : col-1); }
//								else{
//									//LOG_INFO("SAT INCONSISTENT SUBSET FOUND");
//									m_forbidden.set_bit(j);
//									m_forbidden.set_bit(recol);
//									goto next_v;
//								}
//						
//							}
//						}//end of search for new color set j>recol
//
//						///////////////////////////////////////////
//						//searches for new color set j<recol
//						for(int j=1; j<recol; j++){
//							
//
//							//filters out forbidden colors
//							if(m_forbidden.is_bit(j)) continue;
//
//						
//							if( m_colsets[j].is_disjoint(g.get_neighbors(vswap)) ){
//								
//								//updates color subsets
//								if((--pc)==0){ return ((color_used)? col : col-1); }
//								else{
//									//LOG_INFO("RECOLORING FOUND");
//									m_colsets[j].set_bit(vswap);
//									m_colsets[recol].set_bit(v);
//									m_colsets[recol].erase_bit(vswap);
//									goto next_v;
//								}
//
//
//							}else if( m_colsets[j].is_disjoint(g.get_neighbors(v),g.get_neighbors(vswap))){
//
//								//updates inconsistent color set with (recol, j)
//								if((--pc)==0){ return ((color_used)? col : col-1); }
//								else{
//									//LOG_INFO("SAT INCONSISTENT SUBSET FOUND");
//									m_forbidden.set_bit(j);
//									m_forbidden.set_bit(recol);
//									goto next_v;
//								}
//						
//							}
//						}//end of search for new color set j<recol
//					
//					}else if(pc_swap==0){ 
//						if((--pc)==0){	return ((color_used)? col : col-1);	}
//						else{
//							//LOG_INFO("SIMPLE RECOLORING FOUND");
//							m_colsets[recol].set_bit(v);
//							goto next_v;
//						}
//					}
//
//				}//next first candidate color subset
//			}//end of infrachrom+recoloring filter
//
//// End of Infrachrom + recoloring filter
/////////////////////////////////////////
//		
//			if((--pc)==0)
//						return col;			//the last vertex is not filtered so there is NO correction of color here on return
//
//			//main coloring computation
//			m_colsets[col].erase_block(nBB,g.get_neighbors(v));
//			color_used=true;
//	
//		}//next vertex
//		
//		col++;
//		color_used=false;
//		
//	}//next color
//
//	return col;		//should not reach here
//}
//
//inline
//int InitColorUBW::paint_XRN_last (const bitarray& bb){
///////////////////
//// Determines an oriented infra-chromatic + recoloring bound for LAST VERTEX of set 
//// RETURNS: UB (0 if array is empty) 
//
////
//// Assumes color sets have been initialized
//	int vlast=bb.msbn64();
//	if (vlast==EMPTY_ELEM) return 0;					//empty check
//	bitarray neigh(g.number_of_vertices());
//	AND(g.get_neighbors(vlast), bb,neigh);				//could be optimized to the block of vlast if bb is oriented
//	if(neigh.is_empty()) return 1;
//
//	return (paint_XRN(neigh)+1);
//}

inline
int InitColorUBW::paint_last (const bitarray& bb){
/////////////////
// Determines a bound for the LAST VERTEX of set bb, 
// so an oriented bound for set bb
//
// RETURNS: UB (-1 if array is empty) 
	
	int vl=bb.msbn64();
	if(vl==EMPTY_ELEM){
		LOG_ERROR("paint_last():bizarre empty set found");
		return EMPTY_ELEM;							
	}
	
	int wvl=g.get_wv(vl);
	bitarray neigh(g.number_of_vertices());
	AND(g.get_neighbors(vl), bb, neigh);				
	if(neigh.is_empty()) 
				return wvl;								//no neighbors, returns the weight

	return paint(neigh)+wvl;
}

inline
int InitColorUBW::paint (const bitarray& bb){
//////////////////////
// classical coloring extended to weights (each independent color set 
// contributes with the node with highest weight).
//
// RETURNS UB or 0 in case an error occurs
// 
// REMARKS:
// 1.The bitset order MUST be according to non-increasing weights
//   so that the FIRST node is always the one with maximum weight

	const int NV=g.number_of_vertices();
	bitarray m_unsel(bb); 
	bitarray bb_sel(NV);										//*** TODO: allocate once outside function
	int nBB=EMPTY_ELEM, v=EMPTY_ELEM, pc=m_unsel.popcn64();		
	
	if(pc==0) return 0;											//should not occur

	//main loop
	int ub=0;
	//bool is_first_vertex;
	int maxwcol;
	while(true){ 
		bb_sel=m_unsel;
		bb_sel.init_scan(bbo::DESTRUCTIVE);
		//is_first_vertex=true;
		maxwcol=0;
		while(true){
			v=bb_sel.next_bit_del(nBB,m_unsel);
			if(v==EMPTY_ELEM)
				break;

			int wv=g.get_wv(v);
			if(maxwcol<wv){
					maxwcol=wv;		
			}

			//only update the bound for the first vertex in the coloring
			/*if(is_first_vertex){
				ub+=g.get_wv(v);
				is_first_vertex=false;
			}*/

			if((--pc)==0)
					return ub+maxwcol;

			//the actual coloring mask
			bb_sel.erase_block(nBB,g.get_neighbors(v));
		}	
		ub+=maxwcol;
	}
	
	LOG_ERROR("InitColorUBW::paint(..): bizarre coloring");
	return 0;	//should not occurr
}



#endif

