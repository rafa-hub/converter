//example of batch benchmark tests for one shot infrachromatic filter 
//author: pablo san segundo
//date: 2/8/2016

#include <iostream>
#include "../clique/clique_types.h"
#include "../batch/batch_benchmark.h"
#include "../batch/benchmark_color.h"
#include "../batch/batch_gen.h"
#include "../batch/benchmark_clique.h"
#include "utils/file.h"
#include "../clique/clique_sat.h"
#include "../clique/clique_infra.h"
#include "../clique/clique_russian_doll.h"


#define WRITE_PATH				     "C:/Users/i7/Desktop/"	
//#define SINGLE_INSTANCE_PATH		"C:/Users/pablo/Desktop/evil-tests/evil-N154-p98-myc11x14.clq"
//#define SINGLE_INSTANCE_PATH		"C:\\Users\\pablo\\Desktop\\2dc.2048.txt.gz.txt.clq"
//#define SINGLE_INSTANCE_PATH		"C:/Users/pablo/Desktop/CliqueVersion/1tc.512.txt.gz.txt.clq"
//#define SINGLE_INSTANCE_PATH		"C:/Users/pablo/Desktop/keller5_rlf.clq"
//#define SINGLE_INSTANCE_PATH		"C:/Users/pablo/Desktop/r200_0.95_4.txt"
//#define SINGLE_INSTANCE_PATH		"C:/Users/pablo/Desktop/c-fat200-1.clq"
//#define SINGLE_INSTANCE_PATH		"C:/Users/pablo/Desktop/QCP-15/normalized-qcp-15-120-3_ext.clq"
//#define SINGLE_INSTANCE_PATH		"C:/Users/pablo/Desktop/dimacs/sanr400_0.7.clq"
//#define SINGLE_INSTANCE_PATH		"C:/Users/pablo/Desktop/1zc.512.txt.gz.txt.clq"
//#define SINGLE_INSTANCE_PATH		"C:/Users/pablo/Desktop/dimacs/brock200_1.clq"
//#define SINGLE_INSTANCE_PATH		"C:/Users/pablo/Desktop/dimacs/johnson8-2-4.clq"
//#define SINGLE_INSTANCE_PATH		"C:/Users/pablo/Desktop/dimacs/brock200_1.clq"
#define SINGLE_INSTANCE_PATH		"C:/Users/i7/Desktop/conflict_actions_dimacs.txt"

//#define SINGLE_INSTANCE_PATH_POSIX	"/media/datos/graphs/frb_zavalnij/evil-myc-fbr11-k20.clq"
//#define SINGLE_INSTANCE_PATH_POSIX	"/media/datos/graphs/coding/cliqueVersion/1zc.512.txt.gz.txt.clq"


#ifdef TESTS_DIMACS
int main(){
	Logger::SetInformationLevel(LOGGER_INFO);
	PrecisionTimer pt;
	BatchCLQBk<ugraph, Clique<ugraph> > batch;
		
	//configure parameters for all tests
	clqo::param_t p;
	p.tout=3600;							
	p.unrolled=false;
	p.init_order=clqo::RLF_COND;
	p.init_preproc=clqo::UB_HEUR;
	//p.lb=21;	
			
	/*p.alg = clqo::BBMC;
	batch.add_test<Clique<ugraph>>(p);*/

	/*p.alg = clqo::BBMCR_COL_BB;
	batch.add_test<CliqueDollTest>(p);*/

	/*p.alg = clqo::BBMCX;
	batch.add_test<CliqueSat>(p);*/
	
	/*p.init_order=clqo::MIN_WIDTH_COMPOSITE;
	p.alg = clqo::BBMCR;
	batch.add_test<CliqueInfra>(p);*/
		
	p.alg = clqo::BBMCRL_KMIN;
	batch.add_test<CliqueInfra>(p);

	//p.alg = clqo::BBMCR_DOLL;
	//batch.add_test<CliqueDoll>(p);
	
	/*p.alg = clqo::BBMCR_KMIN_COMB;
	batch.add_test<CliqueInfra>(p);*/
	
	/*BkClique bcl(BENCHMARK_DIRECTORY_POSIX);
	bcl.Evil();		
	batch.benchmark_test(bcl, FILE_LOG(LOG_DIMACS_POSIX, APPEND), 1);*/

	/*BkClique bcl(BENCHMARK_PATH);
	bcl.Monotone();		
	batch.benchmark_test(bcl, FILE_LOG(LOG_DIMACS, WRITE), 1);*/

	BkColor bcl(BENCHMARK_PATH_POSIX);
	bcl.Dimacs();		
	batch.benchmark_test(bcl, FILE_LOG(LOG_DIMACS_POSIX, WRITE), 1);

	//batch.run_single_instance(SINGLE_INSTANCE_PATH, FILE_LOG(LOG_DIMACS, WRITE));
	//batch.run_single_instance(SINGLE_INSTANCE_PATH_POSIX, FILE_LOG(LOG_DIMACS_POSIX, APPEND));

	/*BkClique bcl1(BENCHMARK_PATH_POSIX);
	bcl1.SubsetEasyDimacs();		
	batch.benchmark_test(bcl1, FILE_LOG(LOG_DIMACS_POSIX, WRITE), 1);
			
	BkClique bcl2(BENCHMARK_PATH_POSIX);
	bcl2.SubsetHardDimacs();		
	batch.benchmark_test(bcl2, FILE_LOG(LOG_DIMACS_POSIX, APPEND), 1);*/
				  
	/*random_attr_t rt(200, 200, .9, .9, 10, 50, .5);
	batch.random_test(BENCHMARK_RANDOM_PATH,rt, FILE_LOG(LOG_RANDOM, APPEND))*/;
}

