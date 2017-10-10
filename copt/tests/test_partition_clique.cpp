//--------------------------------------------
// test_partition_clique.cpp: some tests for partitioning the vertex set of an input graph with cliques (SAT-PLANNING 2016)
// added tests for simple clique partition based on max cliques (10/17)
// 
// date of creation: 9/11/16
// last update: 6/10/17
//
// author: pss

#include "gtest/gtest.h"
#include "graph/graph_gen.h"
#include "graph/algorithms/decode.h"
#include "utils/common.h"
#include "utils/logger.h"
#include "utils/file.h"
#include "../clique/clique.h"
#include "../interfaces/interface_partition.h"
#include "../clique/algorithms/clique_partition.h"

using namespace std;

#define TEST_PATH_BIG_GRAPHS		"C:/Users/Pablo/Desktop/bigGraphs/"
#define TEST_PATH_RANDOM_GRAPHS		"C:/Users/Pablo/Desktop/random/"

#define TEST_PATH_BIG_GRAPHS_LINUX	 "/media/datos/graphs/large_sparse/benchmark/dimacs/"

TEST(Cover, basic){
///////////////////
// tests adt cover_t type
	LOG_INFO("Cover:basic-----------------------");
	
	vint lv;
	cover_t<vint> cover;		/* set {0}, {1}, {2} as cover */
	lv.push_back(0);
	cover.add_set(lv);
	lv.erase(std::remove(lv.begin(), lv.end(), 0));
	lv.push_back(1);
	cover.add_set(lv);
	lv.erase(std::remove(lv.begin(), lv.end(), 1));
	lv.push_back(2);
	cover.add_set(lv);  
	
	EXPECT_TRUE(cover.is_cover(3));
	EXPECT_TRUE(cover.is_partition(3));

	vint& lvc=cover.cover.front();    /* {0} element */
	com::stl::print_collection(lvc);
	lvc.clear();
		
	EXPECT_FALSE(cover.is_cover(3));
	EXPECT_FALSE(cover.is_partition(3));
	
	LOG_INFO("----------------------------------");
}

TEST(Cover, clq_partition){
///////////////////
// tests adt cover_t type
	LOG_INFO("Cover:clq_partition-----------------");
	
	vint lv;
	cover_t<vint> cover;		/* set {0}, {1}, {2} as cover */
	lv.push_back(0);
	cover.add_set(lv);
	lv.erase(std::remove(lv.begin(), lv.end(), 0));
	lv.push_back(1);
	cover.add_set(lv);
	lv.erase(std::remove(lv.begin(), lv.end(), 1));
	lv.push_back(2);
	cover.add_set(lv);  

	ugraph ug(3);
	ug.add_edge(0,1);
	ug.add_edge(0,2);
	EXPECT_TRUE(cover.is_clq_cover(ug));
	EXPECT_TRUE(cover.is_clq_partition(ug));

	cover.cover[0].push_back(1);					/* set {0, 1}, {1}, {2} as cover */
	EXPECT_TRUE(cover.is_clq_cover(ug));
	EXPECT_FALSE(cover.is_clq_partition(ug));

	cover.cover[1].clear();						
	EXPECT_TRUE(cover.is_clq_cover(ug));			/* {0,1} {}, {2} as cover */
	EXPECT_TRUE(cover.is_clq_partition(ug));	
		
	LOG_INFO("----------------------------------");
}


TEST(Simple_Partition_Clique, basic){
	LOG_INFO("Simple_Partition_Clique:basic-----------------------");

	//Ugraph
	const int NV=6;
	ugraph ug(NV);
	ug.add_edge(0, 1);		
	ug.add_edge(0, 2);
	ug.add_edge(1, 2);
	ug.print_data();

	CliquePartition cp(&ug);
	const cover_t<vint>& part=cp.compute_partition();
	
	EXPECT_TRUE(cp.verify());
	LOG_INFO("-----------------------------------------------------");
}

TEST(Simple_Partition_Clique, brock200_1){
	LOG_INFO("Simple_Partition_Clique:brock200_1--------------------");

	//Ugraph	
	ugraph ug("brock200_1.clq");
	const int NV=ug.number_of_vertices();
	ug.print_data();

	CliquePartition cp(&ug);
	const cover_t<vint>& part=cp.compute_partition();
		
	EXPECT_TRUE(cp.verify());
	LOG_INFO("-------------------------------------------------------");
}

