#include <stdio.h>
#include "gtest/gtest.h"
#include "graph/graph_gen.h"	
#include "utils/prec_timer.h"
#include "../clique/clique.h"							
#include "../clique_para/clique_parallel.h"


using namespace std;

//TEST(Clique_parallel_sparse, basic){
//	cout<<"--------------------------------------------------------"<<endl;
//	PrecisionTimer pt;
//	double time_in_sec;
//	int sol;
//
//	
//	//sparse Ugraph
//	sparse_ugraph usg(6);
//	usg.add_edge(0, 1);
//	usg.add_edge(0, 3);
//	usg.add_edge(1, 2);
//	usg.add_edge(1, 3);
//	usg.add_edge(2, 5);
//
//		
//	Clique_parallel<sparse_ugraph> cps(usg);
//	cout<<"Max cores HW: "<<cps.get_max_cores_hw()<<endl;
//	cps.set_cores(cps.get_max_cores_hw());// CAPADO A NUCLEOS 
//
//	cout<<"setup--------------------------"<<endl;
//	if((sol=cps.set_up_unrolled())!=0){
//	  cout<<usg.get_name()<<"-solved trivially-"<<sol<<endl;
//	  return;
//    }
//	
//	cout<<"search-------------------------"<<endl;
// 
//    pt.wall_tic();
//    cps.run_unrolled();	
//    time_in_sec=pt.wall_toc();
//	 
//	cout<<usg.get_name()<<"[t:"<<time_in_sec<<"]"<<"[w: "<<cps.get_max_clique()<<"]"<<endl;
//
//	EXPECT_EQ(3,cps.get_max_clique());
//	cout<<"--------------------------------------------------------"<<endl;
//}

//
TEST(Clique_parallel_sparse, brock_200_1){
////////////////
//  Sparse Ugraph

	cout<<"--------------------------------------------------------"<<endl;
	PrecisionTimer pt;
	double time_in_sec;
	int sol;
	
    sparse_ugraph usg("brock200_1.clq");


	Clique_parallel<sparse_ugraph> cps(usg);
	cout<<"Max cores HW: "<<cps.get_max_cores_hw()<<endl;
	cps.set_cores(cps.get_max_cores_hw());				// CAPADO A NUCLEOS 

	cout<<"setup--------------------------"<<endl;
	if((sol=cps.set_up_unrolled())!=0){
		cout<<usg.get_name()<<"-solved trivially-"<<sol<<endl;
		return;
	}

	cout<<"search-------------------------"<<endl;

	pt.wall_tic();
	cps.run_unrolled();	
	time_in_sec=pt.wall_toc();

	cout<<usg.get_name()<<"[t:"<<time_in_sec<<"]"<<"[w: "<<cps.get_max_clique()<<"]"<<endl;

	EXPECT_EQ(21,cps.get_max_clique());
	cout<<"--------------------------------------------------------"<<endl;
}

/*TEST(Clique_sparse, delaunay_n24){
////////////////
//  Sparse Ugraph

	cout<<"--------------------------------------------------------"<<endl;
    sparse_ugraph usg("delaunay_n24.mtx");
	usg.print_data();
	Clique_parallel<sparse_ugraph> cps(usg);
	cout<<"Max cores HW: "<<cps.get_max_cores_hw()<<endl;
	cps.set_cores(7); // CAPADO A NUCLEOS 
	int res= cps.set_up_unrolled(); // Set up sequentialy
	cout<<"finished setup unrolled"<<endl;
	if(res==0){
		cps.run_unrolled();// Run parallel
	}
		
	EXPECT_EQ(4,cps.get_max_clique());
	cout<<"--------------------------------------------------------"<<endl;
}*/
/*TEST(Clique_sparse, socfbAanon){
////////////////
//  Sparse Ugraph

	cout<<"--------------------------------------------------------"<<endl;
    sparse_ugraph usg("socfb-A-anon.mtx");
	usg.print_data();
	Clique_parallel<sparse_ugraph> cps(usg);
	cout<<"Max cores HW: "<<cps.get_max_cores_hw()<<endl;
	cps.set_cores(3); // CAPADO A NUCLEOS 
	int res= cps.set_up_unrolled(); // Set up sequentialy
	cout<<"finished setup unrolled"<<endl;
	if(res==0){
		cps.run_unrolled();// Run parallel
	}
		
	EXPECT_EQ(4,cps.get_max_clique());
	cout<<"--------------------------------------------------------"<<endl;
}
TEST(Clique_sparse, socfbBanon){
////////////////
//  Sparse Ugraph

	cout<<"--------------------------------------------------------"<<endl;
    sparse_ugraph usg("socfb-B-anon.mtx");
	usg.print_data();
	Clique_parallel<sparse_ugraph> cps(usg);
	cout<<"Max cores HW: "<<cps.get_max_cores_hw()<<endl;
	cps.set_cores(3); // CAPADO A NUCLEOS 
	int res= cps.set_up_unrolled(); // Set up sequentialy
	cout<<"finished setup unrolled"<<endl;
	if(res==0){
		cps.run_unrolled();// Run parallel
	}
		
	EXPECT_EQ(4,cps.get_max_clique());
	cout<<"--------------------------------------------------------"<<endl;
}*/

