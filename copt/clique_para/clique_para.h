//clique_parallel.h: an OpenMP parallel implementation of BBMCPara (***experimental***)
//date of creation of preliminary version by Alvaro Lopez: 3/11/14
//date of creation: 13/11/15
//last update:  13/11/15
// author: pss
//


#ifndef __CLIQUE_PARA_H__
#define __CLIQUE_PARA_H__

#include <iostream>
#include <sstream>
#include <map>
#include "../clique/clique_types.h"
#include "../init_color.h"
#include "../clique_sort.h"
#include "graph/graph.h"
#include "graph/kcore.h"
#include "utils/prec_timer.h"	
#include "utils/logger.h"
#include "utils/common.h"	
#include <omp.h>


//depedencies for core number hw checking
#ifdef _MSC_VER
	#include <windows.h>
#elif __GNUC__
	#include <unistd.h>
#endif

using namespace std;


template <class T>
class CliquePara:public CLQParam{
protected:
//an array of nodes (seems to be more efficient than STL vectors in this particular case)
struct nodelist_t{
	nodelist_t():nodos(NULL),index(0){}
	int* nodos;
	int index;
};

typedef map<int, int> mint_t;
typedef void (CliquePara::*func)(int, typename T::bb_type&, nodelist_t&);

public:
	static int get_max_cores_hw();
////////////////
// constructors/destructors
	CliquePara(T* gout, param_t p):g(gout), CLQParam(p){	
		m_bbsets=NULL; m_lsets=NULL; m_path=NULL; m_lcol=NULL; m_colsets=NULL; m_sel=NULL;	m_unsel=NULL;
		maxac=0;maxno=0;
		m_alloc=g->max_degree_of_graph();
		m_size=g->number_of_vertices();
		m_nCores=1;
	}

	CliquePara(param_t p):g(NULL), CLQParam(p){
		m_bbsets=NULL; m_lsets=NULL; m_path=NULL; m_lcol=NULL; m_colsets=NULL; m_sel=NULL;	m_unsel=NULL;
		maxac=0; maxno=0; m_alloc=0; m_size=0;
		m_nCores=1;
	}
	
	virtual ~CliquePara(){clear_all();}
		
////////////////
// setters and getters
virtual	void set_graph(T* g_out){
		clear_all();
		maxac=0;
		maxno=0;
		g=g_out;
		m_alloc=g->max_degree_of_graph();
		m_size=g->number_of_vertices();
	}

	int get_max_clique()		{return res.get_upper_bound();}
	void set_malloc(int M)		{m_alloc=M;}
	void set_initial_lb(int lb)	{get_param().lb=lb;}
	mint_t& get_filter()		{return m_filter;}
	
	//cores
	void set_cores(int M)		{m_nCores=M;}

/////////////////////////////////
//allocation of search data structures
	virtual int init_bitarrays();
	virtual int init_nodelists();
	int init_path();
	virtual int init_color_labels();
	virtual int init_color_sets();
	virtual int init_others();

	virtual void clear_bitarrays();
	virtual void clear_nodelists();
	void clear_path();
	virtual void clear_color_labels();
	virtual void clear_color_sets();
	virtual void clear_others();

	void clear_all(){clear_bitarrays(); clear_nodelists(); clear_path(); clear_color_labels(); clear_color_sets(); clear_others();}

	//allocation drives
	int search_allocation(search_alloc_t info);

	//computation of initial bounds
	vector<int> kcore_bounds(int& lb, int& ub,  KCore<T>* = NULL);
	int initial_bounds(int& lb, int& ub,  KCore<T>* = NULL);

//////////////////
//approximate vertex coloring
	void paint							(int depth);					//independent set SEQ

//////////////////	
// search

