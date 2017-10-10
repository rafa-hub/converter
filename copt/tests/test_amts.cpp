//--------------------------------------------
// some tests for AMTS algorithm for maximum clique

#include "gtest/gtest.h"
//#include "graph/graph_gen.h"
#include "utils/common.h"
#include "../amts/amts_exec.h"

using namespace std;

TEST(AMTS, basic){
	cout<<"AMTS:basic--------------------------------"<<endl;
	ugraph ug("brock400_4.clq");
	
	//default params
	/*AMTSexec myAMTS;
	int lb=myAMTS.run(ug);
	EXPECT_EQ(21,lb);*/

	
	//harder params
	int len_time=20;
	int len_improve=50000;
	int ub=33;					//wrong upper bound

	AMTSexec myAMTS2(len_time,len_improve,1,ub);
	int lb2=myAMTS2.run(ug);
	EXPECT_EQ(33,lb2);

	cout<<"------------------------------------------------------"<<endl;
}

TEST(AMTS, sol1){
	cout<<"AMTS:sol1--------------------------------"<<endl;
	ugraph ug("brock200_1.clq");

	//ugraph ug(3);
	//ug.add_edge(0,1);
	//ug.add_edge(1,2);
	//ug.add_edge(0,2);
	
	

	//harder params
	int len_time=20;
	int len_improve=50000;
	
	AMTSexec myAMTS2(len_time,len_improve);
	int lb=myAMTS2.run(ug, true);
	vint sol=myAMTS2.get_nodes();
	EXPECT_EQ(lb,sol.size());

	//verify clique
	for(int i=0; i<lb-1; i++){
		for(int j=i+1; j<lb; j++){
			EXPECT_TRUE(ug.is_edge(sol[i],sol[j]));
		}
	}
	cout<<"------------------------------------------------------"<<endl;
}

TEST(AMTS, sol2){
	cout<<"AMTS:sol2--------------------------------"<<endl;
	ugraph ug("brock400_4.clq");

	//ugraph ug(3);
	//ug.add_edge(0,1);
	//ug.add_edge(1,2);
	//ug.add_edge(0,2);
	
	

	//harder params
	int len_time=20;
	int len_improve=50000;
	
	AMTSexec myAMTS2(len_time,len_improve);
	int lb=myAMTS2.run(ug, true);
	vint sol=myAMTS2.get_nodes();
	EXPECT_EQ(lb,sol.size());

	//verify clique
	for(int i=0; i<lb-1; i++){
		for(int j=i+1; j<lb; j++){
			EXPECT_TRUE(ug.is_edge(sol[i],sol[j]));
		}
	}
	cout<<"------------------------------------------------------"<<endl;
}


TEST(AMTS, pool){
	cout<<"AMTS:pool_of_solutions--------------------------------"<<endl;
	ugraph ug("brock400_4.clq");
	
	//default params
	/*AMTSexec myAMTS;
	int lb=myAMTS.run(ug);
	EXPECT_EQ(21,lb);*/

	
	//harder params
	const int NSOL=10;
	int len_time=20;
	int len_improve=1000;
	
	AMTSexec myAMTS2(len_time,len_improve,1,-1);
	Result r=myAMTS2.pool_of_solutions(ug, NSOL);
	vector<vint> sol=r.get_all_solutions();
	for(int i=0; i<NSOL; i++){
		com::stl::print_collection(sol[i]); cout<<"["<<sol[i].size()<<"]"<<endl;
	}

	EXPECT_EQ(NSOL,r.get_number_of_solutions());

	

	cout<<"------------------------------------------------------"<<endl;
}



