////////////////////////////////
// filter_csp_plus.h: interface for FilterCSP_Plus class, which filters candidates for
//					  a CSP problem during search when solved as an MCP
//                    To be used by CliqueCSP_Plus class
//
// initial date:5/06/17
// last update:5/06/17
// author: pablo san segundo

#ifndef  __FILTER_CSP_PLUS_H__
#define  __FILTER_CSP_PLUS_H__

#include "graph/algorithms/graph_sort.h"
#include "../common/common_clq.h"							/*for range_t */
#include <vector>

using namespace com;	
using namespace comclq;

typedef vector<int> vint;

class FilterCSP_Plus{
///////////////////////////
// For a given ordering of vertices which corresponds to a feasible NUM_VAR-coloring C
// of the full CSP constraint graph G=(V,E), this class attempts to prune nodes (values)
// of any CSP subproblem (node) by applying directional arc-consistency using classical SEQ
//
// Note: ISEQ should not be used here, since vertices in C are not ordered according to V
//	


	enum  ret_t	{PRUNE=-1, OK, ERR};
	static int compare_domains(vint& dom_lhs, vint& dom_rhs);	
    static const int MIN_DIFF_BETWEEN_DOMAINS=10;
public:
	enum  ref_t {CSP_BASE=0, RLF_BASE};						

protected:
	ugraph* g_ori;											/* the original constraint graph */
	ugraph* g_ref;											/* graph sorted according to ord */
	vint DOMAINS_REF;											/* [VAR] sizes of variable DOMAINS_REF, also a feasible coloring of g */	
	vector<comclq::range_t> RANGES;							/* [NODES] ranges [vlow, vhigh] of its root color class (also range of variable domain) */
	bitarray m_unsel;
	bitarray m_sel;
	Decode d_from_ref;										/* REF 2 OUTER */
	Decode d_to_ref;										/* OUTER 2 REF */
	ref_t m_mode_base;										/* CSP or RLF the reference graph */

////////
//state computed in init()
	vint  o_2_n_ref;										/* orderings */
	vint  o_2_n_ext;							
	bool m_is_unsat;	
	bool m_filter_active;
	vint DOMAINS_EXT;	

//////////////////
// data filtering
	ret_t filter_lh(int depth, int kmin, bitarray& bbsg_n, vint& nodes);				/* nodes(values) filtered from subgraph bbsg when directional arc_consistency low to high is applied */		
	ret_t filter_hl(int depth, int kmin, bitarray& bbsg_n, vint& nodes);				/* nodes(values) filtered from subgraph bbsg when directional arc_consistency high to low is applied */	

/////////////////
// init functions
	
	void clear();
	void reset();
	int init_ranges();
	
/////////////////
//mappings
	void ref_2_out(vint& l){d_from_ref.decode_list_in_place(l);}
	void out_2_ref(bitarray& bbsg_out, bitarray& bbsg_ref);
		
public:
//	FilterCSP_Plus(ugraph* g_ref, vint& ord, vint& domains):g(*g_ref), ord(ord), DOMAINS(domains){ init();}
	FilterCSP_Plus(ugraph* g_out=NULL, ref_t mode=CSP_BASE):g_ori(g_out),g_ref(NULL), m_mode_base(mode), m_filter_active(false), m_is_unsat(false){}
	~FilterCSP_Plus(){clear();}
	int init();														/* full init-computes both orderings */
	int init_ref();													/* lean init for reference ordering */
	int init_ext();													/* lean init for external ordering */
	
	void set_graph(ugraph* g_ori){this->g_ori=g_ori;}
	void set_mode(ref_t mode){m_mode_base=mode; reset(); }
	const vint& get_ext_ordering() const {return o_2_n_ext;}		/* [OLD]-->[NEW]*/		
	const vint& get_ref_ordering() const {return o_2_n_ref;}		/* [OLD]-->[NEW]*/	
	const vint& get_rlf_ordering() const;
	const vint& get_csp_ordering() const;
	const vint& get_ext_domains() const {return DOMAINS_EXT;}
	const vint& get_rlf_domains() const;
	const vint& get_csp_domains() const;
	int number_of_var();
	bool is_filter_active(){return m_filter_active;}

