//infra_tools.h: Header for InfraOp which uses vertices to guide the main unit literal
//						propagation mechanism compared to InfraOp (interfaces also change so it is not 
//						inherited from InfraOp)

// date of creation: 27/7/16

#ifndef __INFRA_TOOLS_H__
#define	__INFRA_TOOLS_H__


#include "clique_types.h"
#include "../init_color_ub.h"
#include "bitscan/bbalg.h"						//bb_t
#include "utils/common.h"


template<class graph_t, class bitboard_t>
class InfraOp{
///////////////
//data members

protected:
	static const int MAX_NUM_CONFLICTS=64;							//do NOT change!
	static const int NONE=-1;										//empty or without value
	static const int TRUE_VAL=1;
	static const int FALSE_VAL=0;
	static const int NO_CONFLICT=-2;								//alternative to an empty (conflicting) color set
	static const int NO_REASON=-3;									//for empty node reason state
	static const int MAX_COLOR_TEST_LENGTH=MAX_SIZE_FAILED_COLOR_SET;	
	static const int MAX_COLOR_FURTHER_TEST_LENGTH=2;

	//deprecated
	static const int MAX_CLAUSE_ONE_SHOT_SIZE=2;					//do NOT change!-deprecated
	static const int MAX_LOOK_AHEAD=5;								//deprecated
	static const int MAX_NUM_SEEDS=5;								//deprecated
////////////////////

	graph_t* g;														//input graph
	graph_t* gc;													//complement graph (cache) 

////////////////////
// prior data structures for old infrachrom one-shot inferences
protected:		
	//bitarrays-currently deprecated
	bitboard_t bb_used_colors;									//stores colors during each unit literal inference	
	bitboard_t bb_non_neighbors;								//auxiliary vertex set storage for unit literal inference
	bitboard_t bb_remaining_colors;								//colors used in each inference
	
public:
/////////
// new data structures for new infrachrom a la incMaxCLQ (3/8/16)
	int NB_OF_COLORS;
	int NB_OF_BB_NODES;											//number of bitblocks
	int NB_OF_NODES;
	int ADDED_NODES;											//offset is NV rounded to nearest bitblock
	int NB_OF_BB_ADDED_NODES;									//number of bitblocks including max_num of added nodes (64 CONF. MAX)
	int NB_OF_ADDED_NODES;
	bb_t<bitboard_t>* m_colSets;								//capital letter 'S' to indicate that the ADT also contains the size of the color set

	com::stack_t<int> color_filter_stack;		//used during filter pre-processing
	com::stack_t<int> color_unit_stack;			//initial singleton color seeds for UL
	com::stack_t<int> color_unit_dyn_stack;		//future singleton color seeds during UL
	com::stack_t<int> color_passive_stack;		//contains passive colors
	com::stack_t<int> color_reduced_stack;		//contains reduced colors
	com::stack_t<int> color_reason_stack;		//contains colors	
	com::stack_t<int> color_conflict_stack;		//contains (enlarged) colors of the same conflict (separated by NONE)
	com::stack_t<int> color_enlarged_stack;		//contains enlarged colors
	
	com::stack_t<int> node_stack;				//contains passive nodes
	com::stack_t<int> tested_node_stack;		//nodes tested in a non-unit color set inference
	com::stack_t<int> color_involved_stack;		//colors involved in partial conflicts (tests) of non-unit clauses

	bitboard_t bb_node_state_active;			//TRUE-Active, FALSE-Passive
	int* node_reason;							//[nodes], contains colors or NONE
	int* node_value;							//[nodes], TRUE, FALSE or NONE
	int* node_iset_no;							//[nodes], color label of nodes
	bitboard_t color_state_active;				//[colors], TRUE-Active, FALSE-Passive
	int** node_conflict_set;					//[weakened node]	
	int* iset_involved;							//[conflict node] used to avoid repetitions during look_back (bitset?)

	int* node_tested_state;						//TRUE-tested, FALSE-untested
	int* iset_used_in_test;						//TRUE-color of a partial conflict of a non-unit color set, FALSE-not conflicting
	bitboard_t bb_colors_used_in_extended_test;	//avoids repetitions when testing colors from REDUCED_STACK	
		
////////////////////////
//interface
public:
	InfraOp(graph_t* g=NULL):g(g),gc(NULL), m_colSets(NULL), node_iset_no(NULL), node_reason(NULL), node_value(NULL),
									node_conflict_set(NULL), iset_involved(NULL), node_tested_state(NULL), iset_used_in_test(NULL)	{}
	virtual ~InfraOp(){clear();}
	void set_graph(graph_t* g){this->g=g; }
	void set_color_nb(int cmax){NB_OF_COLORS=cmax;}	

virtual	int init(int MAX_COL);								//allocates gc and node_node_iset_no
virtual	void clear();


///////////////////
// auxiliary operations
	void update_color_sizes(int cmax);
	void set_node_state_active(const bitboard_t& bb);
	
//////////
// infrachrom operations (deprecated: 2/9/16)
	int infrachrom_unit_clause(int cmax, int* lv, int size, int max_nb_inf=CLQ_MAXINT);						//lv: list of vertices that make up the subproblem analysed
	int infrachrom_unit_clause_restore(int cmax, int* lv, int size, int max_nb_inf=CLQ_MAXINT);			
	int infrachrom_2_clause(int cmax, int* lv, int size, int max_nb_inf=CLQ_MAXINT);
	int infrachrom_3_clause(int cmax, int* lv, int size, int max_nb_inf=CLQ_MAXINT);
	int infrachrom_4_clause(int cmax, int* lv, int size, int max_nb_inf=CLQ_MAXINT);


	int infrachrom_one_shot(int cmax);
	int infrachrom_one_shot_pc(int cmax);			//explicit computation of pc
	
private:
	int find_inconsistency_unit_clause(int seed, int* lv, int size, bitarray& bb_free_col);					
	int find_inconsistency_non_unit_clause(int* lv, int size, bitarray& bb_free_col);	
	
///////////
// infrachrom a la incMaxCLQ (3/8/16)
public: 
	//setters and getters
	void reset_added_nodes	()	{ADDED_NODES=WMUL(NB_OF_BB_NODES);}
	int nb_added_nodes		()	{return WMUL(NB_OF_BB_NODES)-ADDED_NODES;}
	
public:
	int init_maxsatz(int v, int clq_size);							//adds v, the expanded node 
	int init_maxsatz(int clq_size);									
virtual	int maxsatz(int max_num_conf);
		
	//only for computing initial upper bounds
	int init_maxsatz_init_ub(int v, int clq_size);	
	int maxsatz_init_ub(int max_num_conf);	
	
///////////////////
//simplified maxsatz reasoning (one-conf)
	void init_inc_maxsatz();
	int inc_maxsatz(int v);											//v is the vertex in unit set to filter (***TODO: currently not used inside the function, check!!)
	int inc_maxsatz_lookahead(int saved_color_unit_stack_pt);
	int inc_test_node(int v, int iset, bool further=false);
	

protected:
	//main UL inferences
	int unit_iset_process();
	int fix_unit_color(int iset);			
	int fix_node_for_iset(int v, int iset);
	int fix_added_node_for_iset(int v, int iset);					//removes v from colors in CONFLICT_STACK 
	int exclude_noneibor(int noneibor, int reason_iset);			//used by fix_node_for_iset
		
	void lookback_for_maxsatz(int iset);
virtual	int enlarge_involved_set();
		
	//non-unit clause tests(inferences)
	void store_involved_sets();										//REASON_STACK to INVOLVED_SET_STACK
virtual	int enlarge_stored_involved_sets();							//INVOLVED_SET_STACK to CONFLICT_STACK
	int test_node(int v, int iset, bool further=false);				//sets v to FALSE to find a conflict
	int unit_iset_process_for_tests();
virtual	int maxsatz_lookahead(int nb_conflict, int max_num_conf);		//Main Test driver
	int further_test(int start);									//tests colors from REDUCED_STACK
	int simple_further_test(int start);								//tests colors from REDUCED_STACK

	//test by eliminating failed nodes
	int test_node_for_failed_nodes(int v, int iset);
	bool test_by_eliminate_failed_nodes();

	//context
	void reset_context_for_maxsatz( int saved_node_stack_pt, 
									int saved_color_passive_stack_pt, 
									int save_color_reduced_stack_pt,
									int save_color_unit_stack_pt		);

	void reset_context_for_maxsatz_node( int saved_node_stack_pt, 
									int saved_color_passive_stack_pt, 
									int save_color_reduced_stack_pt,
									int save_color_unit_stack_pt		);
public:
	void reset_enlarged_isets();
private:

	//update state functions
	void assign_node_value(int v, int val, int reason_iset);



public:
	bool filter();							//uses bb_node_state_active as input subgraph
protected:
	bool unitiset_filter(int c);									
	bool biniset_filter(int c);
	bool triset_filter(int c);
	bool fouriset_filter(int c);
	bool fiveiset_filter(int c);
public:
	bool filter(int* lv, int size);			//for subgraphs as list of vertices
protected:
	bool unitiset_filter(int c, int* lv, int size);
	bool biniset_filter(int c, int* lv, int size);
	bool triset_filter(int c, int* lv, int size);
	bool fouriset_filter(int c, int* lv, int size);

	//I/O
public:
virtual	void print_db(bool active_sets=false, bool active_nodes=false);
	//tests
virtual bool check_consistency_db(){return true;}
};


