////////////////////////////////
// clique_weighted_plus.h: interface for the CliqueWeightedPlus class which contains an exact MWCP algorithm
//						   based on the unweighted algorithm BBMCSAT (currently sent to EJOR-4/07/17)
//
// initial date:04/07/17
// last update: 04/07/17
// author: pablo san segundo

#ifndef  __CLIQUE_WEIGHTED_PLUS_H__
#define  __CLIQUE_WEIGHTED_PLUS_H__

#include "clique.h"
#include "../init_color_ub.h"
#include "../init_color_ub_weighted.h"
#include "../amts/amts_exec.h"
#include "bitscan/bbalg.h"
#include "infra_tools_plus.h"
#include "../common/common_macros.h"
#include <numeric>													// std::accumulate
#include "graph/algorithms/graph_map.h"
#include "../clique/heur/ub_weighted_clique.h"
#include "../clique/heur/super_weight.h"
#include "utils/file.h"

using namespace com;												//for common types (here bb_t)

template class KCore<ugraph>;
typedef vector<int> vint;

///////////////////
//switches
#define REORDER_REVERSE_NON_INC_WIDTH							/* reorders original graph before pre-processing (DEFAULT-ON, hamming degenerates) */
#define OVERLAP_ACCORDING_TO_WEIGHT								/* computes overlap according to non-increasing-weights of nodes (DEFAULT-ON) */
//#define DOUBLE_ROUND_FILTER									/* 2 passes of candidate nodes (DEFAULT-OFF)*/
//#define SUPER_WEIGHT_RANDOM									/* RANDOM / DIMACS switch (activate depending on the type of instances)*/
//#define RUSSIAN_DOLL											/* SETUP RDOLL switch (only for RD algorithms) */
/////////////////////

class CliqueWeightedPlus: public Clique<ugraph>{
////////////////////////
// data members	
	static const int MIN_CAND_SIZE_SUPER = 3;			/* mainly for random, very dense, graphs*/
	enum top_t {NONE=0, TOP1, TOP2, GAP};
public:

//protected:
	InfraOpPlus<ugraph,bitarray> iop;
	ugraph *gw;											/* weighted auxiliary graph for bounding (attempt at double ordering, 1-bounding 2-enum)*/		
	int** m_tw;											/* [COL][k=0,1] top-k+1 weights for each clause (MWSS UB-30/7/17): alloc in init_color_sets()  */
	GraphMap m_gm;		
	UBWC m_ulhs;										/* reference sort */						
	UBWC m_urhs;										/* sorted by non-increasing weight */
	SUPERW m_sw;
	bool* m_sw_on;										/* [DEPTH] switch for applying superweight analysis	*/	
	vint lvsw;											/* nodes removed during superweight analysis */
public:
	CliqueWeightedPlus(ugraph* g, param_t p)			:Clique<ugraph>(g, p),gw(NULL), m_tw(NULL), m_sw_on(NULL){};
	CliqueWeightedPlus(param_t p)						:Clique<ugraph>(p),gw(NULL), m_tw(NULL), m_sw_on(NULL){};
	virtual ~CliqueWeightedPlus()						{clear_bounding_graph();}
	void set_param(param_t p)							{Clique<ugraph>::clear_all(); CLQParam::set_param(p);}	
	
	const ugraph& get_reference_graph() const			{return *g;}
	const ugraph& get_bounding_graph() const			{return *gw;}

////////////////
//framework extended functions
	virtual int init_color_sets		();	
	virtual void clear_color_sets	();
	virtual	int initial_bounds		(int& lb, int& ub,  KCore<ugraph>* pkcore=NULL);
	int initial_node_removal		(int LB, vint& lv);									/* only for Sparse graphs */

////////////////

	ugraph* init_bounding_graph()		{clear_bounding_graph(); gw= new ugraph(*g); return gw;}
	void clear_bounding_graph  ()		{if(gw) delete gw; gw=NULL;}
 	
////////////
//coloring
	void paint_w					(int depth, int kmin);												/* deprecated-working */
	int paint_tw					(int depth, int kmin);												/* deprecated */
				
	void expand_w					(int depth, int maxac, bitarray& l_bb , nodelist_t& l_v);			/* deprecated */
	void expand_tw					(int depth, int maxac, bitarray& l_bb , nodelist_t& l_v);			/* deprecated */

	//recursive function first computes candidate nodelist and then branches
	void expand_w_shared_ref		(int depth, int maxac, bitarray& l_bb , nodelist_t& l_v);			/* reference */
	void expand_w_shared_tests		(int depth, int maxac, bitarray& l_bb , nodelist_t& l_v);			/* for tests-remove */
	
	//recursive function branches on candidate list first (root candidate list preproc)
	void expand_w_shared_preproc	(int depth, int maxac, bitarray& l_bb , nodelist_t& l_v);			/* reference */

////////////
// variable color sets and weights
	void expand_w_shared_preproc_CW		(int depth, int maxac, bitarray& l_bb , nodelist_t& l_v);			
	void expand_w_shared_preproc_CW_RD	(int depth, int maxac, bitarray& l_bb , nodelist_t& l_v);		/* reference-Russian Doll  (without SuperWeight) */

	void expand_w_shared_preproc_CW_SuperW	    (int depth, int maxac, bitarray& l_bb , nodelist_t& l_v);	/* cutting edge!  (1)*/
	void expand_w_shared_preproc_CW_SuperW_3S	(int depth, int maxac, bitarray& l_bb , nodelist_t& l_v);	/* cutting edge  with SIZE_3 INFO */

/////////////
//pruning 
	top_t find_top_k				(int v, int col, int& gap);											/* updates top_k and gap data- TO REMOVE IN NEAR FUTURE*/
	
	//I/O
	ostream& print_top_weights		(int last_col, ostream& o=cout);
							
	//test framework
	virtual	int set_up();
	virtual	void run();	
	virtual	void tear_down()		{clear_bounding_graph(); Clique<ugraph>::tear_down();}
};

inline
int CliqueWeightedPlus::initial_node_removal (int LB, vint& lv){
/////////////////////////
// determines nodes which do NOT belong to a max clique
// according to threshold LB (nodes which cannot improve LB solution)
//
// RETURNS: RET_VAL-number of candidate nodes found 
//
// COMMENTS: Only for Sparse Graphs

	int wv=0;
	lv.clear();
	for(int v=0; v<m_size; v++){
		bitarray& bbn=g->get_neighbors(v);
		bbn.set_bit(v);
		bbn.init_scan(bbo::DESTRUCTIVE);
		wv=0;
		while(true){
			int v=bbn.next_bit_del();
			if(v==EMPTY_ELEM) break;

			//sum weights
			wv+=g->get_wv(v);
			if(wv>LB) break;			/* early exit */
		}

		if(wv<=LB) lv.push_back(v);		
	}

	return lv.size();
}

inline
CliqueWeightedPlus::top_t CliqueWeightedPlus::find_top_k(int v, int col, int& gap){
/////////////////
// Determines top_1 or top_2 for the given vertex, col and gap
// Updates gap, and top_k
//
// Returns NONE, TOP1 or TOP2

	int wv=g->get_wv(v);
	if(wv<=gap){
		gap-=wv;

		//LOG_INFO("FULL GAP COVER ANALYSIS");
		//cin.get();
		m_sel=m_unsel;
		m_sel.erase_bit(g->get_neighbors(v));		/* cover for freed gap */
		m_sel.init_scan(bbo::DESTRUCTIVE);
		int nbb=0;
		while(true){
			int vfc=m_sel.next_bit_del(nbb);
			if(vfc==EMPTY_ELEM) break;

			//LOG_INFO("COVER FOUND FOR FULL GAP: "<<vfc);
			//cin.get();
			m_unsel.erase_bit(vfc);
			m_sel.erase_block(nbb,g->get_neighbors(vfc));
		}		
		return GAP;
	}else if( wv<=(m_tw[col][0]+gap)){
		int vfc=iop.m_colSets[col].bb.lsbn64();
		if( !g->get_neighbors(v).is_bit(vfc) ){
			//update top_k and g
			if(wv>m_tw[col][0]){
				//update gap and top-k
				int gap_freed=wv-m_tw[col][0];
				gap-=gap_freed;	

				//LOG_INFO("TOP1 GAP COVER ANALYSIS");
				m_sel=m_unsel;
				m_sel.erase_bit(g->get_neighbors(v));		/* cover for freed gap */
				m_sel.init_scan(bbo::DESTRUCTIVE);
				int nbb=0;
				while(true){
					vfc=m_sel.next_bit_del(nbb);
					if(vfc==EMPTY_ELEM) break;
					if(g->get_wv(vfc)<=gap_freed){
						//LOG_INFO("COVER FOUND FOR GAP: "<<vfc<<" ,col:"<<col);
						//cin.get();
						m_unsel.erase_bit(vfc);
					}
					m_sel.erase_block(nbb,g->get_neighbors(vfc));
				}				
				m_tw[col][0]=0;	
			}else{
				m_tw[col][0]-=wv;
				
				//LOG_INFO("TOP1 COVER ANALYSIS");
				m_sel=m_unsel;
				m_sel.erase_bit_joint(g->get_neighbors(vfc),g->get_neighbors(v));	
				m_sel.init_scan(bbo::DESTRUCTIVE);
				int nbb=0;
				while(true){
					vfc=m_sel.next_bit_del(nbb);
					if(vfc==EMPTY_ELEM) break;
					//LOG_INFO("COVER FOUND FOR: "<<vfc<<" ,col:"<<col);
					//cin.get();
					m_unsel.erase_bit(vfc);
					m_sel.erase_block(nbb,g->get_neighbors(vfc));
				}	
			}						
			//LOG_INFO("TOP1:"<<v<<":"<<col);
			return TOP1;
		}
	}else if(wv<=(m_tw[col][1]+gap)){
		int vfc1=iop.m_colSets[col].bb.lsbn64();
		iop.m_colSets[col].bb.erase_bit(vfc1);
		int vfc2=iop.m_colSets[col].bb.lsbn64();
		iop.m_colSets[col].bb.set_bit(vfc1);
		
		if(vfc2!=EMPTY_ELEM && !g->get_neighbors(v).is_bit(vfc1) && !g->get_neighbors(v).is_bit(vfc2)){
			//update gap and top-k
			if(wv>m_tw[col][1]){
				gap-=(wv-m_tw[col][1]);
				m_tw[col][1]=0;		
				m_tw[col][0]=0;
			}else{
				m_tw[col][1]-=wv;
				m_tw[col][0]=m_tw[col][1];
			}
						
			//LOG_INFO("TOP2:"<<v<<":"<<col);
			return TOP2;
		}
	}
	return NONE;
}

