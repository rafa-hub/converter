//infra_tools_plus_csp.h: Interface for InfraChrom filter when solving CSP problems as MCP
// date of creation: 02/06/17


#ifndef __INFRA_TOOLS_PLUS_CSP_H__
#define	__INFRA_TOOLS_PLUS_CSP_H__

#include "utils/common.h"
#include "infra_tools_plus.h"

using namespace com;

template<class graph_t, class bitboard_t>
class InfraOpPlusCSP: public InfraOpPlus<graph_t, bitboard_t>{
/////////////////////
// Specializes InfraOpPlus for solving CSP as MCP
// date of creation: 2/6/17

	typedef InfraOpPlus<graph_t, bitboard_t> Parent;

public:
	InfraOpPlusCSP(graph_t* g=NULL):Parent(g){}
	
public:
	bool inc_csp_maxsatz(int v);																	//v is the vertex in unit set to filter (***TODO: v currently not used inside the function, check!!)
	int hard_fix_node_for_unit_iset(int v, int iset);												//essentially removes v from DB
	int hard_fix_node_for_unit_iset(int v, int iset, bitarray& bb);
	int hard_unit_iset_process();
	int hard_unit_iset_process(bitarray& bb, sbb_t<bitarray>& s);
	int inc_test_node_csp(int v, int iset, bool further=false);
	int  unit_iset_process_for_test_csp(bitarray& bbsg, sbb_t<bitarray>& s);
	int test_node_for_failed_nodes_csp(int v, int iset);
	int unit_iset_process_for_test_by_eliminating_failed_nodes_csp();	
	int init_inc_maxsatz_csp();
	int further_test_csp(int start);	

	bool test_by_eliminate_failed_nodes_csp(bitarray& bb, sbb_t<bitarray>&);							//Main Test driver O(ACTIVE ISETS * ACTIVE NODES IN ISET)
	bool test_by_eliminate_failed_nodes_csp(bitarray& bb, sbb_t<bitarray>&, com::stack_t<int>& cand);	//Main Test driver O(ACTIVE ISETS * ACTIVE NODES IN ISET)
	bool inc_maxsatz_lookahead_csp(bitarray& bb, sbb_t<bitarray>& s);

};

template <class graph_t, class bitboard_t>
inline
bool InfraOpPlusCSP<graph_t,bitboard_t>::inc_csp_maxsatz(int v){
//////////////////
// PARAMS: v is the node in a singleton color (kmin) that wants to be filtered
//		  Currently, v is not used (see 2), although it may be used for
//		  tests (*** TODO-CHANGE LOGIC ***)
//
// RETURNS: TRUE if a conflict is found or FALSE_VAL otherwise 
//
// REMARKS:
// 1.Assumes singleton color iset={v] is last in the unit STACK  (2)
	
	int save_color_unit_stack_pt=Parent::color_unit_stack.pt;
	Parent::node_stack.erase();
	Parent::color_passive_stack.erase();
	Parent::color_reduced_stack.erase();
	Parent::color_unit_dyn_stack.erase();			

	//swap first-last colors in color_unit_stack
	//the last color is the singleton color of v
	int first_color=Parent::color_unit_stack.get_elem(0);
	Parent::color_unit_stack.stack[0]=Parent::color_unit_stack.stack[Parent::color_unit_stack.pt-1];
	Parent::color_unit_stack.stack[Parent::color_unit_stack.pt-1]=first_color;
		
	int iset;
	if ((iset=Parent::unit_iset_process())!=Parent::NO_CONFLICT) {
		Parent::lookback_for_maxsatz(iset);
		Parent::reset_context_for_maxsatz(0, 0, 0, save_color_unit_stack_pt);
		Parent::enlarge_conflict_set();
		return true;
	}
		
	//the stacks might not be empty here (NO_CONFLICT state)-PSS
	Parent::reset_context_for_maxsatz(0, 0, 0, save_color_unit_stack_pt);	
	

	//attempt to find a conflict in color classes of sizes 2 or 3
	return(Parent::inc_maxsatz_lookahead(save_color_unit_stack_pt));

	/*bool res=inc_maxsatz_lookahead(save_color_unit_stack_pt);
	reset_context_for_maxsatz(0, 0, 0,save_color_unit_stack_pt);
	 return res; */
	
/////////////	
//*** elimination inferences possible?
//bool result=test_by_eliminate_failed_nodes();
//reset_context_for_maxsatz(0, 0, 0,save_color_unit_stack_pt);
////if(result==true){
////	return EMPTY_ELEM;	//conflict found
////}
////	
////return false;
//  return result;

//	return false;;
}