	virtual int set_up();
	virtual void tear_down				() {clear_all(); m_alloc=0;}	
	virtual void run					();

///////////////////
// bool
	bool is_clique						(typename T::bb_type& l_bb) const;		
	bool is_clique						( vint& v) const;

//protect in the future
	int set_up_unrolled					(search_alloc_t info);

protected:
	void expand							(int depth, typename T::bb_type& l_bb , nodelist_t& l_v);	//BBMC
	void initial_expand					(func);

//////////////////
// heuristics to reduce the problem's scale
	void filter_heur				(int maxno, typename T::bb_type& l_bb , nodelist_t& l_v);			//kcore filter
	void filter_heur_shrink			(int maxno);														//kcore filter

///////////////////
// I/O
public:
	void print_filter				(ostream& o=cout);

protected:	
	T* g;															//T restricted to ugraph and sparse_ugraph
	
	typename T::bb_type**	m_bbsets;								//[THREAD][DEPTH][MAX_VERTEX]
	
	typename T::bb_type*	m_sel;									//[THREAD]	
	typename T::bb_type*	m_unsel;								//[THREAD]

	int	***					m_lcol;									//[THREAD][DEPTH][MAX_VERTEX]
	nodelist_t**			m_lsets;								//[THREAD][DEPTH]

	int**					m_path;									//[THREAD][DEPTH]
	typename T::bb_type**	m_colsets;								//[THREAD][COLORS], storage of color sets								

	mint_t					m_filter;								//[clique LB]-->[FIRST_VERTEX_PRUNED]

	int maxno;														//size of largest clique found in current branch 
	int maxac;														//size of current best clique found at any moment
	int m_alloc;
	int m_size;
	int m_nCores;													//number of threads
};

///////////////
// notation
#define  LISTA_L(thread,depth)		m_lsets[(thread)][(depth)]			//conventional list of vertices
#define  LISTA_BB(thread,depth)		m_bbsets[(thread)][(depth)]			//list of vertices encoded as a bitstring
#define	 MAXAC_PLUS1				maxac+1

template<class T>
int CliquePara<T>::init_bitarrays(){
////////////////
// arrays of bitstrings
	clear_bitarrays();
	try{
		m_bbsets=new typename T::bb_type*[m_nCores];
		for(int i=0; i<m_nCores; i++){
			m_bbsets[i]=new typename T::bb_type[m_alloc];
			for(int j=0; j<m_alloc; j++){
				m_bbsets[i][j].init(m_size);			//set_to_0
			}
		}
	}catch(exception& e){
		throw;
	}
	
	return 0;
}

template<class T>
void CliquePara<T>::clear_bitarrays(){
	if(m_bbsets!=NULL){
		for(int i=0; i<m_nCores; i++)
			delete [] m_bbsets[i];  
		delete [] m_bbsets;
	}
	
	m_bbsets=NULL;
}

template<class T>
int CliquePara<T>::init_nodelists(){
////////////////////
//arrays of lists of nodes
	clear_nodelists();
	
	try{
		
		m_lsets=new nodelist_t*[m_nCores];
		for(int i=0; i<m_nCores; i++){
			m_lsets[i]=new nodelist_t[m_alloc];
			for(int j=0; j<m_alloc; j++){
				m_lsets[i][j].nodos=new int [m_size];				
			}
		}

	}catch(exception& e){
		throw;
	}
	return 0;
}

template<class T>
void CliquePara<T>::clear_nodelists(){
//////////////////////
// deallocates memory for conventional arrays of vertices at each depth level

	if(m_lsets!=NULL){
		for(int i=0; i<m_nCores; i++){
			for(int j=0; j<m_alloc; j++){
				if(m_lsets[i][j].nodos!=NULL){
					delete [] m_lsets[i][j].nodos;
				}
				m_lsets[i][j].nodos=NULL;
			}
		delete [] m_lsets[i];		
		}
	delete m_lsets;
	}
	m_lsets=NULL;
}

template<class T>
int CliquePara<T>::init_path(){
	clear_path();
	try{
		m_path= new int*[m_nCores];
		for(int i=0; i<m_nCores; i++){
			m_path[i]= new int[m_size];
	
			//initializaton
			for(int j=0; j<m_size; j++){
				m_path[i][j]=EMPTY_ELEM;
			}
		}
		
	}catch(exception& e){
		throw;
	}
	return 0;	
}

