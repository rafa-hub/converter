//example of batch benchmark tests for new iterative variant BBMC_dolls_tests which tests different russian doll variants (*experimental*)
//author: pablo san segundo
//date: 27/7/2016

#include <iostream>
#include "../clique/clique_types.h"
#include "../batch/batch_benchmark.h"
#include "../batch/batch_gen.h"
#include "../batch/benchmark_clique.h"
#include "utils/file.h"
#include "../clique/clique_russian_doll.h"
#include "../clique/clique_russian_doll_plus.h"
#include "../clique/clique_iter.h"
#include "../clique/clique_sat.h"
#include "../clique/clique_infra.h"


#define WRITE_PATH				     "C:/Users/pablo/Desktop/test/"	
	

//#define SINGLE_INSTANCE_PATH		"C:/Users/pablo/Desktop/frb30-15-1_rlf.clq"
//#define SINGLE_INSTANCE_PATH		"C:/Users/pablo/Desktop/gen400_p0.9_55_rlf.clq"
//#define SINGLE_INSTANCE_PATH		"C:/Users/pablo/Desktop/dsjc500.9_rlf.clq"
//#define SINGLE_INSTANCE_PATH		"C:/Users/pablo/Desktop/r10_0.4_25.txt"
//#define SINGLE_INSTANCE_PATH		"C:/Users/pablo/Desktop/frb40-19/normalized-frb40-19-5_ext.clq"
//#define SINGLE_INSTANCE_PATH		"C:/Users/i7/Desktop/normalized-qwh-15-106-0_ext.clq"
//#define SINGLE_INSTANCE_PATH		"C:/Users/pablo/Desktop/myciel6.col.dimacs.dim"
//#define SINGLE_INSTANCE_PATH		"C:/Users/i7/Desktop/test/r10_0.7_99.txt"
//#define SINGLE_INSTANCE_PATH		"C:/Users/pablo/Desktop/QCP-15/normalized-qcp-15-120-4_ext.clq"
//#define SINGLE_INSTANCE_PATH		"C:/Users/pablo/Desktop/GRAPHS/1-FullIns_4.col.dimacs.dim"
//#define SINGLE_INSTANCE_PATH		"C:/Users/pablo/Desktop/coding/1dc.1024.txt.gz.txt.clq"
#define SINGLE_INSTANCE_PATH		"C:/Users/pablo/Desktop/vcp/4-FullIns_3.col.dimacs.dim"
//#define SINGLE_INSTANCE_PATH		"C:/Users/pablo/Desktop/vcp/mug88_25.col.dimacs.dim"

//#define SINGLE_INSTANCE_PATH_POSIX	"/media/datos/graphs/frb_zavalnij/evil-myc-fbr11-k20.clq"
#define SINGLE_INSTANCE_PATH_POSIX		"/media/datos/graphs/dimacs_bhosh/brock400_1.clq"

