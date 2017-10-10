//--------------------------------------------
// test_setup.cpp: tests for SETUP class: computes default setup for SELECTIVE branching strategies
// date of creation: 28/09/17
// last update: 28/09/17
// author: pss


#include "gtest/gtest.h"
#include "graph/graph_gen.h"
#include "graph/algorithms/decode.h"
#include "utils/common.h"
#include "utils/logger.h"
#include "../clique/clique.h"
#include "../interfaces/interface_interdiction.h"
#include "../setup.h"
#include "../clique/clique_infra_plus.h"
#include "graph/algorithms/graph_map.h"

using namespace std;
typedef vector<int> vint;


TEST(SETUP_clique, sort_and_store_as_mapping){
	LOG_INFO("SETUP_clique:sort_and_store_as_mapping-----------------------");
	
	ugraph g(10);
	g.add_edge(0,1);
	g.add_edge(1,2);
	g.add_edge(0,2);

	CliqueInfraPlus cip(&g, clqo::param_t());
	SETUP<CliqueInfraPlus> st(cip);
	GraphMapSingle gm;
	st.init_sort(gm);

	//mappings
	EXPECT_EQ(9,gm.map_l2r(3));
	EXPECT_EQ(3,gm.map_r2l(9));
	gm.print_mappings();

			
	LOG_INFO("------------------------------------------------");
}
TEST(SETUP_clique, partitioning_subgraph_brock200_1){
	LOG_INFO("SETUP_clique:partitioning_subgraph_brock200_1-----------------------");
	
	ugraph g("brock200_1.clq");
	int NV=g.number_of_vertices();
		
	CliqueInfraPlus cip(&g, clqo::param_t());

	//allocation-a bit artificial
	clqo::search_alloc_t info;
	info.set(clqo::search_alloc_t::ALLOC_COLOR_SETS);					//for iop/recoloring *TODO-CHECK
	bitarray bbV(NV);
	bbV.set_bit(0,NV-1);
	vint vset;
	cip.set_up_subgraph(bbV,&info);
		
	//setup
	SETUP<CliqueInfraPlus> st(cip);
	st.init_sort();
	int lb=st.lower_bound(vset);
	EXPECT_EQ(lb,vset.size()); 
	
	const int FIRST_100_NODES=100;
	bitarray bbsg(NV);						
	bbsg.set_bit(0,FIRST_100_NODES-1);			/* subgraph FIRST_50_NODES to be partitioned */
	int num_cand=st.partitioning(lb,bbsg);
		
	//Test: all nodes insie
	CliqueInfraPlus::nodelist_t& ln=cip.get_root_nodelist();
	EXPECT_EQ(num_cand, ln.index+1);
	for(int i=0; i<=ln.index; i++){
		EXPECT_LT(ln.nodos[i], FIRST_100_NODES);
	}

	//I/O
	cout<<"num candidates: "<<num_cand<<endl;
	ln.print(cout, true);
		
	LOG_INFO("------------------------------------------------");
}

TEST(SETUP_clique, lower_bound){
	LOG_INFO("SETUP_clique:setup_basic-----------------------");
	
	ugraph g(10);
	g.add_edge(0,1);
	g.add_edge(1,2);
	g.add_edge(0,2);

	ugraph gc(g);
	CliqueInfraPlus cip(&gc, clqo::param_t());
	SETUP<CliqueInfraPlus> st(cip);
	vint vset;
	EXPECT_EQ(3,st.lower_bound(vset));
		
	g.add_edge(0,3);
	g.add_edge(1,3);
	g.add_edge(2,3);

	gc=g;
	CliqueInfraPlus cip1(&gc, clqo::param_t());
	SETUP<CliqueInfraPlus> st1(cip1);
	EXPECT_EQ(4,st1.lower_bound(vset));

	g.remove_edge(2,3);
	gc=g;
	CliqueInfraPlus cip2(&gc, clqo::param_t());
	SETUP<CliqueInfraPlus> st2(cip2);
	EXPECT_EQ(3,st2.lower_bound(vset));
	

	//|E|=0
	ugraph gempty(10);
	CliqueInfraPlus cip3(&gempty, clqo::param_t());
	SETUP<CliqueInfraPlus> st3(cip3);
	EXPECT_EQ(1,st3.lower_bound(vset));
	
	LOG_INFO("------------------------------------------------");
}

TEST(SETUP_clique, lower_bound_forbidden_set){
	LOG_INFO("SETUP_clique:lower_bound_forbidden_set-----------------------");
	
	ugraph g(10);
	g.add_edge(0,1);
	g.add_edge(1,2);
	g.add_edge(0,2);

	ugraph gc(g);
	bitarray bb_forbidden(10);	
	CliqueInfraPlus cip(&gc, clqo::param_t());
	SETUP<CliqueInfraPlus> st(cip);
	vint vset;

	//one forbidden node
	bb_forbidden.set_bit(2);
	EXPECT_EQ(2,st.lower_bound(bb_forbidden,vset));
	vint::iterator it=find(vset.begin(), vset.end(), 2);
	EXPECT_EQ(vset.end(), it);
	//com::stl::print_collection(vset); cout<<endl;

	//2 forbidden nodes
	bb_forbidden.set_bit(1);
	EXPECT_EQ(1,st.lower_bound(bb_forbidden,vset));
	//com::stl::print_collection(vset); cout<<endl;

	//3-clique at init forbidden 
	bb_forbidden.set_bit(0);
	EXPECT_EQ(1,st.lower_bound(bb_forbidden,vset));
	//com::stl::print_collection(vset); cout<<endl;

	//all forbidden nodes
	bb_forbidden.set_bit(0, 9);
	EXPECT_EQ(0,st.lower_bound(bb_forbidden,vset));
		
	LOG_INFO("------------------------------------------------");
}

