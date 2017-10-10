//--------------------------------------------
// test_weighted_clique.cpp: some tests for weighted cliques

#include "gtest/gtest.h"
#include "graph/graph_gen.h"
#include "graph/algorithms/decode.h"
#include "utils/common.h"
#include "utils/logger.h"
#include "bitscan/bbalg.h"
#include "../clique/clique.h"
#include "../clique/clique_weighted.h"					//currently deprecated-4/7/17
#include "../clique/clique_weighted_plus.h"
//#include "../clique/heur/ub_weighted_clique.h"
#include "../clique/heur/super_weight.h"
#include "../clique/heur/cover_mwss.h"
#include <algorithm>

using namespace std;

TEST(Weighted_Clique_Plus, cover_MSS){
/////////////////////
// Removes no nodes in DIMACS, apparently only for sparse graphs

	LOG_INFO("Weighted_Clique_Plus:cover_MSS-----------------------");
		
	ugraph ug("brock200_1.clq"); const int W=2802;
//	ugraph ug("san400_0.7_2.clq"); const int W=3080;		/* 68s!*/
//	ugraph ug("san400_0.7_3.clq"); const int W=2753;
//	ugraph ug("sanr400_0.7.clq"); const int W=3061;
//	ugraph ug("sanr200_0.9.clq"); const int W=5090;
//	ugraph ug("p_hat300-3.clq"); const int W=3945;
//	ugraph ug("p_hat500-3.clq"); const int W=5333;
//	ugraph ug("p_hat500-2.clq"); const int W=3889;
//	ugraph ug("p_hat700-2.clq"); const int W=5250;
//	ugraph ug("p_hat1500-1.clq"); const int W=1609;

//	ugraph ug("r200_0.98_0.txt"); 
//	ugraph ug("r200_0.98_1.txt"); 
//	ugraph ug("r200_0.98_3.txt"); 
	const int NV=ug.number_of_vertices();
		
	COVER cv(&ug);	
	cv.initialize();

	//To TEST
	//int nb_nodes=cv.next_col();
	//cv.print_db();
	//	
	////test independent sets.
	//ugraph ugc;
	//ug.create_complement(ugc);
	//Clique<ugraph> clq_test(&ugc, clqo::param_t());
	//vint lv;
	//int* col=cv.get_col(1);
	//int POINTER_COL=0, v=EMPTY_ELEM;
	//while((v=col[POINTER_COL++])!=COVER::NONE){
	//	lv.push_back(v);
	//}
	//com::stl::print_collection(lv);
	//
	////TESTS
	//EXPECT_TRUE(clq_test.is_clique(lv));
	//EXPECT_EQ(nb_nodes,lv.size());
	
	//next_col
	cv.initialize();
	cout<<"UB: "<<cv.start();
	

	//cv.print_db();
	//cv.print_db(1);


		
	LOG_INFO("PRESS ANY KEY");
	cin.get();
	LOG_INFO("-------------------------------------------------------")	;
}

TEST(Weighted_Clique_Plus, init_node_removal){
/////////////////////
// Removes no nodes in DIMACS, apparently only for sparse graphs

	LOG_INFO("Weighted_Clique_Plus:init_node_removal-----------------------");
		
//	ugraph ug("brock200_1.clq"); const int W=2802;
//	ugraph ug("san400_0.7_2.clq"); const int W=3080;		/* 68s!*/
//	ugraph ug("san400_0.7_3.clq"); const int W=2753;
//	ugraph ug("sanr400_0.7.clq"); const int W=3061;
//	ugraph ug("sanr200_0.9.clq"); const int W=5090;
//	ugraph ug("p_hat300-3.clq"); const int W=3945;
//	ugraph ug("p_hat500-3.clq"); const int W=5333;
//	ugraph ug("p_hat500-2.clq"); const int W=3889;
//	ugraph ug("p_hat700-2.clq"); const int W=5250;
	ugraph ug("p_hat1500-1.clq"); const int W=1609;

//	ugraph ug("r200_0.98_0.txt"); 
//	ugraph ug("r200_0.98_1.txt"); 
//	ugraph ug("r200_0.98_3.txt"); 
	const int NV=ug.number_of_vertices();

	vint lv;
	bitarray bb(NV);
	bb.set_bit(0,NV-1);
	
	//To TEST
	
	ug.print_data();
	clqo::param_t param;
	param.lb=W-5;
//	param.lb=W-1000;
	param.isomorphism=true;										/* default mapping */
	/*param.iso_map.lhs=pair<gbbs::sort_t,gbbs::place_t>(gbbs::MIN_DEG_DEGEN_TIE_STATIC, gbbs::PLACE_LF);
	param.iso_map.rhs=pair<gbbs::sort_t,gbbs::place_t>(gbbs::MAX_WEIGHT, gbbs::PLACE_FL);*/


	param.alg=clqo::BBMC_WEIGHTED_SHARED_PREPROC_CW_SUPER_WEIGHT;
//	param.alg=clqo::BBMC_WEIGHTED_SHARED_PREPROC_CW_RD;
//	param.alg=clqo::BBMC_WEIGHTED_SHARED_PREPROC_CW;
//	param.alg=clqo::BBMC_WEIGHTED_SHARED_PREPROC;
//	param.alg=clqo::BBMC_WEIGHTED_SHARED_TESTS;
//	param.alg=clqo::BBMC_WEIGHTED_SHARED_REF;
//	param.alg=clqo::BBMC_WEIGHTED_BASIC;
//	param.init_order=clqo::init_order_t::MAX_WEIGHTED;				//non-increasing weights with max_deg flavour
//	param.init_order=clqo::init_order_t::MIN_WIDTH;
	param.init_preproc=clqo::init_preproc_t::UB;
	CliqueWeightedPlus cug(&ug, param);
	cug.set_up();
	
	//To TEST
	LOG_INFO("NODE REMOVAL ANALYSIS");
	int nb_removed=cug.initial_node_removal(cug.get_result().get_lower_bound(),lv);
	com::stl::print_collection(lv);
	EXPECT_EQ(0,nb_removed);

		
	LOG_INFO("PRESS ANY KEY");
	cin.get();
	LOG_INFO("-------------------------------------------------------")	;
}

TEST(Weighted_Clique_Plus, new_superweight_search){
	LOG_INFO("Weighted_Clique_Plus:superweight_search-----------------------");
	//ugraph ug(NV);
	//ug.add_edge(0, 1);		
	//ug.add_edge(1, 2);
	//ug.add_edge(2, 3);
	//ug.add_edge(3, 4);
	//ug.add_edge(4, 0);  //5-odd cycle
	//ug.add_edge(4, 5);  //extra node

	//ug.init_wv();
	//ug.set_wv(0, 3); ug.set_wv(1, 5); ug.set_wv(2, 2); ug.set_wv(3, 1); 		
	//ug.set_wv(4, 4); ug.set_wv(5, 6);

//	ugraph ug("brock200_1.clq"); const int W=2802;
//	ugraph ug("san400_0.7_2.clq"); const int W=3080;		/* 68s!*/
//	ugraph ug("san400_0.7_3.clq"); const int W=2753;
//	ugraph ug("sanr400_0.7.clq"); const int W=3061;
//	ugraph ug("sanr200_0.9.clq"); const int W=5090;
//	ugraph ug("p_hat300-3.clq"); const int W=3945;
//	ugraph ug("p_hat500-3.clq"); const int W=5333;
//	ugraph ug("p_hat500-2.clq"); const int W=3889;
//	ugraph ug("p_hat700-2.clq"); const int W=5250;
//	ugraph ug("p_hat1500-1.clq"); const int W=1609;

//	ugraph ug("r200_0.98_0.txt"); 
	ugraph ug("r200_0.98_1.txt"); 
//	ugraph ug("r200_0.98_3.txt"); 
	const int NV=ug.number_of_vertices();
		
	//DEPTH_1-SUPERWEIGHTS EXIST FOR ROOT SUPERWEIGHTS
	GraphSort<ugraph> gs(ug);
	gs.reorder(gs.new_order(gbbs::MAX_WEIGHT,gbbs::PLACE_FL));
	//gs.reorder(gs.new_order(gbbs::MIN_DEG_DEGEN,gbbs::PLACE_FL));
	//ug.print_weights();
	
	SUPERW sw(&ug);
	vint lv;
	bitarray bb(NV);
	bb.set_bit(0,NV-1);
	
	//To TEST
	int sumw=sw.search(bb,lv);
	ug.print_weights(bb); cout<<endl;
	com::stl::print_collection(lv); cout<<"["<<lv.size()<<"]";

	//checking:
	//I. is clique
	CliqueWeightedPlus cug(&ug, clqo::param_t());
	EXPECT_TRUE(cug.Clique<ugraph>::is_clique(lv));


	//II.sum is correct
	int wv=0;
	for(int i=0; i<lv.size(); i++){
		wv+=ug.get_wv(lv[i]);
	}
	EXPECT_EQ(wv, sumw);

		
		
	LOG_INFO("PRESS ANY KEY");
	cin.get();
	LOG_INFO("-------------------------------------------------------")	;
}



