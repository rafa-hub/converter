//init_color_sort.cpp file: implements RLF functionality of InitColorSort class. Cannot be included in the header because it uses clique funcitonality
//date of creation: 2/11/15
//author: pss

#include "init_color_sort.h"
#include "./batch/batch_benchmark.h"
//#include "./clique/clique.h"
#include "./clique/clique_russian_doll_plus.h"
#include "./clique/clique_infra_plus.h"
#include <sstream>

vint InitColorSort::recursiveLargestFirst (vint& ord, bool& is_good_tail, clqo::init_order_t init_order){
/////////////////
// RLF over member graph: reorders vertices after removing each color set. 
//						  color sets are ordered by non-decreasing degree inside each color set
// Includes evaluation based: if is_good=TRUE the tail of the coloring has no more than TAIL_OF_COLORING colors of size 1
// RETURNS: new ordering in 'ord' , size of coloring or -1 if ERROR
//
// REMARKS: ordering[OLD_INDEX]=NEW_INDEX as usual
//
// *** NOW DEPRECATED-USE INCREMENTAL IN GENERAL (27/12/16) ****
	
	vint vres; 
	const int NV=g.number_of_vertices();
	int pc=NV, nb_of_nodes_in_color=0;
	if(pc==0){
		ord.clear();
		LOG_DEBUG("InitColorSort::RLF over an empty graph");
		return vres;	
	}
	
	//initial assignents
	ord.assign(pc, EMPTY_ELEM); 
	ugraph ugc;
	g.create_complement(ugc);
	
	vint cset;
	clqo::param_t p;								
	p.init_order=init_order;								/* (1) */ 
	p.alg=clqo::BBMC;										/* (1)-MIN_WIDTH_TYPE */
	p.init_preproc=clqo::UB;	
	
	int rv=NV;												//keeps track of removed vertices
	is_good_tail=false;
	
	//RLF iteration
	int nCol=0;
	int index=0;
	bool is_no_edges=false;
	do{
		cset=InitColorSort::maxclique(ugc,p);
		rv-=cset.size();										//if the set is single there is no way of knowing if it has been already colored (1)
		stringstream sstr("");
		sstr<<"number of vertices remaining: "<<rv;
		LOG_DEBUG(sstr.str());

		//exit condition
		if(cset.size()==1){  
			//fill remaining orders with a new color and exit
			for(int i=0; i<ord.size(); i++){
				if(ord[i]==EMPTY_ELEM){
					ord[i]=index++;
					nCol++;										//1 color per remaining vertex (color set)
					vres.push_back(1);
				}
			}

			//evaluation
			is_good_tail=(rv<=MAX_SIZE_TAIL_OF_COLORING);
			
			sstr.str("");
			sstr<<endl<<"tail of the RLF coloring: "<<rv+1<<endl;
			sstr<<"number of colors: "<<nCol<<endl;
			sstr<<"-----------------------";
			LOG_INFO(sstr.str());
			break;
		}
			
		//iterates over the color set removing edges
		nb_of_nodes_in_color=0;
		for(int i=0; i<cset.size(); i++){
			ord[cset[i]]=index++;
			ugc.remove_edges(cset[i]);
			nb_of_nodes_in_color++;
		}
		
		vres.push_back(nb_of_nodes_in_color);
		nCol++;
	}while(true);  //end sorting

			
	//check
	int nb_of_nodes=0;
	for(int c=0; c<nCol; c++){
		nb_of_nodes+=vres[c];
	}
	if(nb_of_nodes!=NV){
		LOG_ERROR("recursiveLargestFirst: bizarre number of vertices in color sets");
		vint myv;
		vres.swap(myv);
		return vres;
	}

	return vres;
}

