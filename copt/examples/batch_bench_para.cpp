//examples of batch benchmark tests for CliquePara class


#include <iostream>
#include "../batch/benchmark_clique.h"
#include "../batch/batch_gen.h"
#include "../batch/batch_benchmark.h"
#include "utils/file.h"
#include "gtest/gtest.h"
#include "../clique_para/clique_para.h"
#include "../clique_para/clique_para_sat.h"
#include "../clique_para/clique_para_iter.h"

using namespace std;

#define BENCHMARK_PATH				"/var/tmp/salida/dimacs"	
#define BENCHMARK_RANDOM_PATH		"/var/tmp/salida/random"	
#define LOG							 "/var/tmp/salida/kk_parallel.txt"	
#define LOG_RANDOM					 "/var/tmp/salida/kk_rnd_sat.txt"	
#define WRITE_PATH				     "/var/tmp/salida"	
#define SINGLE_INSTANCE_PATH_POSIX	"/media/datos/graphs/frb_zavalnij/evil-myc-fbr11-k20.clq"

//#define SINGLE_INSTANCE_PATH		"C:\\Users\\pablo\\Desktop\\2dc.2048.txt.gz.txt.clq"
#define SINGLE_INSTANCE_PATH		"/var/tmp/salida"
//#define SINGLE_INSTANCE_PATH		"/var/tmp/salida/evil-myc-fbr11-k16.clq"
//#define SINGLE_INSTANCE_PATH		"/var/tmp/salida/dimacs/brock200_1.clq"
//#define SINGLE_INSTANCE_PATH		"/var/tmp/salida/keller5_rlf.clq"
//#define SINGLE_INSTANCE_PATH		"/var/tmp/salida/r200_0.95_4.txt"
//#define SINGLE_INSTANCE_PATH		"/var/tmp/salida/c-fat200-1.clq"



#define LOG_POSIX				 "/var/www/logs/bbmcxrl_open_problem.txt"
#define BENCHMARK_PATH_POSIX	 "/media/datos/graphs/dimacs_bhosh/"
#define SINGLE_INSTANCE_PATH_POSIX	"/media/datos/graphs/coding/cliqueVersion/2dc.2048.txt.gz.txt.clq"

//random_attr_t rt(200, 200, .1, .9,10, 100, .1);

int main(){
	Logger::SetInformationLevel(LOGGER_INFO);
	PrecisionTimer pt;
	BatchCLQBk<ugraph, CliquePara<ugraph> > batch;
		
	//configure parameters for all tests
	clqo::param_t p;
	//p.tout=10800;											//seconds
	//p.tout=3600*6;
	p.nThreads=CliquePara<ugraph>::get_max_cores_hw()-1;	//all except one core
	p.unrolled=true;
	p.init_order=clqo::RLF_COND;									//default conditional ordering
				
	/*p.alg = BBMC;
	batch.add_test<CliquePara<ugraph>>(p);*/

	/*p.alg = BBMCX;
	batch.add_test<CliqueParaSat>(p);*/

	/*p.alg = BBMCXR_L;
	batch.add_test<CliqueParaSat>(p);*/

	p.alg = clqo::BBMCITXR_L;
	batch.add_test<CliqueParaIter>(p);
			
	/*p.alg = BBMCX;
	batch.add_test<CliqueSat>(p);

	p.alg = BBMCX_L;
	batch.add_test<CliqueSat>(p);

	p.alg = BBMCXR;
	batch.add_test<CliqueSat>(p);

	p.alg = BBMCXR_L;
	batch.add_test<CliqueSat>(p);
		
	p.alg = BBMCXR_L_SEQ;
	batch.add_test<CliqueSat>(p);*/

		
	BkClique bcl(BENCHMARK_PATH);
	bcl.Phat();		
	batch.benchmark_test(bcl, FILE_LOG(LOG, WRITE), 1);
	//batch.run_single_instance(SINGLE_INSTANCE_PATH, FILE_LOG(LOG, WRITE));
			  
	/*random_attr_t rt(200, 200, .9, .9, 10, 50, .5);
	batch.random_test(BENCHMARK_RANDOM_PATH,rt, FILE_LOG(LOG_RANDOM, APPEND));*/
	
	cout<<"----------------------------------------------";	
}

//int main(){
//	Logger::SetInformationLevel(LOGGER_INFO);
//	PrecisionTimer pt;
//	
//	//random_attr_t rt(10, 50, .1, .9, 100, 1, .1);
//	//random_attr_t rt(100, 150, .5, .91, 100, 50, .1);
//	random_attr_t rt(150, 200, .7, .91, 10, 50, .1);
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
//	//random_attr_t rt(200, 200, .9, .9, 10, 50, .5);
//
//	//random_attr_t rt(500, 500, .994, .994, 10, 50, .1);
//
//	//A batch object for tests with BBMC watched variants
//	BatchCLQGen<ugraph, CliquePara<ugraph> > batch;
//	
//	//configure parameters for all tests
//	param_t p;
//	p.nThreads=CliquePara<ugraph>::get_max_cores_hw()-1;	//all except one core
//	p.tout=3600;											//seconds
//	p.unrolled=true;
//	p.init_order=RLF_COND;
//
//	/*p.alg = BBMC;
//	batch.add_test<CliquePara<ugraph>>(p);*/
//
//	/*p.alg = BBMCX;
//	batch.add_test<CliqueParaSat>(p);*/
//
//	p.alg = BBMCXR_L;
//	batch.add_test<CliqueParaSat>(p);
//
//	p.alg = BBMCITXR_L;
//	batch.add_test<CliqueParaIter>(p);
//		
//	/*p.alg = BBMCX;
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
//	batch.add_test<CliqueSat>(p);*/
//
//		
//	/*BkClique bcl(BENCHMARK_PATH_POSIX);
//	bcl.SubsetEasyDimacs();		*/
//	//batch.benchmark_test(bcl, FILE_LOG(LOG_POSIX, WRITE), 1);
//	//batch.run_single_instance(SINGLE_INSTANCE_PATH, FILE_LOG(LOG, WRITE));
//			  
//	/*random_attr_t rt(200, 200, .9, .9, 10, 50, .5);*/
//	//batch.random_test(BENCHMARK_RANDOM_PATH,rt, FILE_LOG(LOG_RANDOM, APPEND));
//
//			
//			
//	//batch.find_property(WRITE_PATH, rt);
//	//batch.compare_two_tests(WRITE_PATH, rt,  DIFFERENT_CLIQUE_SIZE, FILE_LOG(LOG, WRITE));
//	batch.random_test(""/*STORE_INSTANCE_PATH*/, rt, FILE_LOG(LOG_RANDOM, APPEND));
//	
//	cout<<"----------------------------------------------";	
//
//}
