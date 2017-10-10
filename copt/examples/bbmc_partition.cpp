//bbmc_partition_clique.cpp: example of batch benchmark tests for finding a good clique partition of vertices
//							by iteratively computing and removing a partial partition of maximum cliques
//author: pss
//date: 22/11/2016

#include <iostream>
#include "../clique/clique_types.h"
#include "../batch/batch_benchmark.h"
#include "../batch/batch_gen.h"
#include "../batch/benchmark_clique.h"
#include "../batch/benchmark_color.h"
#include "utils/file.h"
#include "../clique/clique_russian_doll.h"
#include "../clique/clique_iter.h"
#include "../clique/clique_sat.h"
#include "../clique/clique_infra.h"
#include "../clique/clique_weighted.h"
#include "../interfaces/interface_partition.h"

#define WRITE_PATH				     "C:/Users/pablo/Desktop/test/"	

//#define SINGLE_INSTANCE_PATH		"C:/Users/pablo/Desktop/frb30-15-1_rlf.clq"
//#define SINGLE_INSTANCE_PATH		"C:/Users/pablo/Desktop/gen400_p0.9_55_rlf.clq"
//#define SINGLE_INSTANCE_PATH		"C:/Users/pablo/Desktop/dsjc500.9_rlf.clq"
//#define SINGLE_INSTANCE_PATH		"C:/Users/pablo/Desktop/r10_0.4_25.txt"
//#define SINGLE_INSTANCE_PATH		"C:/Users/pablo/Desktop/frb40-19/normalized-frb40-19-5_ext.clq"
//#define SINGLE_INSTANCE_PATH		"C:/Users/i7/Desktop/normalized-qwh-15-106-0_ext.clq"
//#define SINGLE_INSTANCE_PATH		"C:/Users/pablo/Desktop/weighted/DSJC125_5g.col"
//#define SINGLE_INSTANCE_PATH		"C:/Users/pablo/Desktop/weighted/DSJC125_9g.col"
//#define SINGLE_INSTANCE_PATH		"C:/Users/pablo/Desktop/weighted/DSJC125_1gb.col"
//#define SINGLE_INSTANCE_PATH		"C:/Users/i7/Desktop/conflict_robot_problem_example.txt"
#define SINGLE_INSTANCE_PATH		 "C:/Users/pablo/Desktop/csp_all/normalized-BlackHole-4-4-e-4_ext.clq"
//#define SINGLE_INSTANCE_PATH		"C:/Users/pablo/Desktop/weighted/p09.col"
//#define SINGLE_INSTANCE_PATH		"C:/Users/pablo/Desktop/weighted/GEOM90.col"
//#define SINGLE_INSTANCE_PATH		"C:/Users/pablo/Desktop/weighted/R100_9gb.col"
//#define SINGLE_INSTANCE_PATH		"C:/Users/pablo/Desktop/dimacs/san400_0.7_1.clq"
//#define SINGLE_INSTANCE_PATH		"C:/Users/pablo/Desktop/coding/1dc.1024.txt.gz.txt.clq"
//#define SINGLE_INSTANCE_PATH		"C:/Users/pablo/Desktop/vcp/4-FullIns_3.col.dimacs.dim"
//#define SINGLE_INSTANCE_PATH		"C:/Users/pablo/Desktop/vcp/mug88_25.col.dimacs.dim"
//#define SINGLE_INSTANCE_PATH		"C:/Users/pablo/Desktop/dimacs/san200_0.9_2.clq"
//#define SINGLE_INSTANCE_PATH		"C:/Users/pablo/Desktop/dimacs/brock400_1.clq"
//#define SINGLE_INSTANCE_PATH		"C:/Users/pablo/Desktop/dimacs/MANN_a27.clq"

//#define SINGLE_INSTANCE_PATH_POSIX	"/media/datos/graphs/frb_zavalnij/evil-myc-fbr11-k20.clq"

