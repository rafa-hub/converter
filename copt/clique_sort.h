//clique_sort.h: header for CliqueSort implementation, a wrapper to order bit encoded undirected graphs by degree criteria
// date of creation in previous framework: 17/09/14  (authors: pablo san segundo, alvaro lopez)
// last update: 25/11/15
// author: pss

#ifndef __CLIQUE_SORT_H__
#define __CLIQUE_SORT_H__

#include "clique/clique_types.h"
#include "graph/graph.h"
#include "graph/algorithms/decode.h"
#include "graph/algorithms/graph_sort.h"
#include "utils/logger.h"
#include <iostream>
#include <algorithm>

#include <vector>

#include "init_color_sort.h"
#include "init_csp.h"
#include "init_color_ub.h"

typedef vector<int> vint;
using namespace gbbs;				//basic enum types for GRAPH sorting algorithms

//////////////////////////
//
// CliqueSort class
// (only for ugraph and sparse_ugraph)
//
////////////////////////////

template <class Graph_t>
class CliqueSort:public GraphSort<Graph_t>, public clqo{
private:
	static const double MIN_DENSITY_FOR_RLF;			//currently > 0.7
	static const double MIN_DENSITY_FOR_CSP_RLF;		//currently > 0.8 (CSP solved as MCP)		
	static const int MAX_SIZE_RLF_SORT= 20000;
	
public:
	typedef vector< pair<init_order_t, place_t> >				vpclq;
	typedef typename vpclq::iterator							vpclq_it;

	CliqueSort					(Graph_t& gout):GraphSort<Graph_t>(gout) {}
		
	vint new_order				(init_order_t, place_t=PLACE_LF, vint* v=NULL /*other info*/);

	//csp as MCP
	vint new_csp_order			(vint& csp_domains, bool& is_unsat,  int& nb_inc_val);				//only defined for ugraph
	vint new_csp_order_rlf		(vint& csp_domains, bool& is_unsat,  int& nb_inc_val);				//only defined for ugraph
	

	vint new_subg_order			(init_order_t, typename Graph_t::bb_type&,  place_t=PLACE_LF);		//not in [OLD_INDEX]=NEW_INDEX format!
	vint new_color_set_order	(init_order_t, vint& cset, place_t=PLACE_LF);						//reorders vertices inside each color set determined by cset (labels start at 1)


	vint new_mixed_order		(int param);	
	
	int reorder_composite		(vpclq&, Decode& d,  ostream* o = NULL);							//includes MAX_WEIGHT option
	int reorder_mixed_init		(int param, Decode& d);												//specific composite (see my Init Sorting paper)

	int eval_order				(vint& order);														//returns an evaluation of current vertex order
};

//////////////////
//static  members
template <class Graph_t>
const double CliqueSort<Graph_t>::MIN_DENSITY_FOR_RLF=0.7;

template <class Graph_t>
const double CliqueSort<Graph_t>::MIN_DENSITY_FOR_CSP_RLF=0.8;

///END STATIC /////////////////