vint InitColorSort::recursiveLargestFirst_INC (vint& ord, bool& is_good_tail){
/////////////////
// RLF over member graph: an ordering is fixed initially and a clique partition of
//                        the complement graph is obtained in that fixed order
// Currently RDOLLS is used to obtain each clique             
//		
// PARAMS: 
// 1.ord: output ordering which is a clique cover in format  ord[OLD_INDEX]=NEW_INDEX, as usual 
// 2. is_good_tail: TRUE if the 'tail' of the coloring has no more than TAIL_OF_COLORING colors of size 1
//                  #define EVAL_OF_COLOR_TAIL_IN_RLF_COND controls its use
//
// RETURNS: vcol[color label]=size of the coloring
//
// REMARKS
// 1.Uses RDOLL without p-maxsat filter (assumes maxclique problem is easy)

	vint vcol, cset; 
	is_good_tail=true;							
	stringstream sstr("");
	const int NV=g.number_of_vertices();
	if(NV==0){
		ord.clear();
		LOG_ERROR("InitColorSort::recursiveLargestFirst_INC()-RLF over an empty graph");
		return vcol;	
	}
	
	//initial assignents
	bitarray bbs(NV); bbs.set_bit(0,NV-1);		/* induced subgraph */
	ord.assign(NV, EMPTY_ELEM); 
	ugraph ugc;
	g.create_complement(ugc);
		
	//sort ugc by some criteria at the beginning:
	//1.MAX_WIDTH if RDOLL is going to be used for RLF (which is currently fixed)
	Decode d;
	CliqueSort<ugraph> cs(ugc);
	//cs.reorder(cs.new_order(clqo::init_order_t::MAX_WIDTH_MAX_TIE_STATIC,gbbs::place_t::PLACE_LF),d);		/* solves some csp:black-hole! */
	cs.reorder(cs.new_order(clqo::init_order_t::MIN_WIDTH_MIN_TIE_STATIC,gbbs::place_t::PLACE_FL),d);		/* default for CSP */
		
	//cs.reorder(cs.new_order(clqo::init_order_t::MAX_WIDTH_MAX_TIE_STATIC,gbbs::place_t::PLACE_FL),d);	
	//cs.reorder(cs.new_order(clqo::init_order_t::MIN_WIDTH_MIN_TIE_STATIC,gbbs::place_t::PLACE_LF),d);
	////////////

	clqo::param_t p;
	p.alg=clqo::BBMCR_NOX_DOLL;			/* RDOLL without pmaxsat: the graphs are too easy anyway */
	CliqueDollPlus cg(&ugc,p);

	/*p.alg=clqo::BBMCL;		
	CliqueInfraPlus cg(&ugc,p);*/

////////////////
//main loop-iteratively find maximum cliques
	int nCol=0, index=0;
	bool is_no_edges=false; 

	cset=InitColorSort::maxclique(cg, bbs, true /* allocation */);	
	do{
		//exit condition I
		if(cset.size()==1){
			bbs.init_scan(bbo::NON_DESTRUCTIVE);
			while(true){
				int v=bbs.next_bit();
				if(v==EMPTY_ELEM) break;
				if(ord[v]!=EMPTY_ELEM){
					LOG_ERROR("InitColorSort::maxclique()-bizarre ordering");
				}
				ord[v]=index++;	nCol++;
				vcol.push_back(1);
			}
					
			//evaluation
			int tail=bbs.popcn64();
#ifdef EVAL_OF_COLOR_TAIL_IN_RLF_COND
			is_good_tail=tail<=MAX_SIZE_TAIL_OF_COLORING;
#endif

			//LOG
			sstr.clear();  sstr.str("");
			sstr<<endl<<"tail of the RLF coloring: "<<tail<<endl;
			sstr<<"number of colors: "<<nCol<<endl;
			sstr<<"-----------------------";
			LOG_INFO(sstr.str());
			break;
		}//end of exit condition
			
		//updates state for current clique in this iteration
		for(int i=0; i<cset.size(); i++){
			ord[cset[i]]=index++;
			bbs.erase_bit(cset[i]);			/* deletes nodes for next iteration */
		}		
		vcol.push_back(cset.size());
		nCol++;
		LOG_INFO("number of vertices remaining: "<<bbs.popcn64());
		
		//exit condition II
		if(bbs.is_empty()) break;			
		
		//compute next maxclique
		cset=InitColorSort::maxclique(cg, bbs, false /* no allocation-incremental */);		
		
	}while(true);  /* next clique step */

////////////////////////////	
//decode ordering 
	vint ord_dec=d.get_first_ordering();	/* MAP[NEW]=OLD */
	vint ord_aux(NV, EMPTY_ELEM);
	for(int i=0; i<ord.size(); i++){
		ord_aux[ord_dec[i] /* new->old*/]=ord[i];		/* new index */
	}
	copy(ord_aux.begin(), ord_aux.end(), ord.begin());

///////////
//I/O
	//cout<<"ORDERING"<<endl;
	//com::stl::print_collection(ord); 
	//cout<<endl<<"--------------------------------"<<endl;

//////////////////
//TESTS-number of nodes per color / nb of colors
	if(nCol!=vcol.size()){
		LOG_ERROR("InitColorSort::recursiveLargestFirst_INC()-bizarre number of color sets");
		vint myv; 	vcol.swap(myv);
		return vcol;
	}

	int nb_of_nodes=0;
	for(int c=0; c<vcol.size(); c++){
		nb_of_nodes+=vcol[c];
	}
	if(nb_of_nodes!=NV){
		LOG_ERROR("InitColorSort::recursiveLargestFirst_INC()-bizarre number of vertices in color sets");
		vint myv; vcol.swap(myv);		
		return vcol;
	}
////////////////

	return vcol;		
}