#else

int main(){
	Logger::SetInformationLevel(LOGGER_INFO);
	PrecisionTimer pt;
	
	//random_attr_t rt(10, 50, .1, .9, 100, 1, .1);
	//random_attr_t rt(100, 150, .5, .91, 100, 50, .1);
	//random_attr_t rt(150, 200, .7, .91, 10, 50, .1);
	//random_attr_t rt(300, 300, .6, .8, 10, 50, .1);
	// random_attr_t rt(500, 500, .994, .995, 10, 50, .1);
	//random_attr_t rt(1000, 1001, .3, .31, 10, 50, .11);
	//random_attr_t rt(3000, 3001, .1, .11, 10, 50, .1);
	//random_attr_t rt(5000, 5001, .1, .11, 10, 50, .1);
	//random_attr_t rt(10000, 10001, .1, .11, 10, 50, .1);
	//random_attr_t rt(15000, 15001, .1, .11, 10, 50, .1);
	//random_attr_t rt(20000, 30001, .1, .11, 10, 5000, .1);
	//random_attr_t rt(30000, 50001, .05, .06, 10, 10000, .1);
	//random_attr_t rt(100000, 100001, .05, .06, 1, 10000, .1);
	//random_attr_t rt(100000, 100001, .05, .06, 1, 25000, .1);

	//random_attr_t rt(3000, 5001, .1, .11, 10, 2000, .1);
	//random_attr_t rt(10000, 30001, .1, .11, 10, 5000, .1);
	  
	random_attr_t rt(200, 200, .95, .95, 10, 50, .05);
	//random_attr_t rt(200, 200, .90, .90, 10, 50, .05);
	//random_attr_t rt(300, 300, .7, .7, 10, 50, .05);
	//random_attr_t rt(500, 500, .6, .6, 10, 50, .05);
	//  random_attr_t rt(1000, 1000, .4, .4, 10, 50, .05);
	//random_attr_t rt(50, 100, .6, .9, 100, 10, .1);
	
	//random_attr_t rt(500, 500, .994, .994, 10, 50, .1);

	//A batch object for tests with BBMC watched variants
	BatchCLQGen<ugraph, Clique<ugraph> > batch;
	
	//configure parameters for all tests
	clqo::param_t p;
	p.tout=3600;			 //seconds
	p.unrolled=false;
	p.init_order=clqo::MIN_WIDTH_COMPOSITE;
	p.init_preproc=clqo::UB_HEUR;

	/*p.alg = clqo::BBMC;
	batch.add_test<Clique<ugraph>>(p);
	*/
	/*p.alg = clqo::BBMCX;
	batch.add_test<CliqueSat>(p);*/
	 		
	/*p.alg = clqo::BBMC_OS;
	batch.add_test<CliqueInfra>(p);*/

	//p.init_order=clqo::MIN_WIDTH_COMPOSITE;
	//p.alg = clqo::BBMCR;
	//batch.add_test<CliqueInfra>(p);

	p.alg = clqo::BBMCRL_KMIN;
	batch.add_test<CliqueInfra>(p);

	//p.alg = clqo::BBMCR_KMIN_COMB;
	//batch.add_test<CliqueInfra>(p);

	/*p.alg = clqo::BBMCRL_KMIN;
	batch.add_test<CliqueInfra>(p);*/
		
			
	//batch.find_property(WRITE_PATH, rt);
	//batch.compare_two_tests(WRITE_PATH, rt,  clqo::DIFFERENT_CLIQUE_SIZE, FILE_LOG(LOG_RANDOM, WRITE));
	batch.random_test(""/*STORE_INSTANCE_PATH*/, rt, FILE_LOG(LOG_RANDOM, WRITE));
	
}

#endif 