	bool is_rlf_ord_valid();	
	bool is_csp_unsat(){return m_is_unsat;}		

///////////
// drivers
	int filter_driver(int depth, int kmin, bitarray& bbsg_o, vint& node_o, bool l2h=true);	

///////////////
// I/O
	ostream& print_nodes(vint& ln,  ostream& = cout);
};

inline
int FilterCSP_Plus::init_ref(){
//////////////
// light init for reference configuration (CSP default)
//
// COMMENTS: the state becomes useless
			
	clear();				/* deallocates g_ref */
	reset();				/* clears all data structures */
	if(g_ori==NULL) {
		LOG_INFO("FilterCSP_Plus::init()-Error in input graph");
		return -1;
	}
	
	/* dummies for sorting functions */
	bool m_is_unsat=false;
	int nb_inc_val=0;		
	
////////////////
// Determines orderings for external and reference graphs
	CliqueSort<ugraph> cs_ref(*g_ori);
	switch(m_mode_base){
	case CSP_BASE:																	/* default mode */
		o_2_n_ref=cs_ref.new_csp_order(DOMAINS_REF, m_is_unsat, nb_inc_val);
		break;
	case RLF_BASE:
		o_2_n_ref=cs_ref.new_csp_order_rlf(DOMAINS_REF, m_is_unsat, nb_inc_val);
		break;
	default:
		LOG_ERROR("FilterCSP_Plus::out_2_ref()-incorrect mode");
	}
		
	// I/O: TRIVIAL SOLUTIONS, INVALID ORDERING //
	if(m_is_unsat){
		LOG_INFO("FilterCSP_Plus::init_ext()-TRIVAL SOL FOUND");
	}else if (o_2_n_ref.empty()){
		//LOG_INFO("FilterCSP_Plus::init_ext()-ord. with excessive nb of variables or bizarre");	
	}
	return 0;
}

inline
int FilterCSP_Plus::init_ext(){
//////////////
// light init for external configuration (RLF default)
//
// COMMENTS: the state becomes useless
			
	clear();				/* deallocates g_ref */
	reset();				/* clears all data structures */
	if(g_ori==NULL) {
		LOG_INFO("FilterCSP_Plus::init()-Error in input graph");
		return -1;
	}
	
	/* dummies for sorting functions */
	bool m_is_unsat=false;
	int nb_inc_val=0;		
	
////////////////
// Determines orderings for external and reference graphs
	CliqueSort<ugraph> cs_ext(*g_ori);
	switch(m_mode_base){	
	case RLF_BASE:																	/* default mode */
		o_2_n_ext=cs_ext.new_csp_order(DOMAINS_EXT, m_is_unsat, nb_inc_val);
		break;
	case CSP_BASE:
		o_2_n_ext=cs_ext.new_csp_order_rlf(DOMAINS_EXT, m_is_unsat, nb_inc_val);
		break;
	default:
		LOG_ERROR("FilterCSP_Plus::out_2_ref()-incorrect mode");
	}

	// I/O: TRIVIAL SOLUTIONS, INVALID ORDERING //
	if(m_is_unsat){
		LOG_INFO("FilterCSP_Plus::init_ext()-TRIVAL SOL FOUND");
	} else if(o_2_n_ext.empty()){
		//	LOG_INFO("FilterCSP_Plus::init_ext()-ord. with excessive nb of variables or bizarre");	
	}

			
	return 0;
}

inline
int FilterCSP_Plus::number_of_var(){
////////////////////
// Returns number of variables, or -1 if ERR  

	if(!DOMAINS_REF.empty()) return DOMAINS_REF.front();
	else if (!DOMAINS_EXT.empty()) return DOMAINS_EXT.front();

	return -1;
}
	
inline
const vint& FilterCSP_Plus::get_rlf_domains() const{
	if(m_mode_base==RLF_BASE){
		return DOMAINS_REF;
	}else if(m_mode_base==CSP_BASE){
		return DOMAINS_EXT;
	}
}

inline
const vint& FilterCSP_Plus::get_csp_domains() const{
	if(m_mode_base==RLF_BASE){
		return DOMAINS_EXT;
	}else if(m_mode_base==CSP_BASE){
		return DOMAINS_REF;
	}
}