vint InitColorSort::recursiveLargestFirst_INC (vint& ord, vint& tail_rlf_dec, bool& is_good_tail){
/////////////////
// RLF over member graph: an ordering is fixed initially and a clique partition of
//                        the complement graph is obtained in that fixed order
// Currently RDOLLS is used to obtain each clique             
//		
// PARAMS: 
// 1. ord: output ordering which is a clique cover in format  ord[OLD_INDEX]=NEW_INDEX, as usual 
// 2. tail_rlf_dec: list of nodes that make up the tail (referred to the original graph)
// 2. is_good_tail: TRUE if the 'tail' of the coloring has no more than TAIL_OF_COLORING colors of size 1
//                  #define EVAL_OF_COLOR_TAIL_IN_RLF_COND controls its use
//
// RETURNS: vcol[color label]=size of the coloring
//
// REMARKS
// 1.Uses RDOLL without p-maxsat filter (assumes maxclique problem is easy)

	vint vcol, cset; 
	is_good_tail=true;							
	stringstream sstr("");
	const int NV=g.number_of_vertices();
	tail_rlf_dec.clear();
	if(NV==0){
		ord.clear();		
		LOG_ERROR("InitColorSort::recursiveLargestFirst_INC()-RLF over an empty graph");
		return vcol;	
	}
	
	//initial assignents
	bitarray bbs(NV); bbs.set_bit(0,NV-1);		/* induced subgraph */
	ord.assign(NV, EMPTY_ELEM); 
	ugraph ugc;
	g.create_complement(ugc);
		
	//sort ugc by some criteria at the beginning:
	//1.MAX_WIDTH if RDOLL is going to be used for RLF (which is currently fixed)
	Decode d;
	CliqueSort<ugraph> cs(ugc);
	//cs.reorder(cs.new_order(clqo::init_order_t::MAX_WIDTH_MAX_TIE_STATIC,gbbs::place_t::PLACE_LF),d);		/* solves 3 csp::black-holes! */
	cs.reorder(cs.new_order(clqo::init_order_t::MIN_WIDTH_MIN_TIE_STATIC,gbbs::place_t::PLACE_FL),d);		/* default for CSP */
		
	//cs.reorder(cs.new_order(clqo::init_order_t::MAX_WIDTH_MAX_TIE_STATIC,gbbs::place_t::PLACE_FL),d);	
	//cs.reorder(cs.new_order(clqo::init_order_t::MIN_WIDTH_MIN_TIE_STATIC,gbbs::place_t::PLACE_LF),d);
	////////////

	clqo::param_t p;
	p.alg=clqo::BBMCR_NOX_DOLL;			/* RDOLL without pmaxsat: the graphs are too easy anyway */
	CliqueDollPlus cg(&ugc,p);

	/*p.alg=clqo::BBMCL;		
	CliqueInfraPlus cg(&ugc,p);*/

////////////////
//main loop-iteratively find maximum cliques
	int nCol=0, index=0;
	bool is_no_edges=false; 

	cset=InitColorSort::maxclique(cg, bbs, true /* allocation */);	
	do{
		//exit condition I
		if(cset.size()==1){
			tail_rlf_dec.clear();
			bbs.init_scan(bbo::NON_DESTRUCTIVE);
			while(true){
				int v=bbs.next_bit();
				if(v==EMPTY_ELEM) break;
				if(ord[v]!=EMPTY_ELEM){
					LOG_ERROR("InitColorSort::maxclique()-bizarre ordering");
				}
				ord[v]=index++;	nCol++;
				vcol.push_back(1);
				tail_rlf_dec.push_back(d.get_first_ordering()[v]);  /* old coordinates */
			}
					
			//evaluation
			int tail=bbs.popcn64();

			//check
			if(tail!=tail_rlf_dec.size()){
				LOG_ERROR("InitColorSort::recursiveLargestFirst_INC-bizarre tail of RLF");
			}

#ifdef EVAL_OF_COLOR_TAIL_IN_RLF_COND
			is_good_tail=tail<=MAX_SIZE_TAIL_OF_COLORING;
#endif

			//LOG
			sstr.clear();  sstr.str("");
			sstr<<endl<<"tail of the RLF coloring: "<<tail<<endl;
			sstr<<"number of colors: "<<nCol<<endl;
			sstr<<"-----------------------";
			LOG_INFO(sstr.str());
			break;
		}//end of exit condition
			
		//updates state for current clique in this iteration
		for(int i=0; i<cset.size(); i++){
			ord[cset[i]]=index++;
			bbs.erase_bit(cset[i]);			/* deletes nodes for next iteration */
		}		
		vcol.push_back(cset.size());
		nCol++;
		LOG_INFO("number of vertices remaining: "<<bbs.popcn64());
		
		//exit condition II
		if(bbs.is_empty()) break;			
		
		//compute next maxclique
		cset=InitColorSort::maxclique(cg, bbs, false /* no allocation-incremental */);		
		
	}while(true);  /* next clique step */

////////////////////////////	
//decode ordering 
	vint ord_dec=d.get_first_ordering();	/* MAP[NEW]=OLD */
	vint ord_aux(NV, EMPTY_ELEM);
	for(int i=0; i<ord.size(); i++){
		ord_aux[ord_dec[i] /* new->old*/]=ord[i];		/* new index */
	}
	copy(ord_aux.begin(), ord_aux.end(), ord.begin());

///////////
//I/O
	//cout<<"ORDERING"<<endl;
	//com::stl::print_collection(ord); 
	//cout<<endl<<"--------------------------------"<<endl;

//////////////////
//TESTS-number of nodes per color / nb of colors
	if(nCol!=vcol.size()){
		LOG_ERROR("InitColorSort::recursiveLargestFirst_INC()-bizarre number of color sets");
		vint myv; 	vcol.swap(myv);
		return vcol;
	}

	int nb_of_nodes=0;
	for(int c=0; c<vcol.size(); c++){
		nb_of_nodes+=vcol[c];
	}
	if(nb_of_nodes!=NV){
		LOG_ERROR("InitColorSort::recursiveLargestFirst_INC()-bizarre number of vertices in color sets");
		vint myv; vcol.swap(myv);		
		return vcol;
	}
////////////////

	return vcol;		
}

