//infra_tools_plus.h: Header for InfraOpPlus which optimizes InfraOp
//							***ADD REMARKS***	
//date of creation: 2/12/16

#ifndef __INFRA_TOOLS_PLUS_H__
#define	__INFRA_TOOLS_PLUS_H__


#include <strstream>
#include <string>
#include "clique_types.h"
#include "../init_color_ub.h"
#include "bitscan/bbalg.h"											//ADT-bb_t
#include "utils/common.h"

using namespace std;

////////////////
//**TODO
//switch #define USE_COMPLEMENT_GRAPH

template<class graph_t, class bitboard_t>
class InfraOpPlus{
///////////////
//data members
public:
	enum stack_t {NODES, COLOR_ACTIVE, COLOR_REDUCED, COLOR_PASSIVE, COLOR_INVOLVED, COLOR_REASON, FILTER};
	static const int MAX_NB_OF_CONFLICTS=-3;	
//protected:	
	static const int MAX_NUM_CONFLICTS=64;							//do NOT change!
	static const int NONE=-1;										//empty or without value
	static const int TRUE_VAL=1;
	static const int FALSE_VAL=0;
	static const int NO_CONFLICT=-2;								//alternative to an empty (conflicting) color set
	static const int NO_REASON=-3;									//for empty node reason state
	static const int MAX_COLOR_TEST_LENGTH=MAX_SIZE_FAILED_COLOR_SET;	
	static const int MAX_COLOR_FURTHER_TEST_LENGTH=2;
protected:
////////////////////
	graph_t* g;														//input graph
	graph_t* gc;													//complement graph (cache) 

public:
/////////
// new data structures for new infrachrom a la incMaxCLQ (3/8/16)
	int NB_OF_COLORS;
	int NB_OF_BB_NODES;							//number of bitblocks which encode normal nodes (not added)
	int NB_OF_NODES;
	int ADDED_NODES;							//offset is NV rounded to nearest bitblock-WMUL(NB_OF_BB_NODES)
	int NB_OF_BB_ADDED_NODES;					//number of bitblocks for added nodes (64 CONF. MAX so NB_OF_BB_NODES+1)
	int NB_OF_ADDED_NODES;						//WMUL(NB_OF_BB_ADDED_NODES)-1 (64 CONF. MAX)
	bb_t<bitboard_t>* m_colSets;				//capital letter 'S' to indicate that the ADT also contains the size of the color set

	com::stack_t<int> color_unit_stack;			//initial singleton color seeds for UL
	com::stack_t<int> color_unit_dyn_stack;		//future singleton color seeds during UL
	com::stack_t<int> color_passive_stack;		//contains passive colors
	com::stack_t<int> color_reduced_stack;		//contains reduced colors
	com::stack_t<int> color_enlarged_stack;		//contains enlarged colors
	com::stack_t<int> color_reason_stack;		//contains colors	
	com::stack_t<int> color_conflict_stack;		//contains (enlarged) colors of the same conflict (separated by NONE)

	com::stack_t<int> color_filter_stack;		//used by specific filter (one-shot) infra-chrom
	
	com::stack_t<int> node_stack;				//contains (passive) nodes set to FALSE/TRUE during inferences

	//lookahead stacks
	com::stack_t<int> node_tested_stack;		//nodes tested in a non-unit color set inference (note, cannot be 'added' nodes) 
	com::stack_t<int> color_involved_stack;		//colors involved in partial conflicts (tests) of non-unit clauses
	////////////////

	bitboard_t node_state_active;				//TRUE-Active, FALSE-Passive
	bitboard_t node_value_false;				//TRUE-FALSE_VAL (UL inference set the node to FALSE), FALSE-FALSE_VAL or NONE 
	int* node_reason;							//[nodes], contains colors or NONE
	int* node_value;							//[nodes], TRUE, FALSE or NONE
	int* node_iset_no;							//[nodes], color label of nodes
	bitboard_t color_state_active;				//[colors], TRUE-Active, FALSE-Passive
	int** node_conflict_set;					//[weakened node]	
	int* color_already_used;					//[color], TRUE or FALSE, used to avoid repetitions during lookback and lookahead conflict storage

	bitboard_t m_unsel;							//used for coloring routines	
	bitboard_t m_sel;							//used for coloring routines		

	//lookahead states
	bitboard_t color_contains_tested_nodes;				//added-colors which contain a tested node (assigned to TRUE (therefore cannot be 'added' nodes) and non-CONFLICTING); drives color selection of lookahead
	int* node_tested_state;								//TRUE-tested (assigned to TRUE and not CONFLICTING), FALSE-untested 
	int* color_involved_state;							//TRUE-color of a partial conflict of a non-unit color set inference, FALSE-not conflicting; drives inference engine (unit_iset_process() chooses used isets first)
	bitboard_t color_already_used_in_extended_test;		//avoids repetitions when testing colors from REDUCED_STACK	
			
////////////////////////
//interface
public:
	InfraOpPlus(graph_t* g=NULL):g(g),gc(NULL), m_colSets(NULL), node_iset_no(NULL), node_reason(NULL), node_value(NULL),
		node_conflict_set(NULL), color_already_used(NULL) , node_tested_state(NULL) , color_involved_state(NULL) {}

	virtual ~InfraOpPlus(){clear();}
	void set_graph(graph_t* g){this->g=g; }
	void set_color_nb(int cmax){NB_OF_COLORS=cmax;}	
	bb_t<bitboard_t>* get_color_db(){return m_colSets;}
	const graph_t* get_graph() const {return g;}

	virtual	int init(int MAX_COL);										//allocates gc and node_node_iset_no
	virtual	void clear();
	void clean_main_unit_literal_stacks(bool unit_isets=false);			//erases UL relevant stacks that are not cleaned elsewhere
		
	//more complex initialization
	void update_color_sizes(int cmax);
	void set_node_state_active(const bitboard_t& bb);
	int set_unit_color_stack();
	int add_node_to_new_color(int node);								//adds node to a new color

public: 
	//added_nodes
	void reset_nb_added_nodes	()	{ADDED_NODES=WMUL(NB_OF_BB_NODES);}
	int number_of_added_nodes	()	{return ADDED_NODES-WMUL(NB_OF_BB_NODES);	/*64 CONFL. MAX*/}  //*** CHECK
	
//////////
// standard drivers
public:
	int init_maxsatz(int v, int clq_size);															//adds v to color_db before starting the inferences  
	int init_maxsatz(int clq_size);									
virtual	int maxsatz(int max_num_conf);
virtual	int maxsatz_unit_literal_only(int max_num_conf);											//mainly for tests 
		int init_maxsatz_unit_literal_only(int v, int clq_size, bool lazey_context_restore=false);	//for testing 
		int init_maxsatz_for_tests(int clq_size, bool restore_context);								//testing context
	
///////////////////
//incremental drivers 
	void init_inc_maxsatz();
	bool inc_maxsatz(int v);										   //v is the vertex in unit set to filter (***TODO: v currently not used inside the function, check!!)
	bool inc_maxsatz_lookahead(int saved_color_unit_stack_pt);
	int inc_test_node(int v, int iset, bool further=false);
//////////////////
//incremental csp drivers
	//bool inc_csp_maxsatz(int v);										//v is the vertex in unit set to filter (***TODO: v currently not used inside the function, check!!)
	//
	//int hard_fix_node_for_unit_iset(int v, int iset);					//essentially removes v from DB
	//int hard_fix_node_for_unit_iset(int v, int iset, bitarray& bb);
	//int hard_unit_iset_process();
	//int hard_unit_iset_process(bitarray& bb, sbb_t<bitarray>& s);
	//bool test_by_eliminate_failed_nodes_csp(bitarray& bb, sbb_t<bitarray>&);	//Main Test driver O(ACTIVE ISETS * ACTIVE NODES IN ISET)
	//bool test_by_eliminate_failed_nodes_csp(bitarray& bb, sbb_t<bitarray>&, com::stack_t<int>& cand);	//Main Test driver O(ACTIVE ISETS * ACTIVE NODES IN ISET)
	//bool inc_maxsatz_lookahead_csp(bitarray& bb, sbb_t<bitarray>& s);
	//int inc_test_node_csp(int v, int iset, bool further=false);
	//int  unit_iset_process_for_test_csp(bitarray& bbsg, sbb_t<bitarray>& s);
	//int test_node_for_failed_nodes_csp(int v, int iset);

	//int unit_iset_process_for_test_by_eliminating_failed_nodes_csp();	
	//int init_inc_maxsatz_csp();
	//int further_test_csp(int start);	

public:
//////////////////
//	main UL inferences
	int unit_iset_process();
	int fix_unit_color(int iset);	
	int fix_unit_color_TEST_MODE(int iset);	
	int fix_node_for_iset(int v, int iset);							//removes non-neighbors of v from other colors ( O(ACTIVE NODES IN ISET) !!!!) 
	int fix_added_node_for_iset(int v, int iset);					//O(LINEAR IN NUMBER OF COLORS IN CONFLICT RELATED TO V)
	int exclude_noneibor(int noneibor, int reason_iset);			//updates color set of noneibor in CONSTANT TIME (used by fix_node_for_iset)
	
	int fix_node_for_non_singleton_iset(int v, int iset);			//same as fix_node_for_iset but for inferences where iset is not a singleton
	
	void lookback_for_maxsatz(int iset);							//O(PASSIVE NODES PER INCONSITENT ISET*NB OF INCONSISTENT ISETS)-fills/manages REASON_STACK, color_already_used
virtual	int enlarge_conflict_set();									//O(REASON_STACK)-assings a new (added) node to all conflicting colores

	void assign_node_value(int v, int val, int reason_iset);
/////
// YES/NO queries
public:
	bool is_enlarged(int iset){return (bool)m_colSets[iset].bb.get_bitboard(NB_OF_BB_NODES);}		/* assumes only 64 possible conflicts for each iset */
	bool is_enlarged_saturated() {return ADDED_NODES>=(NB_OF_BB_ADDED_NODES*WORD_SIZE-1);}
/////////////////

////////////////
// coloring routines to initialize color_db
public:
	int paint	(const bitboard_t& bbs);
	int paint	(const bitboard_t& bbs, int col);					/* bitstring coloring up to, and including, color col if possible */
	int paint_R	(const bitboard_t& bbs, int kmin);
	int paint_R	(const bitboard_t& bbs);							

protected:

//////////////////
//Failed-clause	test (more than one literal, i.e. 2-3)
//propagation, added-nodes for future failed clauses
public:
virtual	int maxsatz_lookahead(int nb_conflict, int max_num_conf);	//Main Test driver O(ACTIVE, NOT WEAKENED, ISET WITH SMALL SIZE AND NO TESTED_LITERALS*NODES IN ISET)
protected:
	int test_node(int v, int iset, bool further=false);				//sets v to FALSE to find a conflict
	int unit_iset_process_for_tests();								//driven by used sets (iset_uset = TRUE_VAL)
	int further_test(int start);									//tests colors from REDUCED_STACK, fills/manages color_already_used_in_extended_tests
	void store_involved_sets();										//REASON_STACK to INVOLVED_SET_STACK, sets iset_used to TRUE_VAL
virtual	int enlarge_stored_involved_sets();							//INVOLVED_SET_STACK to CONFLICT_STACK, fills/manages color_already_used

///////////////////
//Failed-clause	test (more than one literal)
//all nodes in the same clause fail
//one-shot, looks for just one-conflict, no propagation, no added nodes
public:	
	bool test_by_eliminate_failed_nodes(bool lazy_context=false);	//Main Test driver O(ACTIVE ISETS * ACTIVE NODES IN ISET)
//protected:
public:
	int test_node_for_failed_nodes(int v, int iset);
	int simple_further_test(int start);								//tests colors from REDUCED_STACK
	int unit_iset_process_for_test_by_eliminating_failed_nodes();	//simple inference engine over isets in color_dyn_stack

//////////////////
//Context
	void reset_context_for_maxsatz( int saved_node_stack_pt, 
									int saved_color_passive_stack_pt, 
									int save_color_reduced_stack_pt,
									int save_color_unit_stack_pt		);
public:
	void reset_enlarged_isets();
	void reset_enlarged_isets_lazy();


	void reset_context_for_maxsatz_node( int saved_node_stack_pt, 
										int saved_color_passive_stack_pt, 
										int save_color_reduced_stack_pt,
										int save_color_unit_stack_pt		);

	void reset_tested_nodes();			/* nodes set to TRUE during lookahead which produced no conflicts */
	void reset_involved_isets();
private:

/////////////////
// Specific one-shot inference which modifies the color-DB
public:
	bool filter();														//O(COLORS SIZE LESS THAN <X>*ACTIVE VERTICES)-manages filter stack, modifides color-db permanently
protected:
	bool unitiset_filter(int c);									
	bool biniset_filter(int c);
	bool triset_filter(int c);
	bool fouriset_filter(int c);
	bool fiveiset_filter(int c);

public:
	bool filter_non_enlarged();											/* only works in non-enlaged isets */
protected:
	bool unitiset_filter_non_enlarged(int c);
	bool biniset_filter_non_enlarged(int c);
	bool triset_filter_non_enlarged(int c);
	bool fouriset_filter_non_enlarged(int c);
	bool fiveiset_filter_non_enlarged(int c);

	//I/O
public:
	virtual	void print_db				(bool active_sets_only=false, bool active_nodes_only=false);
	void print_test_non_unit_clause_info(bool reduced_stack=false);
	void print_stack					(stack_t stype);
	void print_conflict_set				(int added_node, bool active_colors_only=true);
	

	//tests
virtual bool check_consistency_db(bool check_added_nodes=false);
};

////////////////////////////
// InfraOpPlusMaxConf:

template<typename graph_t, typename bitboard_t>
class InfraOpPlusMaxConf:public InfraOpPlus<graph_t, bitboard_t>{
/////////////////////
// Specializes InfraOpPlus to compute as many conflits
// as possible

	typedef InfraOpPlus<graph_t, bitboard_t> Parent;

public:
	InfraOpPlusMaxConf(graph_t* g=NULL):Parent(g){}
	int init_maxsatz(int v=EMPTY_ELEM, bool reset_enlarged_context=true);	
	
protected:
	int maxsatz();	
	int maxsatz_lookahead(int nb_of_conflicts);
};

template<class graph_t, class bitboard_t>
inline
int InfraOpPlusMaxConf<graph_t, bitboard_t>::init_maxsatz(int v, bool reset_enlarged_context){
///////////////
// PARAMS:  v: node to add as new color to   
//
// FUNCTION: basic maxsatz driver 
// RETURNS: number of conflicts found (or EMPTY_ELEM when conflicts>=clq_size (includes expanded vertex))
//
// REMARKS:
// 1. assumes ACTIVE VERTICES ARE SET
	

	int nb_conflicts=0;
	Parent::reset_nb_added_nodes();											/*possibly not needed if the context is reset properly*/
	if(v!=EMPTY_ELEM){
		Parent::add_node_to_new_color(v);									/* sets new color active as well, which is redundant (1)*/
	}
		
	nb_conflicts=maxsatz();													/*	(1) */
	if(reset_enlarged_context)
		Parent::reset_enlarged_isets();										/* resets contents in full */
	
	if(v!=EMPTY_ELEM){
		--Parent::NB_OF_COLORS;
		Parent::node_state_active.erase_bit(v);		
	}
	
	return (nb_conflicts);
}


template<class graph_t, class bitboard_t>
inline
int InfraOpPlusMaxConf<graph_t, bitboard_t>::maxsatz(){
////////////////////
// maxsatz which computes as many conflicts as possible
	
	//init UL stacks
	int save_color_unit_stack_pt= Parent::set_unit_color_stack();
	Parent::clean_main_unit_literal_stacks(false);									/*not unit_stack*/
			
	int nb_conflicts=0, iset;
	while( (iset=Parent::unit_iset_process())!=Parent::NO_CONFLICT ){						
		Parent::lookback_for_maxsatz(iset);
		Parent::reset_context_for_maxsatz(0, 0,	0, save_color_unit_stack_pt);		/*does not remove the original unit clauses from the stack*/
		Parent::enlarge_conflict_set();
		nb_conflicts++;
	}

	Parent::reset_context_for_maxsatz(0, 0, 0, save_color_unit_stack_pt);			/*resets context if last loop did not produce any conflict*/		
	nb_conflicts=maxsatz_lookahead(nb_conflicts);
		
	if(Parent::test_by_eliminate_failed_nodes()){
		nb_conflicts++;
		//LOG_INFO("CONF ELIMINATE FAILED NODES");
	}/*else{																		// attempt to filter from the inside but BUGGY-*TODO UNDERSTAND 
		Parent::reset_enlarged_isets();	
		if(Parent::filter())
			nb_conflicts++;
		//LOG_INFO("CONF FILTER");
	}*/
	
	return nb_conflicts;
}

template<class graph_t, class bitboard_t>
inline
int InfraOpPlusMaxConf<graph_t, bitboard_t>::maxsatz_lookahead(int nb_conflict){
///////////////
// RETURNS: increments nb_of_conflict with the additional number of conflicts found
			
	bool test_flag, no_conflict_flag;
	Parent::reset_tested_nodes();														/*all nodes clean for testing*/

	//main loop: small color sizes
	for(int k=2; k<=Parent::MAX_COLOR_TEST_LENGTH; k++){
		for(int iset=Parent::NB_OF_COLORS; iset>=1; iset--){	
			if( Parent::color_state_active.is_bit(iset) && Parent::m_colSets[iset].size==k && !Parent::color_contains_tested_nodes.is_bit(iset)				
				&& !Parent::m_colSets[iset].bb.get_bitboard(Parent::NB_OF_BB_NODES) /*not WEAKENED*/ ){

					Parent::color_involved_stack.erase();					/* cleans the stack for the colors */

					//loop through all nodes and test them
					bitboard_t& bbcol=Parent::m_colSets[iset].bb;
					bbcol.init_scan(bbo::NON_DESTRUCTIVE);
					no_conflict_flag=false;							//assumes a conflict will be found
					for(int i=0; i<k; i++){														
						if(Parent::test_node(bbcol.next_bit() /* careful, color should not be scanned inside !*/, iset, (i==k-1) /* is_last*/)==Parent::NO_CONFLICT){			//no need to check if already tested again
							no_conflict_flag=true;					//no conflict
							break;
						}else{				//conflict for this particular node
							Parent::store_involved_sets();							
						}
					}

					//conflict found for all nodes: the REAL CONFLICT
					if(no_conflict_flag==false){
						Parent::enlarge_stored_involved_sets();		   /* also 	color_involved_state-->FALSE_VAL*/
						++nb_conflict;						
					}

					//context operations for next color tests
					Parent::reset_involved_isets();				/* color_involved_state-->FALSE_VAL */


			}//next color to test
		}
	}
	return nb_conflict;
}


///////////////////////////////////////////

template<class graph_t, class bitboard_t>
inline
void InfraOpPlus<graph_t,bitboard_t>::reset_tested_nodes(){
////////////////
// 1.cleans test_node_stack
// 2-set node_tested_state to FALSE_VAL

	for(int i=0; i<node_tested_stack.pt; i++){ 
		int node_tested=node_tested_stack.get_elem(i);
		node_tested_state[node_tested]=FALSE_VAL;									//**remove
		//color_contains_tested_nodes.erase_bit(node_iset_no[node_tested]);			/* has to be done on exit of lookahead_inference!; at the beginning colors are no longer the same (1) */
	}
	node_tested_stack.erase();
	color_contains_tested_nodes.erase_bit();		/* see (1) */
}

