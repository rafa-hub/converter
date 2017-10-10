//enumeration.cpp: binary for maximal clique enumeration
//author: pablo san segundo
//date of creation: 11/12/2015

#include <iostream>
#include "../batch/batch_benchmark.h"
#include "../batch/batch_gen.h"
#include "../batch/benchmark_clique.h"
#include "utils/file.h"
#include "../clique/clique_enum.h"
#include "../clique/clique_enum_sparse.h"
#include "../batch/benchmark_color.h"
//
//#define BENCHMARK_PATH				"C:/Users/pablo/Desktop/dimacs"	
//#define BENCHMARK_RANDOM_PATH		"C:/Users/pablo/Desktop/random"	
//#define LOG							"C:/Users/pablo/Desktop/enum_steps.txt"	
//#define LOG_RANDOM					"C:/Users/pablo/Desktop/rnd_enum.txt"	
#define WRITE_PATH					"C:/Users/pablo/Desktop/results/"	

//#define SINGLE_INSTANCE_PATH		"C:/Users/Pablo/Desktop/dimacs/johnson8-4-4.clq"
//#define SINGLE_INSTANCE_PATH		"C:/Users/Pablo/Desktop/dimacs/johnson16-2-4.clq"
//#define SINGLE_INSTANCE_PATH		"C:/Users/Pablo/Desktop/dimacs/p_hat300-2.clq"
//#define SINGLE_INSTANCE_PATH		"C:/Users/i7/Desktop/dimacs/hamming6-2.clq"
//#define SINGLE_INSTANCE_PATH		"C:/Users/pablo/Desktop/dimacs/hamming6-4.clq"
//#define SINGLE_INSTANCE_PATH		"C:/Users/pablo/Desktop/dimacs/c-fat200-5.clq"
//#define SINGLE_INSTANCE_PATH		"C:/Users/pablo/Desktop/dimacs/c-fat500-10.clq"
//#define SINGLE_INSTANCE_PATH		"C:/Users/Pablo/Desktop/dimacs/MANN_a9.clq"
//#define SINGLE_INSTANCE_PATH		"C:/Users/i7/Desktop/dimacs/keller4.clq"
//#define SINGLE_INSTANCE_PATH		"C:/Users/pablo/Desktop/r12_0.6_47.txt"
#define SINGLE_INSTANCE_PATH		"C:/Users/pablo/Desktop/r8_0.9_225.txt"

//#define LOG_POSIX						 "/var/www/logs/1dc.2048.txt"
//#define LOG_REAL_POSIX				 "/var/www/logs/dimacs_enum_all_2h.txt"
//#define BENCHMARK_DIRECTORY_POSIX		 "/media/datos/graphs/dimacs_bhosh/"
//#define SINGLE_INSTANCE_PATH_POSIX 	 "/media/datos/graphs/coding/cliqueVersion/1dc.2048.txt.gz.txt.clq"

#ifdef TESTS_DIMACS
int main(){
	Logger::SetInformationLevel(LOGGER_INFO);
	PrecisionTimer pt;
	BatchCLQBk<ugraph, Clique<ugraph> > batch;
			
	//configure parameters for all tests
	clqo::param_t p;
	p.unrolled=false ;
	p.init_order=clqo::MAX_WIDTH;							//default conditional ordering
	p.tout=3600*6;											//6h
	//p.init_order=clqo::NONE;							
				
	/*p.alg = clqo::BBMC_EN_GCAND;
	batch.add_test<CliqueEnum>(p);*/

	/*p.alg = clqo::BBMC_EN_GCONF;
	batch.add_test<CliqueEnum>(p);*/

	/*p.alg = clqo::BBMC_EN_GCONF_X;
	batch.add_test<CliqueEnum>(p);*/

	p.alg = clqo::BBMC_EN_GCONF_Xm;
	batch.add_test<CliqueEnum>(p);  

	p.alg = clqo::BBMC_EN_GCONF_XP;
	batch.add_test<CliqueEnum>(p);

	p.alg = clqo::BBMC_EN_GCONF_XPm;
	batch.add_test<CliqueEnum>(p);

	//p.alg = clqo::BBMC_EN_GCONF_FILTER;						//not working
	//batch.add_test<CliqueEnum>(p);

	/*BkClique bcl(BENCHMARK_PATH);
	bcl.ExpectedSubSecondDimacs();		
	batch.benchmark_test(bcl, FILE_LOG(LOG_DIMACS, WRITE), 1);*/


	/*BkColor bcl(BENCHMARK_PATH_POSIX);
	bcl.Flat();		
	batch.benchmark_test(bcl, FILE_LOG(LOG_DIMACS_POSIX, WRITE), 1);*/


	BkClique bcl(BENCHMARK_PATH_POSIX);
	bcl.Others();		
	batch.benchmark_test(bcl, FILE_LOG(LOG_DIMACS_POSIX, APPEND), 1);
	
	//batch.run_single_instance(SINGLE_INSTANCE_PATH, FILE_LOG(LOG_DIMACS, WRITE));
}
#else

int main(){
	Logger::SetInformationLevel(LOGGER_INFO);
	PrecisionTimer pt;
	
	random_attr_t rt(7, 50, .1, .9, 1000, 1, .1);
	//random_attr_t rt(100, 150, .5, .91, 100, 50, .1);
	//random_attr_t rt(150, 200, .7, .91, 10, 50, .1);
	// random_attr_t rt(500, 500, .994, .995, 10, 50, .1);
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
	  
	//random_attr_t rt(5000, 5000, 0.1, 0.1, 1, 10, .1);

	//random_attr_t rt(500, 500, .994, .994, 10, 50, .1);

	//random_attr_t rt(10000, 10000, .03, .03, 5,  50, .1);

	//A batch object for tests with BBMC watched variants
	BatchCLQGen<ugraph, Clique<ugraph> > batch;
	
	//configure parameters for all tests
	clqo::param_t p;
	p.tout=3600;						 //seconds
	p.unrolled=false;
	p.init_order=clqo::MAX_WIDTH;

   /* p.alg = BBMC;
	batch.add_test<Clique<ugraph>>(p);*/

	/*p.alg = clqo::BBMC_EN_GCAND;
	batch.add_test<CliqueEnum>(p);*/

	
	p.alg = clqo::BBMC_EN_GCONF_XP;
	batch.add_test<CliqueEnum>(p);

	p.alg = clqo::BBMC_EN_GCONF_X;
	batch.add_test<CliqueEnum>(p);
			

	//p.alg = clqo::BBMC_EN_GCAND_FILTER;
	//batch.add_test<CliqueEnum>(p);
				
	//batch.find_property(WRITE_PATH, rt);
	batch.compare_two_tests(WRITE_PATH, rt, clqo::DIFFERENT_STEPS_FIRST_GREATER_BY_A_FACTOR, FILE_LOG(LOG_RANDOM, WRITE));
	//batch.random_test(""/*STORE_INSTANCE_PATH*/, rt, FILE_LOG(LOG_RANDOM, APPEND));
	
}


#endif







