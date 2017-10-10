////////////////////////////////
// super_weight.h: interface for the SUPERW class which contains heuristics
//                  to determine if nodes belong to a maximum weighted clique
//
// initial date:01/09/17
// last update: 01/09/17
// author: pablo san segundo

#ifndef  __SUPER_WEIGHT_H__
#define  __SUPER_WEIGHT_H__

#include "../clique.h"
#include "../../../bitscan/bbalg.h"
#include "../../../utils/common.h"
#include "../../../graph/algorithms/graph_map.h"
#include "ub_weighted_clique.h"

using namespace com;												//for common types 
typedef vector<int> vint;

//////////////
// switches
//
//#define MIN_DENSITY_SUPER	 0.95			/* minimum density to consider for superweight analysis */
#define MIN_DENSITY_SUPER	 0.97			/* minimum density to consider for superweight analysis */

class SUPERW{
////////////////////////
// computes super-weight analysis

private:
	ugraph* g;								/* reference graph */		
	bitarray sel;			
	bitarray unsel;			
	bitarray bbnodes;	
	bitarray bbmap;	
public:
	SUPERW(ugraph* gout=NULL)			:g(gout){ if(g) set_graph(g);}
	virtual ~SUPERW()									{clear();}
	void set_graph(ugraph* g);		

////////////////
// Interface 

	int  search								(bitarray& bbsg, vint& lv);									/* driver*/
	int  search								(bitarray& bbsg, vint& lv, const ugraph& gw, GraphMap& gm);	/* driver*/
	
	bool is_super_weight_sum				(int v, bitarray& l_bb /* not const to iterate*/, int& ub);		
	bool is_super_weight_col				(int v, const bitarray& l_bb, int& ub);
	bool is_super_weight_col				(int v, const bitarray& l_bb, int& ub, const ugraph& gw, GraphMap& gm);

	int UB_col	 (const bitarray& bbsg, int KMIN=CLQ_MAXINT);
	int UB_col	 (const bitarray& bbsg, const ugraph& gw, int KMIN=CLQ_MAXINT);
/////////
//I/O

private:
	void init();
	void clear();
				
};

inline
int SUPERW::search (bitarray& bbsg, vint& lv,const ugraph& gw, GraphMap& gm){
/////////////////////////
// Starting from the original graph in bbsg iteratively expands superweights until it
// cannot continue any further
//
// RETURNS: lv: list of nodes expanded, bbsg: remaining subgraph to solve, 
//		    RETURN_VAL: sum of weight of nodes expanded

	int ub=0, wv=0;
	bool super_weight_found=false;
	lv.clear();
	do{
	//	bbsg.init_scan(bbo::NON_DESTRUCTIVE);
		bbsg.init_scan(bbo::NON_DESTRUCTIVE_REVERSE);
		super_weight_found=false;
		while(true){
		//	int super=bbsg.next_bit();
			int super=bbsg.previous_bit();
			if(super==EMPTY_ELEM){
				break;
			}

			if(is_super_weight_col(super,bbsg,ub,gw,gm)){
				super_weight_found=true;
				bbsg&=g->get_neighbors(super);
				lv.push_back(super);
				//LOG_INFO("SUPERWEIGHT FOUND!:"<<super<<" wv: "<<g->get_wv(super)<<" sw:"<<pc);
				wv+=g->get_wv(super);			
				break;
			}
		}

	}while(super_weight_found);

	return wv;
}

inline
int SUPERW::search (bitarray& bbsg, vint& lv){
/////////////////////////
// Starting from the original graph in bbsg iteratively expands superweights until it
// cannot continue any further
//
// RETURNS: lv: list of nodes expanded, bbsg: remaining subgraph to solve, 
//		    RETURN_VAL: sum of weight of nodes expanded

	int ub=0, wv=0;
	bool super_weight_found=false;
	lv.clear();
	do{
	//	bbsg.init_scan(bbo::NON_DESTRUCTIVE);
		bbsg.init_scan(bbo::NON_DESTRUCTIVE_REVERSE);
		super_weight_found=false;
		
		while(true){
		//	int super=bbsg.next_bit();
			int super=bbsg.previous_bit();
			if(super==EMPTY_ELEM){
				break;
			}
		//	LOG_INFO("SUPER EXAN:"<<super);
			if(is_super_weight_col(super,bbsg,ub)){
				super_weight_found=true;			
				bbsg&=g->get_neighbors(super);
				lv.push_back(super);
				//LOG_INFO("SUPERWEIGHT FOUND!:"<<super<<" wv: "<<g->get_wv(super)<<" sw:"<<ub);
				wv+=g->get_wv(super);
				break;
			}
		}

	}while(super_weight_found);

	return wv;
}

inline
int SUPERW::UB_col(const bitarray& bbsg, const ugraph& gw, int KMIN){
////////////////////////////
// greedy bound as sum of maximum weight of ISETS, considering external graph gw
//
// COMMENTS: early exit if bound is GREATER OR EQUAL TO KMIN

	int ub=0, wvISET=0, nBB=EMPTY_ELEM, v=EMPTY_ELEM, wv=0;
	int pc=((unsel=bbsg).popcn64());
	if(pc==0) return 0;					/* MUST BE*/
	
	while(true){ 
		sel=unsel;
		sel.init_scan(bbo::DESTRUCTIVE);
		wvISET=0;
		while(true){
			v=sel.next_bit_del(nBB,unsel);
			if(v==EMPTY_ELEM)
				break;
			wv=gw.get_wv(v);
			if(wvISET<wv) wvISET=wv;
			if((--pc)==0){
				ub+=wvISET;
				if(ub >= KMIN) return KMIN;
				return ub;
			}
			sel.erase_block(nBB,gw.get_neighbors(v));
		}//next node
		ub+=wvISET;
		if(ub >= KMIN) return KMIN;
	}//next ISET

	LOG_ERROR("SUPERW::UB_col()-bizarre exit");
	return ub;										/* should not reach here*/
}

