//--------------------------------------------
// test_interdiction_clique.cpp: some tests for the interdiction clique problem (LAMSADE 2016)
// date of creation: 6/11/16
// last update: 8/11/16
// author: pss


#include "gtest/gtest.h"
#include "graph/graph_gen.h"
#include "graph/algorithms/decode.h"
#include "utils/common.h"
#include "utils/logger.h"

#include "../clique/clique.h"
#include "../interfaces/interface_interdiction.h"



using namespace std;

TEST(Interdiction_Clique, simple_node_interdiction){
	LOG_INFO("Interdiction_Clique:simple_node_interdiction-----------------------");
	Interdiction<CliqueInfra>::result_t r;

	//Ugraph
	const int NV=6;
	ugraph ug(NV);
	ug.add_edge(0, 1);		//3-clique
	ug.add_edge(0, 2);
	ug.add_edge(1, 2);
			
	Interdiction<CliqueInfra> ci(&ug);
	ci.init();

	//run as is
	r=ci.run();
	EXPECT_EQ(3, r.w);

	//remove one node
	int* S= new int[1];
	S[0]=1;
	ci.remove_nodes(S,1);
	r=ci.run();
	EXPECT_EQ(2, r.w);
	EXPECT_EQ(0, r.sol[0]);
	EXPECT_EQ(2, r.sol[1]);
	delete [] S;
		
	LOG_INFO("-------------------------------------------------------")	;
}

TEST(Interdiction_Clique, interdict_to_isolani){
	LOG_INFO("Interdiction_Clique: interdict_to_isolani-----------------------");
	Interdiction<CliqueInfra>::result_t r;

	//Ugraph
	const int NV=6;
	ugraph ug(NV);
	ug.add_edge(0, 1);		//star with center 1
	ug.add_edge(1, 2);
	ug.add_edge(1, 3);
			
	Interdiction<CliqueInfra> ci(&ug);
	ci.init();

	//run as is
	r=ci.run();
	EXPECT_EQ(2, r.w);

	//remove one node
	int* S= new int[1];
	S[0]=1;
	ci.remove_nodes(S,1);
	r=ci.run();
	EXPECT_EQ(1, r.w);
	EXPECT_EQ(0, r.sol[0]);
	//EXPECT_EQ(2, r.sol[1]);
	delete [] S;
		
	LOG_INFO("-------------------------------------------------------")	;
}

TEST(Interdiction_Clique, interdict_error_Fabio){
////////////
// Test not working in Fabios application. Here it is passed without problems
	LOG_INFO("Interdiction_Clique:interdict_error_Fabio-----------------------");
	Interdiction<CliqueInfra>::result_t r;

	//ugraph
	const int NV=5;
	ugraph ug(NV);
	ug.add_edge(0, 2);		
	ug.add_edge(1, 3);
	ug.add_edge(1, 4);
	ug.add_edge(2, 4);
					
	Interdiction<CliqueInfra> ci(&ug);
	ci.init();

	//run as is
	r=ci.run();
	EXPECT_EQ(2, r.w);
	
	int* S= new int[5];

	//remove nodes 2,4
	S[0]=2; S[1]=4;  
	ci.remove_nodes(S,2);
	r=ci.run();
	EXPECT_EQ(2, r.w);
	
	//remove nodes 1, 2, 3, 4
	cout<<"REMOVING 1, 2, 3, 4"<<endl;
	S[0]=1; S[1]=2; S[2]=3; S[3]=4;   
	ci.remove_nodes(S,4);
	r=ci.run();
	EXPECT_EQ(1, r.w);
	EXPECT_EQ(0, r.sol[0]);

	cout<<"----------------"<<endl;
	
	//remove all nodes 
	cout<<"REMOVING ALL"<<endl;
	S[0]=0; S[1]=1; S[2]=2; S[3]=3; S[4]=4;    
	ci.remove_nodes(S,5);
	r=ci.run();
	EXPECT_EQ(0, r.w);


	cout<<"----------------"<<endl;

	//remove 1, 2, 4
	cout<<"REMOVING 1, 2, 4"<<endl;
	S[0]=1; S[1]=2; S[2]=4;     
	ci.remove_nodes(S,3);
	r=ci.run();
	EXPECT_EQ(1, r.w);
	
	
	EXPECT_EQ(r.sol.end(), find(r.sol.begin(), r.sol.end(), 1));
	EXPECT_EQ(r.sol.end(), find(r.sol.begin(), r.sol.end(), 2));
	EXPECT_EQ(r.sol.end(), find(r.sol.begin(), r.sol.end(), 4));

	cout<<"----------------"<<endl;

			
	delete [] S;
		
	LOG_INFO("-------------------------------------------------------")	;
}


