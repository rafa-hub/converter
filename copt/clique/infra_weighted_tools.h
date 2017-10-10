//infra_weighted_tools.h: Header for InfraOpW which extends infra tools in InfraOp for the weighted case
//date of creation: 27/10/16

#ifndef __INFRA_WEIGHTED_TOOLS_H__
#define	__INFRA_WEIGHTED_TOOLS_H__

#include "clique_types.h"
#include "../init_color_ub.h"
#include "bitscan/bbalg.h"						
#include "utils/common.h"
#include "infra_tools.h"


template<typename graph_t, typename bitboard_t>
class InfraOpW: public InfraOp<graph_t, bitboard_t>{

	typedef InfraOp<graph_t, bitboard_t> Parent;					//to access the parents
	static const int UPDATED =0x1;
	static const int NOT_UPDATED =0x0;
///////////////
//data members
public:
	int * m_cw;														//[colors] weights of clauses (may vary:not allocated here)
	int * m_lw;														//[nodes/literals] weights of literals (may vary) (allocated)

	com::stack_t<int> color_simplified_stack;						//contains simplified clauses (literals removed)
	com::stack_t<int> node_simplified_stack;						//contains simplified literals (weights changed)
	com::stack_t<int> node_removed_stack;
	int* node_weight;												//[nodes] weight for context restore
	int* clause_weight;												//*** CHANGE TO STACK [colors] weight for context restore
	int* clause_weight_state;										//[UPDATED, NON UPDATED]-should only be updated the first time (per inferecence)
	int* node_weight_state;											//[UPDATED, NON UPDATED]-should only be updated once per inference (per inferecence)
////////////////////////	
//interface
public:
	InfraOpW(graph_t* g=NULL): InfraOp<graph_t, bitboard_t>(g), m_cw(NULL), m_lw(NULL), node_weight(NULL), clause_weight(NULL),
								node_weight_state(NULL), clause_weight_state(NULL){}
	virtual ~InfraOpW(){clear();}
								
	
virtual	int init(int MAX_COL);										
virtual	void clear();
	void set_clause_weights(int* p_cw) {m_cw=p_cw;}
	void clear_clause_weights(){m_cw=NULL; if(m_lw) delete [] m_lw; m_lw=NULL;}

public:
	int init_weighted_maxsatz(int v, int maxUB, int clqw_size);		//adds v, the expanded node 

protected:
virtual	int maxsatz(int max_num_conf);
	int weight_of_inconsistent_sets(com::stack_t<int>& stack);
	int simplify_clause(int iset, int weight_of_inconsistent_set);
	int simplify_clause_with_context(int iset, int weight_of_inconsistent_set);
	int update_clause_weights();

public:
	int simplify_clause_during_recoloring(int iset, int weight_of_inconsistent_set);
	void swap_clause_during_recoloring(int iset, int v);
	
protected:
	//extension weighted case
virtual	int enlarge_involved_set();									//returns weight of inconsistent subset
		int enlarge_involved_set_with_context();	

	//extension of lookahead procedures for weighted graphs
virtual	int maxsatz_lookahead(int nb_conflict, int max_num_conf);
virtual	int enlarge_stored_involved_sets();							//returns the weight of the cut, compared with UW
	
	 int top_literal_info(int iset, int& red);						//returns node
	 
	//context: weights of literals
public:
	void reset_context_for_literals();
	void reset_inital_weights_of_literals();
	void reset_context_for_simplifications();

	//I/O
public:
virtual	void print_db(bool active_sets=false, bool active_nodes=false);
		void print_clause(int iset);

	//tests/corrections
virtual bool check_consistency_db();
	    bool check_consistency_clause(int iset,  bool only_active=true);
		bool check_literal_weight_consistency();	
		int make_consistent_db();
};

template<class graph_t, class bitboard_t>
inline
void InfraOpW<graph_t, bitboard_t>::clear(){

	InfraOp<graph_t, bitboard_t>::clear();
	clear_clause_weights();	
	
	if(m_lw)
		delete [] m_lw;
	m_lw=NULL;

	if(node_weight)
		delete [] node_weight;
	node_weight=NULL;

	
	if(node_weight_state)
		delete [] node_weight_state;
	node_weight=NULL;

	if(clause_weight)
		delete [] clause_weight;
	clause_weight=NULL;

	if(clause_weight_state)
		delete [] clause_weight_state;
	clause_weight=NULL;

	
 }

template<class graph_t, class bitboard_t>
inline
int InfraOpW<graph_t, bitboard_t>::init(int MAX_NB_COL){
//////////////
// allocates memory for data structures: MAX_NB_COL+1 color sets
//
// RETURNS -1 if ERROR

	clear_clause_weights();
	int res=Parent::init(MAX_NB_COL);
	if(res!=NONE){
		//copies weights of literals locally
		m_lw= new int[Parent::NB_OF_NODES];
		for(int i=0; i<Parent::NB_OF_NODES; i++){
			m_lw[i]=Parent::g->get_wv(i);
		}

		color_simplified_stack.init(MAX_NB_COL);						//perhaps not necessary
		node_simplified_stack.init(Parent::NB_OF_ADDED_NODES);			//necessary to selectively restore context of weights
		node_removed_stack.init(Parent::NB_OF_ADDED_NODES);

		node_weight= new int[Parent::NB_OF_NODES];
		for(int i=0; i<Parent::NB_OF_NODES; i++){
			node_weight[i]=0;
		}

		node_weight_state= new int[Parent::NB_OF_NODES];
		for(int i=0; i<Parent::NB_OF_NODES; i++){
			node_weight_state[i]=NOT_UPDATED;
		}

		clause_weight= new int[Parent::NB_OF_NODES];
		for(int i=0; i<Parent::NB_OF_NODES; i++){
			clause_weight[i]=0;
		}

		clause_weight_state= new int[Parent::NB_OF_NODES];
		for(int i=0; i<Parent::NB_OF_NODES; i++){
			clause_weight_state[i]=NOT_UPDATED;
		}


	}else return -1;
	
	return 0;
}


