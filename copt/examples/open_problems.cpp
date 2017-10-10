//open_problems.cpp: binary for open-problem challenges
//author: pablo san segundo
//date: 10/12/2015

#include <iostream>
#include "../batch/batch_benchmark.h"
#include "../batch/batch_gen.h"
#include "../batch/benchmark_clique.h"
#include "utils/file.h"
#include "../clique/clique_sat.h"
#include "../clique/clique_iter.h"
#include "../clique_para/clique_para_sat.h"
#include "../clique_para/clique_para_iter.h"

#define BENCHMARK_PATH				"C:/Users/pablo/Desktop/dimacs"	
#define BENCHMARK_RANDOM_PATH		"C:/Users/pablo/Desktop/random"	
#define LOG							 "C:/Users/pablo/Desktop/kk_sat.txt"	
#define LOG_RANDOM					 "C:/Users/pablo/Desktop/kk_rnd_sat.txt"	
#define WRITE_PATH				     "C:\\Users\\i7\\Desktop/"	

//#define SINGLE_INSTANCE_PATH_POSIX	"/media/datos/graphs/frb_zavalnij/evil-myc-fbr11-k20.clq"
//#define SINGLE_INSTANCE_PATH		"C:\\Users\\pablo\\Desktop\\2dc.2048.txt.gz.txt.clq"
#define SINGLE_INSTANCE_PATH		"C:/Users/pablo/Desktop/1dc.2048.txt.gz.txt.clq"
//#define SINGLE_INSTANCE_PATH		"C:/Users/pablo/Desktop/evil-myc-fbr11-k16.clq"

#define LOG_POSIX					 "/var/www/logs/1dc.2048.txt"
#define BENCHMARK_DIRECTORY_POSIX	 "/media/datos/graphs/dimacs_bhosh/"
#define SINGLE_INSTANCE_PATH_POSIX	"/media/datos/graphs/coding/cliqueVersion/1dc.2048.txt.gz.txt.clq"

int main(){
	Logger::SetInformationLevel(LOGGER_INFO);
	PrecisionTimer pt;
	BatchCLQBk<ugraph, CliquePara<ugraph> > batch;

		
	//configure parameters for all tests
	clqo::param_t p;
	p.nThreads=CliquePara<ugraph>::get_max_cores_hw()-1;	//all except one core
	p.unrolled=true;
	p.init_order=clqo::RLF_COND;							//default conditional ordering
				
	/*p.alg = clqo::BBMC;
	batch.add_test<Clique<ugraph>>(p);*/
		
	p.alg = clqo::BBMCXR_L;
	batch.add_test<CliqueParaSat>(p);

	/*p.alg = clqo::BBMCITXR_L;
	batch.add_test<CliqueParaIter>(p);*/
			

	/*p.alg = BBMCX_L;
	batch.add_test<CliqueSat>(p);

	p.alg = BBMCXR;
	batch.add_test<CliqueSat>(p);

	p.alg = BBMCXR_L;
	batch.add_test<CliqueSat>(p);
		
	p.alg = BBMCXR_L_SEQ;
	batch.add_test<CliqueSat>(p);*/
	
	/*BkClique bcl(BENCHMARK_PATH);
	bcl.Keller();		
	batch.benchmark_test(bcl, FILE_LOG(LOG, WRITE), 1);*/
	
	batch.run_single_instance(SINGLE_INSTANCE_PATH_POSIX, FILE_LOG(LOG_POSIX, WRITE));
}