TEST(Interdiction_Clique, interdict_to_isolani_first_run){
	LOG_INFO("Interdiction_Clique: interdict_to_isolani-----------------------");
	Interdiction<CliqueInfra>::result_t r;

	//Ugraph
	const int NV=6;
	ugraph ug(NV);
	ug.add_edge(0, 1);		//star with center 1
	ug.add_edge(1, 2);
	ug.add_edge(1, 3);
			
	Interdiction<CliqueInfra> ci(&ug);
	ci.init();

	//remove one node
	int* S= new int[1];
	S[0]=1;
	ci.remove_nodes(S,1);
	r=ci.run();
	EXPECT_EQ(1, r.w);
	delete [] S;
		
	LOG_INFO("-------------------------------------------------------")	;
}

TEST(Interdiction_Clique, node_interdiction_brock){
	LOG_INFO("Interdiction_Clique:node_interdiction_brock-----------------------");
	Interdiction<CliqueInfra>::result_t r;

	//Ugraph
	ugraph ug("brock200_1.clq");
	const int NV=ug.number_of_vertices();
	
	//pre-processing: sort
	CliqueSort<ugraph> o(ug);
	o.reorder(o.new_order(clqo::MIN_WIDTH,gbbs::place_t::PLACE_LF));

	//Interdiction-proper			
	Interdiction<CliqueInfra> ci(&ug);
	ci.init();														

	//run as is
	r=ci.run();
	EXPECT_EQ(21, r.w);

	//remove one node
	int* S= new int[1];
	S[0]=190;
	ci.remove_nodes(S,1);
	r=ci.run();
	//r.print();
	EXPECT_EQ(21, r.w);
	delete [] S;

	//check if the solution is a clique 
	Clique<ugraph> clq_test(&ug, clqo::param_t());
	EXPECT_TRUE(clq_test.is_clique(r.sol));
	EXPECT_EQ(r.w, r.sol.size());
		
	//remove 3 nodes
	int* S1= new int[3];
	S1[0]=185; S1[1]=190; S1[2]=134;
	ci.remove_nodes(S1,3);
	r=ci.run();
	//r.print();
	EXPECT_EQ(21, r.w);
	com::stl::print_collection(r.sol);
	delete [] S1;

	//check if the solution is a clique 
	clq_test.set_graph(&ug);
	EXPECT_TRUE(clq_test.is_clique(r.sol));
	EXPECT_EQ(r.w, r.sol.size());

	//remove 100 nodes
	int* S2= new int[100];
	for(int i=0; i<100; i++)
		S2[i]=i;
	ci.remove_nodes(S2,100);
	r=ci.run();
	//r.print();
	EXPECT_EQ(15, r.w);
	delete [] S2;

	//check if the solution is a clique 
	clq_test.set_graph(&ug);
	EXPECT_TRUE(clq_test.is_clique(r.sol));
	EXPECT_EQ(r.w, r.sol.size());

	//remove 190 nodes
	int* S3= new int[190];
	for(int i=0; i<190; i++)
		S3[i]=i;
	ci.remove_nodes(S3,190);
	r=ci.run();
	//r.print();
	EXPECT_EQ(5, r.w);
	delete [] S3;

	//check if the solution is a clique 
	clq_test.set_graph(&ug);
	EXPECT_TRUE(clq_test.is_clique(r.sol));
	EXPECT_EQ(r.w, r.sol.size());

	//remove all nodes
	int* S4= new int[200];
	for(int i=0; i<200; i++)
		S4[i]=i;
	ci.remove_nodes(S4,200);
	r=ci.run();
	//r.print();
	EXPECT_EQ(0, r.w);
	delete [] S4;

	//remove 190 nodes again
	int* S5= new int[190];
	for(int i=0; i<190; i++)
		S5[i]=i;
	ci.remove_nodes(S5,190);
	r=ci.run();
	//r.print();
	EXPECT_EQ(5, r.w);
	delete [] S5;


	LOG_INFO("-------------------------------------------------------")	;
}

