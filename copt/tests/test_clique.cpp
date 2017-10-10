//--------------------------------------------
// some tests for clique


#include "gtest/gtest.h"
#include "graph/graph_gen.h"
#include "utils/common.h"
#include "../clique/clique.h"

using namespace std;

TEST(Clique, basic){
	cout<<"Clique:basic---------------------------------------------"<<endl;
	
	//Ugraph
	ugraph ug(6);
	ug.add_edge(0, 1);
	ug.add_edge(0, 3);
	ug.add_edge(1, 2);
	ug.add_edge(1, 3);
	ug.add_edge(2, 5);
	Clique<ugraph> cug(&ug, clqo::param_t());
	cug.get_param().unrolled=true;
	if(cug.set_up()==0){
		cug.run();
	}
	EXPECT_EQ(3,cug.get_max_clique());
	com::stl::print_collection(cug.decode_first_solution());
	cout<<"--------------------------------------------------------"<<endl;

	
	//Sparse Ugraph
	sparse_ugraph usg(6);
	usg.add_edge(0, 1);
	usg.add_edge(0, 3);
	usg.add_edge(1, 2);
	usg.add_edge(1, 3);
	usg.add_edge(2, 5);
	Clique<sparse_ugraph> cusg(&usg, clqo::param_t());
	cusg.get_param().unrolled=true;
	if(cusg.set_up()==0){
		cusg.run();
	}
	EXPECT_EQ(3,cusg.get_max_clique());
	com::stl::print_collection(cug.decode_first_solution());
	cout<<"--------------------------------------------------------"<<endl;
	
}

TEST(Clique, sort_root_subgraph){
	cout<<"Clique:basic---------------------------------------------"<<endl;
	
	//Ugraph
	ugraph ug(6);
	ug.add_edge(0, 1);
	ug.add_edge(0, 3);
	ug.add_edge(1, 2);
	ug.add_edge(1, 3);
	ug.add_edge(2, 5);

	bitarray bbs(6);
	bbs.set_bit(0,5);
	clqo::search_alloc_t info;
	Clique<ugraph> cug(&ug, clqo::param_t());
	cug.set_up_subgraph(bbs,&clqo::search_alloc_t());
	cug.sort_root_subgraph(bbs, clqo::MIN_WIDTH, gbbs::place_t::PLACE_LF);

	//sorts root
	Clique<ugraph>::nodelist_t n=cug.get_root_nodelist();
	vint sol;
	for(int i=0; i<=n.index; i++){
		sol.push_back(n.nodos[i]);
	}

	//expected sol
	vint v; v.push_back(3);  v.push_back(1);   v.push_back(0);   v.push_back(2); 
	v.push_back(5);  v.push_back(4);  
	

	EXPECT_EQ(v, sol);
	cout<<"--------------------------------------------------------"<<endl;


	
}

TEST(Clique, subgraph_simple){
/////////////////
// shows alternative interace to run the algorithm (currently only for ugraphs)
	cout<<"Clique:subgraph_simple---------------------------------------------"<<endl;
	
	//Ugraph
	const int NV=6;
	ugraph ug(NV);
	ug.add_edge(0,1);
	ug.add_edge(0,2);
	ug.add_edge(1,2);
	ug.add_edge(0,3);
	
	Clique<ugraph> cug(&ug, clqo::param_t() /* only param.alg is used here*/);			
	bitarray bbs(ug.number_of_vertices());

	//nodes [1,2]
	bbs.set_bit(1,2);
	Clique<ugraph>::search_alloc_t info;
	info.size=ug.number_of_vertices();
	info.set(Clique<ugraph>::search_alloc_t::ALLOC_COLOR_SETS);		
	
	if(cug.set_up_subgraph(bbs, &info)==0){				//allocates memory ony the first time
		cug.run_subgraph(bbs);
	}
	EXPECT_EQ(2,cug.get_max_clique());
	com::stl::print_collection(cug.decode_first_solution());
	LOG_INFO("[w:"<<cug.get_max_clique()<<",s:"<<cug.get_result().number_of_steps()<<"]");
	cout<<"-----------------"<<endl;

	//nodes [0...5]
	bbs.erase_bit();
	bbs.set_bit(0,5);
	if(cug.set_up_subgraph(bbs)==0){
		cug.run_subgraph(bbs);
	}
	EXPECT_EQ(3,cug.get_max_clique());
	com::stl::print_collection(cug.decode_first_solution());
	LOG_INFO("[w:"<<cug.get_max_clique()<<",s:"<<cug.get_result().number_of_steps()<<"]");
	cout<<"-----------------"<<endl;

	//nodes [4..5]
	bbs.erase_bit();
	bbs.set_bit(4,5);
	if(cug.set_up_subgraph(bbs)==0){
		cug.run_subgraph(bbs);
	}
	EXPECT_EQ(1,cug.get_max_clique());
	com::stl::print_collection(cug.decode_first_solution());
	LOG_INFO("[w:"<<cug.get_max_clique()<<",s:"<<cug.get_result().number_of_steps()<<"]");
	cout<<"--------------------------------------------------------"<<endl;
}


