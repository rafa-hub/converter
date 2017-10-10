//setup.h: header for SETUP class which computes a default SETUP for SELECTIVE BOUNDING STRATEGIES
//			1. MIN_WIDTH ordering
//			2. lower bound based on the ordering
//          3. V set partitionining into A | B where B is the branching set at the root node

//date of creation: 28/09/17
//last update: 28/09/17
//author: pablo san segundo

#ifndef __CLIQUE_SETUP_H__
#define __CLIQUE_SETUP_H__

#include "init_color.h"
#include "graph/graph.h"
#include "utils/logger.h"
#include "utils/common.h"
#include "bitscan/bbalg.h"
#include <vector>
#include <algorithm>
#include "clique/clique.h"
#include "graph/algorithms/graph_map.h"

//#include "clique/infra_tools.h"
//#include "clique/infra_tools_plus.h"

using namespace std;

typedef  vector<bitarray* > v_bb;
typedef vector<int> vint;

///////////////////////////
//
// SETUP class (preconditioning selective coloring)
// Note: input algorithm MUST be already allocated
//
////////////////////////////

template<class clq_alg_t>
class SETUP{
	typedef typename clq_alg_t::Clique::g_type::bb_type bb_t;
	typedef typename clq_alg_t::Clique::g_type g_t;

public:	
	enum sort_t {NONE=0, MIN_WIDTH, MIN_WIDTH_BIG};

	//stores the sorting as a l->r mapping
static	int init_sort (g_t& g, g_t& g_new , Decode& d, GraphMapSingle& gm, sort_t = MIN_WIDTH);		
	
//////////////////////
// public interface

	SETUP			(clq_alg_t& cq_out):cq(cq_out)	{}
	virtual	~SETUP	()								{;}
		
	int init_sort	(sort_t = MIN_WIDTH);											//reorders graph, caution!
	int init_sort	(GraphMapSingle& gm,sort_t = MIN_WIDTH);						//stores the sorting as a l->r mapping
	

	int lower_bound (vint& vset);									//feasible solution according to ordering
	int lower_bound (bb_t& bb_forbidden, vint& vset);				//constraint: forbidden set of nodes
	int partitioning (int LB);										//Partitions V in A U B, B=branching set, according to lb	
	int partitioning (int LB, bb_t& bbsg);							//Partitions subgraph bbsg in A U B, B=branching set, according to lb	

/////////
//data members
	clq_alg_t& cq;
};


template<class clq_alg_t>
inline
int SETUP<clq_alg_t>::init_sort (g_t& g, g_t& g_new , Decode& d, GraphMapSingle& gm, sort_t stype){
///////////////
// init_sort for external use-contains all in-out information
//
// **TODO -test

	GraphSort<g_t> gs(g);
	vint o2n ;
	switch(stype){
	case SETUP::MIN_WIDTH:																/* for normal graphs */	
		o2n=gs.new_order_fast(gbbs::MIN_DEG_DEGEN,gbbs::PLACE_LF);
		gm.build_mapping(o2n, "MIN_WIDTH");
		gs.reorder(o2n,g_new, d);		
		break;
	case SETUP::MIN_WIDTH_BIG:															/* for big graphs */
		o2n=gs.new_order_fast_II(gbbs::MIN_DEG_DEGEN);
		gm.build_mapping(o2n, "MIN_WIDTH_BIG");
		gs.reorder(o2n,g_new,d);	
		break;
	case SETUP::NONE:
		break;
	default:
		LOG_ERROR("SETUP<clq_alg_t>::init_sort(...)-unknown sorting algorithm");
		return -1;
	}		
	return 0;
}


template<class clq_alg_t>
inline
int SETUP<clq_alg_t>::init_sort	(sort_t stype){
///////////////
// reorders graph!

	GraphSort<g_t> gs(*cq.get_graph());
	
	switch(stype){
	case SETUP::MIN_WIDTH:																/* for normal graphs */	
		gs.reorder(gs.new_order_fast(gbbs::MIN_DEG_DEGEN,gbbs::PLACE_LF),cq.get_decoder());				
		break;
	case SETUP::MIN_WIDTH_BIG:															/* for big graphs */
		gs.reorder(gs.new_order_fast_II(gbbs::MIN_DEG_DEGEN),cq.get_decoder());					
		break;
	case SETUP::NONE:
		break;
	default:
		LOG_ERROR("SETUP<clq_alg_t>::init_sort()-unknown sorting algorithm");
		return -1;
	}		
	return 0;
}

