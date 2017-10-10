// examples of batch random collections of sets (both in a benchmark or generated on the fly)
// date: 14/09/15
// author: pss


#include <iostream>
#include <vector>
#include <algorithm>
#include "../batch/batch_benchmark.h"
#include "../batch/batch_gen.h"
#include "utils/file.h"
#include "gtest/gtest.h"
#include "../clique/clique.h"
#include "../clique/clique_watched.h"
#include  "../clique/clique_iter.h"
#include  "../clique/clique_sat.h"
#include "../clique/clique_enum.h"
#include "../clique/clique_infra.h"
#include "../clique/clique_infra_plus.h"
#include "../clique/clique_russian_doll.h"
#include "../clique/clique_russian_doll_plus.h"
#include "../clique/clique_weighted.h"
#include "../clique/clique_weighted_plus.h"


using namespace std;

#define STORE_INSTANCE_PATH		 "C:\\Users\\pablo\\Desktop\\set"
#define TIME_OUT_IN_SECONDS					3600/**24*/										//1h
//#define INIT_ORDER_STRATEGY				clqo::MIN_WEIGHTED
//#define INIT_ORDER_STRATEGY				clqo::MAX_WIDTH								//for enumeration
//#define INIT_ORDER_STRATEGY				clqo::RLF_COND
#define INIT_ORDER_STRATEGY					clqo::MIN_WIDTH
//#define INIT_ORDER_STRATEGY				clqo::MIN_WIDTH_MIN_TIE_STATIC


typedef vector<random_attr_t> v_rnd_t;

/////////////////////
// example code for different sets of  random instances in a benchmark