TEST(Clique, subgraph_brock200_1){
/////////////////
// shows alternative interace to run the algorithm
// verifies clique solution in case of sorting
// first update: 10/11/2016

	cout<<"Clique:subgraph_brock200_1---------------------------------------------"<<endl;
	int FIRST_NODE, LAST_NODE;

	//Ugraph
	ugraph ug("brock200_1.clq");
	clqo::param_t myparam;
	myparam.alg=clqo::BBMCL_R;
	Clique<ugraph> cug(&ug, clqo::param_t());	

//nodes [50...199]
	FIRST_NODE=50; LAST_NODE=199;
	bitarray bbs(ug.number_of_vertices());
	bbs.set_bit(FIRST_NODE, LAST_NODE);
	Clique<ugraph>::search_alloc_t info;
	info.size=ug.number_of_vertices();
	info.set(Clique<ugraph>::search_alloc_t::ALLOC_COLOR_SETS);		
	
	if(cug.set_up_subgraph(bbs, &info)==0){
		cug.run_subgraph(bbs);
	}

	EXPECT_EQ(19,cug.get_max_clique());
	com::stl::print_collection(cug.decode_first_solution()); cout<<endl;
	LOG_INFO("[w:"<<cug.get_max_clique()<<",s:"<<cug.get_result().number_of_steps()<<"]");

	cout<<"-----------------"<<endl;

//nodes [0...49] , no allocation
	FIRST_NODE=0; LAST_NODE=49;
	bbs.erase_bit();
	bbs.set_bit(FIRST_NODE, LAST_NODE);
	if(cug.set_up_subgraph(bbs)==0){		/* no info*/
		cug.run_subgraph(bbs);
	}
	EXPECT_EQ(12,cug.get_max_clique());
	com::stl::print_collection(cug.decode_first_solution()); cout<<endl;
	LOG_INFO("[w:"<<cug.get_max_clique()<<",s:"<<cug.get_result().number_of_steps()<<"]");
	cout<<"-----------------"<<endl;


//nodes [0...49] , with allocation 
	FIRST_NODE=0; LAST_NODE=49;
	bbs.erase_bit();
	bbs.set_bit(FIRST_NODE,LAST_NODE);
	if(cug.set_up_subgraph(bbs,&info)==0){
		cug.run_subgraph(bbs);
	}
	EXPECT_EQ(12,cug.get_max_clique());
	com::stl::print_collection(cug.decode_first_solution()); cout<<endl;
	LOG_INFO("[w:"<<cug.get_max_clique()<<",s:"<<cug.get_result().number_of_steps()<<"]");

//nodes [0...49] , without allocation, sorting (and decoding)
	FIRST_NODE=0, LAST_NODE=49;
	ugraph ug1(ug);
	CliqueSort<ugraph> o(ug);
	o.reorder(o.new_order(clqo::MIN_WIDTH,gbbs::place_t::PLACE_LF), cug.get_decoder());
	bbs.erase_bit();
	bbs.set_bit(FIRST_NODE, LAST_NODE);								//of the sorted graph
	if(cug.set_up_subgraph(bbs)==0){
		cug.run_subgraph(bbs);
	}

	
	EXPECT_EQ(17,cug.get_max_clique());														//size of max clique increments greatly, since higher degrees are placed last

	//checks vertex ranges
	vint sol=cug.get_result().get_first_solution();
	for(int i=0; i<sol.size(); i++){
		EXPECT_TRUE(sol[i]>=FIRST_NODE && sol[i]<=LAST_NODE);
	}

	//verifies clique in the original graph
	Clique<ugraph> cug1 (&ug1, clqo::param_t());
	EXPECT_TRUE(cug1.is_clique(cug.decode_first_solution()));
	com::stl::print_collection(cug.decode_first_solution()); cout<<endl;
	LOG_INFO("[w:"<<cug.get_max_clique()<<",s:"<<cug.get_result().number_of_steps()<<"]");
	
	cout<<"--------------------------------------------------------"<<endl;
}


