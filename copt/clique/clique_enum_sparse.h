////////////////////////////////
// clique_enum_sparse.h: interface for CliqueEnumSparse class which solves the problem of maximal clique enumeration for large and massive
//						 sparse graphs
//				  
// initial date:17/12/15
// last update: 
// author: pablo san segundo


#ifndef  __CLIQUE_ENUM_SPARSE_H__
#define  __CLIQUE_ENUM_SPARSE_H__

#include "clique.h"
#include "clique_enum.h"
#include "clique_enum_fixed.h"
#include "../init_color_ub.h"
#include "../amts/amts_exec.h"
#include "utils/common.h"

#include "../common/common_macros.h"

//////////////
//SWITCHES
//#define KCORE_UB_INITIAL_SORTING	0x01							//switch for kcore_UB initial sorting
#define STRONG_ROOT_PIVOT			0x02							//Tomita root pivot

//auxiliary graph
#define AUX_GRAPH_SIZE				400							
#define AUX_GRAPH_DIAMETER			401								//limits memory allocation in depth					
#define	MIN_PROBLEM_SIZE			25								//minimum size of problem to be solved with auxiliary graph (45 is reasonable)
//////////////

class CliqueEnumSparse: public Clique<sparse_ugraph>{
typedef void (CliqueEnumSparse::*func)(int, sparse_bitarray&, sparse_bitarray&);
	static const int N=AUX_GRAPH_SIZE;
	static const int D=AUX_GRAPH_DIAMETER;
public:
	CliqueEnumSparse(sparse_ugraph* g, param_t p):	Clique<sparse_ugraph>(g, p), m_cand(NULL), m_conf(NULL){ /*root_vertex_block=EMPTY_ELEM;*/};
	CliqueEnumSparse(param_t p)				:		Clique<sparse_ugraph>(p), m_cand(NULL), m_conf(NULL){/*root_vertex_block=EMPTY_ELEM;*/};
	virtual ~CliqueEnumSparse(){}
	
	virtual int init_bitarrays();	

	//search procedures
	void expand_greedy_conflict_set			(int maxac, sparse_bitarray& l_bb , sparse_bitarray& l_cand);
	void expand_greedy_conflict_set_XP		(int maxac, sparse_bitarray& l_bb , sparse_bitarray& l_cand);
	void expand_greedy_conflict_set_X		(int maxac, sparse_bitarray& l_bb , sparse_bitarray& l_cand);

	void expand_greedy_conflict_set_heur	(int maxac, sparse_bitarray& l_bb , sparse_bitarray& l_cand);
	void expand_greedy_cand_set				(int maxac, sparse_bitarray& l_bb , sparse_bitarray& l_cand);
	void initial_expand						(func f);
	void initial_expand_sel					(func f);			//uses pivot heuristic for unrolling

	//graph-based search procedures
	void initial_expand_graph				(func f);			//auxiliary graph, not really working for sparse graphs
	
	//test framework
	virtual	int set_up();
	virtual	void run();	
	virtual	void tear_down(){ clear_vertex_sets(); clear_conflicts(); Clique<sparse_ugraph>::tear_down();}

private:
	void init_vertex_sets();
	void clear_vertex_sets();
	void init_conflicts();
	void clear_conflicts();

///////////
// data members
	
	//auxiliary data structures
	sparse_bitarray* m_cand;					//candidate vertices
	sparse_bitarray* m_conf;					//conflicting vertices
	//int root_vertex_block;
	CliqueEnumF<N,D> m_clq;						//no memory allocation here

};

void CliqueEnumSparse::clear_vertex_sets(){
	if (m_cand){
		delete [] m_cand;
		m_cand=NULL;
	}
}

void CliqueEnumSparse::init_vertex_sets(){
	clear_vertex_sets();
	m_cand=new sparse_bitarray[m_size];
	for(int i=0; i<m_size; i++){
		m_cand[i].init(m_size);
	}
}

void CliqueEnumSparse::clear_conflicts(){
	if (m_conf){
		delete [] m_conf;
		m_conf=NULL;
	}
}

void CliqueEnumSparse::init_conflicts(){
	clear_conflicts();
	m_conf=new sparse_bitarray[m_size+1];		//for complete graphs
	for(int i=0; i<=m_size; i++){
		m_conf[i].init(m_size);
	}
}