template <class graph_t, class bitboard_t>
inline
int InfraOpPlusCSP<graph_t,bitboard_t>::hard_fix_node_for_unit_iset(int v, int iset){
//////////////////////
// PARAMS: v: single (not weakened) ACTIVE vertex of iset
//		   iset: unit color set containing single active node v
//
// FUNCTION: removes (simplifies) v from color DB (v=TRUE). stores nodes related to the inference.
//        	 Context notexpected to be stored. 
//
// REMARKS: note that propagation only occurs in active vertices, so passive nodes in color sets will never be reduced/excluded-
//
// date of creation: 22/3/17

	//update state info of v and iset
	Parent::color_state_active.erase_bit(iset);
	//color_passive_stack.push(iset);
	//assign_node_value(v, TRUE_VAL, iset);

	//main loop of non-neighbor vertices
	bitboard_t& non_nbb= Parent::gc->get_neighbors(v);
	
	//main loop: iterates over ACTIVE--NON-NEIGHBOR nodes of v
	BITBOARD bb;
	int nnbor, offset, my_iset;
	for(int nBB=0; nBB<Parent::NB_OF_BB_NODES; nBB++){		//only real vertices, not weakened
		if( bb=(non_nbb.get_bitboard(nBB) & Parent::node_state_active.get_bitboard(nBB)) ){
			//decode vertices
			offset=WMUL(nBB);
			while(bb){
			
				nnbor=BitBoard::lsb64_intrinsic(bb);
				//if( (nnbor=BitBoard::lsb64_intrinsic(bb))==EMPTY_ELEM) {break;}
				bb^=Tables::mask[nnbor];
				
				nnbor+=offset;
				my_iset=Parent::node_iset_no[nnbor];
			    Parent::m_colSets[my_iset].erase_bit(nnbor);
				if( Parent::m_colSets[my_iset].size==0) return my_iset;
				else if(Parent::m_colSets[my_iset].size==1){
						Parent::color_unit_dyn_stack.push(my_iset); 
				}
				Parent::node_state_active.erase_bit(nnbor);
				LOG_INFO("VERTEX REMOVED DURING HARD FIX");

				//if( (my_iset=exclude_noneibor(nnbor,iset))!=NO_CONFLICT) {	
				//	return my_iset;
				//}  //use nnbor
			}
		}
	}
	
	return Parent::NO_CONFLICT;
}

