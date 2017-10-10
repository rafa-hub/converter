////////////////////////////////
// clique_watched.h interface CliqueWatched class which uses watched bitstrings

#ifndef  __CLIQUE_WATCHED_H__
#define  __CLIQUE_WATCHED_H__


#include "clique.h"
#include "graph/algorithms/graph_conversions.h"
using namespace std;


#define  LISTA_BBW(depth)    m_bbwsets[(depth)]						//list of vertices encoded as a bitstring


class CliqueWatched: public Clique<ugraph>{
public:
	CliqueWatched(ugraph* g, param_t p)					:Clique<ugraph>(g, p), m_bbwsets(NULL), m_wcolsets(NULL){};
	CliqueWatched(param_t p)							:Clique<ugraph>(p), m_bbwsets(NULL), m_wcolsets(NULL){};
	virtual ~CliqueWatched(){ clear_bitarrays();}

virtual	void set_graph(ugraph* g_out);
	int init_bitarrays();					
	void clear_bitarrays();
	int init_others();
	int init_color_sets();
	void clear_color_sets();

	virtual	int set_up_non_unrolled		(search_alloc_t info);		//specific initial ordering 
	
	inline void paint_W					(int depth);
	inline void paint_WT				(int depth);

	void expand_W (int maxac, watched_bitarray& l_bb , nodelist_t& l_v);
	void expand_WT (int maxac, watched_bitarray& l_bb , nodelist_t& l_v);
			
virtual	int set_up();
virtual	void run();	
virtual	void tear_down(){clear_bitarrays(); clear_color_sets(), Clique<ugraph>::tear_down();}

protected:
////////////////
// data members
	watched_bitarray*	m_bbwsets;			//[DEPTH][MAX_VERTEX]
	watched_bitarray	m_bbwroot;										
	watched_bitarray	m_wunsel;
	watched_bitarray    m_wsel;

	watched_bitarray*	m_wcolsets;			//storage of color sets
};

inline
int CliqueWatched::set_up_non_unrolled	(search_alloc_t info){
////////////////////////
// specific setup which contains both the real graph case and the normal graph case
// depending on the param.unrolled flag
//
// REMARKS: note that here the param.unrolled flag only discrimates over large or normal graphs
//          but there IS NO REAL UNROLLING
	
	int ub=param.ub; 
	int lb=param.lb;

	if(!param.unrolled){								
		Clique<ugraph>::set_up_non_unrolled(info);	
	}else{																	
		/* we use a similar pre-processing as in the unrolled case for real graphs*/
		sparse_ugraph sug;
		GraphConversion::ug2sug(*g,sug);

		Result r;
		r.tic();
		LOG_PRINT("INIT SETUP WATCHED: ASSUMES CORE NUMBER IS LOW------------");

		//initial kcore bounds and kcore filter
		KCore<sparse_ugraph> kc(sug);
		int ub_kc, lb_kc, sol;
		Clique<sparse_ugraph> cli(&sug, clqo::param_t());

		if( (sol=cli.initial_bounds(lb, ub, &kc)) >0){
			r.toc();
			return sol;
		}

		r.toc();
		LOG_INFO("w:("<<lb<<","<<ub<<")");
		LOG_PRINT("[tkcore:"<<r.get_user_time()<<"]");
		//cout<<"[t:"<<pt.wall_toc()<<"]"<<endl;

		//Initial order (MWS-kcore based)
		r.tic();
		LOG_PRINT("init degeneracy reordering----------------");
		CliqueSort<sparse_ugraph> o(sug);	
		const vint& kco=kc.get_kcore_ordering();
		vint old2new(kco.size());
		int l=0;
		for(vint::const_reverse_iterator it=kco.rbegin(); it!=kco.rend(); ++it){
			old2new[*it]=l++;
		}

		LOG_DEBUG("degeneracy reordering: init in place reordering---------------");
		//o.reorder(old2new);
		o.reorder_in_place(old2new);	//efficient in space requirements, used for very large sparse graphs
		r.toc();
		LOG_PRINT("[tdegord:"<<r.get_user_time()<<"]");


		GraphConversion::sug2ug(sug, *g);

		//Init over the new graph (shrinked)
		r.tic();
		LOG_PRINT("init search allocation----------------");
		info.size=ub;
		if(search_allocation(info)==-1) return -1;

		LOG_PRINT("END OF SETUP UNROLLED");
		r.toc();
		LOG_PRINT("[talloc:"<<r.get_user_time()<<"]");


		//update search parameters
		maxno=lb;
		res.set_LB(lb);
	}

return 0;

}