inline
ostream& CliqueWeightedPlus::print_top_weights	(int last_col /*1 based */, ostream& o){
///////////////
// prints top weight up to, and including, last_col
	for(int i=1; i<=last_col; i++){
		o<<"["<<i<<","<<m_tw[i][0]<<","<<m_tw[i][1]<<"] ";
	}
	o<<endl;
	return o;
}
	

inline
void CliqueWeightedPlus::clear_color_sets (){
	
	iop.clear();

	//top_weights
	if(m_tw){
		for(int i=0; i<=m_alloc; i++){
			delete [] m_tw[i];
		}
		delete [] m_tw; 
		m_tw=NULL;
	}

	if(m_sw_on){
		delete [] m_sw_on; 
		m_sw_on=NULL;
	}
}

inline
int CliqueWeightedPlus::init_color_sets(){

	clear_color_sets();
	
	try{
		iop.set_graph(g);					/* graph already ordered here*/
		if(iop.init(m_alloc+1)==-1){
			runtime_error r("CliqueWeightedPlus::init_color_sets()-error allocating infra-chrom ColorSets");		//***check this exception is caught below
			throw r;
		}

		//superweight switch
		m_sw_on= new bool[m_alloc+1];
		for(int i=0; i<=m_alloc; i++){
			m_sw_on[i]= false;
		}

		//top_weights-init 0
		m_tw= new int*[m_alloc+1];
		for(int i=0; i<=m_alloc; i++){
			m_tw[i]= new int[2];
		}

		for(int i=0; i<=m_alloc; i++){
			m_tw[i][0]=0; m_tw[i][1]=0;
		}
		
	}catch(exception& e){
		throw;
	}

	return 0;
}

inline
int CliqueWeightedPlus::set_up(){
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
	case BBMC_WEIGHTED:
	case BBMC_WEIGHTED_BASIC:
	case BBMC_WEIGHTED_DOUBLE:
	case BBMC_WEIGHTED_SHARED_TESTS:
	case BBMC_WEIGHTED_SHARED_REF:
	case BBMC_WEIGHTED_SHARED_PREPROC:
	case BBMC_WEIGHTED_SHARED_PREPROC_CW:
	case BBMC_WEIGHTED_SHARED_PREPROC_CW_RD:
	case BBMC_WEIGHTED_SHARED_PREPROC_CW_SUPER_WEIGHT:
	case BBMC_WEIGHTED_SHARED_PREPROC_CW_SUPER_WEIGHT_3S:
	break;
	default:
		LOG_ERROR("CliqueWeightedPlus::setup unknown algorithm ");
		return -1;
	}

//////////////////////////////////
//isomorphism pre-processing/mem. allocation 
	if(param.isomorphism==true){
		LOG_INFO("ISOMORPHISM pre-processing INIT----------------------------------");

#ifdef REORDER_REVERSE_NON_INC_WIDTH
		  GraphSort<ugraph> gs(*g);
		  gs.reorder(gs.new_order(gbbs::MAX_WEIGHT,gbbs::PLACE_LF),get_decoder());			/* preprocessing-hamming 8_x does not work!, improves most of structured graphs specially phat */
		//gs.reorder(gs.new_order(gbbs::MIN_WEIGHT,gbbs::PLACE_FL));			
#endif

		if(m_gm.build_mapping(*g,param.iso_map.lhs.first, param.iso_map.lhs.second, 
								param.iso_map.rhs.first, param.iso_map.rhs.second,
								"MIN_WIDTH_LF", "MAXW_FL"	 /* TODO- set strings appropiately*/		)==-1 ){
			LOG_ERROR("CliqueWeightedPlus::set_up()-bizarre mapping");
			return -1;
		}

		ugraph* gw= init_bounding_graph();	/* before ordering */
			
		/* set_current_graph according to lhs mapping (main graph for enumeration) */
		GraphSort<ugraph> olhs(*g);
		olhs.reorder(olhs.new_order(param.iso_map.lhs.first, param.iso_map.lhs.second),get_decoder());

		/* set_ordered_graph according to rhs mapping  (graph for bounding)*/
		GraphSort<ugraph> orhs(*gw);
		orhs.reorder(orhs.new_order(param.iso_map.rhs.first, param.iso_map.rhs.second));
				
		//UBWC
		m_ulhs.set_graph(g);	m_ulhs.init();
		m_urhs.set_graph(gw);	m_urhs.init();	/* TODO-refactor allocation */
		m_sw.set_graph(g);		

	}
	LOG_INFO("ISOMORPHISM pre-processing END----------------------------------");			

//////////////////////////////

	//set_up framework-based
	if(param.unrolled){
		LOG_ERROR("CliqueWeighted::setup unrolled variant undefined");
		return -1;
	}else{
		if( (sol=set_up_non_unrolled(info))>0 ){
				LOG_INFO("[w="<<sol<<"]"<<" TRIVIALLY SOLVED DURING PRECOMPUTATION");
				res.set_UB(sol);
		}else{ 
			//Trivial solution not found
			/* additional extra-initialization non_unrolled case */
					
			//UB[node], each slot is a bound for the russian doll containing the node
			if(param.init_preproc!=HEUR){
				LOG_PRINT("COMPUTING UB");
				InitColorUBW c(*g);
				c.Compute_UB_last(m_lcol[0]);
			
				//output to screen
				stringstream sstr("");
				for(int i=0; i<m_size; i++){
					sstr<<m_lcol[0][i]<<" ";
				}
				LOG_INFO(sstr.str());
			}
						
			//LB: using amts heuristic
			if(param.init_preproc!=UB){
				LOG_PRINT("COMPUTING LB");
				//****TODO: place in setup and check for TRIVIAL SOLUTION
				AMTSexec a(RESTARTS, ITERATIONS_PER_RESTART, WMODE_WEIGHTS);				
				int lb_amts=a.run(*g);
				if(lb_amts>res.get_lower_bound()){
					maxno=lb_amts;
					res.set_LB(lb_amts);
					res.add_solution(a.get_nodes());			/* CHECK- used in the unweighted case */
				}
			}
			
			//initial candidate set: **TODO- 1) add expanded nodes as part of the solution!
			Clique<ugraph>::maxac=0;
			if(param.isomorphism==true){
				int gap=0, nb_col=0;
				bitarray bb(g->number_of_vertices());
				bb.set_bit(0,m_size-1);
				vint lv;

				//super weight analysis!
				LOG_INFO("BEGIN SUPER WEIGHT ANALYSIS:--------------------------");
				int sumw=m_sw.search(bb,lv);
				( sumw>0 || g->density()>=MIN_DENSITY_SUPER )? m_sw_on[1]=true : m_sw_on[1]=false;  /*FALSE -> no SUPERW analysis */
				LOG_INFO("END SUPER WEIGHT ANALYSIS:----------------------------");

				//compute candidate set for remaining subproblem
				Clique<ugraph>::m_bbroot=bb;
			//	gap=m_ulhs.paint_and_map_OPT(bb,res.get_lower_bound()-sumw /* MUST BE!*/,m_gm,m_urhs,nb_col);
				gap=m_ulhs.paint_and_map_CW_3S(bb,res.get_lower_bound()-sumw /* MUST BE!*/,m_gm,m_urhs);
			//	gap=m_ulhs.paint_and_map_CW(bb,res.get_lower_bound()-sumw /* MUST BE!*/,m_gm,m_urhs);
				//m_ulhs.cover(gap,nb_col,m_urhs,m_gm);
				m_ulhs.cover_CW_3S_2R(m_urhs,m_gm);
			//	m_ulhs.cover_CW_resw(m_urhs,m_gm);
			//	m_ulhs.cover_with_overlap(nb_col,m_urhs,m_gm);
				m_ulhs.cover_with_overlap_CW_Ord(m_urhs,m_gm);

#ifdef	RUSSIAN_DOLL
				m_ulhs.get_unsel().to_old_vector_reverse(Clique<ugraph>::m_lroot.nodos,Clique<ugraph>::m_lroot.index);  Clique<ugraph>::m_lroot.index--;		
#else	
				m_ulhs.get_unsel().to_old_vector(Clique<ugraph>::m_lroot.nodos,Clique<ugraph>::m_lroot.index);  Clique<ugraph>::m_lroot.index--;		
#endif		

		
				//trivial solution found
				if(m_ulhs.get_unsel().is_empty()){
					LOG_ERROR("Bizarre solution-CHECK!");
					res.set_UB(res.get_lower_bound());
					sol=res.get_lower_bound();
				}

				//initial data
				Clique<ugraph>::maxac=sumw;					
				g->print_weights(m_ulhs.get_unsel());
				LOG_INFO("INITIAL CLIQUE SIZE:"<<sumw);

			
				//Russian Doll-root bitset			
#ifdef	RUSSIAN_DOLL
				Clique<ugraph>::m_bbroot.erase_bit(m_ulhs.get_unsel());
#endif
	
			
				
				
				/////////////////////////////////////////////
				//superweight attempt NOW DEPRECATED 
				//OPTION A
				//bb.set_bit(0,m_size-1);
				//int pc=0;
				//for(int v=m_size-1; v>=0; v--){
				//	if(is_super_weight(v, bb, pc)){
				//		LOG_INFO("HURRAY");
				//		LOG_INFO("PESO: "<<g->get_wv(v)<<" super-peso: "<<pc);
				//		//cin.get();

				//		//remove remaining candidates
				//		Clique<ugraph>::m_lroot.index=EMPTY_ELEM;
				//		Clique<ugraph>::m_lroot.nodos[++Clique<ugraph>::m_lroot.index]=v;

				//		//Russian Doll
				//	/*	Clique<ugraph>::m_bbroot.set_bit(0, m_size-1);
				//		Clique<ugraph>::m_bbroot.erase_bit(v);*/

				//		break;
				//	}else{
				//	//	LOG_INFO("PESO: "<<g->get_wv(v)<<" super-peso: "<<pc);
				//	}
				//}
				//////////////////////////////////////////
			}
			
			LOG_INFO("PREPROC-END-----------------------------");
			//cin.get();
		}
	}	
	return sol;
}