template <class graph_t, class bitboard_t>
inline
int InfraOpPlusCSP<graph_t,bitboard_t>::hard_fix_node_for_unit_iset(int v, int iset, bitarray& bbsg){
//////////////////////
// PARAMS: v: single (not weakened) ACTIVE vertex of iset
//		   iset: unit color set containing single active node v
//
// FUNCTION: removes (simplifies) v from color DB (v=TRUE). stores nodes related to the inference.
//        	 Context notexpected to be stored. 
//
// REMARKS: note that propagation only occurs in active vertices, so passive nodes in color sets will never be reduced/excluded-
//
// date of creation: 22/3/17

	//update state info of v and iset
	Parent::color_state_active.erase_bit(iset);
	//color_passive_stack.push(iset);

	//assign_node_value(v, TRUE_VAL, iset);			/* CHECK */
	Parent::node_state_active.erase_bit(v);
	
	//alternative loop based on active node enumeration: preformance is similar (25/4/17)
	//bitboard_t& non_nbb= gc->get_neighbors(v);
	//node_state_active.init_scan(bbo::NON_DESTRUCTIVE);
	//while(true){
	//	int nnbor=node_state_active.next_bit();
	//	if(nnbor==EMPTY_ELEM) break;
	//	if(non_nbb.is_bit(nnbor)){

	//main loop of non-neighbor vertices
	bitboard_t& non_nbb= Parent::gc->get_neighbors(v);
	
	//main loop: iterates over ACTIVE--NON-NEIGHBOR nodes of v
	BITBOARD bb;
	int nnbor, offset, my_iset;
	for(int nBB=0; nBB<Parent::NB_OF_BB_NODES; nBB++){		//only real vertices, not weakened
		if( bb=(non_nbb.get_bitboard(nBB) & Parent::node_state_active.get_bitboard(nBB)) ){
			offset=WMUL(nBB);
			while(bb){
				nnbor=BitBoard::lsb64_intrinsic(bb);
				if( (nnbor=BitBoard::lsb64_intrinsic(bb))==EMPTY_ELEM) {break;}
				bb^=Tables::mask[nnbor];
				
				nnbor+=offset;

				bbsg.erase_bit(nnbor);
				//int my_iset=node_iset_no[nnbor];
				my_iset=Parent::node_iset_no[nnbor];
			    Parent::m_colSets[my_iset].erase_bit(nnbor);
				if( Parent::m_colSets[my_iset].size==0) return my_iset;
				else if(Parent::m_colSets[my_iset].size==1){
					Parent::color_unit_dyn_stack.push(my_iset);					
				}/*else if (m_colSets[my_iset].size<=3){
					LOG_INFO("REDUCING SIZE OF COLOR DURING NODE ELIMINATION: "<<my_iset<<" size: "<<m_colSets[my_iset].size);
					cin.get();
				}*/
				Parent::node_state_active.erase_bit(nnbor);
				//LOG_INFO("VERTEX REMOVED DURING HARD FIX");
			
				

				//if( (my_iset=exclude_noneibor(nnbor,iset))!=NO_CONFLICT) {	
				//	return my_iset;
				//}  //use nnbor
			}
		}
	}
	
	return Parent::NO_CONFLICT;
}

template <class graph_t, class bitboard_t>
inline
int InfraOpPlusCSP<graph_t,bitboard_t>::hard_unit_iset_process(){
/////////////
// inference mechanism for singleton colors without contex restoring
//
// should be used in color_DBs WITHOUT added nodes
//
// RETURNS conflicting color set or NO_CONFLICT

	int col, my_iset;
	for(int i=0; i<Parent::color_unit_stack.pt; i++){
		col=Parent::color_unit_stack.get_elem(i);
		if(Parent::color_state_active.is_bit(col) && Parent::m_colSets[col].size==1){		
			Parent::color_unit_dyn_stack.erase();
			int v=FIRST_SHARED(Parent::m_colSets[col].bb, Parent::node_state_active);	
			if ((my_iset=hard_fix_node_for_unit_iset(v,col))!=Parent::NO_CONFLICT )				
				return my_iset;
			
		
			for(int j=0; j<Parent::color_unit_dyn_stack.pt; j++){
				col=Parent::color_unit_dyn_stack.get_elem(j);
				if(Parent::color_state_active.is_bit(col)){
					int v=FIRST_SHARED(Parent::m_colSets[col].bb, Parent::node_state_active);	
					if ((my_iset=hard_fix_node_for_unit_iset(v,col))!=Parent::NO_CONFLICT )
						return my_iset;
				}
			}
		}
	}
	
	
	Parent::color_unit_dyn_stack.erase();			
	return Parent::NO_CONFLICT;
}

template <class graph_t, class bitboard_t>
inline
int InfraOpPlusCSP<graph_t,bitboard_t>::hard_unit_iset_process(bitarray& bbsg, sbb_t<bitarray>& s){
/////////////
// inference mechanism for singleton colors without contex restoring
//
// should be used in color_DBs WITHOUT added nodes
//
// RETURNS conflicting color set or NO_CONFLICT

	int col, my_iset;
	for(int i=0; i<Parent::color_unit_stack.pt; i++){
		col=Parent::color_unit_stack.get_elem(i);
		if(Parent::color_state_active.is_bit(col) && Parent::m_colSets[col].size==1){	
			Parent::color_unit_dyn_stack.erase();
			//int v=FIRST_SHARED(m_colSets[col].bb, node_state_active);
			int v=Parent::m_colSets[col].bb.lsbn64();

			s.push(v);
			if ((my_iset=hard_fix_node_for_unit_iset(v,col, bbsg))!=Parent::NO_CONFLICT )				
				return my_iset;
			
		
			for(int j=0; j<Parent::color_unit_dyn_stack.pt; j++){
				col=Parent::color_unit_dyn_stack.get_elem(j);
				if(Parent::color_state_active.is_bit(col)){
				//	int v=FIRST_SHARED(m_colSets[col].bb, node_state_active);
					int v=Parent::m_colSets[col].bb.lsbn64();
					s.push(v);
					if ((my_iset=hard_fix_node_for_unit_iset(v, col, bbsg))!=Parent::NO_CONFLICT )
						return my_iset;
				}
			}
		}
	}
	
	
	Parent::color_unit_dyn_stack.erase();			
	return Parent::NO_CONFLICT;
}