TEST(Simple_Partition_Clique, random){
	LOG_INFO("Simple_Partition_Clique:random-------------------------");

	//Ugraph	
	string filename(TEST_PATH_RANDOM_GRAPHS);
	filename+="r500_0.5_0.txt";

	ugraph ug(filename);
	const int NV=ug.number_of_vertices();
	ug.print_data();

	CliquePartition cp(&ug);
	const cover_t<vint>& part=cp.compute_partition();
	cp.print();

	EXPECT_TRUE(cp.verify());
	LOG_INFO("-------------------------------------------------------");
}

TEST(DISABLED_Simple_Partition_Clique, big_graphs_rgg_n_2_15_s0){
	LOG_INFO("Simple_Partition_Clique:rgg_n_2_15_s0------------------");

	//Ugraph	
	string filename(TEST_PATH_BIG_GRAPHS);
	filename+="rgg_n_2_15_s0.clq";

	ugraph ug(filename);
	const int NV=ug.number_of_vertices();
	ug.print_data();

	CliquePartition cp(&ug);
	const cover_t<vint>& part=cp.compute_partition();
	
	EXPECT_TRUE(cp.verify());
	LOG_INFO("-------------------------------------------------------");
}

TEST(Simple_Partition_Clique, big_graphs_cond_mat){
	LOG_INFO("Simple_Partition_Clique:big_graphs_cond_mat-----------------------");

	//Ugraph	
	string filename(TEST_PATH_BIG_GRAPHS);
	filename+="cond-mat-2003.clq";

	ugraph ug(filename);
	const int NV=ug.number_of_vertices();
	ug.print_data();

	CliquePartition cp(&ug);
	const cover_t<vint>& part=cp.compute_partition();


	EXPECT_TRUE(cp.verify());
	LOG_INFO("-------------------------------------------------------")	;
}

TEST(Simple_Partition_Clique, big_graphs_fe_body){
	LOG_INFO("Simple_Partition_Clique:big_graphs_fe_body-----------------------");

	//Ugraph	
	string filename(TEST_PATH_BIG_GRAPHS);
	filename+="fe-body.mtx.clq";

	ugraph ug(filename);
	const int NV=ug.number_of_vertices();
	ug.print_data();

	CliquePartition cp(&ug);
	const cover_t<vint>& part=cp.compute_partition();
	
	EXPECT_TRUE(cp.verify());
	LOG_INFO("-------------------------------------------------------")	;
	cin.get();
}

TEST(Partition_Clique, isolani){
	LOG_INFO("Partition_Clique:isolani-----------------------");
	Partition::result_t r;

	//Ugraph
	const int NV=6;
	ugraph ug(NV);
				
	Partition par(&ug);
	par.set_up();
	par.run();
	EXPECT_EQ(6, par.get_result().get_upper_bound());

	//checks it is a disjoint clique partition and prints the result on screen
	const vector<vint>& clqpar=par.get_partition();
	int size=0;
	Clique<ugraph> clq(&ug, clqo::param_t());

	LOG_INFO("The partition is:");
	for(int i=0; i<clqpar.size(); i++){
		com::stl::print_collection(clqpar[i]); cout<<endl;
		vint cpy(clqpar[i]);
		EXPECT_TRUE(clq.is_clique(cpy));
		size+=clqpar[i].size();
	}
	EXPECT_EQ(size, ug.number_of_vertices());
		
	LOG_INFO("-------------------------------------------------------")	;
}

TEST(Partition_Clique, isolani_part){
	LOG_INFO("Partition_Clique:isolani_part-----------------------");
	Partition::result_t r;

	//Ugraph
	const int NV=6;
	ugraph ug(NV);
	ug.add_edge(0, 1);		
	ug.add_edge(0, 2);
	ug.add_edge(1, 2);
			
	Partition par(&ug);
	par.set_up();
	par.run();
	EXPECT_EQ(9, par.get_result().get_upper_bound());
	EXPECT_TRUE(par.verify());

	//checks it is a disjoint clique partition and prints the result on screen
	const vector<vint>& clqpar=par.get_partition();
	int size=0;
	Clique<ugraph> clq(&ug, clqo::param_t());
	LOG_INFO("The partition is:");
	for(int i=0; i<clqpar.size(); i++){
		com::stl::print_collection(clqpar[i]); cout<<endl;
		vint cpy(clqpar[i]);
		EXPECT_TRUE(clq.is_clique(cpy));
		size+=clqpar[i].size();
	}
	EXPECT_EQ(size, ug.number_of_vertices());
	
	LOG_INFO("-------------------------------------------------------")	;
}