template<class T>
void CliquePara<T>::clear_path(){
	if(m_path!=NULL){
		for(int i=0; i<m_nCores; i++)	
			 delete [] m_path[i];	
	delete [] m_path;  
	}
	m_path=NULL;
}

template<class T>
void CliquePara<T>::clear_color_labels(){
	if(m_lcol!=NULL){
		for(int i=0; i<m_nCores; i++){
			for(int j=0; j<m_alloc; j++){	
				delete [] m_lcol[i][j]; 
			}delete [] m_lcol[i];
	}
	delete [] m_lcol;  
		
	}
	m_lcol=NULL;
}

template<class T>
int CliquePara<T>::init_color_labels(){
//////////////////
// 	
	clear_color_labels();
	try{
		m_lcol=new int** [m_nCores];				
		for(int w=0; w<m_nCores;w++){
			m_lcol[w]=new int* [m_alloc];
			for(int i=0; i<m_alloc;i++){
				m_lcol[w][i]=new int [m_size];					
				for(int j=0; j<m_size; j++){
					m_lcol[w][i][j]=CLQ_MAXINT;				
				} 
			}
		}
	}catch(exception& e){
		throw;
	}
	return 0;
}

template<class T>
int CliquePara<T>::init_others(){
//////////////////////
// auxiliary data structures 

	clear_others();
	try{
		m_sel= new typename T::bb_type[m_nCores];		
		for(int i=0; i<m_nCores; i++){
				m_sel[i].init(m_size);
		}

		m_unsel= new typename T::bb_type[m_nCores];		
		for(int i=0; i<m_nCores; i++){
				m_unsel[i].init(m_size);	
		}

	}catch(exception& e){
		throw;
	}

	return 0;
}

template<class T>
void CliquePara<T>::clear_others(){
	if(m_unsel!=NULL){
		delete [] m_unsel;
	}
	m_unsel=NULL;

	if(m_sel!=NULL){
		delete [] m_sel;
	}
	m_sel=NULL;
}


template<class T>
void CliquePara<T>::clear_color_sets (){
////////////////////
// auxiliary color storage (color classes) 
// for pruning inferences

	if(m_colsets!=NULL){
		for(int i=0; i<m_nCores; i++)
			delete [] m_colsets[i];  
		delete [] m_colsets;
	}
	
	m_colsets=NULL;
}

template<class T>
int CliquePara<T>::init_color_sets (){

	clear_color_sets();

	try{
		m_colsets=new typename T::bb_type*[m_nCores];
		for(int i=0; i<m_nCores; i++){
			m_colsets[i]=new typename T::bb_type[m_alloc+1];			//[0] may be used to store the subgraph to color. Actual colors range from [1, N]	
			for(int j=0; j<=m_alloc; j++){
				m_colsets[i][j].init(m_size);			
			}
		}
	}catch(exception& e){
		throw;
	}

	return 0;
}

template<class T>
int CliquePara<T>::search_allocation(search_alloc_t info){
	LOG_PRINT("init search allocation----------------");
	if(info.size>0)
		set_malloc(info.size);								//read from initial configurarion

	try{
		//allocation
		if(init_path()==-1) return -1;							//path is always necessary

		if(info.mem & search_alloc_t::ALLOC_BITARRAYS){
			if(init_bitarrays()==-1) return -1;
		}	

		if(info.mem & search_alloc_t::ALLOC_NODELISTS){
			if(init_nodelists()==-1) return -1;
		}	

		if(info.mem & search_alloc_t::ALLOC_COLOR_LABELS){
			if(init_color_labels()==-1) return -1;
		}

		if(info.mem & search_alloc_t::ALLOC_OTHERS){
			if(init_others()==-1) return -1;
		}

		if(info.mem & search_alloc_t::ALLOC_COLOR_SETS){
			if(init_color_sets()==-1) return -1;
		}
	}catch (exception& e){
		LOG_ERROR(e.what());
		LOG_ERROR("---------------------------------------");
		return -1;
	}


	LOG_PRINT("---------------------------------------");
	return 0;
}

