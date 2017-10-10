//published_alg.cpp : Full command line interface for stand-alone framework
//				      Includes different releases for published algorithms	
// 
//author: pss
//date:14/04/15

#include <iostream> 
#include "utils/prec_timer.h"
#include "utils/file.h"
#include "utils/logger.h"
#include "../batch/batch_benchmark.h"
#include "../clique/clique.h"
#include "../clique/clique_watched.h"
#include "../clique//clique_sat.h"
#include "../clique/clique_infra.h"
#include "../input.h"												//uses TCLAP lib to parse command line

using namespace std;

//#define LOG  "C:/Users/pablo/Desktop/log.txt"
#define LOG  "log.txt"


//int main(int argc, char** argv){
//	BatchCLQBk<ugraph, Clique<ugraph> > batch;
//	input io(argc, argv);
//	clqo::param_t p=io.parse();					//parses cmd line
//			
//	//configure parameters for all tests
//	batch.add_test<Clique<ugraph>>(p);
//	batch.run_single_instance(p.filename, FILE_LOG(LOG, WRITE));
//
//	cout<<"--------------------------------------------"<<endl;
//}

////////////////////////
//
// RELEASES
//
////////////////////////

//////////////
// Optimization Methods and Software 
// title: An enhanced bitstring encoding for exact maximum clique search in sparse graphs
// date: 1/2016
// variants: BBMCW, BBMCWT
// init_order: degeneracy ordering (non-increasing k-core ordering)

//int main(int argc, char** argv){
//	BatchCLQBk<ugraph, CliqueWatched > batch;
//	input io(argc, argv);
//	clqo::param_t p=io.parse();					//parses cmd line
//
//	//algorithms: only BBMCW or BBMCWT
//	if(p.alg!=clqo::BBMC_W && p.alg!=clqo::BBMC_WT){
//		cout<<"incorrect algorithm; this release is only for watched variants"<<endl;
//		cout<<"flag -a 6 (BBMCW) or -a 7 (BBMCWT)"<<endl;
//		return 0;
//	}
//	p.print_algorithm(cout);
//
//	
//	//overwrite specific attributes
//	p.init_order=clqo::MIN_WIDTH_KCORE;
//		
//	//configure parameters for all tests
//	batch.add_test<CliqueWatched>(p);
//	batch.run_single_instance(p.filename, cout);		//FILE_LOG(LOG, WRITE)
//	
//
//	cout<<"--------------------------------------------"<<endl;
//}

////////////////////

////////////////
//// APPLIED INTELLIGENCE 
//// title: Improved initial sorting for exact maximum clique search
//// date: 1/2016
//// variant: BBMCX with enhancements (switch for STRONG_ROOT_COLORING ON)
//// init_order default: RLF_COND initial sorting
//
//int main(int argc, char** argv){
//	BatchCLQBk<ugraph, Clique<ugraph> > batch;
//	input io(argc, argv);
//	clqo::param_t p=io.parse();					//parses cmd line
//	
//	//algorithms: only BBMCX
//	cout<<"this release is only for subchromatic variant BBMCX enhanced with initial sorting enhancements"<<endl;
//	/*cout<<"the first run uses chromatic initial coloring"<<endl;
//	cout<<"the second run uses degree initial coloring with p=4"<<endl;*/
//		
//	//overwrite specific attributes
//	p.alg=clqo::BBMCX;
//	p.init_order=clqo::RLF_COND;
//	batch.add_test<CliqueSat>(p);
//
//	/*p.alg=clqo::BBMCX;
//	p.init_order=clqo::MIXED_4;
//	batch.add_test<CliqueSat>(p);*/
//
//	/*p.alg=clqo::BBMCX;
//	p.init_order=clqo::MIN_WIDTH;
//	batch.add_test<CliqueSat>(p);*/
//			
//	//configure parameters for all tests
//	batch.run_single_instance(p.filename, FILE_LOG(LOG, APPEND));
//
//	cout<<"--------------------------------------------"<<endl;
//}

//////////////
// APPLIED INTELLIGENCE 
// title: Improved initial sorting for exact maximum clique search
// date: 1/2016
// variant: BBMCX WITHOUT enhancements  (switch for STRONG_ROOT_COLORING OFF)
// init_order default: MIN_WIDTH

//int main(int argc, char** argv){
//	BatchCLQBk<ugraph, Clique<ugraph> > batch;
//	input io(argc, argv);
//	clqo::param_t p=io.parse();					//parses cmd line
//	
//	//algorithms: only BBMCX
//	cout<<"this release is only for subchromatic variant BBMCX without initial sorting enhancements"<<endl;
//		
//	//overwrite specific attributes
//	p.alg=clqo::BBMCX;
//	p.init_order=clqo::MIN_WIDTH;
//	batch.add_test<CliqueSat>(p);
//
//			
//	//configure parameters for all tests
//	batch.run_single_instance(p.filename, FILE_LOG(LOG, APPEND));
//
//	cout<<"--------------------------------------------"<<endl;
//}

