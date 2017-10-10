//bbmc_infra_plus.cpp: example of batch benchmark tests for improved infrachrom
//author: pablo san segundo
//date: 15/12/2016

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
#include "../clique/clique_infra_plus.h"
#include "../clique/clique_russian_doll_plus.h"

#define WRITE_PATH				     "C:/Users/pablo/Desktop/"	
//#define SINGLE_INSTANCE_PATH		"C:/Users/pablo/Desktop/evil-tests/evil-N154-p98-myc11x14.clq"
//#define SINGLE_INSTANCE_PATH		"C:\\Users\\pablo\\Desktop\\2dc.2048.txt.gz.txt.clq"
//#define SINGLE_INSTANCE_PATH		"C:/Users/pablo/Desktop/CliqueVersion/1tc.512.txt.gz.txt.clq"
//#define SINGLE_INSTANCE_PATH		"C:/Users/pablo/Desktop/keller5_rlf.clq"
//#define SINGLE_INSTANCE_PATH		"C:/Users/pablo/Desktop/r200_0.95_4.txt"
//#define SINGLE_INSTANCE_PATH		"C:/Users/pablo/Desktop/c-fat200-1.clq"
//#define SINGLE_INSTANCE_PATH		"C:/Users/pablo/Desktop/dimacs/brock400_4.clq"
//#define SINGLE_INSTANCE_PATH		"C:/Users/pablo/Desktop/csp_all/normalized-qcp-10-67-10_ext.clq"
//#define SINGLE_INSTANCE_PATH		"C:/Users/pablo/Desktop/csp_all/normalized-bqwh-18-141-29_ext.clq"
//#define SINGLE_INSTANCE_PATH		"C:/Users/pablo/Desktop/r200_0.7_2.txt"
//#define SINGLE_INSTANCE_PATH		"C:/Users/pablo/Desktop/r43_0.7_75.txt"
//#define SINGLE_INSTANCE_PATH		"C:/Users/pablo/Desktop/r19_0.8_60.txt"
//#define SINGLE_INSTANCE_PATH		"C:/Users/pablo/Desktop/r19_0.8_60.txt"
#define SINGLE_INSTANCE_PATH		"C:/Users/pablo/Desktop/brock197.txt"
//#define SINGLE_INSTANCE_PATH		"C:/Users/pablo/Desktop/bigGraphs/sc-nasasrb.mtx.clq"
//#define SINGLE_INSTANCE_PATH		"C:/Users/pablo/Desktop/bigGraphs/ia-enron-large.mtx.clq"
//#define SINGLE_INSTANCE_PATH		"C:/Users/pablo/Desktop/bigGraphs/ia-email-EU.mtx.clq"
//#define SINGLE_INSTANCE_PATH		"C:/Users/pablo/Desktop/bigGraphs/wing.mtx.clq"
//#define SINGLE_INSTANCE_PATH		"C:/Users/pablo/Desktop/bigGraphs/fe-tooth.mtx.clq"
//#define SINGLE_INSTANCE_PATH		"C:/Users/pablo/Desktop/bigGraphs/fe-body.mtx.clq"
//#define SINGLE_INSTANCE_PATH		"C:/Users/pablo/Desktop/bigGraphs/fe_rotor.mtx.clq"
//#define SINGLE_INSTANCE_PATH		"C:/Users/pablo/Desktop/bigGraphs/delaunay_n16.mtx.clq"
//#define SINGLE_INSTANCE_PATH		"C:/Users/pablo/Desktop/bigGraphs/cond-mat-2003.clq"
//#define SINGLE_INSTANCE_PATH		"C:/Users/pablo/Desktop/bigGraphs/cond-mat-2005.clq"
//#define SINGLE_INSTANCE_PATH		"C:/Users/pablo/Desktop/caidaRouterLevel.graph.dimacs"
//#define SINGLE_INSTANCE_PATH		"C:/Users/pablo/Desktop/bigGraphs/rgg_n_2_15_s0.clq"
//#define SINGLE_INSTANCE_PATH		"C:/Users/pablo/Desktop/bigGraphs/rgg_n_2_16_s0.clq"
//#define SINGLE_INSTANCE_PATH		"C:/Users/pablo/Desktop/dimacs/brock200_1.clq"
//#define SINGLE_INSTANCE_PATH		"C:/Users/i7/Desktop/grafo_5_8.clq"
//#define SINGLE_INSTANCE_PATH		"C:/Users/pablo/Desktop/csp_all/frb59-26-1.clq"
//#define SINGLE_INSTANCE_PATH		"C:/Users/pablo/Desktop/csp_all/normalized-ehi-85-297-19_ext.clq"
//#define SINGLE_INSTANCE_PATH		"C:/Users/pablo/Desktop/csp_all/normalized-le-450-5a-2-ext.clq"
//#define SINGLE_INSTANCE_PATH		"C:/Users/pablo/Desktop/csp_all/normalized-qcp-20-187-0_ext.clq"
//#define SINGLE_INSTANCE_PATH		"C:/Users/pablo/Desktop/csp_all/normalized-langford-3-11-ext.clq"
//#define SINGLE_INSTANCE_PATH		"C:/Users/pablo/Desktop/csp_all/normalized-rand-2-40-25-180-500-13_ext.clq"
//#define SINGLE_INSTANCE_PATH		"C:/Users/pablo/Desktop/csp_all/normalized-rand-2-40-40-135-650-10_ext.clq"
//#define SINGLE_INSTANCE_PATH		"C:/Users/pablo/Desktop/csp_all/normalized-rand-2-40-40-135-650-11_ext.clq"
//#define SINGLE_INSTANCE_PATH		"C:/Users/pablo/Desktop/csp_all/normalized-rand-2-40-40-135-650-25_ext.clq"
//#define SINGLE_INSTANCE_PATH		"C:/Users/pablo/Desktop/csp_all/normalized-rand-2-40-40-135-650-29_ext.clq"
//#define SINGLE_INSTANCE_PATH		"C:/Users/pablo/Desktop/csp_all/normalized-BlackHole-4-4-e-3_ext.clq"
//#define SINGLE_INSTANCE_PATH		"C:/Users/pablo/Desktop/csp_all/normalized-ehi-85-297-19_ext.clq"
//#define SINGLE_INSTANCE_PATH		"C:/Users/pablo/Desktop/dimacs/sanr400_0.7.clq"
//#define SINGLE_INSTANCE_PATH		"C:/Users/pablo/Desktop/1zc.512.txt.gz.txt.clq"
//#define SINGLE_INSTANCE_PATH		"C:/Users/pablo/Desktop/dimacs/brock200_1.clq"
//#define SINGLE_INSTANCE_PATH		"C:/Users/pablo/Desktop/dimacs/johnson8-2-4.clq"
//#define SINGLE_INSTANCE_PATH		"C:/Users/pablo/Desktop/dimacs/brock200_1.clq"
//#define SINGLE_INSTANCE_PATH		"C:/Users/pablo/Desktop/johnson-11-4-3.clq"

