//batch benchmark tests for different algorithms

#include <iostream>
#include "../batch/batch_benchmark.h"
#include "../batch/benchmark_clique.h"
#include "utils/file.h"
#include "gtest/gtest.h"
#include "../clique/clique_watched.h"

using namespace std;


//#define BENCHMARK_PATH		"/media/datos/graphs/large_sparse"
#define BENCHMARK_PATH			"C:/Users/i7/Desktop/dimacs"	
#define SINGLE_INSTANCE_PATH	"C:\\Users\\pablo\\Desktop\\dimacs\\brock200_1.clq"
#define LOG						"/var/www/logs/results_semi_sparse"

TEST(BatchBenchmark, BBMC_T){

	cout<<"BatchBenchmark:BBMC_T--------------------------------"<<endl;
	BkCliqueILS bcl(BENCHMARK_PATH);
	bcl.SubsetEasyDimacsILS();

	//the bacth object for tests
	BatchCLQBk<ugraph, Clique<ugraph> > batch;
	
	//some params
	clqo::param_t p;
	p.tout=3600;
	p.unrolled=false;
		
	p.alg=clqo::BBMC;
	batch.add_test<Clique<ugraph>>(p);
	
	p.alg=clqo::BBMC_T;
	batch.add_test<Clique<ugraph>>(p);

	int ret=batch.compare_two_tests(bcl, clqo::DIFFERENT_STEPS, FILE_LOG(LOG, APPEND));
	EXPECT_EQ(0, ret);			//no difference in steps

	cout<<"------------------------------------------------------"<<endl;
}

TEST(BatchBenchmark, BBMC_T_unrolled){

	cout<<"BatchBenchmark:BBMC_T_unrolled--------------------------------"<<endl;
	BkCliqueILS bcl(BENCHMARK_PATH);
	bcl.SubsetEasyDimacsILS();

	//the bacth object for tests
	BatchCLQBk<ugraph, Clique<ugraph> > batch;

	//some params
	clqo::param_t p;
	p.tout=3600;
	p.unrolled=false;
	
	//add tests
	p.alg=clqo::BBMC;
	batch.add_test<Clique<ugraph>>(p);
	
	p.alg=clqo::BBMC_T;
	batch.add_test<Clique<ugraph>>(p);

	int ret=batch.compare_two_tests(bcl, clqo::DIFFERENT_STEPS, FILE_LOG(LOG, APPEND));
	EXPECT_EQ(0, ret);			//no difference in steps

	cout<<"------------------------------------------------------"<<endl;
}

TEST(BatchBenchmarkSparse, BBMC_T){

	cout<<"BatchBenchmarkSparse:BBMC_T_unrolled--------------------"<<endl;
	BkCliqueILS bcl(BENCHMARK_PATH);
	bcl.SubsetEasyDimacsILS();

	//the bacth object for tests
	BatchCLQBk<sparse_ugraph, Clique<sparse_ugraph> > batch;

	//some params
	clqo::param_t p;
	p.tout=3600;
	p.unrolled=false;
	
	//add tests
	p.alg=clqo::BBMC;
	batch.add_test<Clique<sparse_ugraph>>(p);
	
	p.alg=clqo::BBMC_T;
	batch.add_test<Clique<sparse_ugraph>>(p);

	int ret=batch.compare_two_tests(bcl, clqo::DIFFERENT_STEPS, FILE_LOG(LOG, APPEND));
	EXPECT_EQ(0, ret);			//no difference in steps

	cout<<"------------------------------------------------------"<<endl;
}