TEST(Weighted_Clique_Plus, bound_color){
	LOG_INFO("Weighted_Clique_Plus:bound_color-----------------------");

	ugraph ug("brock200_1.clq"); const int W=2802;
//	ugraph ug("san400_0.7_2.clq"); const int W=3080;		/* 68s!*/
//	ugraph ug("san400_0.7_3.clq"); const int W=2753;
//	ugraph ug("sanr400_0.7.clq"); const int W=3061;
//	ugraph ug("sanr200_0.9.clq"); const int W=5090;
//	ugraph ug("p_hat300-3.clq"); const int W=3945;
//	ugraph ug("p_hat500-3.clq"); const int W=5333;
//	ugraph ug("p_hat500-2.clq"); const int W=3889;
//	ugraph ug("p_hat700-2.clq"); const int W=5250;
//	ugraph ug("p_hat1500-1.clq"); const int W=1609;

//	ugraph ug("r200_0.98_0.txt"); const int W=10577;		/* 68s!*/
//	ugraph ug("r200_0.98_1.txt"); const int W=10292;		/* 68s!*/
//	ugraph ug("r200_0.98_3.txt"); 
	const int NV=ug.number_of_vertices();
	
	
	//DEPTH_1-SUPERWEIGHTS EXIST FOR ROOT SUPERWEIGHTS
	GraphSort<ugraph> gs(ug);
	gs.reorder(gs.new_order(gbbs::MAX_WEIGHT,gbbs::PLACE_FL));			/*seems clearly best for color bound */
//	gs.reorder(gs.new_order(gbbs::MAX_WEIGHT,gbbs::PLACE_LF));			
//	gs.reorder(gs.new_order(gbbs::MIN_DEG_DEGEN,gbbs::PLACE_LF));
//	gs.reorder(gs.new_order(gbbs::MAX_DEG_DEGEN,gbbs::PLACE_FL));
	ug.print_weights();

	
	bitarray bb(NV);
	bb.set_bit(0,NV-1);
	
	//To TEST
	int ubsum=UBWC::UB_sum(ug,bb);
	int ubcol=UBWC::UB_col(ug,bb);
	EXPECT_GE(ubsum, ubcol);
	
	LOG_INFO("ubsum: "<<ubsum<<" ubcol:"<<ubcol);

	//bound SUPER
	SUPERW sw(&ug);
	//To TEST
		
	int ubcol_super=sw.UB_col(bb,ug);
	EXPECT_EQ(ubcol, ubcol_super);
	LOG_INFO("SUPER::ubcol:"<<ubcol_super);			
		
	LOG_INFO("PRESS ANY KEY");
	cin.get();
	LOG_INFO("-------------------------------------------------------")	;
}

TEST(Weighted_Clique_Plus, new_superweight){
	
//	ugraph ug("brock200_1.clq"); const int W=2802;
//	ugraph ug("san400_0.7_2.clq"); const int W=3080;		/* 68s!*/
//	ugraph ug("san400_0.7_3.clq"); const int W=2753;
//	ugraph ug("sanr400_0.7.clq"); const int W=3061;
//	ugraph ug("sanr200_0.9.clq"); const int W=5090;
//	ugraph ug("p_hat300-3.clq"); const int W=3945;
//	ugraph ug("p_hat500-3.clq"); const int W=5333;
//	ugraph ug("p_hat500-2.clq"); const int W=3889;
//	ugraph ug("p_hat700-2.clq"); const int W=5250;
//	ugraph ug("p_hat1500-1.clq"); const int W=1609;
	ugraph ug("r200_0.98_0.txt"); 
//	ugraph ug("r200_0.98_1.txt"); 
//	ugraph ug("r200_0.98_3.txt"); 
	const int NV=ug.number_of_vertices();
	
	/*int vwmax;
	int wv=ug.maximum_weight(vwmax);*/
		
	//DEPTH_1-SUPERWEIGHTS EXIST FOR ROOT SUPERWEIGHTS
	//GraphSort<ugraph> gs(ug);
	////gs.reorder(gs.new_order(gbbs::MAX_WEIGHT,gbbs::PLACE_FL));
	//gs.reorder(gs.new_order(gbbs::MIN_DEG_DEGEN,gbbs::PLACE_FL));

	ug.print_weights();

	SUPERW sw(&ug);
	bitarray bb(NV);
	bb.set_bit(0,NV-1);
	
	int pc=0;
	vint lvroot;
	for(int i=0; i<NV; i++){
		if(sw.is_super_weight_sum(i, bb, pc)){
			LOG_INFO("SW-ROOT:"<<i<<" wv:"<<ug.get_wv(i)<<" super-peso: "<<pc);
			lvroot.push_back(i);
		//	break;
		}else{
		//	LOG_INFO("FAILED SROOT:"<<i<<" wv:"<<ug.get_wv(i)<<" super-peso: "<<pc);
		//	EXPECT_NE(cug.super_weight(i,bb),pc);

		}
	}

	//cin.get();

	vint lvd1;
//	for(int v_root=0; v_root<NV; v_root++){
	for(vint::iterator it=lvroot.begin(); it!=lvroot.end(); it++){
		bb&=ug.get_neighbors(*it);
		bb.init_scan(bbo::NON_DESTRUCTIVE);
		while(true){
			int v=bb.next_bit();
			if(v==EMPTY_ELEM) break;

			if(sw.is_super_weight_sum(v, bb, pc)){
				LOG_INFO("V_ROOT:"<<*it<<" SW-D1:"<<v<<" wv:"<<ug.get_wv(v)<<" super-peso: "<<pc);
				lvd1.push_back(v);
			}else{
			//	LOG_INFO("DEPTH-1-PESO: "<<v<<" wv: "<<ug.get_wv(v)<<" super-peso: "<<cug.super_weight(v,bb));
			}
		}
	}	
		
	LOG_INFO("PRESS ANY KEY");
	cin.get();
	LOG_INFO("-------------------------------------------------------")	;
}

