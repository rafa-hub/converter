////////////////////////////////
// clique_csp_plus.h: interface for the CliqueCSP_Plus class which contains an implementation of
//				   an exact maximum clique solvers for CSP reduction to MCP
//
// initial date:22/02/17
// last update:14/03/17
// author: pablo san segundo


#ifndef  __CLIQUE_CSP_PLUS_H__
#define  __CLIQUE_CSP_PLUS_H__

#include "bitscan/bbalg.h"
#include "graph/algorithms/graph_sort.h"
#include "graph/algorithms/graph_func.h"
#include "clique.h"
#include "../init_color_ub.h"
#include "../amts/amts_exec.h"
#include "../common/common_macros.h"
#include "../clique_sort.h"
#include "infra_tools_plus_csp.h"
#include "../common/common_clq.h"
#include "../clique/filter_csp_plus.h"

using namespace com;												//for common types (here bb_t)
using namespace comclq;

///////////////////////
//CSP FLAGS
#define LEAST_RESTRICTING_VAL										//switch good for FRB (currently only mode for SUBSUME_FILTER)
#undef  LEAST_RESTRICTING_VAL

#ifdef  LEAST_RESTRICTING_VAL
	#define SUBSUME_FILTER											//filters candidates whose neighborhood is contained by already examined ones
	//#undef SUBSUME_FILTER
#endif

//#define CSP_FILTER_ON												//uses CSP-filter if possible 
#ifdef  CSP_FILTER_ON
	#define CSP_FILTER_SEARCH_INFO
	#undef CSP_FILTER_SEARCH_INFO
#endif


template<int N>
struct subset{
/////////////////
// accumulates data about a pmax_sat color_db with maximum size N:
// i. colors (vars) classified by sizes
// ii.nodes related to these colors (currently not implemented)
//
// date_of_creation: During Easter Holiday in Lisbon (15-4-17)
//
// COMMENTS
// Attempt to apply it for:
// i. decission selection heuristic: choose the variable with less size 
//    from the color sets
// ii.reduce pmax-sat techniques to these subset

	static const int MAX_SIZE=N;					/* determines the subset of variables, at most MAX_SIZE */
	int	MAX_NB_SETS_FOR_ANY_SIZE;	
	com::stack_t<int>* lss;

	subset():lss(NULL), MAX_NB_SETS_FOR_ANY_SIZE(-1){ lss = new com::stack_t<int>[MAX_SIZE+1 /*0 based */];}
	~subset(){ delete [] lss /* deallocates all stacks */; lss=NULL;}

	void init(int mss){ 
		MAX_NB_SETS_FOR_ANY_SIZE=mss;
		for(int i=0; i<=N; i++){
			lss[i].init(MAX_NB_SETS_FOR_ANY_SIZE);
		}
	}	
	
/////////////////
// interface proper (once initialized with init(...))
	void push(int size, int iset){if(size<=MAX_SIZE) lss[size].push(iset); }
	void erase(){for(int size=0; size<=MAX_SIZE; size++) lss[size].erase();}
	int get_min_h2l(InfraOpPlus<ugraph,bitarray>&);
	int get_min_l2h(InfraOpPlus<ugraph,bitarray>&);
			
	//I/O
	ostream& print(ostream& o=cout);
};

class CliqueCSP_Plus: public Clique<ugraph>{
	static const int MAX_SIZE_FOR_SETS_IN_SUBSET=8 /* >=5 */;						/* max size of stored colors (vars) in subset */

	enum ret_t {TRIVIAL_SOL=-2, PRUNE=-1, OK=0};
	enum order_t{DEFAULT=0, RLF};

public:
	struct less_restricting_val{													/* functor predicate for max deg */
		bool operator ()(int v, int w) const{
			return (g.degree(v,subgraph)<g.degree(w,subgraph));
		}
		less_restricting_val(const ugraph& g, const bitarray& bbs):g(g), subgraph(bbs){}
		
		const ugraph& g;
		const bitarray& subgraph;
	};

	struct more_restricting_val{													/* functor predicate for min deg */
		bool operator ()(int v, int w) const{
			return (g.degree(v,subgraph)>g.degree(w,subgraph));
		}
		more_restricting_val(const ugraph& g, const bitarray& bbs):g(g), subgraph(bbs){}
		
		const ugraph& g;
		const bitarray& subgraph;
	};

public:
	CliqueCSP_Plus(ugraph* g, param_t p)				: Clique<ugraph>(g, p), NB_CSP_VAR(-1), NB_INC_VAL(0), m_cpath(NULL), m_mode(DEFAULT)/*, gc(NULL) */{};
	CliqueCSP_Plus(param_t p)							: Clique<ugraph>(p), NB_CSP_VAR(-1),  m_cpath(NULL)/*, gc(NULL) */{};
	virtual ~CliqueCSP_Plus(){}
	void set_CSP_VAR(int n){NB_CSP_VAR=n;}
	int get_CSP_VAR(){return NB_CSP_VAR;}
	int get_NB_INC_VAL(){return NB_INC_VAL;}
	void set_ranges();

	virtual int init_color_sets();	
	virtual void clear_color_sets();

	virtual int init_others();
	void clear_others();
		
//operations
	int filter_unit_root();												/* removes unit variables during pre-processing */
	int delete_unit_color_stack(bitarray& bb);							/* delete unit colors from current subgraph */		
	
	//decision heuristics for variables and values
	int highest_variable(nodelist_t& l_v,bitarray& bb);
	int lowest_variable(nodelist_t& l_v, bitarray& bbsg);
	int minimum_domain_variable_h2l(nodelist_t& l_v,bitarray& bb);
	int minimum_domain_variable_l2h(nodelist_t& l_v,bitarray& bb);
	int from_candidate_set(nodelist_t& l_v,bitarray& bb);
	
	//filters for values
	int filter_subsume(nodelist_t& l, bitarray& bbsg);

	//filter based on coloring (directional arc-consistency)
	ret_t r_iseq(int depth, int kmin /* maxno-maxac*/, bitarray& bb);
	ret_t iseq(int depth, int kmin  /* maxno-maxac*/  , bitarray& bb);
	ret_t iseq_subset(int depth, int kmin  /* maxno-maxac*/  , bitarray& bb);

	//pmax-sat filters implemented as external calls to iop
	bool test_by_eliminate_failed_nodes_csp_subset (bitarray& bbsg, sbb_t<bitarray>& s);
	bool inc_maxsatz_lookahead_csp_subset(bitarray& bbsg, sbb_t<bitarray>& s);
			
//drivers
	bool expand_csp (int depth, int maxac, bitarray& l_bb /* nodelist_t& l_v*/);
	bool expand_csp_subset (int depth, int maxac, bitarray& l_bb /* nodelist_t& l_v*/);
					
//test framework
	virtual	int set_up();
	virtual	void run();	
	virtual	void tear_down(){ /*Clique<ugraph>::tear_down();*/}

	virtual int set_up_non_unrolled	(search_alloc_t info);

//tests / checks
	bool check_domains				();
	int sum_of_dom_val				();
	bool check_for_unit_variables	(bitarray& bbsg);
	bool check_for_unit_variables	(bitarray& bbsg, sbb_t<bitarray>& unit_stack);
		
//I/O
	ostream& print_csp_info			(ostream& o=cout);
	ostream& print_ranges			(ostream& o=cout);
	ostream& print_color_ranges		(bitarray& bbsg, ostream& o=cout);		//prints colors based on ranges

private:
//////////////
// data members

//root data
	int NB_CSP_VAR;	
	int NB_INC_VAL;														/* nodes which cannot make part of solution (pre-proc) */
	vint DOMAINS;														/* [VAR] sizes of variable domains */	
	vector<range_t> RANGES;												/* [NODES] ranges [vlow, vhigh] of its root color class; unit color classes->vlow=vhigh  */
	order_t m_mode;														/* initial order mode: will determine pivoting */

//dynamic info
	com::stack_t<int>* m_cpath;
	subset<MAX_SIZE_FOR_SETS_IN_SUBSET> m_subset;						/* subset of colors: order for p-maxsat filters */
	
	InfraOpPlusCSP<ugraph,bitarray> iop;								/* infra-chrom filter tailored for CSP as MCP */
	sbb_t<bitarray> m_color_unit_stack;									/* stack of unit colors at the current node */
	com::stack_t<int> m_candidates;										/* stack to store possible candidates to expand in the current node */
	
	FilterCSP_Plus m_f;
	vint lnodes_f;
};

inline
int CliqueCSP_Plus::filter_subsume(nodelist_t& l_cand, bitarray& bbsg){
///////////
// date_of_creation: 30/5/17
// filters out some candidates from nodelist according to subsumption
// PARAMS: 1. l-cand is the list of candidates
//         2. bbsg is the current subgraph
//
// RETURNS: number of candidate vertices filtered
// 
// COMMENTS
// 1. alfa version: Takes last candidate vertex and deletes 
//   all other candidates subsumed by him (contains its neighborhood in bbsg)                 
//  
// **TODO-OPTIMIZE- allocates two auxiliary bitsets etc...
	
	int nb_filtered=0;
	int v_ref=l_cand.nodos[l_cand.index];
	int j=l_cand.index-1;
	bitarray bb_ref(g->number_of_vertices());
	AND(g->get_neighbors(v_ref), bbsg, bb_ref);
	bitarray bb_aux(g->number_of_vertices());			
	for(;j>=0; j--){
		int v_cand=l_cand.nodos[j];
		AND(g->get_neighbors(v_cand), bbsg, bb_aux);
		if(bb_aux==bb_ref){
		//	LOG_INFO("FILTERED: "<<v_cand<<" por "<<v_ref);
		//	bb_aux.print();
		//	bb_ref.print();
		//	cin.get();
			l_cand.nodos[j]=v_ref;
			l_cand.index=j;
			nb_filtered++;
		}else break;				//** TODO ***
	}

	return nb_filtered;
}