int main(){
	Logger::SetInformationLevel(LOGGER_INFO);
	PrecisionTimer pt;
	v_rnd_t tests;

//set of typical random benchmark tests
//	
	tests.push_back(random_attr_t(150, 150, .7, .9, 10, 50, .1));
	tests.push_back(random_attr_t(150, 150, .95, .95, 10, 50, .1));
	tests.push_back(random_attr_t(150, 150, .98, .98, 10, 50, .1));
	tests.push_back(random_attr_t(200, 200, .7, .9, 10, 50, .1)); 
	tests.push_back(random_attr_t(200, 200, .95, .95, 10, 50, .1));
	tests.push_back(random_attr_t(200, 200, .98, .98, 10, 50, .1));  
	tests.push_back(random_attr_t(300, 300, .6,  .8, 10, 50, .1));
	tests.push_back(random_attr_t(500, 500, .4, .7, 10, 50, .1));
//	tests.push_back(random_attr_t(500, 500, .994, .994, 10, 50, .1));
	tests.push_back(random_attr_t (1000, 1000, .2, .5, 10, 50, .1));
	tests.push_back(random_attr_t (3000, 3000, .1 , .2, 10, 50, .1));
	tests.push_back(random_attr_t (5000, 5000, .1, .2, 10, 50, .1));
	tests.push_back(random_attr_t (10000, 10000, .1, .1, 10, 50, .1));
	tests.push_back(random_attr_t (15000, 15000, .1, .1, 10, 50, .1));

	//tests.push_back(random_attr_t(300, 300, .6, .61, 10, 50, .1));
	//tests.push_back(random_attr_t(500, 500, .994, .994, 10, 50, .1));
	//tests.push_back(random_attr_t (10000, 10000, .1, .1, 10, 50, .1));
	//tests.push_back(random_attr_t (15000, 15000, .1, .1, 10, 50, .1));

	//tests.push_back(random_attr_t(500, 500, .994, .994, 10, 50, .1));

	//tests.push_back(random_attr_t(200, 200, .7, .7, 10, 50, .1));

//	tests.push_back(random_attr_t(150, 150, .95, .95, 10, 50, .1));
//	tests.push_back(random_attr_t(150, 150, .98, .98, 10, 50, .1));
//	tests.push_back(random_attr_t(200, 200, .95, .95, 10, 50, .1));
//	tests.push_back(random_attr_t(200, 200, .98, .98, 10, 50, .1));  
//	tests.push_back(random_attr_t(500, 500, .994, .994, 10, 50, .1));

//	tests.push_back(random_attr_t (15000, 15000, .1, .1, 10, 50, .1));

//set of typical ENUMERATION random benchmark tests
	/*tests.push_back(random_attr_t(100, 100, .6, .9, 10, 50, .1));
	tests.push_back(random_attr_t(300, 300, .1, .6, 10, 50, .1));
	tests.push_back(random_attr_t(500, 500, .1, .5, 10, 50, .1));
	tests.push_back(random_attr_t(700, 700, .1, .31, 10, 50, .1));
	tests.push_back(random_attr_t(1000, 1000, .1, .31, 10, 50, .1));
	tests.push_back(random_attr_t(2000, 2000, .1, .11, 10, 50, .1));
	tests.push_back(random_attr_t(3000, 3000, .1, .11, 10, 50, .1));
	tests.push_back(random_attr_t(10000, 10000, .001, .0051, 10, 50, .002));
	tests.push_back(random_attr_t (10000, 10000, .01, .031, 10, 50, .02));*/
////////////////////////////////////////////////

	//batch object		
	BatchCLQBk<ugraph, Clique<ugraph> > batch;
	
	//configure parameters for all tests
	clqo::param_t p;
	p.tout=TIME_OUT_IN_SECONDS;
	p.unrolled=false;
	p.init_order=INIT_ORDER_STRATEGY;
	p.init_preproc=clqo::UB_HEUR ;
	//p.init_preproc=clqo::UB;

	//add test
	/*p.alg = clqo::BBMC;
	batch.add_test<Clique<ugraph>>(p);*/
						
	/*p.alg = clqo::BBMCX;
	batch.add_test<CliqueSat>(p);*/
		
	/*p.alg = clqo::BBMCRL_KMIN;
	batch.add_test<CliqueInfra>(p);*/

	/*p.alg = clqo::BBMCR_DOLL;
	batch.add_test<CliqueDoll>(p);*/

	/*p.alg = clqo::BBMCX_WEIGHTED;
	batch.add_test<CliqueWeighted>(p);*/

	/*p.alg = clqo::BBMC_EN_GCONF_XP;
	batch.add_test<CliqueEnum>(p);

	p.alg = clqo::BBMC_EN_GCONF_XPm;
	batch.add_test<CliqueEnum>(p);*/
		
	/*p.alg = clqo::BBMCITXR_L;
	batch.add_test<CliqueIter>(p);

	p.alg = clqo::BBMCXR_L;
	batch.add_test<CliqueSat>(p);*/

	/*p.alg = clqo::BBMCR_DOLL;
	batch.add_test<CliqueDollPlus>(p);*/
	
	p.alg = clqo::BBMCL;
	batch.add_test<CliqueInfraPlus>(p);

	//p.alg = clqo::BBMCL_PLUS;
	//batch.add_test<CliqueInfraPlus>(p);
		
	//p.init_preproc=clqo::UB;
	//p.init_preproc=clqo::UB_HEUR;

	/*p.alg = clqo::BBMCL_R;
	batch.add_test<CliqueInfraPlus>(p);*/

	/*p.alg = clqo::BBMCR_SAT_WEIGHTED;
	batch.add_test<CliqueWeighted>(p);*/
	
	/*p.alg = clqo::BBMC_KMIN;
	batch.add_test<CliqueInfra>(p);

	p.alg = clqo::BBMC_KMIN_SORT;
	batch.add_test<CliqueInfra>(p);
	
	p.alg = clqo::BBMCR_KMIN;
	batch.add_test<CliqueInfra>(p);

	p.alg = clqo::BBMCR_KMIN_LOOK_AHEAD;
	batch.add_test<CliqueInfra>(p);

	p.alg = clqo::BBMCR_KMIN_2C;
	batch.add_test<CliqueInfra>(p);

	p.alg = clqo::BBMCR_KMIN_SORT;
	batch.add_test<CliqueInfra>(p);

	p.alg = clqo::BBMCR_KMIN_LOOK_AHEAD_SORT;
	batch.add_test<CliqueInfra>(p);

	p.alg = clqo::BBMCR_KMIN_SORT_2C;
	batch.add_test<CliqueInfra>(p);*/
	
	//p.alg = clqo::BBMC_EN_GCONF;
	//batch.add_test<CliqueEnum>(p);
			
	//p.alg = clqo::BBMCX;
	//batch.add_test<CliqueSat>(p);

	//p.alg = clqo::BBMCXR_L;
	//batch.add_test<CliqueSat>(p);
	
	/*p.alg=BBMC_Iter_Root_Enlarge;
	batch.add_test<CliqueIter>(p);*/

	/*p.alg=BBMC_Iter_Root_Enlarge_Sat_R_N;
	batch.add_test<CliqueIter>(p);*/
		
	//p.alg = BBMCXR_LN;
	//batch.add_test<CliqueSat>(p);

	/*p.alg = BBMC_Iter_Root_Sat_R_N;
	batch.add_test<CliqueIter>(p);*/

//////////////////
// weighted tests

//	p.isomorphism=true;
//   /* p.iso_map.lhs=pair<gbbs::sort_t,gbbs::place_t>(gbbs::MIN_DEG_DEGEN_TIE_STATIC, gbbs::PLACE_FL);
//	p.iso_map.rhs=pair<gbbs::sort_t,gbbs::place_t>(gbbs::MAX_WEIGHT, gbbs::PLACE_FL); */
//
//	p.init_preproc=clqo::init_preproc_t::UB;
////	p.init_preproc=clqo::init_preproc_t::UB_HEUR;
//
//	//p.alg = clqo::BBMC_WEIGHTED_SHARED_REF;
//	//p.alg = clqo::BBMC_WEIGHTED_SHARED_TESTS;
//	//p.alg = clqo::BBMC_WEIGHTED_SHARED_PREPROC;
////	p.alg = clqo::BBMC_WEIGHTED_SHARED_PREPROC_CW;
////	p.alg = clqo::BBMC_WEIGHTED_SHARED_PREPROC_CW_RD;
////	p.alg = clqo::BBMC_WEIGHTED_SHARED_PREPROC_CW;
//
//	p.alg = clqo::BBMC_WEIGHTED_SHARED_PREPROC_CW_SUPER_WEIGHT;
//	batch.add_test<CliqueWeightedPlus>(p);
//
//	
//	/*p.alg = clqo::BBMC_WEIGHTED_SHARED_PREPROC_CW_SUPER_WEIGHT_3S;
//	batch.add_test<CliqueWeightedPlus>(p);*/
//			
//	
//	//p.init_order=clqo::init_order_t::MIN_WIDTH;	
//	//p.alg = clqo::BBMC_WEIGHTED_DOUBLE;
//	//batch.add_test<CliqueWeightedPlus>(p);

///////////////////////////
	
	////iterates over different sets of random instances in the benchmark
	//for(v_rnd_t::iterator it=tests.begin(); it!=tests.end(); it++){
	//	batch.random_test(BENCHMARK_RANDOM_PATH, (*it), FILE_LOG(LOG_RANDOM, APPEND),true);						/* with weights */
	//}

	//iterates over different sets of random instances in the benchmark
	for(v_rnd_t::iterator it=tests.begin(); it!=tests.end(); it++){
		batch.random_test(BENCHMARK_RANDOM_PATH, (*it), FILE_LOG(LOG_RANDOM, APPEND), false);						
	}

	//////iterates over different sets of random instances in the benchmark
	//for(v_rnd_t::iterator it=tests.begin(); it!=tests.end(); it++){
	//	batch.random_test(BENCHMARK_RANDOM_PATH_POSIX, (*it), FILE_LOG(LOG_RANDOM_POSIX, APPEND),true);			/* typically append, to accumulate the different tests */
	//}
}