TEST(Weighted_Clique_Plus, setup){
	LOG_INFO("Weighted_Clique_Plus:setup-----------------------");
			
	//const int NV=6;
	//ugraph ug(NV);
	//ug.add_edge(0, 1);		
	//ug.add_edge(1, 2);
	//ug.add_edge(2, 3);
	//ug.add_edge(3, 4);
	//ug.add_edge(4, 0);  //5-odd cycle
	//ug.add_edge(4, 5);  //extra node

	//ug.init_wv();
	//ug.set_wv(0, 3); ug.set_wv(1, 5); ug.set_wv(2, 2); ug.set_wv(3, 1); 		
	//ug.set_wv(4, 4); ug.set_wv(5, 6);

	ugraph ug("brock200_1.clq"); const int W=2802;
//	ugraph ug("san400_0.7_2.clq"); const int W=3080;		/* 68s!*/
//	ugraph ug("san400_0.7_3.clq"); const int W=2753;
//	ugraph ug("sanr400_0.7.clq"); const int W=3061;
//	ugraph ug("sanr200_0.9.clq"); const int W=5090;
//	ugraph ug("san200_0.9_3.clq"); const int W=4714;
//	ugraph ug("p_hat300-3.clq"); const int W=3945;
//	ugraph ug("p_hat500-3.clq"); const int W=5333;
//	ugraph ug("p_hat500-2.clq"); const int W=3889;
//	ugraph ug("p_hat700-2.clq"); const int W=5250;
//	ugraph ug("p_hat1500-1.clq"); const int W=1609;

//	ugraph ug("r200_0.98_0.txt"); const int W=10577;		/* 68s!*/
//	ugraph ug("r200_0.98_1.txt"); const int W=10292;		/* 68s!*/
//	ugraph ug("r200_0.98_3.txt"); 

	//ordering attemp to sort weights
	//GraphSort<ugraph> gs(ug);
	//gs.reorder(gs.new_order(gbbs::MAX_WEIGHT,PLACE_FL));
	//ug.print_weights();

	ug.print_data();
	clqo::param_t param;
	param.lb=W-5;
//	param.lb=W-1000;
	param.isomorphism=true;										/* default mapping */
	/*param.iso_map.lhs=pair<gbbs::sort_t,gbbs::place_t>(gbbs::MIN_DEG_DEGEN_TIE_STATIC, gbbs::PLACE_LF);
	param.iso_map.rhs=pair<gbbs::sort_t,gbbs::place_t>(gbbs::MAX_WEIGHT, gbbs::PLACE_FL);*/


//	param.alg=clqo::BBMC_WEIGHTED_SHARED_PREPROC_CW_RD;
	param.alg=clqo::BBMC_WEIGHTED_SHARED_PREPROC_CW_SUPER_WEIGHT;
//	param.alg=clqo::BBMC_WEIGHTED_SHARED_PREPROC_CW;
//	param.alg=clqo::BBMC_WEIGHTED_SHARED_PREPROC;
//	param.alg=clqo::BBMC_WEIGHTED_SHARED_TESTS;
//	param.alg=clqo::BBMC_WEIGHTED_SHARED_REF;
//	param.alg=clqo::BBMC_WEIGHTED_BASIC;
//	param.init_order=clqo::init_order_t::MAX_WEIGHTED;				//non-increasing weights with max_deg flavour
//	param.init_order=clqo::init_order_t::MIN_WIDTH;
	param.init_preproc=clqo::init_preproc_t::UB;
	CliqueWeightedPlus cug(&ug, param);
	cug.set_up();
	cug.run();	
	//**TODO- TESTS
	
	LOG_INFO("PRESS ANY KEY");
	cin.get();
	LOG_INFO("-------------------------------------------------------")	;
}

TEST(UBWC_cover_shared, Dimacs_depth_map){
	LOG_INFO("UBWC_cover:Dimacs_map-----------------------");

//	ugraph ug("brock200_1.clq"); const int W=2802;
//	ugraph ug("san400_0.7_2.clq"); const int W=3080;		/* 68s!*/
//	ugraph ug("san400_0.7_3.clq"); const int W=2753;
//	ugraph ug("sanr400_0.7.clq"); const int W=3061;
//	ugraph ug("sanr200_0.9.clq"); const int W=5090;
//	ugraph ug("p_hat300-3.clq"); const int W=3945;
//	ugraph ug("p_hat500-3.clq"); const int W=5333;
//  ugraph ug("p_hat1500-1.clq"); const int W=1609;

	ugraph ug("r200_0.98_0.txt"); const int W=10577;		/* 68s!*/
//	ugraph ug("r200_0.98_1.txt"); const int W=10292;		/* 68s!*/

	ugraph ugw(ug);
	const int NV=ug.number_of_vertices();
	const int DEPTH=2;

	//I/O-graph
	stringstream sstr;
	ug.print_data(true, sstr);
	LOG_INFO(sstr.str());
	
		
	//build mapping
	GraphMap gm;
	gm.build_mapping(ug,MIN_DEG_DEGEN,PLACE_LF,MAX_WEIGHT,PLACE_FL,"MIN_DEG_DEGEN-LF","MAX_WEIGHT-FL");
	
	//sort ugw by weight
	GraphSort<ugraph> ow(ugw);
	vint lvw=ow.new_order(gbbs::sort_t::MAX_WEIGHT, gbbs::place_t::PLACE_FL);
	ow.reorder(lvw);

	//sort ug by degree
	GraphSort<ugraph> od(ug);
	vint lvd=od.new_order(gbbs::sort_t::MIN_DEG_DEGEN, gbbs::place_t::PLACE_LF);
	od.reorder(lvd);

//////////////////////////
// paint and map
	//init UBs
	UBWC ud(&ug), uw(&ugw);
	ud.init(); 
	uw.init();

	bitarray bb(NV);
	bb.set_bit(0,NV-1);

	int VEXP=NV-1;																/* minimum degree */
	LOG_INFO("EXPANDED:("<<VEXP<<","<<ug.get_wv(VEXP)<<")");			
	bb&=ug.get_neighbors(VEXP);	
	int Wnew=W-ug.get_wv(VEXP);
	
	for(int d=1; d<=DEPTH; d++){
		VEXP=bb.msbn64();														/* LEAST DEGREE FIRST!! */			
		LOG_INFO("EXPANDED:("<<VEXP<<","<<ug.get_wv(VEXP)<<")");
		bb&=ug.get_neighbors(VEXP);
		Wnew-=ug.get_wv(VEXP);
	}
	int nb_col=0;
		
	int gap=ud.paint_and_map_OPT(bb,Wnew,gm,uw,nb_col);
	//uw.set_tk(1, nb_col); cout<<endl;
	uw.print_tk(1, nb_col);

	
	//TO TEST
	ug.print_weights(ud.get_unsel());
	ud.cover(gap,nb_col,uw,gm);
	ud.cover_with_overlap(nb_col,uw,gm);
	ug.print_weights(ud.get_unsel());
	uw.print_tk(1, nb_col);
	//**TODO- TESTS

	LOG_INFO("PRESS ANY KEY");
	cin.get();
	LOG_INFO("-------------------------------------------------------")	;
}


TEST(UBWC, basic_map_OPT){
	LOG_INFO("UBWC:basic_map_OPT-----------------------");

	//const int NV=6;
	//const int W=10;	
	//ugraph ug(NV),ugw(NV);
	//ug.add_edge(0, 1);		
	//ug.add_edge(1, 2);
	//ug.add_edge(2, 3);
	//ug.add_edge(3, 4);
	//ug.add_edge(4, 0);  //5-odd cycle
	//ug.add_edge(4, 5);  //extra node

	//ug.init_wv();
	//ug.set_wv(0, 3); ug.set_wv(1, 5); ug.set_wv(2, 2); ug.set_wv(3, 1); 		
	//ug.set_wv(4, 4); ug.set_wv(5, 6);


	ugraph ug("brock200_1.clq"); const int W=2802;
//	ugraph ug("san400_0.7_3.clq"); const int W=2753;
//	ugraph ug("sanr400_0.7.clq"); const int W=3061;
//	ugraph ug("sanr200_0.9.clq"); const int W=5090;
//	ugraph ug("p_hat300-3.clq"); const int W=3945;
//	ugraph ug("p_hat500-3.clq"); const int W=5333;
//	ugraph ug("p_hat500-2.clq"); const int W=3889;
//	ugraph ug("p_hat700-2.clq"); const int W=5250;
//	ugraph ug("p_hat1500-1.clq"); const int W=1609;
	const int NV=ug.number_of_vertices();
	ugraph ugw(ug);

		
	//init UBs
	UBWC ud(&ug), uw(&ugw);
	ud.init(); 
	uw.init();

	//build mapping
	GraphMap gm;
	gm.build_mapping(ug,MIN_DEG_DEGEN,PLACE_LF,MAX_WEIGHT,PLACE_FL,"MIN_DEG_DEGEN-LF","MAX_WEIGHT-FL");
	
	//sort ugw by weight
	GraphSort<ugraph> ow(ugw);
	vint lvw=ow.new_order(gbbs::sort_t::MAX_WEIGHT, gbbs::place_t::PLACE_FL);
	ow.reorder(lvw);

	//sort ug by degree
	GraphSort<ugraph> od(ug);
	vint lvd=od.new_order(gbbs::sort_t::MIN_DEG_DEGEN, gbbs::place_t::PLACE_LF);
	od.reorder(lvd);

	ug.print_weights();
	ugw.print_weights();

//////////////////////////
// paint and map
	bitarray bb(NV);
	bb.set_bit(0,NV-1);
	int nb_col=0;
	
	//TO TEST
	int gap=ud.paint_and_map(bb,W,gm,uw,nb_col);
	EXPECT_EQ(0,gap);

	LOG_INFO("[gap:"<<gap<<"]");
	uw.print_db(1,nb_col);
	ug.print_weights(ud.unsel); cout<<endl;
	uw.set_tk(1, nb_col); cout<<endl;
	uw.print_tk(1, nb_col);
	
	cin.get();

	//TO TEST
	ud.cover(gap,nb_col,uw,gm);
	ud.cover_with_overlap(nb_col,uw,gm);
	ug.print_weights(ud.get_unsel());
	uw.print_tk(1, nb_col); cout<<endl;
	cin.get();

	bb.set_bit(0,NV-1);
	int gap1=ud.paint_and_map_OPT(bb,W,gm,uw,nb_col/*,cand*/);
	EXPECT_EQ(0,gap);

	LOG_INFO("[gap_OPT:"<<gap<<"]");
	//uw.print_db(1,nb_col);
	ug.print_weights(ud.unsel);

	//uw.set_tk(1, nb_col); cout<<endl;
	uw.print_tk(1, nb_col); cout<<endl;

	
	//TO TEST
	ud.cover(gap,nb_col,uw,gm);
	ud.cover_with_overlap(nb_col,uw,gm);
	ug.print_weights(ud.get_unsel());
	uw.print_tk(1, nb_col); cout<<endl;
		
	
	LOG_INFO("PRESS ANY KEY");
	cin.get();
	LOG_INFO("-------------------------------------------------------")	;
}