inline
const vint& FilterCSP_Plus::get_rlf_ordering() const{
	if(m_mode_base==RLF_BASE){
		return o_2_n_ref;
	}else if(m_mode_base==CSP_BASE){
		return o_2_n_ext;
	}
}

inline
const vint& FilterCSP_Plus::get_csp_ordering() const{
	if(m_mode_base==CSP_BASE){
		return o_2_n_ref;
	}else if(m_mode_base==RLF_BASE){
		return o_2_n_ext;
	}
}

inline
bool FilterCSP_Plus::is_rlf_ord_valid(){
	if(m_mode_base==RLF_BASE){
		if(o_2_n_ref.empty()) return false;
	}else if(m_mode_base==CSP_BASE){
		if(o_2_n_ext.empty()) return false;
	}
	
	return true;
}

inline
void FilterCSP_Plus::reset(){
//////////////
// resets state computed by init()
	o_2_n_ref.clear();										
	o_2_n_ext.clear();	
	DOMAINS_EXT.clear();
	DOMAINS_REF.clear();
	m_is_unsat=false;	
	m_filter_active=false;
}

inline 
int FilterCSP_Plus::compare_domains(vint& dom_lhs, vint& dom_rhs){
////////////////////////////////
// Computes the difference in domain sizes for each domain (color set), 
// -1 if NB_VAR are different
	
	//assert
	if(dom_lhs.empty() || dom_rhs.empty()){
		LOG_INFO("FilterCSP_Plus::compare_domains()-bizarre domains at least one EMPTY");
		return -1;
	}
	if(dom_lhs.front()!=dom_rhs.front()){
		LOG_INFO("FilterCSP_Plus::compare_domains()-bizarre domains with diff nb of var");
		return -1;
	}

	int res=0;
	for(int i=1; i<dom_lhs.size(); i++){		/* i=0 is NB_VAR */
		res+=abs(dom_lhs[i]-dom_rhs[i]);
	}

	return res;
}


inline
void FilterCSP_Plus::clear(){
	if(g_ref) delete g_ref;
	g_ref=NULL;
}

inline
int FilterCSP_Plus::filter_driver( int depth, int kmin, bitarray& bbsg_o, vint& ln_out, bool l2h){
////////////////////////
// actual driver: receives subgraph to filter (in ref base)
// computes the nodes to be removed in the external graph
//
// date_of_creation: 11/6/17
//
// RETURNS: number of filtered nodes or -1 in case of PRUNING / ERROR
	
	
	out_2_ref(bbsg_o,m_unsel);
	ret_t res=OK;
	if(l2h)
		res=filter_lh(depth, kmin, m_unsel, ln_out);
	else{
		res=filter_hl(depth, kmin, m_unsel, ln_out);
	}

	/* TODO: REASON WITH RES */
	if(res==PRUNE) return -1;
	else if(res==ERR){
		LOG_ERROR("FilterCSP_Plus::filter_driver_LH-ERROR");
		return -1;
	}

	//change reference of pruned nodes
	ref_2_out(ln_out);	
	
	return ln_out.size();
}


inline
void FilterCSP_Plus::out_2_ref(bitarray& bbsg_out, bitarray& bbsg_ref){
///////////////
// decoded subgraph bbsg_out to reference isomorphism

	bbsg_ref.erase_bit();
	bbsg_out.init_scan(bbo::NON_DESTRUCTIVE);
	while(true){
		int v=bbsg_out.next_bit();
		if(v==EMPTY_ELEM) break;

		bbsg_ref.set_bit(d_to_ref.decode_node(v));
	}
}