int CliqueEnumSparse::init_bitarrays(){

	Clique<sparse_ugraph>::init_bitarrays();
		
	//empty set
	init_vertex_sets();	
	init_conflicts();	
	
	//not used in real graphs (unrolling=TRUE)
	int pivot=m_size-1;	
	m_cand[0].set_bit(0, pivot);						
	m_cand[0].erase_bit(g->get_neighbors(pivot));	

	return 0;
}

inline
int CliqueEnumSparse::set_up(){
//////////////
// allocates memory, evaluates initial bounds and determines 
// initial trivial solutions
//
// RETURN VALUE: -1 Error, 0-ok, >0 trivial solution found

	int sol=0;
	res.clear();
	res.set_name(g->get_name());
	
	//determine allocation info
	search_alloc_t info;								//loads BBMC configuration for search allocation
	info.remove(search_alloc_t::ALLOC_COLOR_SETS);		//for recoloring
	info.remove(search_alloc_t::ALLOC_COLOR_LABELS);
	switch(param.alg){
	case BBMC_EN_GCAND:
	case BBMC_EN_GCONF:
	case BBMC_EN_GCONF_X:
	case BBMC_EN_GCONF_XP:
	case BBMC_EN_GCONF_INIT:
	case BBMC_EN_GCONF_HEUR:
		
	break;
	case BBMC_EN_GCONF_GRAPH:
	case BBMC_EN_GCONF_GRAPH_X:
	case BBMC_EN_GCONF_GRAPH_XP:
		m_clq.init();									//memory allocation for auxiliary graph
	break;
	default:
		LOG_ERROR("CliqueEnumSparse::setup unknown algorithm");
		return -1;
	}
	
	//actual set_up
	if(param.unrolled){
		//use kcore: reverse k-core ordering and allocation
		KCore<sparse_ugraph> kc(*g);
		kc.kcore();
		int ub=kc.get_kcore_number()+1;

//specific kcore_UB bound		
#ifdef KCORE_UB_INITIAL_SORTING
		kc.kcore_UB(ub-1);	
#endif

		//Initial order (MWS-kcore based)
		LOG_PRINT("init degeneracy reordering----------------");
		CliqueSort<sparse_ugraph> o(*g);	
		const vint& kco=kc.get_kcore_ordering();
		vint old2new(kco.size());
		int l=0;
		
#ifdef KCORE_UB_INITIAL_SORTING

		//sorting according to kcore_UB (degeneracy last-to-first)
		for(vint::const_iterator it=kco.begin(); it!=kco.end(); ++it){
			old2new[*it]=l++;
		}
#else 
		//sorting according to kcore (default for enumeration)
		//vertices are sorted by non_decreasing kcore but, since they are picked from first to last
		//they are not taken according to degeneracy
		/*for(vint::const_iterator it=kco.begin(); it!=kco.end(); ++it){
			old2new[*it]=l++;
		}*/

		//sorting reverse order of kcore (default for clique)
		//vertices taken by degeneracy last-to-first	
		for(vint::const_reverse_iterator it=kco.rbegin(); it!=kco.rend(); ++it){
			old2new[*it]=l++;
		}

	/*	cout<<"KCORE ORDERING"<<endl;
		com::stl::print_collection(old2new);
		cout<<"------------"<<endl;*/

#endif

		LOG_DEBUG("degeneracy reordering: init in place reordering---------------");
		//efficient in space requirements, used for very large sparse graphs
		if(o.reorder_in_place(old2new)==-1){	
			LOG_ERROR("CliqueEnumSparse::set_up unrolled: error during reordering");
			return -1;

		}
		LOG_PRINT("END init degeneracy reordering----------------");

		//reduced memory allocation
		info.size=ub;
	
	}else{

		//set_up_non_unrolled (manual)
		LOG_PRINT("initial ordering----------------");
		CliqueSort<sparse_ugraph> o(*g);
		if(o.reorder(o.new_order(param.init_order, (param.init_order==NONE)? gbbs::PLACE_FL : gbbs::PLACE_LF), get_decoder())==-1){
			LOG_ERROR("CliqueEnumSparse::set_up non unrolled: error during reordering");
			return -1;
		}
	
		//standard allocation, unsuitable for real graphs
		info.size=m_size;
	}	

	//allocates memory
	if(search_allocation(info)==-1) return -1;
	res.set_UB(0);	//will store the number of maximal cliques found
	
	return sol;
}