#ifdef TESTS_DIMACS
int main(){
	Logger::SetInformationLevel(LOGGER_INFO);
	PrecisionTimer pt;
	BatchCLQBk<ugraph, Clique<ugraph> > batch;
		
	//configure parameters for all tests
	clqo::param_t p;
	p.tout=3600*24;								//seconds
	p.unrolled=false;
	p.init_order=clqo::RLF_COND;	
	//p.init_order=clqo::MIN_WIDTH;	
	p.init_preproc=clqo::UB_HEUR;

						
	/*p.alg =clqo::BBMC;
	batch.add_test<Clique<ugraph> >(p);*/

	/*p.alg =clqo::BBMCX;
	batch.add_test<CliqueSat>(p);*/

	/*p.alg = clqo::BBMCITX;
	batch.add_test<CliqueIter>(p);*/

	/*p.alg = clqo::BBMCR;
	batch.add_test<CliqueInfra>(p);*/
				
	//p.alg = clqo::BBMC_DOLL_LISTS;
	//batch.add_test<CliqueDollTest>(p);

	//p.alg = clqo::BBMCR_DOLL_LISTS;	
	//batch.add_test<CliqueDollTest>(p);
	
	/*p.alg = clqo::BBMC_DOLL_LISTS;
	batch.add_test<CliqueDoll>(p);*/

	/*p.alg = clqo::BBMCR_DOLL_LISTS;
	batch.add_test<CliqueDoll>(p);*/

	/*p.alg = clqo::BBMCRL_KMIN;
	batch.add_test<CliqueInfra>(p);*/

	//p.alg = clqo::BBMCR_DOLL ;
	//batch.add_test<CliqueDoll>(p);

	p.alg = clqo::BBMCR_DOLL;
	batch.add_test<CliqueDollPlus>(p);
	
	/*p.alg = clqo::BBMC_DOLL;
	batch.add_test<CliqueDoll>(p);*/

	/*p.alg = clqo::BBMCR_DOLL;
	batch.add_test<CliqueDoll>(p);*/

	/*BkClique bcl(BENCHMARK_PATH_POSIX);
	bcl.Frb(); 
	batch.benchmark_test(bcl, FILE_LOG(LOG_DIMACS_POSIX, WRITE), 1);*/	

	/*BkClique bcl(BENCHMARK_PATH);
	bcl.Keller();
	batch.benchmark_test(bcl, FILE_LOG(LOG_DIMACS, WRITE), 1);	*/
					
	//BkClique bcl(BENCHMARK_PATH);
	//bcl.vcp() ;
	//batch.complement_benchmark_test(bcl, FILE_LOG(LOG_DIMACS, APPEND), 1);	

	batch.run_single_instance(SINGLE_INSTANCE_PATH_POSIX, FILE_LOG(LOG_DIMACS_POSIX, WRITE));


	/*BkClique bcl2(BENCHMARK_PATH_POSIX);
	bcl2.Monotone();		
	batch.benchmark_test(bcl2, FILE_LOG(LOG_DIMACS_POSIX, APPEND), 1);*/
	
	/*BkClique bcl(BENCHMARK_PATH);
	bcl.vcp();		
	batch.complement_benchmark_test(bcl, FILE_LOG(LOG_DIMACS, WRITE), 1);*/
	
	//batch.complement_benchmark_test(SINGLE_INSTANCE_PATH, cout);	

	//batch.run_single_instance(SINGLE_INSTANCE_PATH,FILE_LOG(LOG_DIMACS, WRITE));
					  
	/*random_attr_t rt(200, 200, .9, .9, 10, 50, .5);
	batch.random_test(BENCHMARK_RANDOM_PATH,rt, FILE_LOG(LOG_RANDOM, APPEND));*/
}
#else
int main(){
	Logger::SetInformationLevel(LOGGER_INFO);
	PrecisionTimer pt;
	
	//random_attr_t rt(10, 50, .1, .9, 100, 1, .1);
	//random_attr_t rt(48, 50, .1, .9, 100, 1, .1);
	//random_attr_t rt(100, 150, .4, .91, 100, 50, .1);
	random_attr_t rt(150, 200, .7, .91, 10, 50, .1);
	//random_attr_t rt(200, 200, .95, .99, 10, 50, .03);
	//random_attr_t rt(500, 500, .994, .995, 10, 50, .1);
	// random_attr_t rt(1000, 1000, .998, .998, 10, 50, .1);
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
	  
	//random_attr_t rt(200, 200, .90, .90, 10, 50, .1);

	//A batch object for tests with BBMC watched variants
	BatchCLQGen<ugraph, Clique<ugraph> > batch;
	
	//configure parameters for all tests
	clqo::param_t p;
	//p.tout=3600;			 //seconds
	p.unrolled=false;
	//p.init_order=clqo::RLF_COND;
	p.init_order=clqo::RLF_COND;
	//p.init_order = clqo::MIN_WIDTH;
	p.init_preproc = clqo::UB_HEUR;

	p.alg = clqo::BBMC;
	batch.add_test<Clique<ugraph> >(p);

	/*p.alg = clqo::BBMCR_DOLL;
	batch.add_test<CliqueDoll>(p);*/

	p.alg = clqo::BBMCR_DOLL;
	batch.add_test<CliqueDollPlus>(p);

	/*p.alg = clqo::BBMCR;
	batch.add_test<CliqueDollPlus>(p);*/

	/*p.alg = clqo::BBMCX;
	batch.add_test<CliqueSat>(p);*/
		
	/*p.alg = clqo::BBMC_COL;
	batch.add_test<CliqueDollTest>(p);*/
	
	/*p.alg = clqo::BBMCR_DOLL;
	batch.add_test<CliqueDoll>(p);*/
	
	/*p.alg = clqo::BBMC_DOLL_LISTS;
	batch.add_test<CliqueDoll>(p);*/

	/*p.alg = clqo::BBMCR_DOLL_LISTS;
	batch.add_test<CliqueDoll>(p);*/

	/*p.alg = clqo::BBMCR;
	batch.add_test<CliqueInfra>(p);*/

	/*p.alg = clqo::BBMCR_COL;
	batch.add_test<CliqueDollTest>(p);*/
			
	//batch.find_property(WRITE_PATH, rt);
	//batch.compare_two_tests(WRITE_PATH, rt,  clqo::DIFFERENT_CLIQUE_SIZE, FILE_LOG(LOG_RANDOM, WRITE));
	//batch.compare_two_tests(WRITE_PATH, rt,  clqo::DIFFERENT_STEPS, FILE_LOG(LOG_RANDOM, WRITE));
		
	batch.random_test(""/*WRITE_PATH*/, rt, FILE_LOG(LOG_RANDOM, WRITE));
}

#endif








