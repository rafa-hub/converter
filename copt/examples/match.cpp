//example of batch benchmark tests for all matching images benchmarks created by Jorge Artieda
//author: pablo san segundo
//date: 06/7/2015

#include <iostream>
#include "../batch/batch_benchmark.h"
#include "../batch/benchmark_clique.h"
#include "utils/file.h"
#include "../clique/clique_watched.h"
#include "../clique/clique_iter.h"
#include "../clique/clique_sat.h"
#include "../clique/clique_types.h"


#include <vector>
#include <string>
using namespace std;

//#define BENCHMARK_PATH				 "C:\\Users\\i7\\Desktop\\dimacs"
//#define LOG_POSIX					 "/var/www/logs/results_match/log_castle_b_mw_ils.txt"
#define LOG_POSIX					 "/var/www/logs/results_match/log_herzjesu_big_iter_satR_mw.txt"
//#define LOG						     "C:\\Users\\i7\\Desktop\\kk.txt"
#define SINGLE_INSTANCE_PATH		"C:\\Users\\pablo\\Desktop\\dimacs\\brock200_1.clq"


#define TIME_OUT_IN_SECONDS			600									//10min
#define INIT_ORDER_STRATEGY			MIN_WIDTH	
#define LOG_PATH_POSIX				 "/var/www/logs/results_match/"
#define BENCHMARK_DIRECTORY_POSIX	 "/media/datos/graphs/match/"		//missing final folder

enum test_id_t {castle=0, fountain, dino, airview, piedra, kapel, herzjesu, heligrande, ualberta, fpv1};
struct test_data{
	string benchmark_path;
	string log_file;
	test_id_t id;
};
typedef vector<test_data> v_t;


int main(){
	Logger::SetInformationLevel(LOGGER_INFO);
	PrecisionTimer pt;

////////////////
//define tests
	
	test_data d;
	v_t tests;
	//
	//d.benchmark_path=BENCHMARK_DIRECTORY_POSIX;
	//d.benchmark_path+="herzjesu";
	//d.log_file=LOG_PATH_POSIX; 
	//d.log_file+="log_herzjesu_big_mw_tout600.txt";
	//d.id=herzjesu;
	//tests.push_back(d);

	d.benchmark_path=BENCHMARK_DIRECTORY_POSIX;
	d.benchmark_path+="castle_b";
	d.log_file=LOG_PATH_POSIX; 
	d.log_file+="log_castle_big_5_6_mw_tout600.txt";
	d.id=castle;
	tests.push_back(d);

	/*d.benchmark_path=BENCHMARK_DIRECTORY_POSIX;
	d.benchmark_path+="fpv1";
	d.log_file=LOG_PATH_POSIX;
	d.log_file+="log_fpv1_big_mw_no_ils.txt";
	d.id=fpv1;
	tests.push_back(d);*/

	//****

////////////////////////////////
//launches all tests


	for(v_t::iterator it= tests.begin(); it<tests.end() ; it++){

		BatchCLQBk<ugraph, Clique<ugraph>> batch;

		//configures parameters for the benchmark
		clqo::param_t p;
		p.tout=TIME_OUT_IN_SECONDS;						
		p.unrolled=false;
		p.init_order=clqo::INIT_ORDER_STRATEGY;

		//algorithms to be run
		p.alg = clqo::BBMC;
		batch.add_test<Clique<ugraph>>(p);

		p.alg = clqo::BBMC_W;
		batch.add_test<CliqueWatched>(p);

		p.alg = clqo::BBMC_WT;
		batch.add_test<CliqueWatched>(p);

		//p.alg = BBMC_Iter_Root_Sat_R;
		//batch.add_test<CliqueIter>(p);

		//p.alg = BBMCXR_L;
		//batch.add_test<CliqueSat>(p);


		//p.alg = BBMC_Iter_Root_Enlarge_Sat_R_N;
		//batch.add_test<CliqueWatched>(p);

		
		//determines graphs 
		BkClique bcl((*it).benchmark_path);
		switch((*it).id){
		case fpv1:
			bcl.Fpv1();
			break;
		case castle:
			bcl.Castle();
			break;
		case herzjesu:
			bcl.Herzjesu();
			break;
			//***
		default:
			LOG_ERROR("bad test_type in match tests");
		}

		//runs tests
		batch.benchmark_test(bcl, FILE_LOG((*it).log_file.c_str(), APPEND), 1);
		//batch.run_single_instance(SINGLE_INSTANCE_PATH, cout);
	}

}