inline
void CliqueEnumSparse::run(){
	//algorithm
	if(param.unrolled){
		res.tic();
		switch(param.alg){
		case BBMC_EN_GCAND:
			initial_expand(&CliqueEnumSparse::expand_greedy_cand_set);
			break;
		case BBMC_EN_GCONF:
			initial_expand(&CliqueEnumSparse::expand_greedy_conflict_set);
			break;
		case BBMC_EN_GCONF_GRAPH:
			initial_expand_graph(&CliqueEnumSparse::expand_greedy_conflict_set);
			break;
		case BBMC_EN_GCONF_GRAPH_X:
			initial_expand_graph(&CliqueEnumSparse::expand_greedy_conflict_set_X);
			break;
		case BBMC_EN_GCONF_GRAPH_XP:
			initial_expand_graph(&CliqueEnumSparse::expand_greedy_conflict_set_XP);
			break;
		case BBMC_EN_GCONF_X:
			initial_expand(&CliqueEnumSparse::expand_greedy_conflict_set_X);
			break;
		case BBMC_EN_GCONF_XP:
			initial_expand(&CliqueEnumSparse::expand_greedy_conflict_set_XP);
			break;

		case BBMC_EN_GCONF_INIT:
			initial_expand_sel(&CliqueEnumSparse::expand_greedy_conflict_set);
			break;
		case BBMC_EN_GCONF_HEUR:
			initial_expand_sel(&CliqueEnumSparse::expand_greedy_conflict_set_heur);
			break;
			
		default:
			LOG_ERROR("CliqueEnumSparse::run-unrolled:unknown clique algorithm");
		}

		res.toc();	
	
	}else{			//non-unrolled
		res.tic();
		switch(param.alg){
			case BBMC_EN_GCAND:
			expand_greedy_cand_set(0, Clique<sparse_ugraph>::m_bbroot, m_cand[0]);
			break;
		case BBMC_EN_GCONF:
			expand_greedy_conflict_set(0, Clique<sparse_ugraph>::m_bbroot, m_cand[0]);
			break;
		
		default:
			LOG_ERROR("CliqueEnumSparse::run-non_unrolled:unknown clique algorithm");
		}

		res.toc();
	}

	//read solution
	LOG_INFO("[w:"<<res.get_upper_bound()<<","<<"t:"<<res.get_user_time()<<"s,"<<"st:"<<res.number_of_steps()<<"]");
}	

inline
void CliqueEnumSparse::expand_greedy_conflict_set (int maxac, sparse_bitarray& l_bb , sparse_bitarray& l_cand){
////////////////////////
// Ordered pivot selection from conflict set if possible 

	const int MAXACPLUS1=maxac+1;
				
	//main loop
	if(l_cand.init_scan(bbo::NON_DESTRUCTIVE_REVERSE)!=EMPTY_ELEM){
		while(true){

			//select pivot (in order)
			int v=l_cand.previous_bit();
			if(v==EMPTY_ELEM) break;

#ifdef ROOT_VERTEX_PROGRESS
			if(maxac==0)
				cout<<"root vertex: "<<v<<endl;
#endif
			//child node generation by masking
			AND(l_bb, g->get_neighbors(v), LISTA_BB(maxac));					//optimized when placed second the bitset with higher population
			AND(m_conf[maxac], g->get_neighbors(v), m_conf[MAXACPLUS1]);		//has to be evaluated before the leaf node check

			//Leaf node: updates incumbent if necessary
			if( LISTA_BB(maxac).is_empty()){
				if(m_conf[MAXACPLUS1].is_empty()){

					res.set_UB(res.get_upper_bound()+1);						//increments solution
#ifdef VIEW_PROGRESS
					m_path[maxac]=v;
					stringstream sstr("");
					for(int i=0; i<=maxac; i++){
						sstr<<m_path[i]<<" ";
					}
					LOG_INFO(sstr.str());
#endif
				}

				l_bb.erase_bit(v);
				m_conf[maxac].set_bit(v);
				continue;
			}

					
			//greedy new pivot heuristic: last vertex from conflict set (if possible)
			int pivot=m_conf[MAXACPLUS1].msbn64();
			if(pivot==EMPTY_ELEM){
				pivot=LISTA_BB(maxac).msbn64();
			}
			ERASE(LISTA_BB(maxac), g->get_neighbors(pivot), m_cand[MAXACPLUS1]);
	
			///////////////////////////////////////////////////////
			// Branching
			m_path[maxac]=v;			
			res.inc_number_of_steps();
			expand_greedy_conflict_set(maxac+1,LISTA_BB(maxac),m_cand[MAXACPLUS1]);  

			//////////////////////////////////////////////
			// Backtrack (does not delete v from path since it will be overwritten in the same level of search tree)
			l_bb.erase_bit(v); 
			m_conf[maxac].set_bit(v);

		}// next node
	}

return;			
}


