//clique_parallel.h: an OpenMP parallel implementation of BBMCPara based on Clique Class (*** Experimental ***)
//date of creation: 3/11/14
//last update: 23/09/14
//author: alvaro lopez
//
//now deprecated (1/3/2016) TODO: substituted by clique_para.cpp , remove from targets

#ifndef __CLIQUE_PARALLEL_H__
#define __CLIQUE_PARALLEL_H__

#include "graph/graph.h"
#include "graph/kcore.h"
#include "../init_color.h"
#include "graph\algorithms\graph_sort.h"
#include <time.h>				
#include <map>
#include <omp.h>

//Includes obtener HW
#ifdef _MSC_VER
	#include <windows.h>
#elif __GNUC__
	#include <unistd.h>
#endif


///////////////////////////
//
// InitOrder class
// (only for ugraph and sparse_ugraph)
//
////////////////////////////

template <class T=ugraph>
class Clique_parallel{
	struct lista_nodos_t{
		lista_nodos_t():nodos(NULL),index(0){}
		int* nodos;
		int index;
	};

typedef map<int, int> mint_t;
public:
////////////////
// constructors
	Clique_parallel(T& gout):g(gout)	{	
		m_bbsets=NULL;
		m_lsets=NULL;	
		m_path=NULL;										
		m_lcol=NULL;
		maxac=0;
		maxno=0;
		m_alloc=g.max_degree_of_graph();
		m_size=g.number_of_vertices();
		m_numcores=1;
		m_sel=NULL;
		m_unsel=NULL;
	}

////////////////
// setters and getters
	int get_max_clique()	{return maxno;}
	int get_max_cores_hw();
	void set_malloc(int M)	{m_alloc=M;}
	mint_t& get_filter()	{return m_filter;}
	void set_cores(int M)	{m_numcores=M;}

/////////////////////////////////
//init search data structures
	int init_search_basics();
	void clear_search_basics();

	int init_color_labels();
	void clear_color_labels();

	int init_aux_data();
	void clear_aux_data();

//////////////////
//approximate vertex coloring
	inline void paint				(int depth);																	//sequential greedy independent set coloring procedure

//////////////////
// search 
	int set_up_unrolled					();
	void run_unrolled();
	
	void expand								(int depth, typename T::bb_type& l_bb , lista_nodos_t& l_v);			//recursive search procedure
	void initial_expand_deprecated			(int depth, typename T::bb_type& l_bb );								//recursive search procedure
	void initial_expand						();	
	//////////////////
// heuristics
	void filter_heur				(int maxno, typename T::bb_type& l_bb , lista_nodos_t& l_v);			//kcore filter
	void filter_heur_shrink			(int maxno);															//kcore filter

///////////////////
// I/O
	void print_filter				(ostream& o=cout);

private:	
	T& g;															//T restricted to ugraph and sparse_ugraph
	
	typename T::bb_type**	m_bbsets;								//[THREAD][DEPTH][MAX_VERTEX]

	typename T::bb_type*	m_sel;									//[THREAD]
	typename T::bb_type*	m_unsel;								//[THREAD]

	int	***					m_lcol;									//[THREAD][DEPTH][MAX_VERTEX]
	lista_nodos_t**			m_lsets;								//[THREAD][DEPTH]
	
	int**					m_path;									//[THREAD][DEPTH]
	int*					m_path_sol;								//[DEPTH]
	
	mint_t					m_filter;								//[MAXNO]-->[FIRST_VERTEX_PRUNED]

	int maxno;														//size of branch maximal clique
	int maxac;														//[THREAD]size of current best clique found at any moment
	int m_alloc;
	int m_size;
	int m_numcores;													//Num of threads for parallel search
};

///////////////
// notation
#define  LISTA_L(thread,depth)		m_lsets[(thread)][(depth)]		//conventional list of vertices
#define  LISTA_BB(thread,depth)		m_bbsets[(thread)][(depth)]		//list of vertices encoded as a bitstring
#define	 MAXAC_PLUS1				maxac+1
#define  MAXCLQINT					0x1FFFFFFF						//my own MAXINT