template<class T>
int CliquePara<T>::get_max_cores_hw(){
/////////////////
// Checks HW configuration for maximum cores for different platforms

	unsigned max_cores=1;

	#ifdef _MSC_VER
		   SYSTEM_INFO sysinfo;
		   GetSystemInfo( &sysinfo );
		   max_cores = sysinfo.dwNumberOfProcessors;
	#elif __GNUC__
	        char var[]=  "OMP_NUM_THREADS" ;
                char* var_ = getenv(var);
                max_cores = atoi(var_);
	#else
		LOG_INFO("Hardware unknown: assumes 1 core");
	#endif

	return max_cores;
}


template<class T>
vector<int>  CliquePara<T>::kcore_bounds(int& lb, int& ub,  KCore<T>* kc_out){
////////////////////
// kc_out should be NULL in practice except for the sparse case

	lb=0; ub=0;
	vector<int> vs;

	if(kc_out==NULL) {

		KCore<T> kc(*g);
		
		LOG_PRINT("------init kcore bounds-----------");
		kc.kcore();
		ub=kc.get_kcore_number()+1;

		vs=kc.find_heur_clique();
		lb=vs.size();
		return vs;

	}else{
				

		LOG_PRINT("------init kcore bounds-----------");
		kc_out->kcore();
		ub=kc_out->get_kcore_number()+1;

		vs=kc_out->find_heur_clique();
		lb=vs.size();
		
	}
	
	LOG_PRINT("---------------------------------------");
	return vs;
	
}

template<>
inline
vector<int>  CliquePara<sparse_ugraph>::kcore_bounds(int& lb, int& ub, KCore<sparse_ugraph>* kc_out){
///////////////
//specialization for the sparse case: updates filters
	//KCore<sparse_ugraph> kc(*g);
	lb=0; ub=0;
	
	LOG_PRINT("------init kcore bounds-----------");
	kc_out->kcore();
	ub=kc_out->get_kcore_number()+1;

	kc_out->make_kcore_filter(get_filter(), true);
	print_filter();								

	vector<int> vs=kc_out->find_heur_clique_sparse();
	lb=vs.size();

	LOG_PRINT("------------------------ªª---------------");
	return vs;
}

template<class T>
int CliquePara<T>::initial_bounds(int& lb, int& ub,  KCore<T>* kc_out){
	int lb_kc, ub_kc;
	int sol=0;
	vector<int> vset=kcore_bounds(lb_kc, ub_kc, kc_out); 
	lb=max(lb, lb_kc);
	ub=min(ub, ub_kc);							//ub=0 always returns ub_kc
	sol=CLQParam::is_trivial_sol(lb, ub);
	res.add_solution(vset);						//just in case it cannot be improved!

return sol;
}

template<class T>
void CliquePara<T>::paint (int depth){
///////////////////
// Sequential greedy independent set vertex coloring which prunes the search tree

	int ID=omp_get_thread_num();
	int col=1; int kmin=maxno-depth; int nBB=EMPTY_ELEM; int v=EMPTY_ELEM;		
	LISTA_L(ID,depth).index=EMPTY_ELEM;												//cleans the set of candidate vertices
	const int DEPTH_PLUS1=depth+1;

	//copies list of vertices to color and stores size for fast empty check 
	int pc= (m_unsel[ID]=LISTA_BB(ID,depth)).popcn64();

	//CUT based on population size
	if(pc<kmin){
		return;
	}

	while(true){ 
		m_sel[ID]=m_unsel[ID];
		m_sel[ID].init_scan(bbo::DESTRUCTIVE);
		while(true){
			v=m_sel[ID].next_bit_del(nBB,m_unsel[ID]);
			if(v==EMPTY_ELEM)
				break;
			if(col>=kmin){  
				LISTA_L(ID,depth).nodos[++LISTA_L(ID,depth).index]=v;
				m_lcol[ID][DEPTH_PLUS1][v]=col;				//labels start at 1	
			}
			if((--pc)==0)
				return;
			m_sel[ID].erase_block(nBB,g->get_neighbors(v));
		}				
		col++;
	}
}