template <class graph_t, class bitboard_t>
inline
bool InfraOpPlusCSP<graph_t,bitboard_t>::test_by_eliminate_failed_nodes_csp (bitarray& bbsg, sbb_t<bitarray>& s){
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
//
// COMMENTS: Passing subgraph, deletion of conflicting nodes not working!!!
//
// TESTING ELIMINATING ONLY THE LAST NODE WHICH IS IN UNIT COLOR NB_OF_COLORS

	int my_iset, nb_partial_conf, node, save_color_unit_stack_pt=Parent::color_unit_stack.pt;	
	bool conflict=false;
		
	BITBOARD bb;
	do{
		nb_partial_conf=0;
		for(int k=4; k>=2; k--){
			for(int iset=Parent::NB_OF_COLORS/*-1 if it is known that the last color is an already tested singleton */; iset>=1; iset--){
				//for(int iset=1; iset>=NB_OF_COLORS; iset++){
				//if(m_colSets[iset].size<=4 /* seems best for many CSPs */ && color_state_active.is_bit(iset) ){	
				if(Parent::m_colSets[iset].size==k /* seems best for many CSPs */ && Parent::color_state_active.is_bit(iset) ){	
					if(Parent::m_colSets[iset].size==1){
						LOG_ERROR("BIZARRE-TESTING SINGLE COLOR FOR NODE ELIMINATION");
						cin.get();
					}
					conflict=false;
					Parent::color_unit_dyn_stack.erase();

					//attempt to prove FALSE (eliminate) all (not ADDED) nodes from the iset
					for(int nBB=0; nBB<Parent::NB_OF_BB_NODES; nBB++){														/*only not added nodes*/
						if( bb=(Parent::m_colSets[iset].bb.get_bitboard(nBB) & Parent::node_state_active.get_bitboard(nBB)) ){
							while(bb){
								node=BitBoard::lsb64_intrinsic(bb);
								bb^=Tables::mask[node];
								node+=WMUL(nBB);

								/////////////////////
								//testing node
								if( test_node_for_failed_nodes_csp(node, iset)!=Parent::NO_CONFLICT){
									//	if( test_node_for_failed_nodes(node, iset)!=NO_CONFLICT){
									nb_partial_conf++;		

									//remove conflicting node
									bbsg.erase_bit(node);
									Parent::node_state_active.erase_bit(node);
									Parent::m_colSets[iset].erase_bit(node);						/* alternatively m_colSets[iset].size--;	*/

									if(Parent::m_colSets[iset].size==0)	{		
										return true;										/* conflict found: all values conflicting  */
									}
								}

								//////////////////////
								/* no conflict- continues with other nodes of same color in the same bitblock*/

							}//next node in same block

						}
						/* no conflict- continues with other nodes of same color in another bitblock*/

					}//new block to test nodes

					//if active color has turned single by elimination of values, then hard-propagates the unit color, making permanent changes 
					if(Parent::m_colSets[iset].size==1){
						//LOG_INFO("unit color set expansion during test eliminate nodes: "<<iset);
						Parent::color_unit_dyn_stack.erase();
						Parent::color_unit_dyn_stack.push(iset);
						if(unit_iset_process_for_test_csp(bbsg, s)!=Parent::NO_CONFLICT ){
							//LOG_INFO("InfraOpPlus<graph_t,bitboard_t>::test_by_eliminate_failed_nodes_csp-unit color derived when removing nodes");
							return true;	
						}
					}
				}//filter color_state active

			}//next color to test
		}//next color size
	}while(nb_partial_conf>1 /* >=1 increases so much overhead?, check*/ );

//	reset_context_for_maxsatz(0,0,0,save_color_unit_stack_pt);	/* TODO see if needed and why */
	
	return(conflict);
}

