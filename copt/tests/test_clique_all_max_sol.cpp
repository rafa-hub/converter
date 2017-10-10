//--------------------------------------------
// test_clique_all_max_sol.cpp: some tests for the algorithm which enumerates all maximum solutions efficiently


#include "gtest/gtest.h"
#include "graph/graph_gen.h"
#include "graph/algorithms/decode.h"
#include "utils/common.h"
#include "utils/logger.h"
#include "../clique/clique.h"
#include "../clique/clique_weighted.h"
#include "../clique/clique_all_max_sol.h"

using namespace std;

TEST(All_Max_Sol_Clique, basic){
	LOG_INFO("All_Max_Sol_Clique:initial_bounds-----------------------");
	
	//Ugraph
	const int NV=6;
	ugraph ug(NV);
	ug.add_edge(0, 1);		//3-clique
	ug.add_edge(0, 2);
	ug.add_edge(1, 2);
	ug.add_edge(3, 4);
	ug.add_edge(3, 5);
	ug.add_edge(4, 5);
		
	clqo::param_t myparam;
	myparam.alg=clqo::BBMCXR_L;
	myparam.init_preproc=clqo::init_preproc_t::UB;
	CliqueAll cug(&ug, myparam);
	cug.set_up();
	cug.run();			

	EXPECT_EQ(2,cug.get_result().get_number_of_solutions());

	LOG_INFO("-------------------------------------------------------")	;
}

TEST(All_Max_Sol_Clique, brock200_1){
	Logger::SetInformationLevel(LOGGER_INFO);
	LOG_INFO("All_Max_Sol_Clique:brock200_1-----------------------");
	
	//Ugraph
	ugraph ug("brock200_1.clq");
			
	clqo::param_t myparam;
	myparam.alg=clqo::BBMCXR_L;
	myparam.init_order=clqo::MIN_WIDTH;
	myparam.init_preproc=clqo::init_preproc_t::UB_HEUR;
	CliqueAll cug(&ug, myparam);
	cug.set_up();
	cug.run();			

	EXPECT_EQ(2,cug.get_result().get_number_of_solutions());

	LOG_INFO("-------------------------------------------------------")	;
}

TEST(All_Max_Sol_Clique, brock200_1_subgraph){
	Logger::SetInformationLevel(LOGGER_INFO);
	LOG_INFO("All_Max_Sol_Clique:brock200_1-----------------------");
	
	//Ugraph
	ugraph ug("brock200_1.clq");
			
	clqo::param_t myparam;
	myparam.alg=clqo::BBMCXR_L;
	myparam.init_order=clqo::MIN_WIDTH;
	myparam.init_preproc=clqo::init_preproc_t::UB_HEUR;
	CliqueAll cug(&ug, myparam);
	cug.set_up();				//allocates memory and computes bounds
	
	//computes bounds for subgraph without allocation
	bitarray bbs(ug.number_of_vertices());
	bbs.set_bit(0, 50);
	cug.set_up_subgraph(bbs);
	cug.run_subgraph(bbs);	

	//computes bounds for subgraph without allocation
	bbs.set_bit(0, 199);						//all
	cug.set_up_subgraph(bbs);
	cug.fix_lb(21);
	cug.run_subgraph(bbs);
	EXPECT_EQ(2,cug.get_result().get_number_of_solutions());

	LOG_INFO("-------------------------------------------------------")	;
}

TEST(All_Max_Sol_Clique, isolani){
	LOG_INFO("All_Max_Sol_Clique:initial_bounds-----------------------");
	
	//Ugraph
	const int NV=6;
	ugraph ug(NV);
		
		
	clqo::param_t myparam;
	myparam.alg=clqo::BBMCXR_L;
	myparam.init_preproc=clqo::init_preproc_t::UB;
	CliqueAll cug(&ug, myparam);
	cug.set_up();
	cug.run();			

	EXPECT_EQ(6,cug.get_result().get_number_of_solutions());

	LOG_INFO("-------------------------------------------------------")	;
}