//////////////
// Chu-Min Li version for comparison purposes
// date: 8/6/2016
// variant: BBMCX with enhancements (switch for STRONG_ROOT_COLORING ON)-Ref. APPLIED INTELLIGENCE release
// init_order default: RLF_COND initial sorting
//
//int main(int argc, char** argv){
//	BatchCLQBk<ugraph, Clique<ugraph> > batch;
//	input io(argc, argv);
//	clqo::param_t p=io.parse();							//parses cmd line
//	Logger::SetInformationLevel(LOGGER_INFO);			
//	
//	//algorithms: only BBMCX
//	LOG_INFO("BBMCX variant enhanced with preprocessing");
//	LOG_INFO("released for Chu-Min Li (9/6/16)");
//	/*LOG_INFO("Sorting: best between chromatic and min-deg-last [-p 0]");  
//	LOG_INFO("UB: initial tight bounds for every vertex");
//	LOG_INFO("LB: initial tight bound using heur [-o 0]");*/
//			
//	//overwrite specific attributes
//	p.alg=clqo::BBMCX;
//	//p.init_order=clqo::RLF_COND;
//	batch.add_test<CliqueSat>(p);
//
//	/*p.alg=clqo::BBMCX;
//	p.init_order=clqo::MIXED_4;
//	batch.add_test<CliqueSat>(p);*/
//
//	/*p.alg=clqo::BBMCX;
//	p.init_order=clqo::MIN_WIDTH;
//	batch.add_test<CliqueSat>(p);*/
//			
//	//configure parameters for all tests
//	batch.run_single_instance(p.filename, FILE_LOG(LOG, APPEND));
//		
//
//	cout<<"--------------------------------------------"<<endl;
//}

//////////////
// Chu-Min Li version for comparison purposes
// date: 20/7/2016
// variant: BBMCX with enhancements (switch for STRONG_ROOT_COLORING ON)-Ref. APPLIED INTELLIGENCE release
// init_order: MIN_WIDTH (tiebreaks take too long)
// more than 15000 vertices
//
//int main(int argc, char** argv){
//	BatchCLQBk<ugraph, Clique<ugraph> > batch;
//	input io(argc, argv);
//	clqo::param_t p=io.parse();							//parses cmd line
//	Logger::SetInformationLevel(LOGGER_INFO);			
//	
//	//algorithms: only BBMCX
//	LOG_INFO("BBMCX variant enhanced with preprocessing" );
//	LOG_INFO("released for Chu-Min Li (9/6/16)");
//	/*LOG_INFO("Sorting: best between chromatic and min-deg-last [-p 0]");  
//	LOG_INFO("UB: initial tight bounds for every vertex");
//	LOG_INFO("LB: initial tight bound using heur [-o 0]");*/
//			
//	//overwrite specific attributes
//	p.alg=clqo::BBMCX;
//	//p.init_order=clqo::RLF_COND;
//	batch.add_test<CliqueSat>(p);
//
//	/*p.alg=clqo::BBMCX;
//	p.init_order=clqo::MIXED_4;
//	batch.add_test<CliqueSat>(p);*/
//
//	/*p.alg=clqo::BBMCX;
//	p.init_order=clqo::MIN_WIDTH;
//	batch.add_test<CliqueSat>(p);*/
//			
//	//configure parameters for all tests
//	batch.run_single_instance(p.filename, FILE_LOG(LOG, APPEND));
//		
//
//	cout<<"--------------------------------------------"<<endl;
//}


//////////////
// Chu-Min Li version for comparison purposes (best current variant)
// date: 20/8/2016
// variant: BBMCR_1.0-Extended Infrachrom IncMaxCLQ, recoloring up to col-1, when col>=kmin
// init_order default: RLF_COND initial sorting

//int main(int argc, char** argv){
//	BatchCLQBk<ugraph, Clique<ugraph> > batch;
//	input io(argc, argv);
//	clqo::param_t p=io.parse();							//parses cmd line
//	Logger::SetInformationLevel(LOGGER_INFO);			
//	
//	//algorithms: only BBMCX
//	LOG_INFO("BBMCXRL_1.0  enhanced infrachrom for less than 15000 nodes-experimental" );
//	LOG_INFO("released 16/09/16");
//	//LOG_INFO("initial sorting: best between color and degree [-o 0], degree with support [-o 1], degree [-o 2]");  
//	//LOG_INFO("initial solution: strong heuristic");
//
//	/*LOG_INFO("UB: initial tight bounds for every vertex");
//	LOG_INFO("BBMCX variant enhanced with preprocessing with more than 25000 vertices");
//	LOG_INFO("preprocessing and init-ordering are fixed, use -f <filename> only");
//	LOG_INFO("released for Chu-Min Li (20/07/16)");
//	/*LOG_INFO("Sorting: best between chromatic and min-deg-last [-p 0]");  
//	LOG_INFO("UB: initial tight bounds for every vertex");
//	LOG_INFO("LB: initial tight bound using heur [-o 0]");*/
//			
//	//overwrite specific attributes
//	//p.init_order=clqo::MIN_WIDTH;
//	//p.init_preproc=clqo::HEUR;
//	p.init_order=clqo::RLF_COND;
//	p.init_preproc=clqo::UB_HEUR;
//	p.alg=clqo::BBMCRL_KMIN;
//	batch.add_test<CliqueInfra>(p);
//
//				
//	//configure parameters for all tests
//	batch.run_single_instance(p.filename, FILE_LOG(LOG, APPEND));
//		
//
//	cout<<"--------------------------------------------"<<endl;
//}
////////////////////

