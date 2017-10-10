//--------------------------------------------
// some tests for approximate coloring

#include "gtest/gtest.h"
#include "graph/graph_gen.h"
#include "utils/prec_timer.h"
#include "utils/file.h"
#include "../init_color.h"
#include "../init_color_ub.h"
#include "../init_color_sort.h"
//#include "../init_order.h"
#include "../clique_sort.h"
#include "graph\algorithms\graph_sort.h"

using namespace std;

typedef vector<int> vint;

TEST(Color, simple_graph){
	LOG_INFO("Color-simple_graph---------------------------------");
	const int SIZEG =6;
	int myints[] = {1,2,1,3,1,2};
	vint sol (myints, myints + 6);
	vint res;
	
	//Ugraph
	ugraph ug(SIZEG);
	ug.add_edge(0, 1);
	ug.add_edge(0, 3);
	ug.add_edge(1, 2);
	ug.add_edge(1, 3);
	ug.add_edge(2, 5);

	InitColor<ugraph> c(ug);
	int col_size=c.greedyIndependentSetColoring(res);
	EXPECT_TRUE(res==sol);
		
	col_size=c.greedyColoring(res);
	EXPECT_TRUE(res==sol);

	LOG_INFO("--------------------------------------------------------");
}

TEST(Color, simple_degree_coloring){
	LOG_INFO("Color-simple_degree_coloring---------------------------------");
	const int SIZEG =6, NV=20;
	
	ugraph ug(NV);				/* Ugraph: SEQ{1,2,1,3,1,2} */
	ug.add_edge(0, 1);
	ug.add_edge(0, 3);
	ug.add_edge(1, 2);
	ug.add_edge(1, 3);
	ug.add_edge(2, 5);

	EXPECT_EQ(3, ug.max_degree_of_graph());

	InitColor<ugraph> c(ug);
	vint col;
	c.simpleDegreeColoring(col);
	vint vexp(NV, ug.max_degree_of_graph()+1);
	for(int i=1; i<=ug.max_degree_of_graph(); i++){
		vexp[i-1]=i;
	}

	EXPECT_EQ(vexp, col);
	

	LOG_INFO("--------------------------------------------------------");
}

TEST(Color_sparse, simple_degree_coloring){
	LOG_INFO("Color_sparse-simple_degree_coloring---------------------------------");
	
	const int NV =20;
	
	sparse_ugraph ug(NV);			/* graph: SEQ{1,2,1,3,1,2}  */
	ug.add_edge(0, 1);
	ug.add_edge(0, 3);
	ug.add_edge(1, 2);
	ug.add_edge(1, 3);
	ug.add_edge(2, 5);

	EXPECT_EQ(3, ug.max_degree_of_graph());

	InitColor<sparse_ugraph> c(ug);
	vint col;
	c.simpleDegreeColoring(col);
	vint vexp(NV, ug.max_degree_of_graph()+1);
	for(int i=1; i<=ug.max_degree_of_graph(); i++){
		vexp[i-1]=i;
	}

	EXPECT_EQ(vexp, col);
	

	LOG_INFO("--------------------------------------------------------");
}

TEST(Color, simple_degree_coloring_subgrah){
	LOG_INFO("Color-simple_degree_coloring_subgrah---------------------------------");
	
	const int NV =20;
	
	ugraph ug(NV);				/* Ugraph: SEQ{1,2,1,3,1,2} */
	ug.add_edge(0, 1);
	ug.add_edge(0, 3);
	ug.add_edge(1, 2);
	ug.add_edge(1, 3);
	ug.add_edge(2, 5);

	bitarray sg(NV);
	sg.init_bit(2,5);
	
	EXPECT_EQ(1, ug.max_degree_of_subgraph(sg));

	InitColor<ugraph> c(ug);
	vint col;
	c.simpleDegreeColoring(col, sg);
	vint vexp(NV, ug.max_degree_of_graph()+1);
	for(int i=1; i<=ug.max_degree_of_graph(); i++){
		vexp[i-1]=i;
	}


	LOG_INFO("--------------------------------------------------------");
}

