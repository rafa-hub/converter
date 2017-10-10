//--------------------------------------------
// test_interdiction_plus_clique.cpp: some tests for the enhanced Interdiction Interface InterdictionPlus
// Note: work done in cooperation with LAMSADE (Université Dauphine, Paris).
//									 
// date of creation: 01/10/17
// author: pss

#include "gtest/gtest.h"
#include "graph/graph_gen.h"
#include "graph/algorithms/decode.h"
#include "utils/common.h"
#include "utils/logger.h"
#include "utils/file.h"

#include "../clique/clique.h"
#include "../interfaces/interface_interdiction.h"
#include "../interfaces/interface_interdiction_plus.h"

#define TEST_PATH_BIG_GRAPHS "C:/Users/Pablo/Desktop/bigGraphs/"

using namespace std;



TEST(Interdiction_Plus_Clique, simple_comparison){

	LOG_INFO("Interdiction_Plus_Clique:simple_comparison-----------------------");
	InterdictionPlus::result_t rnew;
	Interdiction<Clique<ugraph>>::result_t rold;  /* configured to run PMAX_SAT BBMCRL-KMIN algorithm */

	//Ugraph
	ugraph ug("c:/users/pablo/desktop/test_dimacs.txt");
	///RandomGen<ugraph>::create_ugraph(ug,20,0.9);
	const int NV=ug.number_of_vertices();
	ug.print_data();
	ug.print_edges();
	
	InterdictionPlus ci(&ug);
	rnew=ci.init();
	com::stl::print_collection(rnew.sol); cout<<endl;
	cin.get();

	com::stl::print_collection(ci.get_map().get_l2r());
	cin.get();

	/*Interdiction<Clique<ugraph>> cold(&ug);
	cold.init();
	rold=cold.run(); 
	EXPECT_EQ(rnew.w, rold.w); */
	
	//	for(int K=1; K<3; K++){
	int* S= new int[4];
	int v=0;
	//	for(int j=0; j<NV-3; j++){

	S[0]=0; S[1]=7; S[2]=2; S[3]=4;
	cout<<"interdicting: "<<S[0]<<":"<<S[1]<<":"<<S[2]<<":"<<S[3]<<endl;

	ci.remove_nodes(S,4);
	rnew=ci.run();
	com::stl::print_collection(rnew.sol); cout<<endl;
	EXPECT_TRUE(gfunc::is_clique_edge_based(ug,rnew.sol));
	cin.get();

	/*cold.remove_nodes(S,3);
	rold=cold.run();
	com::stl::print_collection(rold.sol); cout<<endl;
	EXPECT_TRUE(gfunc::is_clique_edge_based(ug,rold.sol));*/

	EXPECT_EQ(rnew.w, rold.w); 
	cin.get();
	//	}

	/*cold.remove_nodes(S,1);
	rold=cold.run();*/

	//cout<<"NEW:";
	//com::stl::print_collection(rnew.sol); cout<<endl;
	//EXPECT_EQ(8, rnew.w);

	/*cout<<"OLD:";
	com::stl::print_collection(rold.sol); cout<<endl;*/

	//EXPECT_EQ(rnew.w, rold.w); /* FAILS IN RELEASE MODE-FIRST INTERDICTION */
	delete [] S;

	//}
	cin.get();	
	
	LOG_INFO("-------------------------------------------------------")	;
}


TEST(Interdiction_Plus_Clique, comparison_Interdiction_Clique_brock){

	LOG_INFO("Interdiction_Plus_Clique:comparison_Interdiction_Clique_brock-----------------------");
	InterdictionPlus::result_t rnew;
	Interdiction<Clique<ugraph>>::result_t rold;

	
	ugraph ug("brock200_1.clq");				
	const int NV=ug.number_of_vertices();
	ug.print_data();
					

	InterdictionPlus ci(&ug);
	rnew=ci.init();

	Interdiction<Clique<ugraph>> cold(&ug);
	cold.set_init_mode(Interdiction<Clique<ugraph>>::TRIVIAL);
	cold.init();
	cold.run();

	const int REM_V=40;					/* number of nodes removed */
	int* S= new int[REM_V];

	for(int v=0; v<NV-REM_V; v++){
		for(int i=0; i<REM_V; i++) 	S[i]=i; 
			
		ci.remove_nodes(S,REM_V);
		rnew=ci.run();
		cold.remove_nodes(S,REM_V);
		rold=cold.run();

		//check if the solution is a clique 
		EXPECT_TRUE(gfunc::is_clique_edge_based(ug,rnew.sol));
		EXPECT_TRUE(gfunc::is_clique_edge_based(ug,rold.sol));
		EXPECT_EQ(rold.w, rold.sol.size());
		EXPECT_EQ(rnew.w, rnew.sol.size());

		cout<<"NEW:";
		com::stl::print_collection(rnew.sol); cout<<endl;

		cout<<"OLD:";
		com::stl::print_collection(rold.sol); cout<<endl;

		if(rnew.w!=rold.w){
			ug.write_dimacs(ofstream("myg.txt",ios::out));
			ug.print_data();
			cout<<"vertex removed: "<<v;
			cin.get();
			//return;
		}

		EXPECT_EQ(rnew.w, rold.w);
	}	

	delete [] S;

	LOG_INFO("-------------------------------------------------------")	;
}