inline
void CliqueWeightedPlus::run(){
	//algorithm
	if(param.unrolled){
		LOG_ERROR("CliqueWeightedPlus::run: unrolling not defined for this familiy of algorithms");
		return;
	}else{
		res.tic();
		switch(param.alg){
		case BBMC_WEIGHTED_BASIC:
			expand_w(0, 0, Clique<ugraph>::m_bbroot, Clique<ugraph>::m_lroot);
		//	LOG_INFO("MOCK-RUNNING WEIGHTED CASE");
			break;
		case BBMC_WEIGHTED:
			expand_tw(0, 0, Clique<ugraph>::m_bbroot, Clique<ugraph>::m_lroot);
		//	LOG_INFO("MOCK-RUNNING WEIGHTED CASE");
			break;
		case BBMC_WEIGHTED_DOUBLE:
		//	expand_w_dg(0, 0, Clique<ugraph>::m_bbroot, Clique<ugraph>::m_lroot);
		//	LOG_INFO("MOCK-RUNNING WEIGHTED CASE");
			break;
		case BBMC_WEIGHTED_SHARED_REF:
			expand_w_shared_ref(0, 0, Clique<ugraph>::m_bbroot, Clique<ugraph>::m_lroot);
			break;
		case BBMC_WEIGHTED_SHARED_TESTS:
			/*m_bbroot.erase_bit();
			m_bbroot.set_bit(7); m_bbroot.set_bit(28); m_bbroot.set_bit(33);  m_bbroot.set_bit(62);
			m_bbroot.set_bit(102); m_bbroot.set_bit(104); m_bbroot.set_bit(134);  m_bbroot.set_bit(135);
			m_bbroot.set_bit(141);  m_bbroot.set_bit(244);  m_bbroot.set_bit(251);  m_bbroot.set_bit(320);
			g->print_weights(m_bbroot); cout<<endl;
			maxno=1228;*/
			expand_w_shared_tests(0, 0, Clique<ugraph>::m_bbroot, Clique<ugraph>::m_lroot);
			break;
		case BBMC_WEIGHTED_SHARED_PREPROC:
			/*m_bbroot.erase_bit();
			m_bbroot.set_bit(7); m_bbroot.set_bit(28); m_bbroot.set_bit(33);  m_bbroot.set_bit(62);
			m_bbroot.set_bit(102); m_bbroot.set_bit(104); m_bbroot.set_bit(134);  m_bbroot.set_bit(135);
			m_bbroot.set_bit(141);  m_bbroot.set_bit(244);  m_bbroot.set_bit(251);  m_bbroot.set_bit(320);
			g->print_weights(m_bbroot); cout<<endl;  
			maxno=10;
			m_bbroot.to_old_vector( Clique<ugraph>::m_lroot.nodos,Clique<ugraph>::m_lroot.index); Clique<ugraph>::m_lroot.index-=12;*/
			expand_w_shared_preproc(0, /*0*/ Clique<ugraph>::maxac, Clique<ugraph>::m_bbroot, Clique<ugraph>::m_lroot);
			break;
		case BBMC_WEIGHTED_SHARED_PREPROC_CW:
			expand_w_shared_preproc_CW(0,/*0*/ Clique<ugraph>::maxac, Clique<ugraph>::m_bbroot, Clique<ugraph>::m_lroot);
			break;
		case BBMC_WEIGHTED_SHARED_PREPROC_CW_RD:
			expand_w_shared_preproc_CW_RD(0, /*0*/ Clique<ugraph>::maxac, Clique<ugraph>::m_bbroot, Clique<ugraph>::m_lroot);
			break;
		case BBMC_WEIGHTED_SHARED_PREPROC_CW_SUPER_WEIGHT:
			expand_w_shared_preproc_CW_SuperW(0, /*0*/ Clique<ugraph>::maxac, Clique<ugraph>::m_bbroot, Clique<ugraph>::m_lroot);
			break;
		case BBMC_WEIGHTED_SHARED_PREPROC_CW_SUPER_WEIGHT_3S: /* SIZE 3 INFO */
			expand_w_shared_preproc_CW_SuperW_3S(0, /*0*/ Clique<ugraph>::maxac, Clique<ugraph>::m_bbroot, Clique<ugraph>::m_lroot);
			break;
		default:
			LOG_ERROR("CliqueWeightedPlus::run-non_unrolled:unknown clique algorithm");
		}

		res.toc();
	}

	//read solution
	res.set_UB(maxno);
	LOG_INFO("[w:"<<res.get_upper_bound()<<","<<res.get_user_time()<<"s]");
}	


inline
void CliqueWeightedPlus::expand_w (int depth, int maxac, bitarray& l_bb , nodelist_t& l_v){
////////////////////////
// basic recursive search algorithm for the weighted case
// 
// date of creation: 4/7/17

	int v, wv;
	res.inc_number_of_steps();

	//main loop
	while(l_v.index>=0){
			
		//Estrategias
		v=l_v.nodos[l_v.index--];
	
		//CUT by color (since [Konc & Janecic, 2007] this is of limited use (only for first branch))
#ifdef STRONG_ROOT_COLORING
		if(depth==0){
			if( m_lcol[depth][v]<=(maxno-maxac) ){
				l_bb.erase_bit(v);
				continue;
			}
		}
#endif
		wv=g->get_wv(v);
#ifdef ROOT_VERTEX_PROGRESS
		if(depth==0)
			cout<<"root vertex: "<<"("<<v<<","<<wv<<")"<<endl;
#endif

/////////////////////////////////
// CHILD NODE GENERATION
		
		//Node generation by masking
		AND(g->get_neighbors(v), l_bb, LISTA_BB(depth));		//optimized when place second the bitset with higher population
		
		//Leaf node: updates incumbent if necessary
		if( LISTA_BB(depth).is_empty()){
			if((maxac+wv)>maxno){
				maxno=maxac+wv;						//NEW GLOBAL OPTIMUM FOUND

				#ifdef STORE_SOLUTION
					res.set_UB(maxno);
					res.clear_all_solutions();
					m_path[depth]=v;
					res.add_solution(depth+1 /* size 1-based */, m_path);
					//LOG_INFO("SOLUTION IMPROVED:"<<maxno);
					//cin.get();
					#ifdef VIEW_PROGRESS						//copied from Weighted-CHECK
						stringstream sstr("");
						res.print_first_sol(sstr);

						//print weights
						vint sol1=res.get_first_solution();			//only one solution
						int wsol=0;
						for(int i=0; i<=depth; i++){
						//	wsol+=iop.m_lw[sol1[i]];				/* TODO-data structure from CliqueWeighted */
							wsol+=g->get_wv(sol1[i]);				/* TODO-data structure from CliqueWeighted */
						}

						/*if(!is_clique(sol1)){
							LOG_ERROR("bizarre clique");
							cin.get();
						}*/

						sstr<<"["<<wsol<<"]";
						LOG_INFO(sstr.str());

					#endif	
					
				#endif
			
			}
		l_bb.erase_bit(v);
		continue;
		}

		//approx. coloring (generates child list of nodes in LISTA_L(maxac))
		paint_w(depth, maxno-(maxac+wv) /* kmin */);  /* default */

		//I/O
		/*int kmin=maxno-(maxac+wv);
		if(kmin<0){
			LOG_INFO("maxno: "<<maxno<<",maxac:"<<maxac<<" v:"<<v<<",wv:"<<wv<<",kmin: "<<kmin<<",depth:"<<depth);
			cin.get();
		}*/



	//	paint_sel(maxac);
	/*	LISTA_L(maxac).print(cout, true); cout<<endl;
		LISTA_BB(maxac).print(); cout<<endl;*/
	

		//cuts if there are no child nodes of v
		if(LISTA_L(depth).index<0){
			l_bb.erase_bit(v);
			continue;
		}
			
	//	LISTA_L(depth).print(cout, true); cout<<"depth"<<depth<<"-------------"<<endl;
	//	cin.get();
///////////////////////////////////////////////////////
// CANDIDATE EXPANSION

		//sets path
		m_path[depth]=v;
		/*LOG_INFO("BRANCHED:["<<v<<","<<g->get_wv(v)<<"]");
		cin.get();*/
				
		//Generacion de nuevos nodos
		expand_w(depth+1, maxac+wv,LISTA_BB(depth),LISTA_L(depth));  

		//////////////////////////////////////////////
		// BACKTRACK (does not delete v from path since it will be overwritten in the same level of search tree)
		l_bb.erase_bit(v); 

	}// next node
}

