//example of simple parallel branch and bound BBMCSP from command line: <binary> <filename>
//author: pablo san segundo


#include <iostream>
#include "utils/prec_timer.h"
#include "../clique/clique.h"
#include "../clique_para/clique_parallel.h"

using namespace std;

int main(int argc, char** argv){
	PrecisionTimer pt;
	double time_in_sec;
	
	if(argc!=2){
		cerr<<"incorrect number of arguments"<<endl;
		return -1;
	}
	
	//parse params
	string filename(argv[1]);
	cout<<"READING----------------------"<<filename<<endl;
	pt.wall_tic();
	sparse_ugraph usg(filename);
	time_in_sec=pt.wall_toc();
	usg.print_data();
	cout<<"[t:"<<time_in_sec<<"]"<<endl<<endl;
	
	//launch parallel branch and bound BBMCSP n
	Clique_parallel<sparse_ugraph> cusg(usg);
	cout<<"Max cores HW: "<<cusg.get_max_cores_hw()<<endl;
    cusg.set_cores(cusg.get_max_cores_hw()); 
	

	cout<<"SETUP----------------------------"<<endl;
	pt.wall_tic();
	int sol=cusg.set_up_unrolled();
	time_in_sec=pt.wall_toc();
	cout<<"[t:"<<time_in_sec<<"]"<<endl<<endl;

	if(sol!=0){
		cout<<usg.get_name()<<"FOUND TRIVIAL SOLUTION DURING SETUP: "<<sol<<endl;
			return 0;
	}

	//search
	cout<<"SEARCH-----------------------------"<<endl;
	pt.wall_tic();
	cusg.run_unrolled();	
	time_in_sec=pt.wall_toc();

	cout<<"[t:"<<time_in_sec<<"]"<<"[w: "<<cusg.get_max_clique()<<"]"<<endl;
	cusg.tear_down();
	cout<<"---------------------------------------------"<<endl;
}