inline
bool CliqueCSP_Plus::test_by_eliminate_failed_nodes_csp_subset (bitarray& bbsg, sbb_t<bitarray>& s){
///////////
// Driver to find ONE conflict by proving all FAILED nodes in a color set.
// Once a literal is proven failed, it is assigned FALSE and removed from
// its color set. This may in turn fire UL inferences (when only one literal is left) which, if succesful, 
// permanently modify color_DB in further inferences.
// 
// Currently applied to ALL ACTIVE COLORS (enlarged or not)
//
// RETURNS: TRUE if a conflict has been found or FALSE otherwise
//
// REMARKS: 
// 1. Possibly filter tested colors by size 
// 2. Possibly stop working on a color after a number of nodes have not been proved FALSE
// 3. DOES NOT TEST FOR ACTIVE NODES! (color_db is consistent always before this call)
//
// COMMENTS: Passing subgraph, deletion of conflicting nodes not working!!!
//
// TESTING ELIMINATING ONLY THE LAST NODE WHICH IS IN UNIT COLOR NB_OF_COLORS

	int my_iset, nb_partial_conf, node, save_color_unit_stack_pt=iop.color_unit_stack.pt;	
	bool conflict=false;
		
	BITBOARD bb;	
	do{
		nb_partial_conf=0;
		for (int size=2; size<=MAX_SIZE_FOR_SETS_IN_SUBSET; size++){			/* more chances to prune in this order */
	//	for (int size=MAX_SIZE_FOR_SETS_IN_SUBSET; size>=2; size--){
			for(int i=0; i<m_subset.lss[size].pt; i++){
				int iset=m_subset.lss[size].get_elem(i);
				if( iop.m_colSets[iset].size<=4 /* curr. best rate speed/pruning */ && iop.color_state_active.is_bit(iset) ){	
					/*if(iop.m_colSets[iset].size==1){
						LOG_ERROR("BIZARRE-TESTING SINGLE COLOR FOR NODE ELIMINATION");
						cin.get();
					}*/
					conflict=false;
					iop.color_unit_dyn_stack.erase();
													
					int col_size=iop.m_colSets[iset].size;																		/* size is not necessarily parameter size */
					iop.m_colSets[iset].bb.init_scan(bbo::NON_DESTRUCTIVE);
					for(int i=0; i<col_size; i++){
						node=iop.m_colSets[iset].bb.next_bit();																/* the node becomes passive: should not be scanned again !*/
						if(!iop.node_state_active.is_bit(node)){
							LOG_ERROR("CliqueCSP_Plus::test_by_eliminate_failed_nodes_csp_subset -bizarre non-active node");
							cin.get();
						}
						
						//testing node
						if( iop.test_node_for_failed_nodes_csp(node, iset)!=InfraOpPlus<ugraph,bitarray>::NO_CONFLICT){
							nb_partial_conf++;		

							//remove conflicting node
							bbsg.erase_bit(node);
							iop.node_state_active.erase_bit(node);
							iop.m_colSets[iset].erase_bit(node);						/* alternatively m_colSets[iset].size--;	*/

							if(iop.m_colSets[iset].size==0)	{		
								return true;										/* conflict found: all values conflicting  */
							}
						}
					}// next node

					//if active color has turned single by elimination of values, then hard-propagates the unit color, making permanent changes 
					if(iop.m_colSets[iset].size==1){
						//LOG_INFO("unit color set expansion during test eliminate nodes: "<<iset);
						iop.color_unit_dyn_stack.erase();
						iop.color_unit_dyn_stack.push(iset);
						if(iop.unit_iset_process_for_test_csp(bbsg, s)!=InfraOpPlus<ugraph,bitarray>::NO_CONFLICT ){
							//LOG_INFO("InfraOpPlus<graph_t,bitboard_t>::test_by_eliminate_failed_nodes_csp-unit color derived when removing nodes");
							return true;	
						}
					}
				}//filter color_state active

			}//next color to test
		}//next color size
	}while(nb_partial_conf>1 /* >=1 increases so much overhead?-always check */ );

//	reset_context_for_maxsatz(0,0,0,save_color_unit_stack_pt);	/* TODO see if needed and why */
	
	return(conflict);
}


bool CliqueCSP_Plus::inc_maxsatz_lookahead_csp_subset(bitarray& bbsg, sbb_t<bitarray>& s){
//////////
// RETURNS TRUE if one conflict is found, FALSE otherwise. 
// Tailored for CSPs but currently applied to non-incremental version
// 
// COMMENTS: Does not store involved sets, since one conflict is enough to CUT
//
// REMARKS:
// 1.NO TESTING OF ACTIVE NODES! (assumes color_db is consistent, i.e. no reduced nodes)
// 2. Because of (1), should in theory be called before the eliminate_node_test filter	(original colors only have active nodes!)

	bool test_flag, no_conflict_flag, one_conflict=false;
	int nb_conflict=0;

	do{
		nb_conflict=0;
		//for (int k=4 /*InfraOpPlus<ugraph,bitarray>::MAX_COLOR_TEST_LENGTH*/; k>=2; k--){
		for (int k=5; k<=MAX_SIZE_FOR_SETS_IN_SUBSET; k++){
			for(int i=0; i<m_subset.lss[k].pt; i++){
				int iset=m_subset.lss[k].get_elem(i);

				if(iop.m_colSets[iset].size>=5  && iop.color_state_active.is_bit(iset)				/* 2 or more elements if color is active */ 
					/*&& !m_colSets[iset].bb.get_bitboard(NB_OF_BB_NODES) /* not weakened*/){				  
						//LOG_INFO("PASSED THROUGH HERE");
						//color_involved_stack.erase();
						bitarray& bbcol=iop.m_colSets[iset].bb;
						bbcol.init_scan(bbo::NON_DESTRUCTIVE);
						no_conflict_flag=false;
						int size=iop.m_colSets[iset].size;												/* size of color set need not be k now !*/
						for(int i=0; i<size; i++){
							int node=bbcol.next_bit();													/* note that colors in color_db only have active nodes */

							if(iop.inc_test_node_csp(node, iset, (i==size-1) /* is last node*/)==InfraOpPlus<ugraph,bitarray>::NO_CONFLICT){		 /*no testing of active nodes-TEST*/
								no_conflict_flag=true;					//no conflict
								break;
							}else{					//conflict for this particular node
								nb_conflict++;
								bbsg.erase_bit(node);

								/*LOG_INFO("REMOVED NODE TEST ELIMINATE NODE");
								cin.get();*/
								iop.node_state_active.erase_bit(node);
								iop.m_colSets[iset].erase_bit(node);								/* m_colSets[iset].size--; */
								if(iop.m_colSets[iset].size==0){									/* only check for empty set, not single color propagation */
									//	LOG_INFO("REMOVED NODE TEST ELIMINATE NODE");
									return true;												/* conflict found */
								}//else if(m_colSets[iset].size==1){
								//	//LOG_INFO("JLJLKHLJG");
								//	s.push(FIRST_SHARED(m_colSets[iset].bb, node_state_active));/* test if node exists */	
								//	color_unit_dyn_stack.erase();								/* test impact of this */
								//	color_unit_dyn_stack.push(iset);							/* a la test_eliminate_node */
								//	if(unit_iset_process_for_test_csp(bbsg)!=NO_CONFLICT ){
								//		return true;											/* conflict found */
								//	}
								//}
							}
						}//next node
						if(no_conflict_flag==false)
							return true;			/* conflict found: all nodes in color set found inconsistent*/
						else if(iop.m_colSets[iset].size==1){   /* hard propagates singleton color-note that initially no color mayhave size 1 so MUST HAVE BEEN REDUCED  */
							//LOG_INFO("JLJLKHLJG");
							//s.push(FIRST_SHARED(m_colSets[iset].bb, node_state_active));/* test if node exists */	
							iop.color_unit_dyn_stack.erase();								/* test impact of this */
							iop.color_unit_dyn_stack.push(iset);							/* a la test_eliminate_node */
							if(iop.unit_iset_process_for_test_csp(bbsg, s)!=InfraOpPlus<ugraph,bitarray>::NO_CONFLICT ){
								return true;											/* conflict found */
							}
						}
				}
			}//next color (iset)
		}
	}while(nb_conflict>1);
	return false;
}


inline
int CliqueCSP_Plus::delete_unit_color_stack(bitarray& bb){
	for(int i=0; i<m_color_unit_stack.size; i++){
		//bb&=g->get_neighbors(m_color_unit_stack.stack[i]);
		bb.erase_bit(m_color_unit_stack.stack[i]);
	}
	//bb.erase_bit(m_color_unit_stack.bb);
	return m_color_unit_stack.size;
}

