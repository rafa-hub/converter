////////////////////////////////
// cover_mwss.h: interface for the COVER class which contains heuristics
//               to cover a set of nodes a la MWSS to determine an UB for MWCP
//
// COMMENTS: currently completely DRAFT! /* Developing */
//
// initial date:04/09/17
// last update: 04/09/17
// author: pablo san segundo

#ifndef  __COVER_MWSS_H__
#define  __COVER_MWSS_H__

#include <algorithm>
#include "../clique.h"
#include "../../../bitscan/bbalg.h"
#include "../../../utils/common.h"
#include "../../../graph/algorithms/graph_map.h"
#include "ub_weighted_clique.h"

using namespace com;												//for common types 
typedef vector<int> vint;

//////////////
// switches

class COVER{
////////////////////////
// computes super-weight analysis
public:
	static const int NO_WEIGHT=CLQ_MAXINT;			/* no interference when MAXIMIZING WEIGHTS */
	static const int NO_DEG=0;
	static const int NONE=CLQ_MAXINT;
private:
	struct less_vsort: public binary_function<int, int, bool>{ 
		bool operator()(int u, int v) const{
			if(d[u]>d[v]) return true;
			else if(d[u]==d[v]){
				if(w[u]<w[v]) return true;
			}
			return false;
		}

		less_vsort(int* d, int* w):d(d), w(w){}
		int *d;
		int *w;
	};

private:
	ugraph* g;								/* reference graph */	
	int m_NV;

	int* deg;								/* degree information */						
	int* vsort;								/* list to sort */
	int** db;								/* [COLOR][NODES] each color set  */
	int* wv;								/* [NODES] weight information */
	int NBCOL;

	bitarray sel;			
	bitarray unsel;			
	bitarray bbnodes;	
	bitarray bbmap;	
public:
	COVER(ugraph* gout=NULL)			:g(gout), m_NV(0), NBCOL(0), deg(NULL), vsort(NULL), db(NULL), wv(NULL) { if(g) set_graph(g);}
	virtual ~COVER()									    {clear();}
	void set_graph(ugraph* g);
	int* get_col(int col)				{return db[col];}		
////////////////
// Interface 
	void initialize	();																		/* sets initial val: MUST BE CALLED*/
	int start  	();																		/* driver */
	int  next_col	(int& ub);																		/* covers first nodein vsort */
	
/////////
//I/O
	ostream& print_db	(ostream& o=cout);
	ostream& print_db	(int col, ostream& o=cout);
	ostream& print_vs	(ostream& o=cout);

private:
	void init();
	void clear();
};

inline
ostream& COVER::print_vs (ostream& o){
	for(int i=0; i<m_NV; i++){
		int v=vsort[i];
		o<<"["<<v<<"|w:"<<wv[v]<<","<<g->get_wv(v)<<"|d:"<<deg[v]<<","<<g->degree(v)<<"]"<<" ";
	}
	return o;
}

inline
ostream& COVER::print_db (ostream& o){
//////////////////////
// paints all colors

	int v=EMPTY_ELEM, POINTER_COL=0;
	for(int c=1; c<=NBCOL; c++){
		POINTER_COL=0;
		o<<endl;
		while((v=db[c][POINTER_COL++])!=NONE){
			o<<"["<<v<<"|w:"<<wv[v]<<","<<g->get_wv(v)<<"|d:"<<deg[v]<<","<<g->degree(v)<<"]"<<" ";
		}
		o<<endl;
	}	
	return o;
}

inline
ostream& COVER::print_db (int c, ostream& o){
//////////////////////
// paints all colors

	int v=EMPTY_ELEM, POINTER_COL=0;
	while((v=db[c][POINTER_COL++])!=NONE){
		o<<"["<<v<<"|weight:"<<wv[v]<<","<<g->get_wv(v)<<"|deg:"<<deg[v]<<","<<g->degree(v)<<"]"<<" ";
	}
	o<<endl;
	return o;
}

