//test_clique_para.cpp: tests for  CliquePara class

#include <stdio.h>
#include "gtest/gtest.h"
#include "graph/graph_gen.h"	
#include "utils/prec_timer.h"	
#include "../clique_para/clique_para.h"
#include "../clique_para/clique_para_iter.h"


using namespace std;

TEST(Clique_para, basic){
	cout<<"--------------------------------------------------------"<<endl;
	
	PrecisionTimer pt;
	double time_in_sec;
	int sol;
	
	ugraph ug(6);
	ug.add_edge(0, 1);
	ug.add_edge(0, 3);
	ug.add_edge(1, 2);
	ug.add_edge(1, 3);
	ug.add_edge(2, 5);

	clqo::param_t myparam;
	myparam.unrolled=true;
	int cpucores=CliquePara<ugraph>::get_max_cores_hw();
	cout<<"Max cores HW: "<<cpucores<<endl;
	myparam.nThreads=cpucores-1;

	CliquePara<ugraph> cp(&ug, myparam);
		
		
	cout<<"setup--------------------------"<<endl;
	if((sol=cp.set_up())!=0){
		cout<<ug.get_name()<<"-solved trivially-"<<sol<<endl;
		
	}else{

		cout<<"search-------------------------"<<endl;
		cp.run();	
		EXPECT_EQ(3,cp.get_max_clique());
	}
	
	//deallocation
	cp.tear_down();
	cout<<"--------------------------------------------------------"<<endl;
}

//TEST(Clique_para, brock400_1){
//	cout<<"--------------------------------------------------------"<<endl;
//	
//	PrecisionTimer pt;
//	double time_in_sec;
//	int sol;
//
//	ugraph ug("brock400_1.clq");
//	ug.print_data();
//
//	param_t myparam;
//	myparam.unrolled=true;
//	int cpucores=CliquePara<ugraph>::get_max_cores_hw();
//	cout<<"Max cores HW: "<<cpucores<<endl;
//	myparam.nThreads=cpucores-1;
//
//	CliquePara<ugraph> cp(&ug, myparam);
//					
//	cout<<"setup--------------------------"<<endl;
//	if((sol=cp.set_up())!=0){
//		cout<<ug.get_name()<<"-solved trivially-"<<sol<<endl;
//	}else{
//		cout<<"search-------------------------"<<endl;
//		cp.run();	
//		EXPECT_EQ(27,cp.get_max_clique());
//	}
//
//	//deallocation
//	cp.tear_down();		
//	cout<<"--------------------------------------------------------"<<endl;
//}

TEST(Clique_para_iter, brock400_1){
		cout<<"--------------------------------------------------------"<<endl;
	
	PrecisionTimer pt;
	double time_in_sec;
	int sol;

	ugraph ug("brock200_1.clq");
	ug.print_data();

	clqo::param_t myparam;
	myparam.unrolled=true;
	int cpucores=CliquePara<ugraph>::get_max_cores_hw();
	cout<<"Max cores HW: "<<cpucores<<endl;
	myparam.nThreads=cpucores-1;
	myparam.alg=clqo::BBMCITXR_L;

	CliqueParaIter cp(&ug, myparam);
					
	cout<<"setup--------------------------"<<endl;
	if((sol=cp.set_up())!=0){
		cout<<ug.get_name()<<"-solved trivially-"<<sol<<endl;
	}else{
		cout<<"search-------------------------"<<endl;
		cp.run();	
		EXPECT_EQ(21,cp.get_max_clique());
	}

	//deallocation
	cp.tear_down();		
	cout<<"--------------------------------------------------------"<<endl;
}