void CliqueEnumSparse::expand_greedy_conflict_set_X (int maxac, sparse_bitarray& l_bb , sparse_bitarray& l_cand){
////////////////////////
// Ordered pivot selection from conflict set if possible 

	const int MAXACPLUS1=maxac+1;
				
	//main loop
	if(l_cand.init_scan(bbo::NON_DESTRUCTIVE_REVERSE)!=EMPTY_ELEM){
		while(true){

			//select pivot (in order)
			int v=l_cand.previous_bit();
			if(v==EMPTY_ELEM) break;

#ifdef ROOT_VERTEX_PROGRESS
			if(maxac==0)
				cout<<"root vertex: "<<v<<endl;
#endif
			//child node generation by masking
			AND(l_bb, g->get_neighbors(v), LISTA_BB(maxac));					//optimized when place second the bitset with higher population
			AND(m_conf[maxac], g->get_neighbors(v), m_conf[MAXACPLUS1]);		//has to be evaluated before the leaf node check

			//Leaf node: updates incumbent if necessary
			if( LISTA_BB(maxac).is_empty()){
				if(m_conf[MAXACPLUS1].is_empty()){

					res.set_UB(res.get_upper_bound()+1);						//increments solution
#ifdef VIEW_PROGRESS
					m_path[maxac]=v;
					stringstream sstr("");
					for(int i=0; i<=maxac; i++){
						sstr<<m_path[i]<<" ";
					}
					LOG_INFO(sstr.str());
#endif
				}

				l_bb.erase_bit(v);
				m_conf[maxac].set_bit(v);
				continue;
			}

			//max deg X set
			int max_neigh=-1; int neigh; int pivot=EMPTY_ELEM;
			if(m_conf[MAXACPLUS1].init_scan(bbo::NON_DESTRUCTIVE_REVERSE)!=EMPTY_ELEM){
				while(true){
					int v=m_conf[MAXACPLUS1].previous_bit();
					if (v==EMPTY_ELEM) break;

					neigh=g->degree(v,LISTA_BB(maxac));
					if(neigh>max_neigh){
						pivot=v;
						max_neigh=neigh;
					}
				}
			}

			//if conflict set is empty, take the first vertex in P as greedy pivot heuristic: last vertex from conflict set (if possible)
			if(pivot==EMPTY_ELEM){
				pivot=LISTA_BB(maxac).msbn64();
			}
			ERASE(LISTA_BB(maxac), g->get_neighbors(pivot), m_cand[MAXACPLUS1]);
	
			///////////////////////////////////////////////////////
			// Branching
			m_path[maxac]=v;			
			res.inc_number_of_steps();
			expand_greedy_conflict_set_X(maxac+1,LISTA_BB(maxac),m_cand[MAXACPLUS1]);  

			//////////////////////////////////////////////
			// Backtrack (does not delete v from path since it will be overwritten in the same level of search tree)
			l_bb.erase_bit(v); 
			m_conf[maxac].set_bit(v);

		}// next node
	}

return;			
}