TEST(Color_sparse, simple_degree_coloring_subgrah){
	LOG_INFO("Color_sparse-simple_degree_coloring_subgrah---------------------------------");
	
	const int NV =20;
	
	sparse_ugraph ug(NV);		/* sparse ugraph: SEQ{1,2,1,3,1,2}  */
	ug.add_edge(0, 1);
	ug.add_edge(0, 3);
	ug.add_edge(1, 2);
	ug.add_edge(1, 3);
	ug.add_edge(2, 5);

	sparse_bitarray sg(NV);
	sg.init_bit(2,5);
	
	EXPECT_EQ(1, ug.max_degree_of_subgraph(sg));

	InitColor<sparse_ugraph> c(ug);
	vint col;
	c.simpleDegreeColoring(col, sg);
	vint vexp(NV, ug.max_degree_of_graph()+1);
	for(int i=1; i<=ug.max_degree_of_graph(); i++){
		vexp[i-1]=i;
	}


	LOG_INFO("--------------------------------------------------------");
}

TEST(ColorUB, simple_graph){
////////////
// tests complex init UB for all vertices of an ugraph
	LOG_INFO("ColorUB-simple_graph---------------------------------");

	const int SIZEG =6;
	int array_col[] = {1,2,1,3,1,2};
	vint sol (array_col, array_col + 6);
	vint res;
	
	
	//Ugraph
	ugraph ug(SIZEG);
	ug.add_edge(0, 1);
	ug.add_edge(0, 3);
	ug.add_edge(1, 2);
	ug.add_edge(1, 3);
	ug.add_edge(2, 5);

	//algorithm for determining an initial upper bound
	InitColorUB c(ug);
	int col_size=c.InitColor<ugraph>::greedyIndependentSetColoring(res);
	EXPECT_EQ(sol,res);
	
	res.clear();
	res.assign(SIZEG, -1);
	c.Compute_UB(&res[0]);
	int ubsol[]={1, 2, 2, 3, 1, 2};
	sol.assign(ubsol, ubsol+6);
	EXPECT_EQ(sol,res);

	LOG_INFO("--------------------------------------------------------");
}


TEST(ColorUB, odd_cycle){
////////////
// tests complex init UB for all vertices of an ugraph

	LOG_INFO("ColorUB-odd_cycle---------------------------------");
	const int SIZEG =5;

	//Ugraph
	ugraph ug(SIZEG);
	ug.add_edge(0, 1);
	ug.add_edge(1, 2);
	ug.add_edge(2, 3);
	ug.add_edge(3, 4);


	//algorithm for determining an initial upper bound
	InitColorUB c(ug);
	
	
	vint res(SIZEG, -1);
	c.Compute_UB(&res[0]);
	int array_sol[]={1, 2, 2, 2, 2};
	vint ubsol(array_sol, array_sol+SIZEG);
	EXPECT_EQ(ubsol,res);

	LOG_INFO("--------------------------------------------------------");
}


TEST(ColorSort, simple_graph){
////////////
// tests basic functions
	
	LOG_INFO("ColorSort-simple_graph---------------------------------");
	const int SIZEG =5;

	//Ugraph
	ugraph ug(SIZEG);
	ug.add_edge(0, 1);
	ug.add_edge(1, 2);
	ug.add_edge(2, 3);
	ug.add_edge(3, 4);


	//determine a new ordering for G by SEQ increasing color number
	vint resCol(SIZEG, -1);
	vint resOrd(SIZEG, -1);
	InitColorSort c(ug);
	c.greedyIndependentSetColoring(resCol, resOrd);

	com::stl::print_collection<vint>(resCol); cout<<endl;
	com::stl::print_collection<vint>(resOrd); cout<<endl;
	
	//generate the new sorted graph in place
	int nVer=ug.number_of_vertices();
	int nEdges=ug.number_of_edges();
	GraphSort<ugraph> io(ug); 
	io.reorder(resOrd);
	
	EXPECT_EQ(ug.number_of_vertices(), nVer);
	EXPECT_EQ(ug.number_of_edges(), nEdges);
			

	LOG_INFO("--------------------------------------------------------");
}