inline
void CliqueWeightedPlus::expand_w_shared_tests (int depth, int maxac, bitarray& l_bb , nodelist_t& l_v){
////////////////////////
// New recursive search algorithm for the weighted case, which uses graph isomorphism 
// for bounding
// 
// date of creation: 22/8/17
//
// COMMENTS: l_v is computed inside the call
	
	int nb_col, gap;
	res.inc_number_of_steps();	
///////////////////////////
//DETERMINE CANDIDATE NODES TO BRANCH
	if(maxac>=maxno){
		l_bb.to_old_vector(l_v.nodos,l_v.index); l_v.index--;
	}else{ /* General case */
		m_ulhs.unsel=l_bb;
		//gap=m_ulhs.paint_and_map(l_bb,maxno-maxac,m_gm,m_urhs,nb_col);	/* updates gap: should be>=0*/
		gap=m_ulhs.paint_and_map_OPT(l_bb,maxno-maxac,m_gm,m_urhs,nb_col);	/* updates gap: should be>=0*/
		if(m_ulhs.unsel.is_empty()){
			//LOG_INFO("CUT-NO CANDIDATES PAINT AND MAP");
			return;
		}
		if(nb_col==0){													/* all nodes have weights higher than the gap */
			l_bb.to_old_vector(l_v.nodos,l_v.index); l_v.index--;
		}else{
			/*if(nb_col==1){
				//caso particular que no afecta a la completitud aunque hace algun camino tortuoso
			}*/

			//m_urhs.set_tk(1, nb_col);									/* not needed, updated in paint_and_map_OPT */
			
			/////////////
			//COVERING (in this precise order seems best)
			m_ulhs.cover(gap,nb_col,m_urhs,m_gm);
			m_ulhs.cover_with_overlap(nb_col, m_urhs, m_gm);		
			///////////////

			m_ulhs.unsel.to_old_vector(l_v.nodos,l_v.index); l_v.index--;
			if(l_v.index==EMPTY_ELEM){
				return;  
			}
		}
	}
/////////////////////////
//BRANCHING:- nodes are taken in reverse order
	int v=EMPTY_ELEM, wv=0; 
	while(l_v.index>=0){
		v=l_v.nodos[l_v.index--];
		wv=g->get_wv(v);
/////////////////////////////////
// CHILD NODE GENERATION
		AND(g->get_neighbors(v), l_bb, LISTA_BB(depth));		//optimized when place second the bitset with higher population
	
		//Leaf node: updates incumbent if necessary
		if( LISTA_BB(depth).is_empty()){
			if((maxac+wv)>maxno){
				maxno=maxac+wv;						//NEW GLOBAL OPTIMUM FOUND

				#ifdef STORE_SOLUTION
					res.set_UB(maxno);
					res.clear_all_solutions();
					m_path[depth]=v;
					res.add_solution(depth+1 /* size 1-based */, m_path);
					//LOG_INFO("SOLUTION IMPROVED:"<<maxno);
					//cin.get();
					#ifdef VIEW_PROGRESS						//copied from Weighted-CHECK
						stringstream sstr("");
						res.print_first_sol(sstr);

						//print weights
						vint sol1=res.get_first_solution();			//only one solution
						int wsol=0;
						for(int i=0; i<=depth; i++){
						//	wsol+=iop.m_lw[sol1[i]];				/* TODO-data structure from CliqueWeighted */
							wsol+=g->get_wv(sol1[i]);				/* TODO-data structure from CliqueWeighted */
						}

						/*if(!is_clique(sol1)){
							LOG_ERROR("bizarre clique");
							cin.get();
						}*/

						sstr<<"["<<wsol<<"]";
						LOG_INFO(sstr.str());

					#endif	
				#endif
			}
		l_bb.erase_bit(v);
		continue;
		}
///////////////////////////////////////////////////////
// CANDIDATE EXPANSION
		m_path[depth]=v;		
		expand_w_shared_tests(depth+1, maxac+wv,LISTA_BB(depth),LISTA_L(depth));  
//////////////////////////////////////////////
// BACKTRACK (does not delete v from path since it will be overwritten in the same level of search tree)
		l_bb.erase_bit(v); 
	}//next v candidate
}

inline
void CliqueWeightedPlus::expand_w_shared_preproc (int depth, int maxac, bitarray& l_bb , nodelist_t& l_v){
////////////////////////
// New recursive search algorithm for the weighted case, which uses graph isomorphism 
// for bounding-reference
// 
// date of creation: 22/8/17
//
// COMMENTS: 
// 1.l_v is computed inside the call
// 2.root nodelist is preprocessed  (as in MCP)
	
	int nb_col, gap, v, wv;
	res.inc_number_of_steps();	

	/*g->print_weights(l_bb); cout<<endl;
	l_v.print(cout, true);
	cin.get();
	*/
	//TODO-CHECK!!
	/*if(maxac>=maxno){
		l_bb.to_old_vector(l_v.nodos,l_v.index); l_v.index--;*/

///////////////////////////
// BRANCH ON CANDIDATES

	//main loop
	while(l_v.index>=0){
		//Estrategias
		v=l_v.nodos[l_v.index--];
		
		//CUT by color (since [Konc & Janecic, 2007] this is of limited use (only for first branch))
//#ifdef STRONG_ROOT_COLORING
//		if(depth==0){
//			if( m_lcol[depth][v]<=(maxno-maxac) ){
//				l_bb.erase_bit(v);
//				continue;
//			}
//		}
//#endif
		wv=g->get_wv(v);
#ifdef ROOT_VERTEX_PROGRESS
		if(depth==0)
			cout<<"root vertex: "<<"("<<v<<","<<wv<<")"<<endl;
#endif
		
/////////////////////////////////
// CHILD NODE GENERATION
		
		//Node generation by masking
		AND(g->get_neighbors(v), l_bb, LISTA_BB(depth));			//optimized when place second the bitset with higher population
		
		//Leaf node: updates incumbent if necessary
		if( LISTA_BB(depth).is_empty()){
			if((maxac+wv)>maxno){
				maxno=maxac+wv;						//NEW GLOBAL OPTIMUM FOUND

				#ifdef STORE_SOLUTION
					res.set_UB(maxno);
					res.clear_all_solutions();
					m_path[depth]=v;
					res.add_solution(depth+1 /* size 1-based */, m_path);
					//LOG_INFO("SOLUTION IMPROVED:"<<maxno);
					//cin.get();
					#ifdef VIEW_PROGRESS						//copied from Weighted-CHECK
						stringstream sstr("");
						res.print_first_sol(sstr);

						//print weights
						vint sol1=res.get_first_solution();			//only one solution
						int wsol=0;
						for(int i=0; i<=depth; i++){
						//	wsol+=iop.m_lw[sol1[i]];				/* TODO-data structure from CliqueWeighted */
							wsol+=g->get_wv(sol1[i]);				/* TODO-data structure from CliqueWeighted */
						}

						/*if(!is_clique(sol1)){
							LOG_ERROR("bizarre clique");
							cin.get();
						}*/

						sstr<<"["<<wsol<<"]";
						LOG_INFO(sstr.str());

					#endif	
				#endif
			}
		l_bb.erase_bit(v);
		continue;
		}

		if(maxac>=maxno){
			LOG_ERROR("bizarre maxno");
			LISTA_BB(depth).to_old_vector(LISTA_L(depth).nodos,LISTA_L(depth).index); LISTA_L(depth).index--;
		}else{
			
			//	m_ulhs.unsel=LISTA_BB(depth);
			gap=m_ulhs.paint_and_map_OPT(LISTA_BB(depth),maxno-maxac-wv,m_gm,m_urhs,nb_col);	/* updates gap: should be>=0*/
			if(m_ulhs.unsel.is_empty()){
				//LOG_INFO("CUT-NO CANDIDATES PAINT AND MAP");
				l_bb.erase_bit(v);
				continue;
			}
			if(nb_col==0){													/* all nodes have weights higher than the gap */
				LISTA_BB(depth).to_old_vector(LISTA_L(depth).nodos,LISTA_L(depth).index); LISTA_L(depth).index--;
			}else{
				/*if(nb_col==1){
				//caso particular que no afecta a la completitud aunque hace algun camino tortuoso
				}*/


				/////////////
				//COVERING (in this precise order seems best)
				m_ulhs.cover(gap,nb_col,m_urhs,m_gm);
				m_ulhs.cover_with_overlap(nb_col, m_urhs, m_gm);		
				///////////////

				m_ulhs.unsel.to_old_vector(LISTA_L(depth).nodos,LISTA_L(depth).index); LISTA_L(depth).index--;
				if(LISTA_L(depth).index==EMPTY_ELEM){
					l_bb.erase_bit(v);
					continue;
				}
			}
		}
		
///////////////////////////////////////////////////////
// CANDIDATE EXPANSION
		m_path[depth]=v;		
		expand_w_shared_preproc(depth+1, maxac+wv,LISTA_BB(depth),LISTA_L(depth));  
//////////////////////////////////////////////
// BACKTRACK (does not delete v from path since it will be overwritten in the same level of search tree)
		l_bb.erase_bit(v); 

	}//next candidate
}


inline
void CliqueWeightedPlus::expand_w_shared_preproc_CW (int depth, int maxac, bitarray& l_bb , nodelist_t& l_v){
////////////////////////
// New recursive search algorithm with variable color sets and weights
// 
// date of creation: 22/8/17
//
// COMMENTS: 
// 1.l_v is computed inside the call
// 2.root nodelist is preprocessed  (as in MCP)
	
	int nb_col, gap, v, wv;
	res.inc_number_of_steps();	
				
	//super_weight analysis
	//if (maxac>maxno) maxno=maxac;
	//if(depth>=1 && depth<=15){
	//	vint lv;
	//	if(wv=super_weight_preproc(l_bb,lv)){
	//		/*if(depth==9)
	//			LOG_INFO("SUPERWEIGHT ANALYSIS SUCCESFUL AT DEPTH:"<<depth);*/
	//	//	com::stl::print_collection(lv); cout<<endl;
	//	//	cin.get();

	//		maxac+=wv; 
	//		if (maxac>maxno) maxno=maxac;
	//		/** store solution **/
	//		l_bb.to_old_vector(l_v.nodos, l_v.index); l_v.index--;
	//	}
	//}

///////////////////////////
// BRANCH ON CANDIDATES

		//main loop
	while(l_v.index>=0){
		
		//Estrategias
		v=l_v.nodos[l_v.index--];
				
		//CUT by color (since [Konc & Janecic, 2007] this is of limited use (only for first branch))
//#ifdef STRONG_ROOT_COLORING
//		if(depth==0){
//			if( m_lcol[depth][v]<=(maxno-maxac) ){
//				l_bb.erase_bit(v);
//				continue;
//			}
//		}
//#endif
		wv=g->get_wv(v);
#ifdef ROOT_VERTEX_PROGRESS
		if(depth==0)
			cout<<"root vertex: "<<"("<<v<<","<<wv<<")"<<endl;
#endif
		
/////////////////////////////////
// CHILD NODE GENERATION
		
		//Node generation by masking
		AND(g->get_neighbors(v), l_bb, LISTA_BB(depth));			//optimized when place second the bitset with higher population
		
		
		//Leaf node: updates incumbent if necessary
		if( LISTA_BB(depth).is_empty()){
			if((maxac+wv)>maxno){
				maxno=maxac+wv;						//NEW GLOBAL OPTIMUM FOUND

				#ifdef STORE_SOLUTION
					res.set_UB(maxno);
					res.clear_all_solutions();
					m_path[depth]=v;
					res.add_solution(depth+1 /* size 1-based */, m_path);
					//LOG_INFO("SOLUTION IMPROVED:"<<maxno);
					//cin.get();
					#ifdef VIEW_PROGRESS						//copied from Weighted-CHECK
						stringstream sstr("");
						res.print_first_sol(sstr);

						//print weights
						vint sol1=res.get_first_solution();			//only one solution
						int wsol=0;
						for(int i=0; i<=depth; i++){
						//	wsol+=iop.m_lw[sol1[i]];				/* TODO-data structure from CliqueWeighted */
							wsol+=g->get_wv(sol1[i]);				/* TODO-data structure from CliqueWeighted */
						}

						/*if(!is_clique(sol1)){
							LOG_ERROR("bizarre clique");
							cin.get();
						}*/

						sstr<<"["<<wsol<<"]"<<":"<<maxno;
						LOG_INFO(sstr.str());

					#endif	
				#endif
			}
		l_bb.erase_bit(v);
		continue;
		}
		//end of leaf node detection
		//////////////////////////////////////////////
		
		
		if(maxac>=maxno){
			//LOG_INFO("solution found as yet undetected");
			LISTA_BB(depth).to_old_vector(LISTA_L(depth).nodos,LISTA_L(depth).index); LISTA_L(depth).index--;
		}else{		
			gap=m_ulhs.paint_and_map_CW(LISTA_BB(depth),maxno-maxac-wv,m_gm,m_urhs);									/* updates gap: should be>=0*/
			if(m_ulhs.unsel.is_empty()){
				//LOG_INFO("CUT-NO CANDIDATES PAINT AND MAP");
				l_bb.erase_bit(v);
				continue;
			}
			if(m_urhs.number_of_colors()==0){																			/* all nodes have weights higher than the gap */
				LISTA_BB(depth).to_old_vector(LISTA_L(depth).nodos,LISTA_L(depth).index); LISTA_L(depth).index--;
			}else{
				/*if(nb_col==1){
				//caso particular que no afecta a la completitud aunque hace algun camino tortuoso
				}*/

				/////////////
				//COVERING (in this precise order seems best)
				m_ulhs.cover_CW(/*gap,*/m_urhs,m_gm);
				//m_ulhs.cover_CW_Ord(/*gap,*/m_urhs,m_gm);
				//m_ulhs.cover_with_overlap_CW(m_urhs, m_gm);		
				m_ulhs.cover_with_overlap_CW_Ord(m_urhs, m_gm);		
				///////////////

				m_ulhs.unsel.to_old_vector(LISTA_L(depth).nodos,LISTA_L(depth).index); LISTA_L(depth).index--;
				if(LISTA_L(depth).index==EMPTY_ELEM){
					l_bb.erase_bit(v);
					continue;
				}
			}
		}
		
		
///////////////////////////////////////////////////////
// CANDIDATE EXPANSION
		m_path[depth]=v;		
		expand_w_shared_preproc_CW(depth+1, maxac+wv,LISTA_BB(depth),LISTA_L(depth));  
//////////////////////////////////////////////
// BACKTRACK (does not delete v from path since it will be overwritten in the same level of search tree)
		l_bb.erase_bit(v); 

	}//next candidate
}