//#define SINGLE_INSTANCE_PATH_POSIX	"/media/datos/graphs/frb_zavalnij/evil-myc-fbr11-k20.clq"
//#define SINGLE_INSTANCE_PATH_POSIX	"/media/datos/graphs/coding/cliqueVersion/1zc.512.txt.gz.txt.clq"


#ifdef TESTS_DIMACS
int main(){
	Logger::SetInformationLevel(LOGGER_INFO);
	PrecisionTimer pt;
	BatchCLQBk<ugraph, Clique<ugraph> > batch;
		
	//configure parameters for all tests
	clqo::param_t p;
	p.tout=3600*24;										//6h			
	p.unrolled=false;
	//p.init_order=clqo::RLF_SORT_DOLL;
	//p.init_order=clqo::RLF_COND;
	p.init_order=clqo::MIN_WIDTH;
	//p.init_order=clqo::NONE;
	//p.init_preproc=clqo::UB_HEUR;
	p.init_preproc=clqo::UB;
	//p.lb=95;
	
	/*p.alg = clqo::BBMC;
	batch.add_test<Clique<ugraph>>(p);*/

	/*p.alg = clqo::BBMCR_COL_BB;
	batch.add_test<CliqueDollTest>(p);*/

	/*p.alg = clqo::BBMCX;
	batch.add_test<CliqueSat>(p);
	*/
	/*p.init_order=clqo::MIN_WIDTH_COMPOSITE;
	p.alg = clqo::BBMCR;
	batch.add_test<CliqueInfra>(p);*/

	/*p.alg = clqo::BBMCR_DOLL;
	batch.add_test<CliqueDollPlus>(p);*/
			
	p.alg = clqo::BBMCL;
	batch.add_test<CliqueInfraPlus>(p);

	/*p.alg = clqo::BBMCL_R;
	batch.add_test<CliqueInfraPlus>(p);

	p.alg = clqo::BBMCL_PLUS;
	batch.add_test<CliqueInfraPlus>(p);*/

	/*p.alg = clqo::BBMCR_DOLL;
	batch.add_test<CliqueDoll>(p);*/
	
	/*p.alg = clqo::BBMCR_KMIN_COMB;
	batch.add_test<CliqueInfra>(p);*/
	
	/*BkClique bcl(BENCHMARK_PATH_POSIX);
	bcl.Keller();		
	batch.benchmark_test(bcl, FILE_LOG(LOG_DIMACS_POSIX, WRITE), 1);*/

	/*BkClique bcl(BENCHMARK_PATH);
	bcl.vcp() ;
	batch.complement_benchmark_test(bcl, FILE_LOG(LOG_DIMACS, APPEND), 1);	*/

	/*BkClique bcl(BENCHMARK_PATH_POSIX);
	bcl.vcp() ;
	batch.complement_benchmark_test(bcl, FILE_LOG(LOG_DIMACS_POSIX, WRITE), 1);	*/
	
	/*BkClique bcl(BENCHMARK_PATH);
	bcl.SubsetEasyDimacs();	
	batch.benchmark_test(bcl, FILE_LOG(LOG_DIMACS, WRITE), 1);*/
	batch.run_single_instance(SINGLE_INSTANCE_PATH, FILE_LOG(LOG_DIMACS, WRITE));

	//BkClique bcl(BENCHMARK_PATH);
	//bcl.SubsetEasyDimacs();		
	//batch.benchmark_test(bcl, FILE_LOG(LOG_DIMACS, APPEND), 1);


	/*BkClique bcl(BENCHMARK_PATH);
	bcl.SubsetHardDimacs();		
	batch.benchmark_test(bcl, FILE_LOG(LOG_DIMACS, APPEND), 1);*/

	/*BkColor bcl(BENCHMARK_PATH_POSIX);
	bcl.Dimacs();		
	batch.benchmark_test(bcl, FILE_LOG(LOG_DIMACS_POSIX, WRITE), 1);*/
	//batch.run_single_instance(SINGLE_INSTANCE_PATH_POSIX, FILE_LOG(LOG_DIMACS_POSIX, APPEND));

	/*BkClique bcl1(BENCHMARK_PATH_POSIX);
	bcl1.SubsetEasyDimacs();		
	batch.benchmark_test(bcl1, FILE_LOG(LOG_DIMACS_POSIX, APPEND), 1);

	BkClique bcl2(BENCHMARK_PATH_POSIX);
	bcl2.SubsetHardDimacs();		
	batch.benchmark_test(bcl2, FILE_LOG(LOG_DIMACS_POSIX, APPEND), 1);*/

	/*BkClique bcl2(BENCHMARK_PATH_POSIX);
	bcl2.Others();		
	batch.benchmark_test(bcl2, FILE_LOG(LOG_DIMACS_POSIX, APPEND), 1);*/
			
	/*BkClique bcl2(BENCHMARK_PATH_POSIX);
	bcl2.SubsetHardDimacs();		
	batch.benchmark_test(bcl2, FILE_LOG(LOG_DIMACS_POSIX, APPEND), 1);*/
				  
	/*random_attr_t rt(200, 200, .9, .9, 10, 50, .5);
	batch.random_test(BENCHMARK_RANDOM_PATH,rt, FILE_LOG(LOG_RANDOM, APPEND))*/;
}

