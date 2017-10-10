//batch random tests for different algorithms

#include <iostream>
#include "../batch/batch_gen.h"
#include "../batch/benchmark_clique.h"
#include "utils/file.h"
#include "gtest/gtest.h"
#include "../clique/clique.h"
#include "../clique/clique_watched.h"

using namespace std;

#define BENCHMARK_PATH			 "C:\\Users\\pablo\\Desktop\\dimacs"
#define RANDOM_BENCHMARK_PATH	 "C:\\Users\\pablo\\Desktop\\random"
#define SINGLE_INSTANCE_PATH	 "C:\\Users\\pablo\\Desktop\\dimacs\\brock200_1.clq"
#define LOG						 "C:\\Users\\pablo\\Desktop\\kk.txt"
#define STORE_INSTANCE_PATH		 "C:\\Users\\pablo\\Desktop\\set"


TEST(BatchGen, BBMCL_R){
	cout<<"BatchGen:BBMCL_R--------------------------------";
	random_attr_t rt(100, 200, .1, .6, 100, 50, .1);
	BatchCLQGen<ugraph, Clique<ugraph> > batch;

	clqo::param_t p;
	p.tout=3600;
	p.unrolled=false;
	
	p.alg = clqo::BBMC;
	batch.add_test<Clique<ugraph>>(p);
	
	p.alg = clqo::BBMCL_R;
	batch.add_test<Clique<ugraph>>(p);

	int ret=batch.compare_two_tests(STORE_INSTANCE_PATH, rt, clqo::DIFFERENT_CLIQUE_SIZE, FILE_LOG(LOG, WRITE));
	EXPECT_EQ(0, ret);
	cout<<"----------------------------------------------";
}

TEST(BatchGen, BBMCL_T){
	cout<<"BatchGen:BBMCL_T--------------------------------";
	random_attr_t rt(100, 200, .1, .6, 100, 50, .1);
	BatchCLQGen<ugraph, Clique<ugraph> > batch;


	clqo::param_t p;
	p.tout=3600;
	p.unrolled=false;
	
	p.alg = clqo::BBMC;
	batch.add_test<Clique<ugraph>>(p);
	
	p.alg = clqo::BBMCL_T;
	batch.add_test<Clique<ugraph>>(p);

	int ret=batch.compare_two_tests(STORE_INSTANCE_PATH, rt, clqo::DIFFERENT_CLIQUE_SIZE, FILE_LOG(LOG, WRITE));
	EXPECT_EQ(0, ret);
	cout<<"----------------------------------------------";
}

TEST(BatchGen, BBMCL){
	cout<<"BatchGen:BBMCL--------------------------------";
	random_attr_t rt(100, 200, .1, .6, 100, 50, .1);
	BatchCLQGen<ugraph, Clique<ugraph> > batch;


	clqo::param_t p;
	p.tout=3600;
	p.unrolled=false;
	
	p.alg = clqo::BBMC;
	batch.add_test<Clique<ugraph>>(p);
	
	p.alg = clqo::BBMCL;
	batch.add_test<Clique<ugraph>>(p);

	int ret=batch.compare_two_tests(STORE_INSTANCE_PATH, rt, clqo::DIFFERENT_CLIQUE_SIZE, FILE_LOG(LOG, WRITE));
	EXPECT_EQ(0, ret);
	cout<<"----------------------------------------------";
}

TEST(BatchGen, BBMCR){
	cout<<"BatchGen:BBMCR--------------------------------";
	random_attr_t rt(100, 200, .1, .6, 100, 50, .1);
	BatchCLQGen<ugraph, Clique<ugraph> > batch;

	clqo::param_t p;
	p.tout=3600;
	p.unrolled=false;
	
	p.alg = clqo::BBMC;
	batch.add_test<Clique<ugraph>>(p);
	
	p.alg = clqo::BBMCR;
	batch.add_test<Clique<ugraph>>(p);

	int ret=batch.compare_two_tests(STORE_INSTANCE_PATH, rt, clqo::DIFFERENT_CLIQUE_SIZE, FILE_LOG(LOG, WRITE));
	EXPECT_EQ(0, ret);
	cout<<"----------------------------------------------";
}

TEST(BatchGen, BBMC_T){
	cout<<"BatchGen:BBMC_T--------------------------------";
	random_attr_t rt(100, 200, .1, .6, 100, 50, .1);
	BatchCLQGen<ugraph, Clique<ugraph> > batch;
	
	clqo::param_t p;
	p.tout=3600;
	p.unrolled=false;
	
	p.alg=clqo::BBMC;
	batch.add_test<Clique<ugraph>>(p);
	
	p.alg=clqo::BBMC_T;
	batch.add_test<Clique<ugraph>>(p);

	int ret=batch.compare_two_tests(STORE_INSTANCE_PATH, rt, clqo::DIFFERENT_STEPS, FILE_LOG(LOG, WRITE));
	EXPECT_EQ(0, ret);
	cout<<"----------------------------------------------";
}

TEST(BatchGen, BBMC_T_unrolled){
	cout<<"BatchGen:BBMC_T--------------------------------";
	random_attr_t rt(100, 200, .1, .6, 100, 50, .1);
	BatchCLQGen<ugraph, Clique<ugraph> > batch;
	
	clqo::param_t p;
	p.tout=3600;
	p.unrolled=true;
	
	p.alg=clqo::BBMC;
	batch.add_test<Clique<ugraph>>(p);
	
	p.alg=clqo::BBMC_T;
	batch.add_test<Clique<ugraph>>(p);

	int ret=batch.compare_two_tests(STORE_INSTANCE_PATH, rt, clqo::DIFFERENT_STEPS, FILE_LOG(LOG, WRITE));
	EXPECT_EQ(0, ret);
	cout<<"----------------------------------------------";
}

TEST(BatchGen, BBMC_Watched){
	cout<<"BatchGen:BBMC_WT--------------------------------";
	random_attr_t rt(100, 200, .1, .6, 100, 50, .1);
	BatchCLQGen<ugraph, CliqueWatched > batch;

	clqo::param_t p;
	p.tout=3600;
	p.unrolled=false;
	
	p.alg = clqo::BBMC_W;
	batch.add_test<CliqueWatched>(p);
	
	p.alg = clqo::BBMC_WT;
	batch.add_test<CliqueWatched>(p);

	int ret=batch.compare_two_tests(STORE_INSTANCE_PATH, rt, clqo::DIFFERENT_CLIQUE_SIZE, FILE_LOG(LOG, WRITE));
	EXPECT_EQ(0, ret);
	cout<<"----------------------------------------------";
}

TEST(BatchGenSparse, BBMC_T){
	cout<<"BatchGenSparse:BBMC_T--------------------------------";
	random_attr_t rt(100, 200, .1, .6, 50, 50, .1);
	BatchCLQGen<sparse_ugraph, Clique<sparse_ugraph> > batch;
	
	clqo::param_t p;
	p.tout=3600;
	p.unrolled=true;
	
	p.alg=clqo::BBMC;
	batch.add_test<Clique<sparse_ugraph>>(p);
	
	p.alg=clqo::BBMC_T;
	batch.add_test<Clique<sparse_ugraph>>(p);

	int ret=batch.compare_two_tests(STORE_INSTANCE_PATH, rt, clqo::DIFFERENT_STEPS, FILE_LOG(LOG, WRITE));
	EXPECT_EQ(0, ret);
	cout<<"----------------------------------------------";
}