template<class T>
void Clique_parallel<T>::clear_search_basics (){
	if(m_bbsets!=NULL){
		for(int j=0; j<m_numcores; j++)
			delete [] m_bbsets[j];  //Llama a destructores de BBN
		delete [] m_bbsets;
	}
	m_bbsets=NULL;

	//lista de nodos
	if(m_lsets!=NULL){
		for(int j=0; j<m_numcores; j++){
			for(int i=0; i<m_alloc; i++){
				if(m_lsets[j][i].nodos!=NULL && m_lsets[j][i].index==0){
					delete [] m_lsets[j][i].nodos;
				}
				m_lsets[j][i].nodos=NULL;
			}
		delete [] m_lsets[j];		//Llama a destructores de STL::Vector
		}
	delete m_lsets;
	}
	m_lsets=NULL;
	
	//path	
	if(m_path!=NULL){
		for(int i=0; i<m_numcores; i++)	
			 delete [] m_path[i];	
	delete [] m_path;  
	}
	m_path=NULL;

	if(m_path_sol!=NULL)
		delete [] m_path_sol;
	m_path_sol=NULL;
}

template<class T>
int Clique_parallel<T>::init_search_basics (){


	m_bbsets=new typename T::bb_type*[m_numcores];
	for(int j=0; j<m_numcores; j++){
		m_bbsets[j]=new typename T::bb_type[m_alloc];
		for(int i=0; i<m_alloc; i++){
			m_bbsets[j][i].init(m_size);			//set_to_0
		}
	}

	//list of nodes
	m_lsets=new lista_nodos_t*[m_numcores];
	for(int i=0; i<m_numcores; i++){
		m_lsets[i]=new lista_nodos_t[m_alloc];
		for(int j=0; j<m_alloc; j++){
			m_lsets[i][j].nodos=new int [m_size];				//index=0 en el constructor
		}
	}
	
			
	//path
	m_path_sol= new int[m_size];
	for(int i=0; i<m_size; i++)
				m_path_sol[i]=EMPTY_ELEM;

	m_path= new int*[m_numcores];
	for(int j=0; j<m_numcores; j++){
		m_path[j]= new int[m_size];
		for(int i=0; i<m_size; i++)
			m_path[j][i]=EMPTY_ELEM;
	}
	return 0;	
}

template<class T>
void Clique_parallel<T>::clear_color_labels(){
	if(m_lcol!=NULL){
		for(int j=0; j<m_numcores; j++){
			for(int i=0; i<m_alloc; i++){	
				delete [] m_lcol[j][i]; 
			}delete [] m_lcol[j];
	}
	delete [] m_lcol;  
		
	}
	m_lcol=NULL;
}

template<class T>
int Clique_parallel<T>::init_color_labels(){
//////////////////
// 	
	clear_color_labels();
	m_lcol=new int** [m_numcores];				
	for(int w=0; w<m_numcores;w++){
		m_lcol[w]=new int* [m_alloc];
		for(int i=0; i<m_alloc;i++){
			m_lcol[w][i]=new int [m_size];					
			for(int j=0; j<m_size; j++){
				#ifdef _WIN32
						m_lcol[w][i][j]=MAXCLQINT;		/*before EMPTY_ELEM but it is better to give a real upper threshold*/
				#else
						m_lcol[w][i][j]=MAXCLQINT;
				#endif
			} 
		}
	}
	return 0;
}

template<class T>
void Clique_parallel<T>::clear_aux_data (){	
	if(m_unsel!=NULL){
		delete [] m_unsel;  //Llama a destructores de BBN
	}
	m_unsel=NULL;

	if(m_sel!=NULL){
		delete [] m_sel;  //Llama a destructores de BBN
	}
	m_sel=NULL;
}

template<class T>
int Clique_parallel<T>::init_aux_data (){
	
	clear_aux_data();

	m_unsel= new typename T::bb_type[m_numcores];
	m_sel= new typename T::bb_type[m_numcores];
	for(int w=0; w<m_numcores;w++){
		m_unsel[w].init(m_size);		
		m_sel[w].init(m_size);
	}

	return 0;
}

template<class T>
int Clique_parallel<T>::get_max_cores_hw (){
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
			cout<<"Hardawre not definded"<<endl;
	#endif

	return max_cores;
}