template<>
inline
vint CliqueSort<ugraph>::new_csp_order_rlf (vint& csp_domains, bool& is_unsat, int& nb_inc_val){
//////////////////////////
// 
// date of creation: 02/03/17
// author: pss
//
// Computes a new ordering for constraint graphs derived from CSPs. 
// Computes the csp_domains related to RLF
//
// RETURNS: the new ordering in format [OLD_INDEX]= NEW_INDEX or EMPTY ARRAY in case of NONE or unknown alg
//
// COMMENTS: the sum of csp_domain_sizes does not have to be the number of vertices; possibly some arc-inconsistent
//			values have been removed
	
	vint res,  rlf_domains, tail_rlf_dec;
	InitCSP csp(g);
	if(csp.nb_csp_var()==-1) return res;
	
	//default values
	is_unsat=false;
	nb_inc_val=0;

	//computation of ordering
	int ret_val=csp.sort_rlf(res, rlf_domains,  tail_rlf_dec,  MIN_DENSITY_FOR_CSP_RLF);				

	//test for valid coloring
	if(ret_val>0){
		vint vempty;
		swap(res, vempty);
		return res;
	}else if(ret_val<0){																	/* trivially UNSAT */
		LOG_INFO("TRIVIALLY UNSAT determined by RLF");
		is_unsat=true;	
		vint vempty;
		swap(res, vempty);
		return res;
	}else{																					/* ret_val=0 , size of coloring exactly the number of variables*/
		
		if(res.size()!=g.number_of_vertices()) {
			LOG_ERROR("CliqueSort<ugraph>::new_csp_order_rlf_test -RLF-CSP ORDERING ERROR, incorrect size returned");
			is_unsat=true;	
			vint vempty;
			swap(res, vempty);
			return res;
		}else{
			//test if the tail of the RLF--coloring is inconsistent
			//RLF color partition is consistent (currently only unit colors that
			//make up the tail of the coloring are tested)
			//Note: NO FILTERING POWER AS YET
			

			LOG_INFO("TAIL OF COLORING DECODED");
			com::stl::print_collection(tail_rlf_dec);
			LOG_INFO("---------------------------");
		//	cin.get();

			//filter of tail of the coloring
			if(!csp.check_domains(tail_rlf_dec)){
				LOG_INFO("TAIL OF COLORING SATURATED");
				LOG_INFO("PROBLEM UNSAT");
				is_unsat=true;	
				vint vempty;
				swap(res, vempty);
				cin.get();
				return res;		
			}
		}
	}
		
	//update rlf_domains to real domains used by the algorithm
	csp_domains.clear();
	csp_domains.push_back(csp.nb_csp_var());
	copy(rlf_domains.begin(), rlf_domains.end(), insert_iterator<vint>(csp_domains, csp_domains.end()));

	LOG_INFO("-----------------------------");
	com::stl::print_collection(csp_domains);
	LOG_INFO("-----------------------------");

	return res;
}

template<>
inline
vint CliqueSort<ugraph>::new_csp_order (vint& csp_domains, bool& is_unsat, int& nb_inc_val){
//////////////////////////
// 
// date of creation:02/03/17
// author: pss
//
// Tests RLF ordering for CSP.  
//
// RETURNS: I.the new ordering in format [OLD_INDEX]= NEW_INDEX or empty array in case of NONE or unknown alg
//          II. Updates csp_domains from parsing CSP file (variables / values)
//
// COMMENTS: the sum of csp_domain_sizes does not have to be the number of vertices; possibly some arc-inconsistent
//			values have been removed
	
	vint res;
	InitCSP csp(g);
	if(csp.nb_csp_var()==-1) return res;

	//driver-computes color-db by ISEQ, determines ordering, variable domains etc.
	InitCSP::csp_t csp_ret=csp.run(res);
	
	//processes results
	is_unsat=false;
	nb_inc_val=csp.nb_inc_val();
	if(csp_ret==InitCSP::CSP_UNSAT){
		is_unsat=true;
		vint vempty;
		swap(res, vempty);
		
	}else if(csp_ret==InitCSP::CSP_ERROR){
		is_unsat=true;
		LOG_ERROR(" CliqueSort<Graph_t>::new_csp_order()-CSP GRAPH ERROR");
		vint vempty;
		swap(res, vempty);
		nb_inc_val=0;
	}else{	//CSP_OK-add values
		csp_domains.clear();
		const vint& dom=csp.get_domains();
		copy(dom.begin(), dom.end(), back_insert_iterator<vint>(csp_domains));
	
		//check
		if(res.size()!=g.number_of_vertices()) {
			LOG_ERROR(" CliqueSort<Graph_t>::new_csp_order()-CSP ORDERING ERROR, incorrect size returned");
			vint vempty;
			swap(res, vempty);
		}
	}
	
	return res;
}