TEST(UBWC_cover_shared, Dimacs_map){
	LOG_INFO("UBWC_cover:Dimacs_map-----------------------");

	ugraph ug("brock200_1.clq"); const int W=2802;
//	ugraph ug("san400_0.7_3.clq"); const int W=2753;
//	ugraph ug("sanr400_0.7.clq"); const int W=3061;
//	ugraph ug("sanr200_0.9.clq"); const int W=5090;
//	ugraph ug("p_hat300-3.clq"); const int W=3945;
//	ugraph ug("p_hat500-3.clq"); const int W=5333;
//	ugraph ug("p_hat1500-1.clq"); const int W=1609;

	ugraph ugw(ug);
	const int NV=ug.number_of_vertices();

	//I/O-graph
	stringstream sstr;
	ug.print_data(true, sstr);
	LOG_INFO(sstr.str());
			
	//build mapping
	GraphMap gm;
	gm.build_mapping(ug,MIN_DEG_DEGEN,PLACE_LF,MAX_WEIGHT,PLACE_FL,"MIN_DEG_DEGEN-LF","MAX_WEIGHT-FL");
	
	//sort ugw by weight
	GraphSort<ugraph> ow(ugw);
	vint lvw=ow.new_order(gbbs::sort_t::MAX_WEIGHT, gbbs::place_t::PLACE_FL);
	ow.reorder(lvw);

	//sort ug by degree
	GraphSort<ugraph> od(ug);
	vint lvd=od.new_order(gbbs::sort_t::MIN_DEG_DEGEN, gbbs::place_t::PLACE_LF);
	od.reorder(lvd);

//////////////////////////
// paint and map
	//init UBs
	UBWC ud(&ug), uw(&ugw);
	ud.init(); 
	uw.init();

	bitarray bb(NV);
	bb.set_bit(0,NV-1);
	//stack_t<int> cand;
	//cand.init(NV);
	int nb_col=0;
		
	int gap=ud.paint_and_map(bb,W,gm,uw,nb_col);
	uw.set_tk(1, nb_col); cout<<endl;
//	EXPECT_EQ(W,wcol);
//	int gap=W-wcol;	LOG_INFO("gap: "<<gap);

	
	//TO TEST
	ugw.print_weights(ud.get_unsel());
	ud.cover(gap,nb_col,uw,gm);
	ugw.print_weights(ud.get_unsel());


	
	//**TODO- TESTS

	LOG_INFO("PRESS ANY KEY");
	cin.get();
	LOG_INFO("-------------------------------------------------------")	;
}


TEST(DISABLED_UBWC_cover, DIMACS_depth_sorted_by_weight){
//////////////////
// Expansion based on weights (max weight first is clearly better than min weight first)

	LOG_INFO("UBWC_cover:DIMACS_depth_sorted_by_weight-----------------------");

//	ugraph ug("brock200_1.clq"); const int W=2802;
//	ugraph ug("san400_0.7_3.clq"); const int W=2753;
//	ugraph ug("sanr400_0.7.clq"); const int W=3061;
//	ugraph ug("sanr200_0.9.clq"); const int W=5090;
	ugraph ug("p_hat300-3.clq");const int W=3945;
//	ugraph ug("p_hat1500-1.clq"); const int W=1609;

	const int NV=ug.number_of_vertices();
	const int DEPTH=1;
	stringstream sstr;
	ug.print_data(true, sstr);
	LOG_INFO(sstr.str());

	//sort graph by weight
	GraphSort<ugraph> o(ug);
	vint lv=o.new_order(gbbs::sort_t::MAX_WEIGHT, gbbs::place_t::PLACE_FL);
	o.reorder(lv);
	
/////////////
//paint
	UBWC u(&ug);
	u.init();
	bitarray bb(NV);
	bb.set_bit(0,NV-1);
	
	//int VEXP=NV-1;														/* minimum weight */
	int VEXP=0;																/* maximum weight */
	LOG_INFO("EXPANDED:("<<VEXP<<","<<ug.get_wv(VEXP)<<")");			
	bb&=ug.get_neighbors(VEXP);	
	int Wnew=W-ug.get_wv(VEXP);
	
	for(int d=1; d<=DEPTH; d++){
		//VEXP=bb.msbn64();													/* minimum weight first */	
		VEXP=bb.lsbn64();													/* maximum weight first */	
		LOG_INFO("EXPANDED:("<<VEXP<<","<<ug.get_wv(VEXP)<<")");
		bb&=ug.get_neighbors(VEXP);
		Wnew-=ug.get_wv(VEXP);
	}
		
	stack_t<int> cand;
	cand.init(NV);
	int nb_col=0;
	int wcol=u.paint_and_tk(bb, W,nb_col, cand);

	//reset unsel with remaining nodes-TESTS
	for(int i=0; i<cand.size(); i++){
		u.get_unsel().set_bit(cand.get_elem(i));
	}

	//I/O
	ug.print_weights(u.get_unsel()); cout<<endl;
	u.print_tk(1, nb_col);			 cout<<endl;	

	//TO TEST
	int gap=W-wcol; LOG_INFO("gap: "<<gap);	
	u.cover(gap,nb_col);
	
	//I/O
	ug.print_weights(u.get_unsel());
	u.print_tk(1, nb_col);					/* TODO-check negative top-ks!*/
	
	//**TODO- TESTS

	LOG_INFO("PRESS ANY KEY");
	cin.get();
	LOG_INFO("-------------------------------------------------------")	;
}