/*TEST(Clique_sparse, soc_pokec){
////////////////
//  Sparse Ugraph

	cout<<"--------------------------------------------------------"<<endl;
    sparse_ugraph usg("soc-pokec.mtx");
	usg.print_data();
	Clique_parallel<sparse_ugraph> cps(usg);
	cout<<"Max cores HW: "<<cps.get_max_cores_hw()<<endl;
	cps.set_cores(7); // CAPADO A NUCLEOS 
	int res= cps.set_up_unrolled(); // Set up sequentialy
	cout<<"finished setup unrolled"<<endl;
	if(res==0){
		cps.run_unrolled();// Run parallel
	}
		
	EXPECT_EQ(4,cps.get_max_clique());
	cout<<"--------------------------------------------------------"<<endl;
}*/

/*	TEST(Clique_sparse, socfbuciuni){
////////////////
//  Sparse Ugraph

	cout<<"--------------------------------------------------------"<<endl;
    sparse_ugraph usg("socfb-uci-uni.mtx");
	usg.print_data();
	Clique_parallel<sparse_ugraph> cps(usg);
	cout<<"Max cores HW: "<<cps.get_max_cores_hw()<<endl;
	cps.set_cores(7); // CAPADO A NUCLEOS 
	int res= cps.set_up_unrolled(); // Set up sequentialy
	cout<<"finished setup unrolled"<<endl;
	if(res==0){
		cps.run_unrolled();// Run parallel
	}
		
	EXPECT_EQ(4,cps.get_max_clique());
	cout<<"--------------------------------------------------------"<<endl;
}*/

//TEST(Clique_sparse, EDGES_format){
//////////////////
////  Sparse Ugraph
//
//	cout<<"--------------------------------------------------------"<<endl;
//    sparse_ugraph usg("bio-yeast-protein-inter.edges");
//	usg.print_data();
//	Clique_parallel<sparse_ugraph> cps(usg);
//	cout<<"Max cores HW: "<<cps.get_max_cores_hw()<<endl;
//	cps.set_cores(3); // CAPADO A NUCLEOS 
//	int res= cps.set_up_unrolled(); // Set up sequentialy
//	cout<<"finished setup unrolled"<<endl;
//	if(res==0){
//		cps.run_unrolled();// Run parallel
//	}
//		
//	EXPECT_EQ(6,cps.get_max_clique());
//	cout<<"--------------------------------------------------------"<<endl;
//}

//TEST(Clique_Rand_Parallel, basic){
///////////////////
//// Compares clique number of sparse_ugraph and ugraph for a set of randomly gennerated graphs
//// author: alopez
////
//// REMARKS: used intermediate file to communicate between both types
//
//    const int TAM_INF=500,TAM_SUP=1000, INC_SIZE=50, REP_MAX=500;
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
//                cout<<"END NON SPARSE-------------------------------------------------------------------------"<<endl;
//                //Sparse Ugraph
//                SparseRandomGen<> spgen;    
//                sparse_ugraph usg(path);
//                Clique_parallel<sparse_ugraph> cps(usg);
//				cout<<"Max cores HW: "<<cps.get_max_cores_hw()<<endl;
//				cps.set_cores(cps.get_max_cores_hw());
//                if(cps.set_up_unrolled()==0)
//							 cps.run_unrolled();
//                //remove("auxfilerandom.txt"); //Puede quitarse porque machaca el archivo
//                ASSERT_EQ(cug.get_max_clique(),cps.get_max_clique());
//                cout<<"--------------------------------------------------------"<<tam<<" "<<den<<" "<<rep<<endl;
//            }
//        }
//    }
//}