inline
void CliqueEnumSparse::expand_greedy_conflict_set_XP (int maxac, sparse_bitarray& l_bb , sparse_bitarray& l_cand){
////////////////////////
// Tomita-based pivot selection (largest degree in P from X+P)

	const int MAXACPLUS1=maxac+1;
				
	//main loop
	if(l_cand.init_scan(bbo::NON_DESTRUCTIVE_REVERSE)!=EMPTY_ELEM){
		while(true){

			//select pivot (in order)
			int v=l_cand.previous_bit();
			if(v==EMPTY_ELEM) break;

#ifdef ROOT_VERTEX_PROGRESS
			if(maxac==0)
				cout<<"root vertex: "<<v<<endl;
#endif
			//child node generation by masking
			AND(l_bb, g->get_neighbors(v), LISTA_BB(maxac));					//optimized when place second the bitset with higher population
			AND(m_conf[maxac], g->get_neighbors(v), m_conf[MAXACPLUS1]);		//has to be evaluated before the leaf node check

			//Leaf node: updates incumbent if necessary
			if( LISTA_BB(maxac).is_empty()){
				if(m_conf[MAXACPLUS1].is_empty()){

					res.set_UB(res.get_upper_bound()+1);						//increments solution
#ifdef VIEW_PROGRESS
					m_path[maxac]=v;
					stringstream sstr("");
					for(int i=0; i<=maxac; i++){
						sstr<<m_path[i]<<" ";
					}
					LOG_INFO(sstr.str());
#endif
				}

				l_bb.erase_bit(v);
				m_conf[maxac].set_bit(v);
				continue;
			}

			//max deg X set
			int max_neigh=-1; int neigh; int pivot=EMPTY_ELEM;
			if(m_conf[MAXACPLUS1].init_scan(bbo::NON_DESTRUCTIVE_REVERSE)!=EMPTY_ELEM){
				while(true){
					int v=m_conf[MAXACPLUS1].previous_bit();
					if (v==EMPTY_ELEM) break;

					neigh=g->degree(v,LISTA_BB(maxac));
					if(neigh>max_neigh){
						pivot=v;
						max_neigh=neigh;
					}
				}
			}

			//max deg P set
			if(LISTA_BB(maxac).init_scan(bbo::NON_DESTRUCTIVE_REVERSE)!=EMPTY_ELEM){
				while(true){
					int v=LISTA_BB(maxac).previous_bit();
					if (v==EMPTY_ELEM) break;

					neigh=g->degree(v,LISTA_BB(maxac));
					if(neigh>max_neigh){
						pivot=v;
						max_neigh=neigh;
					}
				}
			}
			
			
			ERASE(LISTA_BB(maxac), g->get_neighbors(pivot), m_cand[MAXACPLUS1]);
	
			///////////////////////////////////////////////////////
			// Branching
			m_path[maxac]=v;			
			res.inc_number_of_steps();
			expand_greedy_conflict_set_XP(maxac+1,LISTA_BB(maxac),m_cand[MAXACPLUS1]);  

			//////////////////////////////////////////////
			// Backtrack (does not delete v from path since it will be overwritten in the same level of search tree)
			l_bb.erase_bit(v); 
			m_conf[maxac].set_bit(v);

		}// next node
	}

return;			
}

