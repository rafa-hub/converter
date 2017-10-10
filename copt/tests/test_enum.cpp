//test_enum.cpp:tests for maximal clique enumeration
// date of creation: 11/12/15
// last update:
// author: pss

//CHECK: IN UNROLLED SPARSE TESTS THE INITIAL ORDER IS FIXED! 17/1/17

#include "gtest/gtest.h"
#include "graph/graph_gen.h"
#include "graph/algorithms/decode.h"
#include "utils/common.h"
#include "../clique/clique_enum.h"
#include "../clique/clique_enum_sparse.h"
#include "utils/file.h"

#include <algorithm>

using namespace std;

TEST(Enum, basic_enum){
	cout<<"Enum: basic_enum------------------------"<<endl;
	int NV=5;
	ugraph ug(NV);
	ug.add_edge(0,1);
	ug.add_edge(0,2);
	ug.add_edge(1,2);
	ug.add_edge(1,3);
	ug.add_edge(2,4);
	
	//setup for maximal clique enumeration
	clqo::param_t p;
	p.alg=clqo::BBMC_EN_GCAND;
	p.init_order=clqo::MAX_WIDTH;				//gives minimum branches
	p.unrolled=false;
	CliqueEnum cug(&ug, p);
	if(cug.set_up()!=-1){
        cug.run();
    }
	cug.tear_down();

	//tests number of maximal cliques and steps (in this case optimal)
	EXPECT_EQ(3,cug.get_result().get_upper_bound());									
	EXPECT_EQ(3,cug.get_result().number_of_steps());		
	cout<<"--------------------------------------------"<<endl;
}

TEST(Enum, complete_graph){
	cout<<"Enum: complete_graph------------------------"<<endl;
	int NV=4;
	ugraph ug(NV);
	ug.add_edge(0,1);
	ug.add_edge(0,2);
	ug.add_edge(0,3);
	ug.add_edge(1,2);
	ug.add_edge(1,3);
	ug.add_edge(2,3);
	
	//setup for maximal clique enumeration
	clqo::param_t p;
	p.alg=clqo::BBMC_EN_GCONF;
	p.init_order=clqo::MAX_WIDTH;				//gives minimum branches
	p.unrolled=false;
	CliqueEnum cug(&ug, p);
	if(cug.set_up()!=-1){
        cug.run();
    }
	cug.tear_down();

	//tests number of maximal cliques and steps (in this case optimal)
	EXPECT_EQ(1,cug.get_result().get_upper_bound());									
	EXPECT_EQ(3,cug.get_result().number_of_steps());	
	
	cout<<"--------------------------------------------"<<endl;
}

TEST(Enum, isolani){
	cout<<"Enum: isolani------------------------"<<endl;
	int NV=4;
	ugraph ug(NV);
	
	
	//setup for maximal clique enumeration
	clqo::param_t p;
	p.alg=clqo::BBMC_EN_GCONF;
	p.init_order=clqo::MAX_WIDTH;				//gives minimum branches
	p.unrolled=false;
	CliqueEnum cug(&ug, p);
	if(cug.set_up()!=-1){
        cug.run();
    }
	cug.tear_down();

	//tests number of maximal cliques and steps (in this case optimal)
	EXPECT_EQ(4,cug.get_result().get_upper_bound());									
	EXPECT_EQ(0,cug.get_result().number_of_steps());		
	cout<<"--------------------------------------------"<<endl;
}