template<class Graph_t>
inline
vint CliqueSort<Graph_t>::new_order (init_order_t  alg, place_t place, vint* p_info){
//////////////////////////
// 
// date of creation: 20/11/15
// author: pss
//
// Computes a new ordering of the vertices of the graph according to alg.
// Examines vertices sin natural order and places them sequentially according to 
// place.
//
// RETURNS: the new ordering in format [OLD_INDEX]= NEW_INDEX or empty array in case of NONE or unknown alg
			
	vint res(GraphSort<Graph_t>::g.number_of_vertices());
		
//////////////////////
// MIN_WIDTH_KCORE specific case
	//check correct place flag
	if(alg==MIN_WIDTH_KCORE){
		if(place==PLACE_FL) LOG_WARNING("CliqueSort<T>::new_order: ignoring incorrect place flag for MIN_WIDTH_KCORE ordering");
		return GraphSort<Graph_t>::new_order(KCORE, PLACE_LF);
	}
///////////////////////
// ALL REMAINING SORTING CRITERIA (excluding RLF variants-see ugraph specialization)
	switch(alg){
	case MIN_WIDTH:
		res=GraphSort<Graph_t>::new_order_fast(MIN_DEG_DEGEN,place);	
	//	res=GraphSort<Graph_t>::new_order(MIN_DEG_DEGEN,place);
		break;
	case MIN_WIDTH_BIG:
		if(place==gbbs::place_t::PLACE_LF)
			res=GraphSort<Graph_t>::new_order_fast_II(MIN_DEG_DEGEN);		    //20/07/17
		else res=GraphSort<Graph_t>::new_order_fast(MIN_DEG_DEGEN,place);		//20/07/17
		break;
	case MAX_WIDTH:
		res=GraphSort<Graph_t>::new_order(MAX_DEG_DEGEN,place);
		break;
	case MIN_WIDTH_MIN_TIE_STATIC:
		res=GraphSort<Graph_t>::new_order(MIN_DEG_DEGEN_TIE_STATIC,place);
		break;
	case clqo::NONE:
		res=GraphSort<Graph_t>::new_order(gbbs::NONE,place);
		break;
	default:
		LOG_ERROR("CliqueSort<T>::create_new_order: unknown ordering strategy");
		vint vempty;
		vempty.swap(res);
	}
	return res;
}

template<>
inline
vint CliqueSort<ugraph>::new_order (init_order_t  alg_in, place_t place, vint* vcol){
//////////////////////////
// 
// date of creation (refactored from earlier code): 22/11/15
// author: pss
//
// Specialization for RLF variants, for ugraph type only
// (includes degree based sorting as well)
//
// REMARKS: RLF based orderings used default MAXWIDTH in complement graph
//
	bool is_good=false;
	InitColorSort cs(g);
	init_order_t alg=alg_in;

//////////////////////
// MIN_WIDTH_KCORE specific case
	//check correct place flag
	if(alg==MIN_WIDTH_KCORE){
		if(place==PLACE_FL) LOG_WARNING("CliqueSort<T>::new_order: ignoring incorrect place flag for MIN_WIDTH_KCORE ordering");
		return GraphSort<ugraph>::new_order(KCORE, PLACE_LF);
	}
//////////////////////
	
	//Analysis of RLF variants: p>=0.7 , else switches to MIN_WIDTH_MIN_TIE_STATIC
	if((alg==RLF || alg==RLF_COND) && g.density()<MIN_DENSITY_FOR_RLF){
		/*stringstream sstr("");
		sstr<<"density below: "<<MIN_DENSITY_FOR_RLF<<" switching to MIN_WIDTH_MIN_TIE_STATIC";*/
		LOG_INFO("density below: "<<MIN_DENSITY_FOR_RLF<<" switching to MIN_WIDTH_MIN_TIE_STATIC");
		alg=MIN_WIDTH_MIN_TIE_STATIC;
	}

	//Size filter
	const int NV=g.number_of_vertices();
	if((alg==RLF_SORT_DOLL || alg==RLF_SORT_BBMC) && NV>MAX_SIZE_RLF_SORT){
		int nb_of_edges=g.number_of_edges(true);
		if(nb_of_edges>1000000){
			LOG_INFO("n<"<<MAX_SIZE_RLF_SORT<<" m>1000000"<<" -switching to NONE");
			alg=NONE;
			vcol->clear();
		}else{
			LOG_INFO("n<"<<MAX_SIZE_RLF_SORT<<" -switching to MIN_WIDTH_MIN_TIE_STATIC");
			alg=MIN_WIDTH_MIN_TIE_STATIC;
			vcol->clear();
		}
	}

///////////////////////
// ALL REMAINING SORTING CRITERIA

///////////
//variables for RLF_COND
	int eval_rlf, eval_rlf1, min_eval;
	int eval_degsort;
	vint degsort;
	vint res(NV);
	stringstream sstr("");
		
/////////////////////////////
	switch(alg){
	case RLF:
	//	cs.recursiveLargestFirst(res, is_good, clqo::MIN_WIDTH_MIN_TIE_STATIC);				/* uses BBMC type solvers, is_good not used, */		
		cs.recursiveLargestFirst_INC(res, is_good);											/* uses RDOLL type solvers, is_good not used, */	
		break;
	case RLF_SORT_DOLL:
	case RLF_SORT_BBMC:
		//(*vcol)=cs.recursiveLargestFirst(res, is_good, clqo::MIN_WIDTH_MIN_TIE_STATIC);		
		(*vcol)=cs.recursiveLargestFirst_INC(res, is_good);
		break;
	case RLF_COND:
		//cs.recursiveLargestFirst(res, is_good, clqo::MIN_WIDTH_MIN_TIE_STATIC);			/* uses BBMC type solvers, is_good not used, */		
		cs.recursiveLargestFirst_INC(res, is_good);											/* uses RDOLL type solvers, is_good not used, */	
		eval_rlf=eval_order(res);															//uses same evaluation as deg (better than size of coloring)
		
		//compute and evaluate degsort
		degsort=this->GraphSort<ugraph>::new_order(MIN_DEG_DEGEN_TIE_STATIC, PLACE_LF);
		eval_degsort=eval_order(degsort);
		sstr<<"RLF:"<<eval_rlf<<" DEG-BASED:"<<eval_degsort;
		LOG_INFO(sstr.str());
		
		if(eval_rlf>=eval_degsort || (!is_good && ((eval_degsort-eval_rlf)<InitColorSort::MAX_SIZE_TAIL_OF_COLORING)) ){   
			LOG_INFO("RLF not considered suitable: changing to MIN_WIDTH_MIN_TIE_STATIC");
			return degsort;
		}			
	
		break;
	case MIN_WIDTH:
		return(this->GraphSort<ugraph>::new_order_fast(MIN_DEG_DEGEN,place));
		//return(this->GraphSort<ugraph>::new_order(MIN_DEG_DEGEN,place));
		break;
	case MIN_WIDTH_BIG:
		if(place==gbbs::place_t::PLACE_LF)
			return(this->GraphSort<ugraph>::new_order_fast_II(MIN_DEG_DEGEN));					 //20/07/17-fast implementation
		else return(this->GraphSort<ugraph>::new_order_fast(MIN_DEG_DEGEN,place));
	case MAX_WIDTH:
		return(this->GraphSort<ugraph>::new_order(MAX_DEG_DEGEN,place));
	case MIN_WIDTH_MIN_TIE_STATIC:
		return(this->GraphSort<ugraph>::new_order(MIN_DEG_DEGEN_TIE_STATIC,place));
	case MAX_WIDTH_MAX_TIE_STATIC:
		return(this->GraphSort<ugraph>::new_order(MAX_DEG_DEGEN_TIE_STATIC,place));
	case MAX_WIDTH_MAX_TIE:		//on the fly computation of support
		return(this->GraphSort<ugraph>::new_order(MAX_DEG_DEGEN_TIE,place));
	case clqo::NONE:
		return(this->GraphSort<ugraph>::new_order(gbbs::NONE,place));
		break;

	default:
		LOG_ERROR("CliqueSort<ugraph>::new_order: unknown ordering strategy");
		vint vempty;
		return vempty;
		
	}

	return res;			//RLF variant orderings
}