TEST(ColorSort, maxclique_simple_graph){
////////////
// tests maxclique solver 
	
	LOG_INFO("ColorSort-maxclique_simple_graph---------------------------------");
	const int SIZE_G=5;

	//Ugraph
	ugraph ug;
	vint sol=InitColorSort::maxclique(ug,clqo::param_t());		//empty graph
	EXPECT_TRUE(sol.empty());

	ug.init(SIZE_G);	
	sol=InitColorSort::maxclique(ug,clqo::param_t());				//5 vertices, no edges (solution vertex 0)
	EXPECT_EQ(0, sol[0]);								

	
	ug.add_edge(0, 1);
	ug.add_edge(1, 2);
	ug.add_edge(2, 3);
	ug.add_edge(3, 4);
	ug.add_edge(0, 2);

	vint exp_sol;
	exp_sol.push_back(0);
	exp_sol.push_back(1);
	exp_sol.push_back(2);
		
	sol=InitColorSort::maxclique(ug,clqo::param_t());
	sort(sol.begin(), sol.end());
	cout<<endl<<"Solution is: ";
	com::stl::print_collection<vint>(sol); cout<<endl;
	EXPECT_EQ(exp_sol,sol);
		
	LOG_INFO("--------------------------------------------------------");
}

TEST(ColorSort, maxclique_dimacs){
////////////
// tests maxclique solver
	
	LOG_INFO("ColorSort-maxclique_dimacs---------------------------------");
	
	//Ugraph
	ugraph ug("brock200_1.clq");
	
	vint exp_sol;
	exp_sol.push_back(0);
	exp_sol.push_back(1);
	exp_sol.push_back(2);
	
	
	vint sol=InitColorSort::maxclique(ug,clqo::param_t());
	sort(sol.begin(), sol.end());
	cout<<endl<<"Solution is: ";
	com::stl::print_collection<vint>(sol); cout<<endl;
	EXPECT_EQ(21, sol.size());
		
	LOG_INFO("--------------------------------------------------------");
}

TEST(ColorSort, dimacs){
////////////
// tests basic functions over dimacs instances
	
	LOG_INFO("ColorSort-dimacs---------------------------------");
	
	//Ugraph
	ugraph ug("brock200_1.clq");
	const int SIZEG=ug.number_of_vertices();
	
	//determine a new ordering for G by SEQ increasing color number
	vint resCol(SIZEG, -1);
	vint resOrd(SIZEG, -1);
	InitColorSort c(ug);
	c.greedyIndependentSetColoring(resCol, resOrd);

	com::stl::print_collection<vint>(resCol); cout<<endl;
	com::stl::print_collection<vint>(resOrd); cout<<endl;
	
	//generate the new sorted graph in place
	int nVer=ug.number_of_vertices();
	int nEdges=ug.number_of_edges();
	GraphSort<ugraph> io(ug); 
	io.reorder(resOrd);
	
	EXPECT_EQ(ug.number_of_vertices(), nVer);
	EXPECT_EQ(ug.number_of_edges(), nEdges);
	
	//write to file: optional
	ug.write_dimacs(FILE_LOG("brock200_1_new.clq", WRITE));

	LOG_INFO("--------------------------------------------------------");
}

TEST(ColorSort, RLF){
////////////
// tests maxclique solver
	
	LOG_INFO("ColorSort-RLF---------------------------------");
	const int SIZE_G=3;

	//Ugraph: complete, order unchanged
	ugraph ug(SIZE_G);			
	ug.add_edge(0, 1);
	ug.add_edge(1, 2);
	ug.add_edge(0, 2);
			
	InitColorSort cs(ug);
	vint ord;
	bool is_good=false;
	vint vcol=cs.recursiveLargestFirst(ord, is_good); 
	int nCol=vcol.size();
	EXPECT_EQ(3, nCol);
	EXPECT_TRUE(is_good);
	vint expect_sol;
	expect_sol.push_back(0);
	expect_sol.push_back(1);
	expect_sol.push_back(2);
	EXPECT_EQ(expect_sol, ord);
	
	LOG_INFO("--------------------------------------------------------");
}