template <class graph_t, class bitboard_t>
inline
int InfraOpW<graph_t,bitboard_t>::init_weighted_maxsatz(int v, int maxUB, int clqw_size){
///////////////
// Extension to weighted case
//
// PARAMS: clqw_size is the KMIN diff in weights!
//
// REMARKS:
// 1. assumes ACTIVE VERTICES ARE SET!

	int nb_wconflicts=0;
	node_simplified_stack.erase();	
	
	//*** REMOVE FOR FAST PERFORMANCE
	/*if(!is_init_literal_weights_ok()){
		LOG_ERROR("init_weighted_maxsatz::bizarre weights of literals on resting context");
	}*/

	//add new singleton color DB containing v
	Parent::m_colSets[++Parent::NB_OF_COLORS].erase_bit();	
	Parent::m_colSets[Parent::NB_OF_COLORS].push(v);
	Parent::bb_node_state_active.set_bit(v);
	Parent::node_iset_no[v]=Parent::NB_OF_COLORS;
	Parent::node_reason[v]=Parent::NO_REASON;
	m_cw[Parent::NB_OF_COLORS]=m_lw[v];			//add the weight of v to clauses
	
	//***active vertices (assumed set)

	//check WEIGHTS
	//int colw=0;
	//for(int i=1; i<=NB_OF_COLORS; i++){
	//	colw+=m_cw[i];
	//	//cout<<i<<":"<<m_cw[i]<<" ";
	//}
	//if(maxUB!=colw){
	//	LOG_ERROR("BIZARRE COLOR SUM OF WEIGHTS");
	//}

	int nb_max_conf=maxUB-clqw_size;
	nb_wconflicts=maxsatz(nb_max_conf);
				
	reset_inital_weights_of_literals();					//**selectively?

	//*** REMOVE FOR FAST PERFORMANCE
	/*if(!is_init_literal_weights_ok()){
		LOG_ERROR("init_weighted_maxsatz::bizarre weights of literals on resting context");
	}*/
	
	//remove singleton color DB
	Parent::NB_OF_COLORS--;

return nb_wconflicts;
}

template <class graph_t, class bitboard_t>
inline
int InfraOpW<graph_t,bitboard_t>::weight_of_inconsistent_sets(com::stack_t<int>& stack){
//computes the reduction in weight for the current conflicting clauses that are non-relaxed
//
	int mincw=CLQ_MAXINT;
	for(int i=0; i<stack.pt; i++){
		int iset=stack.get_elem(i);
		if(mincw>m_cw[iset] /*&& m_colSets[iset].bb.get_bitboard(NB_OF_BB_NODES)==0 */)
				mincw=m_cw[iset];
	}
	//ASSERT
	if(mincw==CLQ_MAXINT){
		LOG_ERROR("bizarre weight of inconsistent set");
		return 0;
	}
	return mincw;
}
template <class graph_t, class bitboard_t>
inline
int InfraOpW<graph_t,bitboard_t>::simplify_clause(int iset, int wset){
/////////////
// simplifies an inconsistent clause according to the reduction in weight of the inconsistent set it belongs
// (only for non-relaxed clauses)
//
// It is assumed that literals in clauses are not ordered by weights
//
// RETURNS 0 if simplified (change in weights of literals / removal of literals), NONE if not simplified

	bool flag_simplified=false;

	//if not relaxed exit or the weight of clause is below or equal to wc then
	if(m_cw[iset]<=wset) return false;
		
	//main loop: clause should be able to be simplified
	Parent::m_colSets[iset].bb.init_scan(bbo::NON_DESTRUCTIVE);
	while(true){
		int v=Parent::m_colSets[iset].bb.next_bit();
		if(v==EMPTY_ELEM  ) break;
		if(!Parent::bb_node_state_active.is_bit(v))  continue;
			
		//update weights of literals, change the weight of the clause and mark corr. literals as inactive
		if(m_lw[v]>wset){
			m_lw[v]-=wset;
			flag_simplified=true;						
		}else if(m_lw[v]<=wset){								//change in structure: remove the literal
			Parent::bb_node_state_active.erase_bit(v);
			
			//node_removed_stack.push(v);
			Parent::m_colSets[iset].size--;
			if(Parent::m_colSets[iset].size==0){
				LOG_ERROR("simplify_clause()::clause simplified to elimination-bizarre clause weight:"<<iset);
				
				/*m_colSets[iset].bb.print(); cout<<" c-weight: "<<m_cw[iset]<<" curr.v:"<<v; cout<<endl;
				m_colSets[iset].bb.init_scan(bbo::NON_DESTRUCTIVE);
				while(true){
					int w=m_colSets[iset].bb.next_bit();
					if(w==EMPTY_ELEM) break;
					
					cout<<" v: "<<w<<" w:"<<m_lw[w];
				}
				cout<<"-----------------------------------"<<endl;*/

				return false;
			}//else if(m_colSets[iset].size==1){
			//	//LOG_INFO("clause simplified to unit");
			//	color_unit_stack.push(iset);	
			//	flag_simplified=true;
			//	break;
			//}
			flag_simplified=true;
		}
	}

	//if unit clause, add to unit clause propagation
	if(Parent::m_colSets[iset].size==1){
		//LOG_INFO("clause simplified to unit");
		Parent::color_unit_stack.push(iset);		
	}

	//udate new weight of clause
	m_cw[iset]-=wset;

	//TEST
	if( !flag_simplified || m_cw[iset]<=0 ){
		LOG_ERROR("bizarre simplification of clause: "<<iset);
	}
	
	return flag_simplified;
}