template<class Graph_t>
inline
vint  CliqueSort<Graph_t>::new_subg_order (init_order_t alg, typename Graph_t::bb_type& sg,  place_t place){
////////////////
// A translator of enum types

	switch(alg){
	case MIN_WIDTH:
		return (this->GraphSort<Graph_t>::new_subg_order(MIN_DEG_DEGEN, sg, place));
	case MAX_WIDTH:
		return (this->GraphSort<Graph_t>::new_subg_order(MAX_DEG_DEGEN, sg, place));
	case MIN_WIDTH_MIN_TIE_STATIC:
		return (this->GraphSort<Graph_t>::new_subg_order(MIN_DEG_DEGEN_TIE_STATIC, sg, place));
	case clqo::NONE:
		return (this->GraphSort<Graph_t>::new_subg_order(gbbs::NONE, sg, place));


	default:
		LOG_ERROR("CliqueSort<T>::new_subg_order: unknown ordering strategy");
	}

	vint vempty;
	return vempty;
}


template<class Graph_t>
inline
vint  CliqueSort<Graph_t>::new_color_set_order (init_order_t alg, vint& cset, place_t place){
	struct this_template_should_not_be_created{};
	vint vempty;
	return vempty;
}

template<>
inline
vint  CliqueSort<ugraph>::new_color_set_order (init_order_t alg, vint& cset, place_t place){
/////////////////////////
// A color set specification: [x]=y || <x> is the color label, <y> the size of the color set Cx 
// first update: 29/9/16
//
// RETURNS: an ordering in the usual form O[OLD_INDEX]=NEW_INDEX or empty set if ERROR
//
//REMARKS: color labels MUST start at 1
	
	const int NV=g.number_of_vertices();
	bitarray bbfrom(NV);						//each color set
	bitarray bbref(NV);							//subgraph union of color sets

	//current initial order
	vint ord;
	for(int i=0; i<NV; i++){
		ord.push_back(i);
	}
		
	//main loop
	int start=0;
	for(int i=1; i<cset.size(); i++){		/* color labels start at 1*/
		int cs=cset[i];
		bbfrom.erase_bit();
		bbref.erase_bit();
		
		bbfrom.set_bit(start, start+cs-1);
		bbref.set_bit(0,start+cs-1);
		
		if(GraphSort<ugraph>::change_order(bbfrom, bbref, ord, 
							(alg==clqo::init_order_t::MIN_WIDTH)? gbbs::pick_t::PICK_MINFL : gbbs::pick_t::PICK_MAXFL,
								place, true)==-1){	
			vint myv;
			ord.swap(myv);		
			return ord;					//error
		}
		start+=cs;
	}

	return ord;							//ok
}