template<class graph_t, class bitboard_t>
inline
void InfraOpPlus<graph_t,bitboard_t>::reset_involved_isets(){
////////////////
// 1.cleans involved_stack
// 2.sets color_involved_state to FALSE_VAL for all isets involved
	for(int i=0; i<color_involved_stack.pt; i++)
		color_involved_state[color_involved_stack.get_elem(i)]=FALSE_VAL;
	color_involved_stack.pt=0;
}

template<class graph_t, class bitboard_t>
inline
int InfraOpPlus<graph_t,bitboard_t>::add_node_to_new_color(int v){
////////////////////////
// adds a new singleton color with node to color_db
//
// RETURNS: the total number of colors in the DB

	m_colSets[++NB_OF_COLORS].erase_bit();	
	m_colSets[NB_OF_COLORS].push(v);
	node_state_active.set_bit(v);
	node_value_false.erase_bit(v);
	node_iset_no[v]=NB_OF_COLORS;
	node_reason[v]=NO_REASON;

	color_state_active.set_bit(NB_OF_COLORS);	

	return NB_OF_COLORS;
}

/////////////////////////////////////////////

template<class graph_t, class bitboard_t>
inline
void InfraOpPlus<graph_t,bitboard_t>::clean_main_unit_literal_stacks(bool unit_stack){
	if(unit_stack)
		color_unit_stack.erase();			
	color_unit_dyn_stack.erase();	
	color_passive_stack.erase();		
	color_reduced_stack.erase();		
	color_enlarged_stack.erase();
	color_conflict_stack.erase();
	node_stack.erase();	
}

template<class graph_t, class bitboard_t>
inline
int InfraOpPlus<graph_t,bitboard_t>::set_unit_color_stack(){
//////////////////
// fills unit stack with active singleton color classes
// by decreasing color label
//
// RETURNS end pointer (size)

	color_unit_stack.erase();
	for(int c=NB_OF_COLORS; c>=1; c--){
		color_state_active.set_bit(c);
		if(m_colSets[c].size==1){
			color_unit_stack.push(c);
		}
	}

	return color_unit_stack.pt;
}


template<class graph_t, class bitboard_t>
inline
int  InfraOpPlus<graph_t,bitboard_t>::paint(const bitboard_t & bbs){
///////////////////////
// basic coloring bitstring routine for subgraph bbs 
// 
// RETURNS: number of colors used, -1 if ERROR
//
// REMARKS: bbs should not have space for added nodes

	int pc=(m_unsel=bbs).popcn64(), nBB;
	if(pc==0) return 0;								/*empty subgraph */		
	
	NB_OF_COLORS=1;
	m_colSets[NB_OF_COLORS].erase_bit(false);		/* lazy erasing, will be updated later*/	
	while(true){ 
		for(int i=0; i<NB_OF_BB_NODES; i++){
			m_colSets[NB_OF_COLORS].bb.get_bitboard(i)=m_unsel.get_bitboard(i);
		}

		m_colSets[NB_OF_COLORS].bb.init_scan(bbo::NON_DESTRUCTIVE);
		while(true){
			int v=m_colSets[NB_OF_COLORS].bb.next_bit(nBB,m_unsel);
			if(v==EMPTY_ELEM)
				break;

			//stores color label
			m_colSets[NB_OF_COLORS].size++;							//the node is already there, simply increment size: equivalent to push(v)
			node_iset_no[v]=NB_OF_COLORS;

			//checks exit condition
			if((--pc)==0){
				set_node_state_active(bbs);							//this seems better on average than updating each labeled on the fly	
				color_state_active.set_bit(1, NB_OF_COLORS);
				return NB_OF_COLORS;
			}

			//actual painting routine, equivalent to: m_colSets[col].bb.erase_block(nBB,g->get_neighbors(v));		
			bitarray& nbor=g->get_neighbors(v);
			for(int i=nBB; i<NB_OF_BB_NODES; i++){
				m_colSets[NB_OF_COLORS].bb.get_bitboard(i)&= ~nbor.get_bitboard(i);
			}
		}

		//increments color and erases next color in color_db
		m_colSets[++NB_OF_COLORS].erase_bit(false);					/* lazy erasing, will be updated later*/
	}

	return -1;		//should not occurr-bizarre painting
}


template<class graph_t, class bitboard_t>
inline
int  InfraOpPlus<graph_t,bitboard_t>::paint(const bitboard_t & bbs, int kmin){
//////////////////////
// basic partial greedy independent set coloring of subgraph bbs up to kmin colors 
// 
// RETURNS: number of nodes colored, or -1 if ERROR
//
// REMARKS: bbs should not have space for added nodes

	int pc=(m_unsel=bbs).popcn64(), pc_ini=pc,  nBB;
	if(pc==0) return 0;											/*empty subgraph */		
	m_sel.erase_bit();

	NB_OF_COLORS=1;
	m_colSets[NB_OF_COLORS].erase_bit(false);		/* lazy erasing, will be updated later*/	
	while(true){ 
		for(int i=0; i<NB_OF_BB_NODES; i++){
			m_colSets[NB_OF_COLORS].bb.get_bitboard(i)=m_unsel.get_bitboard(i);
		}

		m_colSets[NB_OF_COLORS].bb.init_scan(bbo::NON_DESTRUCTIVE);
		while(true){
			int v=m_colSets[NB_OF_COLORS].bb.next_bit(nBB,m_unsel);
			if(v==EMPTY_ELEM)
				break;

			//stores color label
			m_colSets[NB_OF_COLORS].size++;							//the node is already there, simply increment size: equivalent to push(v)
			node_iset_no[v]=NB_OF_COLORS;
			m_sel.set_bit(v);

			//checks exit condition
			if((--pc)==0){
				set_node_state_active(bbs);							//this seems better on average than updating each labeled on the fly	
				color_state_active.set_bit(1, NB_OF_COLORS);
				return pc_ini;										/* all nodes colored */
			}

			//actual painting routine, equivalent to: m_colSets[col].bb.erase_block(nBB,g->get_neighbors(v));		
			bitarray& nbor=g->get_neighbors(v);
			for(int i=nBB; i<NB_OF_BB_NODES; i++){
				m_colSets[NB_OF_COLORS].bb.get_bitboard(i)&= ~nbor.get_bitboard(i);
			}
		}

		//exit condition for partial coloring up to, and including, kmin
		if(++NB_OF_COLORS>=kmin /* to catch kmin=1 case*/){	
			set_node_state_active(m_sel);								
			color_state_active.set_bit(1, NB_OF_COLORS-1);
			return (pc_ini-pc);
		}

		//increments color and erases next color in color_db
		m_colSets[NB_OF_COLORS].erase_bit(false);					/* lazy erasing, will be updated later*/
	}

	return -1;		//should not occurr-bizarre painting
}

template<class graph_t, class bitboard_t>
inline
int  InfraOpPlus<graph_t,bitboard_t>::paint_R(const bitboard_t & bbs, int kmin){
////////////////////////
// classical independent set coloring with recoloring of subgraph bb
// last update:13/12/16
//
// PARAMETER kmin: color threshold starting from which nodes are tested for recoloring
//
// REMARKS: 
//	1. produces a fully operative color_db (active nodes, active colors, sizes)
//  2. Should be called with no added nodes in the DB 
//  3. Default: renumbering for all color classes between in [kmin, colmax], not just kmin! 

		
	int col=1, nBB=EMPTY_ELEM, v=EMPTY_ELEM, vswap=EMPTY_ELEM;
	const int KMIN_MINUS_ONE=kmin-1, NB_OF_BB_NODES_MINUS_ONE=NB_OF_BB_NODES-1;
	
	int pc=(m_unsel=bbs).popcn64();
	if(pc==0) return 0;								/*empty subgraph:  set NB_OF_COLORS to 0?*/		

	NB_OF_COLORS=1;
	m_colSets[NB_OF_COLORS].erase_bit(false);		/* lazy erasing, will be updated later*/	
	while(true){ 
		for(int i=0; i<NB_OF_BB_NODES; i++){
			m_colSets[NB_OF_COLORS].bb.get_bitboard(i)=m_unsel.get_bitboard(i);
		}
						
		m_colSets[NB_OF_COLORS].bb.init_scan(bbo::NON_DESTRUCTIVE);
		while(true){
next_v:		v=m_colSets[NB_OF_COLORS].bb.next_bit(nBB,m_unsel);
			if(v==EMPTY_ELEM)	break;				

////////////////////////////
//RECOLORING of v, which would be assigned a label greater than kmin

			if( (NB_OF_COLORS>=kmin) && (kmin>=3) ){
				//for(int recol=1; recol<KMIN_MINUS_ONE; recol++){
				for(int recol=1; recol<NB_OF_COLORS-1; recol++){
					int pc_swap=m_colSets[recol].bb.single_disjoint(0, NB_OF_BB_NODES_MINUS_ONE, g->get_neighbors(v), vswap);
					if(pc_swap==1){	//candidate color class found
						//for(int j=recol+1; j<kmin; j++){
						for(int j=recol+1; j<NB_OF_COLORS; j++){
							if(m_colSets[j].bb.is_disjoint(0, NB_OF_BB_NODES_MINUS_ONE, g->get_neighbors(vswap))){

								m_colSets[j].push(vswap);
								m_colSets[recol].push(v);
								m_colSets[recol].erase_bit(vswap);
								m_colSets[NB_OF_COLORS].bb.erase_bit(v);

								node_iset_no[vswap]=j;
								node_iset_no[v]=recol;

								if((--pc)==0){
									set_node_state_active(bbs);
									if(m_colSets[NB_OF_COLORS].size==0){	
										color_state_active.set_bit(1, --NB_OF_COLORS);
									}else{
										color_state_active.set_bit(1, NB_OF_COLORS);
										
									}
									return NB_OF_COLORS;
								}else goto next_v;
							}

						}
					} else if(pc_swap==0){
						m_colSets[NB_OF_COLORS].bb.erase_bit(v);
						m_colSets[recol].push(v);
						node_iset_no[v]=recol;

						//empty check of unsel in case vertex swapped is the last one
						if((--pc)==0){
							set_node_state_active(bbs);
							if(m_colSets[NB_OF_COLORS].size==0){	
								color_state_active.set_bit(1, --NB_OF_COLORS);
							}else{
								color_state_active.set_bit(1, NB_OF_COLORS);
							}
							return NB_OF_COLORS;
						}else goto next_v;
					}
				}
			}

///////////////////////////////////////

			//stores color label
			m_colSets[NB_OF_COLORS].size++;							//the node is already there, simply increment size: equivalent to push(v)
			node_iset_no[v]=NB_OF_COLORS;

			//checks exit condition
			if((--pc)==0){
				set_node_state_active(bbs);							//this seems better on average than updating each labeled on the fly	
				color_state_active.set_bit(1, NB_OF_COLORS);
				return NB_OF_COLORS;
			}

			//actual painting routine, equivalent to: m_colSets[col].bb.erase_block(nBB,g->get_neighbors(v));		
			bitarray& nbor=g->get_neighbors(v);
			for(int i=nBB; i<NB_OF_BB_NODES; i++){
				m_colSets[NB_OF_COLORS].bb.get_bitboard(i)&= ~nbor.get_bitboard(i);
			}
		}

		//increments color and erases next color in color_db
		m_colSets[++NB_OF_COLORS].erase_bit(false);					/* lazy erasing, will be updated later*/
	}

	LOG_ERROR("paint_UB_BB():bizarre coloring");
	return -1;														/* should not reach here */
}

template<class graph_t, class bitboard_t>
inline
int  InfraOpPlus<graph_t,bitboard_t>::paint_R(const bitboard_t & bbs){
////////////////////////
// classical independent set coloring with recoloring of subgraph bb
// last update:13/12/16
//
// REMARKS: 
//	1. produces a fully operative color_db (active nodes, active colors, sizes)
//  2. Should be called with NO ADDED NODES in the DB (assumes consistency)

		
	int nBB=EMPTY_ELEM, v=EMPTY_ELEM, vswap=EMPTY_ELEM;
	const int NB_OF_BB_NODES_MINUS_ONE=NB_OF_BB_NODES-1;
	
	int pc=(m_unsel=bbs).popcn64();
	if(pc==0) return 0;								/*empty subgraph:  set NB_OF_COLORS to 0?*/		

	NB_OF_COLORS=1;
	m_colSets[NB_OF_COLORS].erase_bit(false);		/* lazy erasing, will be updated later*/	
	while(true){ 
		for(int i=0; i<NB_OF_BB_NODES; i++){
			m_colSets[NB_OF_COLORS].bb.get_bitboard(i)=m_unsel.get_bitboard(i);
		}
						
		m_colSets[NB_OF_COLORS].bb.init_scan(bbo::NON_DESTRUCTIVE);
		while(true){
next_v:		v=m_colSets[NB_OF_COLORS].bb.next_bit(nBB,m_unsel);
			if(v==EMPTY_ELEM)	break;				

////////////////////////////
//RECOLORING of v, which would be assigned a label greater than kmin

			if( (NB_OF_COLORS>=3)  ){
				for(int recol=1; recol<NB_OF_COLORS-1; recol++){
					int pc_swap=m_colSets[recol].bb.single_disjoint(0, NB_OF_BB_NODES_MINUS_ONE, g->get_neighbors(v), vswap);
					if(pc_swap==1){	/*candidate color class found*/
						for(int j=recol+1; j<NB_OF_COLORS; j++){
							if(m_colSets[j].bb.is_disjoint(0, NB_OF_BB_NODES_MINUS_ONE, g->get_neighbors(vswap))){

								m_colSets[j].push(vswap);
								m_colSets[recol].push(v);
								m_colSets[recol].erase_bit(vswap);
								m_colSets[NB_OF_COLORS].bb.erase_bit(v);

								node_iset_no[vswap]=j;
								node_iset_no[v]=recol;

								if((--pc)==0){
									set_node_state_active(bbs);
									if(m_colSets[NB_OF_COLORS].size==0){	
										color_state_active.set_bit(1, --NB_OF_COLORS);
									}else{
										color_state_active.set_bit(1, NB_OF_COLORS);
										
									}
									return NB_OF_COLORS;
								}else goto next_v;
							}

						}
					} else if(pc_swap==0){
						m_colSets[NB_OF_COLORS].bb.erase_bit(v);
						m_colSets[recol].push(v);
						node_iset_no[v]=recol;

						//empty check of unsel in case vertex swapped is the last one
						if((--pc)==0){
							set_node_state_active(bbs);
							if(m_colSets[NB_OF_COLORS].size==0){	
								color_state_active.set_bit(1, --NB_OF_COLORS);
							}else{
								color_state_active.set_bit(1, NB_OF_COLORS);
							}
							return NB_OF_COLORS;
						}else goto next_v;
					}
				}
			}

///////////////////////////////////////

			//stores color label
			m_colSets[NB_OF_COLORS].size++;							//the node is already there, simply increment size: equivalent to push(v)
			node_iset_no[v]=NB_OF_COLORS;

			//checks exit condition
			if((--pc)==0){
				set_node_state_active(bbs);							//this seems better on average than updating each labeled on the fly	
				color_state_active.set_bit(1, NB_OF_COLORS);
				return NB_OF_COLORS;
			}

			//actual painting routine, equivalent to: m_colSets[col].bb.erase_block(nBB,g->get_neighbors(v));		
			bitarray& nbor=g->get_neighbors(v);
			for(int i=nBB; i<NB_OF_BB_NODES; i++){
				m_colSets[NB_OF_COLORS].bb.get_bitboard(i)&= ~nbor.get_bitboard(i);
			}
		}

		//increments color and erases next color in color_db
		m_colSets[++NB_OF_COLORS].erase_bit(false);					/* lazy erasing, will be updated later*/
	}

	LOG_ERROR("paint_UB_BB():bizarre coloring");
	return -1;														/* should not reach here */
}

template<class graph_t, class bitboard_t>
inline
bool InfraOpPlus<graph_t,bitboard_t>::check_consistency_db(bool check_for_added_nodes){
//////////////////
// check that color sizes of active colors correspond with nodes

	bitarray bbaux(NB_OF_ADDED_NODES);
	for(int c=1; c<=NB_OF_COLORS; c++){
		if(color_state_active.is_bit(c)){
			AND(m_colSets[c].bb, node_state_active, bbaux);
			if (bbaux.popcn64()!=m_colSets[c].size){
				LOG_ERROR("InfraOpPlus::check_consistency_db()-bizarre color set:"<<c);
				return false;
			}
		}
	}

	//check for active empty colors
	for(int c=1; c<=NB_OF_COLORS; c++){
		if(color_state_active.is_bit(c)){
			if(m_colSets[c].size==0){
				LOG_ERROR("InfraOpPlus::check_consistency_db-active empty color: "<<c);
				return false;
			}
		}
	}

	//check for added nodes if required
	if(check_for_added_nodes){
		for(int c=1; c<=NB_OF_COLORS; c++){
			if(m_colSets[c].bb.get_bitboard(NB_OF_BB_NODES)){
				LOG_ERROR("InfraOpPlus::check_consistency_db-color:"<<c<<"has added nodes");
				return false;
			}
		}

		if(node_state_active.get_bitboard(NB_OF_BB_NODES)){
			LOG_ERROR("InfraOpPlus::check_consistency_db-nodes added");
			return false;
		}
	}		
	return true;
}

template<class graph_t, class bitboard_t>
inline
void InfraOpPlus<graph_t,bitboard_t>::print_db(bool active_sets_only, bool active_nodes_only){
	int col_active=0;
	LOG_INFO("--------------------------------------------");
	LOG_INFO("NB_COL(total):"<<NB_OF_COLORS);
	for(int c=1; c<=NB_OF_COLORS; c++){
		if(active_sets_only && !color_state_active.is_bit(c)) continue;

		col_active++;
		m_colSets[c].bb.init_scan(bbo::NON_DESTRUCTIVE);
		stringstream sstr("");
		sstr<<c<<"-[";
		while(true){
			int v=m_colSets[c].bb.next_bit();
			if(v==EMPTY_ELEM) break;

			if(active_nodes_only && !node_state_active.is_bit(v)) continue;
			sstr<<v<<" ";
		}
		sstr<<"]"<<"[s:"<<m_colSets[c].size<<"]";
		LOG_INFO(sstr.str());
	}
	if(active_sets_only) LOG_INFO("NB_COL(active):"<<col_active);
	LOG_INFO("--------------------------------------------");
}

template<class graph_t, class bitboard_t>
inline
void InfraOpPlus<graph_t,bitboard_t>::print_test_non_unit_clause_info(bool reduced_stack){
	LOG_INFO("Test-non-unit-clause-info-------------------");
	LOG_INFO("INVOLVED COLORS");
	stringstream sstr("");
	sstr<<"[";
	for(int i=0; i<color_involved_stack.pt; i++){
		int iset=color_involved_stack.get_elem(i);
		sstr<<iset<<" ";
	}
	sstr<<"]";
	LOG_INFO(sstr.str());

	LOG_INFO("COLORS WHICH CONTAIN TESTED NODES");
	color_contains_tested_nodes.print(); cout<<endl;

	LOG_INFO("NON-CONFLICTING NODES SET TO TRUE");
	sstr.clear(); sstr.str("");
	sstr<<"[";
	for(int i=0; i<node_tested_stack.pt; i++){
		int node=node_tested_stack.get_elem(i);
		sstr<<node<<":"<<node_iset_no[node]<<" ";
	}
	sstr<<"]";
	LOG_INFO(sstr.str());

	if(reduced_stack){
		LOG_INFO("REDUCED COLOR STACK");
		sstr.clear(); sstr.str("");
		for(int i=0; i<color_reduced_stack.pt; i++){
			int iset=color_reduced_stack.get_elem(i);
			sstr<<iset<<" ";
		}
		LOG_INFO(sstr.str());
	}
	LOG_INFO("--------------------------------------------");
}

