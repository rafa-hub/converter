////////////////////////////////
// clique_enum_fixed.h: interface for CliqueEnumF class for non-sparse graphs of fixed size which 
// solves the problem of maximal clique enumeration. Compared with CliqueEnum, the 
// emphasis is on solving a dataset of graphs quickly (i.e. without memory allocation etc.)
// and not an individual graph. For this purpose, all data structures concerning search are
// allocated prior once at the beginning and cleared when the data set has been processed.
//
// Application: Maximal clique enumeration
// Data Types:  Uses bitarray types for every bitencoded vertex set
//
// Observations: it is outside the current test framework
//				  
// initial date:23/3/16 (in Burgos)
// last update: 
// author: pablo san segundo


#ifndef  __CLIQUE_ENUM_FIXED_H__
#define  __CLIQUE_ENUM_FIXED_H__

#include "clique.h"
#include "graph/simple_fixed_ugraph.h"
#include "../init_color_ub.h"
#include "../amts/amts_exec.h"
#include "utils/common.h"
#include "utils/logger.h"
#include "../common/common_macros.h"

template<int N, int D>
class CliqueEnumF{
	static const int MAX_GRAPH_SIZE=N;
	static const int MAX_CLIQUE_SIZE=D;	//for allocation

	UgraphF<N> g;						//the one and only graph of fixed size	

	int nX;								//X size of current problem
	int nP;								//P size of current problem
	int nG;								//graph size of current problem=nX+NP (<=N)

	//search
	bitarray* m_P;						//[depth] vertices in P set
	bitarray* m_X;						//[depth] vertices in X set
	bitarray* m_L;						//[depth] candidate vertices in P
	int*      m_path;					//alternatively vector<int>
	int		  m_sol;					//number of maximal cliques found
	int		  m_steps;					//number of recursive calls
	
	//auxiliary
	vector<int> map;					//mapping in the new graph
 
public:
	CliqueEnumF			():m_P(NULL), m_X(NULL), m_L(NULL), m_path(NULL), m_sol(0), m_steps(0){}	//does not allocate memory	
virtual ~CliqueEnumF	(){clear();}	
	int init();	
	void clear();
	int set_graph		(sparse_ugraph&, sparse_bitarray& P, sparse_bitarray & X, int nP=-1, int nX=-1);

	/////////////////
// setters and getters
	int number_of_maximal_cliques	(){return m_sol;}
	int number_of_steps				(){return m_steps;}
	
/////////////////
// create_graph (basic operations)
	void add_edge		(int v, int w)	{g.add_edge(v,w);}
	void remove_edge	(int v, int w)	{g.remove_edge(v,w);}
	void empty_graph	()				{g.create_empty_graph();}
		
	//test framework for search
	int set_up();					
	void run(){ expand_greedy_conflict_set(0);	LOG_PRINT("[w:"<<m_sol<<","<<"st:"<<m_steps<<"]");}		
	void tear_down(){}	

private:
	//search procedures
	void expand_greedy_conflict_set	(int depth);
};

template <int N, int D>
void CliqueEnumF<N, D>::clear(){
	if (m_P){
		delete [] m_P;
		m_P=NULL;
	}
	if (m_X){
		delete [] m_X;
		m_X=NULL;
	}
	if (m_L){
		delete [] m_L;
		m_L=NULL;
	}
	if(m_path){
		delete  [] m_path;
		m_path=NULL;
	}
}

template <int N, int D>
int CliqueEnumF<N, D>::init(){
///////////////
// allocates memory for search data structures
//
// OBSERVATIONS
// 1-m_path is not initialized

	clear();
	try{
		m_P=new bitarray[D];
		m_X=new bitarray[D];
		m_L=new bitarray[D];
		m_path=new int[D];
	}catch(exception& e){
		LOG_INFO("CliqueEnumF<N, D>::init-memory could not be allocated-"<<e.what());
		m_P=NULL;  m_X=NULL; m_L=NULL; m_path=NULL;
		return -1;
	}

	//allocate bitstrings;
	for(int i=0; i<D; i++){
		m_P[i].init(N);
		m_X[i].init(N);
		m_L[i].init(N);
	}

	//allocates memory for mapping indexes
	map.assign(N,EMPTY_ELEM);

return 0;
}

template <int N, int D>
inline
int CliqueEnumF<N,D>::set_up(){
///////////////
// Initializes X, P, L sets and other params
// Assumes the graph is correctly set

	m_sol=0; m_steps=0;
		
	//set X, P sets
	m_X[0].erase_bit();
	if(nX>0){
		m_X[0].set_bit(nP,nG-1);		//X are the vertices with highest numbers
	}

	m_P[0].erase_bit();
	m_P[0].set_bit(0,nP-1);

//////////////
//greedy root pivot selection (last vertex)
	//int pivot=nG-1;

////////////////
// Tomita root-pivot selection
	int max_neigh=-1; int neigh; int pivot=EMPTY_ELEM;
	
	//best pivot from X set	
	m_X[0].init_scan(bbo::NON_DESTRUCTIVE_REVERSE);
	while(true){
		int v=m_X[0].previous_bit();
		if (v==EMPTY_ELEM) break;

		neigh=g.degree(v,m_P[0]);
		if(neigh>max_neigh){
			pivot=v;
			max_neigh=neigh;
		}
	}
	
	//best pivot from P set
	m_P[0].init_scan(bbo::NON_DESTRUCTIVE_REVERSE);
	while(true){
		int v=m_P[0].previous_bit();
		if (v==EMPTY_ELEM) break;

		neigh=g.degree(v,m_P[0]);
		if(neigh>max_neigh){
			pivot=v;
			max_neigh=neigh;
		}
	}

////////////////////
// determines candidate vertices based in pivot
	m_L[0]=m_P[0];
	m_L[0].erase_bit(g.get_neighbors(pivot));
	return 0;
}

