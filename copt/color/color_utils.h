//color_utils.h: header for general color types for undirected graphs 
//
//date of creation: 14/01/16
//last update: 14/01/16
//author: pablo san segundo


#ifndef __COLOR_UTILS_H__
#define __COLOR_UTILS_H__

#include "../clique/clique_types.h"
#include "graph/graph.h"
#include <iterator>
#include <algorithm>
#include <deque>

#define MAX_NUM_COLORS 1000			
typedef vector<int> vint;
typedef vector<int>::iterator vint_it;

struct color_bitarray_t{
	int size;																//updated population count
	bitarray col; 

	color_bitarray_t(int MAX_SIZE):size(0){col.init(MAX_SIZE);}					//all bits FALSE
	color_bitarray_t():size(-1){}												//no allocation
	void init(int MAX_SIZE){col.init(MAX_SIZE); size=0;}
	int pop(){if(size>0) {int v=col.lsbn64(); col.erase_bit(v); size--;} else return EMPTY_ELEM;}
	void push(int v) {col.set_bit(v); size++;}
	void empty(){col.erase_bit(); size=0;}
	void update_size(){size=col.popcn64();}
	bool is_empty(){return (size==0);}
	int get_size(){return size;}
};

///////////////////////////
//
// ColorMat class (wrapper for matrix of colors)
// (only for ugraphs)
//
////////////////////////////
template<class T>
class ColorMat{
	const int MAX_COL;						
	const int MAX_VER;

public:
//constructors, setters and getters
	ColorMat								(ugraph& gout, int MAXCOL, int MAXV):g(gout), MAX_COL(MAXCOL), MAX_VER(MAXV), m_col(NULL) {init_matrix();}
virtual ~ColorMat							(){clear_matrix();}
	
	T& get_col(int col)						{return m_col[col];}
	const T& get_col(int col)	const		{return m_col[col];}	
	int	 get_size(int col)		const		{return m_col[col].size;}
	vint& get_color_list()					{return lcol;}
	void set(int col, int v)				{m_col[col].push(v);}
			
//Stack operations
	void push(int col, int v)				{m_col[col].push(v);}
	int  pop(int col)						{return m_col[col].pop();}						//lowest vertex in the set (removes)
	
//YES/NO
	bool is_col(int col, int v)	const		{return  m_col[col].col.is_bit(v);}
	int color(int v);																		//finds first open color for v / *** TODO-not tested
	int color(bitarray& lv);																//*** TODO-not tested
		
//Sorting
	int init_color_list();																	//makes	list of non-empty colors sorted by size			
	void sort_color_list();																	//sorts list of colors by non-decreasing size

//One color reduction (sat-based)
	bool one_color_filter(int nCol);														//***experimental

private:
	void init_matrix();											
	void clear_matrix();

	//int greedyColorMatrixColoring				(vint&);									//acceleration attempt based on color matrix, experimental		

protected:	
	ugraph& g;									
	T* m_col;	
	vint lcol;																				//color_list
};

///////////////
// sorting functors
template<class T>
struct less_size{
	less_size(ColorMat<T>* matrix):mc(matrix){}
	bool operator()(int col1, int col2) const{
		return (mc->get_size(col1)<mc->get_size(col2));
	}
	const ColorMat<T>* mc;
};
////////////////

template<class T>
void ColorMat<T>::clear_matrix(){
	if(m_col){
		delete [] m_col;
		m_col=NULL;
	}
}

template<class T>
void ColorMat<T>::init_matrix(){
	clear_matrix();
	m_col=new T[MAX_COL];
	for(int c=0; c<MAX_COL; c++){
		m_col[c].init(g.number_of_vertices());
	}
}

template<class T>
int ColorMat<T>::init_color_list(){
////////////////
// generates a list of non-empty color labels
// returns the number of colors of the coloring
	lcol.clear();
	for(int i=0; i<MAX_COL; i++){
		if(get_size(i)>0)
			lcol.push_back(i);
	}
return lcol.size();
}	

template<class T>
void  ColorMat<T>::sort_color_list(){
//sort by non decreasing color size
	sort(lcol.begin(),lcol.end(), less_size<T>(this)); 
 }