template <class graph_t, class bitboard_t>
inline
int InfraOpW<graph_t,bitboard_t>::simplify_clause_with_context(int iset, int wset){
/////////////
// simplifies an inconsistent clause according to the reduction in weight of the inconsistent set it belongs
// (only for non-relaxed clauses)
//
// It is assumed that literals in clauses are not ordered by weights
//
// RETURNS 0 if simplified (change in weights of literals / removal of literals), NONE if not simplified

	bool flag_simplified=false;

	//if not relaxed exit or the weight of clause is below or equal to wc then
	if(m_cw[iset]<=wset) return false;
		
	//main loop: clause should be able to be simplified
	Parent::m_colSets[iset].bb.init_scan(bbo::NON_DESTRUCTIVE);
	while(true){
		int v=Parent::m_colSets[iset].bb.next_bit();
		if(v==EMPTY_ELEM  ) break;
		if(!Parent::bb_node_state_active.is_bit(v))  continue;
			
		//update weights of literals, change the weight of the clause and mark corr. literals as inactive
		if(m_lw[v]>wset){
			if(node_weight_state[v]==NOT_UPDATED){
				node_simplified_stack.push(v);
				node_weight_state[v]=UPDATED;
			}
			node_weight[v]=m_lw[v];
			m_lw[v]-=wset;		
			flag_simplified=true;						
		}else if(m_lw[v]<=wset){								//change in structure: remove the literal
			if(node_weight_state[v]==NOT_UPDATED){
				node_simplified_stack.push(v);
				node_weight_state[v]=UPDATED;
			}
			node_weight[v]=m_lw[v];
			Parent::bb_node_state_active.erase_bit(v);
			
			//node_removed_stack.push(v);
			Parent::m_colSets[iset].size--;
			if(Parent::m_colSets[iset].size==0){
				LOG_ERROR("simplify_clause()::clause simplified to elimination-bizarre clause weight:"<<iset);
				
				/*m_colSets[iset].bb.print(); cout<<" c-weight: "<<m_cw[iset]<<" curr.v:"<<v; cout<<endl;
				m_colSets[iset].bb.init_scan(bbo::NON_DESTRUCTIVE);
				while(true){
					int w=m_colSets[iset].bb.next_bit();
					if(w==EMPTY_ELEM) break;
					
					cout<<" v: "<<w<<" w:"<<m_lw[w];
				}
				cout<<"-----------------------------------"<<endl;*/

				return false;
			}//else if(m_colSets[iset].size==1){
			//	//LOG_INFO("clause simplified to unit");
			//	color_unit_stack.push(iset);	
			//	flag_simplified=true;
			//	break;
			//}
			flag_simplified=true;
		}
	}

	//if unit clause, add to unit clause propagation
	if(Parent::m_colSets[iset].size==1){
		//LOG_INFO("clause simplified to unit");
		Parent::color_unit_stack.push(iset);		
	}

	//udate new weight of clause
	if(clause_weight[iset]==NOT_UPDATED){
		clause_weight[iset]=m_cw[iset];
		clause_weight[iset]==UPDATED;
	}
	m_cw[iset]-=wset;


	//TEST
	if( !flag_simplified || m_cw[iset]<=0 ){
		LOG_ERROR("bizarre simplification of clause: "<<iset);
	}
	
	return flag_simplified;
}


template <class graph_t, class bitboard_t>
inline
int InfraOpW<graph_t,bitboard_t>::simplify_clause_during_recoloring(int iset, int wref){
/////////////
// simplifies a clause iset during simple infra-chrom
//
// RETURNS 0 if OK, -1 if clause reduced to elimination (which should not occurr)
//

	m_cw[iset]-=wref;
	Parent::m_colSets[iset].bb.init_scan(bbo::NON_DESTRUCTIVE);
	while(true){
		int v=Parent::m_colSets[iset].bb.next_bit();
		if(v==EMPTY_ELEM  ) break;
		if(!Parent::bb_node_state_active.is_bit(v)) continue;
					
		//update weights of literals, change the weight of the clause and mark corr. literals as inactive
		if(m_lw[v]>wref){
			m_lw[v]-=wref;
		}else if(m_lw[v]<=wref){								//change in structure: remove the literal
			Parent::bb_node_state_active.erase_bit(v);
			Parent::m_colSets[iset].size--;
			if(Parent::m_colSets[iset].size==0){
				LOG_ERROR("simplify_clause_during_recoloring()::clause simplified to elimination-bizarre: "<<iset);
				return -1;
			}
		}
	}

	return 0;
}

