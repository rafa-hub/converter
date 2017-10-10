//init_color_sort.h: header for a specialization of InitColor to determine different 
//					 initial color-based orderings (or infra-chrom) for ugraphs
//
//date of creation: 14/10/15
//last update: 14/10/15
//author: pablo san segundo

#ifndef __INIT_COLOR_SORT_H__
#define __INIT_COLOR_SORT_H__

#include "init_color.h"
#include "utils/logger.h"

#include <vector>

using namespace std;

typedef  vector<bitarray*> v_bb;
typedef vector<int> vint;

class CliqueDollPlus;
//class CliqueInfraPlus;

///////////////////////////
//
// InitColorSort class
// (only for ugraph )
//
////////////////////////////
class InitColorSort: public InitColor<ugraph>{
public:
	static const int MAX_SIZE_TAIL_OF_COLORING=3;																								//allows for TAIL_OF_COLORING final color sets of size 1
	
public:
	static vint maxclique		(ugraph& g, const clqo::param_t& p);																			/* maxclique solver applied for RLF coloring*/
	static vint maxclique		(CliqueDollPlus& cg /* CliqueInfraPlus& cg  */ /*ugraph& g, const clqo::param_t& p,*/, bitarray& bbs, bool init=false);					/* maxclique solver for any subgraph */
		
	InitColorSort				(ugraph& g):InitColor<ugraph>(g), m_colsets(NULL){}
	virtual	~InitColorSort		() 	{clear_color_sets();}

public:	
	int greedyIndependentSetColoring			(vint& color, vint& new_ordering);
	vint recursiveLargestFirst					(vint& new_ordering,  bool& is_good, clqo::init_order_t=clqo::MAX_WIDTH);						/* now deprecated in favor of XXX_INC 29/12/16 */
	vint recursiveLargestFirst_INC				(vint& new_ordering,  bool& is_good);	
	vint recursiveLargestFirst_INC	            (vint& new_ord, vint& tail_rlf_dec, bool& is_good);												

private:	
	int init_color_sets(int nCol);					
	void clear_color_sets();

//////////////
// data members
	bitarray* m_colsets;			
};

inline
void InitColorSort::clear_color_sets(){
	if(m_colsets) {
		delete [] m_colsets; 
		m_colsets=NULL;
	}
}


inline
int InitColorSort::init_color_sets(int nCol){
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
	}
	catch(...){
		LOG_ERROR("InitColorSort::init_color_sets failed when assigining memory to color sets");
		return -1;
	}
	return 0;
}

inline
int InitColorSort::greedyIndependentSetColoring	(vint& color, vint& ordering){
///////////////////////
// computes color classes iteratively 
//
// RETURNS by parameters: color labels, sorting by increasing color order
// RETURNS by value: size of the coloring (number of labels)
//
// REMARKS: As usual new order[OLD_INDEX]=NEW_INDEX (indexes 0 based)

	int pc=g.number_of_vertices(), col=1, v=EMPTY_ELEM, from=EMPTY_ELEM;
	color.assign(pc, EMPTY_ELEM);	
	ordering.assign(pc, EMPTY_ELEM); 
	
	int new_index=0;
	m_unsel.set_bit(0,pc-1);
	while(true){
		m_sel=m_unsel;
		m_sel.init_scan(bbo::DESTRUCTIVE);							
		while(true){
			if((v=m_sel.next_bit_del(from, m_unsel))==EMPTY_ELEM) 
				break;
			color[v]=col;
			ordering[v]=new_index++;

			if((--pc)==0)	
				return col;
			
			//computes next vertex of the current color class
			m_sel.erase_block(from, g.get_neighbors(v));
		}

	++col;
	}

return col;		//should not reach here
}



#endif 