//////////////
// BBMCSP release for  Hamed Karimi
// date: 23/1/2017
// variant: BBMCSP with output for pre-processing and only -f for name of file

//int main(int argc, char** argv){
//	BatchCLQBk<sparse_ugraph, Clique<sparse_ugraph> > batch;
//	input io(argc, argv);
//	clqo::param_t p=io.parse();							//parses cmd line
//	Logger::SetInformationLevel(LOGGER_INFO);			
//	
//	//algorithms: only BBMCX
//	LOG_INFO("BBMCSP_1.0 release for Hamed Karimi (23/01/17)" );
//
//	//LOG_INFO("initial sorting: best between color and degree [-o 0], degree with support [-o 1], degree [-o 2]");  
//	//LOG_INFO("initial solution: strong heuristic");
//
//	/*LOG_INFO("UB: initial tight bounds for every vertex");
//	LOG_INFO("BBMCX variant enhanced with preprocessing with more than 25000 vertices");
//	LOG_INFO("preprocessing and init-ordering are fixed, use -f <filename> only");
//	LOG_INFO("released for Chu-Min Li (20/07/16)");
//	/*LOG_INFO("Sorting: best between chromatic and min-deg-last [-p 0]");  
//	LOG_INFO("UB: initial tight bounds for every vertex");
//	LOG_INFO("LB: initial tight bound using heur [-o 0]");*/
//			
//	//overwrite specific attributes
//	//p.init_order=clqo::MIN_WIDTH;
//	//p.init_preproc=clqo::HEUR;
//	//p.init_order=clqo::RLF_COND;
//	//p.init_preproc=clqo::UB_HEUR;
//
//	p.alg=clqo::BBMC;
//	p.unrolled=true;
//	batch.add_test<Clique<sparse_ugraph>>(p);
//
//				
//	//configure parameters for all tests
//	batch.run_single_instance(p.filename, cout);
//		
//
//	cout<<"--------------------------------------------"<<endl;
//}

//////////////
// BBMC, BBMCL, BBMCXR release 
// date: 29/06/2017
// variant: BBMCSP with output for pre-processing and only -f for name of file

int main(int argc, char** argv){
	BatchCLQBk< ugraph, Clique<ugraph> > batch;
	input io(argc, argv);
	clqo::param_t p=io.parse();							//parses cmd line
	Logger::SetInformationLevel(LOGGER_INFO);			
	
	//algorithms: only BBMCX
	LOG_INFO("BBMC, BBMCR, BBMCL(_R), BBMCXR release for reasearchers (29/06/17)" );

	//LOG_INFO("initial sorting: best between color and degree [-o 0], degree with support [-o 1], degree [-o 2]");  
	//LOG_INFO("initial solution: strong heuristic");

	/*LOG_INFO("UB: initial tight bounds for every vertex");
	LOG_INFO("BBMCX variant enhanced with preprocessing with more than 25000 vertices");
	LOG_INFO("preprocessing and init-ordering are fixed, use -f <filename> only");
	LOG_INFO("released for Chu-Min Li (20/07/16)");
	/*LOG_INFO("Sorting: best between chromatic and min-deg-last [-p 0]");  
	LOG_INFO("UB: initial tight bounds for every vertex");
	LOG_INFO("LB: initial tight bound using heur [-o 0]");*/
			
	//overwrite specific attributes
	p.init_order=clqo::MIN_WIDTH_MIN_TIE_STATIC;
	p.init_preproc=clqo::UB;
	//p.init_order=clqo::RLF_COND;
	//p.init_preproc=clqo::UB_HEUR;

	//p.alg=clqo::BBMC;
	//p.unrolled=false;
	switch(p.alg){
	case clqo::BBMC:
	case clqo::BBMCR:
	case clqo::BBMCL_R:
		batch.add_test<Clique<ugraph>>(p);
		break;
	case clqo::algorithm_t::BBMCXR:
		batch.add_test<CliqueSat>(p);	
	default:
		;
	}
						
	//configure parameters for all tests
	batch.run_single_instance(p.filename, cout);
	
	cout<<"--------------------------------------------"<<endl;
}