template <class graph_t, class bitboard_t>
inline
void InfraOpW<graph_t,bitboard_t>::swap_clause_during_recoloring(int iset, int v){
///////////////////////
// swaps iset by singleton clause v

	//remove all nodes from iset 
	Parent::bb_node_state_active.erase_bit(Parent::m_colSets[iset].bb);
	Parent::m_colSets[iset].erase_bit();

	//add singleton
	Parent::m_colSets[iset].push(v);
	m_cw[iset]=Parent::g->get_wv(v);
	Parent::node_iset_no[v]=iset;
	
}

template <class graph_t, class bitboard_t>
inline
int InfraOpW<graph_t,bitboard_t>::update_clause_weights(){
	
	for(int c=1; c<=Parent::NB_OF_COLORS; c++){
		if(Parent::color_state_active.is_bit(c)){
			int maxw=0;
			Parent::m_colSets[c].bb.init_scan(bbo::NON_DESTRUCTIVE);
			while(true){
				int node=Parent::m_colSets[c].bb.next_bit();
				if(node==EMPTY_ELEM) break;
				if(Parent::bb_node_state_active.is_bit(node)){
					if (maxw<m_lw[node]){
						maxw=m_lw[node];
					}
				}
			}
			m_cw[c]=maxw;
		}
	}

	return 0;
}

template <class graph_t, class bitboard_t>
inline
int InfraOpW<graph_t,bitboard_t>::maxsatz(int max_num_conf){
////////////////////
// driver for main infrachrom reasoning

	//init unit_stack
	Parent::color_unit_stack.erase();
	for(int c=Parent::NB_OF_COLORS; c>=1; c--){
		Parent::color_state_active.set_bit(c);
		if(Parent::m_colSets[c].size==1){
			Parent::color_unit_stack.push(c);
		}
	}

	//print_db(true,true);

	//*** new init unit stack
	//for(int i=0; i<node_removed_stack.pt; i++){
	//	int v=node_removed_stack.get_elem(i);
	//	Parent::m_colSets[++Parent::NB_OF_COLORS].erase_bit();	
	//	Parent::m_colSets[Parent::NB_OF_COLORS].push(v);
	//	Parent::color_state_active.set_bit(Parent::NB_OF_COLORS);
	//	//Parent::bb_node_state_active.set_bit(v);
	//	Parent::node_iset_no[v]=Parent::NB_OF_COLORS;
	//	Parent::node_reason[v]=Parent::NO_REASON;
	//	m_cw[Parent::NB_OF_COLORS]=m_lw[v];						//add the weight of v to clauses
	//	Parent::color_unit_stack.push(Parent::NB_OF_COLORS);
	//}


	int save_color_unit_stack_pt=Parent::color_unit_stack.pt;
	
	//set stacks
	Parent::color_conflict_stack.erase();
	Parent::node_stack.erase();
	Parent::color_passive_stack.erase();
	Parent::color_reduced_stack.erase();
	Parent::color_enlarged_stack.erase();
	Parent::color_unit_dyn_stack.erase();

	//stacks for simplications
	color_simplified_stack.erase();						//contains simplified clauses (literals removed)
	node_simplified_stack.erase();


	/*int saved_node_stack_pt=0;
	int saved_color_passive_stack_pt=0;
	int save_color_reduced_stack_pt=0;*/

	//testing
	//print_db(true,true);
	//update_clause_weights();
	
	/*if(!check_consistency_db()){
		print_db(true, true);
		make_consistent_db();
		print_db(true, true);
	}*/
			
	int nb_wconflicts=0, iset;
	while( (iset=Parent::unit_iset_process())!=Parent::NO_CONFLICT ){						
		Parent::lookback_for_maxsatz(iset);
		Parent::reset_context_for_maxsatz(0, 0,	0, save_color_unit_stack_pt);		
		nb_wconflicts+=enlarge_involved_set();				//includes simplification of clauses
	//	print_db(true, true);
		if (nb_wconflicts>=max_num_conf)
				return nb_wconflicts;
	}

	//the stacks might not be empty here (NO_CONFLICT state) 
	Parent::reset_context_for_maxsatz(0, 0, 0, save_color_unit_stack_pt);
		
	if (nb_wconflicts>=max_num_conf)
					return nb_wconflicts;

#ifdef TEST_WEIGHTED_FAILED_LITERALS
	//char i[10];
	Parent::color_unit_stack.erase();
	int flag_found=false;
	int ubred, node, diff=max_num_conf-nb_wconflicts;
	int nb_col_curr=Parent::NB_OF_COLORS;
	for(int c=nb_col_curr; c>=1; c--){
		node=top_literal_info(c, ubred);
		if(ubred>diff){
			//LOG_INFO("FAILED_LITERAL_REDUCTION (req:real):"<<c<<"-"<<node<<":"<<diff<<":"<<ubred);
			///*print_db(true, true);*/
			//flag_found=true;
			//std::cin>>i;

		//	print_db(true, true);


			//***launch engine for the literal
			//1-add the clause as a new color, 
			Parent::m_colSets[++Parent::NB_OF_COLORS].erase_bit();	
			Parent::m_colSets[Parent::NB_OF_COLORS].push(node);
			Parent::color_state_active.set_bit(Parent::NB_OF_COLORS);
			Parent::color_state_active.erase_bit(c);					//*** active literals-CHECK
			
			//Parent::bb_node_state_active.set_bit(v);
			Parent::node_iset_no[node]=Parent::NB_OF_COLORS;
			Parent::node_reason[node]=Parent::NO_REASON;
			int weight_of_node_stored=m_lw[node];
			m_lw[node]=ubred;
			m_cw[Parent::NB_OF_COLORS]=ubred;							//add the weight of v to clauses

		
			Parent::color_unit_stack.push(Parent::NB_OF_COLORS);
			

			int iset;
			bool flag_is_conflict=false;
			while( (iset=Parent::unit_iset_process())!=Parent::NO_CONFLICT ){						
				Parent::lookback_for_maxsatz(iset);
				Parent::reset_context_for_maxsatz(0, 0,	0, save_color_unit_stack_pt);		
				nb_wconflicts+=enlarge_involved_set_with_context();				//includes simplification of clauses
				//print_db(true, true);
				if (nb_wconflicts>=max_num_conf){
					//LOG_INFO("CUT WITH EXTENSION TO LITERALS");
					return nb_wconflicts;
				}
				flag_is_conflict=true;
			}

			//ONE-SHOT ATTEMPT
			//return nb_wconflicts;

			//the stacks might not be empty here (NO_CONFLICT state) 
			Parent::reset_context_for_maxsatz(0, 0, 0, save_color_unit_stack_pt);

			if(!flag_is_conflict){	
				//restore attempted literal
				Parent::node_iset_no[node]=c;
				//Parent::node_reason[node]=Parent::NO_REASON;	
				m_lw[node]=weight_of_node_stored;
				Parent::color_state_active.erase_bit(Parent::NB_OF_COLORS--);
				Parent::color_state_active.set_bit(c);
				//*** clause weight should not have changed

				//restore everything else
				reset_context_for_simplifications(); 
				Parent::color_unit_stack.erase();			//remove attempted clause
				//print_db(true, true);
			}else{
				//clean context structures for future inferences
				color_simplified_stack.erase();
				for(int i=0; i<node_simplified_stack.pt; i++){
					int node=node_simplified_stack.get_elem(i);
					node_weight_state[node]=NOT_UPDATED;
					clause_weight_state[Parent::node_iset_no[node]]=NOT_UPDATED;
				}
				node_simplified_stack.erase();
				
			}

			diff=max_num_conf-nb_wconflicts;		//updates new goal for cuts
		}
	}

	
	/*if(!flag_found){
		LOG_INFO("NO LITERAL FOUND:"<<diff);
		print_db(true, true);
		std::cin>>i;
	}*/


#endif

#ifdef TEST_LOOK_AHEAD_ISETS	
	////set simplified nodes as non-active
	//for(int i=0; i<node_removed_stack.pt; i++){
	//	int node=node_removed_stack.get_elem(i);
	//	bb_node_state_active.erase_bit(node_removed_stack.get_elem(i));
	//	m_colSets[node_iset_no[node]].size--;
	//	if(m_colSets[node_iset_no[node]].size==0)
	//		color_state_active.set_bit(node_iset_no[node]);
	//}
	
	 nb_wconflicts=maxsatz_lookahead(nb_wconflicts, max_num_conf);
	 if (nb_wconflicts>=max_num_conf){
		 //LOG_INFO("CUT DURING TESTS INFRACHROM");
		 return nb_wconflicts;
	 }
#endif

#ifdef TEST_ELIMINATE_FAILED_NODES
	if( (nb_wconflicts + 1) == max_num_conf){
		if(Parent::test_by_eliminate_failed_nodes()){
			 nb_wconflicts++;
			 //LOG_INFO("CONF ELIMINATE FAILED NODES");
		 }
	 }
#endif	
		
	return nb_wconflicts;
}