TEST(ColorSort, RLF_brock){
////////////
// tests maxclique solver
	
	LOG_INFO("ColorSort-RLF_brock---------------------------------");
	
	//Ugraph
	ugraph ug("brock200_1.clq");			
	

	InitColorSort cs(ug);
	vint ord;
	bool is_good=false;
	vint vcol=cs.recursiveLargestFirst(ord,is_good); 
	int nCol=vcol.size();		
	EXPECT_EQ(47, nCol);
	EXPECT_TRUE(is_good);		
		

	//new order to file (****remove in the future)
	cout<<ug.get_name()<<":"<<nCol<<endl;
	GraphSort<ugraph> io(ug); 
	io.reorder(ord);
	ug.write_dimacs(FILE_LOG("brock200_1_rlf.clq",WRITE));
			
	LOG_INFO("--------------------------------------------------------");
}

TEST(ColorSort, RLF_frb){
////////////
// tests maxclique solver
	
	LOG_INFO("ColorSort-RLF_frb---------------------------------");

	
	//Ugraph
	ugraph ug("frb30-15-1.clq");			
	

	InitColorSort cs(ug);
	vint ord;
	bool is_good=false;
	vint vcol=cs.recursiveLargestFirst(ord,is_good); 
	int nCol=vcol.size();		
	EXPECT_EQ(30, nCol);
	EXPECT_TRUE(is_good);		
		

	//new order to file (****remove in the future)
	cout<<ug.get_name()<<":"<<nCol<<endl;
	GraphSort<ugraph> io(ug); 
	io.reorder(ord);
	ug.write_dimacs(FILE_LOG("frb30-15-1_rlf.clq",WRITE));
			
	LOG_INFO("--------------------------------------------------------");
}

TEST(ColorSort, RLF_keller){
////////////
// tests maxclique solver
	LOG_INFO("ColorSort-RLF_keller---------------------------------");
		
	//Ugraph
	ugraph ug("keller5.clq");			
		
	InitColorSort cs(ug);
	vint ord;
	bool is_good=false;
	vint vcol=cs.recursiveLargestFirst(ord,is_good); 
	int nCol=vcol.size();	
	EXPECT_TRUE(is_good);		
		

	//new order to file (****remove in the future)
	cout<<ug.get_name()<<":"<<nCol<<endl;
	GraphSort<ugraph> io(ug); 
	io.reorder(ord);
	ug.write_dimacs(FILE_LOG("keller5_rlf.clq",WRITE));
			
	LOG_INFO("--------------------------------------------------------");
}

TEST(ColorDIMACS, simple_test){
/////////////////////
// reading DIMACS color format 
// date: 4/1/17
// comments: created for Technical report with Bogdan Zavalnij of Pecs (12/2016)

	LOG_INFO("ColorDIMACS-simple_test---------------------------------");

	ugraph g("1dc.512-c.clq");
	int NV=g.number_of_vertices();
	vint vcol=InitColor<ugraph>::read_dimacs_color_format("1dc.512-c.clq.col", NV);
	com::stl::print_collection(vcol);

	EXPECT_EQ(NV, vcol.size());
	EXPECT_EQ(1, vcol[0]);			 /* c(1)=2 */	
	EXPECT_EQ(2, vcol[511]);		 /* c(512)=2 */
	EXPECT_EQ(46, vcol[501]);		 /* c(502)=46 */
	

	LOG_INFO("--------------------------------------------------------");
}