//int main(){
//	Logger::SetInformationLevel(LOGGER_INFO);
//	PrecisionTimer pt;
//	BkClique bcl(BENCHMARK_PATH_POSIX);
//	bcl.Fpv1();
//
//	//BatchCLQBk<ugraph, CliqueWatched> batch;
//	BatchCLQBk<ugraph, Clique<ugraph>> batch;
//		
//	//configure parameters for all tests
//	param_t p;
//	p.tout=10800;						 //3h
//	p.unrolled=false;
//	p.init_order=MIN_WIDTH_KCORE;
//	//p.init_order=MIN_WIDTH;
//		
//	p.alg = BBMC;
//	batch.add_test<Clique<ugraph>>(p);
//
//	p.alg = BBMC_W;
//	batch.add_test<CliqueWatched>(p);
//
//	p.alg = BBMC_WT;
//	batch.add_test<CliqueWatched>(p);
//				
//	batch.benchmark_test(bcl, FILE_LOG(LOG_POSIX, WRITE), 1);
//	//batch.run_single_instance(SINGLE_INSTANCE_PATH, cout);
//}

////////////
//// random benchmarks
//int main(){
//	Logger::SetInformationLevel(LOGGER_INFO);
//	PrecisionTimer pt;
//	
//	BkClique bcl(BENCHMARK_PATH);
//	//bcl.Real50();
//
//	/*random_attr_t rt(150, 151, .7, .91, 10, 50, .1);
//	random_attr_t rt(150, 151, .95, .99, 10, 50, .03);
//	random_attr_t rt(200, 200, .7, .91, 10, 50, .1);
//	random_attr_t rt(200, 200, .95, .99, 10, 50, .03);
//	random_attr_t rt(300, 300, .6, .81, 10, 50, .1);*/
//	//random_attr_t rt(500, 500, .4, .71, 10, 50, .1);
//	//random_attr_t rt(3000, 3001, .2, .21, 10, 50, .1);
//	random_attr_t rt(5000, 5001, .2, .21, 10, 50, .1);
//	BatchCLQBk<ugraph, CliqueWatched > batch;
//		
//	//configure parameters for all tests
//	param_t p;
//	//p.tout=3600;  //segundos
//	p.unrolled=false;
//	p.init_order=MIN_WIDTH_KCORE;
//		
//	p.alg = BBMC_W;
//	batch.add_test<CliqueWatched>(p);
//
//	p.alg = BBMC_WT;
//	batch.add_test<CliqueWatched>(p);
//		
//	batch.random_test(BENCHMARK_RANDOM_PATH, rt, FILE_LOG(LOG, APPEND));
//	//batch.benchmark_test(bcl, FILE_LOG(LOG, WRITE), 1);
//	//batch.run_single_instance(SINGLE_INSTANCE_PATH, cout);
//}

//////////////
//// random benchmarks
//int main(){
//	Logger::SetInformationLevel(LOGGER_INFO);
//	PrecisionTimer pt;
//	//BkClique bcl(BENCHMARK_PATH);
//	//bcl.Real50();
//
//	/*random_attr_t rt(150, 151, .7, .91, 10, 50, .1);
//	random_attr_t rt(150, 151, .95, .99, 10, 50, .03);
//	random_attr_t rt(200, 200, .7, .91, 10, 50, .1);
//	random_attr_t rt(200, 200, .95, .99, 10, 50, .03);
//	random_attr_t rt(300, 300, .6, .81, 10, 50, .1);*/
//	//random_attr_t rt(500, 500, .4, .71, 10, 50, .1);
//	//random_attr_t rt(3000, 3001, .2, .21, 10, 50, .1);
//	//random_attr_t rt(5000, 5001, .2, .21, 10, 50, .1);
//	random_attr_t rt(30000, 30001, .1, .11, 5, 50, .1);
//	BatchCLQBk<ugraph, Clique<ugraph> > batch;
//		
//	//configure parameters for all tests
//	param_t p;
//	//p.tout=3600;  //segundos
//	p.unrolled=false;
//	p.init_order=MIN_WIDTH_KCORE;
//		
//	p.alg = BBMC;
//	batch.add_test<Clique<ugraph>>(p);
//				
//	batch.random_test(BENCHMARK_RANDOM_PATH, rt, FILE_LOG(LOG, APPEND));
//	//batch.benchmark_test(bcl, FILE_LOG(LOG, WRITE), 1);
//	//batch.run_single_instance(SINGLE_INSTANCE_PATH, cout);
//}