template <class graph_t, class bitboard_t>
inline
int InfraOpW<graph_t,bitboard_t>::enlarge_involved_set(){
//////////////
// does not store conflict sets 
		
	 int riset;
	 int wincon=weight_of_inconsistent_sets(Parent::color_reason_stack);
	 for(int i=0; i<Parent::color_reason_stack.pt; i++){
		 riset=Parent::color_reason_stack.get_elem(i);
		 
		  //conditionally add to conflict set if the clause is not simplified
		 bool is_simplified=simplify_clause(riset, wincon);
		 if(is_simplified){
			// LOG_INFO("enlarge_involved_set()::CLAUSE_SIMPLIFIED:"<<riset);
			 ;

		 }else{
			 m_cw[riset]=0;
			 Parent::color_state_active.erase_bit(riset);					
		 }
	 }

	return wincon;
}

template <class graph_t, class bitboard_t>
inline
int InfraOpW<graph_t,bitboard_t>::enlarge_involved_set_with_context(){
//////////////
// does not store conflict sets 
		
	 int riset;
	 int wincon=weight_of_inconsistent_sets(Parent::color_reason_stack);
	 for(int i=0; i<Parent::color_reason_stack.pt; i++){
		 riset=Parent::color_reason_stack.get_elem(i);
		 
		  //conditionally add to conflict set if the clause is not simplified
		 bool is_simplified=simplify_clause_with_context(riset, wincon);
		 if(is_simplified){
			// LOG_INFO("enlarge_involved_set()::CLAUSE_SIMPLIFIED:"<<riset);
			 ;

		 }else{
			 Parent::color_state_active.erase_bit(riset);
			 color_simplified_stack.push(riset);			
		 }
	 }

	return wincon;
}