inline
int CliqueCSP_Plus::highest_variable(nodelist_t& l_v, bitarray& bbsg){
/////////////////
// Decision heuristics which computes the list of candidates that
// that correspond with the varaible with highest index (color)
//
// PARAMS: bbsg->current subgraph, l_v->set of candidates to return
//
// RETURNS domain size (branching factor for bbsg subgraph), l_v (set of candidates in bbsg)
//
// COMMENTS: 
// 1.called at such a point that there should not be any unit variable to choose from
// 2.bb should not be empty

	int v, v_last=bbsg.msbn64();
	l_v.index=EMPTY_ELEM;

	int offset=RANGES[v_last].vl-1;				
	bbsg.init_scan_from((offset<=0)? -1 : offset, bbo::NON_DESTRUCTIVE);			/* scans excluding 'from' vertex */
	while(true){
		if( (v=bbsg.next_bit())== EMPTY_ELEM || (v > v_last)) break;
		l_v.nodos[++l_v.index]=v;
		
		//tests
		if(m_color_unit_stack.bb.is_bit(v)){
			LOG_ERROR("BIZARRE VERTEX");
			cin.get();
		}
	}
	return l_v.index+1;
}

inline
int CliqueCSP_Plus::from_candidate_set(nodelist_t& l_v, bitarray& bbsg){
/////////////
// Attempt to choose a candidate during iop inferences

	int v, v_last;
	if(m_candidates.pt!=0){
		//LOG_INFO("CHOOSIND CAND FROM CAND SET");
		v_last=m_candidates.stack[0];
	}else v_last=bbsg.msbn64();
	l_v.index=EMPTY_ELEM;

	int offset=RANGES[v_last].vl-1;				
	bbsg.init_scan_from((offset<=0)? -1 : offset, bbo::NON_DESTRUCTIVE);			/* scans excluding 'from' vertex */
	while(true){
		if( (v=bbsg.next_bit())== EMPTY_ELEM || (v> RANGES[v_last].vh)) break;
		l_v.nodos[++l_v.index]=v;
		
		//tests
		if(m_color_unit_stack.bb.is_bit(v)){
			LOG_ERROR("BIZARRE VERTEX");
			cin.get();
		}
	}
	return l_v.index+1;
}

inline
int CliqueCSP_Plus::lowest_variable(nodelist_t& l_v, bitarray& bbsg){
/////////////////
// Decision heuristics which computes the list of candidates that
// that correspond with the variable with lowest index (color)
//
// PARAMS: bbsg->current subgraph, l_v->set of candidates to return
//
// RETURNS domain size (branching factor for bbsg subgraph), l_v (set of candidates in bbsg)
//
// COMMENTS: 
// 1.called at such a point that there should not be any unit variable to choose from
// 2.bb should not be empty

	int v, v_first=bbsg.lsbn64();
	l_v.index=EMPTY_ELEM;

	//int offset=RANGES[v_first].vl-1;	
	int offset=v_first-1;
	bbsg.init_scan_from((offset<=0)? -1 : offset, bbo::NON_DESTRUCTIVE);			/* scans excluding 'from' vertex */
	while(true){
		if( (v=bbsg.next_bit())== EMPTY_ELEM || (v> RANGES[v_first].vh)) break;
		l_v.nodos[++l_v.index]=v;
		
		//tests
		if(m_color_unit_stack.bb.is_bit(v)){
			LOG_ERROR("BIZARRE VERTEX");
			cin.get();
		}
	}
	return l_v.index+1;
}

inline
int CliqueCSP_Plus::minimum_domain_variable_h2l(nodelist_t& l_v,bitarray& bbsg){
/////////////////
// Decision heuristics which computes the smallest possible list of candidates 
//
// PARAMS: bbsg->current subgraph, l_v->set of candidates to return
//
// RETURNS domain size (branching factor for bbsg subgraph), l_v (set of candidates in bbsg)
//
// COMMENTS: 
// 1.called at such a point that there should not be any unit variable to choose from
// 2.bb should not be empty
	
	//int v, v_cand=bbsg.msbn64();
	//l_v.index=EMPTY_ELEM;
	//
	//int iset=iop.node_iset_no[v_cand], min_size=iop.m_colSets[iset].size, min_var=iset;
	//if(min_size>=3){
	//	for(int i=min_var-1; i>=1; i--){
	//		if(min_size>iop.m_colSets[i].size &&  iop.color_state_active.is_bit(i) ){
	//			min_size=iop.m_colSets[i].size;
	//			min_var=i;
	//			if(min_size==2)
	//				break;

	//		}

	//	}
	//	v_cand=FIRST_SHARED(iop.m_colSets[min_var].bb,iop.node_state_active);
	//}
	
	//a more "natural" implementation (all active colors should have at least 2 values)
	//note that not all colors in colors_state_active are inside the range of valid iop.NB_OF_COLORS
	int col=iop.node_iset_no[bbsg.msbn64()], cmin=iop.node_iset_no[bbsg.lsbn64()], min_size=m_size+1, v_cand=EMPTY_ELEM;
	l_v.index=EMPTY_ELEM;
	for(int c=col; c>=cmin; c--){
		if( min_size>iop.m_colSets[c].size && iop.color_state_active.is_bit(c) ){
			col=c;
			if( (min_size=iop.m_colSets[c].size) ==2 ) 
					break;
		}
	}
	//v_cand=FIRST_SHARED(iop.m_colSets[col].bb,iop.node_state_active);
	v_cand=iop.m_colSets[col].bb.lsbn64();										/* any node is good as input to RANGES */	
	
	//determine candidates for chosen color set (taking as reference one of its active values v_cand)
	int offset=RANGES[v_cand].vl-1;
	int max_val=RANGES[v_cand].vh;
	bbsg.init_scan_from((offset<=0)? -1 : offset, bbo::NON_DESTRUCTIVE);			/* scans excluding 'from' vertex */
	while(true){
		if( (v_cand=bbsg.next_bit())>max_val || (v_cand==EMPTY_ELEM) ) break;
		l_v.nodos[++l_v.index]=v_cand;
	}

	//if(m_mode==RLF){
	//	std::reverse(l_v.nodos,l_v.nodos+l_v.index);    /* for rlf sorting-TODO */
	//}

	/*more_restricting_val mrv_pred(*g,bbsg);
	sort(l_v.nodos, l_v.nodos+l_v.index, mrv_pred);*/

#ifdef LEAST_RESTRICTING_VAL
	less_restricting_val lrv_pred(*g,bbsg);
	sort(l_v.nodos, l_v.nodos+l_v.index, lrv_pred);

	#ifdef SUBSUME_FILTER
		int nb_f=filter_subsume(l_v,bbsg);
		/*if(nb_f>1){
			LOG_INFO("filtered: "<<nb_f<<" vertices");
			cin.get();
		}*/
	#endif

#else
	more_restricting_val mrv_pred(*g,bbsg);
	sort(l_v.nodos, l_v.nodos+l_v.index, mrv_pred);
	
#endif
	return l_v.index+1;
}


inline
int CliqueCSP_Plus::minimum_domain_variable_l2h(nodelist_t& l_v,bitarray& bbsg){
/////////////////
// Decision heuristics which computes the smallest possible list of candidates 
//
// PARAMS: bbsg->current subgraph, l_v->set of candidates to return
//
// RETURNS domain size (branching factor for bbsg subgraph), l_v (set of candidates in bbsg)
//
// COMMENTS: 
// 1.called at such a point that there should not be any unit variable to choose from
// 2.bb should not be empty


	//int v, v_cand=bbsg.lsbn64();
	//l_v.index=EMPTY_ELEM;
	//
	//int iset_max=iop.node_iset_no[bbsg.lsbn64()];

	//int iset=iop.node_iset_no[v_cand], min_size=iop.m_colSets[iset].size, min_var=iset;
	//if(min_size>=3){
	//	for(int i=min_var; i<=iset_max; i++){
	////	for(int i=min_var; i<=iop.NB_OF_COLORS; i++){
	//		if(min_size>iop.m_colSets[i].size  && iop.color_state_active.is_bit(i)){
	//			min_size=iop.m_colSets[i].size;
	//			min_var=i;
	//			if(min_size==2)
	//				break;

	//		}
	//	}
	//	v_cand=FIRST_SHARED(iop.m_colSets[min_var].bb,iop.node_state_active);
	//}


	//a more "natural" implementation (all active colors should have at least 2 values)
	//note that not all colors in colors_state_active are inside the range of valid iop.NB_OF_COLORS
	int col=iop.node_iset_no[bbsg.lsbn64()], cmax=iop.node_iset_no[bbsg.msbn64()], min_size=m_size+1, v_cand=EMPTY_ELEM;
	l_v.index=EMPTY_ELEM;
	for(int c=col; c<=cmax; c++){
		if(min_size>iop.m_colSets[c].size && iop.color_state_active.is_bit(c)  ){
			col=c;
			if( (min_size=iop.m_colSets[c].size) ==2 ) 
					break;
		}
	}
	//v_cand=FIRST_SHARED(iop.m_colSets[col].bb,iop.node_state_active);
	v_cand=iop.m_colSets[col].bb.lsbn64();										/* any node is good as input to RANGES */	
	
		
	//determine candidates for chosen color set (taking as reference one of its active values v_cand)
	int offset=RANGES[v_cand].vl-1;
	int max_val=RANGES[v_cand].vh;
	bbsg.init_scan_from((offset<=0)? -1 : offset, bbo::NON_DESTRUCTIVE);			/* scans excluding 'from' vertex */
	while(true){
		if( (v_cand=bbsg.next_bit())>max_val || (v_cand==EMPTY_ELEM) ) break;
		l_v.nodos[++l_v.index]=v_cand;
	}

	//if(m_mode==RLF)
	//	std::reverse(l_v.nodos,l_v.nodos+l_v.index);    /* for rlf sorting-TODO */
	
#ifdef LEAST_RESTRICTING_VAL 
	less_restricting_val lrv_pred(*g,bbsg);
	sort(l_v.nodos, l_v.nodos+l_v.index, lrv_pred);


	#ifdef SUBSUME_FILTER
	int nb_f=filter_subsume(l_v,bbsg);
	/*if(nb_f>1){
		LOG_INFO("filtered: "<<nb_f<<" vertices");
		cin.get();
	}*/
	#endif


#else
	more_restricting_val mrv_pred(*g,bbsg);
	sort(l_v.nodos, l_v.nodos+l_v.index, mrv_pred);


	;

#endif

	return l_v.index+1;
}