inline
int FilterCSP_Plus::init(){
/////////////////
// orders graph according to ord and sets decoder 
//
// RETURNS comparison metric between domains(diff in size of domains), -1 ERROR (also bizarre domains)
//         0 for UNSAT                   
//
// COMMENTS: Done only once at the beginning of the search 
//       	(thus not optimized specifically)
		
	clear(); reset();
	if(g_ori==NULL) {
		LOG_INFO("FilterCSP_Plus::init()-Error in input graph");
		return -1;
	}
	g_ref=new ugraph(*g_ori);
	int NV=g_ref->number_of_vertices();
	int nb_inc_val=0;																/* currently not used */	
	vint n_2_o_ref, n_2_o_ext;
	
////////////////
// Determines orderings for external and reference graphs
	CliqueSort<ugraph> cs_ext(*g_ori);
	CliqueSort<ugraph> cs_ref(*g_ori);
	switch(m_mode_base){
	case CSP_BASE:																	/* default mode */
		DOMAINS_EXT.clear();
		o_2_n_ext=cs_ext.new_csp_order_rlf(DOMAINS_EXT, m_is_unsat, nb_inc_val);
		if(m_is_unsat) break;
		DOMAINS_REF.clear();
		o_2_n_ref=cs_ref.new_csp_order(DOMAINS_REF, m_is_unsat, nb_inc_val);
		break;
	case RLF_BASE:
		DOMAINS_EXT.clear();
		o_2_n_ext=cs_ext.new_csp_order(DOMAINS_EXT, m_is_unsat, nb_inc_val);
		if(m_is_unsat) break;
		DOMAINS_REF.clear();
		o_2_n_ref=cs_ref.new_csp_order_rlf(DOMAINS_REF, m_is_unsat, nb_inc_val);
		break;
	default:
		LOG_ERROR("FilterCSP_Plus::out_2_ref()-incorrect mode");
	}
		
	// Reasons with results to stop the procedure in case of TRIVIAL SOLUTIONS //
	if(m_is_unsat || o_2_n_ref.empty() ||  o_2_n_ext.empty()){
		//I/O
		if(m_is_unsat)
			LOG_INFO("FilterCSP_Plus::init()-TRIVAL SOL FOUND");
		else{
			if(o_2_n_ref.empty())
				LOG_INFO("FilterCSP_Plus::init()-ref ordering empty");
			if(o_2_n_ext.empty())
				LOG_INFO("FilterCSP_Plus::init()-external ordering empty");
		}
	//	cin.get();
		return 0;
	}
		
	//sorts (finally) the reference graph
	CliqueSort<ugraph> cs_sort(*g_ref);
	cs_sort.reorder(o_2_n_ref);				

/////////////////
// update decoders with correct mapping

	n_2_o_ext=o_2_n_ext;
	Decode::reverse_in_place(n_2_o_ext);
	n_2_o_ref=o_2_n_ref;
	Decode::reverse_in_place(n_2_o_ref);

	d_to_ref.clear();
	d_from_ref.clear();
	d_from_ref.insert_ordering(o_2_n_ext);
	d_from_ref.insert_ordering(n_2_o_ref);
	d_to_ref.insert_ordering(o_2_n_ref);
	d_to_ref.insert_ordering(n_2_o_ext);

//////////////////
// allocation, other setters
	m_unsel.init(NV);
	m_sel.init(NV);

	//ranges
	init_ranges();

/////////////////
// activates filter conditioned to difference in colorings
	int res=FilterCSP_Plus::compare_domains(DOMAINS_EXT, DOMAINS_REF);
	
	/*LOG_INFO("[nV:"<<DOMAINS[0]<<", diff:"<<res<<"]");
	 cin.get(); */
	if(res>MIN_DIFF_BETWEEN_DOMAINS){
	 	LOG_INFO("ACTIVATES FILTER CSP-[nV:"<<DOMAINS_REF[0]<<", diff:"<<res<<"]");
		m_filter_active=true;
		cin.get();
	}
	
	return res;
};

inline 
int FilterCSP_Plus::init_ranges(){
/////////////
// Important: This code has been repeated elsewhere: best to use a general purpose function
// from a CSP namespace
//
// Sets RANGES from DOMAINS info (RANGES[NODE]=[vl, vh] where vl and vh mark the vertex limits of 
// the color set defined in DOMAINS)

	int v_first=0, v_last;
	RANGES.assign(g_ref->number_of_vertices(), comclq::range_t());
	for(vint::iterator it=DOMAINS_REF.begin()+1; it!=DOMAINS_REF.end(); it++){
		v_last=v_first+*it-1;
		for(int v=v_first; v<=v_last; v++){
			RANGES[v].vl=v_first;
			RANGES[v].vh=v_last;
		}
		v_first=v_last+1;
	}

	return 0;
}