inline
void CliqueWeightedPlus::expand_w_shared_preproc_CW_SuperW(int depth, int maxac, bitarray& l_bb , nodelist_t& l_v){
////////////////////////
// New recursive search algorithm with variable color sets and weights
// 
// date of creation: 22/8/17
//
// COMMENTS: 
// 1.l_v is computed inside the call
// 2.root nodelist is preprocessed  (as in MCP)
	
	int nb_col, gap, v, wv;
	bool super_weight_found=false;
	res.inc_number_of_steps();	
				
	//super_weight analysis
	//if (maxac>maxno) maxno=maxac;
#ifdef SUPER_WEIGHT_RANDOM
	if(m_sw_on[depth] &&  l_v.index>=3 )    /* (200, 0.98)- &&  l_v.index>MIN_CAND_SIZE_SUPER (>=3); REMOVE for structured graphs (i.e. Mann27)*/
#else
	if(m_sw_on[depth])    
#endif
	{	  
		//LOG_INFO("SUPER NODE ANALYSIS: "<<depth);
		if(wv=m_sw.search(l_bb,lvsw)){
			//LOG_INFO("SUPER NODE DEPTH: "<<depth);
			super_weight_found=true;
			m_sw_on[depth+1]=true;

			/**TODO store path **/						
					
			if ((maxac+=wv)>maxno) {
				maxno=maxac;						
				/**TODO store solution **/
			}

			l_bb.to_old_vector(l_v.nodos, l_v.index); l_v.index--;	/* TODO-currently examines all suproblems- filter */

		}else m_sw_on[depth+1]=false;
	}

///////////////////////////
// BRANCH ON CANDIDATES

		//main loop
	while(l_v.index>=0){
			
		v=l_v.nodos[l_v.index--];
		wv=g->get_wv(v);

#ifdef ROOT_VERTEX_PROGRESS
		if(depth==0)
			cout<<"root vertex: "<<"("<<v<<","<<wv<<")"<<endl;
#endif
		
/////////////////////////////////
// CHILD NODE GENERATION
		
		//Node generation by masking
		AND(g->get_neighbors(v), l_bb, LISTA_BB(depth));			//optimized when place second the bitset with higher population
				
		//Leaf node: updates incumbent if necessary
		if( LISTA_BB(depth).is_empty()){
			if((maxac+wv)>maxno){
				maxno=maxac+wv;						//NEW GLOBAL OPTIMUM FOUND

				#ifdef STORE_SOLUTION
					res.set_UB(maxno);
					res.clear_all_solutions();
					m_path[depth]=v;
					res.add_solution(depth+1 /* size 1-based */, m_path);
					//LOG_INFO("SOLUTION IMPROVED:"<<maxno);
					//cin.get();
					#ifdef VIEW_PROGRESS						//copied from Weighted-CHECK
						stringstream sstr("");
						res.print_first_sol(sstr);

						//print weights
						vint sol1=res.get_first_solution();			//only one solution
						int wsol=0;
						for(int i=0; i<=depth; i++){
						//	wsol+=iop.m_lw[sol1[i]];				/* TODO-data structure from CliqueWeighted */
							wsol+=g->get_wv(sol1[i]);				/* TODO-data structure from CliqueWeighted */
						}

						/*if(!is_clique(sol1)){
							LOG_ERROR("bizarre clique");
							cin.get();
						}*/

						sstr<<"["<<wsol<<"]"<<":"<<maxno;
						LOG_INFO(sstr.str());

					#endif	
				#endif
			}
		l_bb.erase_bit(v);
		continue;
		}
		//end of leaf node detection
		//////////////////////////////////////////////
				
		if(maxac>=maxno && !super_weight_found){
			//LOG_INFO("solution found as yet undetected");
			LISTA_BB(depth).to_old_vector(LISTA_L(depth).nodos,LISTA_L(depth).index); LISTA_L(depth).index--;
		}else{		
		//	gap=m_ulhs.paint_and_map_CW(LISTA_BB(depth),maxno-maxac-wv,m_gm,m_urhs);									/* updates gap: should be>=0*/
			gap=m_ulhs.paint_and_map_CW_dyn(LISTA_BB(depth),maxno-maxac-wv,m_gm,m_urhs);								/* updates gap: should be>=0*/
		
			if(m_ulhs.unsel.is_empty()){						/* TODO-empty-check returned by PAINT */
				//LOG_INFO("CUT-NO CANDIDATES PAINT AND MAP");
				l_bb.erase_bit(v);
				continue;
			}
			if(m_urhs.number_of_colors()==0){																			/* all nodes have weights higher than the gap */
				LISTA_BB(depth).to_old_vector(LISTA_L(depth).nodos,LISTA_L(depth).index); LISTA_L(depth).index--;
			}else{
				/*if(nb_col==1){
				//caso particular que no afecta a la completitud aunque hace algun camino tortuoso
				}*/

				//reduce GAP_SIZE
				//int gap_inc=m_urhs.increase_gap();						/* NOT WORKING-DISCONNECTED! */
				/*if(gap_inc){
					LOG_INFO("GAP INCREASE:"<<gap_inc);
					cin.get();
				}*/

				/////////////
				//COVERING (in this precise order seems best)
				
#ifdef DOUBLE_ROUND_FILTER
				m_ulhs.cover_CW_2R(/*gap,*/m_urhs,m_gm);
#else
				m_ulhs.cover_CW(/*gap,*/m_urhs,m_gm);
				//m_ulhs.cover_CW_resw(/*gap,*/m_urhs,m_gm);			/* TODO***-CHECK, currenlty BUGGY phat500_2! */

#endif
				//m_ulhs.cover_CW_Ord(/*gap,*/m_urhs,m_gm);

#ifdef OVERLAP_ACCORDING_TO_WEIGHT
				m_ulhs.cover_with_overlap_CW_Ord(m_urhs, m_gm);		
#else
				m_ulhs.cover_with_overlap_CW(m_urhs, m_gm);		
				
#endif		
			
				//m_ulhs.cover_with_overlap_CW(m_urhs, m_gm);	
				///////////////
				m_ulhs.unsel.to_old_vector(LISTA_L(depth).nodos,LISTA_L(depth).index); LISTA_L(depth).index--;
				if(LISTA_L(depth).index==EMPTY_ELEM){
					l_bb.erase_bit(v);
					continue;
				}
			}
		}
		
		
///////////////////////////////////////////////////////
// CANDIDATE EXPANSION
		m_path[depth]=v;		
		expand_w_shared_preproc_CW_SuperW(depth+1, maxac+wv,LISTA_BB(depth),LISTA_L(depth));  
//////////////////////////////////////////////
// BACKTRACK (does not delete v from path since it will be overwritten in the same level of search tree)
		l_bb.erase_bit(v); 

	}//next candidate
}


