/*
 * amts_exec_h: interface for AMTSexec Class, a driver for a controlled execution of AMTS algorithm
 *
 *  Created on: 23/10/2015
 *  Developer: psansegundo
 *
 */

#ifndef _AUTO_EXEC_H_
#define _AUTO_EXEC_H_

//#include <ctype.h>
//#include <stdio.h>
//#include <stdlib.h>
#include <iostream>
#include <sstream>
#include "utils/logger.h"
#include "utils/result.h"
#include "../amts/mnts.h"

using namespace std;

typedef vector<int> vint;

class AMTSexec{
///////////
//interface for execution
	static const int DEF_UPPER_BOUND=-1;
	static const int DEF_LEN_TIME=11;
	static const int DEF_LEN_IMPROVE=10000;
	static const int DEF_WMODE=1;					//default mode for unweighted heuristic

public:
	AMTSexec(int lt=DEF_LEN_TIME, int li=DEF_LEN_IMPROVE, int wmode=1, int ub=DEF_UPPER_BOUND):len_time(lt), len_improve(li), weight_mode(wmode), upper_bound(ub){}
	template <typename T>
	int run (const T&g, bool store_sol=false);

	int run (string filename);									//**TODO

	template <typename T>
	Result pool_of_solutions(T&g, int NSOL, int min_size=1);
	
	
	vint get_nodes(){return lnodes;}

private:
	int len_time;									//number of restarts
	int len_improve;								//iterations for each run
	int upper_bound;								//initial upperbound (DEF=-1)
	int weight_mode;								//modulus to compute weights: wi= i*%wmode+1  (18/10/2016)	
	vint lnodes;
};

template<class T>
inline
int AMTSexec::run(const T&gout,  bool store_sol){
////////////////////
// returns the size of the biggest clique found by the AMTS heuristic
	int len_W=0;	//size of clique
	int sol_W=0;	//can be size of clique (w=1) or summation of weights
	lnodes.clear();

	T g(gout);		//works on a copy
		
	//log input
	ostringstream sstr;
	sstr<<"-----------------------"<<endl;
	sstr<<"finding strong clique lb-amts for: ";
	g.print_data(true, sstr);

	//initial upper bound
	if(upper_bound==DEF_UPPER_BOUND){
		int NV=g.number_of_vertices();
		if(weight_mode==1)
			upper_bound=NV;
		else{					//weighted default upper bound
			upper_bound=0;
			for(int i=0; i<NV; i++){
				upper_bound+=g.get_wv(i);
			}		
		}
	}

	//solving
	int lbest=0;
	int lcur=0;
	for(int i = 0; i < len_time; i++ ) {

		//builds object(waim, wlen, len_improve)-jorge
		mnts alg(&g, upper_bound, weight_mode, len_improve);				 //** TODO: allocates memory each time	"
		lcur = tabu(alg.len_improve, alg);
		if( lcur>= lbest )  {
			lbest = lcur;
			len_W = alg.len_best;
			if(store_sol){
				lnodes.clear();
				for(int i=0; i<g.number_of_vertices(); i++){
					if(alg.Tbest[i]==1)
						lnodes.push_back(i);
				}
			}
		}

		//exit if correct solution is found
		/* if( lcur>= upper_bound ){
		lbest=lcur;
		break;
		}*/
	}

	 	 
	//I/O
	sstr<<"AMTS-(lb:"<<lbest<<":["<<len_W<<"])"<<endl;
	/*if(store_sol){
		com::stl::print_collection(lnodes, sstr);
	}*/
	sstr<<"-----------------------"<<endl;
	LOG_INFO(sstr.str());
	return lbest;
};


template<class T>
inline
Result AMTSexec::pool_of_solutions(T& g, int NSOL, int min_size){
///////////////
// One solution per RESTART 
// Currently only for the unweighted case

	ostringstream sstr;
	sstr<<"-----------------------"<<endl;
	sstr<<"finding pool of cliques lb-amts for: ";
	//T g(gout);		//works on a copy
	g.print_data(false, sstr);
	LOG_INFO(sstr.str());
	
	int NV=g.number_of_vertices();

	int lbest=0;
	int lcur=0;
	Result r(NSOL);
	for(int i = 0; i < len_time; i++ ) {
		mnts alg(&g, NV, 1, len_improve);				 //** TODO: allocates memory each time	"
		lcur = tabu(alg.len_improve, alg);
		//if( lcur> lbest ){
		//	lbest=lcur;
			lnodes.clear();
			for(int i=0; i<NV; i++){
				if(alg.Tbest[i]==1)
					lnodes.push_back(i);
			}

			if(lnodes.size()>=min_size){
				if (r.add_solution(lnodes)==false) 
					break;
			}
		//}
	}

	
	LOG_INFO("------------------------------");
	return r;
}





#endif