TEST(UBWC_cover, Dimacs_depth_map){
////////////////////
// Min degree depth expansion with mapping (this test) much better than max weight depth expansion 
// with graphs sorted by weight (see previous test)
	LOG_INFO("UBWC_cover:Dimacs_depth_1_map-----------------------");

//	ugraph ug("brock200_1.clq"); const int W=2802;
//	ugraph ug("san400_0.7_3.clq"); const int W=2753;
	ugraph ug("sanr400_0.7.clq"); const int W=3061;
//	ugraph ug("sanr200_0.9.clq"); const int W=5090;
//	ugraph ug("p_hat300-3.clq"); const int W=3945;
//	ugraph ug("p_hat500-3.clq"); const int W=5333;
//	ugraph ug("p_hat1500-1.clq"); const int W=1609;

	ugraph ugw(ug);
	const int NV=ug.number_of_vertices();
	const int DEPTH=1;

	//I/O-graph
	stringstream sstr;
	ug.print_data(true, sstr);
	LOG_INFO(sstr.str());
			
	//build mapping
	GraphMap gm;
	gm.build_mapping(ug,MIN_DEG_DEGEN,PLACE_LF,MAX_WEIGHT,PLACE_FL,"MIN_DEG_DEGEN-LF","MAX_WEIGHT-FL");
	
	//sort ugw by weight
	GraphSort<ugraph> ow(ugw);
	vint lvw=ow.new_order(gbbs::sort_t::MAX_WEIGHT, gbbs::place_t::PLACE_FL);
	ow.reorder(lvw);

	//sort ug by degree
	GraphSort<ugraph> od(ug);
	vint lvd=od.new_order(gbbs::sort_t::MIN_DEG_DEGEN, gbbs::place_t::PLACE_LF);
	od.reorder(lvd);
	
	ug.print_weights();

//////////////////////////
// paint and map
	//init UBs
	UBWC ud(&ug), uw(&ugw);
	ud.init(); 
	uw.init();

	//induced subgraph by highest weighted vertex
	bitarray bb(NV);
	bb.set_bit(0,NV-1);
//	int VEXP=159;																/* highest weighted */
	int VEXP=NV-1;																/* minimum degree */
	LOG_INFO("EXPANDED:("<<VEXP<<","<<ug.get_wv(VEXP)<<")");			
	bb&=ug.get_neighbors(VEXP);	
	int Wnew=W-ug.get_wv(VEXP);
	
	for(int d=1; d<=DEPTH; d++){
		VEXP=bb.msbn64();														/* LEAST DEGREE FIRST!! */			
		LOG_INFO("EXPANDED:("<<VEXP<<","<<ug.get_wv(VEXP)<<")");
		bb&=ug.get_neighbors(VEXP);
		Wnew-=ug.get_wv(VEXP);
	}

	stack_t<int> cand;
	cand.init(NV);
	int nb_col=0;
		
	//TO TEST
	int gap=ud.paint_and_map(bb,Wnew,gm,uw,nb_col/*,cand*/);
	EXPECT_EQ(0,gap);
	
	//reset unsel with remaining nodes-TESTS
	//for(int i=0; i<cand.size(); i++){
	//	ud.get_unsel().set_bit(cand.get_elem(i));
	//}

	//map
	gm.map_l2r(ud.get_unsel(), uw.get_unsel());
	uw.set_tk(1, nb_col); cout<<endl;

	//I/O
	uw.print_db(1, nb_col); cout<<endl;
	uw.print_tk(1, nb_col); cout<<endl;
	ugw.print_weights(uw.get_unsel());
	
	//TO TEST
	uw.cover(gap,nb_col);
	
	//I/O
	ugw.print_weights(uw.get_unsel());
	uw.print_tk(1, nb_col);					/* TODO-check negative top-ks!*/

	
	LOG_INFO("PRESS ANY KEY");
	cin.get();
	LOG_INFO("-------------------------------------------------------")	;
}

TEST(UBWC_cover, Dimacs_map){
	LOG_INFO("UBWC_cover:Dimacs_map-----------------------");

//	ugraph ug("brock200_1.clq"); const int W=2802;
//	ugraph ug("san400_0.7_3.clq"); const int W=2753;
//	ugraph ug("sanr400_0.7.clq"); const int W=3061;
//	ugraph ug("sanr200_0.9.clq"); const int W=5090;
	//ugraph ug("p_hat300-3.clq"); const int W=3945;
	ugraph ug("p_hat500-3.clq"); const int W=5333;
//	ugraph ug("p_hat1500-1.clq"); const int W=1609;
	ugraph ugw(ug);
	const int NV=ug.number_of_vertices();

	//I/O-graph
	stringstream sstr;
	ug.print_data(true, sstr);
	LOG_INFO(sstr.str());
	
		
	//build mapping
	GraphMap gm;
	gm.build_mapping(ug,MIN_DEG_DEGEN,PLACE_LF,MAX_WEIGHT,PLACE_FL,"MIN_DEG_DEGEN-LF","MAX_WEIGHT-FL");
	
	//sort ugw by weight
	GraphSort<ugraph> ow(ugw);
	vint lvw=ow.new_order(gbbs::sort_t::MAX_WEIGHT, gbbs::place_t::PLACE_FL);
	ow.reorder(lvw);

	//sort ug by degree
	GraphSort<ugraph> od(ug);
	vint lvd=od.new_order(gbbs::sort_t::MIN_DEG_DEGEN, gbbs::place_t::PLACE_LF);
	od.reorder(lvd);

//////////////////////////
// paint and map
	//init UBs
	UBWC ud(&ug), uw(&ugw);
	ud.init(); 
	uw.init();

	bitarray bb(NV);
	bb.set_bit(0,NV-1);
	stack_t<int> cand;
	cand.init(NV);
	int nb_col=0;
		
	//TO TEST
	int gap=ud.paint_and_map(bb,W,gm,uw,nb_col/*,cand*/);
	EXPECT_EQ(0,gap);
	
	//reset unsel with remaining nodes-TESTS
	/*for(int i=0; i<cand.size(); i++){
		ud.get_unsel().set_bit(cand.get_elem(i));
	}*/

	//map
	gm.map_l2r(ud.get_unsel(), uw.get_unsel());
	uw.set_tk(1, nb_col); cout<<endl;

	//I/O
	uw.print_db(1, nb_col); cout<<endl;
	uw.print_tk(1, nb_col); cout<<endl;
	ugw.print_weights(uw.get_unsel());
	

	//TO TEST
	uw.cover(gap,nb_col);
	
	//I/O
	ugw.print_weights(uw.get_unsel());
	uw.print_tk(1, nb_col);					/* TODO-check negative top-ks!*/
	

	//**TODO- TESTS

	LOG_INFO("PRESS ANY KEY");
	cin.get();
	LOG_INFO("-------------------------------------------------------")	;
}

TEST(UBWC_cover, DIMACS_sorted_by_weight){
	LOG_INFO("UBWC_cover:DIMACS_sorted_by_weight-----------------------");

	//ugraph ug("brock200_1.clq"); const int W=2802;
	//	ugraph ug("san400_0.7_3.clq"); const int W=2753;
//	ugraph ug("sanr400_0.7.clq"); const int W=3061;
	ugraph ug("sanr200_0.9.clq"); const int W=5090;
//	ugraph ug("p_hat300-3.clq");const int W=3945;
	const int NV=ug.number_of_vertices();
	stringstream sstr;
	ug.print_data(true, sstr);
	LOG_INFO(sstr.str());

	//sort graph by weight
	GraphSort<ugraph> o(ug);
	vint lv=o.new_order(gbbs::sort_t::MAX_WEIGHT, gbbs::place_t::PLACE_FL);
	o.reorder(lv);

	
/////////////
//paint
	UBWC u(&ug);
	u.init();
	bitarray bb(NV);
	bb.set_bit(0,NV-1);
	stack_t<int> cand;
	cand.init(NV);
	int nb_col=0;
	int wcol=u.paint_and_tk(bb, W,nb_col, cand);

	//reset unsel with remaining nodes-TESTS
	for(int i=0; i<cand.size(); i++){
		u.get_unsel().set_bit(cand.get_elem(i));
	}

	//I/O
	ug.print_weights(u.get_unsel()); cout<<endl;
	u.print_tk(1, nb_col);			cout<<endl;	

	//TO TEST
	int gap=W-wcol;
	u.cover(gap,nb_col);
	
	//I/O
	ug.print_weights(u.get_unsel());
	u.print_tk(1, nb_col);					/* TODO-check negative top-ks!*/
	

	//**TODO- TESTS

	LOG_INFO("PRESS ANY KEY");
	cin.get();
	LOG_INFO("-------------------------------------------------------")	;
}