template<class T>
inline void Clique_parallel<T>::paint (int depth){
///////////////////
// Sequential greedy independent set vertex coloring which prunes the search tree
	int ID=omp_get_thread_num();
	int col=1, kmin=maxno-depth, nBB=EMPTY_ELEM, v=EMPTY_ELEM;		
	LISTA_L(ID,depth).index=EMPTY_ELEM;											//cleans the set fo candidate vertices
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
			//#pragma omp critical (graph)
			{
			m_sel[ID].erase_block(nBB,g.get_neighbors(v));
			}
		}				
	col++;
	}
}

template<class T>
void Clique_parallel<T>::expand(int maxac, typename T::bb_type& l_bb , lista_nodos_t& l_v){
////////////////////////
// recursive search algorithm
	int ID=omp_get_thread_num();
	int v,aux;
	//main loop
	while(l_v.index>=0){
		
		//Estrategias
		v=l_v.nodos[l_v.index--];

		//CUT by color (since [Konc & Janecic, 2007] this is of limited use (only for first branch))
		

		if(m_lcol[ID][maxac][v]+maxac<=maxno){
				return;
		}
/////////////////////////////////
// CHILD NODE GENERATION
		
		//Node generation by masking
		//#pragma omp critical (graph)
		{
			AND(g.get_neighbors(v), l_bb, LISTA_BB(ID,maxac));		//optimized when place second the bitset with higher population
		}
		//bool boolaux=false;
		//Leaf node: updates incumbent if necessary
		

		if( LISTA_BB(ID,maxac).is_empty()){
			//boolaux=true;
			if(maxac>=maxno){
#pragma omp critical (maxno)
{
					maxno=maxac+1;						//NEW GLOBAL OPTIMUM FOUND
					m_path[ID][maxac]=v;
					for(int i=0;i<=maxac;i++){
						m_path_sol[i]=m_path[ID][i];
					}
}

				cout<<"NEW lb:"<<maxno<<" root_v:"<<m_path[ID][0]<<endl;

			}
			l_bb.erase_bit(v);		
			continue;
		
		}
		
		//if(boolaux){
		//	l_bb.erase_bit(v);		
		//	continue;
		//}
		
		
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
		// BACKTRACK (does not delete v fro path since it will be overwritten in the same level of search tree)
		l_bb.erase_bit(v); 
	}// next node

return;
}

template<class T>
void Clique_parallel<T>::run_unrolled(){
////////////////
// runs search unrolling first level
	
	//algorithm
	//clock_t t_ini, t_fin;
	//double secs;

	//t_ini = clock();
	initial_expand(); //Initial expand with OpenMP
	//t_fin = clock();

	//secs = (double)(t_fin - t_ini) / CLOCKS_PER_SEC;

	//cout<<"[w:"<<maxno<<","<<secs<<"s]"<<endl;
}


template<class T>
void Clique_parallel<T>::initial_expand(){
////////////////////
// unrolling of first level

	int v=EMPTY_ELEM;
	KCore<T> kc(g); // Kcore object, shared in OpenMP
	InitColor<T> cinit(g); // Init coloring object, shared in OpenMP
		
	// Set the cores 
	omp_set_num_threads(m_numcores);
	
	//Loop with neighborsets
	//For parallel with setings and data shared
	#pragma omp parallel for schedule(dynamic) firstprivate(kc,cinit) default(none)
	for(v=m_size-1; v>=0; v--){
		int ID=omp_get_thread_num();
		LISTA_BB(ID,0).init_bit(v,g.get_neighbors(v));
		LISTA_BB(ID,0).set_bit(v);								//optimizable					
		

		//CUT related to size
		if(LISTA_BB(ID,0).popcn64()<=maxno){
			//cout<<"PODA SIZE SUBPROBLEMA TAMAÑO"<<ID<<endl;
			continue;
		}
		int aux;
		//COLOR CUT of this subproblem
		//#pragma omp critical (init_color)
		{
			aux=cinit.greedyIndependentSetColoring(LISTA_BB(ID,0));
		}
		if(aux<=maxno){
			//cout<<"PODA COLOR SUBPROBLEMA COLOR"<<ID<<endl;
			continue;
		
		}
		
		//kcore computation
		//#pragma omp critical (Kcore)
		//{
		kc.set_subgraph(&LISTA_BB(ID,0));
		kc.kcore();				
		//KCore cut
		LISTA_L(ID,0).index=-1;
		const vint& kcn=kc.get_kcore_numbers();
		const vint& kcv=kc.get_kcore_ordering();
		
		for(int i=kcv.size()-1; i>=0; i--){
			if(kcn[kcv[i]]<maxno){
				//KCore cut for the subproblem
				for(int j=i; j>=0; j--){
					LISTA_BB(ID,0).erase_bit(kcv[j]);		//O(logn) operation
				}
				break;
			}else{
				//add to candidate list for expansion
				if(kcv[i]!=v){
					LISTA_L(ID,0).nodos[++LISTA_L(ID,0).index]=kcv[i];
					m_lcol[ID][1][kcv[i]]=kcn[kcv[i]]+1;		
				}
			}
		}
		//}
		//Expansion as in BBMC in minimum width order
		
		LISTA_BB(ID,0).erase_bit(v);
		m_path[ID][0]=v;
		expand(1,LISTA_BB(ID,0),LISTA_L(ID,0));
	
// BACKTRACK  from v: vertex already deleted at the beginning of the iterations
	}
}