inline
void CliqueWeightedPlus::expand_w_shared_preproc_CW_SuperW_3S(int depth, int maxac, bitarray& l_bb , nodelist_t& l_v){
////////////////////////
// New recursive search algorithm with variable color sets and weights
// 
// date of creation: 22/8/17
//
// COMMENTS: 
// 1.l_v is computed inside the call
// 2.root nodelist is preprocessed  (as in MCP)
	
	int nb_col, gap, v, wv;
	bool super_weight_found=false;
	res.inc_number_of_steps();	
				
	//super_weight analysis
	//if (maxac>maxno) maxno=maxac;
#ifdef SUPER_WEIGHT_RANDOM
	if(m_sw_on[depth] &&  l_v.index>=3 )    /* (200, 0.98)- &&  l_v.index>MIN_CAND_SIZE_SUPER (>=3); REMOVE for structured graphs (i.e. Mann27)*/
#else
	if(m_sw_on[depth])    
#endif
	{	  
		//LOG_INFO("SUPER NODE ANALYSIS: "<<depth);				
		if(wv=m_sw.search(l_bb,lvsw)){
			//LOG_INFO("SUPER NODE DEPTH: "<<depth);
			super_weight_found=true;
			m_sw_on[depth+1]=true;
			if ((maxac+=wv)>maxno) maxno=maxac;
			/**TODO store solution **/
			l_bb.to_old_vector(l_v.nodos, l_v.index); l_v.index--;
		}else m_sw_on[depth+1]=false;
	}

///////////////////////////
// BRANCH ON CANDIDATES

		//main loop
	while(l_v.index>=0){
				
		v=l_v.nodos[l_v.index--];
		wv=g->get_wv(v);

#ifdef ROOT_VERTEX_PROGRESS
		if(depth==0)
			cout<<"root vertex: "<<"("<<v<<","<<wv<<")"<<endl;
#endif
		
/////////////////////////////////
// CHILD NODE GENERATION
		
		//Node generation by masking
		AND(g->get_neighbors(v), l_bb, LISTA_BB(depth));			//optimized when place second the bitset with higher population
				
		//Leaf node: updates incumbent if necessary
		if( LISTA_BB(depth).is_empty()){
			if((maxac+wv)>maxno){
				maxno=maxac+wv;						//NEW GLOBAL OPTIMUM FOUND

				#ifdef STORE_SOLUTION
					res.set_UB(maxno);
					res.clear_all_solutions();
					m_path[depth]=v;
					res.add_solution(depth+1 /* size 1-based */, m_path);
					//LOG_INFO("SOLUTION IMPROVED:"<<maxno);
					//cin.get();
					#ifdef VIEW_PROGRESS						//copied from Weighted-CHECK
						stringstream sstr("");
						res.print_first_sol(sstr);

						//print weights
						vint sol1=res.get_first_solution();			//only one solution
						int wsol=0;
						for(int i=0; i<=depth; i++){
						//	wsol+=iop.m_lw[sol1[i]];				/* TODO-data structure from CliqueWeighted */
							wsol+=g->get_wv(sol1[i]);				/* TODO-data structure from CliqueWeighted */
						}

						/*if(!is_clique(sol1)){
							LOG_ERROR("bizarre clique");
							cin.get();
						}*/

						sstr<<"["<<wsol<<"]"<<":"<<maxno;
						LOG_INFO(sstr.str());

					#endif	
				#endif
			}
		l_bb.erase_bit(v);
		continue;
		}
		//end of leaf node detection
		//////////////////////////////////////////////
		
		
		if(maxac>=maxno && !super_weight_found){
			//LOG_INFO("solution found as yet undetected");
			LISTA_BB(depth).to_old_vector(LISTA_L(depth).nodos,LISTA_L(depth).index); LISTA_L(depth).index--;
		}else{		
	
			gap=m_ulhs.paint_and_map_CW_3S(LISTA_BB(depth),maxno-maxac-wv,m_gm,m_urhs);									/* updates gap: should be>=0*/

			if(m_ulhs.unsel.is_empty()){
				//LOG_INFO("CUT-NO CANDIDATES PAINT AND MAP");
				l_bb.erase_bit(v);
				continue;
			}
			if(m_urhs.number_of_colors()==0){																			/* all nodes have weights higher than the gap */
				LISTA_BB(depth).to_old_vector(LISTA_L(depth).nodos,LISTA_L(depth).index); LISTA_L(depth).index--;
			}else{
				/*if(nb_col==1){
				//caso particular que no afecta a la completitud aunque hace algun camino tortuoso
				}*/

				/////////////
				//COVERING (in this precise order seems best)
			
#ifdef DOUBLE_ROUND_FILTER
				m_ulhs.cover_CW_3S_2R(m_urhs,m_gm);
#else
				m_ulhs.cover_CW_3S(m_urhs,m_gm);
#endif
				
#ifdef OVERLAP_ACCORDING_TO_WEIGHT
				m_ulhs.cover_with_overlap_CW_Ord(m_urhs, m_gm);		
#else
				m_ulhs.cover_with_overlap_CW(m_urhs, m_gm);		
				
#endif	
				//m_ulhs.cover_with_overlap_CW(m_urhs, m_gm);		
				//m_ulhs.cover_with_overlap_CW_Ord(m_urhs, m_gm);		
				///////////////

				m_ulhs.unsel.to_old_vector(LISTA_L(depth).nodos,LISTA_L(depth).index); LISTA_L(depth).index--;
				if(LISTA_L(depth).index==EMPTY_ELEM){
					l_bb.erase_bit(v);
					continue;
				}
			}
		}
		
		
///////////////////////////////////////////////////////
// CANDIDATE EXPANSION
		m_path[depth]=v;		
		expand_w_shared_preproc_CW_SuperW_3S(depth+1, maxac+wv,LISTA_BB(depth),LISTA_L(depth));  
//////////////////////////////////////////////
// BACKTRACK (does not delete v from path since it will be overwritten in the same level of search tree)
		l_bb.erase_bit(v); 

	}//next candidate
}

inline
void CliqueWeightedPlus::expand_w_shared_preproc_CW_RD (int depth, int maxac, bitarray& l_bb , nodelist_t& l_v){
////////////////////////
// New recursive search algorithm with variable color sets and weights
// 
// date of creation: 22/8/17
//
// COMMENTS: 
// 1.l_v is computed inside the call
// 2.root nodelist is preprocessed  (as in MCP)
	
	int nb_col, gap, v, wv;
	res.inc_number_of_steps();	

///////////////////////////
// BRANCH ON CANDIDATES

	//main loop
	while(l_v.index>=0){
	
		v=l_v.nodos[l_v.index--];
		wv=g->get_wv(v);

#ifdef ROOT_VERTEX_PROGRESS
		if(depth==0)
			cout<<"root vertex: "<<"("<<v<<","<<wv<<")"<<endl;
#endif
		
/////////////////////////////////
// CHILD NODE GENERATION
		
		//Node generation by masking
		AND(g->get_neighbors(v), l_bb, LISTA_BB(depth));			//optimized when place second the bitset with higher population
		
		//Leaf node: updates incumbent if necessary
		if( LISTA_BB(depth).is_empty()){
			if((maxac+wv)>maxno){
				maxno=maxac+wv;						//NEW GLOBAL OPTIMUM FOUND

				#ifdef STORE_SOLUTION
					res.set_UB(maxno);
					res.clear_all_solutions();
					m_path[depth]=v;
					res.add_solution(depth+1 /* size 1-based */, m_path);
					//LOG_INFO("SOLUTION IMPROVED:"<<maxno);
					//cin.get();
					#ifdef VIEW_PROGRESS						//copied from Weighted-CHECK
						stringstream sstr("");
						res.print_first_sol(sstr);

						//print weights
						vint sol1=res.get_first_solution();			//only one solution
						int wsol=0;
						for(int i=0; i<=depth; i++){
						//	wsol+=iop.m_lw[sol1[i]];				/* TODO-data structure from CliqueWeighted */
							wsol+=g->get_wv(sol1[i]);				/* TODO-data structure from CliqueWeighted */
						}

						/*if(!is_clique(sol1)){
							LOG_ERROR("bizarre clique");
							cin.get();
						}*/

						sstr<<"["<<wsol<<"]";
						LOG_INFO(sstr.str());

					#endif	
				#endif
			}
		l_bb.set_bit(v);
		continue;
		}

		if(maxac>=maxno){
			//LOG_INFO("solution found as yet undetected");
			LISTA_BB(depth).to_old_vector_reverse(LISTA_L(depth).nodos,LISTA_L(depth).index); LISTA_L(depth).index--;
		}else{
			
			//	m_ulhs.unsel=LISTA_BB(depth);
			gap=m_ulhs.paint_and_map_CW(LISTA_BB(depth),maxno-maxac-wv,m_gm,m_urhs);									/* updates gap: should be>=0*/
			if(m_ulhs.unsel.is_empty()){
				//LOG_INFO("CUT-NO CANDIDATES PAINT AND MAP");
				l_bb.set_bit(v);
				continue;
			}
			if(m_urhs.number_of_colors()==0){																			/* all nodes have weights higher than the gap */
				LISTA_BB(depth).to_old_vector_reverse(LISTA_L(depth).nodos,LISTA_L(depth).index); LISTA_L(depth).index--;
			}else{
				/*if(nb_col==1){
				//caso particular que no afecta a la completitud aunque hace algun camino tortuoso
				}*/
				
				/////////////
				//COVERING (in this precise order seems best)
#ifdef DOUBLE_ROUND_FILTER
				m_ulhs.cover_CW_2R(/*gap,*/m_urhs,m_gm);
#else
				m_ulhs.cover_CW(/*gap,*/m_urhs,m_gm);
#endif
				//m_ulhs.cover_CW_Ord(/*gap,*/m_urhs,m_gm);

#ifdef OVERLAP_ACCORDING_TO_WEIGHT
				m_ulhs.cover_with_overlap_CW_Ord(m_urhs, m_gm);		
#else
				m_ulhs.cover_with_overlap_CW(m_urhs, m_gm);		
				
#endif					
				///////////////
				m_ulhs.unsel.to_old_vector_reverse(LISTA_L(depth).nodos,LISTA_L(depth).index); LISTA_L(depth).index--;
				if(LISTA_L(depth).index==EMPTY_ELEM){
					l_bb.set_bit(v);
					continue;
				}
			}
		}
		
///////////////////////////////////////////////////////
// CANDIDATE EXPANSION
		LISTA_BB(depth).erase_bit(m_ulhs.unsel);		/*RD idea: **TODO-OPTIMIZE? */
		m_path[depth]=v;		
		expand_w_shared_preproc_CW_RD(depth+1, maxac+wv,LISTA_BB(depth),LISTA_L(depth));  
//////////////////////////////////////////////
// BACKTRACK (does not delete v from path since it will be overwritten in the same level of search tree)
		l_bb.set_bit(v); 

	}//next candidate
}