//
//TEST(Color_sparse, simple_graph){
//	const int SIZEG =6;
//	int myints[] = {1,2,1,3,1,2};
//	vint sol (myints, myints + 6);
//	vint res;
//	cout<<"--------------------------------------------------------"<<endl;
//
//		
//	//sparse graph
//	sparse_ugraph usg(SIZEG);
//	usg.add_edge(0, 1);
//	usg.add_edge(0, 3);
//	usg.add_edge(1, 2);
//	usg.add_edge(1, 3);
//	usg.add_edge(2, 5);
//
//	InitColor<sparse_ugraph> cs(usg);
//	int color_size=cs.greedyIndependentSetColoring(res);
//	EXPECT_TRUE(res==sol);
//		
//	color_size=cs.greedyColoring(res);
//	EXPECT_TRUE(res==sol);
//
//	cout<<"--------------------------------------------------------"<<endl;
//}
//
//TEST(Color_sparse, simple_subgraph){
//	const int SIZEG =6;
//	int myints[] = {1,2,1,3,1,2};
//	vint sol (myints, myints + 6);
//	vint res;
//	cout<<"--------------------------------------------------------"<<endl;
//
//		
//	//sparse graph
//	sparse_ugraph usg(SIZEG);
//	usg.add_edge(0, 1);
//	usg.add_edge(0, 3);
//	usg.add_edge(1, 2);
//	usg.add_edge(1, 3);
//	usg.add_edge(2, 5);
//
//	InitColor<sparse_ugraph> cs(usg);
//	int color_size;
//
//	sparse_ugraph::bb_type bbs(usg.number_of_vertices());
//	bbs.set_bit(0,3);
//	color_size=cs.greedyIndependentSetColoring(bbs);
//	EXPECT_EQ(3, color_size);	//color: 1,2,1,3
//
//	bbs.erase_bit(3);
//	color_size=cs.greedyIndependentSetColoring(bbs);
//	EXPECT_EQ(2, color_size);  //color: 1,2,1
//	
//	cout<<"--------------------------------------------------------"<<endl;
//}
//
//TEST(Color, brock){
//	/*int myints[] = {52,59,58,55,55,51,47,57,41,56,55,1,33,32,26,10,35,25,53,54,54,40,36,49,54
//		,52,45,49,53,52,51,31,51,44,42,47,50,20,45,50,39,22,46,49,37,48,41,48,28,43,47,17,44,
//		26,23,34,47,46,35,45,11,43,32,10,45,38,39,44,42,38,29,41,43,42,36,35,31,19,30,28,40,41,
//		19,33,33,40,39,17,15,26,12,34,37,39,38,29,37,36,22,32,9,30,27,35,28,25,27,34,33,14,32,31,
//		24,5,31,30,19,29,29,28,26,27,27,2,26,24,22,25,25,22,18,15,6,23,14,8,5,23,21,3,20,24,6,20,19,
//		23,21,22,21,10,20,4,19,14,18,17,18,11,16,15,17,12,13,12,16,16,7,15,14,13,9,13,12,11,11,7,4,10,
//		8,1,10,7,9,4,9,6,2,8,7,6,3,5,5,3,2,4,3,2,1,1};
//	std::vector<int> sol1 (myints, myints + sizeof(myints) / sizeof(int) );
//	std::vector<int> res1;
//	int list[200];
//	cout<<"--------------------------------------------------------"<<endl;
//
//	//-------------------------------------------------------------------------
//    //Ugraph
//    ugraph ug("brock200_1.clq");
//	InitColor<ugraph> c(ug);
//	c.init_coloring(DEFAULT_INIT_COLOR,list);
//	for (int i = ug.number_of_vertices() - 1; i >= 0; i--) 
//		{cout << list[i]<<" ";
//		res1.push_back(list[i]);
//		}
//	cout<<endl;
//	res1.clear();
//
//	c.init_coloring(GREDDY,list);
//	for (int i = ug.number_of_vertices() - 1; i >= 0; i--) {
//		cout << list[i]<<" ";
//		res1.push_back(list[i]);}
//	cout<<endl;
//	EXPECT_EQ(true,res1==sol1);
//	res1.clear();
//
//	cout<<"--------------------------------------------------------"<<endl;
//
//
//	//-------------------------------------------------------------------------
//    //Sparse Ugraph
//    sparse_ugraph usg("brock200_1.clq");
//	InitColor<sparse_ugraph> cs(usg);
//	cs.init_coloring(DEFAULT_INIT_COLOR,list);
//	for (int i = usg.number_of_vertices() - 1; i >= 0; i--) {
//		cout << list[i]<<" ";
//		res1.push_back(list[i]);}
//	cout<<endl;
//	res1.clear();
//
//	c.init_coloring(GREDDY,list);
//	for (int i = usg.number_of_vertices() - 1; i >= 0; i--){ 
//		cout << list[i]<<" ";
//		res1.push_back(list[i]);}
//	cout<<endl;
//	EXPECT_EQ(true,res1==sol1);
//	res1.clear();
//
//	c.init_coloring(TOMITA,list);
//	for (int i = usg.number_of_vertices() - 1; i >= 0; i--) {
//		cout << list[i]<<" ";
//		res1.push_back(list[i]);}
//	cout<<endl;
//	EXPECT_EQ(true,res1==sol1);
//	res1.clear();*/
//}
//
//
//
//TEST(Color_matrix, simple_graph){
//    const int SIZEG =6;
//    int myints[] = {1,2,1,3,1,2};
//    vint sol (myints, myints + 6);
//    vint res;
//    cout<<"--------------------------------------------------------"<<endl;
//
//    //Ugraph
//    ugraph ug(SIZEG);
//    ug.add_edge(0, 1);
//    ug.add_edge(0, 3);
//    ug.add_edge(1, 2);
//    ug.add_edge(1, 3);
//    ug.add_edge(2, 5);
//
//    InitColor<ugraph> c(ug);
//	c.init_ColorMatrix();
//    int col_size=c.greedyColoring(res);
//    EXPECT_TRUE(res==sol);    
//
//	col_size=c.greedyColorMatrixColoring(res);
//    EXPECT_TRUE(res==sol);  
//
//    cout<<"--------------------------------------------------------"<<endl;
//}
//
//TEST(Color_matrix, correctness){
///////////////////////
//// OBSERVATIONS: Currently times in this test of the two algorithms are not comparable because
//// memory is allocated inside one of the algorithms
//
//	PrecisionTimer pt;
//    vint sol, res; 
//    double secs, secs2, col_size,col_size2;
//
//    const int NV_INF=2000, NV_SUP=10000, INC_SIZE=500, REP_MAX=10;
//    const double DEN_INF=.1,DEN_SUP=.9, INC_DENSITY=.1;
//    /*string path="auxfilerandom.txt";
//    ofstream f(path, std::ofstream::out);
//    f<<"tam den rep TotalGreedy Greedy TotalMatrix Matrix TotalMatrixTrans MatrixTrans"<<endl;*/
//	for(int tam=NV_INF;tam<NV_SUP;tam+=INC_SIZE)  {
//		for(double den=DEN_INF;den<DEN_SUP;den+=INC_DENSITY){
//			for(int rep=0;rep<REP_MAX;rep++){
//				cout<<"--------------------------------------------------------"<<endl;
//				//-------------------------------------------------------------------------
//				//Ugraph
//				ugraph ug;
//				RandomGen::create_ugraph(ug,tam,den);
//				sol.clear();
//				res.clear();
//
//				InitColor<ugraph> c(ug);
//				c.init_ColorMatrix();
//				
//				pt.wall_tic();
//				col_size=c.greedyIndependentSetColoring(sol);
//				secs=pt.wall_toc();
//
//				pt.wall_tic();				
//				col_size2=c.greedyColorMatrixColoring(res);
//				secs2=pt.wall_toc();
//				
//
//				//tests for same color assignments to all vertices
//				ASSERT_TRUE(res==sol);
//				cout<<"[N:"<<tam<<" p:"<<den<<" r:"<<rep<<" c1:"<<col_size<<" c2:"<<col_size2<<" t1:"<<secs<<" t2:"<<secs2<<"]"<<endl;
//			}
//		}
//	}
//    //f.close();
//}