template<class T>
void Clique_parallel<T>::print_filter (ostream& o){
	for(map<int, int>::iterator it= m_filter.begin(); it!=m_filter.end(); ++it){
		o<<"["<<it->first<<","<<it->second<<"]"<<" "; 
	}
	o<<endl;
}

//clique_parallel_hidden.h: hidden file with core implementation details of clique_parallel
//date:17/11/2014

template<>
inline void Clique_parallel<sparse_ugraph>::paint (int depth){
///////////////////
// Sequential greedy independent set vertex coloring to prune the search tree
	int ID=omp_get_thread_num();
	int col=1, kmin=maxno-depth, nBB=EMPTY_ELEM, v=EMPTY_ELEM;					// color labels start at 1	(col)
	LISTA_L(ID,depth).index=EMPTY_ELEM;											//cleans the set fo candidate vertices
	const int DEPTH_PLUS1=depth+1;
	
	//copies list of vertices to color and stores size for fast empty check 
	int pc=LISTA_BB(ID,depth).popcn64();

	//cut on population 
	if(pc<kmin){
		//cout<<"SIZE CUT-----------------"<<endl;
		return;
	}

	m_unsel[ID]=LISTA_BB(ID,depth);
	while(true){ 
		sparse_bitarray::velem_it it=m_unsel[ID].begin();
		m_sel[ID]=m_unsel[ID];
		m_sel[ID].init_scan(bbo::DESTRUCTIVE);										//no need to check if m_sel is empty (it cannot be)
		while(true){
			v=m_sel[ID].next_bit_del_pos(nBB);
			if(v==EMPTY_ELEM) 
				break;
			it=m_unsel[ID].erase_bit(v,it);											//optimization because vertices selected are in order
			if(col>=kmin){  
				LISTA_L(ID,depth).nodos[++LISTA_L(ID,depth).index]=v;
				m_lcol[ID][DEPTH_PLUS1][v]=col;						
			}
			if((--pc)==0) 
						return;
			m_sel[ID].erase_block_pos(nBB,g.get_neighbors(v));
		}				
	col++;
	}
}


