//--------------------------------------------
// date: 15/1/16
// some tests for approximate coloring utilities
// 1-color matrix

#include "gtest/gtest.h"
#include "graph/graph_gen.h"
#include "utils/prec_timer.h"
#include "utils/file.h"
#include "utils/common.h"
#include "../color/color_utils.h"


using namespace std;

//template <class Collection>
//void printCol(Collection& col){
//	copy(col.begin(), col.end(), ostream_iterator<typename Collection::value_type>(cout, " "));
//}

typedef vector<int> vint;

TEST(ColorMatrix, simple_graph){
	const int SIZEG =6;
	/*int myints[] = {1,2,1,3,1,2};
	vint sol (myints, myints + 6);
	vint res;
	cout<<"--------------------------------------------------------"<<endl;*/

	//Ugraph
	ugraph ug(SIZEG);
	ug.add_edge(0, 1);
	ug.add_edge(0, 3);
	ug.add_edge(1, 2);
	ug.add_edge(1, 3);
	ug.add_edge(2, 5);

	ColorMat<color_bitarray_t> mc(ug, 10, ug.number_of_vertices());

	//adds vertices
	mc.push(0, 3);
	mc.push(0, 4);
	EXPECT_TRUE(2, mc.get_size(0));
	EXPECT_TRUE(mc.is_col(0,3));
	EXPECT_TRUE(mc.is_col(0,4));

	//removes vertices
	mc.pop(0);
	EXPECT_EQ(1, mc.get_size(0));
	EXPECT_FALSE(mc.is_col(0,3));
	EXPECT_TRUE(mc.is_col(0,4));

	//add further color sets
	mc.push(1, 3); mc.push(1, 4); mc.push(1, 5);
	mc.push(2, 3); 
	mc.push(4, 0); mc.push(4, 1); mc.push(4, 2);
	
	
	//create sorted list of colors
	mc.init_color_list();
	mc.sort_color_list();
	com::stl::print_collection<vint>(mc.get_color_list());
	vint exp; exp.push_back(0);  exp.push_back(2);  exp.push_back(1);  exp.push_back(4);		//C={0 (1), 2 (1), 1 (3), 4(3)}
	EXPECT_EQ(exp, mc.get_color_list());


	/*InitColor<ugraph> c(ug);
	int col_size=c.greedyIndependentSetColoring(res);
	EXPECT_TRUE(res==sol);
		
	col_size=c.greedyColoring(res);
	EXPECT_TRUE(res==sol);*/

	cout<<"--------------------------------------------------------"<<endl;
}

