//example of batch benchmark tests for new variants which used degree-based branching BBMCDeg
//author: pablo san segundo
//date: 4/9/2015

#include <iostream>
#include "../batch/batch_benchmark.h"
//#include "../batch/batch_gen.h"
#include "../batch/benchmark_clique.h"
#include "utils/file.h"
#include "../clique/clique_sat.h"
#include "../clique/clique_degsort.h"



#define BENCHMARK_PATH				"C:/Users/pablo/Desktop/dimacs"	
#define LOG							 "C:/Users/pablo/Desktop/kk_largedeg.txt"	
#define WRITE_PATH				     "C:\\Users\\i7\\Desktop/"	
#define SINGLE_INSTANCE_PATH		"C:/Users/pablo/Desktop/frb30-15-1_rlf.clq"

#define LOG_POSIX					 "/var/www/logs/bbmcx_mws_all.txt"
#define BENCHMARK_DIRECTORY_POSIX	 "/media/datos/graphs/dimacs_bhosh/"		

int main(){
	Logger::SetInformationLevel(LOGGER_INFO);
	PrecisionTimer pt;
	BatchCLQBk<ugraph, Clique<ugraph> > batch;
		
	//configure parameters for all tests
	clqo::param_t p;
	p.tout=10800;							//seconds
	p.unrolled=false;
	p.init_order=clqo::MIN_WIDTH_MIN_TIE_STATIC;
			
	/*p.alg = BBMC;
	batch.add_test<Clique<ugraph>>(p);*/

	/*p.alg = BBMCX;
	batch.add_test<CliqueSat>(p);

	p.alg = BBMCX_L;
	batch.add_test<CliqueSat>(p);

	p.alg = BBMCX_LN;
	batch.add_test<CliqueSat>(p);

	p.alg = BBMCXR;
	batch.add_test<CliqueSat>(p);*/
		
	/*p.alg = BBMCD;
	batch.add_test<CliqueDegSort>(p);*/

	//p.alg = BBMCD;
	//batch.add_test<CliqueDegSort>(p);
		
	//p.lb=27;
	batch.add_test<CliqueDegSort>(p);
	
	BkClique bcl(BENCHMARK_PATH);
	bcl.San();		
	batch.benchmark_test(bcl, FILE_LOG(LOG, WRITE), 1);
	//batch.run_single_instance(SINGLE_INSTANCE_PATH, cout);
}

//int main(){
//	Logger::SetInformationLevel(LOGGER_INFO);
//	PrecisionTimer pt;
//	
//	//random_attr_t rt(10, 50, .1, .9, 100, 1, .1);
//	//random_attr_t rt(100, 150, .5, .91, 100, 50, .1);
//	//random_attr_t rt(150, 200, .7, .91, 10, 50, .1);
//	random_attr_t rt(150, 200, .95, .95, 10, 50, .1);
//	//random_attr_t rt(500, 500, .994, .995, 10, 50, .1);
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
//
//	//A batch object for tests with BBMC watched variants
//	BatchCLQGen<ugraph, Clique<ugraph> > batch;
//	
//	//configure parameters for all tests
//	param_t p;
//	//p.tout=3600;			 //seconds
//	p.unrolled=false;
//	p.init_order=MIN_WIDTH_MIN_TIE_STATIC;
//
//   /* p.alg = BBMC;
//	batch.add_test<Clique<ugraph>>(p);*/
//
//		
//	p.alg = BBMCXR_LN;
//	batch.add_test<CliqueSat>(p);
//
//	p.alg = BBMCD_XR_LN;
//	batch.add_test<CliqueDegSort>(p);
//
//	
//	//batch.find_property(WRITE_PATH, rt);
//	//batch.compare_two_tests(WRITE_PATH, rt,  DIFFERENT_CLIQUE_SIZE, FILE_LOG(LOG, WRITE));
//	batch.random_test(""/*STORE_INSTANCE_PATH*/, rt, FILE_LOG(LOG, WRITE));
//	
//}