TEST(Interdiction_Plus_Clique, comparison_Interdiction_Clique_random){

	LOG_INFO("Interdiction_Plus_Clique:comparison_Interdiction_Clique_random-----------------------");
	InterdictionPlus::result_t rnew;
	Interdiction<Clique<ugraph>>::result_t rold;
				
	string name;
	stringstream sstr;
	random_attr_t r(10, 30, 0.1, 0.9, 100, 1, 0.1);

	for(int tam=r.nLB;tam<=r.nUB;tam+=r.incN) {
		for(double den=r.pLB;den<=r.pUB;den+=r.incP){
			name.clear();
			name="r";
			sstr.str("");
			sstr<<tam<<"_"<<setprecision(4)<<den;
			name+=sstr.str();

			for(int rep=0;rep<r.nRep;rep++){
				ugraph ug;
				RandomGen<ugraph>::create_ugraph(ug,tam,den);
				sstr.str(""); sstr<<rep;
				ug.set_name(name+"_"+sstr.str()+".txt");		//change name appropiately

				InterdictionPlus ci(&ug);
				rnew=ci.init();

				Interdiction<Clique<ugraph>> cold(&ug);
				cold.set_init_mode(Interdiction<Clique<ugraph>>::TRIVIAL);
				cold.init();
				cold.run();

				int* S= new int[1];

				for(int v=0; v<tam; v++){
					S[0]=v;
					ci.remove_nodes(S,1);
					rnew=ci.run();
					cold.remove_nodes(S,1);
					rold=cold.run();

					//check if the solution is a clique 
					EXPECT_TRUE(gfunc::is_clique_edge_based(ug,rnew.sol));
					EXPECT_TRUE(gfunc::is_clique_edge_based(ug,rold.sol));
					EXPECT_EQ(rold.w, rold.sol.size());
					EXPECT_EQ(rnew.w, rnew.sol.size());
					
					/*cout<<"NEW:";
					com::stl::print_collection(rnew.sol); cout<<endl;

					cout<<"OLD:";
					com::stl::print_collection(rold.sol); cout<<endl;*/

					if(rnew.w!=rold.w){
						ug.write_dimacs(ofstream("myg.txt",ios::out));
						ug.print_data();
						cout<<"vertex removed: "<<v;
						cin.get();
						//return;
					}
					
					EXPECT_EQ(rnew.w, rold.w);
				}	

				delete [] S;

			}
		}
	}
	LOG_INFO("-------------------------------------------------------")	;
}

TEST(Interdiction_Plus_Clique, node_interdiction_brock){

	LOG_INFO("Interdiction_Plus_Clique:node_interdiction_brock-----------------------");
	InterdictionPlus::result_t r;

	//Ugraph
	ugraph ug("brock200_1.clq");				
	const int NV=ug.number_of_vertices();
	ug.print_data();
				
	InterdictionPlus ci(&ug);
	r=ci.init();	
	EXPECT_EQ(21, r.w);
	

	//remove one node
	int* S= new int[1];
	S[0]=190;
	ci.remove_nodes(S,1);
	r=ci.run();
//	r.print();
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
//	r.print();
	EXPECT_EQ(20, r.w);		
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
	EXPECT_EQ(17, r.w);
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
	r.print();
	EXPECT_EQ(6, r.w);
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
	EXPECT_EQ(6, r.w);
	delete [] S5;

	//init again
	r=ci.init();	
	EXPECT_EQ(21, r.w);
	r.print();	
	
	LOG_INFO("-------------------------------------------------------")	;
}


