//example of simple BBMCSP (exact maximum clique for large and massive graphs) execution from cmd line:  
//Cmd line info: <binary> <filename> <lower bound> 
//author:pablo san segundo
//date: 20/12/14

#include <iostream>
#include <stdlib.h>						 
#include "utils/prec_timer.h"
#include "../clique/clique.h"

using namespace std;

int main(int argc, char** argv){
	PrecisionTimer pt;
	double time_in_sec;
		
	if(argc!=3){
		cerr<<"incorrect number of arguments"<<endl;
		return -1;
	}
			
	//parse cmd lin params
	string filename(argv[1]);
	cout<<"READING-------------------------------"<<filename<<endl;
	pt.wall_tic();
	sparse_ugraph usg(filename);
	time_in_sec=pt.wall_toc();
	usg.print_data();
	cout<<"[t:"<<time_in_sec<<"]"<<endl<<endl;
	
	//launch serial version
	Clique<sparse_ugraph> cusg(&usg, clqo::param_t());
	cusg.hint_lb(atoi(argv[2]));								//suggests initial clique size
	cout<<"SETUP----------------------------------"<<endl;		//note that the setup step will modify the graph in the general case (explicit reordering)
	pt.wall_tic();
	int sol=cusg.set_up_unrolled(clqo::search_alloc_t());			
	time_in_sec=pt.wall_toc();
	cout<<"[t:"<<time_in_sec<<"]"<<endl<<endl;

	//check is trivial solution is found and there is no need for the NP-hard search step
	if(sol!=0){
		cout<<usg.get_name()<<" FOUND TRIVIAL SOLUTION DURING SETUP: "<<sol<<endl;
			return 0;
	}

	//search
	cout<<"SEARCH---------------------------------"<<endl;
	pt.wall_tic();
	cusg.run_unrolled();	
	time_in_sec=pt.wall_toc();

	cout<<"[t:"<<time_in_sec<<"]"<<"[w: "<<cusg.get_max_clique()<<"]"<<endl;
	cusg.tear_down();

	cout<<"---------------------------------------------"<<endl;
}