inline
int SUPERW::UB_col (const bitarray& bbsg, int KMIN){
////////////////////////////
// greedy bound as sum of maximum weight of ISETS
//
// COMMENTS: early exit if bound ub is GREATER OR EQUAL TO KMIN

	int ub=0, wvISET=0, nBB=EMPTY_ELEM, v=EMPTY_ELEM, wv=0;
	int pc=((unsel=bbsg).popcn64());
	if(pc==0) return 0;					/* MUST BE*/
	
	while(true){ 
		sel=unsel;
		sel.init_scan(bbo::DESTRUCTIVE);
		wvISET=0;
		while(true){
			v=sel.next_bit_del(nBB,unsel);
			if(v==EMPTY_ELEM)
				break;
			wv=g->get_wv(v);
			if(wvISET<wv) wvISET=wv;
			if((--pc)==0){
				ub+=wvISET;
				if(ub >= KMIN) return KMIN;
				return ub;
			}
			sel.erase_block(nBB,g->get_neighbors(v));
		}//next node
		ub+=wvISET;
		if(ub >= KMIN) return KMIN;
	}//next ISET

	LOG_ERROR("SUPERW::UB_col()-bizarre exit");
	return ub;								/* should not reach here*/
}

inline
bool SUPERW::is_super_weight_col (int v, const bitarray& bbsg, int& ub){
/////////////////////////
// superweight detection in bbsg using color bound for non-neighbors of v
//
// RETURNS TRUE if superweight, ub: color bound
//
// COMMENTS:
// 1. v MUST belong to bbsg
// 2. ub is maximized if nodes are sorted according to non increasing weights in the graph

	//assert
	if(!bbsg.is_bit(v)){
		LOG_ERROR("CliqueWeightedPlus::is_super_weight()-incorrect vertex");
		return false;
	}
	
	ERASE(bbsg,g->get_neighbors(v),bbnodes);
	bbnodes.erase_bit(v);											/* careful! MUST BE */
		
	//compute color bound
	int wv=g->get_wv(v);
	ub=UB_col(bbnodes, wv+1 /* early exit */);
	return (wv>=ub);
}

inline
bool SUPERW::is_super_weight_col (int v, const bitarray& bbsg, int& ub, const ugraph& gw, GraphMap& gm){
/////////////////////////
// superweight detection given a mapping to another graph 
// (typically ordered by non-increasing weights to improve ub)
//
// RETURNS TRUE if superweight, ub: color bound
//
// COMMENTS:
// 1. v MUST belong to bbsg
	
	//assert
	if(!bbsg.is_bit(v)){
		LOG_ERROR("CliqueWeightedPlus::is_super_weight()-incorrect vertex");
		return false;
	}
	
	ERASE(bbsg,g->get_neighbors(v),bbnodes);
	bbnodes.erase_bit(v);											/* careful! MUST BE */
		
	//compute color bound with mapping to gw
	int wv=g->get_wv(v);
	gm.map_l2r(bbnodes,bbmap);
	ub=UB_col(bbmap, gw, wv+1 /* early exit */);
	return (wv>=ub);
}

inline
bool SUPERW::is_super_weight_sum (int v, bitarray& bbsg, int& ub){
////////////////////////
// Detects if v is a superweight in subgraph bbsg
//
// RETURNS TRUE if superweight, ub:sum of weights of non-neighbors of v in bbsg
//
// COMMENTS: v MUST belong to bbsg

	//assert!
	if(!bbsg.is_bit(v)){
		LOG_ERROR("SUPERW::is_super_weight()-incorrect vertex");
		return false;
	}
	
	bbsg.erase_bit(v);											/* careful! MUST BE (1)*/
	const int nBB=bbsg.number_of_bitblocks();
	ub=0;
	int wv=g->get_wv(v);
	for(int i=0; i<nBB; i++){
		BITBOARD bb = (bbsg.get_bitboard(i)&~ g->get_neighbors(v).get_bitboard(i));
		while(bb){
			int v64=BitBoard::lsb64_intrinsic(bb);
			if(v64==EMPTY_ELEM) break;
			
			ub+=g->get_wv(WMUL(i) + v64);
			if(ub>wv){
				bbsg.set_bit(v);			/* RESTORES (1) */
				return false;
			}

			bb^=Tables::mask[v64];
		}//next vertex in block
	}//next bitblock

//	LOG_INFO("SUPERWEIGHT FOUND!:"<<v);
	bbsg.set_bit(v);							/* RESTORES (1) */
	return true;
}

inline
void SUPERW::set_graph(ugraph* g) {
	clear(); 
	if(g==NULL){
		LOG_ERROR("SUPERW::set_graph()-setting invalid graph");
		return;
	}
	this->g=g;
	//m_NV=g->number_of_vertices(); 
	init();
}



inline
void SUPERW::clear(){
	//**TODO-clear info of unsel, sel?	
}

inline
void SUPERW::init(){
	clear();
	const int NV=g->number_of_vertices();
	
	try{
		unsel.init(NV);
		sel.init(NV);
		bbnodes.init(NV);	
		bbmap.init(NV);
		
	}catch(exception &e){
		e.what();
	}
}


#endif


