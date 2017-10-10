//test_func.cpp: tests for functions in graph_func.h (namespace gfunc)

#include <iostream>
#include "gtest/gtest.h"
#include "../graph.h"
#include "../graph/algorithms/graph_func.h"

using namespace std;

TEST(Graph_func, is_clique){
	LOG_INFO("Graph_func::is_clique()-------------");

	ugraph g(5);
	g.add_edge(0,1);
	g.add_edge(1,2);
	g.add_edge(0,2);
	g.add_edge(0,3);
	
	bitarray bb(5);
	bb.set_bit(0); bb.set_bit(1); bb.set_bit(2);
	EXPECT_TRUE(gfunc::is_clique(g,bb));

	bb.set_bit(3);
	EXPECT_FALSE(gfunc::is_clique(g,bb));

	bb.erase_bit(3);
	vint lv; 
	for(int i=0; i<=2; i++)
		lv.push_back(i);
	EXPECT_TRUE(gfunc::is_clique(g,lv));
	int old_lv[5];
	for(int i=0; i<=2; i++)
		old_lv[i]=i;
	EXPECT_TRUE(gfunc::is_clique(g,old_lv,3));


	bb.set_bit(3);
	lv.push_back(3);
	EXPECT_FALSE(gfunc::is_clique(g,lv));
	for(int i=0; i<=3; i++)
		old_lv[i]=i;
	EXPECT_FALSE(gfunc::is_clique(g,old_lv,4));

	LOG_INFO("------------------------------------");
}