///////////////////
// //example code for different sets of  random instances generated on the fly
//int main(){
//	Logger::SetInformationLevel(LOGGER_INFO);
//	PrecisionTimer pt;
//
//	//set of random tests
//	v_rnd_t tests;
//	tests.push_back(random_attr_t(10, 200, .5, .6, 10, 50, .1));
//	//tests.push_back(random_attr_t(10, 300, .1, .6, 10, 50, .1));
//	
//	//random_attr_t rt(100, 200, .1, .6, 100, 50, .1);
//	//random_attr_t rt(1000, 1001, .1, .51, 10, 50, .1);
//	//random_attr_t rt(3000, 5001, .2, .21, 10, 2000, .1);
//	//random_attr_t rt(10000, 25001, .1, .11, 10, 5000, .1);
//	//random_attr_t rt(20000, 30001, .1, .11, 10, 5000, .1);
//	// random_attr_t rt(30000, 30001, .1, .11, 5, 5000, .1);
//	// random_attr_t rt(30000, 50001, .05, .06, 10, 10000, .1);
//	//random_attr_t rt(100000, 100001, .05, .06, 1, 10000, .1);
//	//random_attr_t rt(75000, 75001, .05, .06, 3, 10000, .1);
//	//random_attr_t rt(100000, 100001, .05, .06, 1, 25000, .1);
//	//random_attr_t rt(10000, 30001, .1, .11, 10, 5000, .1);
//
//	BatchCLQGen<ugraph, Clique<ugraph> > batch;
//		
//	//configure common parameters for all tests
//	clqo::param_t p;
//	p.tout=TIME_OUT_IN_SECONDS;
//	p.unrolled=true;
//	p.init_order=INIT_ORDER_STRATEGY;
//
//	//add test 1
//	//p.alg=clqo::BBMC;
//	//batch.add_test<Clique<ugraph>>(p);
//	//
//	///*p.alg=clqo::BBMC_T;
//	//batch.add_test<Clique<sparse_ugraph>>(p);*/
//	//
//	////iterates over different sets of random instances in the benchmark
//	//for(v_rnd_t::iterator it=tests.begin(); it!=tests.end(); it++){
//	//	batch.random_test("", (*it), FILE_LOG(LOG_RANDOM, APPEND));
//	//}
//
//	//add weighted test (5/8/17)
//	p.init_preproc=clqo::UB;
//	//p.init_preproc=clqo::UB_HEUR;
//	p.init_order=clqo::init_order_t::MAX_WEIGHTED;	
//	p.alg = clqo::BBMC_WEIGHTED;
//	p.unrolled=false;
//	batch.add_test<CliqueWeightedPlus>(p);
//	
//	
//	//iterates over different sets of random instances in the benchmark
//	for(v_rnd_t::iterator it=tests.begin(); it!=tests.end(); it++){
//		batch.random_test("", (*it), FILE_LOG(LOG_RANDOM, APPEND), true);
//	}
//}