void CliqueEnumSparse::expand_greedy_conflict_set_heur (int maxac, sparse_bitarray& l_bb , sparse_bitarray& l_cand){
////////////////////////
// Ordered pivot selection from conflict set if possible 

	const int MAXACPLUS1=maxac+1;
				
	//main loop
	if(l_cand.init_scan(bbo::NON_DESTRUCTIVE_REVERSE)!=EMPTY_ELEM){
		while(true){

			//select pivot (in order)
			int v=l_cand.previous_bit();
			if(v==EMPTY_ELEM) break;

#ifdef ROOT_VERTEX_PROGRESS
			if(maxac==0)
				cout<<"root vertex: "<<v<<endl;
#endif
			//child node generation by masking
			AND(l_bb, g->get_neighbors(v), LISTA_BB(maxac));					//optimized when place second the bitset with higher population
			AND(m_conf[maxac], g->get_neighbors(v), m_conf[MAXACPLUS1]);		//has to be evaluated before the leaf node check

			//Leaf node: updates incumbent if necessary
			if( LISTA_BB(maxac).is_empty()){
				if(m_conf[MAXACPLUS1].is_empty()){

					res.set_UB(res.get_upper_bound()+1);						//increments solution
#ifdef VIEW_PROGRESS
					m_path[maxac]=v;
					stringstream sstr("");
					for(int i=0; i<=maxac; i++){
						sstr<<m_path[i]<<" ";
					}
					LOG_INFO(sstr.str());
#endif
				}

				l_bb.erase_bit(v);
				m_conf[maxac].set_bit(v);
				continue;
			}

			//greedy pivot heuristic: last vertex from conflict set (if possible)
			//select the last vertex which has at least one neighbor in P
			int pivot=EMPTY_ELEM;
			if(m_conf[MAXACPLUS1].init_scan(bbo::NON_DESTRUCTIVE_REVERSE)!=EMPTY_ELEM){
				while(true){
					pivot=m_conf[MAXACPLUS1].previous_bit();
					if(pivot==EMPTY_ELEM) break;

					if(!(LISTA_BB(maxac).is_disjoint(g->get_neighbors(pivot))) ) break;
				}
			}

			if(pivot==EMPTY_ELEM){
				pivot=LISTA_BB(maxac).msbn64();
			}
			ERASE(LISTA_BB(maxac), g->get_neighbors(pivot), m_cand[MAXACPLUS1]);
				
			///////////////////////////////////////////////////////
			// Branching
			m_path[maxac]=v;			
			res.inc_number_of_steps();
			expand_greedy_conflict_set_heur(maxac+1,LISTA_BB(maxac),m_cand[MAXACPLUS1]);  

			//////////////////////////////////////////////
			// Backtrack (does not delete v from path since it will be overwritten in the same level of search tree)
			l_bb.erase_bit(v); 
			m_conf[maxac].set_bit(v);

		}// next node
	}

return;			
}

void CliqueEnumSparse::expand_greedy_cand_set (int maxac, sparse_bitarray& l_bb , sparse_bitarray& l_cand){
////////////////////////
// Ordered pivot selection from candidate set 

	const int MAXACPLUS1=maxac+1;

	//main loop	
	if(l_cand.init_scan(bbo::NON_DESTRUCTIVE_REVERSE)!=EMPTY_ELEM){			//Empty sparse test set condition
		while(true){

			//select pivot (in order)
			int v=l_cand.previous_bit();
			if(v==EMPTY_ELEM) break;

#ifdef ROOT_VERTEX_PROGRESS
			if(maxac==0)
				cout<<"root vertex: "<<v<<endl;
#endif
			//child node generation by masking
			AND(g->get_neighbors(v), l_bb, LISTA_BB(maxac));					//optimized when place second the bitset with higher population
			AND(m_conf[maxac], g->get_neighbors(v), m_conf[MAXACPLUS1]);		//has to be evaluated before the leaf node check

			//Leaf node: updates incumbent if necessary
			if( LISTA_BB(maxac).is_empty()){
				if(m_conf[MAXACPLUS1].is_empty()){

					res.set_UB(res.get_upper_bound()+1);						//increments solution
#ifdef VIEW_PROGRESS
					m_path[maxac]=v;
					stringstream sstr("");
					for(int i=0; i<=maxac; i++){
						sstr<<m_path[i]<<" ";
					}
					LOG_INFO(sstr.str());
#endif
				}

				l_bb.erase_bit(v);
				m_conf[maxac].set_bit(v);
				continue;
			}

			
			//greedy pivot heuristic: last vertex from candidate set
			int pivot=LISTA_BB(maxac).msbn64();
			ERASE(LISTA_BB(maxac), g->get_neighbors(pivot), m_cand[MAXACPLUS1]);

			///////////////////////////////////////////////////////
			// Branching
		
			m_path[maxac]=v;
			res.inc_number_of_steps();
			expand_greedy_cand_set(maxac+1,LISTA_BB(maxac),m_cand[MAXACPLUS1]);  

			//////////////////////////////////////////////
			// Bactrack (does not delete v from path since it will be overwritten in the same level of search tree)
			l_bb.erase_bit(v); 
			m_conf[maxac].set_bit(v);
			
		}// next node
	}
return;		
}