TEST(Clique, grafo_JA){
//////////////
//  Sparse Ugraph

	cout<<"Clique: grafo_JA-------------------------------"<<endl;
    ugraph ug("grafo.clq");
	Clique<ugraph> cug(&ug,   clqo::param_t());
	cug.get_param().unrolled=true;
	int res=cug.set_up(); 
	cout<<"finished setup"<<endl;

	if(res==0){
		cug.run();
	}
		
	
	vint sol=cug.decode_first_solution();
	EXPECT_EQ(182,cug.get_max_clique());
	EXPECT_EQ(182,sol.size());

	com::stl::print_collection(sol);
		
	cout<<"--------------------------------------------------------"<<endl;
}

TEST(CliqueSparse, grafo_JA){
//////////////
//  Sparse Ugraph

	cout<<"Clique: grafo_JA-------------------------------"<<endl;
    sparse_ugraph ug("grafo.clq");
	Clique<sparse_ugraph> cug(&ug,  clqo::param_t());
	cug.get_param().unrolled=true;
	int res=cug.set_up(); 
	cout<<"finished setup"<<endl;

	if(res==0){
		cug.run();
	}
		
	
	vint sol=cug.decode_first_solution();
	EXPECT_EQ(182,cug.get_max_clique());
	EXPECT_EQ(182,sol.size());

	com::stl::print_collection(sol);
		
	cout<<"--------------------------------------------------------"<<endl;
}


TEST(Clique, grafo2_JA){
//////////////
//  Sparse Ugraph

	cout<<"Clique: grafo_JA-------------------------------"<<endl;
    ugraph ug("grafo2.clq");
	Clique<ugraph> cug(&ug,  clqo::param_t());
	cug.get_param().unrolled=true;
	int res=cug.set_up(); 
	cout<<"finished setup"<<endl;

	if(res==0){
		cug.run();
	}
		
	vint sol=cug.decode_first_solution();
	EXPECT_EQ(19,cug.get_max_clique());
	EXPECT_EQ(19,sol.size());

	com::stl::print_collection(sol);
		
	cout<<"--------------------------------------------------------"<<endl;
}

TEST(CliqueSparse, grafo2_JA){
//////////////
//  Sparse Ugraph

	cout<<"Clique: grafo_JA-------------------------------"<<endl;
    sparse_ugraph ug("grafo2.clq");
	Clique<sparse_ugraph> cug(&ug,  clqo::param_t());
	cug.get_param().unrolled=true;
	int res=cug.set_up(); 
	cout<<"finished setup"<<endl;

	if(res==0){
		cug.run();
	}
		
	
	vint sol=cug.decode_first_solution();
	EXPECT_EQ(19,cug.get_max_clique());
	EXPECT_EQ(19,sol.size());

	com::stl::print_collection(sol);
		
	cout<<"--------------------------------------------------------"<<endl;
}




