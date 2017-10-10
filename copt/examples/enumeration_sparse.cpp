//enumeration_sparse.cpp: binary for maximal clique enumeration
//author: pablo san segundo
//date of creation: 19/12/2015

#include <iostream>
#include "../batch/batch_benchmark.h"
#include "../batch/batch_gen.h"
#include "../batch/benchmark_clique.h"
#include "utils/file.h"
#include "../clique/clique_enum.h"
#include "../clique/clique_enum_sparse.h"


#define BENCHMARK_PATH				"C:/Users/pablo/Desktop/dimacs"	
#define BENCHMARK_RANDOM_PATH		"C:/Users/pablo/Desktop/random"	
#define LOG							 "C:/Users/pablo/Desktop/dimacs_enum.txt"	
#define LOG_RANDOM					 "C:/Users/pablo/Desktop/rnd_enum.txt"	
#define WRITE_PATH				     "C:\\Users\\pablo\\Desktop/"	

//#define SINGLE_INSTANCE_PATH		"C:/Users/Pablo/Desktop/dimacs/johnson8-4-4.clq"
//#define SINGLE_INSTANCE_PATH		"C:/Users/pablo/Desktop/dimacs/johnson16-2-4.clq"
//#define SINGLE_INSTANCE_PATH		"C:/Users/Pablo/Desktop/dimacs/p_hat300-2.clq"
//#define SINGLE_INSTANCE_PATH		"C:/Users/pablo/Desktop/dimacs/hamming6-2.clq"
//#define SINGLE_INSTANCE_PATH		"C:/Users/pablo/Desktop/dimacs/hamming6-4.clq"
//#define SINGLE_INSTANCE_PATH		"C:/Users/pablo/Desktop/dimacs/c-fat200-5.clq"
//#define SINGLE_INSTANCE_PATH		"C:/Users/pablo/Desktop/dimacs/c-fat500-10.clq"
//#define SINGLE_INSTANCE_PATH		"C:/Users/pablo/Desktop/dimacs/MANN_a9.clq"
#define SINGLE_INSTANCE_PATH		"C:/Users/Pablo/Desktop/dimacs/brock200_2.clq"
//#define SINGLE_INSTANCE_PATH		"C:/Users/pablo/Desktop/dimacs/keller4.clq"
//#define SINGLE_INSTANCE_PATH		"C:/Users/pablo/Desktop/graph.txt"


#define LOG_POSIX						 "/var/www/logs/1dc.2048.txt"
#define LOG_REAL_POSIX					 "/var/www/logs/real_graph_300_enum_tests.txt"
//#define BENCHMARK_DIRECTORY_POSIX		 "/media/datos/graphs/dimacs_bhosh/"
//#define BENCHMARK_REAL_DIRECTORY_POSIX	 "/media/datos/graphs/large_sparse/benchmark/dimacs"
#define BENCHMARK_REAL_DIRECTORY_POSIX	 "/media/datos/graphs/large_sparse/benchmark"
//#define SINGLE_INSTANCE_PATH_POSIX		 "/media/datos/graphs/coding/cliqueVersion/1dc.2048.txt.gz.txt.clq"
//#define SINGLE_INSTANCE_PATH_POSIX "/media/datos/graphs/large_sparse/benchmark/auto.mtx"
//#define BENCHMARK_REAL_DIRECTORY_POSIX	 "/media/datos/graphs/large_sparse/benchmark"
#define SINGLE_INSTANCE_PATH_POSIX		 "/media/datos/graphs/coding/cliqueVersion/1dc.2048.txt.gz.txt.clq"


////////////////////////////
//SPARSE ENUMERATION