void CliqueEnumSparse::initial_expand (func f){
////////////////////
// Unrolling of first level to avoid working with full vertex sets in sparse real graphs
	
	const int PIVOT_ROOT=m_size-1;
		
	//Loop over neighbor set subproblems
	for(int v=PIVOT_ROOT; v>=0; v--){

		//compute P and X sets in child node
		LISTA_BB(0).init_bit(0,v,g->get_neighbors(v));						//Do NOT use (0, v-1)
		m_conf[1].init_bit(v,PIVOT_ROOT,g->get_neighbors(v));				//Do NOT use (v+1, PIVOT_ROOT)

		//check if maximal clique
		if(LISTA_BB(0).is_empty()){
			if(m_conf[1].is_empty()){

				res.set_UB(res.get_upper_bound()+1);						//increments solution
#ifdef VIEW_PROGRESS
				stringstream sstr("");
				sstr<<v;
				LOG_INFO(sstr.str());
#endif
			}
			continue;
		}

///////////////////////////
// pivot selection at root

#ifdef STRONG_ROOT_PIVOT						//Tomita pivot selection

		//best pivot from X set
		int max_neigh=-1; int neigh; int pivot=EMPTY_ELEM;
		if(m_conf[1].init_scan(bbo::NON_DESTRUCTIVE_REVERSE)!=EMPTY_ELEM){
			while(true){
				int v=m_conf[1].previous_bit();
				if (v==EMPTY_ELEM) break;

				neigh=g->degree(v,LISTA_BB(0));
				if(neigh>max_neigh){
					pivot=v;
					max_neigh=neigh;
				}
			}
		}

		////if conflict set is empty, take the first vertex in P as greedy pivot heuristic: last vertex from conflict set (if possible)
		//if(pivot==EMPTY_ELEM){
		//	pivot=LISTA_BB(0).msbn64();
		//}

		//best pivot from P set
		if(LISTA_BB(0).init_scan(bbo::NON_DESTRUCTIVE_REVERSE)!=EMPTY_ELEM){
			while(true){
				int v=LISTA_BB(0).previous_bit();
				if (v==EMPTY_ELEM) break;

				neigh=g->degree(v,LISTA_BB(0));
				if(neigh>max_neigh){
					pivot=v;
					max_neigh=neigh;
				}
			}
		}
#else	//GREEDY ROOT PIVOT

		//pivot last vertex from conflict set X always -expected largest neighborhood-
		//the original approach, succesful in non-sparse enumeration
		int pivot=m_conf[1].msbn64();
		if(pivot==EMPTY_ELEM){
			pivot=LISTA_BB(0).msbn64();
		}	
#endif

///////////////////////////////			
		ERASE(LISTA_BB(0), g->get_neighbors(pivot), m_cand[1]);	
		
		//Search
		//root_vertex_block=WDIV(v);
		m_path[0]=v;
		res.inc_number_of_steps();
		(this->*f)(1,LISTA_BB(0), m_cand[1]);

	}
}
void CliqueEnumSparse::initial_expand_sel (func f){
////////////////////
// Unrolling of first level to avoid working with full vertex sets in sparse real graphs
// Uses as pivot the last vertex of the set
//
// OBSERVATION: Computation of P and X new sets results it being slower by 3 orders of magnitude 
//				in large real sparse graphs! The justification resides in the fact that the non neighbors
//				of the pivot are ALMOST ALL the vertices in random sparse graphs! 

	const int PIVOT_ROOT=m_size-1;

	//Loop over neighbor set subproblems
	//for(int v=PIVOT_ROOT; v>=0; v--){
	if(m_cand[0].init_scan(bbo::NON_DESTRUCTIVE_REVERSE)!=EMPTY_ELEM){			//Empty sparse test set condition
		//if(g->get_neighbors(v).is_bit(PIVOT_ROOT) /*&& v!=pivot_root*/) continue;
		while(true){

			//select pivot (in order)
			int v=m_cand[0].previous_bit();
			if(v==EMPTY_ELEM) break;

			//compute P and X sets in child node (apparently horribly inefficient in large real graphs)
			//** TO EXPLAIN
			ERASE(g->get_neighbors(v), m_conf[0], LISTA_BB(0));			
			AND(m_conf[0], g->get_neighbors(v), m_conf[1]);				
			
			//check if maximal clique
			if(LISTA_BB(0).is_empty()){
				if(m_conf[1].is_empty()){

					res.set_UB(res.get_upper_bound()+1);						//increments solution
#ifdef VIEW_PROGRESS
					stringstream sstr("");
					sstr<<v;
					LOG_INFO(sstr.str());
#endif
				}
				m_conf[0].set_bit(v);
				continue;
			}

			//greedy pivoting: last vertex from the conlifct set (if possible)
			int pivot=m_conf[1].msbn64();
			if(pivot==EMPTY_ELEM){
				pivot=LISTA_BB(0).msbn64();
			}			
			ERASE(LISTA_BB(0), g->get_neighbors(pivot), m_cand[1]);	
		
			//recursive NP-hard search step
			m_path[0]=v;
			res.inc_number_of_steps();
			(this->*f)(1,LISTA_BB(0), m_cand[1]);
			m_conf[0].set_bit(v);
		}
	}
}