template<class graph_t, class bitboard_t>
inline
void InfraOpPlus<graph_t,bitboard_t>::print_stack(stack_t stype){
	stringstream sstr;
	switch(stype){
	case stack_t::NODES:
		LOG_INFO("NODE STACK");
		sstr<<"[";
		for(int i=0; i<node_stack.pt; i++){
			int node=node_stack.get_elem(i);
			sstr<<node;
			if(node_value[node]==TRUE_VAL) sstr<<":1 ";
			if(node_value[node]==FALSE_VAL) sstr<<":0 ";
		}
		sstr<<"]"<<endl;
		LOG_INFO(sstr.str());
		break;
	case stack_t::COLOR_REDUCED:
		LOG_INFO("COLOR REDUCED STACK");
		sstr<<"[";
		for(int i=0; i<color_reduced_stack.pt; i++){
			sstr<<color_reduced_stack.get_elem(i)<<" ";
		}
		sstr<<"]"<<endl;
		LOG_INFO(sstr.str());
		break;
	case stack_t::COLOR_PASSIVE:
		LOG_INFO("COLOR PASSIVE STACK");
		sstr<<"[";
		for(int i=0; i<color_passive_stack.pt; i++){
			sstr<<color_passive_stack.get_elem(i)<<" ";
		}
		sstr<<"]"<<endl;
		LOG_INFO(sstr.str());
		break;
	case stack_t::COLOR_ACTIVE:
		LOG_INFO("COLOR ACTIVE STACK");
		color_state_active.print();
		cout<<endl;
		break;
	case stack_t::FILTER:
		LOG_INFO("FILTER STACK");
		sstr<<"[";
		for(int i=0; i<color_filter_stack.pt; i++){
			sstr<<color_filter_stack.get_elem(i)<<" ";
		}
		sstr<<"]"<<endl;
		LOG_INFO(sstr.str());
		break;
	case stack_t::COLOR_INVOLVED:
		LOG_INFO("COLOR INVOLVED STACK");
		sstr<<"[";
		for(int i=0; i<color_involved_stack.pt; i++){
			sstr<<color_involved_stack.get_elem(i)<<" ";
		}
		sstr<<"]"<<endl;
		LOG_INFO(sstr.str());
		break;
	case stack_t::COLOR_REASON:
		LOG_INFO("COLOR REASON STACK");
		sstr<<"[";
		for(int i=0; i<color_reason_stack.pt; i++){
			sstr<<color_reason_stack.get_elem(i)<<" ";
		}
		sstr<<"]"<<endl;
		LOG_INFO(sstr.str());
		break;
	}
}

template<class graph_t, class bitboard_t>
inline
void InfraOpPlus<graph_t, bitboard_t>::print_conflict_set (int added_node, bool active_colors_only){
///////////////////
// prints conflict set tagged by added_node (active colors only)

	if(added_node<NB_OF_NODES){
		LOG_ERROR("InfraOpPlus::print_conflict_set-added_node is not enlarged");
		return;
	}

	LOG_INFO("CONFLICT SET FOR ADDED NODE: "<<added_node);
	stringstream sstr("");
	sstr<<"[";
	for(int c=1; c<=NB_OF_COLORS; c++){
		if(active_colors_only && !color_state_active.is_bit(c)) continue;
		if(m_colSets[c].bb.is_bit(added_node))
									sstr<<c<<" ";
	}
	sstr<<"]";
	LOG_INFO(sstr.str());
}

template<class graph_t, class bitboard_t>
inline
void InfraOpPlus<graph_t, bitboard_t>::clear(){

	try{
		if(m_colSets!=NULL){
			delete [] m_colSets;  
		}
		m_colSets=NULL;

		if(node_iset_no){
			delete [] node_iset_no;
		}
		node_iset_no=NULL;

		if(node_value){
			delete [] node_value;
		}
		node_value=NULL;

		if(node_reason){
			delete [] node_reason;
		}
		node_reason=NULL;

		if(node_tested_state){
			delete [] node_tested_state;
		}
		node_tested_state=NULL;
		
		if(node_conflict_set)
			delete [] node_conflict_set;
		node_conflict_set=NULL;

		if(color_already_used){
			delete [] color_already_used;
		}
		color_already_used=NULL;

		if(color_involved_state){
			delete [] color_involved_state;
		}
		color_involved_state=NULL;
		
		//clear complement graph
		if(gc){
			gc->clear();
			delete gc;
		}
		gc=NULL;
	}catch(exception& e){
		LOG_ERROR(e.what();)
		return; 
	}
}

template<class graph_t, class bitboard_t>
inline
int InfraOpPlus<graph_t, bitboard_t>::init(int MAX_NB_COL){
//////////////
// allocates memory for data structures: MAX_NB_COL+1 color sets
//
// RETURNS -1 if ERROR
//
// *** TODO-NOT USING PARAM MAX_NB_COL FOR REDUCED ALLOCATION *****

	if(g==NULL){
		LOG_ERROR("InfraOpPlus::init-graph not defined");
		return -1; 
	}

	clear();

	//int NCOL=MAX_NB_COL+1;
	//int NCOL=MAX_NB_COL+15;				//for solve_first_nodes_incMaxCLQ
	int NCOL=g->number_of_vertices()+1;		//for solve_first_nodes_incMaxCLQ (note: colors are 1 based)

	try{
		//basic params for weakened color sets
		NB_OF_NODES=g->number_of_vertices();
		NB_OF_BB_NODES=g->get_neighbors(0).number_of_bitblocks();
		NB_OF_BB_ADDED_NODES=NB_OF_BB_NODES+1;							//max WORD_SIZE conflicts!
		ADDED_NODES=WMUL(NB_OF_BB_NODES);								
		NB_OF_ADDED_NODES=WMUL(NB_OF_BB_ADDED_NODES)-1;	
		NB_OF_COLORS=0;

		m_colSets= new bb_t<bitboard_t>[NCOL];				//[0] is used to store the subgraph to color. Actual colors range from [1, N]	
		for(int i=0; i<NCOL; i++){
			m_colSets[i].init(NB_OF_ADDED_NODES);			//CAREFUL! normal color data structures use only NB_OF_NODES
		}
		m_unsel.init(NB_OF_NODES);							/*not NB_OF_ADDED_NODES*/
		m_sel.init(NB_OF_NODES);							/*not NB_OF_ADDED_NODES*/

		//allocate node arrays
		node_iset_no= new int[NB_OF_ADDED_NODES];
		node_reason= new int[NB_OF_ADDED_NODES];
		node_value= new int[NB_OF_ADDED_NODES];
		node_tested_state=new int[NB_OF_ADDED_NODES];
		color_contains_tested_nodes.init(NCOL);	

		for(int i=0; i<NB_OF_ADDED_NODES; i++){
			node_iset_no[i]=CLQ_MAXINT;			//*** possibly change to empty traceable value
			node_reason[i]=NO_REASON;
			node_value[i]=NONE;
			node_tested_state[i]=FALSE_VAL;
		}
		
		//color state arrays
		color_already_used=new int[NCOL];
		for(int i=0; i<NCOL; i++){
			color_already_used[i]=FALSE_VAL;	
		}

		color_involved_state=new int[NCOL];
		for(int i=0; i<NCOL; i++){
			color_involved_state[i]=FALSE_VAL;	
		}

		
		//stacks
		color_filter_stack.init(NCOL);
		color_unit_stack.init(NCOL);
		color_unit_dyn_stack.init(NCOL);
		color_passive_stack.init(NCOL);
		color_reason_stack.init(NCOL);	
		color_conflict_stack.init(3*NB_OF_ADDED_NODES);			/* Evil 184- necessary *3, during solve_incMaxSat routine  */
		color_reduced_stack.init(3*NB_OF_ADDED_NODES);			/* Evil 184- necessary *3, during solve_incMaxSat routine  */
		color_enlarged_stack.init(3*NB_OF_ADDED_NODES);			/* Evil 184- necessary *3, during solve_incMaxSat routine  */

		node_stack.init(2*NB_OF_ADDED_NODES);

		//for lookaheads
		color_involved_stack.init(2*NB_OF_ADDED_NODES);	//colors may be repeated
		node_tested_stack.init(NB_OF_ADDED_NODES);

				
		//state
		node_state_active.init(NB_OF_ADDED_NODES);
		color_state_active.init(NCOL);
		color_already_used_in_extended_test.init(NCOL);			//used to avoid repetitions in color testing from reduced stack

		node_value_false.init(NB_OF_ADDED_NODES);

		//other params
		node_conflict_set=new int*[NB_OF_ADDED_NODES];	//max WORD_SIZE conflicts!

		//allocate complement graph
		gc=new graph_t(1);
		g->create_complement(*gc);			//***check -1 

	}catch(exception& e){
		LOG_ERROR(e.what();)
		return -1; 
	}
		
	return 0;
}
template<class graph_t, class bitboard_t>
inline
void InfraOpPlus<graph_t, bitboard_t>::update_color_sizes(int cmax){
	for(int c=1; c<=cmax; c++)
		m_colSets[c].update_size();
}

template <class graph_t, class bitboard_t>
inline
void InfraOpPlus<graph_t,bitboard_t>::set_node_state_active(const bitboard_t& bb){
//////////////////////
// copies subgraph bb to node_state_active (note sizes are different)
	for(int nBB=0; nBB<NB_OF_BB_NODES; nBB++){
		node_state_active.get_bitboard(nBB)=bb.get_bitboard(nBB);
	}

	//cleans all added nodes
	node_state_active.get_bitboard(NB_OF_BB_NODES)=0;		//assuming NB_OF_BB_ADDED_NODES=NB_OF_BB_NODES+1
}