TEST(Partition_Clique, isolani_part_2){
	LOG_INFO("Partition_Clique:isolani_part_2-----------------------");
	Partition::result_t r;

	//Ugraph
	const int NV=6;
	ugraph ug(NV);
	ug.add_edge(0, 1);		
	ug.add_edge(0, 2);
	ug.add_edge(1, 2);
			
	Partition par(&ug);
	par.set_up();
	par.run();
	EXPECT_EQ(9, par.get_result().get_upper_bound());
	EXPECT_TRUE(par.verify());

	//checks it is a disjoint clique partition and prints the result on screen
	const vector<vint>& clqpar=par.get_partition();
	int size=0;
	Clique<ugraph> clq(&ug, clqo::param_t());
	LOG_INFO("The partition is:");
	for(int i=0; i<clqpar.size(); i++){
		com::stl::print_collection(clqpar[i]); cout<<endl;
		vint cpy(clqpar[i]);
		EXPECT_TRUE(clq.is_clique(cpy));
		size+=clqpar[i].size();
	}
	EXPECT_EQ(size, ug.number_of_vertices());

	
	LOG_INFO("-------------------------------------------------------")	;
}

TEST(Partition_Clique, brock200_1){
	LOG_INFO("Partition_Clique:brock200_1-----------------------");
	Partition::result_t r;

	//Ugraph
	ugraph ug("brock200_1.clq");
	
	Partition par(&ug);
	par.set_up();
	par.run();
	//EXPECT_EQ(1674, par.min_sum_col_LB());
	EXPECT_TRUE(par.verify());
	par.print(FILE_LOG("log.txt", WRITE));

	//checks it is a disjoint clique partition and prints the result on screen
	const vector<vint>& clqpar=par.get_partition();
	int size=0;
	Clique<ugraph> clq(&ug, clqo::param_t());
	LOG_INFO("The partition is:");
	for(int i=0; i<clqpar.size(); i++){
		com::stl::print_collection(clqpar[i]); cout<<endl;
		vint cpy(clqpar[i]);
		EXPECT_TRUE(clq.is_clique(cpy));
		size+=clqpar[i].size();
	}
	EXPECT_EQ(size, ug.number_of_vertices());
		
	LOG_INFO("-------------------------------------------------------")	;
}

TEST(Partition_Clique, zeroin_i_2){
	LOG_INFO("Partition_Clique:zeroin.i.2-----------------------");
	Partition::result_t r;

	//Ugraph
	ugraph ug("./color/zeroin.i.2.col");
		
	Partition par(&ug);
	par.set_up();
	par.run();
	EXPECT_TRUE(par.verify());
	par.print(FILE_LOG("log.txt", WRITE));

	//checks it is a disjoint clique partition and prints the result on screen
	const vector<vint>& clqpar=par.get_partition();
	int size=0;
	Clique<ugraph> clq(&ug, clqo::param_t());
	LOG_INFO("The partition is:");
	for(int i=0; i<clqpar.size(); i++){
		com::stl::print_collection(clqpar[i]); cout<<endl;
		vint cpy(clqpar[i]);
		EXPECT_TRUE(clq.is_clique(cpy));
		size+=clqpar[i].size();
	}
	EXPECT_EQ(size, ug.number_of_vertices());
		
	LOG_INFO("-------------------------------------------------------")	;
}

TEST(Partition_Clique, mug100_25){
	LOG_INFO("Partition_Clique:lookahead-----------------------");
	Partition::result_t r;
		
	ugraph ug("./color/mug100_25.col");
	
	//SATPLAN
	//ugraph ug("./satplan/hanoi/conflict_hanoi4.txt");	
	//ugraph ug("./satplan/hanoi/conflict_hanoi8.txt");	
	//ugraph ug("./satplan/slidetile/conflict_slidetile_01_hard1.txt");	
	//ugraph ug("./satplan/blocksworld/conflict_blocksw02.txt");	º
	//ugraph ug("./satplan/robots/graph_Pm01.txt");	
	//ugraph ug("./satplan/aisplog/conflict_acipslog04.txt");	
	//ugraph ug("./satplan/aisplog/conflict_aipslog01.txt");
	
	Partition par(&ug);
	par.set_up();
	par.run();
	EXPECT_TRUE(par.verify());
	par.print(FILE_LOG("log_conflict_aipslog01.txt", WRITE));

	//checks it is a disjoint clique partition and prints the result on screen
	const vector<vint>& clqpar=par.get_partition();
	int size=0;
	Clique<ugraph> clq(&ug, clqo::param_t());
	LOG_INFO("The partition is:");
	for(int i=0; i<clqpar.size(); i++){
		com::stl::print_collection(clqpar[i]); cout<<endl;
		vint cpy(clqpar[i]);
		EXPECT_TRUE(clq.is_clique(cpy));
		size+=clqpar[i].size();
	}
	EXPECT_EQ(size, ug.number_of_vertices());
		
	LOG_INFO("-------------------------------------------------------")	;
}