template <class graph_t, class bitboard_t>
inline
int InfraOpW<graph_t,bitboard_t>::enlarge_stored_involved_sets(){
//////////////
// Enlarges color sets in REASON_STACK (involved in a conflict) by adding a new node. 
// and updates CONFLICT_STACK ending with NONE, together with node_conflict_set[ADDED_NODES]
//
// Used when all nodes of a non-unit color set are found conflicting
//
// RETURNS: minimum weight of conflicting sets

	 
	 int riset, minwub=CLQ_MAXINT;
	 Parent::node_conflict_set[Parent::ADDED_NODES]= &Parent::color_conflict_stack.stack[Parent::color_conflict_stack.pt];
	 Parent::bb_node_state_active.set_bit(Parent::ADDED_NODES);
	 int wincon=weight_of_inconsistent_sets(Parent::color_involved_stack);		//clauses may be repeated

	 for(int i=0; i<Parent::color_involved_stack.pt; i++){
		 riset=Parent::color_involved_stack.get_elem(i);

		 if(Parent::iset_involved[riset]==Parent::FALSE_VAL){		//color not previously added
			 Parent::iset_involved[riset]=Parent::TRUE_VAL;			//set as used
			 
			 //deprecated
			/*  if(minwub>m_cw[riset] &&  m_colSets[riset].bb.get_bitboard(NB_OF_BB_NODES)==0)
						 minwub=m_cw[riset];*/

			 //conditionally add to conflict set if the clause is not simplified
			 bool is_simplified=simplify_clause(riset, wincon);
			 if(is_simplified){
				 // LOG_INFO("enlarge_involved_set()::CLAUSE_SIMPLIFIED:"<<riset);
				 ;
			 }else{


				 //add node
				 if( Parent::m_colSets[riset].bb.is_bit(Parent::ADDED_NODES)){
					 Parent::m_colSets[riset].bb.print(); cout<<" adding node:  "<<Parent::ADDED_NODES<<endl;
					 LOG_ERROR("enlarged_involved_set():error in conflict set");
				 }
				 Parent::m_colSets[riset].push(Parent::ADDED_NODES);
				 Parent::color_enlarged_stack.push(riset);
				 Parent::color_conflict_stack.push(riset);
			  }
		 }
	 }

	 Parent::color_conflict_stack.push(Parent::NONE);		//ends in NONE for fix_added_node_for_iset(..) to work
	 Parent::ADDED_NODES++;

	 //restore context for USED and INVOLVED nodes and empty color_involved_stack
	 for(int i=0; i<Parent::color_involved_stack.pt; i++){
		 Parent::iset_involved[Parent::color_involved_stack.get_elem(i)]=Parent::FALSE_VAL;
		 Parent::iset_used_in_test[Parent::color_involved_stack.get_elem(i)]=Parent::FALSE_VAL;
	 }
	 Parent::color_involved_stack.erase();		//pt=0;


	 if(minwub==CLQ_MAXINT) return 0;
	 else return minwub;
 }

template <class graph_t, class bitboard_t>
inline
int InfraOpW<graph_t,bitboard_t>::maxsatz_lookahead(int nb_wconflict, int max_num_conf){
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
	for(int i=0; i<Parent::tested_node_stack.pt; i++){ 
		Parent::node_tested_state[Parent::tested_node_stack.get_elem(i)]=Parent::FALSE_VAL;
	}
	Parent::tested_node_stack.pt=0;
	int nodes_chosen_set[Parent::MAX_COLOR_TEST_LENGTH];

	//main loop: small color sizes
	//int diff=max_num_conf-nb_conflict;					//must be >0
	for(int k=2; k<=Parent::MAX_COLOR_TEST_LENGTH; k++){
		for(int iset=Parent::NB_OF_COLORS; iset>=1; iset--){	
			if( Parent::color_state_active.is_bit(iset) && Parent::m_colSets[iset].size==k					//***can colors be passive here?
				&& !Parent::m_colSets[iset].bb.get_bitboard(Parent::NB_OF_BB_NODES) /* not WEAKENED*/ )	{		
				//check if all nodes in iset are UNTESTED (i.e. have not let to a CONFLICT when tested to TRUE)

				
				bitboard_t& bbcol=Parent::m_colSets[iset].bb;
				bbcol.init_scan(bbo::NON_DESTRUCTIVE);
				test_flag=true;
				int index=0;
				while(true){
					int node=bbcol.next_bit();
					if(node==EMPTY_ELEM) break;
					if(!Parent::bb_node_state_active.is_bit(node))
													continue;  //may be filtered out on structure changes
					nodes_chosen_set[index]=node;
					if(Parent::node_tested_state[nodes_chosen_set[index++]]==Parent::TRUE_VAL){
						test_flag=false;
						break;
					} 
				}
				
				//ASSERT
				if(index!=k){
					LOG_ERROR("maxsatz_lookahead::bizarre clause size-"<<iset);
					test_flag=false;
				}

				if(test_flag){ //valid color for tests
					Parent::color_involved_stack.erase();

					//loop through all nodes and test them
					no_conflict_flag=false;							//assumes a conflict will be found
					for(int i=0; i<k; i++){
						if(Parent::test_node(nodes_chosen_set[i], iset, (i==k-1) /* is_last*/)==Parent::NO_CONFLICT){		//no need to check if already tested again
							no_conflict_flag=true;					//no conflict
							break;
						}else{				//conflict for this particular node
							Parent::store_involved_sets();		
						}
					}

					//conflict found for all nodes: the REAL CONFLICT
					if(no_conflict_flag==false){
						nb_wconflict+=enlarge_stored_involved_sets();
						//LOG_INFO("TEST CONFLICF FOUND");
						if(nb_wconflict>=max_num_conf)
								return nb_wconflict;			//CUT FOUND
					}

					//context operations for next color tests
					for(int i=0; i<Parent::color_involved_stack.pt; i++)
						Parent::iset_used_in_test[Parent::color_involved_stack.get_elem(i)]=Parent::FALSE_VAL;
					Parent::color_involved_stack.pt=0;
				}
			}//end of color test
		}
	}
	
	return nb_wconflict;
}