inline
ostream& CliqueCSP_Plus::print_ranges (ostream& o){
	for(vector<range_t>::iterator it=RANGES.begin(); it!=RANGES.end(); it++){
		it->print(o);
	}
	o<<endl;
	return o;
}

inline
void CliqueCSP_Plus::set_ranges(){
///////////////////
// Sets RANGES from DOMAINS info (RANGES[NODE]={vl, vh}, where vl and vh mark the vertex limits of 
// the NODE root color independent set)
//
// Comments: DOMAINS must be defined

	int v_first=0, v_last;
	RANGES.assign(m_size, range_t());
	for(vint::iterator it=DOMAINS.begin()+1; it!=DOMAINS.end(); it++){
		v_last=v_first+*it-1;
		for(int v=v_first; v<=v_last; v++){
			RANGES[v].vl=v_first;
			RANGES[v].vh=v_last;
		}
		v_first=v_last+1;
	}
}

inline
ostream& CliqueCSP_Plus::print_csp_info(ostream& o){
	o<<"[VAR:"<<m_size<<",V:"<<NB_CSP_VAR<<",i_val:"<<NB_INC_VAL<<"]"<<endl;
	com::stl::print_collection(DOMAINS, o);
	return o;
}

inline
int CliqueCSP_Plus::sum_of_dom_val(){
///////////////////
// values accounted for in domain specification
//
// COMMENTS: do not necessarily have to be the total number of vertices

	int res=0;
	for(vint::iterator it=DOMAINS.begin()+1; it!=DOMAINS.end(); it++){
		res+=(*it);
	}

	return res;
}

inline 
bool CliqueCSP_Plus::check_for_unit_variables (bitarray& bbsg){
////////////
// RETURNS true if at least one variable in the subgraph has a single value
//
// COMMENTS: MUST be called during search not pre-processing

	bbsg.init_scan(bbo::NON_DESTRUCTIVE);
	while(true){
		int v=bbsg.next_bit();
		if(v==EMPTY_ELEM) break;
		int res=bbsg.is_singleton(RANGES[v].vl, RANGES[v].vh);
		if(res==0){
			LOG_ERROR("CliqueCSP_Plus::check_for_unit_variables-empty variable");
			return  true;
		}else if(res==1){
			LOG_INFO(" CliqueCSP_Plus::check_for_unit_variables -unit color detected");
			return true;
		}
	}
	return false;
}

inline 
bool CliqueCSP_Plus::check_for_unit_variables (bitarray& bbsg, sbb_t<bitarray>& unit_stack){
////////////
// RETURNS true if at least one variable in the subgraph with a single value is NOT in unit_stack
//         Also if one varaible has no color
//
// COMMENTS: MUST be called during search not pre-processing

	bbsg.init_scan(bbo::NON_DESTRUCTIVE);
	while(true){
		int v=bbsg.next_bit();
		if(v==EMPTY_ELEM) break;
		int res=bbsg.is_singleton(RANGES[v].vl, RANGES[v].vh);
		if(res==0){
			LOG_ERROR("CliqueCSP_Plus::check_for_unit_variables-empty variable");
			return  true;
		}else if(res==1 && !unit_stack.bb.is_bit(v)){
			LOG_INFO(" CliqueCSP_Plus::check_for_unit_variables -unit color detected not un UNIT STACK");
			return true;
		}
	}
	return false;
}

inline
bool CliqueCSP_Plus::check_domains (){
/////////////////////
// Checks file csp domain specification. 
// 
// RETURNS TRUE if every variable is indeed a
// feasible color set over the range of values 
// declared in the specification file
		
	if(DOMAINS.empty()){
		LOG_ERROR("InitColorCSPSort::check_domains ()-over empty domain");
		return false;
	}
	bitarray bb(m_size);
		
	int offset=0;
	for(int var=1; var<DOMAINS.size(); var++){
		bb.erase_bit();
		bb.set_bit(offset, offset+DOMAINS[var]-1);
		offset+=DOMAINS[var];
		if(gfunc::is_iset(*g, bb)==false) return false;
	}

	return true;
}

inline
int CliqueCSP_Plus::filter_unit_root(){
/////////////////
// Uses domain information to make variables with single domain at root as a necessary part of
// the solution.
// (Use in CSP problems with partial assignments-i.e. Sudoku with X numbers on the board)
// date: 6/3/17
// author: pss
//
//  RETURNS number of unit variables
//
//  COMMENTS: 
//  I. to be called after root subgraph information (m_lroot and nodelist) is updated
//  II.does not analyse changes for further unit variables-simple filtering

	int nb_unit_var=0;
	bool is_change=false;
	int vertex=0;
	m_cpath[0].erase();

	//check for early inconsistency of unit domains
	if(nb_unit_var){
		LOG_INFO("nb_unit_val preproc: "<<nb_unit_var);
		if(!gfunc::is_clique(*g, m_cpath[0].stack, m_cpath[0].size())){
			LOG_INFO("TRIVIAL UNSAT FOUND DURING PREPROC");
			cin.get();
		}
	}
	
	//domains
	for (int i=1; i<DOMAINS.size(); i++){
		if (DOMAINS[i]==1){
			nb_unit_var++;
			is_change=true;
			m_bbroot&=g->get_neighbors(vertex);			/* filters incompatible values with singleton vertex */
			m_cpath[0].push(vertex);
	//		cout<<"["<<vertex<<"] ";
		}
		vertex+=DOMAINS[i];								/* determines offset */
	}
	
	//update nodelist at root
	/*if (is_change){
		int v=EMPTY_ELEM;
		m_lroot.index=-1;
		m_bbroot.init_scan(bbo::NON_DESTRUCTIVE);
		while(true){
			if( (v=m_bbroot.next_bit())== EMPTY_ELEM)
				break;

			m_lroot.nodos[++m_lroot.index]=v;
		}
	}*/
		
	return nb_unit_var;
}

inline
void CliqueCSP_Plus::clear_others(){
	/*if(maxac){
		delete [] maxac;
		maxac=NULL;
	}*/

	if(m_cpath){
		delete [] m_cpath;
		m_cpath=NULL;
	}

	iop.clear();
}

inline
int CliqueCSP_Plus::init_others(){
///////////////
// 
	Clique<ugraph>::init_others();		//m_sel and m_unsel
	clear_others();

	try{
		m_cpath=new com::stack_t<int>[NB_CSP_VAR];
		for(int i=0; i<NB_CSP_VAR; i++){
			m_cpath[i].init(NB_CSP_VAR);
		}

		m_candidates.init(NB_CSP_VAR);

		//pmax-sat
		iop.set_graph(g);
		if(iop.init(m_alloc+1)==-1){
			runtime_error r("CliqueCSP_Plus::init_others()-error allocating infra-chrom ColorSets");		//***check this exception is caught below
			throw r;
		}

		m_color_unit_stack.init(g->number_of_vertices());
		m_subset.init(m_size /* max nb of var with any number of values */);	

	}
	catch(exception& e){
		throw;
		return -1;
	}
			
	////initializes at 0
	//for(int i=0; i<NB_CSP_VAR; i++){
	//	maxac[i]=0;
	//}
	return 0;
}