inline 
FilterCSP_Plus::ret_t FilterCSP_Plus::filter_lh(int depth, int kmin, bitarray& bbsg /* m_unsel */, vint& fnodes){
///////////////////////////
// A-Computes set of filtered nodes (fnodes) in the subgraph bbsg according to
//   ISEQ coloring (increasing color set number) 
// B-Computes nodes to remove from subgraph
// 
// COMMENTS: This is the same computation as i_seq in CliqueCSP_Plus, but without using 
//           the complex data structures of infra-chrom
//
// RETURNS enum ret_t {-1 PRUNE, 0 ok, 1 ERROR} and nodes to remove from the inner graph in fnodes
		
	int cmax=1, nBB=EMPTY_ELEM, v=EMPTY_ELEM, offset=0;
	bool first_color=true;
	fnodes.clear();

	//*** KMIN CHECK
		
	int pc=bbsg.popcn64();
	if(pc==0){
		LOG_ERROR("EMPTY GRAPH ISEQ_NON_INCREMENTAL");
		return PRUNE;														/*empty subgraph-should not ocurr */	
	}
	
/////////////////////////////////////////////
//color first kmin nodes as usual
		
	while(true){ 
		m_sel=bbsg;
		first_color=true;
		m_sel.init_scan(bbo::NON_DESTRUCTIVE);
		while(true){
			v=m_sel.next_bit(nBB,bbsg);
			if(v==EMPTY_ELEM)
				break;
					
			//color range analysis: removes values outside of range
			if(first_color){
				first_color=false;
				offset=RANGES[v].vh + 1;	
			}else if(v>=offset){ // value removed
				//bbsg.erase_bit(v);
				fnodes.push_back(v);
				if((--pc)==0){
					//LOG_INFO("CMAX: "<<cmax<<" KMIN: "<<kmin);
					//cin.get();
					return PRUNE;	/* should exit here */
				}
				else continue;		/* next vertex to be pruned */
			}
									
			//checks exit condition
			if((--pc)==0){
				if(cmax<=kmin /* CHECK */){
					//LOG_INFO("CMAX: "<<cmax<<" KMIN: "<<kmin);
					return PRUNE;
				}else{
					return OK;											/* all nodes colored with <KMIN colors */
				}
			}

			//actual painting routine
			m_sel.erase_bit(g_ref->get_neighbors(v));
		}/* next vertex of current color */

		cmax++;
	}/* next color */

	LOG_ERROR("CliqueCSP_Plus::iseq-should not reach here");
	return ERR;															/* should not reach here */
}

inline 
FilterCSP_Plus::ret_t FilterCSP_Plus::filter_hl(int depth, int kmin, bitarray& bbsg /* m_unsel */, vint& fnodes){
///////////////////////////
// A-Computes set of filtered nodes (fnodes) in the subgraph bbsg according to
//   reverse ISEQ coloring (decreasing color set number) 
// B-Computes nodes to remove from subgraph
// 
// COMMENTS: This is the same computation as i_seq in CliqueCSP_Plus, but without using 
//           the complex data structures of infra-chrom
//
// RETURNS enum ret_t {-1 PRUNE, 0 ok, 1 ERROR} nd nodes to remove from the inner graph in fnodes

	
	bool first_color_vertex=true;
	int col=1, /*kmin=maxno-maxac,*/ nBB=EMPTY_ELEM, v=EMPTY_ELEM, v_col_ref=EMPTY_ELEM, offset=0 ;		
	fnodes.clear();

	int pc=bbsg.popcn64();
	while(true){ 
		m_sel=bbsg;
		m_sel.init_scan(bbo::DESTRUCTIVE_REVERSE);
		first_color_vertex=true;
		while(true){
			if( (v=m_sel.previous_bit_del(nBB,bbsg)) == EMPTY_ELEM) break;
			if(first_color_vertex){
				first_color_vertex=false;
				v_col_ref=v;
				offset=RANGES[v_col_ref].vl-1;
			}else if(v<=offset){
				//bbsg.erase_bit(v);
				fnodes.push_back(v);
								
				if((--pc)==0){													
					return PRUNE;
				}
				continue;
			}
			//normal exit
			if((--pc)==0){
				if(col<=kmin){										
					return PRUNE;
				}else{
					return OK;
				}
			}
			//color in reverse direction
			m_sel.erase_block(0,nBB,g_ref->get_neighbors(v));
		} /* next v */	
		col++;
	} /* next color */

	return ERR;					//should not reach here
}

#endif