template <int N, int D>
inline
void CliqueEnumF<N,D>::expand_greedy_conflict_set (int depth){
////////////////////////
// Ordered pivot selection from conflict set if possible 

	const int NEXT=depth+1;

	//main loop
	m_L[depth].init_scan(bbo::NON_DESTRUCTIVE_REVERSE);
	while(true){
		int v=m_L[depth].previous_bit();
		if(v==EMPTY_ELEM) break;

		//child node generation
		AND(g.get_neighbors(v),m_P[depth],m_P[NEXT]);					//optimized when place second the bitset with higher population
		AND(m_X[depth],g.get_neighbors(v),m_X[NEXT]);					//has to be evaluated before the leaf node check

		//leaf node: determines if a new maximal clique has been found
		if(m_P[NEXT].is_empty()){
			if(m_X[NEXT].is_empty()){
				m_sol++;	//sol found
			}
			m_P[depth].erase_bit(v);
			m_X[depth].set_bit(v);
			continue;
		}

		//greedy pivot heuristic: last vertex from conflict set if possible
		/*int pivot=m_X[NEXT].msbn64();
		if(pivot==EMPTY_ELEM){ 
			pivot=m_P[NEXT].msbn64();
		}*/


		//Tomita pivot heuristic
		//max deg X set
		int max_neigh=-1; int neigh; int pivot=EMPTY_ELEM;
		m_X[NEXT].init_scan(bbo::NON_DESTRUCTIVE_REVERSE);
		while(true){
			int v=m_X[NEXT].previous_bit();
			if (v==EMPTY_ELEM) break;

			neigh=g.degree(v,m_P[NEXT]);
			if(neigh>max_neigh){
				pivot=v;
				max_neigh=neigh;
			}
		}

	
		//max deg P set
		m_P[NEXT].init_scan(bbo::NON_DESTRUCTIVE_REVERSE);
		while(true){
			int v=m_P[NEXT].previous_bit();
			if (v==EMPTY_ELEM) break;

			neigh=g.degree(v,m_P[NEXT]);
			if(neigh>max_neigh){
				pivot=v;
				max_neigh=neigh;
			}
		}
		////////////////////////////		


		//compute neighborset based on pivot
		m_L[NEXT]=m_P[NEXT];
		m_L[NEXT].erase_bit(g.get_neighbors(pivot));
		
		//branching
		m_path[depth]=v;	
		m_steps++;
		expand_greedy_conflict_set(depth+1);  

		//backtrack (does not delete v from path since it will be overwritten in the same level of search tree)
		m_P[depth].erase_bit(v); 
		m_X[depth].set_bit(v);

	}//next node
}

template <int N, int D>
inline
int CliqueEnumF<N, D>::set_graph(sparse_ugraph& g_out, sparse_bitarray& bb_P, sparse_bitarray& bb_X, int sizeP, int sizeX){
//////////////////////////
// generates graph G(P+X,E[X->P]+E[P->P]), P comes first
//
// RETURNS: 0 if graph is generated appropiately, -1 if size exceeded

	(sizeP==-1)? nP=bb_P.popcn64() : nP=sizeP;
	(sizeX==-1)? nX=bb_X.popcn64() : nX=sizeX;
	if((nG=nX+nP)>N){
		return -1;
	}
		
	empty_graph();		//TODO-make it lazy (possibly only for nG vertices?)

///////////////////	
//create_graph

	//create mapping (overrites old map)
	//first P
	int l=0;
	bb_P.init_scan(bbo::NON_DESTRUCTIVE);
	while(true){
		int v=bb_P.next_bit();
		if(v==EMPTY_ELEM) break;
		map[l++]=v;		//[new to old]
	}

	//last X 
	bb_X.init_scan(bbo::NON_DESTRUCTIVE);
	while(true){
		int v=bb_X.next_bit();
		if(v==EMPTY_ELEM) break;
		map[l++]=v;	//[new to old]
	}	

	//create graph by iterating over vertices
	//P->P
	for(int v=0; v<nP-1; v++){
		for(int w=v+1; w<nP; w++){
			if(g_out.is_edge(map[v],map[w])){
				g.add_edge(v,w);
			}
		}
	}

	//X->P
	for(int v=nP; v<nG; v++){
		for(int w=0; w<nP; w++){
			if(g_out.is_edge(map[v],map[w])){
				g.add_edge(v,w);
			}
		}
	}

	return 0;	//graph-ok
}

#endif