template<class Graph_t>
inline
int CliqueSort<Graph_t>::reorder_composite (vpclq& lord , Decode& d,  ostream* o ){
//////////////////
// iterates over the list of clique sorting algorithms and reorders the graph accordingly.
// Stores in decoder each change in format [NEW_INDEX]=OLD_INDEX for a later decoding
//
// RETURNS -1 if ERROR, 0 if OK

	for(vpclq_it it=lord.begin(); it!=lord.end(); it++){
		pair<init_order_t, place_t> ord=*it;

		//sort
		switch(ord.first){
		case MIN_WIDTH:
			GraphSort<Graph_t>::reorder(GraphSort<Graph_t>::new_order(MIN_DEG_DEGEN,ord.second),d,o);
			break;
		case MAX_WIDTH:
			GraphSort<Graph_t>::reorder(GraphSort<Graph_t>::new_order(MAX_DEG_DEGEN,ord.second),d,o);
			break;
		case MIN_WIDTH_MIN_TIE_STATIC:
			GraphSort<Graph_t>::reorder(GraphSort<Graph_t>::new_order(MIN_DEG_DEGEN_TIE_STATIC,ord.second),d,o);
			break;
		case clqo::NONE:
			GraphSort<Graph_t>::reorder(GraphSort<Graph_t>::new_order(gbbs::NONE,ord.second),d,o);
			break;

		//*** others 

		default:
			LOG_WARNING("CliqueSort<Graph_t>::reorder_composite: unknown ordering strategy. Current sorting discarded");
		}
	}
	
	return 0;		//OK
}

template<>
inline
int CliqueSort<ugraph>::reorder_mixed_init	(int param, Decode& d ){
///////////////////
// composite ordering
// 1-MIN_MIN_WIDTH_MIN_TIE_STATIC
// 2-ordering of a fraction (param) of initial vertices, non-degenerate
	
	GraphSort<ugraph>::reorder(new_order(MIN_WIDTH_MIN_TIE_STATIC,PLACE_LF), d);
	GraphSort<ugraph>::reorder(new_mixed_order(param),d);
			
	return 0;		//OK
}

