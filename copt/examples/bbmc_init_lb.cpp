//example of simple BBMC execution from cmd line: <filename> <lower bound>  
//author:pablo san segundo
//date: 01/07/14

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
			
	//reading file
	string filename(argv[1]);
	cout<<"READING----------------:"<<filename<<endl;
	pt.wall_tic();
	ugraph ug(filename);
	time_in_sec=pt.wall_toc();
	ug.print_data();
	cout<<"[t:"<<time_in_sec<<"]"<<endl<<endl;
	
	//launch default BBMC (set_up, run, tear down)  
	Clique<ugraph> cug(&ug, clqo::param_t());					//default params
	cout<<"SETUP-----------------"<<endl;
	pt.wall_tic();
	cug.hint_lb(atoi(argv[2]));					//sets initial clique size
	int sol=cug.set_up();								//note: this modifies the graph, explicit sorting of adjacency matrix
	time_in_sec=pt.wall_toc();

	cout<<"[t:"<<time_in_sec<<"]"<<endl<<endl;

	//check is trivial solution is found and there is no need for the NP-hard search step
	if(sol>0){
		cout<<ug.get_name()<<" FOUND TRIVIAL SOLUTION DURING SETUP: "<<sol<<endl;
		return 0;
	}

	//NP-hard search
	cout<<"SEARCH-----------------"<<endl;
	pt.wall_tic();
	cug.run();	
	time_in_sec=pt.wall_toc();

	cout<<"[t:"<<time_in_sec<<"]"<<"[w: "<<cug.get_max_clique()<<"]"<<endl;
	
	//deallocates resources
	cug.tear_down();

	cout<<"---------------------------------------------"<<endl;
}