TEST(Interdiction_Clique, init_mode_medium){
	LOG_INFO("Interdiction_Clique:init_mode_medium-----------------------");
	Interdiction<CliqueInfra>::result_t r;

	//Ugraph
	const int NV=6;
	ugraph ug(NV);
	ug.add_edge(0, 1);		//3-clique
	ug.add_edge(0, 2);
	ug.add_edge(1, 2);
			
	Interdiction<CliqueInfra> ci(&ug);		//computes init tight bounds by default
	ci.init();

	//run as is
	r=ci.run();
	EXPECT_EQ(3, r.w);

	//remove one node
	int* S= new int[1];
	S[0]=1;
	ci.remove_nodes(S,1);
	r=ci.run();
	EXPECT_EQ(2, r.w);
	EXPECT_EQ(0, r.sol[0]);
	EXPECT_EQ(2, r.sol[1]);
	delete [] S;

	//test for 
	Interdiction<CliqueInfra> ci_medium(&ug);		
	ci_medium.set_init_mode(Interdiction<CliqueInfra>::MEDIUM);						//fast init: initial bounds not tight
	ci_medium.init();

	//run as is
	r=ci_medium.run();
	EXPECT_EQ(3, r.w);

	//remove one node
	int* S1= new int[1];
	S1[0]=1;
	ci_medium.remove_nodes(S1,1);
	r=ci_medium.run();
	EXPECT_EQ(2, r.w);
	EXPECT_EQ(0, r.sol[0]);
	EXPECT_EQ(2, r.sol[1]);
	

	//remove more than one node
	int* S2= new int[2];
	S2[0]=1; 	S2[1]=2;
	ci_medium.remove_nodes(S2,2);
	r=ci_medium.run();
	
	ci_medium.remove_nodes(S1,1);
	r=ci_medium.run();

	delete [] S2;
	delete [] S1;


		
	LOG_INFO("-------------------------------------------------------")	;
}

TEST(Interdiction_Clique, init_mode_trivial){
	LOG_INFO("Interdiction_Clique:init_mode_trivial-----------------------");
	Interdiction<CliqueInfra>::result_t r;

	//Ugraph
	const int NV=6;
	ugraph ug(NV);
	ug.add_edge(0, 1);		//3-clique
	ug.add_edge(0, 2);
	ug.add_edge(1, 2);
			
	Interdiction<CliqueInfra> ci(&ug);		//computes init tight bounds by default
	ci.init();

	//run as is
	r=ci.run();
	EXPECT_EQ(3, r.w);

	//remove one node
	int* S= new int[1];
	S[0]=1;
	ci.remove_nodes(S,1);
	r=ci.run();
	EXPECT_EQ(2, r.w);
	EXPECT_EQ(0, r.sol[0]);
	EXPECT_EQ(2, r.sol[1]);
	delete [] S;

	//test for 
	Interdiction<CliqueInfra> ci_trivial(&ug);		
	ci_trivial.set_init_mode(Interdiction<CliqueInfra>::TRIVIAL);						//fast init: initial bounds not tight
	ci_trivial.init();

	//run as is
	r=ci_trivial.run();
	EXPECT_EQ(3, r.w);

	//remove one node
	int* S1= new int[1];
	S1[0]=1;
	ci_trivial.remove_nodes(S1,1);
	r=ci_trivial.run();
	EXPECT_EQ(2, r.w);
	EXPECT_EQ(0, r.sol[0]);
	EXPECT_EQ(2, r.sol[1]);
	delete [] S1;
		
	LOG_INFO("-------------------------------------------------------")	;
}