template<class clq_alg_t>
inline
int SETUP<clq_alg_t>::init_sort	(GraphMapSingle& gm, sort_t stype){
///////////////
// extends init_sort to store sorting as a l->r mapping

	GraphSort<g_t> gs(*cq.get_graph());
	vint o2n;
	
	switch(stype){
	case SETUP::MIN_WIDTH:																/* for normal graphs */	
		o2n=gs.new_order_fast(gbbs::MIN_DEG_DEGEN,gbbs::PLACE_LF);
		gm.build_mapping(o2n, "MIN_WIDTH");
		gs.reorder(o2n,cq.get_decoder());				
		break;
	case SETUP::MIN_WIDTH_BIG:															/* for big graphs */
		o2n=gs.new_order_fast_II(gbbs::MIN_DEG_DEGEN);
		gm.build_mapping(o2n, "MIN_WIDTH_BIG");
		gs.reorder(o2n,cq.get_decoder());				
		break;
	case SETUP::NONE:
		break;
	default:
		LOG_ERROR("SETUP<clq_alg_t>::init_sort()-unknown sorting algorithm");
		return -1;
	}		
	return 0;
}


template<class clq_alg_t>
inline
int SETUP<clq_alg_t>::lower_bound(vint& vset){	
/////////////////////
// simple bound based on:
// 1.clique in the FIRST nodes of a MIN_WIDTH ordering (note vertices are placed last)
// 
// RETURNS size of clique
	
	vset.clear();
	g_t* g=cq.get_graph();
	int NV=g->number_of_vertices();
	bb_t bb(NV);
		
	bb.set_bit(0);
	int lb=1;
	vset.push_back(0);
	
	//computes size of sequential clique starting from last node
	for(int v=1; v<NV; v++){
		if(g->degree(v, bb)==lb){
			lb++;
			bb.set_bit(v);
			vset.push_back(v);
		}else  break;		
	}
	
	//update solution and search params
	return lb;
}

template<class clq_alg_t>
inline
int SETUP<clq_alg_t>::lower_bound(bb_t& bb_forbidden, vint& vset){	
/////////////////////
// simple lower bound with additional constraint a forbidden set of nodes:
// 1.clique in the FIRST nodes of a MIN_WIDTH ordering (note vertices are placed last)
// 2.does not consider any forbidden nodes in bb_forbidden
// 
// RETURNS size of clique
	
	vset.clear();
	g_t* g=cq.get_graph();
	int NV=g->number_of_vertices();
	bb_t bb(NV);
	
	//determine first non-forbidden node
	int vf;
	for(vf=0; vf<NV; vf++){
		if(!bb_forbidden.is_bit(vf)){
			break;
		}
	}
	//all nodes forbidden-special case
	if(vf>=NV){
		LOG_INFO("SETUP<clq_alg_t>::lower_bound-All nodes forbidden");
		return 0;
	}
	
	bb.set_bit(vf);
	int lb=1;
	vset.push_back(vf);
	
	//computes size of sequential clique starting from last node
	for(int v=vf+1; v<NV; v++){
		if(bb_forbidden.is_bit(v)) continue;
		if(g->degree(v, bb)==lb){
			lb++;
			bb.set_bit(v);
			vset.push_back(v);
		}else  break;		
	}
	
	//update solution and search params
	return lb;
}

template<class clq_alg_t>
inline
int SETUP<clq_alg_t>::partitioning (int LB){
//////////////
// Computes partition of V as candidate list of nodes l_root and bitsring V (bbroot) to be used by algorithm cq directly 
//rdiction
// RETURNS  the number of candidates to be expanded
	
	cq.Clique<g_t>::get_root_bitstring().set_bit(0, cq.get_graph()->number_of_vertices()-1);
	return cq.partitioning(LB, cq.Clique<g_t>::get_root_bitstring(), cq.Clique<g_t>::get_root_nodelist());
}

template<class clq_alg_t>
inline
int SETUP<clq_alg_t>::partitioning (int LB, bb_t& bbsg){
//////////////
// Computes partition of subgraph bbsg as candidate list of nodes l_root and bitsring V (bbroot) to be used by algorithm cq directly 
//
// RETURNS  the number of candidates to be expanded
	
	cq.Clique<g_t>::get_root_bitstring()=bbsg;
	return cq.partitioning(LB, cq.Clique<g_t>::get_root_bitstring(), cq.Clique<g_t>::get_root_nodelist());
	
}


#endif 