template<>
inline
int CliqueSort<ugraph>::reorder_composite (vpclq& lord , Decode& d,  ostream* o ){
//////////////////
// iterates over the list of clique sorting algorithms and reorders the graph accordingly.
// Stores in decoder each change in format [NEW_INDEX]=OLD_INDEX for a later decoding
//
// RETURNS -1 if ERROR, 0 if OK

	d.clear();
	InitColorSort cs(g);
	vint order;
	bool is_good=false;

	for(vpclq_it it=lord.begin(); it!=lord.end(); it++){
		pair<init_order_t, place_t> ord=*it;

		//sort
		switch(ord.first){
		case MIN_WIDTH:
			GraphSort<ugraph>::reorder(GraphSort<ugraph>::new_order(MIN_DEG_DEGEN,ord.second),d,o);
			break;
		case MAX_WIDTH:
			GraphSort<ugraph>::reorder(GraphSort<ugraph>::new_order(MAX_DEG_DEGEN,ord.second),d,o);
			break;
		case MAX_ABS:
			GraphSort<ugraph>::reorder(GraphSort<ugraph>::new_order(MAX_DEG_ABS,ord.second),d,o);
			break;
		case MIN_WIDTH_MIN_TIE_STATIC:
			GraphSort<ugraph>::reorder(GraphSort<ugraph>::new_order(MIN_DEG_DEGEN_TIE_STATIC,ord.second),d,o);
			break;
		case MAX_WEIGHTED:
			GraphSort<ugraph>::reorder(GraphSort<ugraph>::new_order(MAX_WEIGHT,ord.second),d,o);
			break;
		case MIN_WEIGHTED:
			GraphSort<ugraph>::reorder(GraphSort<ugraph>::new_order(MIN_WEIGHT,ord.second),d,o);
			break;
		case MAX_WEIGHTED_DEG:
			GraphSort<ugraph>::reorder(GraphSort<ugraph>::new_order(MAX_WEIGHT_DEG,ord.second),d,o);
			break;
		case MIN_WEIGHTED_DEG:
			GraphSort<ugraph>::reorder(GraphSort<ugraph>::new_order(MIN_WEIGHT_DEG,ord.second),d,o);
			break;
		case clqo::NONE:
			GraphSort<ugraph>::reorder(GraphSort<ugraph>::new_order(gbbs::NONE,ord.second),d,o);
			break;
		case RLF:
			cs.recursiveLargestFirst(order,is_good);
			GraphSort<ugraph>::reorder(order,d,o);
			break;
		case RLF_COND:
			cs.recursiveLargestFirst(order,is_good);
			if(is_good)	GraphSort<ugraph>::reorder(order,d,o);
			else{
				LOG_INFO("Attempting RLF with NO ORDERING");
				cs.recursiveLargestFirst(order, is_good, clqo::NONE);
				if(is_good)	GraphSort<ugraph>::reorder(order,d,o);
				else{
					LOG_INFO("RLF not considered suitable: changing to MIN_WIDTH_MIN_TIE_STATIC");
					GraphSort<ugraph>::reorder(GraphSort<ugraph>::new_order(MIN_DEG_DEGEN_TIE_STATIC, ord.second),d,o);
				}
			}
			break;

		case MIXED_2:
			reorder_mixed_init(2,d);
			break;
		case MIXED_3:
			reorder_mixed_init(3,d);
			break;
		case MIXED_4:
			reorder_mixed_init(4,d);
			break;
		case MIXED_5:
			reorder_mixed_init(5,d);
			break;
		case MIXED_6:
			reorder_mixed_init(6,d);
			break;
		case MIXED_7:
			reorder_mixed_init(7,d);
			break;
		case MIXED_8:
			reorder_mixed_init(8,d);
			break;
		case MIXED_9:
			reorder_mixed_init(9,d);
			break;
		case MIXED_10:
			reorder_mixed_init(10,d);
			break;
	
		default:
			LOG_WARNING("CliqueSort<Graph_t>::reorder_composite: unknown ordering strategy. Current sorting discarded");
		}
	}
	
	return 0;		//OK
}


template<class Graph_t>
inline
vint CliqueSort<Graph_t>::new_mixed_order(int param){
///////////////////////
// Reordering of a fraction of the initial vertices wrt to non-degenerate properties
// in V 
// PARAMS: param ranges [2 (half the vertices of V) to 10 (a tenth of the vertices of V)]

	int NV=GraphSort<Graph_t>::g.number_of_vertices();
	bitarray sg(NV);
	bitarray sgV(NV);
	int nVsort=max<int>(1,GraphSort<Graph_t>::g.number_of_vertices()/param);
	sg.set_bit(0, nVsort);
	sgV.set_bit(0, NV-1);

	return (GraphSort<Graph_t>::new_order(sg, sgV, PICK_MAXFL, PLACE_FL, false));
}

template<class Graph_t>
inline
int CliqueSort<Graph_t>::eval_order (vint& order){
///////////////////
// Evaluates an initial order of vertices for clique based  on
// the maximum infra-chrom bound of all oriented subproblems
// date: 9/12/15
//
// REMARKS: Could also use the biggest subproblem (hanging from the last vertex)	

	Graph_t g1(GraphSort<Graph_t>::g);

	//reorders
	CliqueSort cs(g1);
	if(cs.reorder(order)==-1){
		LOG_ERROR("CliqueSort<ugraph>::eval_order()-Impossible to evaluate the ordering");
		return -1;
	}

	//evaluates the ordering according to infra-chrom bounds for all subproblems
	InitColorUB cub(g1);
	return(cub.eval_init_order());
}


#endif