template <class graph_t, class bitboard_t >
inline
bool InfraOpPlus<graph_t,bitboard_t>::filter(){
//////////////////////
// guided by color stack a la incMaxCLQ
// RETURNS TRUE if filter succeeds, FALSE if it does not
//
// REMARKS
// 1.assumes size of color sets is updated
// 2.assumes that active nodes are updated
// Note that the colors in the filter stack cannot be repeated
					
	color_filter_stack.erase();
	for(int c=NB_OF_COLORS; c>=1; c--){				/* important, since higher color vertices tend to have less size*/
#ifdef FIVE_ISET_FILTER 
		if(m_colSets[c].size<=5){ 
#else
		if(m_colSets[c].size<=4){ 
#endif
			color_filter_stack.push(c);
		}
	}
	
	//main loop
	for(int i=0; i<color_filter_stack.pt; i++){
		//print_stack(FILTER);
		int c=color_filter_stack.get_elem(i);
		if(m_colSets[c].size==1 && unitiset_filter(c)){
			//LOG_INFO("ONE-SHOT-SIZE-1");
			return true;
		}else if(m_colSets[c].size==2 && biniset_filter(c)){
			//LOG_INFO("ONE-SHOT-SIZE-2");
			return true;
		}else if(m_colSets[c].size==3 && triset_filter(c)){
			//LOG_INFO("ONE-SHOT-SIZE-3");
			return true;
		}else if(m_colSets[c].size==4 && fouriset_filter(c)){
			//LOG_INFO("ONE-SHOT-SIZE-4");
			return true;
		}
#ifdef FIVE_ISET_FILTER 
		else if(m_colSets[c].size==5 && fiveiset_filter(c)){
			//LOG_INFO("ONE-SHOT-SIZE-4");
			return true;
		}
#endif
	}

	return false;		//no filter possible
}

template <class graph_t, class bitboard_t >
inline
bool InfraOpPlus<graph_t,bitboard_t>::filter_non_enlarged(){
//////////////////////
// guided by color stack a la incMaxCLQ
// RETURNS TRUE if filter succeeds, FALSE if it does not
//
// REMARKS
// 1.assumes size of color sets is updated
// 2.assumes that active nodes are updated
// Note that the colors in the filter stack cannot be repeated
					
	color_filter_stack.erase();
	for(int c=NB_OF_COLORS; c>=1; c--){				/* important, since higher color vertices tend to have less size*/
		if(is_enlarged(c)) continue;
#ifdef FIVE_ISET_FILTER 
		if(m_colSets[c].size<=5){ 
#else
		if(m_colSets[c].size<=4){ 
#endif
			color_filter_stack.push(c);
		}
	}
	
	//main loop
	for(int i=0; i<color_filter_stack.pt; i++){
		int c=color_filter_stack.get_elem(i);
		if(m_colSets[c].size==1 && unitiset_filter_non_enlarged(c)){
			//LOG_INFO("ONE-SHOT-SIZE-1");
			return true;
		}else if(m_colSets[c].size==2 && biniset_filter_non_enlarged(c)){
			//LOG_INFO("ONE-SHOT-SIZE-2");
			return true;
		}else if(m_colSets[c].size==3 && triset_filter_non_enlarged(c)){
			//LOG_INFO("ONE-SHOT-SIZE-3");
			return true;
		}else if(m_colSets[c].size==4 && fouriset_filter_non_enlarged(c)){
			//LOG_INFO("ONE-SHOT-SIZE-4");
			return true;
		}
#ifdef FIVE_ISET_FILTER 
		else if(m_colSets[c].size==5 && fiveiset_filter_non_enlarged(c)){
			//LOG_INFO("ONE-SHOT-SIZE-4");
			return true;
		}
#endif
	}

	return false;		//no filter possible
}

template <class graph_t, class bitboard_t >
inline
bool InfraOpPlus<graph_t,bitboard_t>::unitiset_filter(int c){
////////////////
// returns TRUE if filter succeeds, FALSE if it does not
	int v;
	bitarray& nnb=gc->get_neighbors(m_colSets[c].bb.lsbn64());
	node_state_active.init_scan(bbo::NON_DESTRUCTIVE);
	while(true){
		v=node_state_active.next_bit();
		if(v==EMPTY_ELEM) return false;
		if(node_iset_no[v]!=c && nnb.is_bit(v)){
			node_state_active.erase_bit(v);
			m_colSets[node_iset_no[v]].erase_bit(v);

#ifdef FIVE_ISET_FILTER 
			if(m_colSets[node_iset_no[v]].size==5){ 
#else
			if(m_colSets[node_iset_no[v]].size==4){ 
#endif
				color_filter_stack.push(node_iset_no[v]);
			}else if(m_colSets[node_iset_no[v]].size==0) return true;
		}
	}
	return false;
}

template <class graph_t, class bitboard_t >
inline
bool InfraOpPlus<graph_t,bitboard_t>::unitiset_filter_non_enlarged(int c){
//////////////////
//// returns TRUE if filter succeeds, FALSE if it does not
//
	int v;
	bitarray& nnb=gc->get_neighbors(m_colSets[c].bb.lsbn64());
	node_state_active.init_scan(bbo::NON_DESTRUCTIVE);
	while(true){
		v=node_state_active.next_bit();
		if(v==EMPTY_ELEM) return false;
		else if(v>=NB_OF_NODES) continue;
		int iset=node_iset_no[v];
		if(iset!=c && nnb.is_bit(v)){
			node_state_active.erase_bit(v);
			m_colSets[iset].erase_bit(v);
			if(is_enlarged(iset)) continue;							/* filters non enlarged sets */
#ifdef FIVE_ISET_FILTER 
			if(m_colSets[iset].size==5){ 
#else
			if(m_colSets[iset].size==4){ 
#endif
				color_filter_stack.push(iset);
			}else if(m_colSets[iset].size==0) return true;
		}
	}

	return false;
}

template <class graph_t, class bitboard_t >
inline
bool InfraOpPlus<graph_t,bitboard_t>::biniset_filter(int c){
////////////////
// returns TRUE if filter succeeds, FALSE if it does not
	int v;
	m_colSets[c].bb.init_scan(bbo::NON_DESTRUCTIVE);
	bitarray& nnb1=gc->get_neighbors(m_colSets[c].bb.next_bit());
	bitarray& nnb2=gc->get_neighbors(m_colSets[c].bb.next_bit());
	node_state_active.init_scan(bbo::NON_DESTRUCTIVE);
	while(true){
		v=node_state_active.next_bit();
		if(v==EMPTY_ELEM) return false;
		if(node_iset_no[v]!=c && nnb1.is_bit(v) && nnb2.is_bit(v)) {
			node_state_active.erase_bit(v);
			m_colSets[node_iset_no[v]].erase_bit(v);
#ifdef FIVE_ISET_FILTER 
			if(m_colSets[node_iset_no[v]].size==5){ 
#else
			if(m_colSets[node_iset_no[v]].size==4){ 
#endif
				color_filter_stack.push(node_iset_no[v]);
			}else if(m_colSets[node_iset_no[v]].size==0) return true;
		}
	}
	return false;
}

template <class graph_t, class bitboard_t >
inline
bool InfraOpPlus<graph_t,bitboard_t>::biniset_filter_non_enlarged(int c){
////////////////
// returns TRUE if filter succeeds, FALSE if it does not

	int v;
	m_colSets[c].bb.init_scan(bbo::NON_DESTRUCTIVE);
	bitarray& nnb1=gc->get_neighbors(m_colSets[c].bb.next_bit());
	bitarray& nnb2=gc->get_neighbors(m_colSets[c].bb.next_bit());
	node_state_active.init_scan(bbo::NON_DESTRUCTIVE);
	while(true){
		v=node_state_active.next_bit();
		if(v==EMPTY_ELEM) return false;
		else if(v>=NB_OF_NODES) continue;
		int iset=node_iset_no[v];
		if(iset!=c && nnb1.is_bit(v) && nnb2.is_bit(v)) {
			node_state_active.erase_bit(v);
			m_colSets[iset].erase_bit(v);
			if(is_enlarged(iset)) continue;							/* filters non enlarged sets */
#ifdef FIVE_ISET_FILTER 
			if(m_colSets[iset].size==5){ 
#else
			if(m_colSets[iset].size==4){ 
#endif
				color_filter_stack.push(iset);
			}else if(m_colSets[iset].size==0) return true;
		}
	}
	return false;
}



template <class graph_t, class bitboard_t >
inline
bool InfraOpPlus<graph_t,bitboard_t>::triset_filter(int c){
////////////////
// returns TRUE if filter succeeds, FALSE if it does not
	int v;

	m_colSets[c].bb.init_scan(bbo::NON_DESTRUCTIVE);
	bitarray& nnb1=gc->get_neighbors(m_colSets[c].bb.next_bit());
	bitarray& nnb2=gc->get_neighbors(m_colSets[c].bb.next_bit());
	bitarray& nnb3=gc->get_neighbors(m_colSets[c].bb.next_bit());
	node_state_active.init_scan(bbo::NON_DESTRUCTIVE);
	while(true){
		v=node_state_active.next_bit();
		if(v==EMPTY_ELEM) return false;
		if(node_iset_no[v]!=c &&  nnb1.is_bit(v) && nnb2.is_bit(v) && nnb3.is_bit(v)){
			node_state_active.erase_bit(v);
			m_colSets[node_iset_no[v]].erase_bit(v);
#ifdef FIVE_ISET_FILTER 
			if(m_colSets[node_iset_no[v]].size==5){ 
#else
			if(m_colSets[node_iset_no[v]].size==4){ 
#endif
				color_filter_stack.push(node_iset_no[v]);
			}else if(m_colSets[node_iset_no[v]].size==0) return true;
		}
	}

	return false;
}

template <class graph_t, class bitboard_t >
inline
bool InfraOpPlus<graph_t,bitboard_t>::triset_filter_non_enlarged(int c){
////////////////
// returns TRUE if filter succeeds, FALSE if it does not
	int v;

	m_colSets[c].bb.init_scan(bbo::NON_DESTRUCTIVE);
	bitarray& nnb1=gc->get_neighbors(m_colSets[c].bb.next_bit());
	bitarray& nnb2=gc->get_neighbors(m_colSets[c].bb.next_bit());
	bitarray& nnb3=gc->get_neighbors(m_colSets[c].bb.next_bit());
	node_state_active.init_scan(bbo::NON_DESTRUCTIVE);
	while(true){
		v=node_state_active.next_bit();
		if(v==EMPTY_ELEM) return false;
		else if(v>=NB_OF_NODES) continue;
		int iset=node_iset_no[v];
		if(iset!=c &&  nnb1.is_bit(v) && nnb2.is_bit(v) && nnb3.is_bit(v)){
			node_state_active.erase_bit(v);
			m_colSets[iset].erase_bit(v);
			if(is_enlarged(iset)) continue;							/* filters non enlarged sets */
#ifdef FIVE_ISET_FILTER 
			if(m_colSets[iset].size==5){ 
#else
			if(m_colSets[iset].size==4){ 
#endif
				color_filter_stack.push(iset);
			}else if(m_colSets[iset].size==0) return true;
		}
	}

	return false;
}

template <class graph_t, class bitboard_t >
inline
bool InfraOpPlus<graph_t,bitboard_t>::fouriset_filter(int c){
////////////////
// returns TRUE if filter succeeds, FALSE if it does not
	int v;
	m_colSets[c].bb.init_scan(bbo::NON_DESTRUCTIVE);
	bitarray& nnb1=gc->get_neighbors(m_colSets[c].bb.next_bit());
	bitarray& nnb2=gc->get_neighbors(m_colSets[c].bb.next_bit());
	bitarray& nnb3=gc->get_neighbors(m_colSets[c].bb.next_bit());
	bitarray& nnb4=gc->get_neighbors(m_colSets[c].bb.next_bit());
	//*** possibly compute the AND Bitset
	node_state_active.init_scan(bbo::NON_DESTRUCTIVE);
	while(true){
		v=node_state_active.next_bit();
		if(v==EMPTY_ELEM) return false;
		if(node_iset_no[v]!=c && nnb1.is_bit(v) && nnb2.is_bit(v) && nnb3.is_bit(v) && nnb4.is_bit(v)){
			node_state_active.erase_bit(v);
			m_colSets[node_iset_no[v]].erase_bit(v);
#ifdef FIVE_ISET_FILTER 
			if(m_colSets[node_iset_no[v]].size==5){ 
#else
			if(m_colSets[node_iset_no[v]].size==4){ 
#endif
				color_filter_stack.push(node_iset_no[v]);
			}else if(m_colSets[node_iset_no[v]].size==0) return true;
		}
	}
	return false;
}


template <class graph_t, class bitboard_t >
inline
bool InfraOpPlus<graph_t,bitboard_t>::fouriset_filter_non_enlarged(int c){
////////////////
// returns TRUE if filter succeeds, FALSE if it does not
	int v;
	m_colSets[c].bb.init_scan(bbo::NON_DESTRUCTIVE);
	bitarray& nnb1=gc->get_neighbors(m_colSets[c].bb.next_bit());
	bitarray& nnb2=gc->get_neighbors(m_colSets[c].bb.next_bit());
	bitarray& nnb3=gc->get_neighbors(m_colSets[c].bb.next_bit());
	bitarray& nnb4=gc->get_neighbors(m_colSets[c].bb.next_bit());
	//*** possibly compute the AND Bitset
	node_state_active.init_scan(bbo::NON_DESTRUCTIVE);
	while(true){
		v=node_state_active.next_bit();
		if(v==EMPTY_ELEM) return false;
		else if(v>=NB_OF_NODES) continue;
		int iset=node_iset_no[v];
		if(iset!=c && nnb1.is_bit(v) && nnb2.is_bit(v) && nnb3.is_bit(v) && nnb4.is_bit(v)){
			node_state_active.erase_bit(v);
			m_colSets[iset].erase_bit(v);
			if(is_enlarged(iset)) continue;							/* filters non enlarged sets */
#ifdef FIVE_ISET_FILTER 
			if(m_colSets[iset].size==5){ 
#else
			if(m_colSets[iset].size==4){ 
#endif
				color_filter_stack.push(iset);
			}else if(m_colSets[iset].size==0) return true;
		}
	}
	return false;
}

template <class graph_t, class bitboard_t >
inline
bool InfraOpPlus<graph_t,bitboard_t>::fiveiset_filter(int c){
////////////////
// returns TRUE if filter succeeds, FALSE if it does not
	int v;
	m_colSets[c].bb.init_scan(bbo::NON_DESTRUCTIVE);
	bitarray& nnb1=gc->get_neighbors(m_colSets[c].bb.next_bit());
	bitarray& nnb2=gc->get_neighbors(m_colSets[c].bb.next_bit());
	bitarray& nnb3=gc->get_neighbors(m_colSets[c].bb.next_bit());
	bitarray& nnb4=gc->get_neighbors(m_colSets[c].bb.next_bit());
	bitarray& nnb5=gc->get_neighbors(m_colSets[c].bb.next_bit());
	//*** possibly compute the AND Bitset
	node_state_active.init_scan(bbo::NON_DESTRUCTIVE);
	while(true){
		v=node_state_active.next_bit();
		if(v==EMPTY_ELEM) return false;
		if(node_iset_no[v]!=c && nnb1.is_bit(v) && nnb2.is_bit(v) && nnb3.is_bit(v) && nnb4.is_bit(v) && nnb5.is_bit(v)){
			node_state_active.erase_bit(v);
			m_colSets[node_iset_no[v]].erase_bit(v);
#ifdef FIVE_ISET_FILTER 
			if(m_colSets[node_iset_no[v]].size==5){ 
#else
			if(m_colSets[node_iset_no[v]].size==4){ 
#endif
				color_filter_stack.push(node_iset_no[v]);
			}else if(m_colSets[node_iset_no[v]].size==0) return true;
		}
	}
	return false;
}

template <class graph_t, class bitboard_t >
inline
bool InfraOpPlus<graph_t,bitboard_t>::fiveiset_filter_non_enlarged(int c){
////////////////
// returns TRUE if filter succeeds, FALSE if it does not
	int v;
	m_colSets[c].bb.init_scan(bbo::NON_DESTRUCTIVE);
	bitarray& nnb1=gc->get_neighbors(m_colSets[c].bb.next_bit());
	bitarray& nnb2=gc->get_neighbors(m_colSets[c].bb.next_bit());
	bitarray& nnb3=gc->get_neighbors(m_colSets[c].bb.next_bit());
	bitarray& nnb4=gc->get_neighbors(m_colSets[c].bb.next_bit());
	bitarray& nnb5=gc->get_neighbors(m_colSets[c].bb.next_bit());
	//*** possibly compute the AND Bitset
	node_state_active.init_scan(bbo::NON_DESTRUCTIVE);
	while(true){
		v=node_state_active.next_bit();
		if(v==EMPTY_ELEM) return false;
		else if(v>=NB_OF_NODES) continue;
		int iset=node_iset_no[v];
		if(iset!=c && nnb1.is_bit(v) && nnb2.is_bit(v) && nnb3.is_bit(v) && nnb4.is_bit(v) && nnb5.is_bit(v)){
			node_state_active.erase_bit(v);
			m_colSets[iset].erase_bit(v);
			if(is_enlarged(iset)) continue;							/* filters non enlarged sets */
#ifdef FIVE_ISET_FILTER 
			if(m_colSets[iset].size==5){ 
#else
			if(m_colSets[iset].size==4){ 
#endif
				color_filter_stack.push(iset);
			}else if(m_colSets[iset].size==0) return true;
		}
	}
	return false;
}

template <class graph_t, class bitboard_t>
inline
int InfraOpPlus<graph_t,bitboard_t>::init_maxsatz(int v, int clq_size){
///////////////
// PARAMS: v: expanded vertex which has led to the current subgraph
//		   clq_size: kmin for the current subgraph (BEST_CLQ_SOL-DEPTH)
//
// FUNCTION: init operations previous to call infrachrom solver
// RETURNS: number of conflicts found or MAX_NB_OF_CONFLICTS when conflicts>=clq_size (includes expanded vertex)
//
// REMARKS:
// 1. assumes ACTIVE VERTICES ARE SET!
// 2. clq_size MUST be at least 1 for lookahead inference to work  (clq_size=0 does NOT fire LOOKAHEAD)
//    Note that this is consistent with clq_size semantics

	int nb_conflicts=0;
	reset_nb_added_nodes();					/*possibly not needed if the context is reset properly*/
	add_node_to_new_color(v);				/* sets new color active as well, which is redundant (1)*/

	int nb_max_conf=NB_OF_COLORS-clq_size;
	nb_conflicts=maxsatz(nb_max_conf);		/*	(1) */
	reset_enlarged_isets();					/* resets added nodes as well */
	//reset_enlarged_isets_lazy();	
	
	NB_OF_COLORS--;
		
	//check if nb_conflicts is saturated
	if(nb_conflicts>=nb_max_conf) return MAX_NB_OF_CONFLICTS;		
	
	return nb_conflicts;
}

template <class graph_t, class bitboard_t>
inline
int InfraOpPlus<graph_t,bitboard_t>::init_maxsatz_unit_literal_only(int v, int clq_size, bool lazy_context){
///////////////////
// driver for unit literal inferences only

	int nb_conflicts=0;
	reset_nb_added_nodes();					/*possibly not needed if the context is reset properly*/
	add_node_to_new_color(v);				/* sets new color active as well, which is redundant (1)*/
	
	int nb_max_conf=NB_OF_COLORS-clq_size;
	nb_conflicts=maxsatz_unit_literal_only(nb_max_conf);			/*	(1) */
	if(lazy_context)
		reset_enlarged_isets_lazy();								/* resets context only for added_nodes number*/
	else reset_enlarged_isets();									/* resets context in full (added nodes as well) */

	
	return (--NB_OF_COLORS);
}

template <class graph_t, class bitboard_t>
inline
int InfraOpPlus<graph_t,bitboard_t>::init_maxsatz(int clq_size){
///////////////
// PARAMS:    clq_size: kmin for the current subgraph (BEST_CLQ_SOL-DEPTH)
//
// FUNCTION: basic maxsatz driver 
// RETURNS: number of conflicts found or EMPTY_ELEM when conflicts>=clq_size (includes expanded vertex)
//
// REMARKS:
// 1. assumes ACTIVE VERTICES ARE SET

	int nb_conflicts=0;
	reset_nb_added_nodes();						/*possibly not needed if the context is reset properly*/
			
	int nb_max_conf=NB_OF_COLORS-clq_size;
	nb_conflicts=maxsatz(nb_max_conf);
	reset_enlarged_isets();		
	//reset_enlarged_isets_lazy();
		
	if(nb_conflicts>=nb_max_conf) return MAX_NB_OF_CONFLICTS;
	return nb_conflicts;
}

template <class graph_t, class bitboard_t>
inline
int InfraOpPlus<graph_t,bitboard_t>::init_maxsatz_for_tests(int clq_size, bool restore_context){
///////////////
// PARAMS:    clq_size: kmin for the current subgraph (BEST_CLQ_SOL-DEPTH)
//
// FUNCTION: basic maxsatz driver 
// RETURNS: number of conflicts found or EMPTY_ELEM when conflicts>=clq_size (includes expanded vertex)
//
// REMARKS:
// 1. assumes ACTIVE VERTICES ARE SET

	int nb_conflicts=0;
	reset_nb_added_nodes();						/*possibly not needed if the context is reset properly*/
			
	int nb_max_conf=NB_OF_COLORS-clq_size;
	nb_conflicts=maxsatz(nb_max_conf);
	
	if(restore_context){
		reset_enlarged_isets();		
		//reset_enlarged_isets_lazy();
	}
		
	if(nb_conflicts>=nb_max_conf) return MAX_NB_OF_CONFLICTS;
	return nb_conflicts;
}

template <class graph_t, class bitboard_t>
inline
int InfraOpPlus<graph_t,bitboard_t>::maxsatz(int max_num_conf){
////////////////////
// basic driver for main infrachrom reasoning

	//init UL stacks
	int save_color_unit_stack_pt=set_unit_color_stack();
	clean_main_unit_literal_stacks(false);					/*not unit_stack*/
			
	int nb_conflicts=0, iset;
	while( (iset=unit_iset_process())!=NO_CONFLICT ){						
		lookback_for_maxsatz(iset);
		reset_context_for_maxsatz(0, 0,	0, save_color_unit_stack_pt);		/*does not remove the original unit clauses from the stack*/
		enlarge_conflict_set();
		if (++nb_conflicts>=max_num_conf)
				break;
	}

	reset_context_for_maxsatz(0, 0, 0, save_color_unit_stack_pt);			/*resets context if last loop did not produce any conflict*/		

	if (nb_conflicts>=max_num_conf)
					return nb_conflicts;

#ifdef TEST_LOOK_AHEAD_ISETS	
	 nb_conflicts=maxsatz_lookahead(nb_conflicts, max_num_conf);
	  reset_tested_nodes();
	 if (nb_conflicts>=max_num_conf){
		 //LOG_INFO("CUT DURING TESTS INFRACHROM");
		 return nb_conflicts;
	 }
#endif

#ifdef TEST_ELIMINATE_FAILED_NODES
	if( (nb_conflicts + 1) == max_num_conf){
		 if(test_by_eliminate_failed_nodes(false /* no lazy context restoring*/)){		
			 nb_conflicts++;
			 //LOG_INFO("CONF ELIMINATE FAILED NODES");
		 }
	 }
#endif	

	return nb_conflicts;
}

template <class graph_t, class bitboard_t>
inline
int InfraOpPlus<graph_t,bitboard_t>::maxsatz_unit_literal_only(int max_nb_conf){
//////////////////
// driver for basic unit literal inference only

	int save_color_unit_stack_pt=set_unit_color_stack();
	clean_main_unit_literal_stacks(false /*not unit_stack*/);				

	/*int saved_node_stack_pt=0;
	int saved_color_passive_stack_pt=0;
	int save_color_reduced_stack_pt=0;*/
			
	int nb_conflicts=0;
	int iset;
	while( (iset=unit_iset_process())!=NO_CONFLICT ){						
		lookback_for_maxsatz(iset);
		reset_context_for_maxsatz(0, 0,	0, save_color_unit_stack_pt);		/*does not remove the original unit clauses from the stack*/
		enlarge_conflict_set();
		if (++nb_conflicts >= max_nb_conf)
			break;
	}

	reset_context_for_maxsatz(0, 0, 0, save_color_unit_stack_pt);			/*resets context if last loop did not produce any conflict*/	
	return nb_conflicts;
}


//template <class graph_t, class bitboard_t>
//inline
//int InfraOpPlus<graph_t,bitboard_t>::hard_unit_iset_process(){
///////////////
//// inference mechanism for singleton colors without contex restoring
////
//// should be used in color_DBs WITHOUT added nodes
////
//// RETURNS conflicting color set or NO_CONFLICT
//
//	int col, my_iset;
//	for(int i=0; i<color_unit_stack.pt; i++){
//		col=color_unit_stack.get_elem(i);
//		if(color_state_active.is_bit(col) && m_colSets[col].size==1){		
//			color_unit_dyn_stack.erase();
//			int v=FIRST_SHARED(m_colSets[col].bb, node_state_active);	
//			if ((my_iset=hard_fix_node_for_unit_iset(v,col))!=NO_CONFLICT )				
//				return my_iset;
//			
//		
//			for(int j=0; j<color_unit_dyn_stack.pt; j++){
//				col=color_unit_dyn_stack.get_elem(j);
//				if(color_state_active.is_bit(col)){
//					int v=FIRST_SHARED(m_colSets[col].bb, node_state_active);	
//					if ((my_iset=hard_fix_node_for_unit_iset(v,col))!=NO_CONFLICT )
//						return my_iset;
//				}
//			}
//		}
//	}
//	
//	
//	color_unit_dyn_stack.erase();			
//	return NO_CONFLICT;
//}

//template <class graph_t, class bitboard_t>
//inline
//int InfraOpPlus<graph_t,bitboard_t>::hard_unit_iset_process(bitarray& bbsg, sbb_t<bitarray>& s){
///////////////
//// inference mechanism for singleton colors without contex restoring
////
//// should be used in color_DBs WITHOUT added nodes
////
//// RETURNS conflicting color set or NO_CONFLICT
//
//	int col, my_iset;
//	for(int i=0; i<color_unit_stack.pt; i++){
//		col=color_unit_stack.get_elem(i);
//		if(color_state_active.is_bit(col) && m_colSets[col].size==1){	
//			color_unit_dyn_stack.erase();
//			//int v=FIRST_SHARED(m_colSets[col].bb, node_state_active);
//			int v=m_colSets[col].bb.lsbn64();
//
//			s.push(v);
//			if ((my_iset=hard_fix_node_for_unit_iset(v,col, bbsg))!=NO_CONFLICT )				
//				return my_iset;
//			
//		
//			for(int j=0; j<color_unit_dyn_stack.pt; j++){
//				col=color_unit_dyn_stack.get_elem(j);
//				if(color_state_active.is_bit(col)){
//				//	int v=FIRST_SHARED(m_colSets[col].bb, node_state_active);
//					int v=m_colSets[col].bb.lsbn64();
//					s.push(v);
//					if ((my_iset=hard_fix_node_for_unit_iset(v, col, bbsg))!=NO_CONFLICT )
//						return my_iset;
//				}
//			}
//		}
//	}
//	
//	
//	color_unit_dyn_stack.erase();			
//	return NO_CONFLICT;
//}

template <class graph_t, class bitboard_t>
inline
int InfraOpPlus<graph_t,bitboard_t>::unit_iset_process(){
/////////////
// inference mechanism for singleton colors
//
// RETURNS conflicting color set or NO_CONFLICT

	int col, my_iset;
	for(int i=0; i<color_unit_stack.pt; i++){
		col=color_unit_stack.get_elem(i);
		if(color_state_active.is_bit(col) && m_colSets[col].size==1){		
			color_unit_dyn_stack.erase();
			if ((my_iset=fix_unit_color(col))!=NO_CONFLICT )
				return my_iset;
		
			for(int j=0; j<color_unit_dyn_stack.pt; j++){
				col=color_unit_dyn_stack.get_elem(j);
				if(color_state_active.is_bit(col)){
					if ((my_iset=fix_unit_color(col))!=NO_CONFLICT )
						return my_iset;
				}
			}
		}
	}

	color_unit_dyn_stack.erase();			
	return NO_CONFLICT;
}


template <class graph_t, class bitboard_t>
inline
int InfraOpPlus<graph_t,bitboard_t>::fix_unit_color(int iset){
/////////////////
// PARAMS: color is a singleton color set
//
// FUNCTION: driver for main UL inference with color .Called when iset
//           has a single open node
//
// REMARKS: no checking if there is only one active vertex in iset
//			use fix_unit_color_TEST_MODE for developing

	int v=FIRST_SHARED(m_colSets[iset].bb,node_state_active);					//finds the one (and ONLY!) vertex 			
	if(v>=NB_OF_NODES){
		return fix_added_node_for_iset(v, iset);
	}else return fix_node_for_iset(v, iset);
	
	LOG_ERROR("fix_unit_color():should not reach here");
	return NO_CONFLICT;
}

template <class graph_t, class bitboard_t>
inline
int InfraOpPlus<graph_t,bitboard_t>::fix_unit_color_TEST_MODE(int iset){
/////////////////
// PARAMS: color is a singleton color set
//
// FUNCTION: driver for main UL inference with color
// 
// REMARKS: Tests that iset only has one active vertex

	int v=EMPTY_ELEM;
	int pc=m_colSets[iset].bb.single_disjoint(node_state_active, v);				 //also finds weakened nodes in last bitblock
	
	
	if(pc!=1){
		if(pc==0){
			cout<<endl;m_colSets[iset].bb.print(); cout<<":"<<m_colSets[iset].size<<endl;
			node_state_active.print(); cout<<endl;

			cout<<"REDUCED STACK"<<endl;
			for(int i=0; i<color_reduced_stack.pt; i++){
				m_colSets[color_reduced_stack.get_elem(i)].bb.print(); cout<<":"<<m_colSets[color_reduced_stack.get_elem(i)].size;
				cout<<endl;
			}

			cout<<"NODE STACK"<<endl;
			for(int i=0; i<node_stack.pt; i++){
				cout<<node_stack.get_elem(i)<<endl;
			}

			cout<<"CONFLICT STACK"<<endl;
			for(int i=0; i<color_conflict_stack.pt; i++){
				cout<<color_conflict_stack.get_elem(i)<<endl;
			}
				

			LOG_ERROR("fix_unit_color(): fixing empty color set of ACTIVE vertices");
		}else if(pc==EMPTY_ELEM){

			cout<<endl;m_colSets[iset].bb.print(); cout<<":"<<m_colSets[iset].size<<endl;
			node_state_active.print(); cout<<endl;

			cout<<"REDUCED STACK"<<endl;
			for(int i=0; i<color_reduced_stack.pt; i++){
				m_colSets[color_reduced_stack.get_elem(i)].bb.print(); cout<<":"<<m_colSets[color_reduced_stack.get_elem(i)].size;
				cout<<endl;
			}

			cout<<"NODE STACK"<<endl;
			for(int i=0; i<node_stack.pt; i++){
				cout<<node_stack.get_elem(i)<<endl;
			}

			cout<<"CONFLICT STACK"<<endl;
			for(int i=0; i<color_conflict_stack.pt; i++){
				cout<<color_conflict_stack.get_elem(i)<<endl;
			}
						
			LOG_ERROR("fix_unit_color(): fixing color set of more than one ACTIVE vertex");
		}
	}
	

	if(v>=NB_OF_NODES){
		return fix_added_node_for_iset(v, iset);
	}else return fix_node_for_iset(v, iset);
	

  LOG_ERROR("fix_unit_color():should not reach here");
  return NO_CONFLICT;
}

template <class graph_t, class bitboard_t>
inline
int InfraOpPlus<graph_t,bitboard_t>::fix_node_for_iset(int v, int iset){
//////////////////////
// PARAMS: v: single (not weakened) ACTIVE vertex of iset
//		   iset: color set containing single active node v
//
// FUNCTION: erases active non-neighbors of v from color sets different from iset
//			 updates state of v, iset, excluded non-neighbors, adds respective color
//			 sets to stack etc.
//
// REMARKS: note that propagation only occurs in active vertices, so passive nodes in color sets will never be reduced/excluded-
//
// FUTURE RESEARCH: *** consider reversing vertex scanning (nodes in last colored vertices fixed first) ***
	
	//update state info of v and iset
	color_state_active.erase_bit(iset);
	color_passive_stack.push(iset);
	assign_node_value(v, TRUE_VAL, iset);

	//main loop of non-neighbor vertices
	bitboard_t& non_nbb= gc->get_neighbors(v);
	bitboard_t& bbcol=m_colSets[iset].bb;

	//main loop: iterates over ACTIVE--NON-NEIGHBOR nodes of v
	BITBOARD bb;
	int nnbor, offset, my_iset;
	for(int nBB=0; nBB<NB_OF_BB_NODES; nBB++){		//only real vertices, not weakened
		if( bb=(non_nbb.get_bitboard(nBB) & node_state_active.get_bitboard(nBB)) ){
			//decode vertices
			offset=WMUL(nBB);
			while(bb){
				nnbor=BitBoard::lsb64_intrinsic(bb);
				//if( (nnbor=BitBoard::lsb64_intrinsic(bb))==EMPTY_ELEM) {break;}
				bb^=Tables::mask[nnbor];
				
				nnbor+=offset;
				if( (my_iset=exclude_noneibor(nnbor,iset))!=NO_CONFLICT) {	
					return my_iset;
				}  //use nnbor
			}
		}
	}
	
	return NO_CONFLICT;
}

//template <class graph_t, class bitboard_t>
//inline
//int InfraOpPlus<graph_t,bitboard_t>::hard_fix_node_for_unit_iset(int v, int iset){
////////////////////////
//// PARAMS: v: single (not weakened) ACTIVE vertex of iset
////		   iset: unit color set containing single active node v
////
//// FUNCTION: removes (simplifies) v from color DB (v=TRUE). stores nodes related to the inference.
////        	 Context notexpected to be stored. 
////
//// REMARKS: note that propagation only occurs in active vertices, so passive nodes in color sets will never be reduced/excluded-
////
//// date of creation: 22/3/17
//
//	//update state info of v and iset
//	color_state_active.erase_bit(iset);
//	//color_passive_stack.push(iset);
//	//assign_node_value(v, TRUE_VAL, iset);
//
//	//main loop of non-neighbor vertices
//	bitboard_t& non_nbb= gc->get_neighbors(v);
//	
//	//main loop: iterates over ACTIVE--NON-NEIGHBOR nodes of v
//	BITBOARD bb;
//	int nnbor, offset, my_iset;
//	for(int nBB=0; nBB<NB_OF_BB_NODES; nBB++){		//only real vertices, not weakened
//		if( bb=(non_nbb.get_bitboard(nBB) & node_state_active.get_bitboard(nBB)) ){
//			//decode vertices
//			offset=WMUL(nBB);
//			while(bb){
//			
//				nnbor=BitBoard::lsb64_intrinsic(bb);
//				//if( (nnbor=BitBoard::lsb64_intrinsic(bb))==EMPTY_ELEM) {break;}
//				bb^=Tables::mask[nnbor];
//				
//				nnbor+=offset;
//				my_iset=node_iset_no[nnbor];
//			    m_colSets[my_iset].erase_bit(nnbor);
//				if( m_colSets[my_iset].size==0) return my_iset;
//				else if(m_colSets[my_iset].size==1){
//						color_unit_dyn_stack.push(my_iset); 
//				}
//				node_state_active.erase_bit(nnbor);
//				LOG_INFO("VERTEX REMOVED DURING HARD FIX");
//
//				//if( (my_iset=exclude_noneibor(nnbor,iset))!=NO_CONFLICT) {	
//				//	return my_iset;
//				//}  //use nnbor
//			}
//		}
//	}
//	
//	return NO_CONFLICT;
//}
//
//template <class graph_t, class bitboard_t>
//inline
//int InfraOpPlus<graph_t,bitboard_t>::hard_fix_node_for_unit_iset(int v, int iset, bitarray& bbsg){
////////////////////////
//// PARAMS: v: single (not weakened) ACTIVE vertex of iset
////		   iset: unit color set containing single active node v
////
//// FUNCTION: removes (simplifies) v from color DB (v=TRUE). stores nodes related to the inference.
////        	 Context notexpected to be stored. 
////
//// REMARKS: note that propagation only occurs in active vertices, so passive nodes in color sets will never be reduced/excluded-
////
//// date of creation: 22/3/17
//
//	//update state info of v and iset
//	color_state_active.erase_bit(iset);
//	//color_passive_stack.push(iset);
//
//	//assign_node_value(v, TRUE_VAL, iset);			/* CHECK */
//	node_state_active.erase_bit(v);
//	
//	//alternative loop based on active node enumeration: preformance is similar (25/4/17)
//	//bitboard_t& non_nbb= gc->get_neighbors(v);
//	//node_state_active.init_scan(bbo::NON_DESTRUCTIVE);
//	//while(true){
//	//	int nnbor=node_state_active.next_bit();
//	//	if(nnbor==EMPTY_ELEM) break;
//	//	if(non_nbb.is_bit(nnbor)){
//
//	//main loop of non-neighbor vertices
//	bitboard_t& non_nbb= gc->get_neighbors(v);
//	
//	//main loop: iterates over ACTIVE--NON-NEIGHBOR nodes of v
//	BITBOARD bb;
//	int nnbor, offset, my_iset;
//	for(int nBB=0; nBB<NB_OF_BB_NODES; nBB++){		//only real vertices, not weakened
//		if( bb=(non_nbb.get_bitboard(nBB) & node_state_active.get_bitboard(nBB)) ){
//			offset=WMUL(nBB);
//			while(bb){
//				nnbor=BitBoard::lsb64_intrinsic(bb);
//				if( (nnbor=BitBoard::lsb64_intrinsic(bb))==EMPTY_ELEM) {break;}
//				bb^=Tables::mask[nnbor];
//				
//				nnbor+=offset;
//
//				bbsg.erase_bit(nnbor);
//				//int my_iset=node_iset_no[nnbor];
//				my_iset=node_iset_no[nnbor];
//			    m_colSets[my_iset].erase_bit(nnbor);
//				if( m_colSets[my_iset].size==0) return my_iset;
//				else if(m_colSets[my_iset].size==1){
//					color_unit_dyn_stack.push(my_iset);					
//				}/*else if (m_colSets[my_iset].size<=3){
//					LOG_INFO("REDUCING SIZE OF COLOR DURING NODE ELIMINATION: "<<my_iset<<" size: "<<m_colSets[my_iset].size);
//					cin.get();
//				}*/
//				node_state_active.erase_bit(nnbor);
//				//LOG_INFO("VERTEX REMOVED DURING HARD FIX");
//			
//				
//
//				//if( (my_iset=exclude_noneibor(nnbor,iset))!=NO_CONFLICT) {	
//				//	return my_iset;
//				//}  //use nnbor
//			}
//		}
//	}
//	
//	return NO_CONFLICT;
//}

template <class graph_t, class bitboard_t>
inline
int InfraOpPlus<graph_t,bitboard_t>::fix_node_for_non_singleton_iset(int v, int iset){
//////////////////////
// PARAMS: v: single (not weakened) ACTIVE vertex of iset
//		   iset: color set which is the reason set but does not necessarily have to be singleton
//
// FUNCTION: similar to fix_node_for_iset, but filters out nodes from reason set iset
//
// REMARKS:
// 1. To be used when fixing nodes is not called from fix_unit_color function
	
	//update state info of v and iset
	color_state_active.erase_bit(iset);
	color_passive_stack.push(iset);
	assign_node_value(v, TRUE_VAL, iset);

	//main loop of non-neighbor vertices
	bitboard_t& non_nbb= gc->get_neighbors(v);
	bitboard_t& bbcol=m_colSets[iset].bb;

	//main loop: iterates over ACTIVE--NON-NEIGHBOR nodes of v
	BITBOARD bb;
	int nnbor, offset, my_iset;
	for(int nBB=0; nBB<NB_OF_BB_NODES; nBB++){									//only real vertices, not weakened
		if( bb=(non_nbb.get_bitboard(nBB) & node_state_active.get_bitboard(nBB) /*& (~m_colSets[iset].bb.get_bitboard(nBB))  (1)*/) ){
			//decode vertices
			offset=WMUL(nBB);
			while(bb){
				nnbor=BitBoard::lsb64_intrinsic(bb);
				//if( (nnbor=BitBoard::lsb64_intrinsic(bb))==EMPTY_ELEM) {break;}
				bb^=Tables::mask[nnbor];
				
				nnbor+=offset;
				if(node_iset_no[nnbor]==iset) continue;							/* much more efficient than (1) */
				if( (my_iset=exclude_noneibor(nnbor,iset))!=NO_CONFLICT) {	
					return my_iset;
				}  //use nnbor
			}
		}
	}
	
	return NO_CONFLICT;
}
template <class graph_t, class bitboard_t>
inline
int InfraOpPlus<graph_t,bitboard_t>::fix_added_node_for_iset(int v, int iset){
//////////////////////
// PARAMS: v: single WEAKENED vertex of iset
//		   iset: ACTIVE color set containing single WEAKENED node v
//
// FUNCTION: adds weakened variable to conflicting nodes, updates state info etc.
//			 
// REMARKS:	
	
	//update state info of v and iset
	color_state_active.erase_bit(iset);					/* note that iset can be part of the confict set of v- iset s filtered in (1) because it is set to passive*/	
	color_passive_stack.push(iset);
	assign_node_value(v, FALSE_VAL, iset);				//note it is set to FALSE for lookback
	int* isets=node_conflict_set[v];
		
	for(int c_iset=*isets; c_iset!=NONE; c_iset=*(++isets)){
		if (color_state_active.is_bit(c_iset)) {		//***check if it is really required -(1)
			m_colSets[c_iset].size--;
			color_reduced_stack.push(c_iset);
			if(m_colSets[c_iset].size==1){
				color_unit_dyn_stack.push(c_iset);
			}else if(m_colSets[c_iset].size==0){
				return c_iset;							//CONFLICT_FOUND
			}
		}
	}
	
	return NO_CONFLICT;
}

template <class graph_t, class bitboard_t>
inline
int InfraOpPlus<graph_t,bitboard_t>::exclude_noneibor(int nnbor, int reason_iset){
///////////////////////
// PARAMS: noneibor: vertex to be set to FALSE
//		   iset:	 cause of noneibor values
//
// FUNCTION: implicitly removes noneibor from its color set and updates state info
//
// REMARKS:
// 1.nnbor can belong to the reason_iset in node elimination routines when fix_node functions are called
//   on color sets that have active vertices

	int my_iset=node_iset_no[nnbor];
	assign_node_value(nnbor, FALSE_VAL, reason_iset);
	if (color_state_active.is_bit(my_iset)) {		/* nnbor could be active but its color set passive? see REMARKS*/ 
		m_colSets[my_iset].size--;
		color_reduced_stack.push(my_iset);
		if(m_colSets[my_iset].size==1){
			color_unit_dyn_stack.push(my_iset);
		}else if(m_colSets[my_iset].size==0){
			return my_iset;							//CONFLICT_FOUND
		}
	}
  return NO_CONFLICT;
}

template <class graph_t, class bitboard_t>
inline
void InfraOpPlus<graph_t,bitboard_t>::lookback_for_maxsatz(int iset){
/////////////////////
// PARAMS: iset: empty conflicting set
//
// FUNCION: Fills REASON STACK with colors related to the conflict by tracing the cause of
//			FALSE values of nodes (obviously includes added nodes)
//
// REMARKS: nodes in color sets are filtered as PASSIVE (with bitmasks)
//          added bitset for FALSE VALUE of nodes

	int offset,node;
	color_reason_stack.erase();
	color_reason_stack.push(iset);
	color_already_used[iset]=TRUE_VAL;		/* to avoid repetition */

	int my_iset;
	for(int i=0; i<color_reason_stack.pt; i++){
		my_iset=color_reason_stack.get_elem(i);

		//loop through nodes of riset (filter with passive state nodes)
		bitboard_t& cnodes=m_colSets[my_iset].bb;
		BITBOARD bb;
		for(int nBB=0; nBB<NB_OF_BB_ADDED_NODES; nBB++){
			if( (bb=cnodes.get_bitboard(nBB) & node_value_false.get_bitboard(nBB) /*&~ node_state_active.get_bitboard(nBB)*/) ){
				//decode vertices
				offset=WMUL(nBB);
				while(true){
					if( (node=BitBoard::lsb64_intrinsic(bb))==EMPTY_ELEM) {break;}
					bb^=Tables::mask[node];
					
					node+=offset;
					if(color_already_used[node_reason[node]]==FALSE_VAL  /*&& node_reason[node]!=NO_REASON  */
						/* && node_value[node]==FALSE_VAL */	){
							int reason_iset=node_reason[node];
							color_reason_stack.push(reason_iset);
							//node_reason[node]=NO_REASON;					//*** check if required 
							color_already_used[reason_iset]=TRUE_VAL;
					}
				}
			}
		}
	}

	//clear color_already_used state
	for(int i=0; i<color_reason_stack.pt; i++)
		color_already_used[color_reason_stack.get_elem(i)]=FALSE_VAL;
}

template <class graph_t, class bitboard_t>
inline
int InfraOpPlus<graph_t,bitboard_t>::enlarge_conflict_set(){
//////////////
// Enlarges color sets in REASON_STACK (involved in a conflict) by adding a new node. 
// and updates CONFLICT_STACK ending with NONE, together with node_conflict_set[ADDED_NODES]

	 int riset;
	 node_conflict_set[ADDED_NODES]= &color_conflict_stack.stack[color_conflict_stack.pt];
	 node_state_active.set_bit(ADDED_NODES);
	 for(int i=0; i<color_reason_stack.pt; i++){
		 riset=color_reason_stack.get_elem(i);
		
		 //add node
		 if( m_colSets[riset].bb.is_bit(ADDED_NODES)){
			 LOG_ERROR("InfraOpPlus::enlarge_conflict_set()-when enlarging color set "<<riset<<" with node "<<ADDED_NODES);
			 LOG_ERROR("maximum node to encode enlargement is "<<NB_OF_BB_ADDED_NODES*WORD_SIZE-1);
			 //m_colSets[riset].bb.print(); 
			 //print_db(true, true);
			 cin.get();
		 }
		 m_colSets[riset].push(ADDED_NODES);
		 color_enlarged_stack.push(riset);
		 color_conflict_stack.push(riset);
	 }

	 color_conflict_stack.push(NONE);		//ends in NONE for fix_added_node_for_iset(..) to work
	 ADDED_NODES++;
	/* if(number_of_added_nodes()>50){
		 LOG_INFO("DANGER NB ADDED NODE:"<<50);
	 }*/
	 return 0;
}

template <class graph_t, class bitboard_t>
inline
void InfraOpPlus<graph_t,bitboard_t>::assign_node_value(int v, int val, int color_reason){
	node_value[v]=val; 
	node_value_false.set_bit(v);				//for bitmasking (2/12/16)
	node_state_active.erase_bit(v);
	node_stack.push(v);
	node_reason[v]=color_reason;
}

template <class graph_t, class bitboard_t>
inline
void InfraOpPlus<graph_t,bitboard_t>::reset_context_for_maxsatz( int saved_node_stack_pt, 
									int saved_color_passive_stack_pt, 
									int save_color_reduced_stack_pt,
									int save_color_unit_stack_pt		){
///////////////////
// PARAMS:	<xxxx> reference in stacks to restore. Also STACKS will be deleted up to that point
//
// FUNCTION: restores state from STACKS and then cleans STACKS
// 
// REMARKS: 
// 1. color_enlarged_stack is not restored! (i.e added nodes remain)
// 2. Empties color_unit_dyn_stack									

  //node state and NODE_STACK
  int node;									
  for(int i=saved_node_stack_pt; i<node_stack.pt; i++) {
	  node=node_stack.get_elem(i);
	  node_state_active.set_bit(node);
	  node_value_false.erase_bit(node);
	  node_reason[node]=NO_REASON;
	  //node_value[node]
  }
  node_stack.pt=saved_node_stack_pt;

  //color state and passive_color_stack
   for(int i=saved_color_passive_stack_pt; i<color_passive_stack.pt; i++) {
	   color_state_active.set_bit(color_passive_stack.get_elem(i));
   }
   color_passive_stack.pt=saved_color_passive_stack_pt;

   //color DB and reduced color stack
    for(int i=save_color_reduced_stack_pt; i<color_reduced_stack.pt; i++) {
		m_colSets[color_reduced_stack.get_elem(i)].size++;
	}
	color_reduced_stack.pt=save_color_reduced_stack_pt;
	
	//stack-only operations
	color_unit_stack.pt=save_color_unit_stack_pt;
	color_unit_dyn_stack.pt=0;		
}


template <class graph_t, class bitboard_t>
inline
void InfraOpPlus<graph_t,bitboard_t>::reset_enlarged_isets(){
///////////////////
// resets context for enlarged sets:
// 1. sets them to ACTIVE
// 2. reduces sets size by one
// 3. removes ADDED_NODES from sets
// 4. clears ENLARGED_STACK
// 5. resets ADDED_NODES index
	
	int iset;
	for(int i=0; i<color_enlarged_stack.pt; i++) {
		iset=color_enlarged_stack.get_elem(i);	
		color_state_active.set_bit(iset);							
		m_colSets[iset].size--;										
		m_colSets[iset].bb.get_bitboard(NB_OF_BB_NODES)=0;			/*deletes added nodes*/
	
	}

	node_state_active.get_bitboard(NB_OF_BB_NODES)=0;				
	color_enlarged_stack.pt=0;
	reset_nb_added_nodes();
}


template <class graph_t, class bitboard_t>
inline
void InfraOpPlus<graph_t,bitboard_t>::reset_enlarged_isets_lazy(){
///////////////////
// resets context forenlarged sets:
// 1. removes ADDED_NODES from sets
// 2. clears ENLARGED_STACK
// 3. resets ADDED_NODES index
//
// REMARKS: conceived as a fast context reset. Assumes that
//          the color_db will be renewed completely in next usate

	int iset;
	for(int i=0; i<color_enlarged_stack.pt; i++) {
		m_colSets[color_enlarged_stack.get_elem(i)].bb.get_bitboard(NB_OF_BB_NODES)=0;			
	
	}
	node_state_active.get_bitboard(NB_OF_BB_NODES)=0;	
	color_enlarged_stack.pt=0;
	reset_nb_added_nodes();
}

template <class graph_t, class bitboard_t>
inline
int InfraOpPlus<graph_t,bitboard_t>::enlarge_stored_involved_sets(){
//////////////
// Enlarges color sets in INVOLVED_STACK (involved in a conflict) by adding a new node. 
// and updates CONFLICT_STACK ending with NONE, together with node_conflict_set[ADDED_NODES]
//
// Used when all nodes of a non-unit color set are found conflicting

	 int riset;
	 node_conflict_set[ADDED_NODES]= &color_conflict_stack.stack[color_conflict_stack.pt];
	 node_state_active.set_bit(ADDED_NODES);
	 for(int i=0; i<color_involved_stack.pt; i++){
		 riset=color_involved_stack.get_elem(i);

		 if(color_already_used[riset]==FALSE_VAL){		//color not previously added
			 color_already_used[riset]=TRUE_VAL;			//set as used
			 
			 //add node
			 if( m_colSets[riset].bb.is_bit(ADDED_NODES)){
				 LOG_ERROR("InfraOpPlus::enlarge_stored_involved_sets()-when enlarging color set "<<riset<<" with node "<<ADDED_NODES);
				 LOG_ERROR("maximum node to encode enlargement is "<<NB_OF_BB_ADDED_NODES*WORD_SIZE-1);
				// m_colSets[riset].bb.print(); 
				 cin.get();

			 }
			 m_colSets[riset].push(ADDED_NODES);
			 color_enlarged_stack.push(riset);
			 color_conflict_stack.push(riset);
		 }
	 }

	 color_conflict_stack.push(NONE);		//ends in NONE for fix_added_node_for_iset(..) to work
	 ADDED_NODES++;

	 //restore context for USED and INVOLVED nodes and empty color_involved_stack
	 for(int i=0; i<color_involved_stack.pt; i++){
		 color_already_used[color_involved_stack.get_elem(i)]=FALSE_VAL;
		 color_involved_state[color_involved_stack.get_elem(i)]=FALSE_VAL;
	 }

	 color_involved_stack.erase();		
	 return 0;
 }

template <class graph_t, class bitboard_t>
inline
void InfraOpPlus<graph_t,bitboard_t>::store_involved_sets(){
///////////////
// REASON_STACK to INVOLVED_SET_STACK and iset_used[]<--TRUE

  int iset;
  for(int i=0; i<color_reason_stack.pt; i++){
	iset=color_reason_stack.get_elem(i);
	color_involved_stack.push(iset);
	color_involved_state[iset]=TRUE_VAL;
  }
}

template <class graph_t, class bitboard_t>
inline
int InfraOpPlus<graph_t,bitboard_t>::test_node(int node, int iset, bool is_last_node){
///////////////////////////
// PARAMS: node from iset to set to TRUE and make UL inferences
//		   further: control further tests if the node is the last (and only) non-conflicting node in iset
//	sets v in iset to FALSE hoping to get lucky and find conflict
//
// RETURNS: iset conflicting (size=0) or NO_CONFLICT
// REMARKS: note this is a partial conflict of vertex v in non-singleton iset

	int my_iset;
	int save_color_unit_stack_pt=color_unit_stack.pt;				
	color_unit_dyn_stack.pt=0;

	//test-remove RELEASE
	//if(!node_state_active.is_bit(node)){
	//	LOG_ERROR("InfraOpPlus::test_node-node inactive");
	//	return NO_CONFLICT;
	//}

	if( (my_iset=fix_node_for_non_singleton_iset(node, iset)) != NO_CONFLICT || 
		(my_iset=unit_iset_process_for_tests()) !=  NO_CONFLICT 
#ifdef FURTHER_TEST_FAILED_COLOR_SETS
		|| 	(is_last_node && (my_iset=further_test(0)) != NO_CONFLICT)
#endif		
		){ /*CONFLICT related to node*/
			lookback_for_maxsatz(my_iset);
			reset_context_for_maxsatz(0, 0, 0, save_color_unit_stack_pt);
			return my_iset;

	}else{ /*NO CONFLICT*/
		//resets context and memorizes non-conflicting TRUE nodes to stop re-testing
		reset_context_for_maxsatz_node(0, 0, 0, save_color_unit_stack_pt);
	}

  	return NO_CONFLICT;
}

template <class graph_t, class bitboard_t>
inline
int InfraOpPlus<graph_t,bitboard_t>::unit_iset_process_for_tests(){
///////////////////////////
// specific UL inference for testing nodes:
// 1. First unit USED color sets are examined (i.e. those that already belong to a partial conflict; this aims at reducing the size of conflicts)
// 2. Then, for each unit UNUSED color set, new USED nodes are always checked
//	  The process ends when all USED color sets are processed//
//
// REMARKS
// called after fix_node_for_iset(v, iset) has been called, so all related nodes are 
// in color_unit_dyn_stack (not color_unit_stack )
	
	int j, iset, my_iset, iset_start=0, used_iset_start=0;
	do{
		//tests active nodes USED in previous CONFLICTS of related tested nodes
		for(j=used_iset_start; j<color_unit_dyn_stack.pt; j++) {
			iset=color_unit_dyn_stack.get_elem(j);
			if(color_state_active.is_bit(iset) && color_involved_state[iset]==TRUE_VAL){
				if ((my_iset=fix_unit_color(iset))!=NO_CONFLICT)						//iset is turned passive (*)
					return my_iset;
			}
		}
		used_iset_start=j;

		//tests remaining nodes (not necessarily USED)
		for(j=iset_start; j<color_unit_dyn_stack.pt; j++) {
			iset=color_unit_dyn_stack.get_elem(j);
			if(color_state_active.is_bit(iset)){										//used isets cannot be selected here(*)
				if ((my_iset=fix_unit_color(iset))!=NO_CONFLICT){
					return my_iset;
				}
				iset_start=j+1;
				break;
			}
		}
	}while (j<color_unit_dyn_stack.pt);

	return NO_CONFLICT;
}

template <class graph_t, class bitboard_t>
inline
int  InfraOpPlus<graph_t,bitboard_t>::unit_iset_process_for_test_by_eliminating_failed_nodes(){
/////////////////
// simple inference engine over color_unit_dyn_stack
	int iset, my_iset;
	for(int j=0; j<color_unit_dyn_stack.pt; j++) {
		iset=color_unit_dyn_stack.get_elem(j);
		if(color_state_active.is_bit(iset)){						/* proving test unnecessary will be difficult */
			if ((my_iset=fix_unit_color(iset))!=NO_CONFLICT)						
				return my_iset;
		}
	}

	return NO_CONFLICT;
}

//template <class graph_t, class bitboard_t>
//inline
//int  InfraOpPlus<graph_t,bitboard_t>::unit_iset_process_for_test_by_eliminating_failed_nodes_csp(){
///////////////////
//// simple inference engine over color_unit_dyn_stack
//	int iset, my_iset;
//	for(int j=0; j<color_unit_dyn_stack.pt; j++) {
//		iset=color_unit_dyn_stack.get_elem(j);
//		if(color_state_active.is_bit(iset)){						/* proving test unnecessary will be difficult */
//			int v=FIRST_SHARED(m_colSets[iset].bb,node_state_active);		
//			if ((my_iset=fix_node_for_iset(v, iset))!=NO_CONFLICT)						
//				return my_iset;
//		}
//	}
//
//	return NO_CONFLICT;
//}

//template <class graph_t, class bitboard_t>
//inline
//int  InfraOpPlus<graph_t,bitboard_t>::unit_iset_process_for_test_csp(bitarray& bbsg, sbb_t<bitarray>& s){
///////////////////
//// simple inference engine over color_unit_dyn_stack
//
//	int iset, my_iset;
//	for(int j=0; j<color_unit_dyn_stack.pt; j++) {
//		iset=color_unit_dyn_stack.get_elem(j);
//		if(color_state_active.is_bit(iset)){						/* proving test unnecessary will be difficult */
//			//int v=FIRST_SHARED(m_colSets[iset].bb, node_state_active);	
//			int v=m_colSets[iset].bb.lsbn64();
//
//			s.push(v);
//			if ((my_iset=hard_fix_node_for_unit_iset(v,iset, bbsg))!=NO_CONFLICT )
//						return my_iset;
//			
//		}
//	}
//
//	return NO_CONFLICT;
//}


template <class graph_t, class bitboard_t>
inline
int InfraOpPlus<graph_t,bitboard_t>::maxsatz_lookahead(int nb_conflict, int max_num_conf){
////////////////////////////
// Driver for testing color sets of small size (greater than one)
//
// PARAMS: nb_conflicts-current number of conflicts, max_num_conf=UB-LB
//
// RETURNS the total number of conflicts: nb_conflict + new conflicts found
//
// REMARKS:
// 1.no checking of active nodes from colors (assumes color_db is consistent)
// 2.max_num_conf>nb_conflict
// 3.currently does not reset TESTED_NODES (3/12/16)

	//int nodes_chosen_set[MAX_COLOR_TEST_LENGTH];

	//test- remove in RELEASE
	if(max_num_conf<=nb_conflict){
		LOG_ERROR("InfraOpPlus::maxsatz_lookahead-bizarre call, nb of conflicts already enough");
		return 0;
	}
	
	bool test_flag, no_conflict_flag;
	//reset_tested_nodes();														/*all nodes clean for testing*/
	
	//test-remove in RELEASE
	/*if(!color_contains_tested_nodes.is_empty()){
		LOG_ERROR("InfraOpPlus::maxsatz_lookahead-bizarre number of tested nodes");
	}*/

	//main loop: small color sizes
	///int diff=max_num_conf-nb_conflict;
	for(int k=2; k<=MAX_COLOR_TEST_LENGTH; k++){
		for(int iset=NB_OF_COLORS; iset>=(max_num_conf-nb_conflict); iset--){	
			if( m_colSets[iset].size==k && !m_colSets[iset].bb.get_bitboard(NB_OF_BB_NODES) /*not WEAKENED*/ && color_state_active.is_bit(iset) 				
				 && !color_contains_tested_nodes.is_bit(iset) ){
						
					 
					color_involved_stack.erase();					/* cleans the stack for the colors */

					//loop through all nodes and test them
					bitboard_t& bbcol=m_colSets[iset].bb;
					bbcol.init_scan(bbo::NON_DESTRUCTIVE);
					no_conflict_flag=false;						
					for(int i=0; i<k; i++){														
						if(test_node(/*nodes_chosen_set[i]*/ bbcol.next_bit() /* careful, color should not be scanned inside !*/, iset, (i==k-1) /* is_last*/)==NO_CONFLICT){			//no need to check if already tested again
							no_conflict_flag=true;					//no conflict
							break;
						}else{				//conflict for this particular node
							store_involved_sets();	
						}
					}

					//print_test_non_unit_clause_info();

					//conflict found for all nodes: the REAL CONFLICT
					if(no_conflict_flag==false){
						enlarge_stored_involved_sets();		   /* also 	color_involved_state-->FALSE_VAL*/
						//LOG_INFO("TEST CONFLICT FOUND");
						if( ++nb_conflict >= max_num_conf){
							return nb_conflict;				//CUT FOUND
						}
					}

					//context operations for next color tests
					reset_involved_isets();				/* color_involved_state-->FALSE_VAL */
				
				
			}//next color to test
		}
	}
		
	return nb_conflict;
}

template <class graph_t, class bitboard_t>
inline
int InfraOpPlus<graph_t,bitboard_t>::further_test(int start){
///////////////////////////
// PARAMS: start-starting position from REDUCED_STACK from where to select candidate 
//		   conflicting colors
//
// FUNCTION: tests colors from reduced stack for conflicts
// APPLICATION: used when the last node from a tested color is found non-conflicting
//
// RETURNS: conflicting color set from the REDUCED_STACK or NO_CONFLICT
//
// COMMENTS: candidate color sets from REDUCED_STACK cannot have been WEAKENED
//

	int my_iset, chosen_iset;
	int save_color_unit_stack_pt=color_unit_stack.pt;				
	int saved_node_stack_pt=node_stack.pt;						
	int saved_color_passive_stack_pt=color_passive_stack.pt;		
	int save_color_reduced_stack_pt=color_reduced_stack.pt;	
	int node;

	color_already_used_in_extended_test.erase_bit();							/*avoids repetition of color in this test (REDUCED STACK contains repeated colors)*/
	/*for(int i=start; i<save_color_reduced_stack_pt; i++){
		color_already_used_in_extended_test.is_bit(color_reduced_stack.get_elem(i));
	}*/
	
	//main loop
	for(int i=start; i<save_color_reduced_stack_pt; i++) {						
		chosen_iset=color_reduced_stack.get_elem(i);
		if(!color_already_used_in_extended_test.is_bit(chosen_iset) && color_state_active.is_bit(chosen_iset) 
			&& m_colSets[chosen_iset].size==MAX_COLOR_FURTHER_TEST_LENGTH									/*must always be greater than 1, MAX_COLOR_FURTHER_TEST_LENGTH is typically 2 */
			&& m_colSets[chosen_iset].bb.get_bitboard(NB_OF_BB_NODES)==0/* not weakened*/){
			
			color_already_used_in_extended_test.set_bit(chosen_iset);

			//loop to test all (both) nodes
			bitboard_t bbcol=m_colSets[chosen_iset].bb;
			bbcol.init_scan(bbo::NON_DESTRUCTIVE);
			for(int j=0; j<MAX_COLOR_FURTHER_TEST_LENGTH; j++){
				do{
					node=bbcol.next_bit();											/*careful, should not be scanned inside fix_node... and unit_iset_process*/
				}while(!node_state_active.is_bit(node));							/* active nodes HAVE to be filtered here, could have been set to FALSE before*/
				color_unit_dyn_stack.erase();
				if ( (my_iset=fix_node_for_non_singleton_iset(node, chosen_iset)) !=NO_CONFLICT ||
					 (my_iset=unit_iset_process_for_tests()) != NO_CONFLICT  ) {
						
						color_already_used[chosen_iset]=TRUE_VAL;					/*excludes current reduced color set because it will be included in the inference of previous level (1)*/
						lookback_for_maxsatz(my_iset);
						color_already_used[chosen_iset]=FALSE_VAL;
						reset_context_for_maxsatz(saved_node_stack_pt,
												saved_color_passive_stack_pt,
												save_color_reduced_stack_pt,
												save_color_unit_stack_pt );
						store_involved_sets();										/* chosen_iset will not be involved here (1) */
				}else{ /* NO CONFLICT */
					reset_context_for_maxsatz(saved_node_stack_pt,
											saved_color_passive_stack_pt,
											save_color_reduced_stack_pt,
											save_color_unit_stack_pt );
					break;
				}

				if(j== MAX_COLOR_FURTHER_TEST_LENGTH-1){							/* CONFLICT FOUND FOR ALL NODES- global conflict */
					//LOG_INFO("FURTHER TEST SUCCESFUL");
					return chosen_iset;
				}
			}
		}
	}

  return  NO_CONFLICT;
}

//template <class graph_t, class bitboard_t>
//inline
//int InfraOpPlus<graph_t,bitboard_t>::further_test_csp(int start){
/////////////////////////////
//// PARAMS: start-starting position from REDUCED_STACK from where to select candidate 
////		   conflicting colors
////
//// FUNCTION: tests colors from reduced stack for conflicts
//// APPLICATION: used when the last node from a tested color is found non-conflicting
////
//// RETURNS: conflicting color set from the REDUCED_STACK or NO_CONFLICT
////
//// COMMENTS: candidate color sets from REDUCED_STACK cannot have been WEAKENED
////
//
//	int my_iset, chosen_iset;
//	int save_color_unit_stack_pt=color_unit_stack.pt;				
//	int saved_node_stack_pt=node_stack.pt;						
//	int saved_color_passive_stack_pt=color_passive_stack.pt;		
//	int save_color_reduced_stack_pt=color_reduced_stack.pt;	
//	int node;
//
//	color_already_used_in_extended_test.erase_bit();							/*avoids repetition of color in this test (REDUCED STACK contains repeated colors)*/
//	/*for(int i=start; i<save_color_reduced_stack_pt; i++){
//		color_already_used_in_extended_test.is_bit(color_reduced_stack.get_elem(i));
//	}*/
//	
//	//main loop
//	for(int i=start; i<save_color_reduced_stack_pt; i++) {						
//		chosen_iset=color_reduced_stack.get_elem(i);
//		if(!color_already_used_in_extended_test.is_bit(chosen_iset) && color_state_active.is_bit(chosen_iset) 
//			&& m_colSets[chosen_iset].size==MAX_COLOR_FURTHER_TEST_LENGTH									/*must always be greater than 1, MAX_COLOR_FURTHER_TEST_LENGTH is typically 2 */
//			/*&& m_colSets[chosen_iset].bb.get_bitboard(NB_OF_BB_NODES)==0/* not weakened*/){
//			
//			color_already_used_in_extended_test.set_bit(chosen_iset);
//
//			//loop to test all (both) nodes
//			bitboard_t bbcol=m_colSets[chosen_iset].bb;
//			bbcol.init_scan(bbo::NON_DESTRUCTIVE);
//			for(int j=0; j<MAX_COLOR_FURTHER_TEST_LENGTH; j++){
//				do{
//					node=bbcol.next_bit();											/*careful, should not be scanned inside fix_node... and unit_iset_process*/
//				}while(!node_state_active.is_bit(node));							/* active nodes HAVE to be filtered here, could have been set to FALSE before*/
//				color_unit_dyn_stack.erase();
//				if ( (my_iset=fix_node_for_non_singleton_iset(node, chosen_iset)) !=NO_CONFLICT ||
//					(my_iset= unit_iset_process_for_test_by_eliminating_failed_nodes() ) != NO_CONFLICT  ) {
//						
//					//	color_already_used[chosen_iset]=TRUE_VAL;					/*excludes current reduced color set because it will be included in the inference of previous level (1)*/
//					//	lookback_for_maxsatz(my_iset);
//					//	color_already_used[chosen_iset]=FALSE_VAL;
//						reset_context_for_maxsatz(saved_node_stack_pt,
//												saved_color_passive_stack_pt,
//												save_color_reduced_stack_pt,
//												save_color_unit_stack_pt );
//					//	store_involved_sets();										/* chosen_iset will not be involved here (1) */
//				}else{ /* NO CONFLICT */
//					reset_context_for_maxsatz(saved_node_stack_pt,
//											saved_color_passive_stack_pt,
//											save_color_reduced_stack_pt,
//											save_color_unit_stack_pt );
//					break;
//				}
//
//				if(j== MAX_COLOR_FURTHER_TEST_LENGTH-1){							/* CONFLICT FOUND FOR ALL NODES- global conflict */
//					//LOG_INFO("FURTHER TEST SUCCESFUL");
//					return chosen_iset;
//				}
//			}
//		}
//	}
//
//  return  NO_CONFLICT;
//}

template <class graph_t, class bitboard_t>
inline
void InfraOpPlus<graph_t,bitboard_t>::reset_context_for_maxsatz_node( int saved_node_stack_pt, 
									int saved_color_passive_stack_pt, 
									int save_color_reduced_stack_pt,
									int save_color_unit_stack_pt		){
///////////////////
// PARAMS:	<xxxx> reference in stacks to restore. Also STACKS will be deleted up to that point
//
// FUNCTION: restores state from STACKS and then cleans STACKS.
//			 compared with basic reset_context_for_maxsatz it also sets TESTED_NODE_STACK
//			 with nodes set to TRUE in NODE_STACK
//
// 
// REMARKS: 
// 1. color_enlarged_stack is not restored!
// 2. Empties color_unit_dyn_stack									

  //node state and NODE_STACK
  int node;									
  for(int i=saved_node_stack_pt; i<node_stack.pt; i++) {
	  node=node_stack.get_elem(i);
	  node_state_active.set_bit(node);
	  node_value_false.erase_bit(node);
	  node_reason[node]=NO_REASON;
	  if (node_value[node]==TRUE_VAL && node_tested_state[node]==FALSE_VAL) {
		  node_tested_state[node]=TRUE_VAL;			// no need to re-test at this point
		  node_tested_stack.push(node);
		  color_contains_tested_nodes.set_bit(node_iset_no[node]);
	  }
  }
  node_stack.pt=saved_node_stack_pt;

  //color state and passive_color_stack
   for(int i=saved_color_passive_stack_pt; i<color_passive_stack.pt; i++) {
	   color_state_active.set_bit(color_passive_stack.get_elem(i));
   }
   color_passive_stack.pt=saved_color_passive_stack_pt;

   //color DB and reduced color stack
    for(int i=save_color_reduced_stack_pt; i<color_reduced_stack.pt; i++) {
		m_colSets[color_reduced_stack.get_elem(i)].size++;
	}
	color_reduced_stack.pt=save_color_reduced_stack_pt;
	
	//stack-only operations
	color_unit_stack.pt=save_color_unit_stack_pt;
	color_unit_dyn_stack.pt=0;		
}


template <class graph_t, class bitboard_t>
inline
int InfraOpPlus<graph_t,bitboard_t>::test_node_for_failed_nodes(int node, int iset){
/////////////////////////
// Attempts to prove v FAILED
// RETURNS: Conflicting (empty) iset or NO_CONFLICT
//
// APPLICATION: called by test which tries to show conflicts by eliminating FAILED nodes
//
	int my_iset;
	//saved pointers necessary to store FAILED NODES in previous inferences
	//note that the STACKS are NOT empty here, since they contain possible 
	//reduced colors of setting a node to TRUE
	int save_color_unit_stack_pt=color_unit_stack.pt;	
	int saved_node_stack_pt=node_stack.pt;							
	int saved_color_passive_stack_pt=color_passive_stack.pt;		
	int save_color_reduced_stack_pt=color_reduced_stack.pt;			
	color_unit_dyn_stack.erase();
	if ((my_iset=fix_node_for_non_singleton_iset(node, iset)) == NO_CONFLICT) {	
#ifdef FURTHER_TEST_ELIMINATE_FAILED_NODES
		if ((my_iset=unit_iset_process_for_test_by_eliminating_failed_nodes()) == NO_CONFLICT)
			  my_iset=simple_further_test(save_color_reduced_stack_pt);								/* propagation through REDUCED_STACK */
#else
			my_iset=unit_iset_process_for_tests();
#endif
	}

	 reset_context_for_maxsatz(saved_node_stack_pt,
								saved_color_passive_stack_pt,
								save_color_reduced_stack_pt,
								save_color_unit_stack_pt);
	return my_iset;
}

//template <class graph_t, class bitboard_t>
//inline
//int InfraOpPlus<graph_t,bitboard_t>::test_node_for_failed_nodes_csp(int node, int iset){
///////////////////////////
//// Attempts to prove v FAILED
//// RETURNS: Conflicting (empty) iset or NO_CONFLICT
////
//// APPLICATION: called by test which tries to show conflicts by eliminating FAILED nodes
////
//	int my_iset;
//	//saved pointers necessary to store FAILED NODES in previous inferences
//	//note that the STACKS are NOT empty here, since they contain possible 
//	//reduced colors of setting a node to TRUE
//	int save_color_unit_stack_pt=color_unit_stack.pt;	
//	int saved_node_stack_pt=node_stack.pt;							
//	int saved_color_passive_stack_pt=color_passive_stack.pt;		
//	int save_color_reduced_stack_pt=color_reduced_stack.pt;			
//	color_unit_dyn_stack.erase();
//	if ((my_iset=fix_node_for_non_singleton_iset(node, iset)) == NO_CONFLICT) {	
//#ifdef FURTHER_TEST_ELIMINATE_FAILED_NODES
//		if ((my_iset=unit_iset_process_for_test_by_eliminating_failed_nodes_csp()) == NO_CONFLICT)
//			  my_iset=simple_further_test(save_color_reduced_stack_pt);								/* propagation through REDUCED_STACK */
//#else
//			my_iset=unit_iset_process_for_tests();
//#endif
//	}
//
//	 reset_context_for_maxsatz(saved_node_stack_pt,
//								saved_color_passive_stack_pt,
//								save_color_reduced_stack_pt,
//								save_color_unit_stack_pt);
//	return my_iset;
//}

//
//template <class graph_t, class bitboard_t>
//inline
//bool InfraOpPlus<graph_t,bitboard_t>::test_by_eliminate_failed_nodes_csp (bitarray& bbsg, sbb_t<bitarray>& s){
/////////////
//// Driver to find ONE conflict by proving all FAILED nodes in a color set.
//// Once a literal is proven failed, it is assigned FALSE and removed from
//// its color set. This may in turn fire UL inferences (when only one literal is left) which, if succesful, 
//// permanently modify color_DB in further inferences.
//// 
//// Currently applied to ALL ACTIVE COLORS (enlarged or not)
////
//// RETURNS: TRUE if a conflict has been found or FALSE otherwise
////
//// REMARKS: 
//// 1. Possibly filter tested colors by size 
//// 2. Possibly stop working on a color after a number of nodes have not been proved FALSE
////
//// COMMENTS: Passing subgraph, deletion of conflicting nodes not working!!!
////
//// TESTING ELIMINATING ONLY THE LAST NODE WHICH IS IN UNIT COLOR NB_OF_COLORS
//
//	int my_iset, nb_partial_conf, node, save_color_unit_stack_pt=color_unit_stack.pt;	
//	bool conflict=false;
//		
//	BITBOARD bb;
//	do{
//		nb_partial_conf=0;
//		for(int k=4; k>=2; k--){
//			for(int iset=NB_OF_COLORS/*-1 if it is known that the last color is an already tested singleton */; iset>=1; iset--){
//				//for(int iset=1; iset>=NB_OF_COLORS; iset++){
//				//if(m_colSets[iset].size<=4 /* seems best for many CSPs */ && color_state_active.is_bit(iset) ){	
//				if(m_colSets[iset].size==k /* seems best for many CSPs */ && color_state_active.is_bit(iset) ){	
//					if(m_colSets[iset].size==1){
//						LOG_ERROR("BIZARRE-TESTING SINGLE COLOR FOR NODE ELIMINATION");
//						cin.get();
//					}
//					conflict=false;
//					color_unit_dyn_stack.erase();
//
//					//attempt to prove FALSE (eliminate) all (not ADDED) nodes from the iset
//					for(int nBB=0; nBB<NB_OF_BB_NODES; nBB++){														/*only not added nodes*/
//						if( bb=(m_colSets[iset].bb.get_bitboard(nBB) & node_state_active.get_bitboard(nBB)) ){
//							while(bb){
//								node=BitBoard::lsb64_intrinsic(bb);
//								bb^=Tables::mask[node];
//								node+=WMUL(nBB);
//
//								/////////////////////
//								//testing node
//								if( test_node_for_failed_nodes_csp(node, iset)!=NO_CONFLICT){
//									//	if( test_node_for_failed_nodes(node, iset)!=NO_CONFLICT){
//									nb_partial_conf++;		
//
//									//remove conflicting node
//									bbsg.erase_bit(node);
//									node_state_active.erase_bit(node);
//									m_colSets[iset].erase_bit(node);						/* alternatively m_colSets[iset].size--;	*/
//
//									if(m_colSets[iset].size==0)	{		
//										return true;										/* conflict found: all values conflicting  */
//									}
//								}
//
//								//////////////////////
//								/* no conflict- continues with other nodes of same color in the same bitblock*/
//
//							}//next node in same block
//
//						}
//						/* no conflict- continues with other nodes of same color in another bitblock*/
//
//					}//new block to test nodes
//
//					//if active color has turned single by elimination of values, then hard-propagates the unit color, making permanent changes 
//					if(m_colSets[iset].size==1){
//						//LOG_INFO("unit color set expansion during test eliminate nodes: "<<iset);
//						color_unit_dyn_stack.erase();
//						color_unit_dyn_stack.push(iset);
//						if(unit_iset_process_for_test_csp(bbsg, s)!=NO_CONFLICT ){
//							//LOG_INFO("InfraOpPlus<graph_t,bitboard_t>::test_by_eliminate_failed_nodes_csp-unit color derived when removing nodes");
//							return true;	
//						}
//					}
//				}//filter color_state active
//
//			}//next color to test
//		}//next color size
//	}while(nb_partial_conf>1 /* >=1 increases so much overhead?, check*/ );
//
////	reset_context_for_maxsatz(0,0,0,save_color_unit_stack_pt);	/* TODO see if needed and why */
//	
//	return(conflict);
//}

//template <class graph_t, class bitboard_t>
//inline
//bool InfraOpPlus<graph_t,bitboard_t>::test_by_eliminate_failed_nodes_csp (bitarray& bbsg, sbb_t<bitarray>& s, com::stack_t<int>& cand){
/////////////
//// Driver to find ONE conflict by proving all FAILED nodes in a color set.
//// Once a literal is proven failed, it is assigned FALSE and removed from
//// its color set. This may in turn fire UL inferences (when only one literal is left) which, if succesful, 
//// permanently modify color_DB in further inferences.
//// 
//// Currently applied to ALL ACTIVE COLORS (enlarged or not)
////
//// RETURNS: TRUE if a conflict has been found or FALSE otherwise
////
//// REMARKS: 
//// 1. Possibly filter tested colors by size 
//// 2. Possibly stop working on a color after a number of nodes have not been proved FALSE
////
//// COMMENTS: Passing subgraph, deletion of conflicting nodes not working!!!
////
//// TESTING ELIMINATING ONLY THE LAST NODE WHICH IS IN UNIT COLOR NB_OF_COLORS
//
//	int my_iset, nb_partial_conf, node, save_color_unit_stack_pt=color_unit_stack.pt;	
//	bool conflict=false;
//		
//	BITBOARD bb;
//	do{
//		nb_partial_conf=0;
//		for(int iset=NB_OF_COLORS /*-1 if it is known that the last color is an already tested singleton */; iset>=1; iset--){
//			if(m_colSets[iset].size==2 /* seems best for many CSPs */ && color_state_active.is_bit(iset) ){	
//				if(m_colSets[iset].size==1){
//					LOG_ERROR("BIZARRE-TESTING SINGLE COLOR FOR NODE ELIMINATION");
//					cin.get();
//				}
//				conflict=false;
//				color_unit_dyn_stack.erase();
//
//				//attempt to prove FALSE (eliminate) all (not ADDED) nodes from the iset
//				for(int nBB=0; nBB<NB_OF_BB_NODES; nBB++){														/*only not added nodes*/
//					if( bb=(m_colSets[iset].bb.get_bitboard(nBB) & node_state_active.get_bitboard(nBB)) ){
//						while(bb){
//							node=BitBoard::lsb64_intrinsic(bb);
//							bb^=Tables::mask[node];
//							node+=WMUL(nBB);
//												
//							/////////////////////
//							//testing node
//							if( test_node_for_failed_nodes_csp(node, iset)!=NO_CONFLICT){
//						//	if( test_node_for_failed_nodes(node, iset)!=NO_CONFLICT){
//								nb_partial_conf++;		
//
//								//remove conflicting node
//								bbsg.erase_bit(node);
//								node_state_active.erase_bit(node);
//								m_colSets[iset].erase_bit(node);						/* alternatively m_colSets[iset].size--;	*/
//												
//								if(m_colSets[iset].size==0)	{		
//									return true;										/* conflict found: all values conflicting  */
//								}
//							}
//
//							//////////////////////
//							/* no conflict- continues with other nodes of same color in the same bitblock*/
//							
//						}//next node in same block
//											
//					}
//					/* no conflict- continues with other nodes of same color in another bitblock*/
//									
//				}//new block to test nodes
//
//				//if active color has turned single by elimination of values, then hard-propagates the unit color, making permanent changes 
//				if(m_colSets[iset].size==1){
//					//LOG_INFO("unit color set expansion during test eliminate nodes: "<<iset);
//					color_unit_dyn_stack.erase();
//					color_unit_dyn_stack.push(iset);
//					if(unit_iset_process_for_test_csp(bbsg, s)!=NO_CONFLICT ){
//						//LOG_INFO("InfraOpPlus<graph_t,bitboard_t>::test_by_eliminate_failed_nodes_csp-unit color derived when removing nodes");
//						return true;	
//					}
//				}else {cand.stack[0]=FIRST_SHARED(m_colSets[iset].bb, node_state_active); cand.pt++;};	
//			}//filter color_state active
//	
//		}//next color to test
//	}while(nb_partial_conf>1 /* >=1 increases so much overhead?, check*/ );
//
////	reset_context_for_maxsatz(0,0,0,save_color_unit_stack_pt);	/* TODO see if needed and why */
//	
//	return(conflict);
//}

template <class graph_t, class bitboard_t>
inline
bool InfraOpPlus<graph_t,bitboard_t>::test_by_eliminate_failed_nodes(bool lazy_context/*, int c_low, int c_high*/){
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

	int my_iset, nb_partial_conf, node, save_color_unit_stack_pt=color_unit_stack.pt;	
	bool conflict=false;
		
	BITBOARD bb;
	do{
		nb_partial_conf=0;
		for(int iset=NB_OF_COLORS /* -1 if it is known that the last color is an already tested singleton */; iset>=1; iset--){
			/*print_db(true, true);
			print_stack(NODES);*/
			
			if(color_state_active.is_bit(iset) /*&& size filter */){				
				conflict=false;
				color_unit_dyn_stack.erase();

				//attempt to prove FALSE (eliminate) all (not ADDED) nodes from the iset
				for(int nBB=0; nBB<NB_OF_BB_NODES; nBB++){														/*only not added nodes*/
					if( bb=(m_colSets[iset].bb.get_bitboard(nBB) & node_state_active.get_bitboard(nBB)) ){
						while(bb){
							node=BitBoard::lsb64_intrinsic(bb);
							bb^=Tables::mask[node];
							node+=WMUL(nBB);
												
							/////////////////////
							//testing node
							if( test_node_for_failed_nodes(node, iset)!=NO_CONFLICT){
								nb_partial_conf++;												
								color_unit_dyn_stack.erase();
								assign_node_value(node, FALSE_VAL, NO_REASON /* no lookback*/);
								m_colSets[iset].size--;
								color_reduced_stack.push(iset);
								if(m_colSets[iset].size==1){
									color_unit_dyn_stack.push(iset);							/* to launch UL inference, only one node is left*/
									break;		
								}else if(m_colSets[iset].size==0){								/* single active vertex removed */
									if(lazy_context){
										LOG_INFO("InfraOpPlus::test_by_eliminate_failed_nodes()-conflict detected. context not restored! ");
										return true;	
									}
									conflict=true;		
									break;
								}
							}
							//////////////////////
							/* no conflict- continues with other nodes of same color in the same bitblock*/

						}//next node in same block
						if (conflict) break;		
					}
					/* no conflict- continues with other nodes of same color in another bitblock*/
				}//new block to test nodes

				if(conflict) break;
				else if(color_unit_dyn_stack.pt>0 && 
					unit_iset_process_for_test_by_eliminating_failed_nodes() !=NO_CONFLICT){
						if(lazy_context){
							LOG_INFO("InfraOpPlus::test_by_eliminate_failed_nodes()-conflict detected. context not restored! ");
							return true;	
						}
						conflict=true;
						break;
				}
			}//filter color_state active
			if(conflict) break;
		}//next color to test
	}while(nb_partial_conf>1 /* ==1 increases so much overhead?, check*/ && conflict==false);

	reset_context_for_maxsatz(0,0,0,save_color_unit_stack_pt);
	return(conflict);
}

template <class graph_t, class bitboard_t>
inline
int InfraOpPlus<graph_t,bitboard_t>::simple_further_test(int start){
///////////////////////////
// PARAMS: start-starting position from REDUCED_STACK from where to select candidate 
//		   conflicting colors
//
// FUNCTION: tests colors from reduced stack for conflicts
// APPLICATION: used during test by elimination of FAILED NODES, trying to prove a node v FAILS
//				by examining the REDUCED_STACK when v is set to TRUE
//
// RETURNS: conflicting color set from the REDUCED_STACK or NO_CONFLICT
//
// COMMENTS: candidate color sets from REDUCED_STACK cannot have been WEAKENED
//
// REMARKS:
// 1. no lazy exit is possible here, context hast to be restored
// 2. using test_nb for debugging: REMOVE in RELEASE
// 3. MAX_COLOR_FURTHER_TEST_LENGTH should be 2 since it tries to prove the second node is failed even if
//    the first node is not conflicting (see note (1)). This would be questionable for MAX_COLOR_FURTHER_TEST_LENGTH >2.
//    In this case I would recommend stopping the search in chosen_iset if the first node is proven non-conflicting

	int my_iset, chosen_iset;
	bool conflict_found=false;

	//init: 1.stores current end-pointers of STACKS 
	//      2.sets candidate colors in REDUCED_STACK to UNUSED
	int saved_color_unit_stack_pt=color_unit_stack.pt;				
	int saved_node_stack_pt=node_stack.pt;						
	int saved_color_passive_stack_pt=color_passive_stack.pt;		
	int saved_color_reduced_stack_pt=color_reduced_stack.pt;	

	int my_saved_color_unit_stack_pt=color_unit_stack.pt;
	int my_saved_node_stack_pt=node_stack.pt;
	int my_saved_color_passive_stack_pt=color_passive_stack.pt;	
	int my_saved_color_reduced_stack_pt=color_reduced_stack.pt;

	color_already_used_in_extended_test.erase_bit();												/*to avoids repetition in REDUCED_STACK */

	//main loop
	BITBOARD bb;
	for(int i=start; i<saved_color_reduced_stack_pt; i++) {		
		chosen_iset=color_reduced_stack.get_elem(i);												/* WEAKENED or NOT, nodes weakened will be discarded */
		if( m_colSets[chosen_iset].size==MAX_COLOR_FURTHER_TEST_LENGTH /* can it be <=?*/ && !color_already_used_in_extended_test.is_bit(chosen_iset)
			&& color_state_active.is_bit(chosen_iset) ) {
				color_already_used_in_extended_test.set_bit(chosen_iset);		   /* to avoid repetitions */	
				
				//int test_nb=0;
				for(int nBB=0; nBB<NB_OF_BB_NODES; nBB++){	/* non added nodes*/
					if( bb=(m_colSets[chosen_iset].bb.get_bitboard(nBB) & node_state_active.get_bitboard(nBB)) ){
						while(bb){
							int node=BitBoard::lsb64_intrinsic(bb);
							bb^=Tables::mask[node];
							node+=WMUL(nBB);

							//TEST: nb of nodes> MAX_COLOR_FURTHER_TEST_LENGTH -remove in RELEASE
						/*	if(++test_nb > MAX_COLOR_FURTHER_TEST_LENGTH){
								LOG_ERROR("simple_further_test():-bizarre number of nodes"); 
							}*/
							////////////

							//attempt to eliminate node
							color_unit_dyn_stack.erase();
							my_iset=fix_node_for_non_singleton_iset(node, chosen_iset);
							if (my_iset == NO_CONFLICT){
								if(color_unit_dyn_stack.pt>0)
									my_iset=unit_iset_process_for_test_by_eliminating_failed_nodes();
							}

							/*print_stack(stack_t::COLOR_PASSIVE);*/

							reset_context_for_maxsatz(my_saved_node_stack_pt,
								my_saved_color_passive_stack_pt,
								my_saved_color_reduced_stack_pt,
								my_saved_color_unit_stack_pt );

							if(my_iset!=NO_CONFLICT){	               	
								assign_node_value(node, FALSE_VAL, NO_REASON);
								m_colSets[chosen_iset].size--;
								color_reduced_stack.push(chosen_iset);
								
								/*print_stack(stack_t::COLOR_PASSIVE);
								print_stack(stack_t::NODES);*/

								//UL inferences 
								if(m_colSets[chosen_iset].size==1){
									color_unit_dyn_stack.erase();
									color_unit_dyn_stack.push(chosen_iset);
									if( (my_iset=unit_iset_process_for_test_by_eliminating_failed_nodes())!=NO_CONFLICT){
										conflict_found=true;										
										break;
									}

									/*print_stack(stack_t::COLOR_REDUCED);
									print_stack(stack_t::NODES);*/

									//set modified colors as candidate for re-election
									for(int j=my_saved_color_reduced_stack_pt; j<color_reduced_stack.pt; j++){
										color_already_used_in_extended_test.erase_bit(color_reduced_stack.get_elem(j));
									}	

									/*print_db(true, true);*/

									my_saved_color_unit_stack_pt=color_unit_stack.pt;
									my_saved_node_stack_pt=node_stack.pt;
									my_saved_color_passive_stack_pt=color_passive_stack.pt;	
									my_saved_color_reduced_stack_pt=color_reduced_stack.pt;
																		
									goto next_color;  /* since chosen_iset has size 1, there is no need to test last node */
								}
							}
							/* note that, since MAX_COLOR_FURTHER_TEST_LENGTH=2, it is also interesting to test for the second node since
							   if proven failed, chosen_iset is a singleton and UL inferences apply (1)*/

						}//next node in same block
						if (conflict_found)	break;
					}
				}//next node in next blocks
				if (conflict_found) break;
		}//filter condition for color
next_color:	;
	}//next color

	//reset context
	reset_context_for_maxsatz(saved_node_stack_pt,
							saved_color_passive_stack_pt,
							saved_color_reduced_stack_pt,
							saved_color_unit_stack_pt );

	if(conflict_found==true){
		//LOG_INFO("SIMPLE FURTHER TEST CONFLICT");
		return chosen_iset;
	}else return NO_CONFLICT;

}

//template <class graph_t, class bitboard_t>
//inline
//bool InfraOpPlus<graph_t,bitboard_t>::inc_csp_maxsatz(int v){
////////////////////
//// PARAMS: v is the node in a singleton color (kmin) that wants to be filtered
////		  Currently, v is not used (see 2), although it may be used for
////		  tests (*** TODO-CHANGE LOGIC ***)
////
//// RETURNS: TRUE if a conflict is found or FALSE_VAL otherwise 
////
//// REMARKS:
//// 1.Assumes singleton color iset={v] is last in the unit STACK  (2)
//	
//	int save_color_unit_stack_pt=color_unit_stack.pt;
//	node_stack.erase();
//	color_passive_stack.erase();
//	color_reduced_stack.erase();
//	color_unit_dyn_stack.erase();			
//
//	//swap first-last colors in color_unit_stack
//	//the last color is the singleton color of v
//	int first_color=color_unit_stack.get_elem(0);
//	color_unit_stack.stack[0]=color_unit_stack.stack[color_unit_stack.pt-1];
//	color_unit_stack.stack[color_unit_stack.pt-1]=first_color;
//	
//	
//	int iset;
//	if ((iset=unit_iset_process())!=NO_CONFLICT) {
//		lookback_for_maxsatz(iset);
//		reset_context_for_maxsatz(0, 0, 0, save_color_unit_stack_pt);
//		enlarge_conflict_set();
//		return true;
//	}
//	
//	
//	//the stacks might not be empty here (NO_CONFLICT state)-PSS
//	reset_context_for_maxsatz(0, 0, 0, save_color_unit_stack_pt);	
//	
//
//	//attempt to find a conflict in color classes of sizes 2 or 3
//	return(inc_maxsatz_lookahead(save_color_unit_stack_pt));
//
//	/*bool res=inc_maxsatz_lookahead(save_color_unit_stack_pt);
//	reset_context_for_maxsatz(0, 0, 0,save_color_unit_stack_pt);
//	 return res; */
//
//	
//
//
///////////////	
////*** elimination inferences possible?
////bool result=test_by_eliminate_failed_nodes();
////reset_context_for_maxsatz(0, 0, 0,save_color_unit_stack_pt);
//////if(result==true){
//////	return EMPTY_ELEM;	//conflict found
//////}
//////	
//////return false;
////  return result;
//
////	return false;;
//}


template <class graph_t, class bitboard_t>
inline
bool InfraOpPlus<graph_t,bitboard_t>::inc_maxsatz(int v){
//////////////////
// PARAMS: v is the node in a singleton color (kmin) that wants to be filtered
//		  Currently, v is not used (see 2), although it may be used for
//		  tests (*** TODO-CHANGE LOGIC ***)
//
// RETURNS: TRUE if a conflict is found or FALSE_VAL otherwise 
//
// REMARKS:
// 1.Assumes singleton color iset={v] is last in the unit STACK  (2)
	
	int save_color_unit_stack_pt=color_unit_stack.pt;
	node_stack.erase();
	color_passive_stack.erase();
	color_reduced_stack.erase();
	color_unit_dyn_stack.erase();	

	//swap first-last colors in color_unit_stack
	//the last color is the singleton color of v
	if(color_unit_stack.pt>1){																	
		int first_color=color_unit_stack.get_elem(0);
		color_unit_stack.stack[0]=color_unit_stack.stack[color_unit_stack.pt-1];
		color_unit_stack.stack[color_unit_stack.pt-1]=first_color;
	}
		
	int iset;
	if ((iset=unit_iset_process())!=NO_CONFLICT) {
		lookback_for_maxsatz(iset);
		reset_context_for_maxsatz(0, 0, 0, save_color_unit_stack_pt);
		enlarge_conflict_set();
		return true;
	}
		
	//the stacks might not be empty here (NO_CONFLICT state)-PSS
	reset_context_for_maxsatz(0, 0, 0, save_color_unit_stack_pt);	
		
	//attempt to find a conflict in color classes of sizes 2 or 3
	return(inc_maxsatz_lookahead(save_color_unit_stack_pt));					//** removed for tests


/////////////	
//*** elimination inferences possible?
//result=test_by_eliminate_failed_nodes();
//reset_context_for_maxsatz(0, 0, 0,save_color_unit_stack_pt);
//if(result==true){
//	return EMPTY_ELEM;	//conflict found
//}
	
return false;		
}


template <class graph_t, class bitboard_t>
inline
void InfraOpPlus<graph_t,bitboard_t>::init_inc_maxsatz(){
///////////////
// incremental maxsatz which is configured for
// a single conflict
//
// REMARKS:
// 1. assumes ACTIVE NODES ARE CORRECTLY SET
// 2. assumes color_db is consistent, no enlarged color sets etc.!!
	
	//reset_enlarged_isets_lazy();						/* cleans enlarged color sets as well */
	color_unit_stack.pt=0;
	for(int i=NB_OF_COLORS; i>=1; i--){					/* smallest colors first! */
		color_involved_state[i]=FALSE_VAL;				/* new compared with non-incremental, is it necessary? */
		color_state_active.set_bit(i);
		if (m_colSets[i].size==1){
			color_unit_stack.push(i);
		}
	}

	//minimal init config. on exit: possible remove
	reset_nb_added_nodes();
	node_state_active.get_bitboard(NB_OF_BB_NODES)=0;	
	color_conflict_stack.erase();
	color_enlarged_stack.erase();
	
	/* CHECK IF BEST LAZY RESETTING : reset_enlarged_isets_lazy()? */
}

//template <class graph_t, class bitboard_t>
//inline
//int InfraOpPlus<graph_t,bitboard_t>::init_inc_maxsatz_csp(){
/////////////////
//// incremental maxsatz which is configured for
//// a single conflict
////
//// RETURNS: number of unit colors found
////
//// REMARKS:
//// 1. assumes ACTIVE NODES ARE CORRECTLY SET
//// 2. assumes color_db is consistent, no enlarged color sets etc.!!
//
//	
//  int nb_unit_col=0;
//	//reset_enlarged_isets_lazy();						/* cleans enlarged color sets as well */
//	color_unit_stack.pt=0;
//	for(int i=NB_OF_COLORS; i>=1; i--){					/* smallest colors first! */
//	//	color_involved_state[i]=FALSE_VAL;				/* new compared with non-incremental, is it necessary? */
//		color_state_active.set_bit(i);
//		if (m_colSets[i].size==1){
//			nb_unit_col++;
//			color_unit_stack.push(i);
//		}
//	}
//
//	//minimal init config. on exit: possible remove
//	/*reset_nb_added_nodes();
//	node_state_active.get_bitboard(NB_OF_BB_NODES)=0;	
//	color_conflict_stack.erase();
//	color_enlarged_stack.erase();*/
//	
//	/* CHECK IF BEST LAZY RESETTING : reset_enlarged_isets_lazy()? */
//
//	return nb_unit_col;
//}


template <class graph_t, class bitboard_t>
inline
int InfraOpPlus<graph_t,bitboard_t>::inc_test_node(int node, int iset, bool is_last_node){
///////////////////////////
// Simplified test_node function
// 1.uses simple unit_iset_process() instead of  unit_iset_process_for_tests()
//
// REMARKS: check reset_contexts
	
	//test-remove in release
	if(!node_state_active.is_bit(node)){
		LOG_ERROR("InfraOpPlus::inc_test_node-called for a node which is not active");
		return NO_CONFLICT;
	}
	
	int save_color_unit_stack_pt=color_unit_stack.pt, my_iset;				
	color_unit_dyn_stack.erase();
	
	if( (my_iset=fix_node_for_non_singleton_iset(node, iset)) != NO_CONFLICT || 
		//(my_iset=inc_unit_iset_process_for_tests()) !=  NO_CONFLICT						/* recent attempt, so as not to waste singletons found in the prev. step -26-3-17 (tests were not impressive)*/ 
		(my_iset=unit_iset_process()) !=  NO_CONFLICT										

#ifdef FURTHER_TEST_FAILED_COLOR_SETS
		|| 	(is_last_node && (my_iset=further_test(0 /* all colors in reduced stack*/)) != NO_CONFLICT)
#endif		
		){
			lookback_for_maxsatz(my_iset);													/* can be removed for csp derived graphs */
			reset_context_for_maxsatz(0, 0,  0,	 save_color_unit_stack_pt);
			return my_iset;

	}else{ /* NO CONFLICT */
		reset_context_for_maxsatz(0, 0, 0, save_color_unit_stack_pt);
	}

  	return NO_CONFLICT;
}


//template <class graph_t, class bitboard_t>
//inline
//int InfraOpPlus<graph_t,bitboard_t>::inc_test_node_csp(int node, int iset, bool is_last_node){
/////////////////////////////
//// Simplified test_node function
//// 1.uses simple unit_iset_process() instead of  unit_iset_process_for_tests()
////
//// Does not use lookback to store involved sets
////
//// REMARKS: check reset_contexts
//	
//	//test-remove in release
//	if(!node_state_active.is_bit(node)){
//		LOG_ERROR("InfraOpPlus::inc_test_node-called for a node which is not active");
//		return NO_CONFLICT;
//	}
//	
//	int save_color_unit_stack_pt=color_unit_stack.pt, my_iset;				
//	color_unit_dyn_stack.erase();
//	
//	if( (my_iset=fix_node_for_non_singleton_iset(node, iset)) != NO_CONFLICT ||		
//		//(my_iset=unit_iset_process()) !=  NO_CONFLICT												/* default option in normal incremental implementation */
//		(my_iset=unit_iset_process_for_test_by_eliminating_failed_nodes_csp()) !=  NO_CONFLICT			/* recent attempt, so as not to waste singletons found in the prev. step -26-3-17 (tests were not impressive)*/ 
//		
//
//#ifdef FURTHER_TEST_FAILED_COLOR_SETS
//		|| 	(is_last_node && (my_iset=further_test_csp(0 /* all colors in reduced stack*/)) != NO_CONFLICT)
//#endif		
//		){
//		//	lookback_for_maxsatz(my_iset);													/* not needed for csp */			
//			reset_context_for_maxsatz(0, 0,  0,	 save_color_unit_stack_pt);
//			return my_iset;
//
//	}else{ /* NO CONFLICT */
//		reset_context_for_maxsatz(0, 0, 0, save_color_unit_stack_pt);
//	}
//
//  	return NO_CONFLICT;
//}


template <class graph_t, class bitboard_t>
inline
bool InfraOpPlus<graph_t,bitboard_t>::inc_maxsatz_lookahead(int saved_color_unit_stack_pt){
//////////
// RETURNS TRUE if one conflict is found, FALSE otherwise
//
// REMARKS:
// 1.no testing of active nodes inside the colors (assumes color_db is consistent, i.e. no reduced nodes)

	bool test_flag, no_conflict_flag, one_conflict=false;
	
	for(int k=2; k<=MAX_COLOR_TEST_LENGTH; k++){
		for(int iset=NB_OF_COLORS; iset>=1; iset--){	
			if( color_state_active.is_bit(iset) && m_colSets[iset].size==k 
				&& !m_colSets[iset].bb.get_bitboard(NB_OF_BB_NODES) /* not weakened*/){						  
				
					color_involved_stack.erase();
					bitboard_t& bbcol=m_colSets[iset].bb;
					bbcol.init_scan(bbo::NON_DESTRUCTIVE);
					no_conflict_flag=false;
					for(int i=0; i<k; i++){
						if(inc_test_node(bbcol.next_bit(), iset, (i==k-1) /* is last node*/)==NO_CONFLICT){		 /*no testing of active nodes-TEST*/
							no_conflict_flag=true;					//no conflict
							break;
						}else{				//conflict for this particular node
							store_involved_sets();		
						}
					}
										
					if(no_conflict_flag==false){
						//reset_context_for_maxsatz(0, 0, 0, saved_color_unit_stack_pt);   /*** CHECK THAT THIS IS NOT NEEDED
						//**CHECKS: see if iset is enlarged by only one added node
						//saved_size=ISETS_SIZE[iset];
						enlarge_stored_involved_sets();						/*(1)*/
						/*if (saved_size+1 != ISETS_SIZE[iset]){
								printf("erreur iset involved %d %d %d...", 
								saved_size,  ISETS_SIZE[iset], iset);
						}*/
						one_conflict=true;
					}
															
					//reset context
					if(no_conflict_flag)						/*already reset if CONFLICT detected in (1)*/
						reset_involved_isets();

					if (one_conflict) 
							return true;					
			}
		}//next color
	}

	return false;
}

//template <class graph_t, class bitboard_t>
//inline
//bool InfraOpPlus<graph_t,bitboard_t>::inc_maxsatz_lookahead_csp(bitarray& bbsg, sbb_t<bitarray>& s){
////////////
//// RETURNS TRUE if one conflict is found, FALSE otherwise. 
//// Tailored for CSPs but currently applied to non-incremental version
//// 
//// COMMENTS: Does not store involved sets, since one conflict is enough to CUT
////
//// REMARKS:
//// 1.no testing of active nodes inside the colors (assumes color_db is consistent, i.e. no reduced nodes)
//
//	bool test_flag, no_conflict_flag, one_conflict=false;
//
//	for(int k=2; k<=MAX_COLOR_TEST_LENGTH; k++){
//		for(int iset=1; iset<=NB_OF_COLORS; iset++){	
//		//for(int iset=NB_OF_COLORS; iset>=1; iset--){	
//			if(m_colSets[iset].size==k  && color_state_active.is_bit(iset) 
//				/*&& !m_colSets[iset].bb.get_bitboard(NB_OF_BB_NODES) /* not weakened*/){				  
//				
//					//color_involved_stack.erase();
//					bitboard_t& bbcol=m_colSets[iset].bb;
//					bbcol.init_scan(bbo::NON_DESTRUCTIVE);
//					no_conflict_flag=false;
//					for(int i=0; i<k; i++){
//						int node=bbcol.next_bit();
//						if(inc_test_node_csp(node, iset, (i==k-1) /* is last node*/)==NO_CONFLICT){		 /*no testing of active nodes-TEST*/
//							no_conflict_flag=true;					//no conflict
//							break;
//						}else{					//conflict for this particular node
//							bbsg.erase_bit(node);
//																						
//							/*LOG_INFO("REMOVED NODE TEST ELIMINATE NODE");
//							cin.get();*/
//							node_state_active.erase_bit(node);
//							m_colSets[iset].erase_bit(node);								/* m_colSets[iset].size--; */
//							if(m_colSets[iset].size==0){									/* only check for empty set, not single color propagation */
//								//	LOG_INFO("REMOVED NODE TEST ELIMINATE NODE");
//								return true;												/* conflict found */
//							}//else if(m_colSets[iset].size==1){
//							//	//LOG_INFO("JLJLKHLJG");
//							//	s.push(FIRST_SHARED(m_colSets[iset].bb, node_state_active));/* test if node exists */	
//							//	color_unit_dyn_stack.erase();								/* test impact of this */
//							//	color_unit_dyn_stack.push(iset);							/* a la test_eliminate_node */
//							//	if(unit_iset_process_for_test_csp(bbsg)!=NO_CONFLICT ){
//							//		return true;											/* conflict found */
//							//	}
//							//}
//						}
//					}//next node
//					if(no_conflict_flag==false)
//								return true;			/* conflict found: all nodes in color set found inconsistent*/
//					else if(m_colSets[iset].size==1){   /* hard propagates singleton color-note that initially no color mayhave size 1 so MUST HAVE BEEN REDUCED  */
//						//LOG_INFO("JLJLKHLJG");
//						//s.push(FIRST_SHARED(m_colSets[iset].bb, node_state_active));/* test if node exists */	
//						color_unit_dyn_stack.erase();								/* test impact of this */
//						color_unit_dyn_stack.push(iset);							/* a la test_eliminate_node */
//						if(unit_iset_process_for_test_csp(bbsg, s)!=NO_CONFLICT ){
//							return true;											/* conflict found */
//						}
//					}
//			}
//		}//next color (iset)
//	}
//
//	return false;
//}

#endif