template<class graph_t, class bitboard_t>
inline
void InfraOp<graph_t,bitboard_t>::print_db(bool active_sets_only, bool active_nodes_only){
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

			if(active_nodes_only && !bb_node_state_active.is_bit(v)) continue;
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
void InfraOp<graph_t, bitboard_t>::clear(){

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

	if(node_conflict_set)
		delete [] node_conflict_set;
	node_conflict_set=NULL;

	if(iset_involved){
		delete [] iset_involved;
	}
	iset_involved=NULL;

	if(iset_used_in_test){
		delete [] iset_used_in_test;
	}
	iset_used_in_test=NULL;

		
	//clear complement graph
	if(gc){
		gc->clear();
	}
	gc=NULL;
}

template<class graph_t, class bitboard_t>
inline
int InfraOp<graph_t, bitboard_t>::init(int MAX_NB_COL){
//////////////
// allocates memory for data structures: MAX_NB_COL+1 color sets
//
// RETURNS -1 if ERROR

	if(g==NULL) return -1; 
	clear();

	//int NCOL=MAX_NB_COL+1;
	//int NCOL=MAX_NB_COL+15;			//for solve_first_nodes_incMaxCLQ
	int NCOL=g->number_of_vertices();		//for solve_first_nodes_incMaxCLQ 
	
	try{
		//basic params for weakened color sets
		NB_OF_NODES=g->number_of_vertices();
		NB_OF_BB_NODES=g->get_neighbors(0).number_of_bitblocks();
		NB_OF_BB_ADDED_NODES=NB_OF_BB_NODES+1;							//max WORD_SIZE conflicts!
		ADDED_NODES=WMUL(NB_OF_BB_NODES);								//max WORD_SIZE conflicts!
		NB_OF_ADDED_NODES=WMUL(NB_OF_BB_ADDED_NODES)-1;	
		
		m_colSets= new bb_t<bitboard_t>[NCOL];				//[0] is used to store the subgraph to color. Actual colors range from [1, N]	
		for(int i=0; i<NCOL; i++){
			m_colSets[i].init(NB_OF_ADDED_NODES);			//!! PROBLEM: should be NB_OF_ADDED_NODES but then BBMC coloring uses only NB_OF_NODES
		}
		
		//deprecated
		bb_non_neighbors.init(NB_OF_NODES);
		bb_used_colors.init(NCOL);
		bb_remaining_colors.init(NCOL);

		
		//allocate node arrays
		node_iset_no= new int[NB_OF_ADDED_NODES];
		node_reason= new int[NB_OF_ADDED_NODES];
		node_value= new int[NB_OF_ADDED_NODES];
		node_tested_state=new int[NB_OF_ADDED_NODES];
		for(int i=0; i<NB_OF_ADDED_NODES; i++){
			node_iset_no[i]=CLQ_MAXINT;			//*** possibly change to empty traceable value
			node_reason[i]=NO_REASON;
			node_value[i]=NONE;
			node_tested_state[i]=FALSE_VAL;
		}


		//color state arrays
		iset_involved=new int[NCOL];
		for(int i=0; i<NCOL; i++){
			iset_involved[i]=FALSE_VAL;	
		}
		iset_used_in_test=new int[NCOL];
		for(int i=0; i<NCOL; i++){
			iset_used_in_test[i]=FALSE_VAL;	
		}

		//stacks
		color_filter_stack.init(NCOL);
		color_unit_stack.init(NCOL);
		color_unit_dyn_stack.init(NCOL);
		color_passive_stack.init(NCOL);
		color_reason_stack.init(NCOL);	
		color_conflict_stack.init(2*NB_OF_ADDED_NODES);	//colors may be repeated (*** CHECK ***)
		color_reduced_stack.init(2*NB_OF_ADDED_NODES);	//colors may be repeated
		color_enlarged_stack.init(2*NB_OF_ADDED_NODES);
		color_involved_stack.init(2*NB_OF_ADDED_NODES);	//colors may be repeated
		node_stack.init(2*NB_OF_ADDED_NODES);
		tested_node_stack.init(NB_OF_ADDED_NODES);
				
		//state
		bb_node_state_active.init(NB_OF_ADDED_NODES);
		color_state_active.init(NCOL);
		bb_colors_used_in_extended_test.init(NCOL);		//used to avoid repetitions in color testing from reduced stack

		//other params
		node_conflict_set=new int*[NB_OF_ADDED_NODES];	//max WORD_SIZE conflicts!

		//allocate complement graph
		gc=new graph_t(1);
		g->create_complement(*gc);			//***check -1 

	}catch(exception& e){
		LOG_INFO(e.what();)
		return -1; 
	}
		
	return 0;
}
template<class graph_t, class bitboard_t>
inline
void InfraOp<graph_t, bitboard_t>::update_color_sizes(int cmax){
	for(int c=1; c<=cmax; c++)
		m_colSets[c].update_size();
}

template<class graph_t, class bitboard_t>
inline
int InfraOp<graph_t, bitboard_t>::infrachrom_unit_clause(int cmax, int* lv, int size, int clqsize){
/////////////////////////
// Experimental driver for full inferences
//
// PARAMS: cmax: size of color set, lv/size: the set of colored vertices(subgraph)
// RETURNS: number_of_conflicts
//
// COMMENTS
// 1.No seed limitation

	int number_of_conflicts=0;
	bb_remaining_colors.init_bit(1,cmax);			//all bits clear except 1-cmax
		
	//find number of unit clauses and add them to the stack
	for(int c=cmax; c>=1; c--){
		if(m_colSets[c].size==1 && bb_remaining_colors.is_bit(c) ){
			bb_remaining_colors.erase_bit(c);
			if(find_inconsistency_unit_clause(m_colSets[c].bb.lsbn64(), lv, size, bb_remaining_colors)>0){		//updates active vertices and remaining colors if true
				if(++number_of_conflicts == clqsize){
					break;
				}
			}else bb_remaining_colors.set_bit(c);
		}
	}

	return number_of_conflicts;
}

template<class graph_t, class bitboard_t>
inline
int InfraOp<graph_t, bitboard_t>::infrachrom_unit_clause_restore(int cmax, int* lv, int size, int clqsize){
/////////////////////////
// Experimental driver for full inferences
//
// PARAMS: cmax: size of color set, lv/size: the set of colored vertices(subgraph)
// RETURNS: number_of_conflicts

	int number_of_conflicts=0;
	bb_remaining_colors.init_bit(1,cmax);			//all bits clear except 1-cmax
		
	//find number of unit clauses and add them to the stack
	int nSeeds=0;
	for(int c=cmax; c>=1; c--){
		if(m_colSets[c].get_size()==1 && bb_remaining_colors.is_bit(c) ){
			nSeeds++;
			bb_remaining_colors.erase_bit(c);
			if(find_inconsistency_unit_clause_restore(m_colSets[c].bb.lsbn64(), lv, size, bb_remaining_colors)>0){		//updates active vertices and remaining colors if true
				if(++number_of_conflicts == clqsize){
					break;
				}
			}else bb_remaining_colors.set_bit(c);
		}
		if(nSeeds==MAX_NUM_SEEDS) break;
	}

	return number_of_conflicts;
}

template<class graph_t, class bitboard_t>
inline
int InfraOp<graph_t, bitboard_t>::infrachrom_2_clause(int cmax, int* lv, int size, int clqsize){
/////////////////////////
// Experimental driver for full inferences
// RETURNS: º
		
	int number_of_conflicts=0;
	bb_remaining_colors.init_bit(1,cmax);			//all bits clear except 1-cmax
		
	//find number of unit clauses and add them to the stack
	int nSeeds=0;
	for(int c=cmax; c>=1; c--){
		
		if(m_colSets[c].size<=2 && bb_remaining_colors.is_bit(c)){
			nSeeds++;
			if(m_colSets[c].size==1){
				bb_non_neighbors=gc->get_neighbors(m_colSets[c].bb.lsbn64());
			}else if(m_colSets[c].size==2){
				m_colSets[c].bb.init_scan(bbo::NON_DESTRUCTIVE);
				int v1=m_colSets[c].bb.next_bit();
				int v2=m_colSets[c].bb.next_bit();
				AND(gc->get_neighbors(v1), gc->get_neighbors(v2), bb_non_neighbors);
			}
			////////////////////
			bb_remaining_colors.erase_bit(c);
			if(find_inconsistency_non_unit_clause(lv, size, bb_remaining_colors)>0){		//updates active vertices and remaining colors if true
				if(++number_of_conflicts == clqsize){
					break;
				}
			}else bb_remaining_colors.set_bit(c);
		}

		if(nSeeds==MAX_NUM_SEEDS) break;		//*** CHECK REMOVING THIS LIMIT
	}

	return number_of_conflicts;
}

template<class graph_t, class bitboard_t>
inline
int InfraOp<graph_t, bitboard_t>::infrachrom_3_clause(int cmax, int* lv, int size, int clqsize){
/////////////////////////
// Experimental driver for full inferences
// RETURNS: number_of_conflicts
		
	int number_of_conflicts=0;
	bb_remaining_colors.init_bit(1,cmax);			//all bits clear except 1-cmax
		
	//find number of unit clauses and add them to the stack
	int nSeeds=0;
	for(int c=cmax; c>=1; c--){
		if(m_colSets[c].size<=3 && bb_remaining_colors.is_bit(c)){
			nSeeds++;
			//determine filter vertex set
			if(m_colSets[c].size==1){
				bb_non_neighbors=gc->get_neighbors(m_colSets[c].bb.lsbn64());
			}else if(m_colSets[c].size==2){
				m_colSets[c].bb.init_scan(bbo::NON_DESTRUCTIVE);
				int v1=m_colSets[c].bb.next_bit();
				int v2=m_colSets[c].bb.next_bit();
				AND(gc->get_neighbors(v1), gc->get_neighbors(v2), bb_non_neighbors);
				
			}else if(m_colSets[c].size==3){
				m_colSets[c].bb.init_scan(BBObject::NON_DESTRUCTIVE);
				int v1=m_colSets[c].bb.next_bit();
				int v2=m_colSets[c].bb.next_bit();
				int v3=m_colSets[c].bb.next_bit();
				int nBB=bb_non_neighbors.number_of_bitblocks();
				for(int i=0; i<nBB; i++){
					bb_non_neighbors.get_bitboard(i)=gc->get_neighbors(v1).get_bitboard(i)&gc->get_neighbors(v2).get_bitboard(i)& gc->get_neighbors(v3).get_bitboard(i);
				}
			
			}
			////////////////////
			bb_remaining_colors.erase_bit(c);
			if(find_inconsistency_non_unit_clause(lv, size, bb_remaining_colors)>0){		//updates active vertices and remaining colors if true
				if(++number_of_conflicts == clqsize){
					break;
				}
			}else bb_remaining_colors.set_bit(c);
		}

		if(nSeeds==MAX_NUM_SEEDS) break;
	}

	return number_of_conflicts;
}


template<class graph_t, class bitboard_t>
inline
int InfraOp<graph_t, bitboard_t>::infrachrom_4_clause(int cmax, int* lv, int size, int clqsize){
/////////////////////////
// Experimental driver for full inferences
// RETURNS: number_of_conflicts
//
//**TODO: determine lower color bound (sorting?)
		
	int number_of_conflicts=0;
	bb_remaining_colors.init_bit(1,cmax);			//all bits clear except 1-cmax
		
	//find number of unit clauses and add them to the stack
	int nSeeds=0;
	for(int c=cmax; c>=1; c--){
		if(m_colSets[c].size<=4 && bb_remaining_colors.is_bit(c)){
		
			nSeeds++;

			//determine filter vertex set
			if(m_colSets[c].size==1){
				bb_non_neighbors=gc->get_neighbors(m_colSets[c].bb.lsbn64());
			}else if(m_colSets[c].size==2){
				m_colSets[c].bb.init_scan(bbo::NON_DESTRUCTIVE);
				int v1=m_colSets[c].bb.next_bit();
				int v2=m_colSets[c].bb.next_bit();
				AND(gc->get_neighbors(v1), gc->get_neighbors(v2), bb_non_neighbors);
			}else if(m_colSets[c].size==3){
				m_colSets[c].bb.init_scan(BBObject::NON_DESTRUCTIVE);
				int v1=m_colSets[c].bb.next_bit();
				int v2=m_colSets[c].bb.next_bit();
				int v3=m_colSets[c].bb.next_bit();
				int nBB=bb_non_neighbors.number_of_bitblocks();
				for(int i=0; i<nBB; i++){
					bb_non_neighbors.get_bitboard(i)=gc->get_neighbors(v1).get_bitboard(i)&gc->get_neighbors(v2).get_bitboard(i)& gc->get_neighbors(v3).get_bitboard(i);
				}
			}else if(m_colSets[c].size==4){
				m_colSets[c].bb.init_scan(BBObject::NON_DESTRUCTIVE);
				int v1=m_colSets[c].bb.next_bit();
				int v2=m_colSets[c].bb.next_bit();
				int v3=m_colSets[c].bb.next_bit();
				int v4=m_colSets[c].bb.next_bit();
				int nBB=bb_non_neighbors.number_of_bitblocks();
				for(int i=0; i<nBB; i++){
					bb_non_neighbors.get_bitboard(i)=gc->get_neighbors(v1).get_bitboard(i)&gc->get_neighbors(v2).get_bitboard(i) & 
										gc->get_neighbors(v3).get_bitboard(i) & gc->get_neighbors(v4).get_bitboard(i);
				}			
			}
			////////////////////
			
			bb_remaining_colors.erase_bit(c);
			if(find_inconsistency_non_unit_clause(lv, size, bb_remaining_colors)>0){		//updates active vertices and remaining colors if true
				if(++number_of_conflicts == clqsize){
					break;
				}
			}else bb_remaining_colors.set_bit(c);
			
		}

		if(nSeeds==MAX_NUM_SEEDS) break;			//*** BUGGY IF REMOVED  (with PRE-PROCESSING)
	}

	return number_of_conflicts;

}



template <class graph_t, class bitboard_t >
inline
int InfraOp<graph_t,bitboard_t>::infrachrom_one_shot(int cmax){
///////////////////////
// Looks for ONE inconsistent subset taking as seed the highest color class
// Uses non-neighbors for inference
// first_update:1/7/16
// RETURNS 1 or 0
// REMARKS: Only inference which modifies color BBDD


	//determines inference bitset
	int col=EMPTY_ELEM;
	for(int c=cmax; c>=2; c--){
		if (m_colSets[c].size<MAX_CLAUSE_ONE_SHOT_SIZE){
			col=c;
			break;
		}
	}
	
	if(col!=EMPTY_ELEM){
		if(m_colSets[col].size==1){
			bb_non_neighbors=gc->get_neighbors(m_colSets[col].bb.lsbn64());		//*** TODO. change name
		}else if(m_colSets[col].size==2){
			m_colSets[col].bb.init_scan(BBObject::NON_DESTRUCTIVE);
			int v1=m_colSets[col].bb.next_bit();
			int v2=m_colSets[col].bb.next_bit();
			int nBB=bb_non_neighbors.number_of_bitblocks();
			for(int i=0; i<nBB; i++){
				bb_non_neighbors.get_bitboard(i)=gc->get_neighbors(v1).get_bitboard(i)&gc->get_neighbors(v2).get_bitboard(i);
			}
		}/*else if(m_colSets[col].size==3){
			m_colSets[col].bb.init_scan(BBObject::NON_DESTRUCTIVE);
			int v1=m_colSets[col].bb.next_bit();
			int v2=m_colSets[col].bb.next_bit();
			int v3=m_colSets[col].bb.next_bit();
			int nBB=bb_non_neighbors.number_of_bitblocks();
			for(int i=0; i<nBB; i++){
				bb_non_neighbors.get_bitboard(i)=gc->get_neighbors(v1).get_bitboard(i)&gc->get_neighbors(v2).get_bitboard(i) &gc->get_neighbors(v3).get_bitboard(i);
			}
		}*/

		//main loop
		int v=EMPTY_ELEM;
		for(int c1=col-1; c1>=1; c1--){
			int pc=m_colSets[c1].bb.single_joint(bb_non_neighbors, v);		//sets MUST BE in this order
			if(pc==0) {
				return 1;			//inconsistency found
			}else if(pc==1){		
				bb_non_neighbors|=gc->get_neighbors(v);	 //reduces vertex inference set
			}
		}
	}

	return 0;		
}

template <class graph_t, class bitboard_t >
inline
int InfraOp<graph_t,bitboard_t>::infrachrom_one_shot_pc(int cmax){
///////////////////////
// infrachrom_one_shot with explicit computation of population size
// first_update:1/7/16
// RETURNS 1 or 0
// REMARKS: Only inference which modifies color BBDD


	//determines inference bitset
	int col=EMPTY_ELEM;
	int pc=0; 
	for(int c=cmax; c>=2; c--){
		pc=m_colSets[c].bb.popcn64();
		if (pc<MAX_CLAUSE_ONE_SHOT_SIZE){
			col=c;
			break;
		}
	}
	
	if(col!=EMPTY_ELEM){
		if(pc==1){
			bb_non_neighbors=gc->get_neighbors(m_colSets[col].bb.lsbn64());		//*** TODO. change name
		}else if(pc==2){
			m_colSets[col].bb.init_scan(BBObject::NON_DESTRUCTIVE);
			int v1=m_colSets[col].bb.next_bit();
			int v2=m_colSets[col].bb.next_bit();
			int nBB=bb_non_neighbors.number_of_bitblocks();
			for(int i=0; i<nBB; i++){
				bb_non_neighbors.get_bitboard(i)=gc->get_neighbors(v1).get_bitboard(i)&gc->get_neighbors(v2).get_bitboard(i);
			}
		}/*else if(m_colSets[col].size==3){
			m_colSets[col].bb.init_scan(BBObject::NON_DESTRUCTIVE);
			int v1=m_colSets[col].bb.next_bit();
			int v2=m_colSets[col].bb.next_bit();
			int v3=m_colSets[col].bb.next_bit();
			int nBB=bb_non_neighbors.number_of_bitblocks();
			for(int i=0; i<nBB; i++){
				bb_non_neighbors.get_bitboard(i)=gc->get_neighbors(v1).get_bitboard(i)&gc->get_neighbors(v2).get_bitboard(i) &gc->get_neighbors(v3).get_bitboard(i);
			}
		}*/

		//main loop
		int v=EMPTY_ELEM;
		for(int c1=col-1; c1>=1; c1--){
			int pc=m_colSets[c1].bb.single_joint(bb_non_neighbors, v);		//sets MUST BE in this order
			if(pc==0) {
				return 1;			//inconsistency found
			}else if(pc==1){		
				bb_non_neighbors|=gc->get_neighbors(v);	 //reduces vertex inference set
			}
		}
	}

	return 0;		
}


template <class graph_t, class bitboard_t >
inline
bool InfraOp<graph_t,bitboard_t>::filter(int* lv, int size){
//////////////////////
// guided by color stack a la incMaxCLQ
// RETURNS TRUE if filter succeeds, FALSE if it does not
//
// REMARKS
// 1.assumes size of color sets is updated
	bb_node_state_active.erase_bit();		//**TODO-on the fly while coloring
	color_filter_stack.erase();
	for(int c=NB_OF_COLORS; c>=1; c--){		
		if(m_colSets[c].size<=4) 
				color_filter_stack.push(c);
	}

	//main loop
	for(int i=0; i<color_filter_stack.pt; i++){
		int c=color_filter_stack.get_elem(i);
		if(m_colSets[c].size==1 && unitiset_filter(c, lv, size)){
			//LOG_INFO("ONE-SHOT-SIZE-1");
			return true;
		}
		if(m_colSets[c].size==2 && biniset_filter(c, lv, size)){
			//LOG_INFO("ONE-SHOT-SIZE-2");
			return true;
		}
		if(m_colSets[c].size==3 && triset_filter(c, lv, size)){
			//LOG_INFO("ONE-SHOT-SIZE-3");
			return true;
		}
		if(m_colSets[c].size==4 && fouriset_filter(c, lv, size)){
			//LOG_INFO("ONE-SHOT-SIZE-4");
			return true;
		}
	}

return false;		//no filter possible
}

template <class graph_t, class bitboard_t >
inline
bool InfraOp<graph_t,bitboard_t>::filter(){
//////////////////////
// guided by color stack a la incMaxCLQ
// RETURNS TRUE if filter succeeds, FALSE if it does not
//
// REMARKS
// 1.assumes size of color sets is updated
// 2.assumes that active nodes are updated
					
	color_filter_stack.erase();
	for(int c=NB_OF_COLORS; c>=1; c--){		
		if(m_colSets[c].size<=5) 
				color_filter_stack.push(c);
	}

	//main loop
	for(int i=0; i<color_filter_stack.pt; i++){
		int c=color_filter_stack.get_elem(i);
		if(m_colSets[c].size==1 && unitiset_filter(c)){
			//LOG_INFO("ONE-SHOT-SIZE-1");
			return true;
		}
		if(m_colSets[c].size==2 && biniset_filter(c)){
			//LOG_INFO("ONE-SHOT-SIZE-2");
			return true;
		}
		if(m_colSets[c].size==3 && triset_filter(c)){
			//LOG_INFO("ONE-SHOT-SIZE-3");
			return true;
		}
		if(m_colSets[c].size==4 && fouriset_filter(c)){
			//LOG_INFO("ONE-SHOT-SIZE-4");
			return true;
		}
#ifdef FIVE_ISET_FILTER 
		if(m_colSets[c].size==5 && fiveiset_filter(c)){
			//LOG_INFO("ONE-SHOT-SIZE-4");
			return true;
		}
#endif
	}


return false;		//no filter possible
}



template <class graph_t, class bitboard_t >
inline
bool InfraOp<graph_t,bitboard_t>::unitiset_filter(int c, int* lv, int size){
////////////////
// returns TRUE if filter succeeds, FALSE if it does not
	int v;
	bitarray& nnb=gc->get_neighbors(m_colSets[c].bb.lsbn64());
	for(int i=size-1; i>=1; i--){
	//for(int i=0; i<size; i++){
		v=lv[i];
		//*** TODO-CHECK node_iset_no[v] should not be c
		if(!bb_node_state_active.is_bit(v) && nnb.is_bit(v)){
			bb_node_state_active.set_bit(v);
			m_colSets[node_iset_no[v]].erase_bit(v);
			if(m_colSets[node_iset_no[v]].size==4){ 
				color_filter_stack.push(node_iset_no[v]);
			}else if(m_colSets[node_iset_no[v]].size==0) return true;
		}
	}

	return false;
}

template <class graph_t, class bitboard_t >
inline
bool InfraOp<graph_t,bitboard_t>::unitiset_filter(int c){
////////////////
// returns TRUE if filter succeeds, FALSE if it does not
	int v;
	bitarray& nnb=gc->get_neighbors(m_colSets[c].bb.lsbn64());
	bb_node_state_active.init_scan(bbo::NON_DESTRUCTIVE);
	while(true){
		v=bb_node_state_active.next_bit();
		if(v==EMPTY_ELEM) return false;
		if(node_iset_no[v]!=c && nnb.is_bit(v)){
			bb_node_state_active.erase_bit(v);
			m_colSets[node_iset_no[v]].erase_bit(v);
			if(m_colSets[node_iset_no[v]].size==4){ 
				color_filter_stack.push(node_iset_no[v]);
			}else if(m_colSets[node_iset_no[v]].size==0) return true;
		}
	}

	return false;
}


template <class graph_t, class bitboard_t >
inline
bool InfraOp<graph_t,bitboard_t>::biniset_filter(int c, int* lv, int size){
////////////////
// returns TRUE if filter succeeds, FALSE if it does not
	int v;
	m_colSets[c].bb.init_scan(bbo::NON_DESTRUCTIVE);
	//int v1=m_colSets[c].bb.next_bit();
	//int v2=m_colSets[c].bb.next_bit();
	bitarray& nnb1=gc->get_neighbors(m_colSets[c].bb.next_bit());
	bitarray& nnb2=gc->get_neighbors(m_colSets[c].bb.next_bit());
	for(int i=size-1; i>=1; i--){
	//for(int i=0; i<size; i++){
		v=lv[i];
		//*** TODO-CHECK node_iset_no[v] should not be c
		if(!bb_node_state_active.is_bit(v) && nnb1.is_bit(v) && nnb2.is_bit(v)){
			bb_node_state_active.set_bit(v);
			m_colSets[node_iset_no[v]].erase_bit(v);
			if(m_colSets[node_iset_no[v]].size==4){ 
				color_filter_stack.push(node_iset_no[v]);
			}else if(m_colSets[node_iset_no[v]].size==0) return true;
		}
	}
	return false;
}

template <class graph_t, class bitboard_t >
inline
bool InfraOp<graph_t,bitboard_t>::biniset_filter(int c){
////////////////
// returns TRUE if filter succeeds, FALSE if it does not
	int v;
	m_colSets[c].bb.init_scan(bbo::NON_DESTRUCTIVE);
	bitarray& nnb1=gc->get_neighbors(m_colSets[c].bb.next_bit());
	bitarray& nnb2=gc->get_neighbors(m_colSets[c].bb.next_bit());
	bb_node_state_active.init_scan(bbo::NON_DESTRUCTIVE);
	while(true){
		v=bb_node_state_active.next_bit();
		if(v==EMPTY_ELEM) return false;
		if(node_iset_no[v]!=c && nnb1.is_bit(v) && nnb2.is_bit(v)) {
			bb_node_state_active.erase_bit(v);
			m_colSets[node_iset_no[v]].erase_bit(v);
			if(m_colSets[node_iset_no[v]].size==4){ 
				color_filter_stack.push(node_iset_no[v]);
			}else if(m_colSets[node_iset_no[v]].size==0) return true;
		}
	}
	return false;
}



template <class graph_t, class bitboard_t >
inline
bool InfraOp<graph_t,bitboard_t>::triset_filter(int c, int* lv, int size){
////////////////
// returns TRUE if filter succeeds, FALSE if it does not
	int v;
	m_colSets[c].bb.init_scan(bbo::NON_DESTRUCTIVE);
	/*int v1=m_colSets[c].bb.next_bit();
	int v2=m_colSets[c].bb.next_bit();
	int v3=m_colSets[c].bb.next_bit();*/
	bitarray& nnb1=gc->get_neighbors(m_colSets[c].bb.next_bit());
	bitarray& nnb2=gc->get_neighbors(m_colSets[c].bb.next_bit());
	bitarray& nnb3=gc->get_neighbors(m_colSets[c].bb.next_bit());
	for(int i=size-1; i>=1; i--){
//	for(int i=0; i<size; i++){
		v=lv[i];
		//*** TODO-CHECK node_iset_no[v] should not be c
		if(!bb_node_state_active.is_bit(v) && nnb1.is_bit(v) && nnb2.is_bit(v) && nnb3.is_bit(v) ){
			bb_node_state_active.set_bit(v);
			m_colSets[node_iset_no[v]].erase_bit(v);
			if(m_colSets[node_iset_no[v]].size==4){ 
				color_filter_stack.push(node_iset_no[v]);
			}else if(m_colSets[node_iset_no[v]].size==0) return true;
		}
	}

	return false;
}

template <class graph_t, class bitboard_t >
inline
bool InfraOp<graph_t,bitboard_t>::triset_filter(int c){
////////////////
// returns TRUE if filter succeeds, FALSE if it does not
	int v;

	m_colSets[c].bb.init_scan(bbo::NON_DESTRUCTIVE);
	bitarray& nnb1=gc->get_neighbors(m_colSets[c].bb.next_bit());
	bitarray& nnb2=gc->get_neighbors(m_colSets[c].bb.next_bit());
	bitarray& nnb3=gc->get_neighbors(m_colSets[c].bb.next_bit());
	bb_node_state_active.init_scan(bbo::NON_DESTRUCTIVE);
	while(true){
		v=bb_node_state_active.next_bit();
		if(v==EMPTY_ELEM) return false;
		if(node_iset_no[v]!=c &&  nnb1.is_bit(v) && nnb2.is_bit(v) && nnb3.is_bit(v)){
			bb_node_state_active.erase_bit(v);
			m_colSets[node_iset_no[v]].erase_bit(v);
			if(m_colSets[node_iset_no[v]].size==4){ 
				color_filter_stack.push(node_iset_no[v]);
			}else if(m_colSets[node_iset_no[v]].size==0) return true;
		}
	}

	return false;
}

template <class graph_t, class bitboard_t >
inline
bool InfraOp<graph_t,bitboard_t>::fouriset_filter(int c, int* lv, int size){
////////////////
// returns TRUE if filter succeeds, FALSE if it does not
	int v;
	m_colSets[c].bb.init_scan(bbo::NON_DESTRUCTIVE);
	bitarray& nnb1=gc->get_neighbors(m_colSets[c].bb.next_bit());
	bitarray& nnb2=gc->get_neighbors(m_colSets[c].bb.next_bit());
	bitarray& nnb3=gc->get_neighbors(m_colSets[c].bb.next_bit());
	bitarray& nnb4=gc->get_neighbors(m_colSets[c].bb.next_bit());
	for(int i=size-1; i>=1; i--){
//	for(int i=0; i<size; i++){
		if(v==EMPTY_ELEM) return false;
		v=lv[i];
		//*** TODO-CHECK node_iset_no[v] should not be c
		if(!bb_node_state_active.is_bit(v) && nnb1.is_bit(v) && nnb2.is_bit(v) && nnb3.is_bit(v) && nnb4.is_bit(v) ){
			bb_node_state_active.set_bit(v);
			m_colSets[node_iset_no[v]].erase_bit(v);
			if(m_colSets[node_iset_no[v]].size==4){ 
				color_filter_stack.push(node_iset_no[v]);
			}else if(m_colSets[node_iset_no[v]].size==0) return true;
		}
	}
	return false;
}


template <class graph_t, class bitboard_t >
inline
bool InfraOp<graph_t,bitboard_t>::fouriset_filter(int c){
////////////////
// returns TRUE if filter succeeds, FALSE if it does not
	int v;
	m_colSets[c].bb.init_scan(bbo::NON_DESTRUCTIVE);
	bitarray& nnb1=gc->get_neighbors(m_colSets[c].bb.next_bit());
	bitarray& nnb2=gc->get_neighbors(m_colSets[c].bb.next_bit());
	bitarray& nnb3=gc->get_neighbors(m_colSets[c].bb.next_bit());
	bitarray& nnb4=gc->get_neighbors(m_colSets[c].bb.next_bit());
	//*** possibly compute the AND Bitset
	bb_node_state_active.init_scan(bbo::NON_DESTRUCTIVE);
	while(true){
		v=bb_node_state_active.next_bit();
		if(v==EMPTY_ELEM) return false;
		if(node_iset_no[v]!=c && nnb1.is_bit(v) && nnb2.is_bit(v) && nnb3.is_bit(v) && nnb4.is_bit(v)){
			bb_node_state_active.erase_bit(v);
			m_colSets[node_iset_no[v]].erase_bit(v);
			if(m_colSets[node_iset_no[v]].size==4){ 
				color_filter_stack.push(node_iset_no[v]);
			}else if(m_colSets[node_iset_no[v]].size==0) return true;
		}
	}
	return false;
}

template <class graph_t, class bitboard_t >
inline
bool InfraOp<graph_t,bitboard_t>::fiveiset_filter(int c){
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
	bb_node_state_active.init_scan(bbo::NON_DESTRUCTIVE);
	while(true){
		v=bb_node_state_active.next_bit();
		if(v==EMPTY_ELEM) return false;
		if(node_iset_no[v]!=c && nnb1.is_bit(v) && nnb2.is_bit(v) && nnb3.is_bit(v) && nnb4.is_bit(v) && nnb5.is_bit(v)){
			bb_node_state_active.erase_bit(v);
			m_colSets[node_iset_no[v]].erase_bit(v);
			if(m_colSets[node_iset_no[v]].size==5){ 
				color_filter_stack.push(node_iset_no[v]);
			}else if(m_colSets[node_iset_no[v]].size==0) return true;
		}
	}
	return false;
}



template <class graph_t, class bitboard_t>
inline
int InfraOp<graph_t,bitboard_t>::find_inconsistency_unit_clause(int seed, int* lv, int size, bitarray& bb_free_col){
////////////////
// Overrides main unit literal propagation (UL) procedure by iterating over vertices that are available
// at each inference. The procedure is thus somewhat similar to sequential greedy coloring using bitsets (EXPLAIN)
//
// OLD COMMENTS
// RETURNS the number of colors of the inconsistent set for that seed  (0 if unsuccesful)
// Removes inconsistent colors from bb_free_col
//
// REMARKS: Does not change color sets
	
	int num_col=0;
	bb_used_colors.erase_bit();
	int v=EMPTY_ELEM; int w=EMPTY_ELEM;
	bb_non_neighbors=gc->get_neighbors(seed);			//bb_v in my pseudocode
		
	for(int i=0; i<size; i++){
		int c=node_iset_no[lv[i]];
		//if(!bb_non_neighbors.is_bit(lv[i])) continue;		//directs the search to colors more likely to produce UNSAT cores!!
		if(!bb_free_col.is_bit(c)) continue;				//check that it is not a pevious UNSAT core
		if(bb_used_colors.is_bit(c)) continue;
					
		//main unit literal inference
		int pc=m_colSets[c].bb.double_joint(bb_non_neighbors, v, w);
		if(pc==0){												//INCONSISTENCY FOUND
			bb_used_colors.set_bit(c);
			bb_free_col.erase_bit(bb_used_colors);
			return num_col+1;
		}else if(pc==1){										//PROPAGATES UNIT LITERAL
			num_col++;
			bb_used_colors.set_bit(c);
			bb_non_neighbors|=gc->get_neighbors(v);				//NON_NEIGHBORS in the inference ***TODO opt?
		}else if(pc==2){
			bb_used_colors.set_bit(c);							//*** check wether this is needed
			int nBB=bb_non_neighbors.number_of_bitblocks();
			bitboard_t& bbnv=gc->get_neighbors(v);
			bitboard_t& bbnw=gc->get_neighbors(w);
			for(int k=0; k<nBB; k++){
				bb_non_neighbors.get_bitboard(k)|=bbnv.get_bitboard(k)& bbnw.get_bitboard(k);
			}
			//***note that v and w are also going into bb_non_neighbors 
		} 
	}
	return 0;		//no inference found: 0 colors involved
}


template <class graph_t, class bitboard_t>
inline
int InfraOp<graph_t,bitboard_t>::find_inconsistency_non_unit_clause(int* lv, int size, bitarray& bb_free_col){	
////////////////
// Overrides main unit literal propagation (UL) procedure by iterating over vertices that are available
// at each inference. The procedure is thus somewhat similar to sequential greedy coloring using bitsets (EXPLAIN)
//
// OLD COMMENTS
// RETURNS the number of colors of the inconsistent set for that seed  (0 if unsuccesful)
// Removes inconsistent colors from bb_free_col
//
// REMARKS: Does not change color sets
	
	int num_col=0;
	bb_used_colors.erase_bit();
	int v=EMPTY_ELEM; int w=EMPTY_ELEM;
		
	for(int i=0; i<size; i++){
		int node=lv[i];
		int c=node_iset_no[node];		
		if( bb_free_col.is_bit(c) && !bb_used_colors.is_bit(c) &&
			bb_node_state_active.is_bit(node) && bb_non_neighbors.is_bit(node) /* directs search-better colors */
			){		
				int pc=m_colSets[c].bb.single_joint(bb_non_neighbors, v);
				//int pc=m_colSets[c].bb.double_joint(bb_non_neighbors, v, w);
				if(pc==0){												//INCONSISTENCY FOUND
					bb_used_colors.set_bit(c);
					bb_free_col.erase_bit(bb_used_colors);
					return num_col+1;
				}else if(pc==1){										//PROPAGATES UNIT LITERAL
					num_col++;
					bb_used_colors.set_bit(c);
					bb_non_neighbors|=gc->get_neighbors(v);				//NON_NEIGHBORS in the inference ***TODO opt?
					//}else if(pc==2){
					//	bb_used_colors.set_bit(c);						//*** check whether this is needed
					//	int nBB=bb_non_neighbors.number_of_bitblocks();
					//	for(int k=0; k<nBB; k++){
					//		bb_non_neighbors.get_bitboard(k)|=gc->get_neighbors(v).get_bitboard(k)& gc->get_neighbors(w).get_bitboard(k);
					//	}

					//	//***note that v and w are also going into bb_non_neighbors 
				} 
		}
	}
		
	return 0;		//no inference found: 0 colors involved
}

template <class graph_t, class bitboard_t>
inline
int InfraOp<graph_t,bitboard_t>::init_maxsatz(int v, int clq_size){
///////////////
// PARAMS: v: expanded vertex which has led to the current subgraph
//		   clq_size: kmin for the current subgraph (BEST_CLQ_SOL-DEPTH)
//
// FUNCTION: init operations previous to call infrachrom solver
// RETURNS: number of conflicts found or EMPTY_ELEM when conflicts>=clq_size (includes expanded vertex)
//
// REMARKS:
// 1. assumes ACTIVE VERTICES ARE SET!

	int nb_conflicts=0;
	reset_added_nodes();

	//add new singleton color DB containing v
	m_colSets[++NB_OF_COLORS].erase_bit();	
	m_colSets[NB_OF_COLORS].push(v);
	bb_node_state_active.set_bit(v);
	node_iset_no[v]=NB_OF_COLORS;
	node_reason[v]=NO_REASON;
	
	//***active vertices (assumed set)

	//nb_conflicts=maxsatz(clq_size);
	int nb_max_conf=NB_OF_COLORS-clq_size;
	nb_conflicts=maxsatz(nb_max_conf);

	reset_enlarged_isets();		//*** Check if this restoring can be optimized

	
	//remove singleton color DB
	NB_OF_COLORS--;
	reset_added_nodes();
	
	
	if(nb_conflicts>=nb_max_conf) return EMPTY_ELEM;
	
return nb_conflicts;
}

template <class graph_t, class bitboard_t>
inline
int InfraOp<graph_t,bitboard_t>::init_maxsatz_init_ub(int v, int clq_size){
///////////////
// PARAMS: v: expanded vertex which has led to the current subgraph
//		   clq_size: kmin for the current subgraph (BEST_CLQ_SOL-DEPTH)
//
// FUNCTION: init operations previous to call infrachrom solver
// RETURNS: number of conflicts found or EMPTY_ELEM when conflicts>=clq_size (includes expanded vertex)
//
// REMARKS:
// 1. assumes ACTIVE VERTICES ARE SET!

	int nb_conflicts=0;
	reset_added_nodes();

	//add new singleton color DB containing v
	m_colSets[++NB_OF_COLORS].erase_bit();	
	m_colSets[NB_OF_COLORS].push(v);
	bb_node_state_active.set_bit(v);
	node_iset_no[v]=NB_OF_COLORS;
	node_reason[v]=NO_REASON;
	
	//***active vertices (assumed set)

	//nb_conflicts=maxsatz(clq_size);
	int nb_max_conf=NB_OF_COLORS-clq_size;
	nb_conflicts=maxsatz_init_ub(nb_max_conf);

	reset_enlarged_isets();		//*** Check if this restoring can be optimized

	
	//remove singleton color DB
	NB_OF_COLORS--;
	reset_added_nodes();
	
	
	if(nb_conflicts>=nb_max_conf) return EMPTY_ELEM;
	
return nb_conflicts;
}

template <class graph_t, class bitboard_t>
inline
int InfraOp<graph_t,bitboard_t>::init_maxsatz(int clq_size){
///////////////
// PARAMS: v: expanded vertex which has led to the current subgraph
//		   clq_size: kmin for the current subgraph (BEST_CLQ_SOL-DEPTH)
//
// FUNCTION: init operations previous to call infrachrom solver
// RETURNS: number of conflicts found or EMPTY_ELEM when conflicts>=clq_size (includes expanded vertex)
//
// REMARKS:
// 1. assumes ACTIVE VERTICES ARE SET

	int nb_conflicts=0;
	reset_added_nodes();
	
		
	//***active vertices (assumed set)
	int nb_max_conf=NB_OF_COLORS-clq_size;
	nb_conflicts=maxsatz(nb_max_conf);

	reset_enlarged_isets();		//*** Check if this restoring can be optimized
	
	//remove singleton color DB
	reset_added_nodes();	
	
	if(nb_conflicts>=nb_max_conf) return EMPTY_ELEM;
	return nb_conflicts;
}

template <class graph_t, class bitboard_t>
inline
void InfraOp<graph_t,bitboard_t>::init_inc_maxsatz(){
///////////////
// initialization of simplified maxsatz which looks for a single conflict
//
// REMARKS:
// 1. assumes ACTIVE NODES ARE SET
	
	//*** reset_added_nodes()?

	color_unit_stack.pt=0;
	for(int i=NB_OF_COLORS; i>=1; i--){				//important: smallest colors first!
//	for(int i=1; i<=NB_OF_COLORS; i++){				
		iset_used_in_test[i]=FALSE_VAL;
		color_state_active.set_bit(i);
		if (m_colSets[i].size==1){
			color_unit_stack.push(i);
		}
	}

	reset_added_nodes();
	color_conflict_stack.pt=0;
	color_enlarged_stack.pt=0;
}


template <class graph_t, class bitboard_t>
inline
int InfraOp<graph_t,bitboard_t>::maxsatz(int max_num_conf){
////////////////////
// driver for main infrachrom reasoning

	//*** check clq_size>0;

	//init unit_stack
	color_unit_stack.erase();
	for(int c=NB_OF_COLORS; c>=1; c--){
		color_state_active.set_bit(c);
		if(m_colSets[c].size==1){
			color_unit_stack.push(c);
		}
	}

	int save_color_unit_stack_pt=color_unit_stack.pt;
	
	//set stacks
	color_conflict_stack.erase();
	node_stack.erase();
	color_passive_stack.erase();
	color_reduced_stack.erase();
	color_enlarged_stack.erase();
	color_unit_dyn_stack.erase();
	/*int saved_node_stack_pt=0;
	int saved_color_passive_stack_pt=0;
	int save_color_reduced_stack_pt=0;*/
			
	int nb_conflicts=0;
	int iset;
	while( (iset=unit_iset_process())!=NO_CONFLICT ){						
		lookback_for_maxsatz(iset);
		reset_context_for_maxsatz(0, 0,	0, save_color_unit_stack_pt);		//does not eliminate original unit clauses from the stack
		enlarge_involved_set();
		nb_conflicts++;
		if (nb_conflicts>=max_num_conf)
			break;
	}

	//the stacks might not be empty here (NO_CONFLICT state) 
	reset_context_for_maxsatz(0, 0, 0, save_color_unit_stack_pt);			

	if (nb_conflicts>=max_num_conf)
				return nb_conflicts;

#ifdef TEST_LOOK_AHEAD_ISETS	
	 nb_conflicts=maxsatz_lookahead(nb_conflicts, max_num_conf);
	 	
	 if (nb_conflicts>=max_num_conf){
		 //LOG_INFO("CUT DURING TESTS INFRACHROM");
		 return nb_conflicts;
	 }
#endif

#ifdef TEST_ELIMINATE_FAILED_NODES
	if( (nb_conflicts + 1) == max_num_conf){
		 if(test_by_eliminate_failed_nodes()){
			 nb_conflicts++;
			 //LOG_INFO("CONF ELIMINATE FAILED NODES");
		 }
	 }
#endif	

	return nb_conflicts;
}


template <class graph_t, class bitboard_t>
inline
int InfraOp<graph_t,bitboard_t>::maxsatz_init_ub(int max_num_conf){
////////////////////
// driver for main infrachrom reasoning which at the end always does
// ELIMIATE_NODES_TEST to reduce by one the total nb of conf if possible
//
// REMARKS: Used to compute initial UB for the input graph

	//*** check clq_size>0;

	//init unit_stack
	color_unit_stack.erase();
	for(int c=NB_OF_COLORS; c>=1; c--){
		color_state_active.set_bit(c);
		if(m_colSets[c].size==1){
			color_unit_stack.push(c);
		}
	}

	int save_color_unit_stack_pt=color_unit_stack.pt;
	
	//set stacks
	color_conflict_stack.erase();
	node_stack.erase();
	color_passive_stack.erase();
	color_reduced_stack.erase();
	color_enlarged_stack.erase();
	color_unit_dyn_stack.erase();
	/*int saved_node_stack_pt=0;
	int saved_color_passive_stack_pt=0;
	int save_color_reduced_stack_pt=0;*/
			
	int nb_conflicts=0;
	int iset;
	while( (iset=unit_iset_process())!=NO_CONFLICT ){						
		lookback_for_maxsatz(iset);
		reset_context_for_maxsatz(0, 0,	0, save_color_unit_stack_pt);		//does not eliminate original unit clauses from the stack
		enlarge_involved_set();
		nb_conflicts++;
		/*if (nb_conflicts>=max_num_conf)
			break;*/
	}

	//the stacks might not be empty here (NO_CONFLICT state) 
	reset_context_for_maxsatz(0, 0, 0, save_color_unit_stack_pt);			

	//if (nb_conflicts>=max_num_conf)
	//			return nb_conflicts;

	//MUST DO THIS ANALYSIS	
	 nb_conflicts=maxsatz_lookahead(nb_conflicts, max_num_conf);
	 if (nb_conflicts>=max_num_conf){
		 LOG_INFO("CUT DURING TESTS INFRACHROM");
		 return nb_conflicts;
	 }


	 //MUST DO THIS ANALYSIS
	  if(test_by_eliminate_failed_nodes()){
		 nb_conflicts++;
		// LOG_INFO("CONF ELIMINATE FAILED NODES");
	 }


	return nb_conflicts;
}


template <class graph_t, class bitboard_t>
inline
int InfraOp<graph_t,bitboard_t>::inc_maxsatz(int v){
//////////////////
// PARAMS: v is the node in a singleton color (kmin) that wants to be filtered
//
// RETURNS: EMPTY_ELEM if a conflict is found or FALSE_VAL otherwise 
//
// REMARKS
// 1.Assumes singleton color iset={v] is last in UNIT_ISET_STACK 
	
	int save_color_unit_stack_pt=color_unit_stack.pt;
	node_stack.pt=0;
	color_passive_stack.pt=0;
	color_reduced_stack.pt=0;

	//swap first-last colors in color_unit_stack
	//the last color is the singleton color of v
	int first_color=color_unit_stack.get_elem(0);
	color_unit_stack.stack[0]=color_unit_stack.stack[color_unit_stack.pt-1];
	color_unit_stack.stack[color_unit_stack.pt-1]=first_color;
	color_unit_dyn_stack.pt=0;

	int iset;
	if ((iset=unit_iset_process())!=NO_CONFLICT) {
		lookback_for_maxsatz(iset);
		reset_context_for_maxsatz(0, 0, 0, save_color_unit_stack_pt);
		enlarge_involved_set();
		return EMPTY_ELEM;
	}

	//the stacks might not be empty here (NO_CONFLICT state) -PSS
	reset_context_for_maxsatz(0, 0, 0, save_color_unit_stack_pt);	

	//attempt to find a conflict in color classes of sizes 2 or 3
	int result=inc_maxsatz_lookahead(save_color_unit_stack_pt);
	reset_context_for_maxsatz(0, 0, 0,save_color_unit_stack_pt);
	
	if(result==EMPTY_ELEM) 
		return EMPTY_ELEM;	//conflict found




//**** NOT WORKING current version of eliminate failed nodes becuase
// it does not store conflicts, so cannot be incremental
//*** prepare an incremental variant
	//result=test_by_eliminate_failed_nodes();
	//reset_context_for_maxsatz(0, 0, 0,save_color_unit_stack_pt);
	//if(result==true){
	//	return EMPTY_ELEM;	//conflict found
	//}

	
return FALSE_VAL;		

}

template <class graph_t, class bitboard_t>
inline
int InfraOp<graph_t,bitboard_t>::unit_iset_process(){
/////////////
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

	color_unit_dyn_stack.erase();		//not sure if it is needed
	return NO_CONFLICT;
}

template <class graph_t, class bitboard_t>
inline
int InfraOp<graph_t,bitboard_t>::fix_unit_color(int iset){
/////////////////
// PARAMS: color is a singleton color set
//
// FUNCTION: driver for main UL inference with color
// 
// REMARKS: SingleDisjoint is for DEBUGGING. 
//			**TODO-OPTIMIZE to a lazy detection of the first
//			(and only) node of the intersection of the active
//			nodes and the vertices in iset (1/12/16)

	int v=EMPTY_ELEM;
	int pc=m_colSets[iset].bb.single_disjoint(bb_node_state_active, v);				 //also finds weakened nodes in last bitblock
	
	if(pc!=1){
		if(pc==0){
			cout<<endl;m_colSets[iset].bb.print(); cout<<":"<<m_colSets[iset].size<<endl;
			bb_node_state_active.print(); cout<<endl;

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
			bb_node_state_active.print(); cout<<endl;

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
int InfraOp<graph_t,bitboard_t>::fix_node_for_iset(int v, int iset){
//////////////////////
// PARAMS: v: single (not weakened) ACTIVE vertex of iset
//		   iset: color set containing single active node v
//
// FUNCTION: erases active non-neighbors of v from color sets different from iset
//			 updates state of v, iset, excluded non-neighbors, adds respective color
//			 sets to stack etc.
//
// REMARKS: note that propagation only occurs in active vertices, so passive nodes in color sets will never be reduced/excluded-
	
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
		if( bb=(non_nbb.get_bitboard(nBB) & bb_node_state_active.get_bitboard(nBB)) ){
			//decode vertices
			offset=WMUL(nBB);
			while(true){
				if( (nnbor=BitBoard::lsb64_intrinsic(bb))==EMPTY_ELEM) {break;}
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

template <class graph_t, class bitboard_t>
inline
int InfraOp<graph_t,bitboard_t>::fix_added_node_for_iset(int v, int iset){
//////////////////////
// PARAMS: v: single WEAKENED vertex of iset
//		   iset: ACTIVE color set containing single WEAKENED node v
//
// FUNCTION: adds weakened variable to conflicting nodes, updates state info etc.
//			 
// REMARKS:	
	
	//update state info of v and iset
	color_state_active.erase_bit(iset);
	color_passive_stack.push(iset);
	assign_node_value(v, FALSE_VAL, iset);				//note it is set to FALSE for lookback
	int* isets=node_conflict_set[v];
		
	for(int c_iset=*isets; c_iset!=NONE; c_iset=*(++isets)){
		if (color_state_active.is_bit(c_iset)) {		//***check if it is really required
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
int InfraOp<graph_t,bitboard_t>::exclude_noneibor(int nnbor, int reason_iset){
///////////////////////
// PARAMS: noneibor: vertex to be set to FALSE
//		   iset:	 cause of noneibor values
//
// FUNCTION: implicitly removes noneibor from its color set and updates state info

	int my_iset=node_iset_no[nnbor];
	assign_node_value(nnbor, FALSE_VAL, reason_iset);
	if (color_state_active.is_bit(my_iset)) {		//nnbor could be active but its color set passive? Should not occurr
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
void InfraOp<graph_t,bitboard_t>::lookback_for_maxsatz(int iset){
/////////////////////
// PARAMS: iset: empty conflicting set
//
// FUNCION: Fills REASON STACK with colors related to the conflict by tracing the cause of
//			FALSE values of nodes (obviously includes added nodes)
//
// REMARKS: nodes in color sets are filtered as PASSIVE (with bitmasks)

	int offset,node;
	color_reason_stack.erase();
	color_reason_stack.push(iset);
	iset_involved[iset]=TRUE_VAL;		//to avoid repetition

	int my_iset;
	for(int i=0; i<color_reason_stack.pt; i++){
		my_iset=color_reason_stack.get_elem(i);

		//loop through nodes of riset (filter with passive state nodes)
		bitboard_t& cnodes=m_colSets[my_iset].bb;
		BITBOARD bb;
		for(int nBB=0; nBB<NB_OF_BB_ADDED_NODES; nBB++){
			if( (bb=cnodes.get_bitboard(nBB) &~ bb_node_state_active.get_bitboard(nBB)) ){
				//decode vertices
				offset=WMUL(nBB);
				while(true){
					if( (node=BitBoard::lsb64_intrinsic(bb))==EMPTY_ELEM) {break;}
					bb^=Tables::mask[node];
					
					node+=offset;
					if( node_value[node]==FALSE_VAL /*redundant passive?*/ && node_reason[node]!=NO_REASON 
						&& iset_involved[node_reason[node]]==FALSE_VAL /*forced*/){
				
							int reason_iset=node_reason[node];
							color_reason_stack.push(reason_iset);
							//node_reason[node]=NO_REASON;					//check if required 
							iset_involved[reason_iset]=TRUE_VAL;

					}
				}
			}
		}
	}

	//clear iset_involved state
	for(int i=0; i<color_reason_stack.pt; i++)
		iset_involved[color_reason_stack.get_elem(i)]=FALSE_VAL;
}

template <class graph_t, class bitboard_t>
inline
int InfraOp<graph_t,bitboard_t>::enlarge_involved_set(){
//////////////
// Enlarges color sets in REASON_STACK (involved in a conflict) by adding a new node. 
// and updates CONFLICT_STACK ending with NONE, together with node_conflict_set[ADDED_NODES]

	 int riset;
	 node_conflict_set[ADDED_NODES]= &color_conflict_stack.stack[color_conflict_stack.pt];
	 bb_node_state_active.set_bit(ADDED_NODES);
	 for(int i=0; i<color_reason_stack.pt; i++){
		 riset=color_reason_stack.get_elem(i);
		
		 //add node
		 if( m_colSets[riset].bb.is_bit(ADDED_NODES)){
			 m_colSets[riset].bb.print(); cout<<" adding node:  "<<ADDED_NODES<<endl;
			 LOG_ERROR("enlarged_involved_set():error in conflict set");
		 }
		 m_colSets[riset].push(ADDED_NODES);
		 color_enlarged_stack.push(riset);
		 color_conflict_stack.push(riset);
	 }

	 color_conflict_stack.push(NONE);		//ends in NONE for fix_added_node_for_iset(..) to work
	 ADDED_NODES++;
	/* if(nb_added_nodes()>50){
		 LOG_INFO("DANGER NB ADDED NODE:"<<50);
	 }*/
	 return 0;
}


template <class graph_t, class bitboard_t>
inline
int InfraOp<graph_t,bitboard_t>::enlarge_stored_involved_sets(){
//////////////
// Enlarges color sets in REASON_STACK (involved in a conflict) by adding a new node. 
// and updates CONFLICT_STACK ending with NONE, together with node_conflict_set[ADDED_NODES]
//
// Used when all nodes of a non-unit color set are found conflicting

	 int riset;
	 node_conflict_set[ADDED_NODES]= &color_conflict_stack.stack[color_conflict_stack.pt];
	 bb_node_state_active.set_bit(ADDED_NODES);
	 for(int i=0; i<color_involved_stack.pt; i++){
		 riset=color_involved_stack.get_elem(i);

		 if(iset_involved[riset]==FALSE_VAL){		//color not previously added
			 iset_involved[riset]=TRUE_VAL;			//set as used
			 
			 //add node
			 if( m_colSets[riset].bb.is_bit(ADDED_NODES)){
				 m_colSets[riset].bb.print(); cout<<" adding node:  "<<ADDED_NODES<<endl;
				 LOG_ERROR("enlarged_involved_set():error in conflict set");
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
		 iset_involved[color_involved_stack.get_elem(i)]=FALSE_VAL;
		 iset_used_in_test[color_involved_stack.get_elem(i)]=FALSE_VAL;
	 }
	 color_involved_stack.erase();		//pt=0;
	 return 0;
 }


template <class graph_t, class bitboard_t>
inline
void InfraOp<graph_t,bitboard_t>::assign_node_value(int v, int val, int color_reason){
	node_value[v]=val;
	bb_node_state_active.erase_bit(v);
	node_stack.push(v);
	node_reason[v]=color_reason;
}

template <class graph_t, class bitboard_t>
inline
void InfraOp<graph_t,bitboard_t>::reset_context_for_maxsatz( int saved_node_stack_pt, 
									int saved_color_passive_stack_pt, 
									int save_color_reduced_stack_pt,
									int save_color_unit_stack_pt		){
///////////////////
// PARAMS:	<xxxx> reference in stacks to restore. Also STACKS will be deleted up to that point
//
// FUNCTION: restores state from STACKS and then cleans STACKS
// 
// REMARKS: 
// 1. color_enlarged_stack is not restored!
// 2. Empties color_unit_dyn_stack									

  //node state and NODE_STACK
  int node;									
  for(int i=saved_node_stack_pt; i<node_stack.pt; i++) {
	  node=node_stack.get_elem(i);
	  bb_node_state_active.set_bit(node);
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
void InfraOp<graph_t,bitboard_t>::reset_context_for_maxsatz_node( int saved_node_stack_pt, 
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
	  bb_node_state_active.set_bit(node);
	  node_reason[node]=NO_REASON;
	  if (node_value[node]==TRUE_VAL && node_tested_state[node]==FALSE_VAL) {
		  node_tested_state[node]=TRUE_VAL;			// no need to re-test at this point
		  tested_node_stack.push(node);
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
void InfraOp<graph_t,bitboard_t>::reset_enlarged_isets(){
///////////////////
// removes nodes added to enlarged sets, sets them to ACTIVE
// and clears ENLARGED_STACK

	int iset;
	for(int i=0; i<color_enlarged_stack.pt; i++) {
		iset=color_enlarged_stack.get_elem(i);	
		color_state_active.set_bit(iset);							//not needed since it is reset for each new subgraph	(careful!)
		m_colSets[iset].size--;										//not needed since it is reset for each new subgraph	(careful!)	
		m_colSets[iset].bb.get_bitboard(NB_OF_BB_NODES)=0;			//deletes all enlarged vertices (forced)
	
	}
	color_enlarged_stack.pt=0;
}

template <class graph_t, class bitboard_t>
inline
void InfraOp<graph_t,bitboard_t>::set_node_state_active(const bitboard_t& bb){
//////////////////////
// copies subgraph bb to node_state_active (note sizes are different)
	for(int nBB=0; nBB<NB_OF_BB_NODES; nBB++){
		bb_node_state_active.get_bitboard(nBB)=bb.get_bitboard(nBB);
	}

	//cleans all added nodes
	bb_node_state_active.get_bitboard(NB_OF_BB_NODES)=0;		//assuming NB_OF_BB_ADDED_NODES=NB_OF_BB_NODES+1
}

template <class graph_t, class bitboard_t>
inline
void InfraOp<graph_t,bitboard_t>::store_involved_sets(){
///////////////
// REASON_STACK to INVOLVED_SET_STACK and iset_used[]<--TRUE

  int iset;
  for(int i=0; i<color_reason_stack.pt; i++){
	iset=color_reason_stack.get_elem(i);
	color_involved_stack.push(iset);
	iset_used_in_test[iset]=TRUE_VAL;
  }
}

template <class graph_t, class bitboard_t>
inline
int InfraOp<graph_t,bitboard_t>::test_node(int node, int iset, bool is_last_node){
///////////////////////////
// PARAMS: node from iset to set to TRUE and make UL inferences
//		   further: control further tests if the node is the last (and only) non-conflicting node in iset
//	sets v in iset to FALSE hoping to get lucky and find conflict
//
// RETURNS: iset conflicting (size=0) or NO_CONFLICT
// REMARKS: note this is a partial conflict of vertex v in non-singleton iset

	int saved_node_stack_pt, saved_color_passive_stack_pt, save_color_reduced_stack_pt,
		save_color_unit_stack_pt;
	int my_iset;

	save_color_unit_stack_pt=color_unit_stack.pt;				//forced!
	//saved_node_stack_pt=node_stack.pt;							//=0-stack is empty
   // saved_color_passive_stack_pt=color_passive_stack.pt;		//=0-stack is empty
	//save_color_reduced_stack_pt=color_reduced_stack.pt;			//=0-stack is empty
	color_unit_dyn_stack.pt=0;

	if( (my_iset=fix_node_for_iset(node, iset)) != NO_CONFLICT || 
		(my_iset=unit_iset_process_for_tests()) !=  NO_CONFLICT 
#ifdef FURTHER_TEST_FAILED_COLOR_SETS
		|| 	(is_last_node && (my_iset=further_test(0)) != NO_CONFLICT)
#endif		
		){
			lookback_for_maxsatz(my_iset);
			/*reset_context_for_maxsatz(saved_node_stack_pt,
			saved_color_passive_stack_pt,
			save_color_reduced_stack_pt,
			save_color_unit_stack_pt);*/
			reset_context_for_maxsatz(0, 0,  0,	 save_color_unit_stack_pt);
			return my_iset;

	}else{ //NO CONFLICT
		//resets context and memorizes TRUE nodes to stop re-testing
		/*reset_context_for_maxsatz_node(saved_node_stack_pt,
				 saved_color_passive_stack_pt,
				 save_color_reduced_stack_pt,
				 save_color_unit_stack_pt);*/

		reset_context_for_maxsatz_node(0, 0, 0, save_color_unit_stack_pt);
	}

  	return NO_CONFLICT;
}

template <class graph_t, class bitboard_t>
inline
int InfraOp<graph_t,bitboard_t>::inc_test_node(int node, int iset, bool is_last_node){
///////////////////////////
// Simplified test_node function
// 1.uses simple unit_iset_process() instead of  unit_iset_process_for_tests()
//
// REMARKS: check reset_contexts

	int saved_node_stack_pt, saved_color_passive_stack_pt, save_color_reduced_stack_pt,
		save_color_unit_stack_pt;
	int my_iset;

	save_color_unit_stack_pt=color_unit_stack.pt;				//forced!
	//saved_node_stack_pt=node_stack.pt;						//=0-stack is empty
   // saved_color_passive_stack_pt=color_passive_stack.pt;		//=0-stack is empty
	//save_color_reduced_stack_pt=color_reduced_stack.pt;		//=0-stack is empty
	color_unit_dyn_stack.pt=0;

	if( (my_iset=fix_node_for_iset(node, iset)) != NO_CONFLICT || 
		(my_iset=unit_iset_process()) !=  NO_CONFLICT 
#ifdef FURTHER_TEST_FAILED_COLOR_SETS
		|| 	(is_last_node && (my_iset=further_test(0)) != NO_CONFLICT)
#endif		
		){
			lookback_for_maxsatz(my_iset);
			reset_context_for_maxsatz(0, 0,  0,	 save_color_unit_stack_pt);
			return my_iset;

	}else{ //NO CONFLICT
		reset_context_for_maxsatz(0, 0, 0, save_color_unit_stack_pt);
	}

  	return NO_CONFLICT;
	
  //saved_unitiset_stack_fill_pointer=UNITISET_STACK_fill_pointer;
  //saved_node_stack_fill_pointer=NODE_STACK_fill_pointer;
  //saved_passive_iset_stack_fill_pointer=PASSIVE_ISET_STACK_fill_pointer;
  //saved_reduced_iset_stack_fill_pointer=REDUCED_ISET_STACK_fill_pointer;
  //MY_UNITISET_STACK_fill_pointer=0;
  ////  if (ISETS_SIZE[iset]>1 && (my_iset=unitIsetProcess())!=NO_CONFLICT)
  ////  printf("bizzar.....\n");
  //if ((my_iset=fix_node_for_iset(node, iset)) != NO_CONFLICT ||
  //    (my_iset=my_unitIsetProcess()) !=  NO_CONFLICT ||
  //    (further &&
  //     (my_iset=further_test_node(saved_reduced_iset_stack_fill_pointer)) 
  //     != NO_CONFLICT)) {
  //  lookback_for_maxsatz(my_iset);
  //  reset_context_for_maxsatz(saved_node_stack_fill_pointer,
		//	      saved_passive_iset_stack_fill_pointer,
		//	      saved_reduced_iset_stack_fill_pointer,
		//	      saved_unitiset_stack_fill_pointer);
  //  return my_iset;
  //}
  //else {
  //  reset_context_for_maxsatz(saved_node_stack_fill_pointer,
		//		 saved_passive_iset_stack_fill_pointer,
		//		 saved_reduced_iset_stack_fill_pointer,
		//		 saved_unitiset_stack_fill_pointer);
  //  return NO_CONFLICT;
  //}



}


template <class graph_t, class bitboard_t>
inline
int InfraOp<graph_t,bitboard_t>::unit_iset_process_for_tests(){
///////////////////////////
// specific UL inference for testing nodes:
// First unit USED color sets are examined. Then, for each unit UNUSED color set, new USED nodes are always checked
// The process ends when all USED color sets are processed//
//
// REMARKS
// called after fix_node_for_iset(v, iset) has been called, so all related nodes are 
// in color_unit_dyn_stack (not color_unit_stack )
	
	int j, iset, my_iset, iset_start=0, used_iset_start=0;
	do{
		//tests active nodes USED in previous CONFLICTS of related tested nodes
		for(j=used_iset_start; j<color_unit_dyn_stack.pt; j++) {
			iset=color_unit_dyn_stack.get_elem(j);
			if(color_state_active.is_bit(iset) && iset_used_in_test[iset]==TRUE_VAL){
				if ((my_iset=fix_unit_color(iset))!=NO_CONFLICT)				//iset is turned passive (*)
					return my_iset;
			}
		}
		used_iset_start=j;

		//tests remaining nodes (not necessarily USED)
		for(j=iset_start; j<color_unit_dyn_stack.pt; j++) {
			iset=color_unit_dyn_stack.get_elem(j);
			if(color_state_active.is_bit(iset)){								//used isets cannot be selected here(*)
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
int InfraOp<graph_t,bitboard_t>::maxsatz_lookahead(int nb_conflict, int max_num_conf){
////////////////////////////
// Driver for testing color sets of small size
//
// PARAMS: nb_conflicts-current number of conflicts, max_num_conf=UB-LB
//
// RETURNS the total number of conflicts: nb_conflict + new conflicts found
//
	//int iset,  *nodes, node, no_conflict, test_flag, k, saved_size, ctr, i;
	bool test_flag, no_conflict_flag;

	//reset node_tested_state and TESTED_NODE_STACK (only place that this is done)
	for(int i=0; i<tested_node_stack.pt; i++){ 
		node_tested_state[tested_node_stack.get_elem(i)]=FALSE_VAL;
	}
	tested_node_stack.pt=0;
	int nodes_chosen_set[MAX_COLOR_TEST_LENGTH];

	//main loop: small color sizes
	int diff=max_num_conf-nb_conflict;					//must be >0
	for(int k=2; k<=MAX_COLOR_TEST_LENGTH; k++){
		for(int iset=NB_OF_COLORS; iset>=diff; iset--){	
			if( color_state_active.is_bit(iset) && m_colSets[iset].size==k					//***can colors be passive here?
				&& !m_colSets[iset].bb.get_bitboard(NB_OF_BB_NODES) /* not WEAKENED*/ )	{		
				//check if all nodes in iset are UNTESTED (i.e. have not let to a CONFLICT when tested to TRUE)
				bitboard_t& bbcol=m_colSets[iset].bb;
				bbcol.init_scan(bbo::NON_DESTRUCTIVE);
				test_flag=true;
				for(int i=0; i<k;i++){
					nodes_chosen_set[i]=bbcol.next_bit();
					if(node_tested_state[nodes_chosen_set[i]]==TRUE_VAL){
						test_flag=false;
						break;
					}
				}
				
				if(test_flag){ //valid color for tests
					color_involved_stack.erase();

					//loop through all nodes and test them
					no_conflict_flag=false;							//assumes a conflict will be found
					for(int i=0; i<k; i++){
						if(test_node(nodes_chosen_set[i], iset, (i==k-1) /* is_last*/)==NO_CONFLICT){		//no need to check if already tested again
							no_conflict_flag=true;					//no conflict
							break;
						}else{				//conflict for this particular node
							store_involved_sets();		
						}
					}

					//conflict found for all nodes: the REAL CONFLICT
					if(no_conflict_flag==false){
						enlarge_stored_involved_sets();
						nb_conflict++;
						//LOG_INFO("TEST CONFLICF FOUND");
						if(nb_conflict>=max_num_conf)
								return nb_conflict;			//CUT FOUND
					}

					//context operations for next color tests
					for(int i=0; i<color_involved_stack.pt; i++)
						iset_used_in_test[color_involved_stack.get_elem(i)]=FALSE_VAL;
					color_involved_stack.pt=0;
				}
			}//end of color test
		}
	}
	
	return nb_conflict;
}

template <class graph_t, class bitboard_t>
inline
int InfraOp<graph_t,bitboard_t>::inc_maxsatz_lookahead(int saved_color_unit_stack_pt){
//////////
// RETURNS EMPTY_ELEM if one conflict is found, TRUE_VAL otherwise

	bool test_flag, no_conflict_flag, one_conflict;
	int nodes_chosen_set[MAX_COLOR_TEST_LENGTH];

	for(int k=2; k<=MAX_COLOR_TEST_LENGTH; k++){
		for(int iset=NB_OF_COLORS; iset>=1; iset--){	
			if( color_state_active.is_bit(iset) && m_colSets[iset].size==k 
				&& !m_colSets[iset].bb.get_bitboard(NB_OF_BB_NODES) /* not weakened*/){
				
				//checks that all nodes in iset are active (note they cannot be weak)
				bitboard_t& bbcol=m_colSets[iset].bb;
				bbcol.init_scan(bbo::NON_DESTRUCTIVE);
				test_flag=true;
				for(int i=0; i<k;i++){
					nodes_chosen_set[i]=bbcol.next_bit();
					if(!bb_node_state_active.is_bit(nodes_chosen_set[i])){
						test_flag=false;
						break;
					}
				}

				//valid color to test
				if (test_flag==true) {
					color_involved_stack.erase();
					no_conflict_flag=false;
					for(int i=0; i<k; i++){
						if(inc_test_node(nodes_chosen_set[i], iset, (i==k-1) /* is last node*/)==NO_CONFLICT){		//no need to check if already tested again
							no_conflict_flag=true;					//no conflict
							break;
						}else{				//conflict for this particular node
							store_involved_sets();		
						}
					}

					//Failed color set, all nodes FAILED
					if(no_conflict_flag==false){
						reset_context_for_maxsatz(0, 0, 0,saved_color_unit_stack_pt);
						//**CHECKS: see if iset is enlarged by only one added node
						//saved_size=ISETS_SIZE[iset];
						enlarge_stored_involved_sets(); 
						/*if (saved_size+1 != ISETS_SIZE[iset]){
								printf("erreur iset involved %d %d %d...", 
								saved_size,  ISETS_SIZE[iset], iset);
						}*/
						one_conflict=true;
					}

					//reset context
					for(int i=0; i<color_involved_stack.pt; i++){
						iset_used_in_test[color_involved_stack.get_elem(i)]=FALSE_VAL;
					}
					color_involved_stack.pt=0;
					if (one_conflict==true) 
						return EMPTY_ELEM;
				}
			}
		}
	}

	return TRUE_VAL;
	
	// int iset, *nodes, node, no_conflict, test_flag, k, saved_size, ctr, 
 //   i, one_conflict=FALSE, result;
	//for(k=2; k<=FL_TEST_LENGTH; k++) 
	//	for(iset=ISET_NB-1; iset>=0; iset--) {
	//		if (iset_state[iset]==ACTIVE && ISETS_SIZE[iset]==k) {
	//			nodes=ISETS[iset]; test_flag=TRUE;
	//			for(node=*nodes; node!=NONE; node=*(++nodes)) {
	//				if (node>NB_NODE && node_state[node]==ACTIVE) {
	//					test_flag=FALSE; break;
	//				}
	//			}
	//			if (test_flag==TRUE) {
	//				nodes=ISETS[iset]; no_conflict=FALSE; ctr=k+1;
	//				INVOLVED_ISET_STACK_fill_pointer=0;
	//				for(node=*nodes; node!=NONE; node=*(++nodes))
	//					if (node_state[node]==ACTIVE) {
	//						if (inc_test_node(node, iset, (*(nodes+1)==NONE))==NO_CONFLICT) {
	//							no_conflict=TRUE; break;
	//						}
	//						else store_involved_isets();
	//					}
	//					if (no_conflict==FALSE) { 
	//						reset_context_for_maxsatz(0, 0, 0,
	//							saved_unitiset_stack_fill_pointer);
	//						saved_size=ISETS_SIZE[iset];
	//						enlarge_stored_involved_isets();
	//						if (saved_size+1 != ISETS_SIZE[iset])
	//							printf("erreur iset involved %d %d %d...", 
	//							saved_size,  ISETS_SIZE[iset], iset);
	//						one_conflict=TRUE;
	//					}
	//					for(i=0; i<INVOLVED_ISET_STACK_fill_pointer; i++)
	//						iset_used[INVOLVED_ISET_STACK[i]]=FALSE;
	//					INVOLVED_ISET_STACK_fill_pointer=0;
	//					if (one_conflict==TRUE) 
	//						return NONE;
	//			}
	//		}
	//	}
	//	return TRUE;

}

template <class graph_t, class bitboard_t>
inline
int InfraOp<graph_t,bitboard_t>::further_test(int start){
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

	int saved_node_stack_pt, saved_color_passive_stack_pt, save_color_reduced_stack_pt,
		save_color_unit_stack_pt;
	int my_iset, chosen_iset;
	int nodes_chosen_set[MAX_COLOR_FURTHER_TEST_LENGTH];

	//init: 1.stores current end-pointers of STACKS 
	//      2.sets candidate colors in REDUCED_STACK to UNUSED
	save_color_unit_stack_pt=color_unit_stack.pt;				
	saved_node_stack_pt=node_stack.pt;						
	saved_color_passive_stack_pt=color_passive_stack.pt;		
	save_color_reduced_stack_pt=color_reduced_stack.pt;	
	bb_colors_used_in_extended_test.erase_bit();			//avoids repetition in REDUCED_STACK
	/*for(int i=start; i<save_color_reduced_stack_pt; i++){
		bb_colors_used_in_extended_test.is_bit(color_reduced_stack.get_elem(i)]=FALSE_VAL;
	}*/
	
	//main loop
	for(int i=start; i<save_color_reduced_stack_pt; i++) {		//note REDUCED_STACK will be enlarged
		chosen_iset=color_reduced_stack.get_elem(i);
		if(!bb_colors_used_in_extended_test.is_bit(chosen_iset) && color_state_active.is_bit(chosen_iset) 
			&& m_colSets[chosen_iset].size==MAX_COLOR_FURTHER_TEST_LENGTH 
			&& m_colSets[chosen_iset].bb.get_bitboard(NB_OF_BB_NODES)==0	/* not weakened*/){
			
			bb_colors_used_in_extended_test.set_bit(chosen_iset);

			//get nodes from chosen set and check they are all ACTIVE
			bitboard_t& bbchset=m_colSets[chosen_iset].bb;
			bbchset.init_scan(bbo::NON_DESTRUCTIVE);
			nodes_chosen_set[0]=bbchset.next_bit();
			if(!bb_node_state_active.is_bit(nodes_chosen_set[0]) )
				continue; //next candidate color
			nodes_chosen_set[1]=bbchset.next_bit();
			if(!bb_node_state_active.is_bit(nodes_chosen_set[1]) )
				continue; //next candidate color

			//test (both) nodes
			for(int j=0; j<MAX_COLOR_FURTHER_TEST_LENGTH; j++){
				int node=nodes_chosen_set[j];
				color_unit_dyn_stack.erase();
				if ( (my_iset=fix_node_for_iset(node, chosen_iset)) !=NO_CONFLICT ||
					 (my_iset=unit_iset_process_for_tests()) != NO_CONFLICT  ) {
						//CONFLICT FOUND for node: store involved sets
						iset_involved[chosen_iset]=TRUE_VAL;		//includes reduced candidate set in inference
						lookback_for_maxsatz(my_iset);
						iset_involved[chosen_iset]=FALSE_VAL;
						reset_context_for_maxsatz(saved_node_stack_pt,
												saved_color_passive_stack_pt,
												save_color_reduced_stack_pt,
												save_color_unit_stack_pt );
						store_involved_sets();

				}else{
					//NO CONFLICT for node
					reset_context_for_maxsatz(saved_node_stack_pt,
											saved_color_passive_stack_pt,
											save_color_reduced_stack_pt,
											save_color_unit_stack_pt );
					break;
				}

				//CONFLICT FOUND for chosen_iset from REDUCED STACK
				if(j== MAX_COLOR_FURTHER_TEST_LENGTH-1 /*1*/){
					//LOG_INFO("FURTHER TEST SUCCESFUL");
					return chosen_iset;
				}
			}
		}
	}

  return  NO_CONFLICT;
}

template <class graph_t, class bitboard_t>
inline
int InfraOp<graph_t,bitboard_t>::simple_further_test(int start){
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

	bb_colors_used_in_extended_test.erase_bit();			//avoids repetition in REDUCED_STACK

	//main loop
	for(int i=start; i<saved_color_reduced_stack_pt; i++) {		//note REDUCED_STACK will be enlarged
		chosen_iset=color_reduced_stack.get_elem(i);			//WEAKENED or NOT
		if(!bb_colors_used_in_extended_test.is_bit(chosen_iset) && color_state_active.is_bit(chosen_iset) 
			&& m_colSets[chosen_iset].size==MAX_COLOR_FURTHER_TEST_LENGTH ){
				bb_colors_used_in_extended_test.set_bit(chosen_iset);		//to avoid repetitions
				bitboard_t& bbcol=m_colSets[chosen_iset].bb;
				bbcol.init_scan(bbo::NON_DESTRUCTIVE);
				while(true){
					int node=bbcol.next_bit();
					if (node==EMPTY_ELEM) break;
					if(node<NB_OF_NODES /* not weakened*/ && bb_node_state_active.is_bit(node)){
						color_unit_dyn_stack.erase();
						my_iset=fix_node_for_iset(node, chosen_iset);
						if (my_iset == NO_CONFLICT){
							my_iset=unit_iset_process_for_tests();
						}
						reset_context_for_maxsatz(my_saved_node_stack_pt,
							my_saved_color_passive_stack_pt,
							my_saved_color_reduced_stack_pt,
							my_saved_color_unit_stack_pt );
						//CONFLICT FOUND
						if(my_iset!=NO_CONFLICT){		
							assign_node_value(node, FALSE_VAL, NO_REASON);
							m_colSets[chosen_iset].size--;
							color_reduced_stack.push(chosen_iset);
							if(m_colSets[chosen_iset].size==1){
								color_unit_dyn_stack.erase();
								color_unit_dyn_stack.push(chosen_iset);
								if( (my_iset=unit_iset_process_for_tests())!=NO_CONFLICT){
									conflict_found=true;
									break;
								}

								//restore context and tested colors for next node
								for(int j=my_saved_color_reduced_stack_pt; j<color_reduced_stack.pt; j++){
									bb_colors_used_in_extended_test.erase_bit(color_reduced_stack.get_elem(j));
								}	

								my_saved_color_unit_stack_pt=color_unit_stack.pt;
								my_saved_node_stack_pt=node_stack.pt;
								my_saved_color_passive_stack_pt=color_passive_stack.pt;	
								my_saved_color_reduced_stack_pt=color_reduced_stack.pt;
							}
						}
					}
				}//next node
				if (conflict_found==true)
					break;
		}
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

template <class graph_t, class bitboard_t>
inline
int InfraOp<graph_t,bitboard_t>::test_node_for_failed_nodes(int node, int iset){
/////////////////////////
// Attempts to prove v FAILED
// RETURNS: Conflicting (empty) iset or NO_CONFLICT
//
// APPLICATION: called by test which tries to show conflicts by eliminating FAILED nodes
//
	int my_iset;
				
	/*if(!node_stack.empty() || !color_reduced_stack.empty() || !color_passive_stack.empty()){
		LOG_ERROR("STACKS NOT EMPTY");
	}*/

	//saved pointers necessary to store FAILED NODES in previous inferences
	//note that the STACKS are NOT empty here, since they contain possible 
	//reduced colors of setting a node to TRUE
	int save_color_unit_stack_pt=color_unit_stack.pt;	
	int saved_node_stack_pt=node_stack.pt;							
	int saved_color_passive_stack_pt=color_passive_stack.pt;		
	int save_color_reduced_stack_pt=color_reduced_stack.pt;			
	color_unit_dyn_stack.erase();
	if ((my_iset=fix_node_for_iset(node, iset)) == NO_CONFLICT) {
#ifdef FURTHER_TEST_ELIMINATE_FAILED_NODES
		if ((my_iset=unit_iset_process_for_tests()) == NO_CONFLICT)
			  my_iset=simple_further_test(save_color_reduced_stack_pt);		//analysis of REDUCED_STACK
#else
			my_iset=unit_iset_process_for_tests();
#endif
	}
	 reset_context_for_maxsatz(saved_node_stack_pt,
								saved_color_passive_stack_pt,
								save_color_reduced_stack_pt,
								save_color_unit_stack_pt);
	//reset_context_for_maxsatz(0,0,0,save_color_unit_stack_pt);
	return my_iset;

}

template <class graph_t, class bitboard_t>
inline
bool InfraOp<graph_t,bitboard_t>::test_by_eliminate_failed_nodes(){
///////////
// Driver to find conflicts by proving all FAILED nodes in a color set
//
// APPLICATION: used to reduce by ONE the number of conflicts
// RETURNS: TRUE if a conflict has been found or FALSE otherwise

	int my_iset, nb_partial_conf;
	bool conflict=false;
	int save_color_unit_stack_pt=color_unit_stack.pt;			//forced!
	//saved_node_stack_pt=node_stack.pt;						//=0-stack is empty
	//saved_color_passive_stack_pt=color_passive_stack.pt;		//=0-stack is empty
	//save_color_reduced_stack_pt=color_reduced_stack.pt;		//=0-stack is empty

	do{
		nb_partial_conf=0;
		for(int iset=NB_OF_COLORS-1; iset>=1; iset--){
			if(color_state_active.is_bit(iset)){		//WEAKENED or NOT
				conflict=false;
				color_unit_dyn_stack.erase();
				bitboard_t& bb_col=m_colSets[iset].bb;
				bb_col.init_scan(bbo::NON_DESTRUCTIVE);
				while(true){						
					int node=bb_col.next_bit();
					if(node==EMPTY_ELEM) break;
					
					//test node if ACTIVE and NOT ADDED
					if(node<NB_OF_NODES && bb_node_state_active.is_bit(node) 
						&& test_node_for_failed_nodes(node, iset)!=NO_CONFLICT ){
						nb_partial_conf++;												//FAILED node found
						color_unit_dyn_stack.erase();
						assign_node_value(node, FALSE_VAL, NO_REASON /* no lookback*/);
						m_colSets[iset].size--;
						color_reduced_stack.push(iset);
						if(m_colSets[iset].size==1){
							color_unit_dyn_stack.push(iset);
							break;		//to launch a UL inference
						}else if(m_colSets[iset].size==0){
							conflict=true;
							break;
						}
					}
				} //next node of color set
				if(conflict) break;
				else if(color_unit_dyn_stack.pt>0 && 
						unit_iset_process_for_tests() !=NO_CONFLICT){
					//CONFLICT
					conflict=true;
					break;
				}
			}

			if(conflict==true) 
					break;

		}//next color to test
	}while(nb_partial_conf>1 /*not only one, check*/ && conflict==false);
	/* reset_context_for_maxsatz(saved_node_stack_pt,
								saved_color_passive_stack_pt,
								save_color_reduced_stack_pt,
								save_color_unit_stack_pt);*/

	reset_context_for_maxsatz(0,0,0,save_color_unit_stack_pt);
	return(conflict);

}




#endif