template<class T>
void CliquePara<T>::expand(int maxac, typename T::bb_type& l_bb , nodelist_t& l_v){
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
		if( (m_lcol[ID][maxac][v]+maxac)<=maxno )
				return;

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
		maxno=maxac+1;						

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
		paint(maxac);

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
		expand(maxac+1,LISTA_BB(ID,maxac),LISTA_L(ID,maxac));  

		//////////////////////////////////////////////
		// BACKTRACK (does not delete v from path since it will be overwritten in the same level of search tree)
		l_bb.erase_bit(v); 
	}// next node
}

template<class T>
inline
int CliquePara<T>::set_up(){
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
	search_alloc_t info;			//loads BBMC configuration for search allcoation
	switch(param.alg){
	case BBMC:
		break;
	//case BBMC_T:
	//case BBMCR:
	//	info.set(search_alloc_t::ALLOC_COLOR_SETS);
	//	break;
	//case BBMCL:
	//	info.remove(search_alloc_t::ALLOC_COLOR_LABELS);		//no color labels or sets
	//	break;
	//case BBMCL_R:
	//	info.set(search_alloc_t::ALLOC_COLOR_SETS);			
	//	break;
	//case BBMCL_T:
	//	info.remove(search_alloc_t::ALLOC_COLOR_LABELS);		//no color labels or sets
	//	info.set(search_alloc_t::ALLOC_COLOR_SETS);				//requires auxiliary color sets for classical SEQ
	//	break;
	default:
		LOG_ERROR("set_up: unknown algorithm");
		return -1;
	}


	//actual set_up
	int sol=0;
	if(param.unrolled){
		if((sol=set_up_unrolled(info))>0){ 
			res.set_UB(sol);
		}
	}else{
		LOG_ERROR("CliquePara::set_up()-non_unrolled cannot be set to TRUE in multicore execution");
		return -1;
	}	
	
	if(sol>0){
		LOG_INFO("[w="<<sol<<"]"<<" TRIVIALLY SOLVED DURING PRECOMPUTATION");
	}

	return sol;
}

template<class T>
void CliquePara<T>::run(){
	//algorithm
	if(param.unrolled){
		res.tic();

		//algorithm selection
		switch(param.alg){
		case BBMC:
			initial_expand(&CliquePara<T>::expand);
			break;
		default:
			LOG_ERROR("run-unrolled:unknown clique algorithm");
		}


		res.toc();
	}else{
		LOG_ERROR("CliquePara::run()-non_unrolled cannot be set to TRUE in multicore execution");
	}

	//read solution
	res.set_UB(maxno);
	LOG_INFO("[w:"<<res.get_upper_bound()<<","<<res.get_user_time()<<"s]");
}