#else

int main(){
	Logger::SetInformationLevel(LOGGER_INFO);
	PrecisionTimer pt;
	
	random_attr_t rt(10, 50, .1, .9, 100, 1, .1);
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
	  
	
	//random_attr_t rt(200, 200, .95, .95, 10, 50, .05);
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
	p.tout=3600;											//seconds
	p.unrolled=false;
	p.init_order=clqo::MIN_WIDTH;
	p.init_preproc=clqo::UB;

	p.alg = clqo::BBMC;
	batch.add_test<Clique<ugraph>>(p);
	
	/*p.alg = clqo::BBMCX;
	batch.add_test<CliqueSat>(p);*/
	 		
	/*p.alg = clqo::BBMC_OS;
	batch.add_test<CliqueInfra>(p);*/

	//p.init_order=clqo::MIN_WIDTH_COMPOSITE;
	//p.alg = clqo::BBMCR;
	//batch.add_test<CliqueInfra>(p);

	//p.alg = clqo::BBMCL_PLUS;
	//batch.add_test<CliqueInfraPlus>(p);

	p.alg = clqo::BBMCL;
	batch.add_test<CliqueInfraPlus>(p);
	

	/*p.alg = clqo::BBMCL_R;
	batch.add_test<CliqueInfraPlus>(p);*/

	//p.alg = clqo::BBMCR_KMIN_COMB;
	//batch.add_test<CliqueInfra>(p);

	/*p.alg = clqo::BBMCRL_KMIN;
	batch.add_test<CliqueInfra>(p);*/
		
			
	//batch.find_property(WRITE_PATH, rt);
	batch.compare_two_tests(WRITE_PATH, rt,  clqo::DIFFERENT_CLIQUE_SIZE, FILE_LOG(LOG_RANDOM, WRITE));
	//batch.compare_two_tests(WRITE_PATH, rt,  clqo::DIFFERENT_STEPS_FIRST_GREATER, FILE_LOG(LOG_RANDOM, WRITE));
	//batch.random_test(""/*STORE_INSTANCE_PATH*/, rt, FILE_LOG(LOG_RANDOM, WRITE));
	
}

#endif 