//TEST(CliqueSparse, brock200_1){
//////////////////
////  Sparse Ugraph
//
//	cout<<"CliqueSparse: brock200_1-------------------------------"<<endl;
//    sparse_ugraph usg("brock200_1.clq");
//	Clique<sparse_ugraph> cusg(&usg, param_t());
//	cusg.get_param().unrolled=true;
//	int res=cusg.set_up(); 
//	cout<<"finished setup"<<endl;
//
//	if(res==0){
//		cusg.run();
//	}
//		
//	EXPECT_EQ(21,cusg.get_max_clique());
//	cout<<"--------------------------------------------------------"<<endl;
//}
//
//
//TEST(CliqueSparse, real_CA_CondMat){
//////////////////
////  Sparse Ugraph
//
//	cout<<"CliqueSparse: real_CA_CondMat----------------------------"<<endl;
//    sparse_ugraph usg("ca-CondMat.mtx");
//	Clique<sparse_ugraph> cusg(&usg, param_t());
//	cusg.get_param().unrolled=true;
//	int res=cusg.set_up(); 
//	cout<<"finished setup"<<endl;
//
//	if(res==0){
//		cusg.run();
//	}
//		
//	EXPECT_EQ(26,cusg.get_max_clique());
//	cout<<"--------------------------------------------------------"<<endl;
//}
//
//TEST(CliqueSparse, real_you_tube_snap){
//////////////////
////  Sparse Ugraph
//
//	cout<<"CliqueSparse: real_you_tube_snap------------------------"<<endl;
//    sparse_ugraph usg("soc-youtube-snap.mtx");
//	Clique<sparse_ugraph> cusg(&usg, param_t());
//	cusg.get_param().unrolled=true;
//	int res=cusg.set_up(); 
//	cout<<"finished setup"<<endl;
//
//	if(res==0){
//		cusg.run();
//	}
//		
//	EXPECT_EQ(17,cusg.get_max_clique());
//	cout<<"--------------------------------------------------------"<<endl;
//}
//
//
//TEST(CliqueSparse, real_tech_as_skitter){
//////////////////
////  Sparse Ugraph
//
//	cout<<"CliqueSparse: real_tech_as_skitter----------------------"<<endl;
//    sparse_ugraph usg("tech-as-skitter.mtx");
//	Clique<sparse_ugraph> cusg(&usg, param_t());
//	cusg.get_param().unrolled=true;
//	int res=cusg.set_up(); 
//	cout<<"finished setup"<<endl;
//
//	if(res==0){
//		cusg.run();
//	}
//		
//	EXPECT_EQ(67,cusg.get_max_clique());
//	cout<<"--------------------------------------------------------"<<endl;
//}
//
//
//TEST(CliqueSparse, fe_rotor){
//////////////////
////  Sparse Ugraph
//
//	cout<<"CliqueSparse: fe_rotor-----------------------------------"<<endl;
//    sparse_ugraph usg("fe_rotor.mtx");
//	Clique<sparse_ugraph> cusg(&usg, param_t());
//	cusg.get_param().unrolled=true;
//	int res=cusg.set_up(); 
//	cout<<"finished setup"<<endl;
//
//	if(res==0){
//		cusg.run();
//	}
//		
//	EXPECT_EQ(5,cusg.get_max_clique());
//	cout<<"--------------------------------------------------------"<<endl;
//}
//
//TEST(CliqueSparse, delaunay_n19){
//////////////////
////  Sparse Ugraph
//
//	cout<<"CliqueSparse: delaunay_n19------------------------------"<<endl;
//    sparse_ugraph usg("delaunay_n20.mtx");
//	usg.print_data();
//	Clique<sparse_ugraph> cusg(&usg, param_t());
//	cusg.get_param().unrolled=true;
//	int res=cusg.set_up(); 
//	cout<<"finished setup"<<endl;
//
//	if(res==0){
//		cusg.run();
//	}
//		
//	EXPECT_EQ(4,cusg.get_max_clique());
//	cout<<"--------------------------------------------------------"<<endl;
//}
//
//
//TEST(CliqueSparse, sc_ldoor){
//////////////////
////  Sparse Ugraph
//
//	cout<<"CliqueSparse: sc_ldoor---------------------------------"<<endl;
//    sparse_ugraph usg("sc-ldoor.mtx");
//	usg.print_data();
//	Clique<sparse_ugraph> cusg(&usg, param_t());
//	cusg.get_param().unrolled=true;
//	int res=cusg.set_up(); 
//	cout<<"finished setup"<<endl;
//
//	if(res==0){
//		cusg.run();
//	}
//		
//	EXPECT_EQ(21,cusg.get_max_clique());
//	cout<<"--------------------------------------------------------"<<endl;
//}
//
//
//TEST(CliqueSparse, hugetrace_0000){
//////////////////
////  Sparse Ugraph
//
//	cout<<"CliqueSparse: hugetrace_0000----------------------------"<<endl;
//    sparse_ugraph usg("hugetrace-00000.mtx");
//	usg.print_data();
//	Clique<sparse_ugraph> cusg(&usg, param_t());
//	cusg.get_param().unrolled=true;
//	int res=cusg.set_up(); 
//	cout<<"finished setup"<<endl;
//
//	if(res==0){
//		cusg.run();
//	}
//		
//	EXPECT_EQ(2,cusg.get_max_clique());
//	cout<<"--------------------------------------------------------"<<endl;
//}
//
//
//
//TEST(CliqueSparse, some_graph){
//////////////////
////  Sparse Ugraph
//
//	cout<<"CliqueSparse:some_graph--------------------------------"<<endl;
//    ugraph usg("auxfilerandom.txt");
//	Clique<ugraph> cusg(&usg, param_t());
//	int res= cusg.set_up();
//	cout<<"finished setup"<<endl;
//
//	if(res==0){
//		cusg.run();
//	}
//		
//	EXPECT_EQ(3,cusg.get_max_clique());
//	cout<<"--------------------------------------------------------"<<endl;
//}
//
//TEST(CliqueSparse, EDGES_format){
//////////////////
////  Sparse Ugraph
//
//	cout<<"CliqueSparse: EDGES_format-------------------------------"<<endl;
//    sparse_ugraph usg("bio-yeast-protein-inter.edges");
//	// sparse_ugraph usg("soc-orkut-dir.edges");
//	Clique<sparse_ugraph> cusg(&usg, param_t());
//	cusg.get_param().unrolled=true;
//	int res= cusg.set_up();
//	cout<<"finished setup unrolled"<<endl;
//	
//	if(res==0){
//		cusg.run();
//	}
//		
//	EXPECT_EQ(6,cusg.get_max_clique());
//	cout<<"--------------------------------------------------------"<<endl;
//}
//
//
//TEST(Clique_Rand, basic){
///////////////////
//// Compares clique number of sparse_ugraph and ugraph for a set of randomly generated graphs
//// author: alopez
////
//// REMARKS: used intermediate file to communicate between both types
//
//    const int TAM_INF=300,TAM_SUP=1000, INC_SIZE=50, REP_MAX=500;
//    const double DEN_INF=.02,DEN_SUP=.2, INC_DENSITY=.01;
//    string path="auxfilerandom.txt";
//
//    for(int tam=TAM_INF;tam<TAM_SUP;tam+=INC_SIZE)  {
//        for(double den=DEN_INF;den<DEN_SUP;den+=INC_DENSITY){
//            for(int rep=0;rep<REP_MAX;rep++){
//                cout<<"--------------------------------------------------------"<<endl;
//                //-------------------------------------------------------------------------
//                //Ugraph
//                ugraph ug;
//                RandomGen<ugraph>::create_ugraph(ug,tam,den);
//                ofstream f(path, std::ofstream::out);   
//                ug.write_dimacs(f);
//                f.close();
//                Clique<ugraph> cug(&ug, param_t());
//                if(cug.set_up()==0);
//							 cug.run();
//    
//                //-------------------------------------------------------------------------
//                //Sparse Ugraph
//                SparseRandomGen<> spgen;    
//                sparse_ugraph usg(path);
//                Clique<sparse_ugraph> cusg(&usg, param_t());
//				cusg.get_param().unrolled=true;
//                if(cusg.set_up()==0)
//							 cusg.run();
//                //remove("auxfilerandom.txt"); 
//                ASSERT_EQ(cug.get_max_clique(),cusg.get_max_clique());
//                cout<<"--------------------------------------------------------"<<tam<<" "<<den<<" "<<rep<<endl;
//            }
//        }
//    }
//}
//
TEST(Clique, Decode_basic){
	cout<<"Clique:Decode basic--------------------------------------------------------"<<endl;
	int sol[]={133, 17, 92, 177, 38, 19, 84, 134, 107, 89, 185, 91, 67, 141, 149, 72, 101, 135, 86, 93, 80};  
	vector<int> expected_sol(sol, sol+21);
	sort(expected_sol.begin(),expected_sol.end());

	//Ugraph
	ugraph ug("brock200_1.clq");
	Clique<ugraph> cug(&ug,  clqo::param_t());		//default param
	if(cug.set_up()==0){
		cug.run();
		EXPECT_EQ(21,cug.get_max_clique());
	}

	//decodes first --and only-- solution
	vector<int> act_sol= cug.decode_first_solution();
	sort(act_sol.begin(),act_sol.end());

	EXPECT_EQ(expected_sol,act_sol);
	
	cout<<"--------------------------------------------------------"<<endl;
	
}