inline
int CliqueCSP_Plus::set_up_non_unrolled	(search_alloc_t info){
////////////////////////
// non unrolled setup configuration for CSP constraint graphs
// 1-memory allocation
// 2-initial lower bound computation based on kcore
// 3-initial upperbound computation based on kcore
//
// RETURNS: size of max_clique, 0 if not found, -1 if error
//
//** TODO- nb_inc_val now disabled: shoulbd be enabled again for CSP ordering **/

	
	PrecisionTimer pt;
	int nb_inc_val=0;							/* currently always 0 */
	LOG_PRINT("INIT SETUP NON_UNROLLED CSP");
	int ub=param.ub; 
	int lb=param.lb;
	m_mode=DEFAULT;	

//////////////////////
//initial sorting: reordering of adjacency matrix following degeneracy
	bool is_csp_unsat=false;
	CliqueSort<ugraph> o(*g);
	bool rlf_not_valid=false;

	m_f.set_graph(g);
	if(param.init_order==CSP_FILTER){
		m_f.set_mode(FilterCSP_Plus::CSP_BASE );								/*	CSP_BASE-default value  */									
		m_f.init();
	}else if(param.init_order==CSP_RLF){
		m_f.set_mode(FilterCSP_Plus::CSP_BASE /* do not change */);				/*	CSP_BASE-default value  */									
		m_f.init_ext();
	}else if(param.init_order==CSP){
		m_f.set_mode(FilterCSP_Plus::CSP_BASE /* do not change  */);			/*	CSP_BASE-default value  */									
		m_f.init_ref();
	}

//////////////////
//UNSAT-TRIVIAL (exit)
	if(m_f.is_csp_unsat()){
		if(param.init_order==CSP_FILTER){
			int num_var=m_f.number_of_var();
			return (num_var!=-1)? num_var:1;	
		}else{
			if(param.init_order==CSP_RLF)
				return m_f.get_rlf_domains().front();
			else return m_f.get_csp_domains().front();		
		}
	}
///////////////////
//RLF VALID
	if(m_f.is_rlf_ord_valid()){
		//if(param.init_order==CSP_RLF ){	m_f.init_ref(); }
		this->DOMAINS=m_f.get_rlf_domains();
		if(o.reorder(m_f.get_rlf_ordering(), get_decoder())==-1){
			LOG_ERROR("set_up_unrolled: error during RLF-reordering");
			return -1;
		}
		m_mode=RLF;
		LOG_INFO("Using RLF coloring");
	}else{
//////////////////
// CSP VALID
		if(param.init_order==CSP_RLF){	m_f.init_ref(); }
		this->DOMAINS=m_f.get_csp_domains();
		if(o.reorder(m_f.get_csp_ordering(), get_decoder())==-1){					/* reorders graph */
			LOG_ERROR("CliqueCSP_Plus::set_up_non_unrolled(): error during CSP-reordering");
			return -1;
		}
		m_mode=DEFAULT;
		LOG_INFO("Using CSP_BASE coloring");
	}
				
//////////////////////////////
//DOMAIN-PREPROCESSING: nb_inc_val currently not used (set to 0) 
	if(!check_domains()){
		LOG_ERROR("CliqueCSP_Plus::set_up_non_unrolled-INCONSISTENT DOMAINS");
	}
	set_ranges();							/* fills RANGES[NODE]={vl, vh} */
	/*print_ranges(); cin.get();*/
	LOG_INFO("[Var:"<<this->DOMAINS[0]<<",NV:"<<m_size<<",d_val:"<<sum_of_dom_val()<<",inc_val:"<<nb_inc_val<<"]");
//////////////////////////////
					
	//initial bounds
	NB_CSP_VAR=this->DOMAINS[0];
	NB_INC_VAL=nb_inc_val;
	lb=NB_CSP_VAR-1; ub=NB_CSP_VAR;
	LOG_INFO("w:["<<lb<<","<<ub<<"]");
			
	//allocation
	info.size=NB_CSP_VAR;
	if(search_allocation(info)==-1) return -1;
	maxno=lb;
	res.set_LB(lb);
		
	//update csp specification data
	int offset=0, color=1, v=0;
	for(vint::iterator it=this->DOMAINS.begin()+1; it!=this->DOMAINS.end(); it++){
		offset+=(*it);
		for(; v<offset; v++){
			m_lcol[0][v]=color;
		}
		color++;
	}
	
	//check color=NB_CSP_VAR
	if(color!=NB_CSP_VAR+1){
		LOG_ERROR("CliqueCSP_Plus::set_up_non_unrolled-bizarre nb of CSP_VAR");
	}

	//remove inconsistent values found during pre-processing from root subgraph to search
	//(and placed at the end in the new ordering)
	
	if(nb_inc_val>0){
		LOG_PRINT("CliqueCSP_Plus::set_up_non_unrolled-inconsistent values found during preproc: "<<nb_inc_val);

		//modify initial set of vertices
		int v=g->number_of_vertices()-1;
		for(int i=0; i<nb_inc_val; i++){
			m_bbroot.erase_bit(v--);
		}
		//m_lroot.index-=nb_inc_val;
	}


	int nb_unit_var=filter_unit_root();								/* deals with unit values of variables-partial CSP assignments given */
	//maxac[0]+=nb_unit_var;										/* updates size of current clique (at root) */
	
	//determines root candidate list: first domain non-unit
	int v_last=m_bbroot.msbn64();
	int col=m_lcol[0][v_last];
	m_bbroot.init_scan(bbo::NON_DESTRUCTIVE_REVERSE);
	m_lroot.index=-1;
	vint last_color;
	while(true){
		int v=m_bbroot.previous_bit();
		if(v==EMPTY_ELEM || m_lcol[0][v]!=col) break;
		last_color.push_back(v);
	}

	for(vint::reverse_iterator it=last_color.rbegin(); it!=last_color.rend(); it++)
		m_lroot.nodos[++m_lroot.index]=*it;
	

	return 0;					//upper bound
}

inline
void CliqueCSP_Plus::clear_color_sets (){
	/*if(gc){
		gc->clear();
	}
	gc=NULL;*/
}

inline
int CliqueCSP_Plus::init_color_sets(){

	clear_color_sets();
	
	//try{
	//	
	//	//allocate complement graph
	//	gc=new ugraph(1);
	//	g->create_complement(*gc);	

	//}catch(exception& e){
	//	throw;
	//}

	return 0;
}

inline
int CliqueCSP_Plus::set_up(){
//////////////
// allocates memory, evaluates initial bounds and determines 
// initial trivial solutions
//
// RETURN VALUE: -1 Error, 0-ok, >0 trivial solution found

	int sol=0;
	res.clear();
	res.set_name(g->get_name());
	
	//determine allocation info
	search_alloc_t info;							//loads BBMC configuration for search allocation
	info.set(search_alloc_t::ALLOC_COLOR_SETS);		//for recoloring
	switch(param.alg){
	case BBMC_CSP:
	case BBMC_CSP_SUBSET:
		
	break;
	default:
		LOG_ERROR("CliqueCSP_Plus::setup unknown algorithm");
		return -1;
	}
	
	//actual set_up
	if(param.unrolled){
		LOG_ERROR("CliqueCSP_Plus::setup unrolled variant not defined");
		return -1;
	}else{
		if( (sol=set_up_non_unrolled(info))>0 ){
				res.set_UB(sol);
		}else{//Trivial solution not found 
						
			//UB: output to screen (TODO-LOGGER)
			stringstream sstr;
			sstr<<"ROOT COLORING"<<endl;
			for(int i=0; i<m_size-NB_INC_VAL; i++){
				sstr<<m_lcol[0][i]<<" ";
			}
			sstr<<endl;
			sstr<<"[inc_val:"<<NB_INC_VAL<<"]"<<endl;
			LOG_INFO(sstr.str());

			sstr.str(""); sstr.clear();
			sstr<<"ROOT CANDIDATE SET: ";
			m_lroot.print(sstr,true);
			LOG_INFO(sstr.str());
			
									
			//LB: using amts heuristic
			if(param.init_preproc!=UB){
				LOG_PRINT("COMPUTING LB with AMTS");
				//****TODO: place in setup and check for TRIVIAL SOLUTION
				AMTSexec a(RESTARTS, ITERATIONS_PER_RESTART);
				int lb_amts=a.run(*g, true /* store sol */);
				if(lb_amts==NB_CSP_VAR){
					LOG_PRINT("Solved by AMTS");
					sol=lb_amts;
					res.set_UB(lb_amts);
					res.add_solution(a.get_nodes());		/* stores solution */
				}
			}
		}
	}	
	
	//trivial solution
	if(sol>0){
		LOG_INFO("[w="<<sol<<"]"<<" TRIVIALLY SOLVED DURING PRECOMPUTATION");
		stringstream sstr;
		com::stl::print_collection(res.get_first_solution(),sstr);
		LOG_INFO(sstr.str());
	}
		
	return sol;
}

inline
void CliqueCSP_Plus::run(){
	//algorithm
	if(param.unrolled){
		LOG_ERROR("CliqueCSP_Plus::run: unrolling not defined for this familiy of algorithms");
		return;
	}else{
		res.tic();
		switch(param.alg){
		case BBMC_CSP:
			expand_csp((m_cpath[0].size()>0)? 1:0, m_cpath[0].size(), Clique<ugraph>::m_bbroot /*, Clique<ugraph>::m_lroot*/);
			break;
		case BBMC_CSP_SUBSET:
			expand_csp_subset((m_cpath[0].size()>0)? 1:0, m_cpath[0].size(), Clique<ugraph>::m_bbroot /*, Clique<ugraph>::m_lroot*/);
			break;
		default:
			LOG_ERROR("CliqueCSP_Plus::run-non_unrolled:unknown clique algorithm");
		}
		res.toc();
	}

	//read solution
	res.set_UB(maxno);
	LOG_INFO("[w:"<<res.get_upper_bound()<<","<<res.get_user_time()<<"s]");
}	