TEST(Interdiction_Plus_Clique, big_graphs_cond_mat){
////////////////
// CHECK: the interdicting results are not the same than in a similar test with previous
//		   variant of Interdiction_Clique
// 
// *TODO: complete the tests

	LOG_INFO("Interdiction_Plus_Clique:big_graphs_cond_mat-----------------------");
	InterdictionPlus::result_t r;

	//Ugraph
	string filename(TEST_PATH_BIG_GRAPHS);
	filename+="cond-mat-2003.clq";

	ugraph ug(filename);				
	const int NV=ug.number_of_vertices();
	ug.print_data();
				
	InterdictionPlus ci(&ug);
	r=ci.init();		
	
	//remove 29000 nodes
	int* S= new int[29000];
	for(int i=0; i<29000; i++){
			S[i]=i;
	}
	ci.remove_nodes(S,29000);
	r=ci.run();
	com::stl::print_collection(r.sol); cout<<endl;
	delete [] S;

	//check if the solution is a clique 
	EXPECT_TRUE(gfunc::is_clique_edge_based(ug,r.sol));
	EXPECT_EQ(r.w, r.sol.size());


	//remove 31000 nodes
	int* S1= new int[31000];
	for(int i=0; i<31000; i++){
			S1[i]=i;
	}
	ci.remove_nodes(S1,31000);
	r=ci.run();
	com::stl::print_collection(r.sol); cout<<endl;
	delete [] S1;

	//check if the solution is a clique 
	EXPECT_TRUE(gfunc::is_clique_edge_based(ug,r.sol));
	EXPECT_EQ(r.w, r.sol.size());

	//remove all except-2
	int* S2= new int[NV-2];
	for(int i=0; i<NV-2; i++){
			S2[i]=i;
	}
	ci.remove_nodes(S2,NV-2);
	r=ci.run();	
	com::stl::print_collection(r.sol);cout<<endl;
	EXPECT_GT(3, r.w);
	delete [] S2;
			
	LOG_INFO("-------------------------------------------------------")	;
}


TEST(Interdiction_Plus_Clique, big_graphs_fe_body){
////////////////
// CHECK: the interdicting results are not the same than in a similar test with previous
//		   variant of Interdiction_Clique
// 
// *TODO: complete the tests

	LOG_INFO("Interdiction_Plus_Clique:big_graphs_fe_body-----------------------");
	InterdictionPlus::result_t r;

	//Ugraph
	string filename(TEST_PATH_BIG_GRAPHS);
	filename+="fe-body.mtx.clq";

	ugraph ug(filename);				
	const int NV=ug.number_of_vertices();
	ug.print_data();
				
	InterdictionPlus ci(&ug);
	r=ci.init();	
	//EXPECT_EQ(21, r.w);
	
	//remove 29000 node
	int* S= new int[29000];
	for(int i=0; i<29000; i++){
			S[i]=i;
	}
	ci.remove_nodes(S,29000);
	r=ci.run();
	com::stl::print_collection(r.sol); cout<<endl;
	//EXPECT_EQ(21, r.w);
	delete [] S;

	//check if the solution is a clique 
	EXPECT_TRUE(gfunc::is_clique_edge_based(ug,r.sol));
	EXPECT_EQ(r.w, r.sol.size());
	
	//remove 31000 node
	int* S1= new int[31000];
	for(int i=0; i<31000; i++){
			S1[i]=i;
	}
	ci.remove_nodes(S1,31000);
	r=ci.run();
	com::stl::print_collection(r.sol); cout<<endl;
	//EXPECT_EQ(21, r.w);
	delete [] S1;

	//check if the solution is a clique 
	EXPECT_TRUE(gfunc::is_clique_edge_based(ug,r.sol));
	EXPECT_EQ(r.w, r.sol.size());

	//remove all except-2
	int* S2= new int[NV-2];
	for(int i=0; i<NV-2; i++){
			S2[i]=i;
	}
	ci.remove_nodes(S2,NV-2);
	r=ci.run();
	com::stl::print_collection(r.sol); cout<<endl;
	EXPECT_GT(3, r.w);
	delete [] S2;
			
	LOG_INFO("-------------------------------------------------------")	;
}

TEST(Interdiction_Plus_Clique, simple_node_interdiction){
	LOG_INFO("Interdiction_Plus_Clique:simple_node_interdiction-----------------------");
	InterdictionPlus::result_t r;

	//Ugraph
	const int NV=6;
	ugraph ug(NV);
	ug.add_edge(0, 1);							//3-clique
	ug.add_edge(0, 2);
	ug.add_edge(1, 2);
			
	InterdictionPlus ci(&ug);
	r=ci.init();
	EXPECT_EQ(3, r.w);
		
	//remove one node
	int* S= new int[1];
	S[0]=1;
	ci.remove_nodes(S,1);
	r=ci.run();
	EXPECT_EQ(2, r.w);
	EXPECT_EQ(2, r.sol[0]);
	EXPECT_EQ(0, r.sol[1]);

	//remove two nodes
	delete [] S;
	S= new int[2];
	S[0]=1; S[1]=0;
	ci.remove_nodes(S,2);
	r=ci.run();
	EXPECT_EQ(1, r.w);
	EXPECT_EQ(2, r.sol[0]);


	delete [] S;
	LOG_INFO("-------------------------------------------------------")	;
}

