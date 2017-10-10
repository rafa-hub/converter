//example of batch random tests for new small and middle-size sparse variant BBMCSSP (*experimental*)
//author: pablo san segundo
//date: 19/5/2015

#include <iostream>
#include "../batch/batch_gen.h"
#include "utils/file.h"
#include "../clique/clique_watched.h"
#include "../clique/clique_types.h"

//#define STORE_INSTANCE_PATH	 "PATH TO STORE GENERATED RANDOM INSTANCES"
#define LOG						 "/var/www/logs/results_semi_sparse/log_bbmcssp_rnd.txt"
//
//int main(){
//	Logger::SetInformationLevel(LOGGER_INFO);
//	PrecisionTimer pt;
//	
//	random_attr_t rt(1000, 1001, .1, .51, 10, 50, .1);
//	//random_attr_t rt(3000, 3001, .1, .11, 10, 50, .1);
//	//random_attr_t rt(5000, 5001, .1, .11, 10, 50, .1);
//	//random_attr_t rt(10000, 10001, .1, .11, 10, 50, .1);
//	//random_attr_t rt(15000, 15001, .1, .11, 10, 50, .1);
//	//random_attr_t rt(20000, 30001, .1, .11, 10, 5000, .1);
//	//random_attr_t rt(30000, 50001, .05, .06, 10, 10000, .1);
//	//random_attr_t rt(100000, 100001, .05, .06, 1, 25000, .1);
//	  
//
//	//A batch object for tests with BBMC watched variants
//	BatchCLQGen<ugraph, CliqueWatched > batch;
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
//	batch.random_test(""/*STORE_INSTANCE_PATH*/, rt, FILE_LOG(LOG, APPEND));
//	
//}

int main(){
	Logger::SetInformationLevel(LOGGER_INFO);
	PrecisionTimer pt;
	
	random_attr_t rt(1000, 1001, .1, .51, 10, 50, .1);
	//random_attr_t rt(3000, 3001, .1, .11, 10, 50, .1);
	//random_attr_t rt(5000, 5001, .1, .11, 10, 50, .1);
	//random_attr_t rt(10000, 10001, .1, .11, 10, 50, .1);
	//random_attr_t rt(15000, 15001, .1, .11, 10, 50, .1);
	//random_attr_t rt(20000, 30001, .1, .11, 10, 5000, .1);
	//random_attr_t rt(30000, 50001, .05, .06, 10, 10000, .1);
	//random_attr_t rt(100000, 100001, .05, .06, 1, 10000, .1);
	//random_attr_t rt(75000, 75001, .05, .06, 5, 10000, .1);
	//random_attr_t rt(100000, 100001, .05, .06, 1, 25000, .1);

	//random_attr_t rt(3000, 5001, .1, .11, 10, 2000, .1);
	//random_attr_t rt(10000, 30001, .1, .11, 10, 5000, .1);
	  

	//A batch object for tests with BBMC watched variants
	BatchCLQGen<ugraph, Clique<ugraph> > batch;
	
	//configure parameters for all tests
	clqo::param_t p;
	//p.tout=3600;  //segundos
	p.unrolled=false;
	p.init_order=clqo::MIN_WIDTH_KCORE;
	
	p.alg = clqo::BBMC;
	batch.add_test<Clique<ugraph>>(p);
	
	batch.random_test(""/*STORE_INSTANCE_PATH*/, rt, FILE_LOG(LOG, APPEND));
	
}