TEST(UBWC, basic_sorted_by_weight){
	LOG_INFO("UBWC:basic_sorted_by_weight-----------------------");

	const int NV=6;
	const int W=10;
	ugraph ug(NV);
	ug.add_edge(0, 1);		
	ug.add_edge(1, 2);
	ug.add_edge(2, 3);
	ug.add_edge(3, 4);
	ug.add_edge(4, 0);  //5-odd cycle
	ug.add_edge(4, 5);  //extra node

	ug.init_wv();
	ug.set_wv(0, 3); ug.set_wv(1, 5); ug.set_wv(2, 2); ug.set_wv(3, 1); 		
	ug.set_wv(4, 4); ug.set_wv(5, 6);
	
	//sort graph by weight
	GraphSort<ugraph> o(ug);
	vint lv=o.new_order(gbbs::sort_t::MAX_WEIGHT, gbbs::place_t::PLACE_FL);
	o.reorder(lv);

/////////////
//paint
	UBWC u(&ug);
	u.init();
	bitarray bb(NV);
	bb.set_bit(0,NV-1);
	stack_t<int> cand;
	cand.init(NV);
	int nb_col=0;

	//TO TEST
	int wcol=u.paint(bb, W,nb_col, cand);
	
	//test independent sets: determine cliques in complement graph
	ugraph ugc(NV);
	ug.create_complement(ugc);
	Clique<ugraph> clq(&ugc, clqo::param_t());
	for(int c=1; c<=nb_col; c++){
		u.get_db().pbb[c].print();
		EXPECT_TRUE(clq.is_clique(u.get_db().pbb[c]));
	}

	//cand and unsel should be disjoint
	for(int i=0; i<cand.pt; i++){
		EXPECT_TRUE(!u.get_unsel().is_bit(cand.get_elem(i)));
	}

	//I/O
	LOG_INFO("[ub:"<<wcol<<"]");
	u.print_db(1,nb_col); 	
	u.set_tk(1,nb_col);
	u.print_tk(1,nb_col); cout<<endl;
	u.get_unsel().print();

//////////////
//paint_tk
	//TO TEST
	wcol=u.paint_and_tk(bb, W, nb_col, cand);

	//test independent sets: determine cliques in complement graph
	Clique<ugraph> clq_tk(&ugc, clqo::param_t());
	for(int c=1; c<=nb_col; c++){
		u.get_db().pbb[c].print();
		EXPECT_TRUE(clq_tk.is_clique(u.get_db().pbb[c]));
	}

	//cand and unsel should be disjoint
	for(int i=0; i<cand.pt; i++){
		EXPECT_TRUE(!u.get_unsel().is_bit(cand.get_elem(i)));
	}

	//top-k tests
	lv[3];
	for(int c=1; c<=nb_col; c++){
		int num_nodes=first_k_bits(3,u.get_db().pbb[c],lv);
		EXPECT_EQ(ug.get_wv(lv[0]),u.get_tk(c)[0]);
		if(num_nodes==3){
			EXPECT_EQ(ug.get_wv(lv[0])-ug.get_wv(lv[1]),u.get_tk(c)[1]);
			EXPECT_EQ(ug.get_wv(lv[0])-ug.get_wv(lv[2]),u.get_tk(c)[2]);
		}else if(num_nodes==2){
			EXPECT_EQ(ug.get_wv(lv[0])-ug.get_wv(lv[1]),u.get_tk(c)[1]);
		}
	}
	
	//I/O
	LOG_INFO("[ub:"<<wcol<<"]");
	u.print_db(1,nb_col); 
	u.print_tk(1,nb_col); cout<<endl;
	u.get_unsel().print(); 
					
	
	/*LOG_INFO("PRESS ANY KEY");
	cin.get();*/
	LOG_INFO("-------------------------------------------------------")	;
}

TEST(UBWC, Dimacs_sorted_by_weight){
	LOG_INFO("UBWC:Dimacs_sorted_by_weight-----------------------");

	ugraph ug("brock200_1.clq"); const int W=2802;	
//	ugraph ug("p_hat300-3.clq");	const int W=3945;		
	UBWC u(&ug);
	u.init();
	const int NV=ug.number_of_vertices();
	

	//sort graph by weight
	GraphSort<ugraph> o(ug);
	vint lv=o.new_order(gbbs::sort_t::MAX_WEIGHT, gbbs::place_t::PLACE_FL);
	o.reorder(lv);

/////////////
//paint
	bitarray bb(NV);
	bb.set_bit(0,NV-1);
	stack_t<int> cand;
	cand.init(NV);
	int nb_col=0;

	//TO TEST
	int wcol=u.paint(bb,W,nb_col,cand);
	EXPECT_EQ(W,wcol);

	//test independent sets: determine cliques in complement graph
	ugraph ugc(NV);
	ug.create_complement(ugc);
	Clique<ugraph> clq(&ugc, clqo::param_t());
	for(int c=1; c<=nb_col; c++){
		u.get_db().pbb[c].print();
		EXPECT_TRUE(clq.is_clique(u.get_db().pbb[c]));
	}

	//cand and unsel should be disjoint
	for(int i=0; i<cand.pt; i++){
		EXPECT_TRUE(!u.get_unsel().is_bit(cand.get_elem(i)));
	}
		
	//I/O
	LOG_INFO("[ub:"<<wcol<<"]");
	u.print_db(1,nb_col); 	
	u.set_tk(1,nb_col);
	u.print_tk(1,nb_col);  cout<<endl;
	u.get_unsel().print(); 

//////////////
//paint_tk

	//TO TEST
	wcol=u.paint_and_tk(bb,W, nb_col, cand);
	EXPECT_EQ(W,wcol);


	//test independent sets: determine cliques in complement graph
	Clique<ugraph> clq_tk(&ugc, clqo::param_t());
	for(int c=1; c<=nb_col; c++){
		u.get_db().pbb[c].print();
		EXPECT_TRUE(clq_tk.is_clique(u.get_db().pbb[c]));
	}

	//cand and unsel should be disjoint
	for(int i=0; i<cand.pt; i++){
		EXPECT_TRUE(!u.get_unsel().is_bit(cand.get_elem(i)));
	}

	//top-k tests
	lv[3];
	for(int c=1; c<=nb_col; c++){
		int num_nodes=first_k_bits(3,u.get_db().pbb[c],lv);
		EXPECT_EQ(ug.get_wv(lv[0]),u.get_tk(c)[0]);
		if(num_nodes==3){
			EXPECT_EQ(ug.get_wv(lv[0])-ug.get_wv(lv[1]),u.get_tk(c)[1]);
			EXPECT_EQ(ug.get_wv(lv[0])-ug.get_wv(lv[2]),u.get_tk(c)[2]);
		}else if(num_nodes==2){
			EXPECT_EQ(ug.get_wv(lv[0])-ug.get_wv(lv[1]),u.get_tk(c)[1]);
		}
	}
	
	//I/O
	LOG_INFO("[ub:"<<wcol<<"]");
	u.print_db(1,nb_col); 
	u.print_tk(1,nb_col); cout<<endl;
	u.get_unsel().print(); 
		
	/*LOG_INFO("PRESS ANY KEY");
	cin.get();*/
	LOG_INFO("-------------------------------------------------------")	;
}

TEST(UBWC, basic_map_structure){
	LOG_INFO("UBWC:basic_map_structure-----------------------");

	ugraph ug("san400_0.7_3.clq");
	const int NV=ug.number_of_vertices();
	ugraph ugw(ug);
	

	//const int NV=6;
	//const int W=10;	
	//ugraph ug(NV),ugw(NV);
	//ug.add_edge(0, 1);		
	//ug.add_edge(1, 2);
	//ug.add_edge(2, 3);
	//ug.add_edge(3, 4);
	//ug.add_edge(4, 0);  //5-odd cycle
	//ug.add_edge(4, 5);  //extra node

	//ug.init_wv();
	//ug.set_wv(0, 3); ug.set_wv(1, 5); ug.set_wv(2, 2); ug.set_wv(3, 1); 		
	//ug.set_wv(4, 4); ug.set_wv(5, 6);
	//ugw=ug;
	
	
	//init UBs
	UBWC ud(&ug), uw(&ugw);
	ud.init(); 
	uw.init();

	//build mapping
	GraphMap gm;
	gm.build_mapping(ug,MIN_DEG_DEGEN,PLACE_LF,MAX_WEIGHT,PLACE_FL,"MIN_DEG_DEGEN-LF","MAX_WEIGHT-FL");
	
	//sort ugw by weight
	GraphSort<ugraph> ow(ugw);
	vint lvw=ow.new_order(gbbs::sort_t::MAX_WEIGHT, gbbs::place_t::PLACE_FL);
	ow.reorder(lvw);

	//sort ug by degree
	GraphSort<ugraph> od(ug);
	vint lvd=od.new_order(gbbs::sort_t::MIN_DEG_DEGEN, gbbs::place_t::PLACE_LF);
	od.reorder(lvd);

	ug.print_weights();
	ugw.print_weights();

//////////////////////////
// structure: edge preservation
	/*ug.print_edges(); cout<<endl;
	ugw.print_edges();*/

	int v1=7, v2=251, vw1=gm.map_l2r(v1), vw2=gm.map_l2r(v2);
	LOG_INFO(vw1<<","<<vw2);
	if(ug.is_edge(v1, v2)) LOG_INFO("EDGE");
	if(ugw.is_edge(vw1, vw1)) LOG_INFO("EDGEW");
	

	for(int i=0; i<=NV-2; i++){
		for(int j=i+1; j<=NV-1; j++){
			if(ug.is_edge(i,j)){
				EXPECT_TRUE(ugw.is_edge(gm.map_l2r(i),gm.map_l2r(j)));
				/*if(!ugw.is_edge(gm.map_l2r(i),gm.map_l2r(j))){
					LOG_ERROR("["<<i<<","<<j<<"]");
					LOG_ERROR("["<<gm.map_l2r(i)<<","<<gm.map_l2r(j)<<"]");
				}*/
			}

			if(!ug.is_edge(i,j)){
				EXPECT_TRUE(!ugw.is_edge(gm.map_l2r(i),gm.map_l2r(j)));
				/*if(ugw.is_edge(gm.map_l2r(i),gm.map_l2r(j))){
					LOG_ERROR("["<<i<<","<<j<<"]");
					LOG_ERROR("["<<gm.map_l2r(i)<<","<<gm.map_l2r(j)<<"]");
				}*/
			}
		}//endfor
	}
	
	
	//LOG_INFO("PRESS ANY KEY");
	//cin.get();
	LOG_INFO("-------------------------------------------------------")	;
}