inline
void CliqueWeightedPlus::expand_w_shared_ref (int depth, int maxac, bitarray& l_bb , nodelist_t& l_v){
///////////////////////
// Refernce recursive search algorithm which uses graph isomorphism 
// for bounding
// 
// date of creation: 22/8/17
//
// COMMENTS: l_v is computed inside the call
	
	int nb_col, gap;
	res.inc_number_of_steps();	
///////////////////////////
//DETERMINE CANDIDATE NODES TO BRANCH
	if(maxac>=maxno){
		l_bb.to_old_vector(l_v.nodos,l_v.index); l_v.index--;
	}else{ /* General case */
		m_ulhs.unsel=l_bb;
		//gap=m_ulhs.paint_and_map(l_bb,maxno-maxac,m_gm,m_urhs,nb_col);	/* updates gap: should be>=0*/
		gap=m_ulhs.paint_and_map_OPT(l_bb,maxno-maxac,m_gm,m_urhs,nb_col);	/* updates gap: should be>=0*/
		if(m_ulhs.unsel.is_empty()){
			//LOG_INFO("CUT-NO CANDIDATES PAINT AND MAP");
			return;
		}
		if(nb_col==0){													/* all nodes have weights higher than the gap */
			l_bb.to_old_vector(l_v.nodos,l_v.index); l_v.index--;
		}else{
			/*if(nb_col==1){
				//caso particular que no afecta a la completitud aunque hace algun camino tortuoso
			}*/

			//m_urhs.set_tk(1, nb_col);									/* not needed, updated in paint_and_map_OPT */
			
			/////////////
			//COVERING (in this precise order seems best)
			m_ulhs.cover(gap,nb_col,m_urhs,m_gm);
			m_ulhs.cover_with_overlap(nb_col, m_urhs, m_gm);		
			///////////////

			m_ulhs.unsel.to_old_vector(l_v.nodos,l_v.index); l_v.index--;
			if(l_v.index==EMPTY_ELEM){
				return;  
			}
		}
	}
/////////////////////////
//BRANCHING:- nodes are taken in reverse order
	int v=EMPTY_ELEM, wv=0; 
	while(l_v.index>=0){
		v=l_v.nodos[l_v.index--];
		wv=g->get_wv(v);
/////////////////////////////////
// CHILD NODE GENERATION
		AND(g->get_neighbors(v), l_bb, LISTA_BB(depth));		//optimized when place second the bitset with higher population
	
		//Leaf node: updates incumbent if necessary
		if( LISTA_BB(depth).is_empty()){
			if((maxac+wv)>maxno){
				maxno=maxac+wv;						//NEW GLOBAL OPTIMUM FOUND

				#ifdef STORE_SOLUTION
					res.set_UB(maxno);
					res.clear_all_solutions();
					m_path[depth]=v;
					res.add_solution(depth+1 /* size 1-based */, m_path);
					//LOG_INFO("SOLUTION IMPROVED:"<<maxno);
					//cin.get();
					#ifdef VIEW_PROGRESS						//copied from Weighted-CHECK
						stringstream sstr("");
						res.print_first_sol(sstr);

						//print weights
						vint sol1=res.get_first_solution();			//only one solution
						int wsol=0;
						for(int i=0; i<=depth; i++){
						//	wsol+=iop.m_lw[sol1[i]];				/* TODO-data structure from CliqueWeighted */
							wsol+=g->get_wv(sol1[i]);				/* TODO-data structure from CliqueWeighted */
						}

						/*if(!is_clique(sol1)){
							LOG_ERROR("bizarre clique");
							cin.get();
						}*/

						sstr<<"["<<wsol<<"]";
						LOG_INFO(sstr.str());

					#endif	
				#endif
			}
		l_bb.erase_bit(v);
		continue;
		}
///////////////////////////////////////////////////////
// CANDIDATE EXPANSION
		m_path[depth]=v;		
		expand_w_shared_ref(depth+1, maxac+wv,LISTA_BB(depth),LISTA_L(depth));  
//////////////////////////////////////////////
// BACKTRACK (does not delete v from path since it will be overwritten in the same level of search tree)
		l_bb.erase_bit(v); 
	}//next v candidate
}

inline
void CliqueWeightedPlus::expand_tw (int depth, int maxac, bitarray& l_bb , nodelist_t& l_v){
////////////////////////
// basic recursive search algorithm for the weighted case
// 
// date of creation: 4/7/17

	int v, wv;
	res.inc_number_of_steps();

	//main loop
	while(l_v.index>=0){
			
		//Estrategias
		v=l_v.nodos[l_v.index--];
	
		//CUT by color (since [Konc & Janecic, 2007] this is of limited use (only for first branch))
#ifdef STRONG_ROOT_COLORING
		if(depth==0){
			if( m_lcol[depth][v]<=(maxno-maxac) ){
				l_bb.erase_bit(v);
				continue;
			}
		}
#endif
		wv=g->get_wv(v);
#ifdef ROOT_VERTEX_PROGRESS
		if(depth==0)
			cout<<"root vertex: "<<"("<<v<<","<<wv<<")"<<endl;
#endif

/////////////////////////////////
// CHILD NODE GENERATION
		
		//Node generation by masking
		AND(g->get_neighbors(v), l_bb, LISTA_BB(depth));		//optimized when place second the bitset with higher population
		
		//Leaf node: updates incumbent if necessary
		if( LISTA_BB(depth).is_empty()){
			if((maxac+wv)>maxno){
				maxno=maxac+wv;						//NEW GLOBAL OPTIMUM FOUND

				#ifdef STORE_SOLUTION
					res.set_UB(maxno);
					res.clear_all_solutions();
					m_path[depth]=v;
					res.add_solution(depth+1 /* size 1-based */, m_path);
					//LOG_INFO("SOLUTION IMPROVED:"<<maxno);
					//cin.get();
					#ifdef VIEW_PROGRESS						//copied from Weighted-CHECK
						stringstream sstr("");
						res.print_first_sol(sstr);

						//print weights
						vint sol1=res.get_first_solution();			//only one solution
						int wsol=0;
						for(int i=0; i<=depth; i++){
						//	wsol+=iop.m_lw[sol1[i]];				/* TODO-data structure from CliqueWeighted */
							wsol+=g->get_wv(sol1[i]);				/* TODO-data structure from CliqueWeighted */
						}

						/*if(!is_clique(sol1)){
							LOG_ERROR("bizarre clique");
							cin.get();
						}*/

						sstr<<"["<<wsol<<"]";
						LOG_INFO(sstr.str());

					#endif	
					
				#endif
			
			}
		l_bb.erase_bit(v);
		continue;
		}

		//approx. coloring (generates child list of nodes in LISTA_L(maxac))
		int gap=paint_tw(depth, maxno-(maxac+wv) /* kmin */);  /* testing */
	/*	cout<<"GAP: "<<gap<<"\t"<<  maxno-(maxac+wv)<<endl;
		print_top_weights(iop.NB_OF_COLORS);
		iop.print_db(true,true);
		g->print_weights();*/
		

		//cin.get();

		//I/O
		/*int kmin=maxno-(maxac+wv);
		if(kmin<0){
			LOG_INFO("maxno: "<<maxno<<",maxac:"<<maxac<<" v:"<<v<<",wv:"<<wv<<",kmin: "<<kmin<<",depth:"<<depth);
			cin.get();
		}*/
			

		//cuts if there are no child nodes of v
		if(LISTA_L(depth).index<0){
			l_bb.erase_bit(v);
			continue;
		}
			
	//	LISTA_L(depth).print(cout, true); cout<<"depth"<<depth<<"-------------"<<endl;
	//	cin.get();
///////////////////////////////////////////////////////
// CANDIDATE EXPANSION

		//sets path
		m_path[depth]=v;
		/*LOG_INFO("BRANCHED:["<<v<<","<<g->get_wv(v)<<"]");
		cin.get();*/
				
		//Generacion de nuevos nodos
		expand_tw(depth+1, maxac+wv,LISTA_BB(depth),LISTA_L(depth));  

		//////////////////////////////////////////////
		// BACKTRACK (does not delete v from path since it will be overwritten in the same level of search tree)
		l_bb.erase_bit(v); 

	}// next node
}