TEST(Enum, bigger_graph){
	cout<<"Enum: bigger_graph------------------------"<<endl;
	int NV=7;
	ugraph ug(NV);
	ug.add_edge(0,1);
	ug.add_edge(0,2);
	ug.add_edge(1,2);
	ug.add_edge(1,3);
	ug.add_edge(2,4);
	ug.add_edge(2,4);
	ug.add_edge(3,6);
	ug.add_edge(4,6);
	ug.add_edge(0,6);
	ug.add_edge(2,6);
	ug.add_edge(0,5);
		
	//setup for maximal clique enumeration
	clqo::param_t p;
	p.alg=clqo::BBMC_EN_GCONF;
	p.init_order=clqo::NONE;				//gives minimum branches
	p.unrolled=false;
	CliqueEnum cug(&ug, p);
	if(cug.set_up()!=-1){
        cug.run();
    }
	cug.tear_down();

	//tests number of maximal cliques and steps (in this case optimal)
	EXPECT_EQ(6,cug.get_result().get_upper_bound());									
	//EXPECT_EQ(0,cug.get_result().number_of_steps());		
	cout<<"--------------------------------------------"<<endl;
}

TEST(Enum, trick_graph){
	cout<<"Enum: trick_graph------------------------"<<endl;
	int NV=4;
	ugraph ug(NV);
	ug.add_edge(0,1);		
	ug.add_edge(0,3);
	ug.add_edge(1,2);		//0-1 is not enumerated if there isno reordering
		
	
	//setup for maximal clique enumeration
	clqo::param_t p;
	p.alg=clqo::BBMC_EN_GCONF;
	p.init_order=clqo::NONE;				//gives minimum branches
	p.unrolled=false;
	CliqueEnum cug(&ug, p);
	if(cug.set_up()!=-1){
        cug.run();
    }
	cug.tear_down();

	//tests number of maximal cliques and steps (in this case optimal)
	EXPECT_EQ(3,cug.get_result().get_upper_bound());									
	//EXPECT_EQ(0,cug.get_result().number_of_steps());		
	cout<<"--------------------------------------------"<<endl;
}

TEST(EnumSparse, basic_enum){
	cout<<"EnumSparse: basic_enum------------------------"<<endl;
	int NV=5;
	sparse_ugraph ug(NV);
	ug.add_edge(0,1);
	ug.add_edge(0,2);
	ug.add_edge(1,2);
	ug.add_edge(1,3);
	ug.add_edge(2,4);
	
	//setup for maximal clique enumeration
	clqo::param_t p;
	p.alg=clqo::BBMC_EN_GCAND;
	p.init_order=clqo::MAX_WIDTH;				//gives minimum branches
	p.unrolled=false;
	CliqueEnumSparse cug(&ug, p);
	if(cug.set_up()!=-1){
        cug.run();
    }
	cug.tear_down();

	//tests number of maximal cliques and steps (in this case optimal)
	EXPECT_EQ(3,cug.get_result().get_upper_bound());									
	EXPECT_EQ(3,cug.get_result().number_of_steps());		
	cout<<"--------------------------------------------"<<endl;
}

TEST(EnumSparse, complete_graph){
	cout<<"EnumSparse: complete_graph------------------------"<<endl;
	int NV=4;
	sparse_ugraph ug(NV);
	ug.add_edge(0,1);
	ug.add_edge(0,2);
	ug.add_edge(0,3);
	ug.add_edge(1,2);
	ug.add_edge(1,3);
	ug.add_edge(2,3);
	
	//setup for maximal clique enumeration
	clqo::param_t p;
	p.alg=clqo::BBMC_EN_GCONF;
	p.init_order=clqo::MAX_WIDTH;				//gives minimum branches
	p.unrolled=false;
	CliqueEnumSparse cug(&ug, p);
	if(cug.set_up()!=-1){
        cug.run();
    }
	cug.tear_down();

	//tests number of maximal cliques and steps (in this case optimal)
	EXPECT_EQ(1,cug.get_result().get_upper_bound());									
	EXPECT_EQ(3,cug.get_result().number_of_steps());	
	
	cout<<"--------------------------------------------"<<endl;
}