template <class graph_t, class bitboard_t>
inline
bool InfraOpW<graph_t,bitboard_t>::check_literal_weight_consistency(){
/////////////////////////
//check consistency of initial wieghts of literals which should be 
//the same as in the original graph

	int NV=Parent::g->number_of_vertices();
	for(int node=0; node<NV; node++){
		if(Parent::g->get_wv(node)!=m_lw[node]) 
			return false;
	}
	
	return true;
}

template <class graph_t, class bitboard_t>
inline
void InfraOpW<graph_t,bitboard_t>::reset_context_for_literals(){
////////////////////
// resets weights and state of literals in the simplified stack

	for(int i=0; i<node_simplified_stack.pt; i++){
		int node=node_simplified_stack.get_elem(i);
		m_lw[node]=node_weight[node];
		node_weight_state[node]=NOT_UPDATED;
		Parent::bb_node_state_active.set_bit(node);
		int iset=Parent::node_iset_no[node];
		Parent::m_colSets[iset].size++;

		//**OPTIMIZE-once for each clause
		
		if(clause_weight_state[iset]==UPDATED){
			m_cw[iset]=clause_weight[iset];
			clause_weight_state[iset]=NOT_UPDATED;
		}
	}
	
}

template <class graph_t, class bitboard_t>
inline
void InfraOpW<graph_t,bitboard_t>::reset_inital_weights_of_literals(){
////////////////////
// resets weights of literals and stack

	for(int node=0; node<Parent::NB_OF_NODES; node++){
		m_lw[node]=Parent::g->get_wv(node);
	}
}

template <class graph_t, class bitboard_t>
inline
void InfraOpW<graph_t,bitboard_t>::reset_context_for_simplifications(){
	reset_context_for_literals();
	for(int i=0; i<color_simplified_stack.pt; i++){
		int iset=color_simplified_stack.get_elem(i);
		Parent::color_state_active.set_bit(iset);
	}

	color_simplified_stack.erase();
	node_simplified_stack.erase();
}


template <class graph_t, class bitboard_t>
inline
void InfraOpW<graph_t,bitboard_t>::print_db(bool active_sets_only, bool active_nodes_only){
	int col_active=0;
	cout<<"--------------------------------------------"<<endl;
	cout<<"NB_COL(total):"<<Parent::NB_OF_COLORS<<endl;
	for(int c=1; c<Parent::NB_OF_COLORS; c++){
		if(active_sets_only && !Parent::color_state_active.is_bit(c)) continue;
		col_active++;
		Parent::m_colSets[c].bb.init_scan(bbo::NON_DESTRUCTIVE);
		cout<<c<<"-[";
		while(true){
			int v=Parent::m_colSets[c].bb.next_bit();
			if(v==EMPTY_ELEM) break;
			if(active_nodes_only && !Parent::bb_node_state_active.is_bit(v)) continue;
			cout<<v<<":"<<m_lw[v]<<" ";
		}
		cout<<"]"<<" [w:"<<m_cw[c]<<"]"<<"[s:"<<Parent::m_colSets[c].size<<"]"<<endl;
	}
	if(active_sets_only) cout<<"NB_COL(active):"<<col_active<<endl;
	cout<<"--------------------------------------------"<<endl;
}

template <class graph_t, class bitboard_t>
inline
void InfraOpW<graph_t,bitboard_t>::print_clause(int iset){
	if(!Parent::color_state_active.is_bit(iset)){
		LOG_INFO("printing a non-active clause");
	}
	Parent::m_colSets[iset].bb.init_scan(bbo::NON_DESTRUCTIVE);
	cout<<iset<<"-[";
	while(true){
		int v=Parent::m_colSets[iset].bb.next_bit();
		if(v==EMPTY_ELEM) break;
		if(Parent::bb_node_state_active.is_bit(v)){
			cout<<v<<":"<<m_lw[v]<<" ";
		}
	}
	cout<<"]"<<" [w:"<<m_cw[iset]<<"]"<<"[s:"<<Parent::m_colSets[iset].size<<"]"<<endl;
}