template<class T>
void CliquePara<T>::initial_expand(func f){
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
	#pragma omp parallel for schedule(dynamic)  default(shared) //shared(cout) //firstprivate(o/*,cinit*/)
	for(int v=m_size-1; v>=maxno; v--){
		int ID=omp_get_thread_num(); 
		LISTA_BB(ID,0).init_bit(v,g->get_neighbors(v));

					
		//CUT related to size: possibly remove?
		int pc=LISTA_BB(ID,0).popcn64();
		if(pc>=maxno){
			
			//order LISTA_BB by root ordering strategy and simple initial coloring
			LISTA_L(ID,0).index=EMPTY_ELEM;
			//vint new_ord=o.create_new_order(param.init_order, LISTA_BB(ID,0), (param.init_order==NONE)? PLACE_FL : PLACE_LF);
			////if(new_ord.empty()) return;
			int gdeg=g->max_degree_of_subgraph(LISTA_BB(ID,0));
			//for(int i=0; i<new_ord.size(); i++){
			//	LISTA_L(ID,0).nodos[++LISTA_L(ID,0).index]=new_ord[i];
			//	m_lcol[ID][1][new_ord[i]]=(i<=gdeg)? i+1 : gdeg+1;					//simple initial coloring
			//}

			LISTA_BB(ID,0).init_scan(bbo::NON_DESTRUCTIVE);
			int i=0;
			while(true){
				int w=LISTA_BB(ID,0).next_bit();
				if(w==EMPTY_ELEM) break;
				LISTA_L(ID,0).nodos[++LISTA_L(ID,0).index]=w;
				m_lcol[ID][1][w]=(i<=gdeg)? ++i : gdeg+1;
				//m_lcol[ID][1][w]=++i;
			}

			//Search
			m_path[ID][0]=v;
#pragma omp critical
{
#ifdef ROOT_VERTEX_PROGRESS
		cout<<"ID:"<<ID<<" root vertex: "<<v<<" w:"<<maxno<<endl;
#endif
}
			(this->*f)(1,LISTA_BB(ID,0),LISTA_L(ID,0));
		
		}
	}
}

template<>
inline void CliquePara<sparse_ugraph>::initial_expand(func f){
////////////////////
// unrolling of first level

//	int v=EMPTY_ELEM;
//	KCore<sparse_ugraph> kc(*g);
//	InitColor<sparse_ugraph> cinit(*g);
//		
//	//Loop over neighbor set subproblems
//	for(int v=m_size-1; v>=0; v--){
//		LISTA_BB(0).init_bit(v,g->get_neighbors(v));
//				
//		//CUT related to size
//		if(LISTA_BB(0).popcn64()<maxno){
//			continue;
//		}
//
//		//COLOR CUT of this subproblem
//		if(cinit.greedyIndependentSetColoring(LISTA_BB(0))<maxno){
//		//	cout<<"PODA COLOR SUBPROBLEMA:"<<col<<endl;
//			continue;
//		}
//				
//		//kcore computation
//		LISTA_BB(0).set_bit(v);								//add bit for kcore analysis only		
//		kc.set_subgraph(&LISTA_BB(0));
//		kc.kcore(); 
//
//		//kcore graph number cut
//		if(kc.get_kcore_number()<maxno){
//			//	cout<<"PODA KCORE GRAPH:"<<col<<endl;
//			continue;
//		}
//		
//		//KCore cut
//		LISTA_L(0).index=-1;
//		const vint& kcn=kc.get_kcore_numbers();
//		const vint& kcv=kc.get_kcore_ordering();
//		for(int i=kcv.size()-1; i>=0; i--){
//			if(kcn[kcv[i]]<maxno){
//				//KCore cut for the subproblem
//				for(int j=i; j>=0; j--){
//					LISTA_BB(0).erase_bit(kcv[j]);		//O(logn) operation
//				}
//				break;
//			}else{
//				//add to candidate list for expansion
//				if(kcv[i]!=v){
//					LISTA_L(0).nodos[++LISTA_L(0).index]=kcv[i];
//					m_lcol[1][kcv[i]]=kcn[kcv[i]]+1;		
//				}
//			}
//		}
//	
////Expansion as in BBMC in minimum width order
//		
//		LISTA_BB(0).erase_bit(v);
//		m_path[0]=v;
//		(this->*f)(1,LISTA_BB(0),LISTA_L(0));		//Note: LISTA_L should not be empty: it would have been detected in KCORE-GRAPH CUT
//	
//// BACKTRACK  from v: vertex already deleted at the beginning of the iterations
//	}
}

template<class T>
void CliquePara<T>::print_filter (ostream& o){
	for(map<int, int>::iterator it= m_filter.begin(); it!=m_filter.end(); ++it){
		o<<"["<<it->first<<","<<it->second<<"]"<<" "; 
	}
	o<<endl;
}


