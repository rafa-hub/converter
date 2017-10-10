//example of batch benchmark tests for new small and middle-size sparse variant BBMCSSP (*experimental*)
//author: pablo san segundo
//date: 19/5/2015

#include <iostream>
#include "../batch/batch_benchmark.h"
#include "../batch/benchmark_clique.h"
#include "utils/file.h"
#include "../clique/clique_watched.h"

#define BENCHMARK_PATH			 "/media/datos/graphs/large_sparse/benchmark/"
#define BENCHMARK_RANDOM_PATH	 "/media/datos/graphs/random/"
#define LOG						 "/var/www/logs/results_semi_sparse/log_bbmcssp_301_700.txt"
//#define SINGLE_INSTANCE_PATH	"C:\\Users\\pablo\\Desktop\\dimacs\\brock200_1.clq"

int main(){
	Logger::SetInformationLevel(LOGGER_INFO);
	PrecisionTimer pt;
	BkClique bcl(BENCHMARK_PATH);
	bcl.Real301_700();

	BatchCLQBk<ugraph, CliqueWatched > batch;
		
	//configure parameters for all tests
	clqo::param_t p;
	//p.tout=7200;  //segundos
	p.unrolled=false;
	p.init_order=clqo::MIN_WIDTH_KCORE;
		
	p.alg = clqo::BBMC_W;
	batch.add_test<CliqueWatched>(p);

	p.alg = clqo::BBMC_WT;
	batch.add_test<CliqueWatched>(p);
		
	batch.benchmark_test(bcl, FILE_LOG(LOG, APPEND), 1);
	//batch.run_single_instance(SINGLE_INSTANCE_PATH, cout);
}

////////////
//// random benchmarks
//int main(){
//	Logger::SetInformationLevel(LOGGER_INFO);
//	PrecisionTimer pt;
//	
//	BkClique bcl(BENCHMARK_PATH);
//	//bcl.Real50();
//
//	/*random_attr_t rt(150, 151, .7, .91, 10, 50, .1);
//	random_attr_t rt(150, 151, .95, .99, 10, 50, .03);
//	random_attr_t rt(200, 200, .7, .91, 10, 50, .1);
//	random_attr_t rt(200, 200, .95, .99, 10, 50, .03);
//	random_attr_t rt(300, 300, .6, .81, 10, 50, .1);*/
//	//random_attr_t rt(500, 500, .4, .71, 10, 50, .1);
//	//random_attr_t rt(3000, 3001, .2, .21, 10, 50, .1);
//	random_attr_t rt(5000, 5001, .2, .21, 10, 50, .1);
//	BatchCLQBk<ugraph, CliqueWatched > batch;
//		
//	//configure parameters for all tests
//	param_t p;
//	//p.tout=3600;  //segundos
//	p.unrolled=false;
//	p.init_order=MIN_WIDTH_KCORE;
//		
//	p.alg = BBMC_W;
//	batch.add_test<CliqueWatched>(p);
//
//	p.alg = BBMC_WT;
//	batch.add_test<CliqueWatched>(p);
//		
//	batch.random_test(BENCHMARK_RANDOM_PATH, rt, FILE_LOG(LOG, APPEND));
//	//batch.benchmark_test(bcl, FILE_LOG(LOG, WRITE), 1);
//	//batch.run_single_instance(SINGLE_INSTANCE_PATH, cout);
//}

//////////////
//// random benchmarks
//int main(){
//	Logger::SetInformationLevel(LOGGER_INFO);
//	PrecisionTimer pt;
//	//BkClique bcl(BENCHMARK_PATH);
//	//bcl.Real50();
//
//	/*random_attr_t rt(150, 151, .7, .91, 10, 50, .1);
//	random_attr_t rt(150, 151, .95, .99, 10, 50, .03);
//	random_attr_t rt(200, 200, .7, .91, 10, 50, .1);
//	random_attr_t rt(200, 200, .95, .99, 10, 50, .03);
//	random_attr_t rt(300, 300, .6, .81, 10, 50, .1);*/
//	//random_attr_t rt(500, 500, .4, .71, 10, 50, .1);
//	//random_attr_t rt(3000, 3001, .2, .21, 10, 50, .1);
//	//random_attr_t rt(5000, 5001, .2, .21, 10, 50, .1);
//	random_attr_t rt(30000, 30001, .1, .11, 5, 50, .1);
//	BatchCLQBk<ugraph, Clique<ugraph> > batch;
//		
//	//configure parameters for all tests
//	param_t p;
//	//p.tout=3600;  //segundos
//	p.unrolled=false;
//	p.init_order=MIN_WIDTH_KCORE;
//		
//	p.alg = BBMC;
//	batch.add_test<Clique<ugraph>>(p);
//				
//	batch.random_test(BENCHMARK_RANDOM_PATH, rt, FILE_LOG(LOG, APPEND));
//	//batch.benchmark_test(bcl, FILE_LOG(LOG, WRITE), 1);
//	//batch.run_single_instance(SINGLE_INSTANCE_PATH, cout);
//}