TEST(EnumSparse, isolani){
	cout<<"EnumSparse:isolani------------------------"<<endl;
	int NV=4;
	sparse_ugraph ug(NV);
	
	
	//setup for maximal clique enumeration
	clqo::param_t p;
	p.alg=clqo::BBMC_EN_GCONF;
	p.init_order=clqo::MAX_WIDTH;				//gives minimum branches
	p.unrolled=false;
	CliqueEnumSparse cug(&ug, p);
	if(cug.set_up()!=-1){
        cug.run();
    }
	cug.tear_down();

	//tests number of maximal cliques and steps (in this case optimal)
	EXPECT_EQ(4,cug.get_result().get_upper_bound());									
	EXPECT_EQ(0,cug.get_result().number_of_steps());		
	cout<<"--------------------------------------------"<<endl;
}

TEST(EnumSparse, bigger_graph){
	cout<<"EnumSparse: bigger_graph------------------------"<<endl;
	int NV=7;
	sparse_ugraph ug(NV);
	ug.add_edge(0,1);
	ug.add_edge(0,2);
	ug.add_edge(1,2);
	ug.add_edge(1,3);
	ug.add_edge(2,4);
	ug.add_edge(2,4);
	ug.add_edge(3,6);
	ug.add_edge(4,6);
	ug.add_edge(0,6);
	ug.add_edge(2,6);
	ug.add_edge(0,5);
		
	//setup for maximal clique enumeration
	clqo::param_t p;
	p.alg=clqo::BBMC_EN_GCONF;
	p.init_order=clqo::NONE;				
	p.unrolled=false;
	CliqueEnumSparse cug(&ug, p);
	if(cug.set_up()!=-1){
        cug.run();
    }
	cug.tear_down();

	//tests number of maximal cliques and steps (in this case optimal)
	EXPECT_EQ(6,cug.get_result().get_upper_bound());									
	//EXPECT_EQ(0,cug.get_result().number_of_steps());		
	cout<<"--------------------------------------------"<<endl;
}

TEST(EnumSparse, trick_graph){
	cout<<"EnumSparse: trick_graph------------------------"<<endl;
	int NV=4;
	sparse_ugraph ug(NV);
	ug.add_edge(0,1);		
	ug.add_edge(0,3);
	ug.add_edge(1,2);		//0-1 is not enumerated if there isno reordering
		
	
	//setup for maximal clique enumeration
	clqo::param_t p;
	p.alg=clqo::BBMC_EN_GCONF;
	p.init_order=clqo::NONE;				
	p.unrolled=false;
	CliqueEnumSparse cug(&ug, p);
	if(cug.set_up()!=-1){
        cug.run();
    }
	cug.tear_down();

	//tests number of maximal cliques and steps (in this case optimal)
	EXPECT_EQ(3,cug.get_result().get_upper_bound());									
	//EXPECT_EQ(0,cug.get_result().number_of_steps());		
	cout<<"--------------------------------------------"<<endl;
}

TEST(EnumSparseUnrolled, basic_enum){
	cout<<"EnumSparse: basic_enum------------------------"<<endl;
	int NV=5;
	sparse_ugraph ug(NV);
	ug.add_edge(0,1);
	ug.add_edge(0,2);
	ug.add_edge(1,2);
	ug.add_edge(1,3);
	ug.add_edge(2,4);
	
	//setup for maximal clique enumeration
	clqo::param_t p;
	p.alg=clqo::BBMC_EN_GCAND;
	p.init_order=clqo::MAX_WIDTH;					
	p.unrolled=true;
	CliqueEnumSparse cug(&ug, p);
	if(cug.set_up()!=-1){
        cug.run();
    }
	cug.tear_down();

	//tests number of maximal cliques and steps (in this case optimal)
	EXPECT_EQ(3,cug.get_result().get_upper_bound());									
	EXPECT_EQ(5,cug.get_result().number_of_steps());		
	cout<<"--------------------------------------------"<<endl;
}