inline
int CliqueWatched::init_bitarrays(){
	clear_bitarrays();
	
	//bb nodes
	try{
		m_bbwsets=new watched_bitarray[m_alloc];
		for(int i=0; i<m_alloc; i++){
			m_bbwsets[i].init(m_size);			//set_to_0
			m_bbwsets[i].init_sentinels();
		}
	}catch(exception e){
		throw;
	}

	//bitstring at root node
	m_bbwroot.init(m_size);
	m_bbwroot.init_sentinels();
	m_bbwroot.set_bit(0, m_size-1);
	return 0;
}

inline
void CliqueWatched::clear_bitarrays(){
	if(m_bbwsets!=NULL){
		delete [] m_bbwsets;					
	}
	m_bbwsets=NULL;
}

inline
int CliqueWatched::init_others(){
	m_wunsel.init(m_size);
	m_wunsel.init_sentinels();	
	m_wsel.init(m_size);
	m_wsel.init_sentinels();	
	return 0;
}

inline
int CliqueWatched::init_color_sets(){
	clear_color_sets();

	try{
		m_wcolsets=new watched_bitarray[(m_alloc)+1];			//[0] is used to store the subgraph to color. Actual colors range from [1, N]	
		for(int i=0; i<=m_alloc; i++){
			m_wcolsets[i].init(m_size);						
	}
	}catch(exception& e){
		throw;
	}

	return 0;
}

inline
void CliqueWatched::clear_color_sets(){
	if(m_wcolsets!=NULL){
		delete [] m_wcolsets;  
	}
	m_wcolsets=NULL;
}

inline
void CliqueWatched::set_graph(ugraph* g){
	this->clear_bitarrays();
	Clique<ugraph>::set_graph(g);
}

inline
int CliqueWatched::set_up(){
//////////////
// allocates memory, evaluates initial bounds and determines 
// initial trivial solutions
//
// RETURN VALUE: -1 Error, 0-ok, >0 trivial solution found

	int sol=0;
	res.clear();
	res.set_name(g->get_name());

	//determine allocation info
	search_alloc_t info;					//loads BBMC configuration for search allocation
	switch(param.alg){
	case BBMC_W:
		//default info is ok
		break;
	case BBMC_WT:
		info.set(search_alloc_t::ALLOC_COLOR_SETS);
		break;
	default:
		LOG_ERROR("CliqueWatched::setup unknown algorithm");
		return -1;
	}
	
	//actual set_up, in this case the parm.unrolled flag is allowed (it affects set_up_non_unrolled())
	res.tic(true);
	if( (sol=set_up_non_unrolled(info))>0 ){
		res.set_UB(sol);
	}
	res.toc(true);

	
	
	if(sol>0){
		LOG_INFO("[w="<<sol<<"t:"<<res.get_pre_time()<<"]"<<" TRIVIALLY SOLVED DURING PRECOMPUTATION");
	}else{
		LOG_INFO("[t:"<<res.get_pre_time()<<"]");
	}

	return sol;
}

inline
void CliqueWatched::run(){
	//algorithm (param.unrolled flag is allowed; in all cases it is non_unrolled)
	res.tic();
	switch(param.alg){
	case BBMC_W:
		expand_W(0, m_bbwroot, m_lroot);
		res.toc();
		break;
	case BBMC_WT:
		expand_WT(0, m_bbwroot, m_lroot);
		res.toc();
		break;
	default:
		LOG_ERROR("CliqueWatched::run-non_unrolled:unknown clique algorithm");
	}
	

	//read solution
	res.set_UB(maxno);
	LOG_INFO("[w:"<<res.get_upper_bound()<<","<<res.get_user_time()<<"s]");
}	