void CliqueEnumSparse::initial_expand_graph (func f){
////////////////////
// Unrolling of first level to avoid working with full vertex sets in sparse real graphs
	
	const int PIVOT_ROOT=m_size-1;
		
	//Loop over neighbor set subproblems
	for(int v=PIVOT_ROOT; v>=0; v--){

		//compute P and X sets in child node
		LISTA_BB(0).init_bit(0,v,g->get_neighbors(v));						//Do NOT use (0, v-1)
		m_conf[1].init_bit(v,PIVOT_ROOT,g->get_neighbors(v));				//Do NOT use (v+1, PIVOT_ROOT)

		//check if maximal clique
		if(LISTA_BB(0).is_empty()){
			if(m_conf[1].is_empty()){

				res.set_UB(res.get_upper_bound()+1);						//increments solution
#ifdef VIEW_PROGRESS
				LOG_INFO("max_clique: "<<v);
#endif
			}
			continue;
		}

//////////////////////
// Use auxiliary graph to solve it when the size is less than N
		int nP=LISTA_BB(0).popcn64();
		if(nP>MIN_PROBLEM_SIZE && m_clq.set_graph(*g,LISTA_BB(0), m_conf[1],nP,EMPTY_ELEM)!=-1){
			//	LOG_INFO("graph-v:"<<v);
			m_clq.set_up();
			m_clq.run();
			res.inc_number_of_steps(m_clq.number_of_steps());
			res.set_UB(res.get_upper_bound()+m_clq.number_of_maximal_cliques());
			res.inc_counter(0);		
			continue;
		}	

///////////////////////////
// pivot selection at root: if (P+X) > size threshold

#ifdef STRONG_ROOT_PIVOT						//Tomita pivot selection

		//best pivot from X set
		int max_neigh=-1; int neigh; int pivot=EMPTY_ELEM;
		if(m_conf[1].init_scan(bbo::NON_DESTRUCTIVE_REVERSE)!=EMPTY_ELEM){
			while(true){
				int v=m_conf[1].previous_bit();
				if (v==EMPTY_ELEM) break;

				neigh=g->degree(v,LISTA_BB(0));
				if(neigh>max_neigh){
					pivot=v;
					max_neigh=neigh;
				}
			}
		}

		////if conflict set is empty, take the first vertex in P as greedy pivot heuristic: last vertex from conflict set (if possible)
		//if(pivot==EMPTY_ELEM){
		//	pivot=LISTA_BB(0).msbn64();
		//}

		//best pivot from P set
		if(LISTA_BB(0).init_scan(bbo::NON_DESTRUCTIVE_REVERSE)!=EMPTY_ELEM){
			while(true){
				int v=LISTA_BB(0).previous_bit();
				if (v==EMPTY_ELEM) break;

				neigh=g->degree(v,LISTA_BB(0));
				if(neigh>max_neigh){
					pivot=v;
					max_neigh=neigh;
				}
			}
		}
#else	//GREEDY ROOT PIVOT

		//pivot last vertex from conflict set X always -expected largest neighborhood-
		//the original approach, succesful in non-sparse enumeration
		int pivot=m_conf[1].msbn64();
		if(pivot==EMPTY_ELEM){
			pivot=LISTA_BB(0).msbn64();
		}	
#endif

///////////////////////////////			
		ERASE(LISTA_BB(0), g->get_neighbors(pivot), m_cand[1]);	
		
		//Search
		//root_vertex_block=WDIV(v);
		m_path[0]=v;
		res.inc_number_of_steps();
		(this->*f)(1,LISTA_BB(0), m_cand[1]);

	}
}
#endif
