//examples of batch benchmark tests

#include <iostream>
#include "../batch/benchmark_clique.h"
#include "../batch/batch_benchmark.h"
#include "utils/file.h"
#include "gtest/gtest.h"
#include "../clique/clique.h"
#include "../clique/clique_watched.h"

using namespace std;


#define BENCHMARK_PATH			"/media/datos/graphs/dimacs_bhosh"
//#define BENCHMARK_PATH		"C:\\Users\\pablo\\Desktop\\dimacs"
#define SINGLE_INSTANCE_PATH	"C:\\Users\\pablo\\Desktop\\dimacs\\brock200_1.clq"
//#define LOG					 "/var/www/logs/results_semi_sparse"
#define LOG						"/var/www/logs/log_kk.txt"
#define LOG_RANDOM_VENUS				"/var/www/logs/rnd200_watched_mw.txt"
#define BENCHMARK_RANDOM_PATH_VENUS		"/media/datos/graphs/random"


random_attr_t rt(200, 200, .1, .9,10, 100, .1);

int main(){
	Logger::SetInformationLevel(LOGGER_PRINT);
	PrecisionTimer pt;
	
	//benchmark object and database selection
	BkClique bc(BENCHMARK_PATH);
	bc.Dimacs();

	//A batch benchmark object for simple ugraphs
	BatchCLQBk<ugraph, Clique<ugraph> > batch;
	
	//configure parameters for all tests
	clqo::param_t p;
	p.tout=10800;							//in seconds
	p.unrolled=false;
	p.init_order=clqo::MIN_WIDTH;

	//add test
	p.alg=clqo::BBMC;
	batch.add_test< Clique<ugraph> >(p);
	
	//add test
	p.alg=clqo::BBMC_W;
	batch.add_test<CliqueWatched>(p);

	p.alg=clqo::BBMC_WT;
	batch.add_test<CliqueWatched>(p);
	
	//compares the two algorithms for the whole DIMACS benchmark
	//batch.compare_two_tests(bc,DIFFERENT_STEPS, FILE_LOG(LOG, WRITE));
	
	//runs single instance (independent of benchmark)
	//batch.run_single_instance(SINGLE_INSTANCE_PATH, FILE_LOG(LOG, APPEND));

	//runs the particular database
	//batch.benchmark_test(bc, FILE_LOG(LOG, APPEND), 1);
	batch.random_test(BENCHMARK_RANDOM_PATH_VENUS,rt,FILE_LOG(LOG_RANDOM_VENUS, WRITE));

	
	cout<<"----------------------------------------------";	

}