inline
void CliqueWatched::expand_W (int maxac, watched_bitarray& l_bb , nodelist_t& l_v){
////////////////////////
// recursive search algorithm

	int v;
//main loop
	res.inc_number_of_steps();
	

	while(l_v.index>=0){
		//l_bb.print(); cout<<endl;

		//Estrategias
		v=l_v.nodos[l_v.index--];
	
		//CUT by color (since [Konc & Janecic, 2007] this is of limited use (only for first branch))
		if( (m_lcol[maxac][v]+maxac)<=maxno )
				return;
/////////////////////////////////
// CHILD NODE GENERATION
		
		//Node generation by masking
		AND(g->get_neighbors(v), l_bb, LISTA_BBW(maxac));		//optimized for lbb placed second the bitset with higher population
		//LISTA_BBW(maxac).print(); cout<<endl;

		//Leaf node: updates incumbent if necessary
		if( LISTA_BBW(maxac).update_sentinels()==EMPTY_ELEM){
			if(maxac>=maxno){
				maxno=maxac+1;						//NEW GLOBAL OPTIMUM FOUND
				
				#ifdef STORE_SOLUTION
					res.set_UB(maxno);
					res.clear_all_solutions();
					m_path[maxac]=v;
					res.add_solution(maxno, m_path);
				
					#ifdef VIEW_PROGRESS
						stringstream sstr("");
						res.print_first_sol(sstr);
						LOG_INFO(sstr.str());
					#endif
					
				#endif
			}
		//l_bb.erase_bit(v);
		l_bb.erase_bit_and_update(v);
		continue;
		}
		//approx. coloring (generates child list of nodes in LISTA_L(maxac))
		paint_W(maxac);

		//cuts if there are no child nodes of v
		if(LISTA_L(maxac).index<0){
			//l_bb.erase_bit(v);
			l_bb.erase_bit_and_update(v);
			continue;
		}
				
///////////////////////////////////////////////////////
// CANDIDATE EXPANSION

		//sets path
		m_path[maxac]=v;
				
		//Generacion de nuevos nodos
		expand_W(maxac+1,LISTA_BBW(maxac),LISTA_L(maxac));  

		//////////////////////////////////////////////
		// BACKTRACK (does not delete v from path since it will be overwritten in the same level of search tree)
		//l_bb.erase_bit(v);
		l_bb.erase_bit_and_update(v);
	}// next node

return;
}

inline
void CliqueWatched::expand_WT (int maxac, watched_bitarray& l_bb , nodelist_t& l_v){
////////////////////////
// recursive search algorithm 

	int v;
//main loop
	res.inc_number_of_steps();
	

	while(l_v.index>=0){
		//l_bb.print(); cout<<endl;

		//Estrategias
		v=l_v.nodos[l_v.index--];
	
		//CUT by color (since [Konc & Janecic, 2007] this is of limited use (only for first branch))
		if( (m_lcol[maxac][v]+maxac)<=maxno )
				return;
/////////////////////////////////
// CHILD NODE GENERATION
		
		//Node generation by masking
		AND(g->get_neighbors(v), l_bb, LISTA_BBW(maxac));		//optimized for lbb placed second the bitset with higher population
		//LISTA_BBW(maxac).print(); cout<<endl;

		//Leaf node: updates incumbent if necessary
		if( LISTA_BBW(maxac).update_sentinels()==EMPTY_ELEM){
			if(maxac>=maxno){
				maxno=maxac+1;						//NEW GLOBAL OPTIMUM FOUND
				
				#ifdef STORE_SOLUTION
					res.set_UB(maxno);
					res.clear_all_solutions();
					m_path[maxac]=v;
					res.add_solution(maxno, m_path);
				
					#ifdef VIEW_PROGRESS
						stringstream sstr("");
						res.print_first_sol(sstr);
						LOG_INFO(sstr.str());
					#endif
					
				#endif
			}
		//l_bb.erase_bit(v);
		l_bb.erase_bit_and_update(v);
		continue;
		}
		//approx. coloring (generates child list of nodes in LISTA_L(maxac))
		paint_WT(maxac);

		//cuts if there are no child nodes of v
		if(LISTA_L(maxac).index<0){
			//l_bb.erase_bit(v);
			l_bb.erase_bit_and_update(v);
			continue;
		}
				
///////////////////////////////////////////////////////
// CANDIDATE EXPANSION

		//sets path
		m_path[maxac]=v;
				
		//Generacion de nuevos nodos
		expand_WT(maxac+1,LISTA_BBW(maxac),LISTA_L(maxac));  

		//////////////////////////////////////////////
		// BACKTRACK (does not delete v from path since it will be overwritten in the same level of search tree)
		//l_bb.erase_bit(v);
		l_bb.erase_bit_and_update(v);
	}// next node

return;
}