TEST(UBWC, basic_map){
	LOG_INFO("UBWC:basic_map-----------------------");

	const int NV=6;
	const int W=10;	
	ugraph ug(NV),ugw(NV);
	ug.add_edge(0, 1);		
	ug.add_edge(1, 2);
	ug.add_edge(2, 3);
	ug.add_edge(3, 4);
	ug.add_edge(4, 0);  //5-odd cycle
	ug.add_edge(4, 5);  //extra node

	ug.init_wv();
	ug.set_wv(0, 3); ug.set_wv(1, 5); ug.set_wv(2, 2); ug.set_wv(3, 1); 		
	ug.set_wv(4, 4); ug.set_wv(5, 6);
	ugw=ug;
	
	//init UBs
	UBWC ud(&ug), uw(&ugw);
	ud.init(); 
	uw.init();

	//build mapping
	GraphMap gm;
	gm.build_mapping(ug,MIN_DEG_DEGEN,PLACE_LF,MAX_WEIGHT,PLACE_FL,"MIN_DEG_DEGEN-LF","MAX_WEIGHT-FL");
	
	//sort ugw by weight
	GraphSort<ugraph> ow(ugw);
	vint lvw=ow.new_order(gbbs::sort_t::MAX_WEIGHT, gbbs::place_t::PLACE_FL);
	ow.reorder(lvw);

	//sort ug by degree
	GraphSort<ugraph> od(ug);
	vint lvd=od.new_order(gbbs::sort_t::MIN_DEG_DEGEN, gbbs::place_t::PLACE_LF);
	od.reorder(lvd);

	ug.print_weights();
	ugw.print_weights();

//////////////////////////
// paint and map
	bitarray bb(NV);
	bb.set_bit(0,NV-1);
	stack_t<int> cand;
	cand.init(NV);
	int nb_col=0;
	
	//TO TEST
	int gap=ud.paint_and_map(bb,W,gm,uw,nb_col/*,cand*/);
	EXPECT_EQ(0,gap);


	//test independent sets: determine cliques in complement graph
	ugraph ugc(NV);
	ugw.create_complement(ugc);
	Clique<ugraph> clq(&ugc, clqo::param_t());
	for(int c=1; c<=nb_col; c++){
		uw.get_db().pbb[c].print();
		EXPECT_TRUE(clq.is_clique(uw.get_db().pbb[c]));
	}

	//cand and unsel should be disjoint
	for(int i=0; i<cand.pt; i++){
		EXPECT_TRUE(!ud.get_unsel().is_bit(cand.get_elem(i)));
	}

	//I/O
	LOG_INFO("[gap:"<<gap<<"]");
	uw.print_db(1,nb_col);
	ud.get_unsel().print();
	
	LOG_INFO("PRESS ANY KEY");
	//cin.get();
	LOG_INFO("-------------------------------------------------------")	;
}

TEST(UBWC, Dimacs_map){
	LOG_INFO("UBWC:Dimacs_map-----------------------");

	ugraph ug("brock200_1.clq");	const int W=2802;
//	ugraph ug("p_hat300-3.clq");	const int W=3945;		
	ugraph ugw(ug);
	const int NV=ug.number_of_vertices();
	
		
	//init UBs
	UBWC ud(&ug), uw(&ugw);
	ud.init(); 
	uw.init();

	//build mapping
	GraphMap gm;
	gm.build_mapping(ug,MIN_DEG_DEGEN,PLACE_LF,MAX_WEIGHT,PLACE_FL,"MIN_DEG_DEGEN-LF","MAX_WEIGHT-FL");
	
	//sort ugw by weight
	GraphSort<ugraph> ow(ugw);
	vint lvw=ow.new_order(gbbs::sort_t::MAX_WEIGHT, gbbs::place_t::PLACE_FL);
	ow.reorder(lvw);

	//sort ug by degree
	GraphSort<ugraph> od(ug);
	vint lvd=od.new_order(gbbs::sort_t::MIN_DEG_DEGEN, gbbs::place_t::PLACE_LF);
	od.reorder(lvd);

	ug.print_weights();
	ugw.print_weights();

//////////////////////////
// paint and map
	bitarray bb(NV);
	bb.set_bit(0,NV-1);
	stack_t<int> cand;
	cand.init(NV);
	int nb_col=0;
	
	//TO TEST
	int gap=ud.paint_and_map(bb,W,gm,uw,nb_col/*,cand*/);
	EXPECT_EQ(0,gap);

	//test independent sets: determine cliques in complement graph
	ugraph ugc(NV);
	ugw.create_complement(ugc);
	Clique<ugraph> clq(&ugc, clqo::param_t());
	for(int c=1; c<=nb_col; c++){
		uw.get_db().pbb[c].print();
		EXPECT_TRUE(clq.is_clique(uw.get_db().pbb[c]));
	}

	//cand and unsel should be disjoint
	for(int i=0; i<cand.pt; i++){
		EXPECT_TRUE(!ud.get_unsel().is_bit(cand.get_elem(i)));
	}
	
	//weight of clauses tests (paint_and_map makes no top_1 or top_2 computation)
	int lv[3];
	for(int c=1; c<=nb_col; c++){
		int num_nodes=first_k_bits(3,uw.get_db().pbb[c],lv);
		EXPECT_EQ(ugw.get_wv(lv[0]),uw.get_tk(c)[0]);
	}

	//TO TEST
	uw.set_tk(1,nb_col);

	//top-k tests
	for(int c=1; c<=nb_col; c++){
		int num_nodes=first_k_bits(3,uw.get_db().pbb[c],lv);
		EXPECT_EQ(ugw.get_wv(lv[0]),uw.get_tk(c)[0]);
		if(num_nodes==3){
			EXPECT_EQ(ugw.get_wv(lv[0])-ugw.get_wv(lv[1]),uw.get_tk(c)[1]);
			EXPECT_EQ(ugw.get_wv(lv[0])-ugw.get_wv(lv[2]),uw.get_tk(c)[2]);
		}else if(num_nodes==2){
			EXPECT_EQ(ugw.get_wv(lv[0])-ugw.get_wv(lv[1]),uw.get_tk(c)[1]);
		}
	}
	

	//I/O
	LOG_INFO("[gap:"<<gap<<"]");
	uw.print_db(1,nb_col);
	uw.print_tk(1,nb_col); cout<<endl;
			
	LOG_INFO("PRESS ANY KEY");
	//cin.get();
	LOG_INFO("-------------------------------------------------------")	;
}

// END UBWC
///////////////////////////////////////////////////////////////////////////////