TEST(Interdiction_Plus_Clique, interdict_to_isolani){
	LOG_INFO("Interdiction_Plus_Clique: interdict_to_isolani-----------------------");
	InterdictionPlus::result_t r;

	//Ugraph
	const int NV=6;
	ugraph ug(NV);
	ug.add_edge(0, 1);		//star with center 1
	ug.add_edge(1, 2);
	ug.add_edge(1, 3);
			
	InterdictionPlus ci(&ug);
	r=ci.init();
	EXPECT_EQ(2, r.w);

	//remove one node
	int* S= new int[1];
	S[0]=1;
	ci.remove_nodes(S,1);
	r=ci.run();
	EXPECT_EQ(1, r.w);
	EXPECT_EQ(3, r.sol[0]);

	delete [] S;	
	LOG_INFO("-------------------------------------------------------")	;
}







//TEST(Interdiction_Clique, interdict_error_Fabio){
//////////////
//// Test not working in Fabios application. Here it is passed without problems
//	LOG_INFO("Interdiction_Clique:interdict_error_Fabio-----------------------");
//	Interdiction<CliqueInfra>::result_t r;
//
//	//ugraph
//	const int NV=5;
//	ugraph ug(NV);
//	ug.add_edge(0, 2);		
//	ug.add_edge(1, 3);
//	ug.add_edge(1, 4);
//	ug.add_edge(2, 4);
//					
//	Interdiction<CliqueInfra> ci(&ug);
//	ci.init();
//
//	//run as is
//	r=ci.run();
//	EXPECT_EQ(2, r.w);
//	
//	int* S= new int[5];
//
//	//remove nodes 2,4
//	S[0]=2; S[1]=4;  
//	ci.remove_nodes(S,2);
//	r=ci.run();
//	EXPECT_EQ(2, r.w);
//	
//	//remove nodes 1, 2, 3, 4
//	cout<<"REMOVING 1, 2, 3, 4"<<endl;
//	S[0]=1; S[1]=2; S[2]=3; S[3]=4;   
//	ci.remove_nodes(S,4);
//	r=ci.run();
//	EXPECT_EQ(1, r.w);
//	EXPECT_EQ(0, r.sol[0]);
//
//	cout<<"----------------"<<endl;
//	
//	//remove all nodes 
//	cout<<"REMOVING ALL"<<endl;
//	S[0]=0; S[1]=1; S[2]=2; S[3]=3; S[4]=4;    
//	ci.remove_nodes(S,5);
//	r=ci.run();
//	EXPECT_EQ(0, r.w);
//
//
//	cout<<"----------------"<<endl;
//
//	//remove 1, 2, 4
//	cout<<"REMOVING 1, 2, 4"<<endl;
//	S[0]=1; S[1]=2; S[2]=4;     
//	ci.remove_nodes(S,3);
//	r=ci.run();
//	EXPECT_EQ(1, r.w);
//	
//	
//	EXPECT_EQ(r.sol.end(), find(r.sol.begin(), r.sol.end(), 1));
//	EXPECT_EQ(r.sol.end(), find(r.sol.begin(), r.sol.end(), 2));
//	EXPECT_EQ(r.sol.end(), find(r.sol.begin(), r.sol.end(), 4));
//
//	cout<<"----------------"<<endl;
//
//			
//	delete [] S;
//		
//	LOG_INFO("-------------------------------------------------------")	;
//}
//
//
//TEST(Interdiction_Clique, interdict_to_isolani_first_run){
//	LOG_INFO("Interdiction_Clique: interdict_to_isolani-----------------------");
//	Interdiction<CliqueInfra>::result_t r;
//
//	//Ugraph
//	const int NV=6;
//	ugraph ug(NV);
//	ug.add_edge(0, 1);		//star with center 1
//	ug.add_edge(1, 2);
//	ug.add_edge(1, 3);
//			
//	Interdiction<CliqueInfra> ci(&ug);
//	ci.init();
//
//	//remove one node
//	int* S= new int[1];
//	S[0]=1;
//	ci.remove_nodes(S,1);
//	r=ci.run();
//	EXPECT_EQ(1, r.w);
//	delete [] S;
//		
//	LOG_INFO("-------------------------------------------------------")	;
//}
//