template <class T>
int CliquePara<T>::set_up_unrolled(search_alloc_t info){
///////
// set up for the general purpose unrolled case (non sparse graphs, i.e. DIMACS)
// NOTES:
// Similar to the non_unrolled case but without coloring the root graph

	LOG_PRINT("INIT SETUP UNROLLED");
	int ub=param.ub; 
	int lb=param.lb;
	
	//initial ordering
	LOG_PRINT("init ordering----------");
	CliqueSort<T> o(*g);	
	if(o.reorder(o.new_order(param.init_order, (param.init_order==NONE)? gbbs::PLACE_FL : gbbs::PLACE_LF), get_decoder())==-1){
		LOG_ERROR("set_up_unrolled: error during reordering");
		return -1;
	}
	LOG_PRINT("------------------------------------");

	//initial bounds
	int sol;
	if((sol=initial_bounds(lb, ub))>0){
		return sol;
	}
	LOG_INFO("w:("<<lb<<","<<ub<<")");	


	//init search memory allocation
	info.size=ub;
	if(search_allocation(info)==-1) return -1;

	LOG_PRINT("END OF SETUP UNROLLED");

	//updates initial solution for search
	maxno=lb;
	res.set_LB(lb);

	return 0;
}

template<class T>
bool CliquePara<T>::is_clique (typename T::bb_type& bb) const {
////////////////
// TRUE if bitstring bb clique (empty clique is not a clique)
//
// REMARKS: bb should have capacity for the number of vertices of the graph
	
	typename T::bb_type neighbor(g->number_of_vertices());
	if(bb.init_scan(bbo::NON_DESTRUCTIVE)==EMPTY_ELEM) return false;	//empty cliqe
	while(1){
		int v=bb.next_bit();
		if (v==EMPTY_ELEM) break;

		//check neighborhood						***experimental***
		AND(g->get_neighbors(v), bb, neighbor);
		neighbor.set_bit(v);
		if(!(bb==neighbor)) 
					return false; 
						
	}
return true;		//is clique
}

template<class T>
bool CliquePara<T>::is_clique (vint & v) const {
////////////////
// TRUE if bitstring bb clique (empty clique is not a clique)
//
	if (v.empty()) return false;
	
	//copies the set of vertices to a bitstring with capacity for all vertices
	typename T::bb_type bb(g->number_of_vertices());
	for(vint::iterator it=v.begin(); it!=v.end(); it++){
		 bb.set_bit(*it);
	}
	

return is_clique(bb);   		
}



template<>
inline void CliquePara<sparse_ugraph>::paint (int depth){
///////////////////
// Sequential greedy independent set vertex coloring to prune the search tree

	//int col=1, kmin=maxno-depth, nBB=EMPTY_ELEM, v=EMPTY_ELEM;					// color labels start at 1	(col)
	//LISTA_L(depth).index=EMPTY_ELEM;											//cleans the set fo candidate vertices
	//const int DEPTH_PLUS1=depth+1;
	//
	////copies list of vertices to color and stores size for fast empty check 
	//int pc=LISTA_BB(depth).popcn64();

	////cut on population 
	//if(pc<kmin){
	//	LOG_DEBUG("SIZE CUT-----------------");
	//	return;
	//}

	//m_unsel=LISTA_BB(depth);
	//while(true){ 
	//	sparse_bitarray::velem_it it=m_unsel.begin();
	//	m_sel=m_unsel;
	//	m_sel.init_scan(bbo::DESTRUCTIVE);										//no need to check if m_sel is empty (it cannot be)
	//	while(true){
	//		v=m_sel.next_bit_del_pos(nBB);
	//		//v=m_sel.next_bit_del(nBB);
	//		if(v==EMPTY_ELEM) 
	//			break;
	//		it=m_unsel.erase_bit(v,it);											//optimization because vertices selected are in order
	//		if(col>=kmin){  
	//			LISTA_L(depth).nodos[++LISTA_L(depth).index]=v;
	//			m_lcol[DEPTH_PLUS1][v]=col;						
	//		}
	//		if((--pc)==0) 
	//					return;
	//		//m_sel.erase_block(nBB,g.get_neighbors(v));
	//		m_sel.erase_block_pos(nBB,g->get_neighbors(v));
	//	}				
	//col++;
	//}
}