inline
bool CliqueCSP_Plus::expand_csp(int depth, int maxac, bitarray& l_bb /*, nodelist_t& l_v*/){
////////////////////////
// p-max-sat driver which works directly on the root subproblem
//
// date: 18/3/17

	int v;
	res.inc_number_of_steps();

/////////////////////////////////
// CHECK IF SOLUTION FOUND

	if(maxac>maxno){ /* solution found */
		maxno=maxac;						//NEW GLOBAL OPTIMUM FOUND: MUST BE NB_CSP_VAR

#ifdef STORE_SOLUTION
		res.set_UB(maxno);
		res.clear_all_solutions();

		//generate solution: optimize
		/*m_cpath[depth].erase();
		m_cpath[depth].push(v);*/
		int path_pt=0;
		for(int d=0; d<depth; d++){
			for(int j=0; j<m_cpath[d].size(); j++){
				m_path[path_pt++]=m_cpath[d].stack[j];
			}
		}
		res.add_solution(maxno, m_path);
		if(!gfunc::is_clique(*g, m_path, maxno)){
			LOG_ERROR("BIZARRE SOL");
		}

#ifdef VIEW_PROGRESS
		stringstream sstr("");
		res.print_first_sol(sstr);
		LOG_INFO(sstr.str());
#endif

#endif
		//CSP-SAT found-END-SEARCH
		return true;
	}
///////////////////////////
//  iseq incremental filter

	ret_t res=iseq(depth, maxno-maxac, l_bb);
	if(res==PRUNE){
		return false;
	}else if(res==TRIVIAL_SOL){
		if(l_bb.is_empty()){
			LOG_ERROR("CliqueCSP_Plus::expand_csp_pms_from_root_def-bizarre trivial sol, subgraph empty");
			cin.get();
		}
		LOG_INFO("TRIVIAL SOL");
		maxno=maxac+1;				/* non-empty l_bb with maxno==maxac */
		return true;
	}

	/////////////////////////////
	// r_iseq incremental filter	

	if(r_iseq(depth, maxno-maxac, l_bb)==PRUNE){					/* can update m_color_unit_stack */
		//	LOG_INFO("RISEQ CUT:"<<v<<" d:"<<depth);
		return false;
	}

	/////////////////////////////
	// pmax-sat incremental filter
	m_color_unit_stack.erase();
	//m_candidates.erase();
	int nb= iop.init_inc_maxsatz_csp();	
	if((nb>0) && iop.hard_unit_iset_process(l_bb, m_color_unit_stack)>0){			
		//LOG_INFO("HARD UNIT ISET CUT");
		//iop.print_stack(InfraOpPlus<ugraph, bitarray>::NODES);
		return false;
	}

	if(/*(nb>0) &&*/ iop.inc_maxsatz_lookahead_csp(l_bb, m_color_unit_stack)==true){
		//LOG_INFO("inc_maxsatz_lookahead CUT");
		return false;
	}

	//if(iop.test_by_eliminate_failed_nodes_csp(l_bb, m_color_unit_stack, m_candidates)==true){
	if(iop.test_by_eliminate_failed_nodes_csp(l_bb, m_color_unit_stack)==true){
		//LOG_INFO("test_by_eliminate_failed_nodes CUT");
		return false;
	}

	//if(check_for_unit_variables(l_bb, m_color_unit_stack)){
	//	LOG_ERROR("UNIT VAR NOT UN COLOR UNIT STACK");
	//	cin.get();
	//}

	//expand known unit variables
	if(!m_color_unit_stack.is_empty()){
		/*if(!gfunc::is_clique(*g, m_color_unit_stack.stack, m_color_unit_stack.size)){
		LOG_INFO("UNIT COLOR NOT CLIQUE");
		cin.get();
		return false;
		}*/

		/* delete unit colors from subgraph (before determining nodes*/
		delete_unit_color_stack(l_bb);	
		if(l_bb.is_empty()){
			//LOG_INFO("ALL COLORS LEFT ARE UNIT COLORS-TRIVIAL SOLUTION!");
			maxno=maxac+m_color_unit_stack.get_size();
			return true;		/* TODO-change to an enum - SOLUTION FOUND */
		}
	}
	
/////////////////////////////////
// determine candidate var/values
	//minimum_domain_variable_h2l(LISTA_L(depth), l_bb);


	if(depth>=0){
		if(depth%2==0)
			minimum_domain_variable_h2l(LISTA_L(depth), l_bb);
		else minimum_domain_variable_l2h(LISTA_L(depth), l_bb);
	}else minimum_domain_variable_h2l(LISTA_L(depth), l_bb);

	

	/*if(depth>=1)
		from_candidate_set(LISTA_L(depth), l_bb);
	else minimum_domain_variable_h2l(LISTA_L(depth), l_bb);*/
	

	//minimum_domain_variable_l2h(LISTA_L(depth), l_bb);
	//minimum_domain_variable_h2l(LISTA_L(depth), l_bb);
	//highest_variable(LISTA_L(depth), l_bb);
	//lowest_variable(LISTA_L(depth), l_bb);

		
/////////////////////////////////
// expand
	int clq_size=m_color_unit_stack.get_size()+maxac+1;
	while(LISTA_L(depth).index>=0){
		v=LISTA_L(depth).nodos[LISTA_L(depth).index--];
		AND(g->get_neighbors(v), l_bb, LISTA_BB(depth));
		//if(clq_size + LISTA_BB(depth).popcn64() < maxno){ 
		////if(LISTA_BB(depth).is_empty() && (clq_size < maxno)  ){	
		//	LOG_INFO("CUTTING");
		//	l_bb.erase_bit(v);
		//	continue;
		//}		
		m_cpath[depth].push(v);
		
		//if(m_color_unit_stack.size>0){
		//	m_color_unit_stack.print();
		//	//cin.get();
		//	for(int i=0; i<m_color_unit_stack.size; i++){
		//		m_cpath[depth].push(m_color_unit_stack.stack[i]);
		//	}
		//}
		if(expand_csp(depth+1, /*maxac+1*/ clq_size, LISTA_BB(depth) /*,LISTA_L(depth)*/)==true) 
				return true;
		else{
			l_bb.erase_bit(v);					/* actually unnecessary since all the other candidates form an independent set */
		}
		m_cpath[depth].pt--;					//removes candidate


		//if(m_color_unit_stack.size>0){
		//	m_cpath[depth].pt-=m_color_unit_stack.size;
		//}
	}

	return false;
}