template <class graph_t, class bitboard_t>
inline
bool InfraOpPlusCSP<graph_t,bitboard_t>::test_by_eliminate_failed_nodes_csp (bitarray& bbsg, sbb_t<bitarray>& s, com::stack_t<int>& cand){
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
//
// COMMENTS: Passing subgraph, deletion of conflicting nodes not working!!!
//
// TESTING ELIMINATING ONLY THE LAST NODE WHICH IS IN UNIT COLOR NB_OF_COLORS

	int my_iset, nb_partial_conf, node, save_color_unit_stack_pt=Parent::color_unit_stack.pt;	
	bool conflict=false;
		
	BITBOARD bb;
	do{
		nb_partial_conf=0;
		for(int iset=Parent::NB_OF_COLORS /*-1 if it is known that the last color is an already tested singleton */; iset>=1; iset--){
			if(Parent::m_colSets[iset].size==2 /* seems best for many CSPs */ && Parent::color_state_active.is_bit(iset) ){	
				if(Parent::m_colSets[iset].size==1){
					LOG_ERROR("BIZARRE-TESTING SINGLE COLOR FOR NODE ELIMINATION");
					cin.get();
				}
				conflict=false;
				Parent::color_unit_dyn_stack.erase();

				//attempt to prove FALSE (eliminate) all (not ADDED) nodes from the iset
				for(int nBB=0; nBB<Parent::NB_OF_BB_NODES; nBB++){														/*only not added nodes*/
					if( bb=(Parent::m_colSets[iset].bb.get_bitboard(nBB) & Parent::node_state_active.get_bitboard(nBB)) ){
						while(bb){
							node=BitBoard::lsb64_intrinsic(bb);
							bb^=Tables::mask[node];
							node+=WMUL(nBB);
												
							/////////////////////
							//testing node
							if( test_node_for_failed_nodes_csp(node, iset)!=Parent::NO_CONFLICT){
						//	if( test_node_for_failed_nodes(node, iset)!=NO_CONFLICT){
								nb_partial_conf++;		

								//remove conflicting node
								bbsg.erase_bit(node);
								Parent::node_state_active.erase_bit(node);
								Parent::m_colSets[iset].erase_bit(node);						/* alternatively m_colSets[iset].size--;	*/
												
								if(Parent::m_colSets[iset].size==0)	{		
									return true;										/* conflict found: all values conflicting  */
								}
							}

							//////////////////////
							/* no conflict- continues with other nodes of same color in the same bitblock*/
							
						}//next node in same block
											
					}
					/* no conflict- continues with other nodes of same color in another bitblock*/
									
				}//new block to test nodes

				//if active color has turned single by elimination of values, then hard-propagates the unit color, making permanent changes 
				if(Parent::m_colSets[iset].size==1){
					//LOG_INFO("unit color set expansion during test eliminate nodes: "<<iset);
					Parent::color_unit_dyn_stack.erase();
					Parent::color_unit_dyn_stack.push(iset);
					if(unit_iset_process_for_test_csp(bbsg, s)!=Parent::NO_CONFLICT ){
						//LOG_INFO("InfraOpPlus<graph_t,bitboard_t>::test_by_eliminate_failed_nodes_csp-unit color derived when removing nodes");
						return true;	
					}
				}else {cand.stack[0]=FIRST_SHARED(Parent::m_colSets[iset].bb, Parent::node_state_active); cand.pt++;};	
			}//filter color_state active
	
		}//next color to test
	}while(nb_partial_conf>1 /* >=1 increases so much overhead?, check*/ );

//	reset_context_for_maxsatz(0,0,0,save_color_unit_stack_pt);	/* TODO see if needed and why */
	
	return(conflict);
}