inline
void  CliqueWeightedPlus::paint_w (int depth, int KMIN ){						
////////////
// Basic independent set coloring routine with pmaxsat branching filter for
// the weighted case.
//
// PARAMS: KMIN= maxno-maxac (gap real, including current vertex)-CHECK
//
// date of creation: 4/7/17
//
// COMMENTS: nodes must be sorted according to non-increasing weights

	int cmax=1, nBB=EMPTY_ELEM, v=EMPTY_ELEM, w_col=0;
	LISTA_L(depth).index=EMPTY_ELEM;								
	const int DEPTHPLUS1=depth+1;
	
	int pc=(m_unsel=LISTA_BB(depth)).popcn64();
	if(pc==0) return;														/*empty subgraph-should not ocurr*/		
	
/////////////////////////////////////////////
//color first KMIN nodes as usual

	bool first_vertex=true;
	iop.node_state_active.erase_bit();
	iop.NB_OF_COLORS=1;
	iop.m_colSets[iop.NB_OF_COLORS].erase_bit(false);						/*lazy erasing, will be updated later*/	
	while(true){ 
		for(int i=0; i<iop.NB_OF_BB_NODES; i++){
			iop.m_colSets[iop.NB_OF_COLORS].bb.get_bitboard(i)=m_unsel.get_bitboard(i);
		}

		//next color
		first_vertex=true;
		iop.m_colSets[iop.NB_OF_COLORS].bb.init_scan(bbo::NON_DESTRUCTIVE);
		while(true){
			v=iop.m_colSets[iop.NB_OF_COLORS].bb.next_bit(nBB,m_unsel);
			if(v==EMPTY_ELEM)
				break;

			//updates weight of coloring
			if(first_vertex){
				first_vertex=false;
				w_col+=g->get_wv(v);
				if(w_col>KMIN){											/* exit condition 2 filter */
					//restores data
					m_unsel.set_bit(v);
					iop.NB_OF_COLORS--;
					w_col-=g->get_wv(v);

					//inits iop data structures
					iop.init_inc_maxsatz();									/* includes setting all colors active */
					goto exit_partial_coloring;			
				}
			}

			//stores color label
			iop.m_colSets[iop.NB_OF_COLORS].size++;							/* the node is already there, simply increment size: equivalent to push(v) */
			iop.node_iset_no[v]=iop.NB_OF_COLORS;
			//m_lcol[DEPTHPLUS1][v]=iop.NB_OF_COLORS;						/* is this needed? */
			iop.node_state_active.set_bit(v);
				
			//checks exit condition
			if( (--pc)==0){
				//if(iop.NB_OF_COLORS==1){									/* is this needed? */
				//	int v=LISTA_BB(depth).lsbn64();
				//	LISTA_L(depth).nodos[++LISTA_L(depth).index]=v;
				//}
				return;														/*UB <= KMIN -cut */
			}

			//actual painting routine, equivalent to: m_colSets[col].bb.erase_block(nBB,g->get_neighbors(v));		
			bitarray& nbor=g->get_neighbors(v);
			for(int i=nBB; i<iop.NB_OF_BB_NODES; i++){
				iop.m_colSets[iop.NB_OF_COLORS].bb.get_bitboard(i)&= ~nbor.get_bitboard(i);
			}
		}//next node of current color

		////exit condition for partial coloring up to, and including, kmin
		//if(w_col>=KMIN ){													/* >= is strictly necessary */
		//	//iop.set_node_state_active(LISTA_BB(depth));					/* active nodes already set */			
		//	iop.init_inc_maxsatz();											/* includes setting all colors active */
		//	break;
		//}

		//increments color and erases next color in color_db
		iop.m_colSets[++iop.NB_OF_COLORS].erase_bit(false);					/* lazy erasing, will be updated later*/

	}//next color

/////////////////////////////
// pmax-sat incremental filter

exit_partial_coloring:	

	int gap=KMIN-w_col;
	m_unsel.init_scan(bbo::DESTRUCTIVE);
//	iop.print_db(true, true);
	while(true){
		int v=m_unsel.next_bit_del();
		if(v==EMPTY_ELEM){
			if(!(LISTA_L(depth).index==EMPTY_ELEM)){
				//removes as many nodes with minimum weight as possible from the candiate set before exiting
				int w=LISTA_L(depth).nodos[LISTA_L(depth).index];
				gap-=g->get_wv(w);
				while(gap>=0 ){
					LISTA_L(depth).index--;
					if(LISTA_L(depth).index==EMPTY_ELEM) break;				/* check if this is necessary */
					w=LISTA_L(depth).nodos[LISTA_L(depth).index];
					gap-=g->get_wv(w);
				}
			}
			break;
		}
		iop.add_node_to_new_color(v);
		iop.color_unit_stack.push(iop.NB_OF_COLORS);
		//iop.print_db();
		//if gap is not closed, there is no need to filter according to structure
		/*int wv=g->get_wv(v);
		if(wv<gap) {
			gap-=wv;
		}else*/ if(!iop.inc_maxsatz(v) /* conflict not found- exit */ ){ 

			//removes unpruned vertex from DB, else it is incomplete
			iop.node_state_active.erase_bit(v);
			iop.color_state_active.erase_bit(iop.NB_OF_COLORS);	
			iop.NB_OF_COLORS--;
			// iop.print_db(true, true);

			//remove v from unit color stack
			if(iop.color_unit_stack.pt>1){
				iop.color_unit_stack.stack[0]=iop.color_unit_stack.stack[iop.color_unit_stack.pt-1];
				iop.color_unit_stack.pt--;
			}else iop.color_unit_stack.pt=0;

			//adds v to the final candidate set
			LISTA_L(depth).nodos[++LISTA_L(depth).index]=v;


			////final update of candidate set: 
			//LISTA_L(depth).nodos[++LISTA_L(depth).index]=v;
			//while(true){
			//	v=m_unsel.next_bit_del();
			//	if(v==EMPTY_ELEM){
			//		iop.reset_enlarged_isets();
			//		return;									
			//	}
			//	LISTA_L(depth).nodos[++LISTA_L(depth).index]=v;
			//}		
			
		}//end-conflict not found for vertex 	
	}//next node
	
//	iop.reset_enlarged_isets();		/* reset context operations: check if this is necessary */
	return;								
}

inline
int  CliqueWeightedPlus::paint_tw (int depth, int KMIN){						
////////////
// Basic independent set coloring routine which stores the top-1 weights (difference between the highest and 
// second highest weight of the coloring)
// date of creation: 30/7/17
//
// PARAMS: KMIN= maxno-maxac (gap real, including current vertex)-CHECK
//
// RETURNS: UB determined by the coloring
//
// COMMENTS: nodes must be sorted according to non-increasing weights
//
// STATE: Developping-NOT COMPLETE
	
	int cmax=1, nBB=EMPTY_ELEM, v=EMPTY_ELEM, w_col=0, count_3=1;
	LISTA_L(depth).index=EMPTY_ELEM;								
	const int DEPTHPLUS1=depth+1;

	//check in case solution is already improved: 
	if(KMIN<0){
		//LOG_INFO("expanding without filtering");
		bitarray& bbcand=LISTA_BB(depth);
		bbcand.init_scan(bbo::NON_DESTRUCTIVE);
		while(true){
			int v=bbcand.next_bit();
			if(v==EMPTY_ELEM) break;
			LISTA_L(depth).nodos[++LISTA_L(depth).index]=v;
		}
		return 0;
	}
	///////////////////
	int pc=(m_unsel=LISTA_BB(depth)).popcn64();
	if(pc==0) return 0;														/*empty subgraph-should not ocurr*/		
/////////////////////////////////////////////
//color first KMIN nodes as usual
//(uses infraop data structures to store colors, 
// but currently does not make use of SAT pruning)
		
	iop.NB_OF_COLORS=1;
	iop.m_colSets[iop.NB_OF_COLORS].erase_bit(false);						/*lazy erasing, will be updated later*/	
	while(true){ 
		for(int i=0; i<iop.NB_OF_BB_NODES; i++){
			iop.m_colSets[iop.NB_OF_COLORS].bb.get_bitboard(i)=m_unsel.get_bitboard(i);
		}
		//next color
		count_3=1;
		iop.m_colSets[iop.NB_OF_COLORS].bb.init_scan(bbo::NON_DESTRUCTIVE);
		while(true){
			v=iop.m_colSets[iop.NB_OF_COLORS].bb.next_bit(nBB,m_unsel);
			if(v==EMPTY_ELEM)
				break;
			//updates top_weights and weight of partial coloring (w_col)
			if(count_3<=3 && count_3>=1){
				int wv=g->get_wv(v);
				if(count_3==1){
					w_col+=wv;
					if(w_col>KMIN){												/*exit condition 2 filter*/
						//restores data
						iop.NB_OF_COLORS--;										
						m_unsel.set_bit(v);	
						w_col-=wv;

						//inits iop data structures
						//iop.init_inc_maxsatz();								/* includes setting all colors active */
						goto exit_partial_coloring;			
					}					
					m_tw[iop.NB_OF_COLORS][0]=wv;	
					count_3++;
				}else if(count_3==2){
					m_tw[iop.NB_OF_COLORS][0]-=wv;
					m_tw[iop.NB_OF_COLORS][1]=wv;
					count_3++;
				}else if(count_3==3){
					m_tw[iop.NB_OF_COLORS][1]-=wv;
					count_3++;
				}
			}
			//checks exit condition
			if((--pc)==0){
				/*iop.init_inc_maxsatz();	
				iop.print_db(true, false);
				cin.get();*/

				return 0;													
			}
			//actual painting routine, equivalent to: m_colSets[col].bb.erase_block(nBB,g->get_neighbors(v));		
			bitarray& nbor=g->get_neighbors(v);
			for(int i=nBB; i<iop.NB_OF_BB_NODES; i++){
				iop.m_colSets[iop.NB_OF_COLORS].bb.get_bitboard(i)&= ~nbor.get_bitboard(i);
			}
		}//next node of current color
		iop.m_colSets[++iop.NB_OF_COLORS].erase_bit(false);					/* lazy erasing, will be updated later*/
	}//next color

/////////////////////////////
// pmax-sat incremental filter
exit_partial_coloring:	
	int gap=KMIN-w_col; top_t tk_res;
	m_unsel.init_scan(bbo::DESTRUCTIVE);
	while(true){
		int v=m_unsel.next_bit_del();				
		if(v==EMPTY_ELEM) break;
		tk_res=NONE;
		for(int col=1; col<=iop.NB_OF_COLORS; col++){
			tk_res=find_top_k(v,col,gap);						/* updates gap and top-k */
			if(tk_res!=NONE) break;
		}//next color				
		if(tk_res==NONE){
			LISTA_L(depth).nodos[++LISTA_L(depth).index]=v;
			//LOG_INFO("COVER NOT FOUND FOR: "<<v);
		}/*else break;		 top_k found */
	}//next candidate node 
	return gap;								
}

inline
int CliqueWeightedPlus::initial_bounds(int& lb, int& ub,  KCore<ugraph>* pkcore){
////////////////////////////////
// Computes initial lower and upper bounds for the MWCP, starting
// from the input values (lb, ub)
//
// RETURNS lb if solution is found (lb==ub), or 0
//
// 1.UB: sum of all the weights
// 2.LB: sum of weights of an initial greedy clique 
//		(vertices should already be ordered for improved performance)
//
// /**TODO-CHECK **/

////////////////////	
//lower bound (feasible solution)

	//initial greedy lower bound
	vint vset;
	int lb1=0;
	bitarray bb(g->number_of_vertices());
	bb.set_bit(0, g->number_of_vertices()-1);		
	bb.init_scan(bbo::DESTRUCTIVE);
	while(true){
		int v=bb.next_bit_del();
		if (v==EMPTY_ELEM) break;
		lb1+=g->get_wv(v);
		bb&=g->get_neighbors(v);	
		vset.push_back(v);
	}
	lb=max<int>(lb,lb1);

	//attempt finding a node with super-weight
	int node_max=EMPTY_ELEM;
	int w_max=g->maximum_weight(node_max);
	lb=max<int>(lb,w_max);									
	if(lb==w_max){
		vset.clear(); 
		vset.push_back(node_max);
	}

	//add feasible solution: just in case it cannot be improved!
	res.add_solution(vset);									

/////////////////////
//upper bound
	int ub1=std::accumulate(g->get_weights().begin(), g->get_weights().end(), 0, plus<int>());
	ub=min<int>(ub,ub1);

return CLQParam::is_trivial_sol(lb, ub);			/* lb if (lb==ub) */
}


#endif