TEST(EnumSparseUnrolled, complete_graph){
	cout<<"EnumSparse: complete_graph------------------------"<<endl;
	int NV=4;
	sparse_ugraph ug(NV);
	ug.add_edge(0,1);
	ug.add_edge(0,2);
	ug.add_edge(0,3);
	ug.add_edge(1,2);
	ug.add_edge(1,3);
	ug.add_edge(2,3);
	
	//setup for maximal clique enumeration
	clqo::param_t p;
	p.alg=clqo::BBMC_EN_GCONF;
	p.init_order=clqo::MAX_WIDTH;				//gives minimum branches
	p.unrolled=true;
	CliqueEnumSparse cug(&ug, p);
	if(cug.set_up()!=-1){
        cug.run();
    }
	cug.tear_down();

	//tests number of maximal cliques and steps (in this case optimal)
	EXPECT_EQ(1,cug.get_result().get_upper_bound());									
	EXPECT_EQ(5,cug.get_result().number_of_steps());	
	
	cout<<"--------------------------------------------"<<endl;
}


TEST(EnumSparseUnrolled, isolani){
	cout<<"EnumSparse: isolani------------------------"<<endl;
	int NV=4;
	sparse_ugraph ug(NV);
	
	
	//setup for maximal clique enumeration
	clqo::param_t p;
	p.alg=clqo::BBMC_EN_GCONF;
	p.init_order=clqo::MAX_WIDTH;				//gives minimum branches
	p.unrolled=true;
	CliqueEnumSparse cug(&ug, p);
	if(cug.set_up()!=-1){
        cug.run();
    }
	cug.tear_down();

	//tests number of maximal cliques and steps (in this case optimal)
	EXPECT_EQ(4,cug.get_result().get_upper_bound());									
	EXPECT_EQ(0,cug.get_result().number_of_steps());		
	cout<<"--------------------------------------------"<<endl;
}

TEST(EnumSparseUnrolled, bigger_graph){
	cout<<"EnumSparse: bigger_graph------------------------"<<endl;
	int NV=7;
	sparse_ugraph ug(NV);
	ug.add_edge(0,1);
	ug.add_edge(0,2);
	ug.add_edge(1,2);
	ug.add_edge(1,3);
	ug.add_edge(2,4);
	ug.add_edge(2,4);
	ug.add_edge(3,6);
	ug.add_edge(4,6);
	ug.add_edge(0,6);
	ug.add_edge(2,6);
	ug.add_edge(0,5);
		
	//setup for maximal clique enumeration
	clqo::param_t p;
	p.alg=clqo::BBMC_EN_GCONF;
	p.init_order=clqo::NONE;				//gives minimum branches
	p.unrolled=true;
	CliqueEnumSparse cug(&ug, p);
	if(cug.set_up()!=-1){
        cug.run();
    }
	cug.tear_down();

	//tests number of maximal cliques and steps (in this case optimal)
	EXPECT_EQ(6,cug.get_result().get_upper_bound());									
	//EXPECT_EQ(0,cug.get_result().number_of_steps());		
	cout<<"--------------------------------------------"<<endl;
}

TEST(EnumSparseUnrolled, trick_graph){
	cout<<"EnumSparseUnrolled: trick_graph------------------------"<<endl;
	int NV=4;
	sparse_ugraph ug(NV);
	ug.add_edge(0,1);		
	ug.add_edge(0,3);
	ug.add_edge(1,2);		//0-1 is not enumerated if there isno reordering
		
	
	//setup for maximal clique enumeration
	clqo::param_t p;
	p.alg=clqo::BBMC_EN_GCONF;
	p.init_order=clqo::NONE;				//gives minimum branches
	p.unrolled=true;
	CliqueEnumSparse cug(&ug, p);
	if(cug.set_up()!=-1){
        cug.run();
    }
	cug.tear_down();

	//tests number of maximal cliques and steps (in this case optimal)
	EXPECT_EQ(3,cug.get_result().get_upper_bound());									
	//EXPECT_EQ(0,cug.get_result().number_of_steps());		
	cout<<"--------------------------------------------"<<endl;
}