inline
int COVER::start(){
//cover nodes in order 
	int nb_col=0, ub=0;
	less_vsort pred(this->deg, this->wv);
	do{
		
		sort(vsort, vsort+m_NV, pred);
	//	print_vs();
	//	cin.get();

		nb_col=next_col(ub);		
	//	print_db(NBCOL); cout<<endl;
		//LOG_INFO("-----------------");

		/* TODO-sort vsort accordingly */
		
		//print_vs(); cout<<endl;
		/*cin.get();*/
		
		

		///print_db(NBCOL);
		//LOG_INFO("NEXT COLOR-PRESS ANY KEY");
		//cin.get();
		
	}while(nb_col);
	print_vs();

	return ub;
}

inline 
int COVER::next_col(int& ub){
////////////////
// computes next col in db fomr subgraph vsol
//
// RETURNS: nb of nodes in the fresh color set
//
// COMMENTS: seed vertex (first vertex in the coloring) is always removed

	int w=EMPTY_ELEM, vs=EMPTY_ELEM, POINTER_COL=0, POINTER_DB=0,wfv=0, counter=0;
	bool neighbor_found=false;
	
	NBCOL++;								/* fresh color */
	wfv= wv[vsort[0]];						/* weight of seed vertedx */
	if(wfv==NO_WEIGHT) return 0;			/*DO NOT FILTER BY DEG==NO_DEG*/
	
	db[NBCOL][0]=NONE;
	for(int i=0; i<m_NV; i++){
		vs=vsort[i];
		if(wv[vs]==NO_WEIGHT) break;			/*DO NOT FILTER BY DEG==NO_DEG*/
		neighbor_found=false;
		POINTER_COL=0; 
		while((w=db[NBCOL][POINTER_COL++])!=NONE){
			//LOG_INFO("ASSESING NEIGHBORS: "<<vs<<","<<w);		
			if(g->is_edge(w,vs)){
				neighbor_found=true;
				break;
			}
		}
		if(!neighbor_found){
		//	LOG_INFO("ADDING TO COLOR: "<<vs);
			db[NBCOL][POINTER_COL-1]=vs;
			db[NBCOL][POINTER_COL]=NONE;
			counter ++;
			
			//assert!
			if(wv[vs]==NO_WEIGHT){
				LOG_ERROR("bizarre weight");
			}

			wv[vs]-=wfv;
			if(wv[vs]<=0){					/* vertex covered: always the case for first vertex of color set */
				wv[vs]=NO_WEIGHT;
				deg[vs]=NO_DEG;		
				//update degrees
				bitarray& bbn=g->get_neighbors(vs);
				bbn.init_scan(bbo::NON_DESTRUCTIVE);
				while(true){
					int vd=bbn.next_bit();
					if(vd==EMPTY_ELEM) break;					
					(deg[vd]!=NO_DEG)? deg[vd]-=1 : 1;
				}							
			}
		}//end if
	}//next vertex
		
	//end value
	ub+=wfv;
	return counter;
}


inline
void COVER::initialize (){
///////////////
// set initial values for the data structures
	
	for(int v=0; v<m_NV; v++){
		deg[v]=g->degree(v);
		vsort[v]=v;
		wv[v]=g->get_wv(v);
	}

	NBCOL=0;
}


inline
void COVER::set_graph(ugraph* g) {
	clear(); 
	if(g==NULL){
		LOG_ERROR("COVER::set_graph()-setting invalid graph");
		return;
	}
	this->g=g;
	m_NV=g->number_of_vertices();
	init();
}

inline
void COVER::clear(){
	//**TODO-clear info of unsel, sel?	
}

inline
void COVER::init(){
	clear();
		
	try{
		unsel.init(m_NV);
		sel.init(m_NV);
		bbnodes.init(m_NV);	
		bbmap.init(m_NV);

		db= new int*[m_NV+1];			/* COLORS 1-based */
		for(int c=0; c<=m_NV; c++){
			db[c]= new int[m_NV];		/* NODES 0-based */
		}		
		for(int c=0; c<=m_NV; c++){
			for(int v=0; v<m_NV; v++){
				db[c][v]=NONE;
			}
		}

		wv= new int[m_NV];
		for(int v=0; v<m_NV; v++){
			wv[v]=NO_WEIGHT;
		}

		deg= new int[m_NV];
		for(int v=0; v<m_NV; v++){
			deg[v]=NO_DEG;
		}

		vsort= new int[m_NV];			
		for(int v=0; v<m_NV; v++){
			vsort[v]=NONE;
		}
	

	}catch(exception &e){
		e.what();
	}
}


#endif