inline
bool CliqueCSP_Plus::expand_csp_subset (int depth, int maxac, bitarray& l_bb /*, nodelist_t& l_v*/){
////////////////////////
// p-max-sat driver which works directly on the root subproblem
//
// date: 18/3/17
	
	int v;
	res.inc_number_of_steps();

/////////////////////////////////
// CHECK IF SOLUTION FOUND

	if(maxac>maxno){ /* solution found */
		maxno=maxac;						//NEW GLOBAL OPTIMUM FOUND: MUST BE NB_CSP_VAR

#ifdef STORE_SOLUTION
		res.set_UB(maxno);
		res.clear_all_solutions();

		//generate solution: optimize
		/*m_cpath[depth].erase();
		m_cpath[depth].push(v);*/
		int path_pt=0;
		for(int d=0; d<depth; d++){
			for(int j=0; j<m_cpath[d].size(); j++){
				m_path[path_pt++]=m_cpath[d].stack[j];
			}
		}
		res.add_solution(maxno, m_path);
		if(!gfunc::is_clique(*g, m_path, maxno)){
			LOG_ERROR("BIZARRE SOL");
		}

#ifdef VIEW_PROGRESS
		stringstream sstr("");
		res.print_first_sol(sstr);
		LOG_INFO(sstr.str());
#endif

#endif
		//CSP-SAT found-END-SEARCH
		return true;
	}
///////////////////////
// NEW TEST-filterCSP

#ifdef CSP_FILTER_ON
	if(m_f.is_filter_active()){
		lnodes_f.clear();			//not necessary
		int fres=m_f.filter_driver(depth, maxno-maxac,l_bb,lnodes_f,true);
		if(fres==-1){
			//	LOG_INFO("PRUNING WITH FILTER CSP");
			return false;
		}

#ifdef CSP_FILTER_SEARCH_INFO
		if(!lnodes_f.empty()){
			com::stl::print_collection(lnodes_f); 
			cout<<"["<<depth<<"]"<<endl;
		}
#endif

		for(int i=0; i<lnodes_f.size(); i++){
			l_bb.erase_bit(lnodes_f[i]);
		}

		lnodes_f.clear();			//not necessary
		fres=m_f.filter_driver(depth, maxno-maxac,l_bb,lnodes_f,false);
		if(fres==-1){
			//	LOG_INFO("PRUNING WITH FILTER CSP IN REVERSE");
			return false;
		}

#ifdef CSP_FILTER_SEARCH_INFO
		if(!lnodes_f.empty()){
			com::stl::print_collection(lnodes_f);
			cout<<endl;
		}
#endif

		for(int i=0; i<lnodes_f.size(); i++){
			l_bb.erase_bit(lnodes_f[i]);
		}
		//LOG_INFO("STOPPING");
		//cin.get();


		//	LOG_INFO("------------------------------------");
		//	cin.get();
	}
#endif

//////////////////////////

///////////////////////////
//  iseq incremental filter

	ret_t res=iseq_subset(depth, maxno-maxac, l_bb);
//	iop.print_db();
	//if(iop.NB_OF_COLORS!=maxno-maxac+1){
	//	LOG_ERROR("CliqueCSP_Plus::TOO MANY COLORS");
	//	cin.get();
	//	return false;
	//}
	if(res==PRUNE){
		return false;
	}else if(res==TRIVIAL_SOL){
		if(l_bb.is_empty()){
			LOG_ERROR("CliqueCSP_Plus::expand_csp_pms_from_root_def-bizarre trivial sol, subgraph empty");
			cin.get();
		}
		LOG_INFO("TRIVIAL SOL");
		maxno=maxac+1;				/* non-empty l_bb with maxno==maxac */
		return true;
	}
	
	/////////////////////////////
	// r_iseq incremental filter	
	if(r_iseq(depth, maxno-maxac, l_bb)==PRUNE){					/* can update m_color_unit_stack */
		//	LOG_INFO("RISEQ CUT:"<<v<<" d:"<<depth);
		return false;
	}

	/////////////////////////////
	// pmax-sat incremental filter

	m_color_unit_stack.erase();
	//m_candidates.erase();
	int nb= iop.init_inc_maxsatz_csp();	
	if((nb>0) && iop.hard_unit_iset_process(l_bb, m_color_unit_stack)>0){			
		//LOG_INFO("CliqueCSP_Plus::test_unit_set"<<"[d:"<<depth<<",s:"<<maxac<<"]");
		//iop.print_stack(InfraOpPlus<ugraph, bitarray>::NODES);
		return false;
	}		
			
	if(test_by_eliminate_failed_nodes_csp_subset(l_bb, m_color_unit_stack)==true){
		//LOG_INFO("CliqueCSP_Plus::test_by_eliminate_failed_nodes CUT"<<"[d:"<<depth<<",s:"<<maxac<<"]");
		return false;
	}

	if(/*(nb!=0) &&*/ inc_maxsatz_lookahead_csp_subset(l_bb, m_color_unit_stack)==true){
		//LOG_INFO("CliqueCSP_Plus::inc_maxsatz_lookahead CUT"<<"[d:"<<depth<<",s:"<<maxac<<"]");
		return false;
	}
	
	//if(check_for_unit_variables(l_bb, m_color_unit_stack)){
	//	LOG_ERROR("UNIT VAR NOT UN COLOR UNIT STACK");
	//	cin.get();
	//}

	//expand known unit variables
	if(!m_color_unit_stack.is_empty()){
		/*if(!gfunc::is_clique(*g, m_color_unit_stack.stack, m_color_unit_stack.size)){
		LOG_INFO("UNIT COLOR NOT CLIQUE");
		cin.get();
		return false;
		}*/

		/* delete unit colors from subgraph (before determining nodes*/
		delete_unit_color_stack(l_bb);	
		if(l_bb.is_empty()){
			//LOG_INFO("ALL COLORS LEFT ARE UNIT COLORS-TRIVIAL SOLUTION!");
			maxno=maxac+m_color_unit_stack.get_size();
			return true;		/* TODO-change to an enum - SOLUTION FOUND */
		}
	}
	
/////////////////////////////////
// determine candidate var/values

	if(m_mode==RLF){
		//minimum_domain_variable_h2l(LISTA_L(depth), l_bb);
		if(depth%2==0)
			minimum_domain_variable_h2l(LISTA_L(depth), l_bb);
		else minimum_domain_variable_l2h(LISTA_L(depth), l_bb);
	}else if(depth>=0){
		if(depth%2==0)
			minimum_domain_variable_h2l(LISTA_L(depth), l_bb);
		else minimum_domain_variable_l2h(LISTA_L(depth), l_bb);
	}else minimum_domain_variable_h2l(LISTA_L(depth), l_bb);
	
	/*if(m_mode==RLF)
		if(depth%2==0)
			minimum_domain_variable_h2l(LISTA_L(depth), l_bb);
		else minimum_domain_variable_l2h(LISTA_L(depth), l_bb);
	else{

		if(depth>=0){
			if(depth%2==0)
				minimum_domain_variable_h2l(LISTA_L(depth), l_bb);
			else minimum_domain_variable_l2h(LISTA_L(depth), l_bb);
		}else minimum_domain_variable_h2l(LISTA_L(depth), l_bb);
	}*/

	

	/*if(depth>=1)
		from_candidate_set(LISTA_L(depth), l_bb);
	else minimum_domain_variable_h2l(LISTA_L(depth), l_bb);*/
	

	//minimum_domain_variable_l2h(LISTA_L(depth), l_bb);
	//minimum_domain_variable_h2l(LISTA_L(depth), l_bb);
	//highest_variable(LISTA_L(depth), l_bb);
	//lowest_variable(LISTA_L(depth), l_bb);

		
/////////////////////////////////
// expand
	int clq_size=m_color_unit_stack.get_size()+maxac+1;
	while(LISTA_L(depth).index>=0){
		v=LISTA_L(depth).nodos[LISTA_L(depth).index--];

		//test if adjacent to unit_color_stack
		/*for(int i=0; i<m_color_unit_stack.size; i++){
			if(!g->is_edge(v,m_color_unit_stack.stack[i])){
				LOG_INFO("NON ADJACENT TO UNIT STACK");
				cin.get();
			}
		}*/
		
		AND(g->get_neighbors(v), l_bb, LISTA_BB(depth));
		//if(clq_size + LISTA_BB(depth).popcn64() < maxno){ 
		////if(LISTA_BB(depth).is_empty() && (clq_size < maxno)  ){	
		//	LOG_INFO("CUTTING");
		//	l_bb.erase_bit(v);
		//	continue;
		//}		
		m_cpath[depth].push(v);
		
		//if(m_color_unit_stack.size>0){
		//	m_color_unit_stack.print();
		//	//cin.get();
		//	for(int i=0; i<m_color_unit_stack.size; i++){
		//		m_cpath[depth].push(m_color_unit_stack.stack[i]);
		//	}
		//}
		if(expand_csp_subset(depth+1, /*maxac+1*/ clq_size, LISTA_BB(depth) /*,LISTA_L(depth)*/)==true) 
				return true;
		else{
			l_bb.erase_bit(v);					/* actually unnecessary since all the other candidates form an independent set */
		}
		m_cpath[depth].pt--;					//removes candidate


		//if(m_color_unit_stack.size>0){
		//	m_cpath[depth].pt-=m_color_unit_stack.size;
		//}
	}

	return false;
}



inline
CliqueCSP_Plus::ret_t CliqueCSP_Plus::r_iseq(int depth, int kmin,  bitarray& bbsg){
/////////////////////
// r_iseq (reversed iseq coloring) tuned for CSP
// same as r_iseq except that it doesn´t update unit color stack
//
// COMMENTS: 
// 1. uses iop.m_unsel so that it may be used inside iseq_pms
// 2. kmin should not be 0 here
// 3. currently using iop.m_unsel to store the initial set (cannot use the inherited one as in i_seq: check)
// 4. SHOULD BE CALLED AFTER iseq
// 
	
	bool first_color_vertex=true;
	int col=1, /*kmin=maxno-maxac,*/ nBB=EMPTY_ELEM, v=EMPTY_ELEM, v_col_ref=EMPTY_ELEM, offset=0 /*,  c_size=0 , nb_inc_v=0*/;		
	
	if(kmin<=1 /* 2 ? */){return OK;}										/* seems to help in some cases: too much effort with bad compromise with pruning effort: note KMIN should not be 0 here  */
	int pc=(iop.m_unsel=bbsg).popcn64();
	while(true){ 
		m_sel=iop.m_unsel;
		m_sel.init_scan(bbo::DESTRUCTIVE_REVERSE);
		first_color_vertex=true;
		while(true){
			if( (v=m_sel.previous_bit_del(nBB,iop.m_unsel)) == EMPTY_ELEM) break;
			if(first_color_vertex){
				first_color_vertex=false;
				v_col_ref=v;
				offset=RANGES[v_col_ref].vl-1;
			}else if(v<=offset){
				bbsg.erase_bit(v);
				iop.m_colSets[iop.node_iset_no[v]].erase_bit(v);
				iop.node_state_active.erase_bit(v);
				if(iop.m_colSets[iop.node_iset_no[v]].size==0){					/* occurs seldom but occurs */
					//LOG_INFO("UNIT COLOR FOUND R_ISEQ_PMS");		
					//cin.get();		
					//LISTA_L(depth).index=EMPTY_ELEM;
					return PRUNE;
				}
				if((--pc)==0){													/* CUT since the last color has no vertex */
					LISTA_L(depth).index=EMPTY_ELEM;
					return PRUNE;
				}
				continue;
			}
			//normal exit
			if((--pc)==0){
				if(col<=kmin){										
					LISTA_L(depth).index=EMPTY_ELEM;
					return PRUNE;
				}else{
					return OK;
				}
			}
			//color in reverse direction
			m_sel.erase_block(0,nBB,g->get_neighbors(v));
		} /* next v */	
		col++;
	} /* next color */

	return PRUNE;					//should not reach here
}