inline void CliqueWatched::paint_W(int depth){
///////////////////
// Sequential greedy independent set vertex coloring which prunes the search tree

	int col=1, kmin=maxno-depth, v=EMPTY_ELEM;		
	LISTA_L(depth).index=EMPTY_ELEM;												//cleans the set fo candidate vertices
	const int DEPTH_PLUS1=depth+1;
		
	//copies list of vertices to color and stores size for fast empty check 
	//m_wunsel=LISTA_BBW(depth);
	int pc= (m_wunsel=LISTA_BBW(depth)).popcn64();

	//CUT based on population size
	if(pc<kmin){
			return;
	}

	//main loop
	while(true){ 
		m_wsel=m_wunsel;
		m_wsel.init_scan(bbo::DESTRUCTIVE);		
		while(true){
			v=m_wsel.next_bit_del(m_wunsel);
			if(v==EMPTY_ELEM)
							break;
			if(col>=kmin){  
				LISTA_L(depth).nodos[++LISTA_L(depth).index]=v;
				m_lcol[DEPTH_PLUS1][v]=col;											//labels start at 1	
			}
			if((--pc)==0){
				return;
			}
			m_wsel.erase_bit(g->get_neighbors(v));
		}	
	if(m_wunsel.update_sentinels()==EMPTY_ELEM)										//it is not necessary to check for emptyness here
			return;
	col++;
	}
}

inline void CliqueWatched::paint_WT	(int depth){
/////////////////////////
// non-independent set SEQ 
// date of creation: 20/03/15
																															
	int cmax=1, kmin=maxno-depth, nBB=EMPTY_ELEM, v=EMPTY_ELEM; 
	LISTA_L(depth).index=-1;								
	const int DEPTHPLUS1=depth+1;
	bool iscol;
	
	//clears primer color (starts at index 1)
	m_wcolsets[1].set_sentinels(LISTA_BBW(depth).get_sentinel_L(), LISTA_BBW(depth).get_sentinel_H());
	m_wcolsets[1].erase_bit();
	
	//outer loop: select a vertex v to color
	LISTA_BBW(depth).init_scan(bbo::NON_DESTRUCTIVE);
	while(1){
		if((v=LISTA_BBW(depth).next_bit(nBB))==EMPTY_ELEM)
											break;
		
		//inner loop: search for SEQ color for v
		for(int col=1; col<=cmax; col++){
			iscol=g->get_neighbors(v).is_disjoint(m_wcolsets[col].get_sentinel_L(), nBB, m_wcolsets[col]);	
							
			//color found for vertex
			if(iscol){
				m_wcolsets[col].set_bit(v);			//doesn´t update sentinels but v is in range	
				break;
			}
		}

		//color not found: new color
		if(!iscol){
			cmax++;
			m_wcolsets[cmax].set_sentinels(LISTA_BBW(depth).get_sentinel_L(), LISTA_BBW(depth).get_sentinel_H());
			m_wcolsets[cmax].erase_bit();			
			m_wcolsets[cmax].set_bit(v);			//doesn´t update sentinels but v is in range			
		}
	}//pick next vertex to color	

////////////////////////
//copy vertices to child node candidate list sorted by color

	if(kmin<=0) kmin=1;								//if first branch (and no initial UB) kmin=1 so as to copy all nodes	
	for(int col=kmin; col<=cmax; col++){
		m_wcolsets[col].init_scan(bbo::DESTRUCTIVE);
		while(1){
			if( (v=m_wcolsets[col].next_bit_del())==EMPTY_ELEM )
						break;

			LISTA_L(depth).nodos[++LISTA_L(depth).index]=v;
			m_lcol[DEPTHPLUS1][v]=col;
		}
	}	
	return;
}

#endif