TEST(SETUP_clique, sort){
	LOG_INFO("SETUP_clique:sort-----------------------");
	
	ugraph g("brock200_1.clq");
	CliqueInfraPlus cip(&g, clqo::param_t());
	SETUP<CliqueInfraPlus> st(cip);
	st.init_sort();
	vint vset;
	int lb=st.lower_bound(vset);

	//check if there is a clique of size lb
	bitarray bb(g.number_of_vertices());
	bb.set_bit(0,lb-1);
	EXPECT_TRUE(cip.is_clique(bb));

	bb.set_bit(lb);
	EXPECT_FALSE(cip.is_clique(bb));
			
	LOG_INFO("------------------------------------------------");
}

TEST(SETUP_clique, partitioning){
	LOG_INFO("SETUP_clique:partitioning-----------------------");
	
	int NV=10;
	ugraph g(NV);
	g.add_edge(0,1);
	g.add_edge(1,2);
	g.add_edge(0,2);

	
	CliqueInfraPlus cip(&g, clqo::param_t());
	clqo::search_alloc_t info;
	info.set(clqo::search_alloc_t::ALLOC_COLOR_SETS);			//for recoloring
	bitarray bbV(NV);
	bbV.set_bit(0,NV-1);
	vint vset;
	cip.set_up_subgraph(bbV,&info);
		
	SETUP<CliqueInfraPlus> st(cip);
	st.init_sort();
	int lb=st.lower_bound(vset);
	EXPECT_EQ(lb,vset.size()); 
	
	cout<<"LB: "<<lb<<" number of candidates: "<<st.partitioning(lb);
			
	LOG_INFO("------------------------------------------------");
}


TEST(SETUP_clique, partitioning_brock200_1){
///////////
// **TODO-Lacks test!

	LOG_INFO("SETUP_clique:partitioning_brock200_1-----------------------");
	
	ugraph g("brock200_1.clq");
	int NV=g.number_of_vertices();
		
	CliqueInfraPlus cip(&g, clqo::param_t());

	//allocation-a bit artificial
	clqo::search_alloc_t info;
	info.set(clqo::search_alloc_t::ALLOC_COLOR_SETS);					//for iop/recoloring *TODO-CHECK
	bitarray bbV(NV);
	bbV.set_bit(0,NV-1);
	vint vset;
	cip.set_up_subgraph(bbV,&info);
		
	//setup
	SETUP<CliqueInfraPlus> st(cip);
	st.init_sort();
	int lb=st.lower_bound(vset);
	EXPECT_EQ(lb,vset.size()); 
	LOG_INFO("LB: "<<lb<<" number of candidates: "<<st.partitioning(lb));
	do{
		lb++;
		LOG_INFO("LB: "<<lb<<" number of candidates: "<<st.partitioning(lb));
	}while(lb<21);
			
	LOG_INFO("------------------------------------------------");
}


TEST(SETUP_clique, run_brock){
////////////
// Note: switch PARTITION_SETUP (clique.h) must be ON

	LOG_INFO("SETUP_clique:run_brock-----------------------");
	
	ugraph g("brock200_1.clq");
	int NV=g.number_of_vertices();
	ugraph g1(g);

	clqo::param_t param;
	param.alg=clqo::BBMCL;
	param.init_preproc=clqo::UB;  /* no AMTS */
		
	CliqueInfraPlus cip(&g1, param);
	if(cip.set_up()==0){
		cip.run();
	}

	vint vres=cip.decode_first_solution();						/* init sort-MIN_WIDTH */
	com::stl::print_collection(vres);
	cip.set_graph(&g);
	EXPECT_TRUE(cip.is_clique(vres));

			
	LOG_INFO("------------------------------------------------");
}

TEST(SETUP_clique, run_single_iset){
////////////
// Note: switch PARTITION_SETUP (clique.h) must be ON

	LOG_INFO("SETUP_clique:run_single_iset-----------------------");
	
	ugraph g(10);
	int NV=g.number_of_vertices();

	clqo::param_t param;
	param.alg=clqo::BBMCL;
	param.init_preproc=clqo::UB;  /* no AMTS */
		
	CliqueInfraPlus cip(&g, param);
	if(cip.set_up()==0){
		cip.run();
	}

	vint vres=cip.get_result().get_first_solution();			/* init sort-MIN_WIDTH */
	com::stl::print_collection(vres);
	cip.set_graph(&g);
	EXPECT_TRUE(cip.is_clique(vres));
	EXPECT_EQ(1,cip.get_max_clique());
			
	LOG_INFO("------------------------------------------------");
}