#ifdef TESTS_DIMACS
int main(){
	Logger::SetInformationLevel(LOGGER_INFO);
	PrecisionTimer pt;
	BatchCLQBk<ugraph, Partition> batch;
		
	//configure parameters for all tests
	clqo::param_t p;
	p.tout=1800;			 //seconds
	p.unrolled=false;
	p.init_order=clqo::MIN_WIDTH;	
//	p.init_preproc=clqo::UB_HEUR;
	p.init_preproc=clqo::UB;
	//p.lb=50000;
						
	/*p.alg =clqo::BBMC;
	batch.add_test<Clique<ugraph> >(p);*/
	
	/*p.alg = clqo::BBMC_WEIGHTED;
	batch.add_test<CliqueWeighted>(p);*/

	/*p.alg = clqo::BBMCR_WEIGHTED;
	batch.add_test<CliqueWeighted>(p);*/

	/*p.alg = clqo::BBMCX_WEIGHTED;
	batch.add_test<CliqueWeighted>(p);*/

	p.alg = clqo::BBMCXR_L;
	batch.add_test<Partition>(p);
							
	/*BkCliqueW bclw(BENCHMARK_PATH);
	bclw.EasyWeighted() ;
	batch.benchmark_test(bclw, FILE_LOG(LOG_DIMACS, WRITE), 1);*/
		
	/*BkColor bcol(BENCHMARK_PATH);
	bcol.Flat();
	batch.benchmark_test(bcol, FILE_LOG(LOG_DIMACS, WRITE), 1);*/
	//batch.run_single_instance(SINGLE_INSTANCE_PATH, FILE_LOG(LOG_DIMACS, WRITE));

	//CSP tests: uses complement
	BkClique bcl(BENCHMARK_PATH);
	bcl.black_hole();
	batch.complement_benchmark_test(bcl,FILE_LOG(LOG_DIMACS, WRITE),1);

	/*BkColor bcol(BENCHMARK_PATH_POSIX);
	bcol.Queen();
	batch.benchmark_test(bcol, FILE_LOG(LOG_DIMACS_POSIX, APPEND), 1);*/

	/*BkClique bcl(BENCHMARK_PATH_POSIX);
	bcl.SubsetEasyDimacs();
	batch.benchmark_test(bcl, FILE_LOG(LOG_DIMACS_POSIX, APPEND), 1);*/

	//batch.run_single_instance(SINGLE_INSTANCE_PATH, FILE_LOG(LOG_DIMACS, WRITE));	
					  
	//random_attr_t rt(150, 200, .7, .91, 10, 50, .1);
	////random_attr_t rt(150, 150, .95, .98, 10, 50, .03);
	//batch.random_test(BENCHMARK_RANDOM_PATH,rt, FILE_LOG(LOG_RANDOM, WRITE));

	//random_attr_t rt(150, 200, .7, .91, 10, 50, .1);
	////random_attr_t rt(150, 150, .95, .98, 10, 50, .03);
	//batch.random_test(BENCHMARK_RANDOM_PATH_POSIX,rt, FILE_LOG(LOG_RANDOM_POSIX, WRITE));
}
#else
int main(){
	Logger::SetInformationLevel(LOGGER_INFO);
	PrecisionTimer pt;
	
	//random_attr_t rt(10, 50, .1, .9, 100, 1, .1);
	//random_attr_t rt(100, 150, .4, .91, 100, 50, .1);
	//random_attr_t rt(150, 200, .7, .91, 10, 50, .1);
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
	  
	//random_attr_t rt(200, 200, .90, .90, 10, 50, .1

	random_attr_t rt(100, 100, .2, .21, 100, 1, .1);

	//A batch object for tests with BBMC watched variants
	BatchCLQGen<ugraph, Clique<ugraph> > batch;
	
	//configure parameters for all tests
	clqo::param_t p;
	//p.tout=3600;			 //seconds
	p.unrolled=false;
	//p.init_order=clqo::RLF_COND;
	//p.init_order=clqo::MIN_WEIGHTED;
	p.init_preproc=clqo::UB_HEUR;

	p.alg = clqo::BBMC;
	batch.add_test<Clique<ugraph> >(p);

	/*p.alg = clqo::BBMCX;
	batch.add_test<CliqueSat>(p);*/
		
	/*p.alg = clqo::BBMC_COL;
	batch.add_test<CliqueDollTest>(p);*/
	
	/*p.alg = clqo::BBMC_WEIGHTED;
	batch.add_test<CliqueWeighted>(p);*/
			
	//batch.find_property(WRITE_PATH, rt);
	//batch.compare_two_tests(WRITE_PATH, rt,  clqo::DIFFERENT_CLIQUE_SIZE, FILE_LOG(LOG_RANDOM, WRITE));
	batch.random_test(/*""*/WRITE_PATH, rt, FILE_LOG(LOG_RANDOM, WRITE));
}

#endif