template<class T>
void Clique_parallel<T>::filter_heur(int maxno, typename T::bb_type& l_bb , lista_nodos_t& l_v){
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
void Clique_parallel<T>::filter_heur_shrink	(int maxno){
///////////////////
// shrinks graph

	int vfilter=EMPTY_ELEM;
	if(m_filter.count(maxno)){
		int new_size=m_filter[maxno];
		g.shrink_to_fit(new_size);
		m_size=new_size;
	}
	else return;
}

template<>
inline
int Clique_parallel<sparse_ugraph>::set_up_unrolled(){
/////
// set up for the unrolled case
	
	KCore<sparse_ugraph> kc(g);
    kc.kcore();

	cout<<"determining kcore and kcore filter--------------"<<endl;
	cout<<"kcore ub:"<<kc.get_kcore_number()+1<<endl;

	//determine and set kcore filter for the above ordering
	kc.make_kcore_filter(get_filter(), true);
	print_filter();
	

	//Calculo de lb
	cout<<"determining an initial large clique--------------"<<endl;
	if(maxno>kc.get_kcore_number()){			//allows for an external initial value
		cout<<"[w:"<<maxno<<"]"<<" TRIVIALLY SOLVED DURING PRECOMPUTATION"<<endl;
		return maxno;
	}
    vector<int> v=kc.find_heur_clique_sparse();
	if(maxno<v.size())								//updates current best solution (allows initial value from other sources)
			maxno=v.size();
	if(maxno>kc.get_kcore_number()){
		cout<<"[w:"<<maxno<<"]"<<" TRIVIALLY SOLVED DURING PRECOMPUTATION"<<endl;
		return maxno;
	}
	cout<<"[lb:"<<maxno<<"]"<<endl;
			
	//Orden inicial (MWS-kcore based)
	cout<<"determining initial order-------------"<<endl;
	GraphSort<sparse_ugraph> o(g);	
	const vint& kco=kc.get_kcore_ordering();
	//new order table
	vint old2new(kco.size());
	int l=0;
	for(vint::const_reverse_iterator it=kco.rbegin(); it!=kco.rend(); ++it){
		old2new[*it]=l++;
	}
	cout<<"degeneracy reordering: init in place reordering---------------"<<endl;
	o.reorder_in_place(old2new);
		
	//shrinks graph
	cout<<"shrinkin graph-------------"<<endl;
	filter_heur_shrink(maxno);
	
	//Init over the new graph (shrinked)
	cout<<"allocating memory for search:"<<kc.get_kcore_number()+1<<"-------------"<<endl;
	set_malloc(kc.get_kcore_number()+1);
	init_search_basics();
	init_color_labels();
	init_aux_data();
	
	//initial coloring not necessary at level 0 (will be produced in run())

return 0;
}

template<class T>
int Clique_parallel<T>::set_up_unrolled(){
/////
// set up for the unrolled case (non sparse graphs)
//
// COMMENTS: 
// A- Uses find_heur_clique(15) --15 iterations-- to compute lower bound
// B- Initial order: MIN_WIDTH_MIN_TIE_STATIC 
	
	PrecisionTimer pt;
	KCore<T> kc(g);
	pt.wall_tic();
	cout<<"init kcore analysis----------------"<<endl;
	kc.kcore();
	cout<<"kcore ub:"<<kc.get_kcore_number()+1<<endl;
	cout<<"[t:"<<pt.wall_toc()<<"]"<<endl;

	///Caomputation of lb for middle size graphs (NOT large sparse graphs)
	cout<<"determining an initial large clique--------------"<<endl;
	pt.wall_tic();
	if(maxno>kc.get_kcore_number()){			//allows for an external initial value
		cout<<"[w:"<<maxno<<"]"<<" TRIVIALLY SOLVED DURING PRECOMPUTATION"<<endl;
		return maxno;
	}
	vector<int> v=kc.find_heur_clique();
	cout<<"[t:"<<pt.wall_toc()<<"]"<<endl;

	if(maxno<v.size())								//updates current best solution (allows initial value from other sources)
			maxno=v.size();
	if(maxno>kc.get_kcore_number()){
		cout<<"[w:"<<maxno<<"]"<<" TRIVIALLY SOLVED DURING PRECOMPUTATION"<<endl;
		return maxno;
	}
	cout<<"[lb:"<<maxno<<"]"<<endl;
			
	//Initial ordering for middle size graphs
	cout<<"init degeneracy reordering (macro)----------------"<<endl;
	InitOrder<T> o(g);	
	o.reorder(o.create_new_order(MIN_WIDTH_MIN_TIE_STATIC));
			
		
	//Init search allocation
	cout<<"allocating memory for search:"<<kc.get_kcore_number()+1<<"-------------"<<endl;
	set_malloc(kc.get_kcore_number()+1);
	init_search_basics();
	init_color_labels();
	init_aux_data();
	
	//initial coloring not necessary at level 0 (will be produced in run())
	

return 0;
}

	



#endif