inline
CliqueCSP_Plus::ret_t CliqueCSP_Plus::iseq(int depth, int kmin , bitarray& bbsg){
//////////////////////////
// RETURNS 	{TRIVIAL_SOL, PRUNE, OK-Continues search}
//
// COMMENTS
// 1. Uses inherited m_unsel from Clique (also possible to use the iop one)

	int cmax=1, nBB=EMPTY_ELEM, v=EMPTY_ELEM, offset=0;
	bool first_color;

	// one 
	if(kmin== /*1*/ 0){
		/* only one candidate is enough*/
		//LOG_INFO("KMIN IS 1");
		//LISTA_L(depth).nodos[++LISTA_L(depth).index]=bbsg.msbn64();
		return TRIVIAL_SOL;
	}

	int pc=(m_unsel=bbsg).popcn64();
	if(pc==0){
		LOG_ERROR("EMPTY GRAPH ISEQ_NON_INCREMENTAL");
		return PRUNE;														/*empty subgraph-should not ocurr */	
	}
	
/////////////////////////////////////////////
//color first kmin nodes as usual

	iop.node_state_active.erase_bit();
	iop.NB_OF_COLORS=1;
	iop.m_colSets[iop.NB_OF_COLORS].erase_bit(false);						/* lazy erasing (size=0 but contents not destroyed); will be updated later*/	
	while(true){ 
		for(int i=0; i<iop.NB_OF_BB_NODES; i++){
			iop.m_colSets[iop.NB_OF_COLORS].bb.get_bitboard(i)=m_unsel.get_bitboard(i);
		}
						
		iop.m_colSets[iop.NB_OF_COLORS].bb.init_scan(bbo::NON_DESTRUCTIVE);
		first_color=true;
		while(true){
			v=iop.m_colSets[iop.NB_OF_COLORS].bb.next_bit(nBB,m_unsel);
			if(v==EMPTY_ELEM)
				break;
					
			//color range analysis: remove values outside of range
			if(first_color){
				first_color=false;
				offset=RANGES[v].vh + 1;	
			}else if(v>=offset){ // value removed
				bbsg.erase_bit(v);
				iop.m_colSets[iop.NB_OF_COLORS].bb.erase_bit(v);				/* (to be SAFE)-do not call the erase_bit interface */
				//int res=bbsg.is_singleton(RANGES[v].vl, RANGES[v].vh);
				//if(res==0){
				//	/*LOG_INFO("CUT iseq EMPTY DOMAIN");
				//	cin.get();*/
				//	return PRUNE;				
				//}

				if((--pc)==0){
					return PRUNE;
				}else{
					continue;	/* next vertex to be pruned */
				}
			}
			
			//stores color label
			iop.m_colSets[iop.NB_OF_COLORS].size++;								/* the node is already there, simply increment size: equivalent to push(v) */
			iop.node_iset_no[v]=iop.NB_OF_COLORS;
			iop.node_state_active.set_bit(v);
			
			//checks exit condition: cut
			if((--pc)==0){
				if(iop.NB_OF_COLORS<=kmin){
					/*LOG_INFO("LESS NUMBER OF COLORS");
					cin.get();*/
					return PRUNE;
				}else{
					return OK;											/* all nodes colored with <KMIN colors */
				}
			}

			//actual painting routine, equivalent to: m_colSets[col].bb.erase_block(nBB,g->get_neighbors(v));		
			bitarray& nbor=g->get_neighbors(v);
			for(int i=nBB; i<iop.NB_OF_BB_NODES; i++){
				iop.m_colSets[iop.NB_OF_COLORS].bb.get_bitboard(i)&= ~nbor.get_bitboard(i);
			}
		}/* next vertex of current color */

		iop.m_colSets[++iop.NB_OF_COLORS].erase_bit(false);					/* lazy erasing, will be updated later*/
	}/* next color */

	LOG_ERROR("CliqueCSP_Plus::iseq-should not reach here");
	return PRUNE;			/* should not reach here */
}

inline
CliqueCSP_Plus::ret_t CliqueCSP_Plus::iseq_subset(int depth, int kmin , bitarray& bbsg){
//////////////////////////
// Compared to reference non_subset function, color classes below a certain size are
// assigned to m_subset accoding to size. Filtering will take place on this fresh data structure
//
// RETURNS: RET_VAL-{TRIVIAL_SOL, PRUNE, OK-Continues search}
//
// COMMENTS
// 1. Uses inherited m_unsel from Clique (also possible to use the iop one)

	int cmax=1, nBB=EMPTY_ELEM, v=EMPTY_ELEM, offset=0;
	bool first_color;

	//cut
	if(kmin== /*1*/ 0){
		/* only one candidate is enough*/
		//LOG_INFO("KMIN IS 1");
		//LISTA_L(depth).nodos[++LISTA_L(depth).index]=bbsg.msbn64();
		return TRIVIAL_SOL;
	}

	int pc=(m_unsel=bbsg).popcn64();
	if(pc==0){
		LOG_ERROR("EMPTY GRAPH ISEQ_NON_INCREMENTAL");
		return PRUNE;														/*empty subgraph-should not ocurr */	
	}
	
/////////////////////////////////////////////
//color first kmin nodes as usual

	m_subset.erase();
	iop.node_state_active.erase_bit();
	iop.NB_OF_COLORS=1;
	iop.m_colSets[iop.NB_OF_COLORS].erase_bit(false);						/* lazy erasing (size=0 but contents not destroyed); will be updated later*/	
	while(true){ 
		for(int i=0; i<iop.NB_OF_BB_NODES; i++){
			iop.m_colSets[iop.NB_OF_COLORS].bb.get_bitboard(i)=m_unsel.get_bitboard(i);
		}
						
		iop.m_colSets[iop.NB_OF_COLORS].bb.init_scan(bbo::NON_DESTRUCTIVE);
		first_color=true;
		while(true){
			v=iop.m_colSets[iop.NB_OF_COLORS].bb.next_bit(nBB,m_unsel);
			if(v==EMPTY_ELEM)
				break;
					
			//color range analysis: remove values outside of range
			if(first_color){
				first_color=false;
				offset=RANGES[v].vh + 1;	
			}else if(v>=offset){ // value removed
				bbsg.erase_bit(v);
				iop.m_colSets[iop.NB_OF_COLORS].bb.erase_bit(v);				/* (to be SAFE)-do not call the erase_bit interface */
				//int res=bbsg.is_singleton(RANGES[v].vl, RANGES[v].vh);
				//if(res==0){
				//	/*LOG_INFO("CUT iseq EMPTY DOMAIN");
				//	cin.get();*/
				//	return PRUNE;				
				//}

				if((--pc)==0){
					return PRUNE;
				}else{
					continue;	/* next vertex to be pruned */
				}
			}
			
			//stores color label
			iop.m_colSets[iop.NB_OF_COLORS].size++;								/* the node is already there, simply increment size: equivalent to push(v) */
			iop.node_iset_no[v]=iop.NB_OF_COLORS;
			iop.node_state_active.set_bit(v);
			
			//checks exit condition: cut
			if((--pc)==0){
				if(iop.NB_OF_COLORS<=kmin){
					/*LOG_INFO("LESS NUMBER OF COLORS");
					cin.get();*/
					return PRUNE;
				}else{
					//update subset info
					if(iop.m_colSets[iop.NB_OF_COLORS].size<=MAX_SIZE_FOR_SETS_IN_SUBSET){
						m_subset.push(iop.m_colSets[iop.NB_OF_COLORS].size,iop.NB_OF_COLORS);
						/*if(nb_subset_col<15){
							m_subset.push_bb(iop.NB_OF_COLORS);
							nb_subset_col++;
						}*/
					}

					return OK;											/* all nodes colored with <KMIN colors */
				}
			}

			//actual painting routine, equivalent to: m_colSets[col].bb.erase_block(nBB,g->get_neighbors(v));		
			bitarray& nbor=g->get_neighbors(v);
			for(int i=nBB; i<iop.NB_OF_BB_NODES; i++){
				iop.m_colSets[iop.NB_OF_COLORS].bb.get_bitboard(i)&= ~nbor.get_bitboard(i);
			}
		}/* next node of current color  */

		//update subset info
		if(iop.m_colSets[iop.NB_OF_COLORS].size<=MAX_SIZE_FOR_SETS_IN_SUBSET){
			m_subset.push(iop.m_colSets[iop.NB_OF_COLORS].size,iop.NB_OF_COLORS);
			/*if(nb_subset_col<15){
			m_subset.push_bb(iop.NB_OF_COLORS);
			nb_subset_col++;
			}*/
		}

		iop.m_colSets[++iop.NB_OF_COLORS].erase_bit(false);					/* lazy erasing, will be updated later*/
	}/* next color */

	LOG_ERROR("CliqueCSP_Plus::iseq-should not reach here");
	return PRUNE;			/* should not reach here */

}

/////////////
//
// subset (implementation)
//
////////////

template<int N>
inline
int subset<N>::get_min_h2l(InfraOpPlus<ugraph,bitarray>& iop){
////////////////
// RETURNS the active variable with highest index from the variables with 
// smallest domain in the subset, or -1 if the subset is empty
	
	//for(int size=MAX_SIZE /* 0 or 1 slots should be empty */ ; size>=2; size--){
	for(int size=2 /* 0 or 1 slots should be empty */ ; size<=MAX_SIZE; size++){
		//for(int i=0; i<lss[size].pt; i++){
		for(int i=lss[size].pt-1; i>=0; i--){
			int iset=lss[size].get_elem(i);
			if(/*iop.m_colSets[iset].size == size  could have been simplified && */ iop.color_state_active.is_bit(iset))
				return iset;
			//LOG_INFO("var skipped from initial pool");
		}//next variable
	}//next size
	return EMPTY_ELEM;
}

template<int N>
inline
int subset<N>::get_min_l2h(InfraOpPlus<ugraph,bitarray>& iop){
////////////////
// RETURNS the active variable with lowest index from the variables with 
// smallest domains in the subset, or -1 if the subset is empty
	
	//for(int size=MAX_SIZE /* 0 or 1 slots should be empty */ ; size>=2; size--){
	for(int size=2 /* 0 or 1 slots should be empty */ ; size<=MAX_SIZE; size++){
		for(int i=0; i<lss[size].pt; i++){
		//for(int i=lss[size].pt-1; i>=0; i--){
			int iset=lss[size].get_elem(i);
			if( /*iop.m_colSets[iset].size == size  could have been simplified  &&*/ iop.color_state_active.is_bit(iset))
				return iset;
			//LOG_INFO("var skipped from initial pool");
		}//next variable
	}//next size
	return EMPTY_ELEM;
}

template<int N>
inline
ostream& subset<N>::print(ostream& o){
	for(int i=0; i<=MAX_SIZE; i++){
		o<<"[size:"<<i<<"]";
		o<<lss[i];
	}
	o<<endl;
	return o;
}

#endif