template <class graph_t, class bitboard_t>
inline
bool InfraOpPlusCSP<graph_t,bitboard_t>::inc_maxsatz_lookahead_csp(bitarray& bbsg, sbb_t<bitarray>& s){
//////////
// RETURNS TRUE if one conflict is found, FALSE otherwise. 
// Tailored for CSPs but currently applied to non-incremental version
// 
// COMMENTS: Does not store involved sets, since one conflict is enough to CUT
//
// REMARKS:
// 1.no testing of active nodes inside the colors (assumes color_db is consistent, i.e. no reduced nodes)

	bool test_flag, no_conflict_flag, one_conflict=false;

	for(int k=2; k<=Parent::MAX_COLOR_TEST_LENGTH; k++){
		for(int iset=1; iset<=Parent::NB_OF_COLORS; iset++){	
		//for(int iset=NB_OF_COLORS; iset>=1; iset--){	
			if(Parent::m_colSets[iset].size==k  && Parent::color_state_active.is_bit(iset) 
				/*&& !m_colSets[iset].bb.get_bitboard(NB_OF_BB_NODES) /* not weakened*/){				  
				
					//color_involved_stack.erase();
					bitboard_t& bbcol=Parent::m_colSets[iset].bb;
					bbcol.init_scan(bbo::NON_DESTRUCTIVE);
					no_conflict_flag=false;
					for(int i=0; i<k; i++){
						int node=bbcol.next_bit();
						if(inc_test_node_csp(node, iset, (i==k-1) /* is last node*/)==Parent::NO_CONFLICT){		 /*no testing of active nodes-TEST*/
							no_conflict_flag=true;					//no conflict
							break;
						}else{					//conflict for this particular node
							bbsg.erase_bit(node);
																						
							/*LOG_INFO("REMOVED NODE TEST ELIMINATE NODE");
							cin.get();*/
							Parent::node_state_active.erase_bit(node);
							Parent::m_colSets[iset].erase_bit(node);								/* m_colSets[iset].size--; */
							if(Parent::m_colSets[iset].size==0){									/* only check for empty set, not single color propagation */
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
					else if(Parent::m_colSets[iset].size==1){   /* hard propagates singleton color-note that initially no color mayhave size 1 so MUST HAVE BEEN REDUCED  */
						//LOG_INFO("JLJLKHLJG");
						//s.push(FIRST_SHARED(m_colSets[iset].bb, node_state_active));/* test if node exists */	
						Parent::color_unit_dyn_stack.erase();								/* test impact of this */
						Parent::color_unit_dyn_stack.push(iset);							/* a la test_eliminate_node */
						if(unit_iset_process_for_test_csp(bbsg, s)!=Parent::NO_CONFLICT ){
							return true;											/* conflict found */
						}
					}
			}
		}//next color (iset)
	}

	return false;
}

template <class graph_t, class bitboard_t>
inline
int InfraOpPlusCSP<graph_t,bitboard_t>::inc_test_node_csp(int node, int iset, bool is_last_node){
///////////////////////////
// Simplified test_node function
// 1.uses simple unit_iset_process() instead of  unit_iset_process_for_tests()
//
// Does not use lookback to store involved sets
//
// REMARKS: check reset_contexts
	
	//test-remove in release
	if(!Parent::node_state_active.is_bit(node)){
		LOG_ERROR("InfraOpPlus::inc_test_node-called for a node which is not active");
		return Parent::NO_CONFLICT;
	}
	
	int save_color_unit_stack_pt=Parent::color_unit_stack.pt, my_iset;				
	Parent::color_unit_dyn_stack.erase();
	
	if( (my_iset=Parent::fix_node_for_non_singleton_iset(node, iset)) != Parent::NO_CONFLICT ||		
		//(my_iset=unit_iset_process()) !=  NO_CONFLICT												/* default option in normal incremental implementation */
		(my_iset=unit_iset_process_for_test_by_eliminating_failed_nodes_csp()) !=  Parent::NO_CONFLICT			/* recent attempt, so as not to waste singletons found in the prev. step -26-3-17 (tests were not impressive)*/ 
		

#ifdef FURTHER_TEST_FAILED_COLOR_SETS
		|| 	(is_last_node && (my_iset=further_test_csp(0 /* all colors in reduced stack*/)) != Parent::NO_CONFLICT)
#endif		
		){
		//	lookback_for_maxsatz(my_iset);													/* not needed for csp */			
			Parent::reset_context_for_maxsatz(0, 0,  0,	 save_color_unit_stack_pt);
			return my_iset;

	}else{ /* NO CONFLICT */
		Parent::reset_context_for_maxsatz(0, 0, 0, save_color_unit_stack_pt);
	}

  	return Parent::NO_CONFLICT;
}

template <class graph_t, class bitboard_t>
inline
int  InfraOpPlusCSP<graph_t,bitboard_t>::unit_iset_process_for_test_csp(bitarray& bbsg, sbb_t<bitarray>& s){
/////////////////
// simple inference engine over color_unit_dyn_stack

	int iset, my_iset;
	for(int j=0; j<Parent::color_unit_dyn_stack.pt; j++) {
		iset=Parent::color_unit_dyn_stack.get_elem(j);
		if(Parent::color_state_active.is_bit(iset)){						/* proving test unnecessary will be difficult */
			//int v=FIRST_SHARED(m_colSets[iset].bb, node_state_active);	
			int v=Parent::m_colSets[iset].bb.lsbn64();

			s.push(v);
			if ((my_iset=hard_fix_node_for_unit_iset(v,iset, bbsg))!=Parent::NO_CONFLICT )
						return my_iset;
			
		}
	}

	return Parent::NO_CONFLICT;
}


template <class graph_t, class bitboard_t>
inline
int InfraOpPlusCSP<graph_t,bitboard_t>::test_node_for_failed_nodes_csp(int node, int iset){
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
	int save_color_unit_stack_pt=Parent::color_unit_stack.pt;	
	int saved_node_stack_pt=Parent::node_stack.pt;							
	int saved_color_passive_stack_pt=Parent::color_passive_stack.pt;		
	int save_color_reduced_stack_pt=Parent::color_reduced_stack.pt;			
	Parent::color_unit_dyn_stack.erase();
	if ((my_iset=Parent::fix_node_for_non_singleton_iset(node, iset)) == Parent::NO_CONFLICT) {	
#ifdef FURTHER_TEST_ELIMINATE_FAILED_NODES
		if ((my_iset=unit_iset_process_for_test_by_eliminating_failed_nodes_csp()) == Parent::NO_CONFLICT)
			  my_iset=Parent::simple_further_test(save_color_reduced_stack_pt);								/* propagation through REDUCED_STACK */
#else
			my_iset=unit_iset_process_for_tests();
#endif
	}

	 Parent::reset_context_for_maxsatz(saved_node_stack_pt,
								saved_color_passive_stack_pt,
								save_color_reduced_stack_pt,
								save_color_unit_stack_pt);
	return my_iset;
}

template <class graph_t, class bitboard_t>
inline
int  InfraOpPlusCSP<graph_t,bitboard_t>::unit_iset_process_for_test_by_eliminating_failed_nodes_csp(){
/////////////////
// simple inference engine over color_unit_dyn_stack
	int iset, my_iset;
	for(int j=0; j<Parent::color_unit_dyn_stack.pt; j++) {
		iset=Parent::color_unit_dyn_stack.get_elem(j);
		if(Parent::color_state_active.is_bit(iset)){						/* proving test unnecessary will be difficult */
			int v=FIRST_SHARED(Parent::m_colSets[iset].bb,Parent::node_state_active);		
			if ((my_iset=Parent::fix_node_for_iset(v, iset))!=Parent::NO_CONFLICT)						
				return my_iset;
		}
	}

	return Parent::NO_CONFLICT;
}

template <class graph_t, class bitboard_t>
inline
int InfraOpPlusCSP<graph_t,bitboard_t>::init_inc_maxsatz_csp(){
///////////////
// incremental maxsatz which is configured for
// a single conflict
//
// RETURNS: number of unit colors found
//
// REMARKS:
// 1. assumes ACTIVE NODES ARE CORRECTLY SET
// 2. assumes color_db is consistent, no enlarged color sets etc.!!

	
  int nb_unit_col=0;
	//reset_enlarged_isets_lazy();						/* cleans enlarged color sets as well */
	Parent::color_unit_stack.pt=0;
	for(int i=Parent::NB_OF_COLORS; i>=1; i--){					/* smallest colors first! */
	//	color_involved_state[i]=FALSE_VAL;				/* new compared with non-incremental, is it necessary? */
		Parent::color_state_active.set_bit(i);
		if (Parent::m_colSets[i].size==1){
			nb_unit_col++;
			Parent::color_unit_stack.push(i);
		}
	}

	//minimal init config. on exit: possible remove
	/*reset_nb_added_nodes();
	node_state_active.get_bitboard(NB_OF_BB_NODES)=0;	
	color_conflict_stack.erase();
	color_enlarged_stack.erase();*/
	
	/* CHECK IF BEST LAZY RESETTING : reset_enlarged_isets_lazy()? */

	return nb_unit_col;
}

template <class graph_t, class bitboard_t>
inline
int InfraOpPlusCSP<graph_t,bitboard_t>::further_test_csp(int start){
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
	int save_color_unit_stack_pt=Parent::color_unit_stack.pt;				
	int saved_node_stack_pt=Parent::node_stack.pt;						
	int saved_color_passive_stack_pt=Parent::color_passive_stack.pt;		
	int save_color_reduced_stack_pt=Parent::color_reduced_stack.pt;	
	int node;

	Parent::color_already_used_in_extended_test.erase_bit();							/*avoids repetition of color in this test (REDUCED STACK contains repeated colors)*/
	/*for(int i=start; i<save_color_reduced_stack_pt; i++){
		color_already_used_in_extended_test.is_bit(color_reduced_stack.get_elem(i));
	}*/
	
	//main loop
	for(int i=start; i<save_color_reduced_stack_pt; i++) {						
		chosen_iset=Parent::color_reduced_stack.get_elem(i);
		if(!Parent::color_already_used_in_extended_test.is_bit(chosen_iset) && Parent::color_state_active.is_bit(chosen_iset) 
			&& Parent::m_colSets[chosen_iset].size==Parent::MAX_COLOR_FURTHER_TEST_LENGTH									/*must always be greater than 1, MAX_COLOR_FURTHER_TEST_LENGTH is typically 2 */
			/*&& m_colSets[chosen_iset].bb.get_bitboard(NB_OF_BB_NODES)==0/* not weakened*/){
			
			Parent::color_already_used_in_extended_test.set_bit(chosen_iset);

			//loop to test all (both) nodes
			bitboard_t bbcol=Parent::m_colSets[chosen_iset].bb;
			bbcol.init_scan(bbo::NON_DESTRUCTIVE);
			for(int j=0; j<Parent::MAX_COLOR_FURTHER_TEST_LENGTH; j++){
				do{
					node=bbcol.next_bit();											/*careful, should not be scanned inside fix_node... and unit_iset_process*/
				}while(!Parent::node_state_active.is_bit(node));							/* active nodes HAVE to be filtered here, could have been set to FALSE before*/
				Parent::color_unit_dyn_stack.erase();
				if ( (my_iset=Parent::fix_node_for_non_singleton_iset(node, chosen_iset)) !=Parent::NO_CONFLICT ||
					(my_iset= Parent::unit_iset_process_for_test_by_eliminating_failed_nodes() ) != Parent::NO_CONFLICT  ) {
						
					//	color_already_used[chosen_iset]=TRUE_VAL;					/*excludes current reduced color set because it will be included in the inference of previous level (1)*/
					//	lookback_for_maxsatz(my_iset);
					//	color_already_used[chosen_iset]=FALSE_VAL;
						Parent::reset_context_for_maxsatz(saved_node_stack_pt,
												saved_color_passive_stack_pt,
												save_color_reduced_stack_pt,
												save_color_unit_stack_pt );
					//	store_involved_sets();										/* chosen_iset will not be involved here (1) */
				}else{ /* NO CONFLICT */
					Parent::reset_context_for_maxsatz(saved_node_stack_pt,
											saved_color_passive_stack_pt,
											save_color_reduced_stack_pt,
											save_color_unit_stack_pt );
					break;
				}

				if(j== Parent::MAX_COLOR_FURTHER_TEST_LENGTH-1){							/* CONFLICT FOUND FOR ALL NODES- global conflict */
					//LOG_INFO("FURTHER TEST SUCCESFUL");
					return chosen_iset;
				}
			}
		}
	}

  return  Parent::NO_CONFLICT;
}


#endif