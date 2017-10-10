//examples of batch random tests

#include <iostream>
#include "../batch/batch_gen.h"
#include "utils/file.h"
#include "gtest/gtest.h"
#include "../clique/clique.h"
#include "../clique/clique_watched.h"
#include "../clique/clique_weighted_plus.h"

using namespace std;

#define LOG						 "C:\\Users\\pablo\\Desktop\\kk.txt"
//#define LOG						"/var/www/logs/results_semi_sparse/log_bbmcsp_rnd.txt"
//#define STORE_INSTANCE_PATH		 "C:\\Users\\pablo\\Desktop"
#define STORE_INSTANCE_PATH		 "C:\\Users\\pablo\\Desktop"

int main(){
	Logger::SetInformationLevel(LOGGER_INFO);
	PrecisionTimer pt;

	//random_attr_t rt(7, 30, .1, .9, 100, 1, .1);
	//random_attr_t rt(50, 100, .1, .9, 100, 1, .1);

	//random_attr_t rt(100, 200, .1, .6, 100, 50, .1);
	//random_attr_t rt(150, 175, .5, .9, 100, 25, .1);
	//random_attr_t rt(1000, 1001, .3, .51, 10, 50, .1);
	//random_attr_t rt(3000, 5001, .2, .21, 10, 2000, .1);
	//random_attr_t rt(10000, 25001, .1, .11, 10, 5000, .1);
	//random_attr_t rt(20000, 30001, .1, .11, 10, 5000, .1);
	// random_attr_t rt(30000, 30001, .1, .11, 5, 5000, .1);
	// random_attr_t rt(30000, 50001, .05, .06, 10, 10000, .1);
	//random_attr_t rt(100000, 100001, .05, .06, 1, 10000, .1);
	//random_attr_t rt(75000, 75001, .05, .06, 3, 10000, .1);
	//random_attr_t rt(100000, 100001, .05, .06, 1, 25000, .1);
	
	//random_attr_t rt(5000, 5001, .2, .21, 10, 5000, .1);
	  random_attr_t rt(2000, 2001, .3, .31, 10, 5000, .1);

	//random_attr_t rt(10000, 30001, .1, .11, 10, 5000, .1);

	//batch object for random tests
	//BatchCLQGen<sparse_ugraph, Clique<sparse_ugraph> > batch;

	//batch object for random tests
	BatchCLQGen<ugraph, Clique<ugraph> > batch;
	
	//configure parameters for all tests
	clqo::param_t p;
	//p.tout=3600;
	//p.unrolled=true;
	p.unrolled=false;
	//p.init_order=MIN_WIDTH;
	//p.init_preproc=clqo::UB;
	//p.init_preproc=clqo::UB_HEUR;
	//p.init_order=clqo::init_order_t::MAX_WEIGHTED;	
	
	
	//add test
	/*p.alg=clqo::BBMC
	batch.add_test<Clique<sparse_ugraph>>(p);*/
	
	/*p.alg=BBMC_T;
	batch.add_test<Clique<sparse_ugraph>>(p);*/
		
	////reference test (5/8/17)
	//p.alg = clqo::BBMC_WEIGHTED_BASIC;
	//batch.add_test<CliqueWeightedPlus>(p);

	////add weighted test (5/8/17)
	//p.alg = clqo::BBMC_WEIGHTED;
	//batch.add_test<CliqueWeightedPlus>(p);

	//add weighted test (23/8/17)
	p.isomorphism=true;
	//p.init_preproc=clqo::UB_HEUR;
	p.init_preproc=clqo::UB;

	//p.alg = clqo::BBMC_WEIGHTED_SHARED_REF;
	//batch.add_test<CliqueWeightedPlus>(p);

	////add weighted test (23/8/17)
	//p.alg = clqo::BBMC_WEIGHTED_SHARED_TESTS;
	//batch.add_test<CliqueWeightedPlus>(p);
	//

	//add weighted test (23/8/17)
	p.alg = clqo::BBMC_WEIGHTED_SHARED_PREPROC;
	batch.add_test<CliqueWeightedPlus>(p);

	//simple random test
	batch.random_test( "", rt, FILE_LOG(LOG, WRITE),true);

	//search for a specific property
	//batch.find_property(STORE_INSTANCE_PATH, rt);

	//comparison tests between both algorithms (only two)
	//batch.compare_two_tests(STORE_INSTANCE_PATH, rt, clqo::DIFFERENT_CLIQUE_SIZE, FILE_LOG(LOG, WRITE),true);
	
	//batch.run_single_instance(SINGLE_INSTANCE_PATH, FILE_LOG(LOG, APPEND));
	
}