vint InitColorSort::maxclique(ugraph& ug, const clqo::param_t& p){
///////////////////
// Interface which solves max clique for g with params in p
// returns solution (empty if graph is empty or an error has ocurred)
//
// REMARKS
// 1. Does not modify the input graph ug
// 2. Application to RLF now deprecated: use InitColorSort::maxclique(CliqueDollPlus&, ...)
//    instead
	
	//empty graph check
	if(ug.number_of_vertices()==0){
		LOG_DEBUG("InitColorSort::maxclique over an empty graph");
		vint vempty;
		return vempty;
	}

	//no edges check: returns first vertex as max clique
	if(ug.number_of_edges()==0){
		LOG_DEBUG("InitColorSort::maxclique over a graph with no edges");
		vint res;
		res.push_back(0);
		return res;
	}
	
///////////////
//max clique computation
	BatchCLQBk<ugraph, Clique<ugraph> > batch;

	switch(p.alg){
	case clqo::algorithm_t::BBMC:					/* default */
		batch.add_test<Clique<ugraph>>(p);
		break;
	case clqo::algorithm_t::BBMC_DOLL:				/* russian doll */
	case clqo::algorithm_t::BBMCR_DOLL:
		batch.add_test<CliqueDollPlus>(p);
		break;
	default:
		LOG_ERROR("InitColorSort::maxclique(..)-unknown algorithm");
		vint vempty;
		return vempty;
	}
	
	//returns an empty graph in case of error
	return batch.run_single_instance(ug,cout);
}

vint InitColorSort::maxclique (CliqueDollPlus& cq /*CliqueInfraPlus& cq*/ /*ugraph& g, const clqo::param_t& p,*/, bitarray& bbs, bool init){
///////////////////
// Interface which solves max clique for induced subgraph bbs of g with configuration params in p
// with RDOLL algorithm
// date of creation: 27/12/2016
// COMMENTS: Extends maxclique interface for any induced subgraph. Application RLF-type
//           sorting, which can now be incremental
//
// RETURNS: clique solution (empty if graph is empty or an error has ocurred)
//
// REMARKS: 
// 1. does not modify the input graph ug
// 2. assumes input graph is not empty
	
	//empty subgraph check
	if(bbs.is_empty()){
		LOG_INFO("InitColorSort::maxclique()- over an empty subgraph");
		vint vempty;
		return vempty;
	}
		
///////////////
//max clique computation
	
	if(init){												
		clqo::search_alloc_t info;
		info.set(clqo::search_alloc_t::ALLOC_COLOR_SETS);			/* check, does not seem necessary */
		if(cq.set_up_subgraph(bbs, &info)==0){
			cq.run_subgraph(bbs);
		}
	}else if(cq.set_up_subgraph(bbs)==0){
		cq.run_subgraph(bbs);
//		bbs.print();
	}

	return cq.decode_first_solution();		/* no decoding necessary, will return first solution as is */
}