template<class T>
bool one_color_filter(int nCol){
/////////////////
// attempts to reduce by one the size of the coloring a la incMaxCLQ
// PARAMS: nCol is the size of the coloring

//**EXPERIMENTAL: ASSUMES COLOR SIZES ARE UPDATED INITIALLY, which can be achieved in constant time in SEQ***

	const int MAX_COLOR_SIZE=4;
	vector<int> lc(g.number_of_vertices(),EMPTY_ELEM);
	
	//determine colors in reasonable order
	int lsize=0;
	for(int col=nCol-1; col>=0; col--){
		if(m_col[col].get_size()<=MAX_COLOR_SIZE){
			lc[l++]=col;
		}
	}

	//main loop
	for(int i=0; i<lsize; i++){
		int col=lv[l];
		for(int c1=0; c1<nCol; c1++){
			if(c1==col) continue;
			int new_size=filter(col, c1):
			if(new_size<=MAX_COLOR_SIZE){
				if(new_size==0) return true;
				lv[lsize++]=c1;
			}
		}
	}

	return false;
}

template<class T>
int filter(int from, int to){
///////////////
//	erases vertices in color 'to' according to 'from'
//  RETURNS: size of the 'to' coloring
			
	T bb(g.number_of_vertices());

	//compute all neighbors
	m_col[from].init_scan(bbo::NON_DESTRUCTIVE);
	while(true){
		int v=bb.next_bit();
		if(v==EMPTY_ELEM) break;
		
		bb|=g.get_neighbors(v);
		return 0;
	}

	//main operation
	m_col[to].col&=bb;
	m_col[to].update_size();

return m_col[to].get_size();
}



template<class T>
int color(int v){
///////////////////////
// finds first open color for v
// RETURNS color_label
	
	for(vint_it=lcol.begin(); it!=lcol.end(); lcol++){
		int col=*it;
		if(g.get_neighbors(v).is_disjoint(g.get_neighbors(col))){		//*** TODO optimize sequential coloring with block ranges
			set(col, v);
			return col;
		}
	}

	//new color: next possible label from color list
	int new_col=lcol.size()+1;					//*** TODO- 0 color-label problem
	lcol.push_back(new_col);
	set(new_col, v);
	return new_col;
}

template<class T>
int color(bitarray& lv){
///////////////////////
// colors a list of vertices (i.e. induced subgraph)

	lv.init_scan(bbo::NON_DESTRUCTIVE);
	while(true){
		int v=lv.next_bit();
		if(v==EMPTY_ELEM) break;
		
		color(v);
	}

return lcol.size();
}


//template<class T>
//int ColorMat<T>::greedyColorMatrixColoring(vint& color){
//////////////////////
//// New implementation of Sequential Coloring using a color matrix to optimize speed
//// in a bit-parallel framework [Komosko,L 2014]
////
//// OBSERVATIONS: Requires previous call to init_ColorMatrix()
////
//// Original idea: Larisa Komosko (LATNA), described in the paper 
//// [A fast greedy sequential heuristic for the vertex colouring problem based on bitwise operations, 2014]
////	(currently under review)
//
//// date:21/12/14
//
//    int NV=g.number_of_vertices(), cmax=1, v=EMPTY_ELEM, from=EMPTY_ELEM;
//    color.assign(NV, EMPTY_ELEM);
//
//	//initializes first bit of first color of color_matrix to 0 so that the first vertex
//	//is assigned color 1
//	m_color_matrix[cmax].erase_bit(0);
//	
//	//main nested loop: outer->vertices, inner->possible color assignments
//	for(int v=0; v<NV; v++){
//		for(int c=1; c<=cmax; c++){
//			if(!m_color_matrix[c].is_bit(v)){		//checks if color is available
//				color[v]=c;
//				m_color_matrix[c].set_block(WDIV(v),g.get_neighbors(v));					//OR operation
//				goto next_vertex;
//			}
//		}
//
//		//new color
//		cmax++;
//		color[v]=cmax;
//		m_color_matrix[cmax].copy_from_block(WDIV(v),g.get_neighbors(v));					//overwrite operation	
//		
//next_vertex:
//			;
//	}
//  
//   return cmax;
//
//
//   //Alternative implementation: Alvaro Lopez
//   //for(int v=0; v<NV; v++){
//   //for(int c=1; c<MAX_NUM_COLORS; c++){
//   //	if(!m_color_matrix[c].is_bit(v)){
//   //		color[v]=c;									//1-based
//   //		//selective update (useful if the same color matrix is used for different colorings)
//   //		if(c>col_size){						
//   //			col_size=c;
//   //			m_color_matrix[c].copy_from_block(WDIV(v),g.get_neighbors(v));		
//   //			//colorMatrix[c].set_block(WDIV(v),g.get_neighbors(v));
//   //			//colorMatrix[c]=g.get_neighbors(v);			// falta una funcion void init (int first block, const BitBoardN& );
//   //		}
//   //		else
//   //			m_color_matrix[c].set_block(WDIV(v),g.get_neighbors(v));
//   //		break;
//   //	}
//   //}
//   //}
//
//}



#endif