TEST(Interdiction_BBMC, simple_node_interdiction){
//////////////
// Testing with Basic BBMC: it seems it works directly
	LOG_INFO("nterdiction_BBMC:simple_node_interdiction-----------------------");
	Interdiction<Clique<ugraph>>::result_t r;

	//Ugraph
	const int NV=6;
	ugraph ug(NV);
	ug.add_edge(0, 1);		//3-clique
	ug.add_edge(0, 2);
	ug.add_edge(1, 2);
			
	Interdiction<Clique<ugraph>> ci(&ug);
	ci.set_alg(Interdiction<Clique<ugraph>>::SIMPLE);
	ci.init();

	//run as is
	r=ci.run();
	EXPECT_EQ(3, r.w);

	//remove one node
	int* S= new int[1];
	S[0]=1;
	ci.remove_nodes(S,1);
	r=ci.run();
	EXPECT_EQ(2, r.w);
	EXPECT_EQ(0, r.sol[0]);
	EXPECT_EQ(2, r.sol[1]);
	delete [] S;
		
	LOG_INFO("-------------------------------------------------------")	;
}

TEST(DISABLED_Interdiction_Clique, r50000_interdiction){
	LOG_INFO("Interdiction_Clique:simple_node_interdiction-----------------------");
	Interdiction<CliqueInfra>::result_t r;
	
	//ugraph-random
	const int NV=1;
	ugraph ug(NV);
	RandomGen<ugraph>::create_ugraph(ug,80000, 0.01); 
	
				
	Interdiction<CliqueInfra> ci(&ug);	
	ci.set_init_mode(Interdiction<CliqueInfra>::TRIVIAL);
	LOG_INFO("Interdiction_Clique-Finished generating the random graph");

	ci.init();
	LOG_INFO("Interdiction_Clique-Finished intitialization of interdiction");

	
	//run as is
	r=ci.run();
	//EXPECT_EQ(3, r.w);

	//remove one node
	int* S= new int[1];
	S[0]=1;
	ci.remove_nodes(S,1);
	r=ci.run();
	/*EXPECT_EQ(2, r.w);
	EXPECT_EQ(0, r.sol[0]);
	EXPECT_EQ(2, r.sol[1]);*/
	delete [] S;
		
	LOG_INFO("-------------------------------------------------------")	;
}

//TEST(Interdiction_BBMC, r50000_interdiction){
//	LOG_INFO("Interdiction_BBMC:simple_node_interdiction-----------------------");
//	Interdiction<Clique<ugraph>>::result_t r;
//
//	//ugraph-random
//	const int NV=1;
//	ugraph ug(NV);
//	RandomGen<ugraph>::create_ugraph(ug,50000, 0.01); 
//	
//				
//	Interdiction<Clique<ugraph>> ci(&ug);
//	ci.set_alg(Interdiction<Clique<ugraph>>::SIMPLE);					//default is PMAX_SAT
//	ci.set_init_mode(Interdiction<Clique<ugraph>>::TRIVIAL);
//	LOG_INFO("Interdiction_BBMC-Finished generating the random graph");
//
//	ci.init();
//	LOG_INFO("Interdiction_BBMC-Finished intitialization of interdiction");
//
//	
//	//run as is
//	r=ci.run();
//	//EXPECT_EQ(3, r.w);
//
//	//remove one node
//	int* S= new int[1];
//	S[0]=1;
//	ci.remove_nodes(S,1);
//	r=ci.run();
//	/*EXPECT_EQ(2, r.w);
//	EXPECT_EQ(0, r.sol[0]);
//	EXPECT_EQ(2, r.sol[1]);*/
//	delete [] S;
//		
//	LOG_INFO("-------------------------------------------------------")	;
//}