TEST(Weighted_Clique_Plus, test_configuration){
	LOG_INFO("Weighted_Clique_Plus:test_configuration-----------------------");
	
	//Ugraph
	const int NV=6;
	ugraph ug(NV);
	ug.add_edge(0, 1);		//3-clique
	ug.add_edge(0, 2);
	ug.add_edge(1, 2);
	
	ug.init_wv();
	for(int i=0; i<NV; i++){
		ug.set_wv(i, i+1);			//incremental weights, {1(1), 2(2) 3(3), 4(4), 5(5), 6(6)}
	}
	
	clqo::param_t param;
	param.alg=clqo::BBMC_WEIGHTED;
	param.init_order=clqo::init_order_t::MAX_WEIGHTED;			//non-increasing weights with max_deg flavour
	CliqueWeightedPlus cug(&ug, param);
	cug.set_up();
	cug.run();
	
	/* TEST CONDITIONS */
	int lb=-1, ub=NV*6;			
	cug.initial_bounds(lb, ub);
	int ub_exp=std::accumulate (ug.get_weights().begin(), ug.get_weights().end(), 0, plus<int>());
	EXPECT_EQ(ub_exp,ub);
	EXPECT_EQ(6,lb);
	
	LOG_INFO("-------------------------------------------------------")	;
}

TEST(Weighted_Clique_Plus, test_configuration_II){
/////////////
//tests graph in papers: 5-odd cycle with an appendix (see my notes)
	LOG_INFO("Weighted_Clique_Plus:test_configuration_II-----------------------");
	
	//Ugraph
	const int NV=6;
	ugraph ug(NV);
	ug.add_edge(0, 1);		
	ug.add_edge(1, 2);
	ug.add_edge(2, 3);
	ug.add_edge(3, 4);
	ug.add_edge(4, 0);  //5-odd cycle
	ug.add_edge(4, 5);  //extra node
	
	ug.init_wv();
	ug.set_wv(0, 1); ug.set_wv(1, 7); ug.set_wv(2, 2); ug.set_wv(3, 3); 		
	ug.set_wv(4, 4); ug.set_wv(5, 6);
	
	clqo::param_t param;
	param.alg=clqo::BBMC_WEIGHTED;
	param.init_order=clqo::init_order_t::MAX_WEIGHTED;			//non-increasing weights with max_deg flavour
	CliqueWeightedPlus cug(&ug, param);
	cug.set_up();
	cug.run();

	/* TEST CONDITIONS */
	int lb=-1, ub=NV*7;
	cug.initial_bounds(lb, ub);
	int ub_exp=std::accumulate (ug.get_weights().begin(), ug.get_weights().end(), 0, plus<int>());
	EXPECT_EQ(ub_exp,ub);
	EXPECT_EQ(9,lb);			/* 0(7) + 2(2): elementary clique in order */
	
	LOG_INFO("-------------------------------------------------------")	;
}



TEST(DISABLED_Weighted_Clique_Plus, simple_run){
	LOG_INFO("Weighted_Clique_Plus:simple_run-----------------------");
	
	///Ugraph
	//const int NV=6;
	ugraph ug("brock200_1.clq");
	//ugraph ug("san400_0.7_2.clq");
	//ugraph ug("san400_0.7_3.clq");
	//ugraph ug("p_hat300-3.clq");
	//ugraph ug("p_hat500-3.clq");
			
	clqo::param_t param;
	param.alg=clqo::BBMC_WEIGHTED;
	param.init_order=clqo::init_order_t::MAX_WEIGHTED;					//non-increasing weights with max_deg flavour
	//param.init_order=clqo::init_order_t::MIN_WIDTH;					//non-increasing weights with max_deg flavour
	param.init_preproc=clqo::init_preproc_t::UB_HEUR;
	CliqueWeightedPlus cug(&ug, param);
	cug.set_up();
	cug.run();
		
	LOG_INFO("-------------------------------------------------------")	;
}

//////////////////////////
// LAMSADE-CliqueWeighted, currently deprecated (4/7/17)

TEST(DISABLED_Weighted_Clique, initial_bounds){
	LOG_INFO("Weighted_Clique:initial_bounds-----------------------");
	
	//Ugraph
	const int NV=6;
	ugraph ug(NV);
	ug.add_edge(0, 1);		//3-clique
	ug.add_edge(0, 2);
	ug.add_edge(1, 2);
	
	ug.init_wv();
	for(int i=0; i<NV; i++){
		ug.set_wv(i, i+1);			//incremental weights
	}
	
	CliqueWeighted cug(&ug, clqo::param_t());
	int ub=NV*6; int lb=-1;
	cug.initial_bounds(lb, ub);
	EXPECT_EQ(NV*(NV+1)/2,ub);
	EXPECT_EQ(6,lb);

	lb=0;
	ug.add_edge(0, 3);		//4-clique
	ug.add_edge(1, 3);
	ug.add_edge(2, 3);
	cug.initial_bounds(lb, ub);
	EXPECT_EQ(NV*(NV+1)/2,ub);
	EXPECT_EQ(10,lb);

	lb=0;
	ug.remove_edge(0,3);	//3-clique
	cug.initial_bounds(lb, ub);
	EXPECT_EQ(NV*(NV+1)/2,ub);
	EXPECT_EQ(6,lb);

	LOG_INFO("-------------------------------------------------------")	;
}


TEST(DISABLED_Weighted_Clique, initial_order){
////////////
// degree + MAX_WEIGHT
	LOG_INFO("Weighted_Clique:initial_order-----------------------");
	
	const int NV=6;
	ugraph ug(NV);
	ug.add_edge(0, 1);		//3-clique
	ug.add_edge(0, 2);
	ug.add_edge(1, 2);
	ug.add_edge(4, 5);
	
	ug.init_wv();
	for(int i=0; i<3; i++){
		ug.set_wv(i, i+1);			//incremental weights
	}
	
	Decode d;
	CliqueSort<ugraph> o(ug);
	CliqueSort<ugraph>::vpclq lord;
	lord.push_back( pair< clqo::init_order_t, gbbs::place_t>(clqo::MIN_WIDTH_MIN_TIE_STATIC, gbbs::PLACE_LF) );
	lord.push_back( pair< clqo::init_order_t, gbbs::place_t>(clqo::MAX_WEIGHTED, gbbs::PLACE_FL) );
	o.reorder_composite(lord, d);

	vint sol(NV);
	sol[0]=3; sol[1]=2; sol[2]=1;  sol[3]=1;  sol[4]=1;  sol[5]=1;
	EXPECT_EQ(sol, ug.get_weights());
	EXPECT_TRUE( ug.is_edge(3,4));			//min deg flavour: originally [4[->[5]
	//ug.print_edges();
	//ug.print_weights();
	LOG_INFO("-------------------------------------------------------")	;
}

TEST(DISABLED_Weighted, init_bounds){
////////////
// initial clique bounds

	LOG_INFO("Weighted:init_bounds---------------------------------------");

	const int NV=5;
	ugraph ug(NV);
	ug.set_name("my_wgraph");
	
	//set edges and cliques
	ug.add_edge(0,1);
	ug.add_edge(1,2);
	ug.add_edge(0,2);
	ug.init_wv();
	ug.set_wv(0,4); ug.set_wv(1,8); ug.set_wv(2,3);

	clqo::param_t my_param;
	my_param.init_order=clqo::init_order_t::MAX_WEIGHTED;
	my_param.alg=clqo::algorithm_t::BBMC_WEIGHTED;
	CliqueWeighted cw(&ug, my_param);
	int res=cw.set_up();							//allocates, sorts and computes initial oriented bounds
	int* c=cw.Clique<ugraph>::get_color_set(0);
	EXPECT_EQ(8,c[0]);
	EXPECT_EQ(12,c[1]);
	EXPECT_EQ(15,c[2]);
	EXPECT_EQ(1, c[3]);
	EXPECT_EQ(1, c[4]);


				
	LOG_INFO("----------------------------------------------------------------");
}




//TEST(Clique, grafo_JA){
////////////////
////  Sparse Ugraph
//
//	cout<<"Clique: grafo_JA-------------------------------"<<endl;
//    ugraph ug("grafo.clq");
//	Clique<ugraph> cug(&ug,   clqo::param_t());
//	cug.get_param().unrolled=true;
//	int res=cug.set_up(); 
//	cout<<"finished setup"<<endl;
//
//	if(res==0){
//		cug.run();
//	}
//		
//	
//	vint sol=cug.decode_first_solution();
//	EXPECT_EQ(182,cug.get_max_clique());
//	EXPECT_EQ(182,sol.size());
//
//	com::stl::print_collection(sol);
//		
//	cout<<"--------------------------------------------------------"<<endl;
//}


