//tests for mixed initial sorting with the different variants
//author: pablo san segundo
//date: 25/11/2015

#include <iostream>
#include "../batch/batch_benchmark.h"
#include "../batch/batch_gen.h"
#include "../batch/benchmark_clique.h"
#include "utils/file.h"
#include "../clique/clique_sat.h"
//#include "../clique/clique_watched.h"
#include "../clique/clique_iter.h"

#define BENCHMARK_PATH				"C:/Users/pablo/Desktop/dimacs"	
#define BENCHMARK_RANDOM_PATH		"C:/Users/pablo/Desktop/random"	
#define LOG							 "C:/Users/pablo/Desktop/init_sorting_bbmc.txt"	
#define LOG_RANDOM					 "C:/Users/pablo/Desktop/init_sorting_rnd_bbmc.txt"	
#define WRITE_PATH				     "C:\\Users\\i7\\Desktop/"	
//#define SINGLE_INSTANCE_PATH_POSIX	"/media/datos/graphs/frb_zavalnij/evil-myc-fbr11-k20.clq"
//#define SINGLE_INSTANCE_PATH		"C:\\Users\\pablo\\Desktop\\2dc.2048.txt.gz.txt.clq"
#define SINGLE_INSTANCE_PATH		"C:/Users/pablo/Desktop/brock400_1.clq"
//#define SINGLE_INSTANCE_PATH		"C:/Users/pablo/Desktop/evil-myc-fbr11-k16.clq"
//#define SINGLE_INSTANCE_PATH		"C:/Users/pablo/Desktop/keller5_rlf.clq"
//#define SINGLE_INSTANCE_PATH		"C:/Users/pablo/Desktop/r200_0.95_4.txt"
//#define SINGLE_INSTANCE_PATH		"C:/Users/pablo/Desktop/c-fat200-1.clq"


#define LOG_POSIX					 "/var/www/logs/bbmc_sat_all_rlf_refactored.txt"
#define BENCHMARK_DIRECTORY_POSIX	 "/media/datos/graphs/dimacs_bhosh/"
#define SINGLE_INSTANCE_PATH_POSIX	"/media/datos/graphs/coding/cliqueVersion/2dc.2048.txt.gz.txt.clq"

int main(){
	Logger::SetInformationLevel(LOGGER_INFO);
	PrecisionTimer pt;
	BatchCLQBk<ugraph, Clique<ugraph> > batch;
		
	//configure parameters for all tests
	clqo::param_t p;
	//p.tout=10800;								//seconds
	//p.tout=3600*6;							
	p.unrolled=false;
	p.init_order=clqo::MIN_WIDTH;	

	//p.alg = clqo::BBMC;
	//batch.add_test<Clique<ugraph>>(p);

	p.init_order=clqo::MIXED_4;		
	p.alg = clqo::BBMCXR_L;
	batch.add_test<CliqueSat>(p);

	p.init_order=clqo::MIXED_4;		
	p.alg = clqo::BBMCITXR_L;
	batch.add_test<CliqueIter>(p);

	/*p.init_order=clqo::MIXED_4;		
	p.alg = clqo::BBMCX;
	batch.add_test<CliqueSat>(p);

	p.init_order=clqo::MIXED_8;		
	p.alg = clqo::BBMCX;
	batch.add_test<CliqueSat>(p);

	p.init_order=clqo::MIXED_10;		
	p.alg = clqo::BBMCX;
	batch.add_test<CliqueSat>(p);*/
	
	BkClique bcl(BENCHMARK_PATH);
	bcl.C();		
	batch.benchmark_test(bcl, FILE_LOG(LOG, WRITE), 1);
	//batch.run_single_instance(SINGLE_INSTANCE_PATH, FILE_LOG(LOG, WRITE));
			  
	/*random_attr_t rt(200, 200, .9, .9, 10, 50, .5);
	batch.random_test(BENCHMARK_RANDOM_PATH,rt, FILE_LOG(LOG_RANDOM, APPEND));*/
}

//int main(){
//	Logger::SetInformationLevel(LOGGER_INFO);
//	PrecisionTimer pt;
//	
//	//random_attr_t rt(10, 50, .1, .9, 100, 1, .1);
//	//random_attr_t rt(100, 150, .5, .91, 100, 50, .1);
//	//random_attr_t rt(150, 200, .7, .91, 10, 50, .1);
//	// random_attr_t rt(500, 500, .994, .995, 10, 50, .1);
//	//random_attr_t rt(3000, 3001, .1, .11, 10, 50, .1);
//	//random_attr_t rt(5000, 5001, .1, .11, 10, 50, .1);
//	//random_attr_t rt(10000, 10001, .1, .11, 10, 50, .1);
//	//random_attr_t rt(15000, 15001, .1, .11, 10, 50, .1);
//	//random_attr_t rt(20000, 30001, .1, .11, 10, 5000, .1);
//	//random_attr_t rt(30000, 50001, .05, .06, 10, 10000, .1);
//	//random_attr_t rt(100000, 100001, .05, .06, 1, 10000, .1);
//	//random_attr_t rt(100000, 100001, .05, .06, 1, 25000, .1);
//
//	//random_attr_t rt(3000, 5001, .1, .11, 10, 2000, .1);
//	//random_attr_t rt(10000, 30001, .1, .11, 10, 5000, .1);
//	  
//	random_attr_t rt(200, 200, .9, .9, 10, 50, .5);
//
//	//random_attr_t rt(500, 500, .994, .994, 10, 50, .1);
//
//	//A batch object for tests with BBMC watched variants
//	BatchCLQGen<ugraph, Clique<ugraph> > batch;
//	
//	//configure parameters for all tests
//	param_t p;
//	p.tout=3600;			 //seconds
//	p.unrolled=false;
//	p.init_order=MIN_WIDTH_MIN_TIE_STATIC;
//
//   /* p.alg = BBMC;
//	batch.add_test<Clique<ugraph>>(p);*/
//
//	p.alg = BBMCX;
//	batch.add_test<CliqueSat>(p);
//
//	p.alg = BBMCX_L;
//	batch.add_test<CliqueSat>(p);
//
//	p.alg = BBMCXR;
//	batch.add_test<CliqueSat>(p);
//
//	p.alg = BBMCXR_L;
//	batch.add_test<CliqueSat>(p);
//
//	p.alg = BBMCXR_L_SEQ;
//	batch.add_test<CliqueSat>(p);
//		
//			
//	//batch.find_property(WRITE_PATH, rt);
//	//batch.compare_two_tests(WRITE_PATH, rt,  DIFFERENT_CLIQUE_SIZE, FILE_LOG(LOG, WRITE));
//	batch.random_test(""/*STORE_INSTANCE_PATH*/, rt, FILE_LOG(LOG_RANDOM, APPEND));
//	
//}