template <class graph_t, class bitboard_t>
inline
 bool InfraOpW<graph_t,bitboard_t>::check_consistency_db( ){
/////////////////
// test consistency of color DB (sizes, weights etc.)
// considering active colors and weights
//
// returns TRUE if test is passed
// 
	
	int cw=0;
	int size=0;
	int col_active=0;
	for(int c=1; c<Parent::NB_OF_COLORS; c++){
		if(Parent::color_state_active.is_bit(c)) {
			col_active++;
			cw=0; size=0;
			Parent::m_colSets[c].bb.init_scan(bbo::NON_DESTRUCTIVE);
			while(true){
				int v=Parent::m_colSets[c].bb.next_bit();
				if(v==EMPTY_ELEM) break;
				if(Parent::bb_node_state_active.is_bit(v)) {
					size++;
					if(cw<m_lw[v])
							cw=m_lw[v];
				}
			}

			if(cw!=m_cw[c]){
				cout<<"--------------------------------------------"<<endl;
				LOG_ERROR("check_consistency()::"<<"clause weight inconsistent-["<<c<<":"<<m_cw[c]<<":"<<cw<<"]");
				print_clause(c);
				cout<<"--------------------------------------------"<<endl;
				return false;
			}
			if(size!=Parent::m_colSets[c].size){
				cout<<"--------------------------------------------"<<endl;
				LOG_ERROR("check_consistency()::"<<"clause size inconsistent-["<<c<<":"<<Parent::m_colSets[c].size<<":"<<size<<"]");
				print_clause(c);
				cout<<"--------------------------------------------"<<endl;
				return false;
			}
		}
	}
	return true;
	
}

template <class graph_t, class bitboard_t>
inline
int InfraOpW<graph_t,bitboard_t>::make_consistent_db(){
///////////////////////
// makes color db consistent taking into account active color sets 
// and literals (removes empty literals and colors)
//
// RETURNS 0 if db was already consistent, -1 otherwise
// REMARKS
// 1-Takes as reference the active nodes for clause parameters

	int cw, csize;
	bool flag_change=false;
	for(int c=1; c<Parent::NB_OF_COLORS; c++){
		if(Parent::color_state_active.is_bit(c)) {
			cw=0; csize=0;
			Parent::m_colSets[c].bb.init_scan(bbo::NON_DESTRUCTIVE);
			while(true){
				int v=Parent::m_colSets[c].bb.next_bit();
				if(v==EMPTY_ELEM) break;
				if(Parent::bb_node_state_active.is_bit(v)) {
					csize++;
					if(m_lw[v]<=0){
						Parent::bb_node_state_active.erase_bit(v);
						flag_change=true;
					}else if(cw<m_lw[v]){
						cw=m_lw[v];
					}
				}
			}

			if(cw!=m_cw[c]){
				m_cw[c]=cw;
				flag_change=true;
			}
			if(csize<=0){
				Parent::color_state_active.erase_bit(c);
				flag_change=true;
			}else if (csize!=Parent::m_colSets[c].size){
				Parent::m_colSets[c].size=csize;
				flag_change=true;
			}
		}
	}

	if(flag_change) return -1;
	return 0;
}


template <class graph_t, class bitboard_t>
inline
bool InfraOpW<graph_t,bitboard_t>::check_consistency_clause(int iset, bool only_active){
	if(only_active && !Parent::color_state_active.is_bit(iset)){
		LOG_ERROR("check_consistency_clause()::inactive clause-"<<iset);
		return false;
	}

	int cw=0, size=0;
	Parent::m_colSets[iset].bb.init_scan(bbo::NON_DESTRUCTIVE);
	while(true){
		int v=Parent::m_colSets[iset].bb.next_bit();
		if(v==EMPTY_ELEM) break;
		if(Parent::bb_node_state_active.is_bit(v)) {
			size++;
			if(cw<m_lw[v])
				cw=m_lw[v];
		}
	}

	if(cw!=m_cw[iset]){
		cout<<"--------------------------------------------"<<endl;
		LOG_ERROR("check_consistency()::"<<"clause weight inconsistent-["<<iset<<":"<<m_cw[iset]<<":"<<cw<<"]");
		print_clause(iset);
		cout<<"--------------------------------------------"<<endl;
		return false;
	}
	if(size!=Parent::m_colSets[iset].size){
		cout<<"--------------------------------------------"<<endl;
		LOG_ERROR("check_consistency()::"<<"clause size inconsistent-["<<iset<<":"<<Parent::m_colSets[iset].size<<":"<<size<<"]");
		print_clause(iset);
		cout<<"--------------------------------------------"<<endl;
		return false;
	}
	
	
	return true;
}

template <class graph_t, class bitboard_t>
inline
int InfraOpW<graph_t,bitboard_t>::top_literal_info(int iset, int& red){
/////////////////////////////
// For a given clause iset, computes the top active node and the reduction red
// red: difference in weight wrt to the second active node
//
// RETURNS: top literal or -1 if EMPTY, red: reduction (measured as diff wrt second literal)
//
// REMARKS: Assumes literals are ordered by non-decreasing weights in iset

	red=0;
	if(!Parent::color_state_active.is_bit(iset) ||Parent::m_colSets[iset].size==1 ) return EMPTY_ELEM;
	int lv[2], counter=0; 
	

	bool flag_found=false;
	bitarray& bbiset=Parent::m_colSets[iset].bb;
	bbiset.init_scan(bbo::NON_DESTRUCTIVE_REVERSE);
	while(true){
		int v=bbiset.previous_bit();
		if(v==EMPTY_ELEM) break;
		if(Parent::bb_node_state_active.is_bit(v)){
			lv[counter++]=v;
			if(counter==2){
				flag_found=true;
				break;
			}
		}
	}

	if(flag_found){
		red=m_lw[lv[0]]-m_lw[lv[1]];
		return lv[0];
	}
	
	return EMPTY_ELEM;
}




#endif