template<class T>
void CliquePara<T>::filter_heur(int maxno, typename T::bb_type& l_bb , nodelist_t& l_v){
///////////////////////
// KCore filter (experimental)
	
	int vfilter=EMPTY_ELEM;
	if(m_filter.count(maxno))
			 vfilter=m_filter[maxno];
	else return;

	//remove all vertices from bitstring list of vertices
	//l_bb.erase_bit(vfilter, m_size);
	l_bb.clear_bit(vfilter, EMPTY_ELEM);
	
	//update conventional list of vertices
	int v=EMPTY_ELEM, i=0;
	if(l_bb.init_scan(bbo::NON_DESTRUCTIVE)!=EMPTY_ELEM){
		while(true){
			v=l_bb.next_bit();
			if(v==EMPTY_ELEM) break;
			l_v.nodos[i++]=v;
		}
	}

	//sets index of last vertex in the list
	l_v.index=i-1;
}

template<class T>
void CliquePara<T>::filter_heur_shrink	(int maxno){
///////////////////
// shrinks graph
		
	int level=maxno;
	do{
		if(m_filter.count(level)){
			int new_size=m_filter[level];				//and not: int new_size=m_filter[maxno]-1; which is not complete
			g->shrink_to_fit(new_size);
			m_size=new_size;
			LOG_DEBUG("new size of graph: "<<new_size);
			return;
		}
		level--;
	}while(level>1);
	
	return;
}

template<>
inline
int CliquePara<sparse_ugraph>::set_up_unrolled(search_alloc_t info){
/////////////////
// set up for the unrolled case (specialization for sparse graphs which is the typical application)
//
// Comments: Compared with standard (non_unrolled) variant:
// 1-Uses initial kcore filter to shrink the graph as much as possible
// 2-uses initial lower bound heuristic optimized for large sparse graphs
		
	PrecisionTimer pt;
	int ub=param.ub; 
	int lb=param.lb;
	LOG_PRINT("INIT SETUP SPARSE UNROLLED--WILL SHRINK INITIAL GRAPH");

	//initial kcore bounds and kcore filter
	//pt.wall_tic();
	KCore<sparse_ugraph> kc(*g);
	int ub_kc, lb_kc, sol;
	if( (sol=initial_bounds(lb, ub, &kc)) >0){
		return sol;
	}
	LOG_INFO("w:("<<lb<<","<<ub<<")");
	//cout<<"[t:"<<pt.wall_toc()<<"]"<<endl;
	
	//Initial order (MWS-kcore based)
	LOG_PRINT("init degeneracy reordering----------------");
	CliqueSort<sparse_ugraph> o(*g);	
	const vint& kco=kc.get_kcore_ordering();
	vint old2new(kco.size());
	int l=0;
	for(vint::const_reverse_iterator it=kco.rbegin(); it!=kco.rend(); ++it){
		old2new[*it]=l++;
	}
		
	LOG_DEBUG("degeneracy reordering: init in place reordering---------------");
	o.reorder_in_place(old2new);	//efficient in space requirements, used for very large sparse graphs
		
	//shrinks graph
	LOG_DEBUG("init graph reduction----------------");
	filter_heur_shrink(lb);
		
	//Init over the new graph (shrinked)
	LOG_PRINT("init search allocation----------------");
	info.size=ub;
	if(search_allocation(info)==-1) return -1;

	LOG_PRINT("END OF SETUP UNROLLED");

	//update search parameters
	maxno=lb;
	res.set_LB(lb);

return 0;
}

#endif