int main(){
	Logger::SetInformationLevel(LOGGER_INFO);
	PrecisionTimer pt;
	BatchCLQBk<sparse_ugraph, Clique<sparse_ugraph> > batch;
			
	//configure parameters for all tests
	clqo::param_t p;
	p.unrolled=true;
	p.tout=900;										//15min
	//p.init_order=gbbs::KCORE_UB;
	//p.init_order=clqo::MAX_WIDTH;					//default conditional ordering
	
	/*p.alg = clqo::BBMC_EN_GCAND;
	batch.add_test<CliqueEnumSparse>(p);*/

	/*p.alg = clqo::BBMC_EN_GCONF;
	batch.add_test<CliqueEnumSparse>(p);*/

	/*p.alg = clqo::BBMC_EN_GCONF_GRAPH;
	batch.add_test<CliqueEnumSparse>(p);*/

	/*p.alg = clqo::BBMC_EN_GCONF_GRAPH_X;
	batch.add_test<CliqueEnumSparse>(p);*/

	p.alg = clqo::BBMC_EN_GCONF_GRAPH_XP;
	batch.add_test<CliqueEnumSparse>(p);

	/*p.alg = clqo::BBMC_EN_GCONF_X;
	batch.add_test<CliqueEnumSparse>(p);

	p.alg = clqo::BBMC_EN_GCONF_XP;
	batch.add_test<CliqueEnumSparse>(p);*/

	/*p.alg = clqo::BBMC_EN_GCONF_HEUR;
	batch.add_test<CliqueEnumSparse>(p);*/

	//p.alg = clqo::BBMC_EN_GCAND;
	//batch.add_test<CliqueEnumSparse>(p);

	//p.alg = clqo::BBMC_EN_GCONF_INIT;
	//batch.add_test<CliqueEnumSparse>(p);

	//p.unrolled=false;
	///*p.alg = clqo::BBMC_EN_GCAND;
	//batch.add_test<CliqueEnumSparse>(p);*/

	//p.alg = clqo::BBMC_EN_GCONF;
	//batch.add_test<CliqueEnumSparse>(p);
	
	
	BkClique bcl(BENCHMARK_REAL_DIRECTORY_POSIX);
	/*bcl.RealTrivialEnum(10);
	bcl.RealTrivialEnum(20);
	bcl.RealTrivialEnum(30);
	bcl.RealTrivialEnum(50);
	bcl.RealTrivialEnum(100);*/
	
	bcl.RealLargeEnum(10);
	bcl.RealLargeEnum(20);
	bcl.RealLargeEnum(30);		
	bcl.RealLargeEnum(50);
	bcl.RealLargeEnum(60);
	//bcl.RealLargeHardEnum();
	//bcl.RealLargeExtremelyHardEnum();
	batch.benchmark_test(bcl, FILE_LOG(LOG_REAL_POSIX, APPEND), 1);
	
	//batch.run_single_instance(SINGLE_INSTANCE_PATH, FILE_LOG(LOG, WRITE));
}

//int main(){
//	Logger::SetInformationLevel(LOGGER_INFO);
//	PrecisionTimer pt;
//	
//	//random_attr_t rt(10, 50, .1, .9, 100, 1, .1);
//	random_attr_t rt(100, 150, .1, .4, 100, 50, .1);
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
//	//random_attr_t rt(5000, 5000, 0.1, 0.1, 1, 10, .1);
//
//	//random_attr_t rt(500, 500, .994, .994, 10, 50, .1);
//
//	//random_attr_t rt(1000, 1000, .1, .3, 10,  50, .01);
//	//random_attr_t rt(10000, 10000, .03, .03, 1,  50, .1);
//
//	//A batch object for tests with BBMC watched variants
//	BatchCLQGen<sparse_ugraph, Clique<sparse_ugraph> > batch;
//	
//	//configure parameters for all tests
//	clqo::param_t p;
//	p.tout=3600;			 //seconds
//	p.unrolled=true;
//	p.init_order=clqo::MAX_WIDTH;
//
//	p.alg = clqo::BBMC_EN_GCONF;
//	batch.add_test<CliqueEnumSparse>(p);
//
//	p.alg = clqo::BBMC_EN_GCONF_INIT;
//	batch.add_test<CliqueEnumSparse>(p);
//
//	//p.unrolled=false;
//	//p.alg = clqo::BBMC_EN_GCAND;
//	//batch.add_test<CliqueEnumSparse>(p);
//
//	//p.alg = clqo::BBMC_EN_GCONF;
//	//batch.add_test<CliqueEnumSparse>(p);
//	
//			
//	//batch.find_property(WRITE_PATH, rt);
//	batch.compare_two_tests(WRITE_PATH, rt, clqo::DIFFERENT_CLIQUE_SIZE, FILE_LOG(LOG_RANDOM, WRITE));
//	//batch.random_test(""/*STORE_INSTANCE_PATH*/, rt, FILE_LOG(LOG_RANDOM, APPEND));
//}










