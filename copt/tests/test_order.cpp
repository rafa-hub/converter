//--------------------------------------------
// tests for reordering 

#include "gtest/gtest.h"
#include "graph/graph_gen.h"
#include "graph/algorithms/decode.h"
#include "utils/common.h"
#include "../clique/clique.h"
#include "../clique/clique_watched.h"
#include "../clique/clique_infra_plus.h"
#include "../clique_sort.h"
#include "../clique/clique_types.h"

#include <algorithm>

using namespace std;

TEST(OrderFast, random_fast_min_width_big){
///////////////
// Uses watched encoding

	LOG_INFO("OrderFast: random_fast_min_width_big------------------------");
		
	ugraph ug;
	RandomGen<ugraph>::create_ugraph(ug, 60000, .01);
	ug.print_data();   
	
	clqo::param_t p;
	p.alg=clqo::BBMC_WT;
	p.init_order=clqo::MIN_WIDTH_BIG;
	p.init_preproc=clqo::UB;
	CliqueWatched cug(&ug, p);
	if(cug.set_up()==0){
        cug.run();
    }
	cug.tear_down();

    EXPECT_EQ(5,cug.get_max_clique());
	LOG_INFO("--------------------------------------------------");
	cin.get();
}

TEST(OrderFast, brock200_1_min_width_big){
	LOG_INFO("OrderFast: brock200_1_min_width_big------------------------");
	ugraph ug("brock200_1.clq");    
	
	clqo::param_t p;
	p.alg=clqo::BBMCL;
	p.init_order=clqo::MIN_WIDTH_BIG;
	p.init_preproc=clqo::UB_HEUR;
	CliqueInfraPlus cug(&ug, p);
	if(cug.set_up()==0){
        cug.run();
    }
	cug.tear_down();

    EXPECT_EQ(21,cug.get_max_clique());
	LOG_INFO("--------------------------------------------------");
	cin.get();
}

TEST(OrderFast, brock200_1){
	LOG_INFO("OrderFast: brock200_1------------------------");
	ugraph ug("brock200_1.clq");    
	
	//clique 3

	CliqueSort<ugraph> cs(ug);
	vint vno=cs.new_order(clqo::MIN_WIDTH);						//uses GraphSort::new_order_fast

	GraphSort<ugraph> gs(ug);
	vint vno1=gs.new_order(gbbs::MIN_DEG_DEGEN,gbbs::PLACE_LF);

	EXPECT_EQ(vno1,vno);

	LOG_INFO("--------------------------------------------");
}

TEST(OrderFast, random){
	LOG_INFO("OrderFast: random------------------------");
	
	Result r;
	ugraph ug;
	RandomGen<ugraph>::create_ugraph(ug, 3000, .01);
	ug.print_data();

	LOG_INFO("STARTING_TEST");		
	r.tic();
	CliqueSort<ugraph> cs(ug);
	vint vno=cs.new_order(clqo::MIN_WIDTH);					//uses GraphSort::new_order_fast
	//com::stl::print_collection(vno); cout<<endl;
	r.toc();
	LOG_INFO("Fast-[t: "<<r.get_user_time()<<"]");
	
	r.tic();
	GraphSort<ugraph> gs(ug);
	vint vno1=gs.new_order(gbbs::MIN_DEG_DEGEN,gbbs::PLACE_LF);
	//com::stl::print_collection(vno);
	r.toc();
	LOG_INFO("Default-[t: "<<r.get_user_time()<<"]");

	EXPECT_EQ(vno, vno1);
	LOG_INFO("--------------------------------------------");
}

TEST(OrderFast, random_sparse_furini){
	LOG_INFO("OrderFast: random_sparse_furini------------------------");
	
	Result r;
	ugraph ug;
	RandomGen<ugraph>::create_ugraph(ug, 20000, .01);
	ug.print_data();

	LOG_INFO("STARTING_TEST");		
	r.tic();
	CliqueSort<ugraph> cs(ug);
	vint vno=cs.new_order(clqo::MIN_WIDTH);   //uses GraphSort::new_order_fast
	//com::stl::print_collection(vno); cout<<endl;
	r.toc();
	LOG_INFO("Fast-[t: "<<r.get_user_time()<<"]");
	
	r.tic();
	GraphSort<ugraph> gs(ug);
	vint vno1=gs.new_order_furini(gbbs::MIN_DEG_DEGEN,gbbs::PLACE_LF);
	//com::stl::print_collection(vno);
	r.toc();
	LOG_INFO("Furini-[t: "<<r.get_user_time()<<"]");

	//EXPECT_EQ(vno, vno1);    /* not equal */
	LOG_INFO("--------------------------------------------");
}

TEST(Order, min_width_kcore){
	cout<<"Order: min_width_kcore------------------------"<<endl;
	ugraph ug("brock200_1.clq");    
	
	//clique 3
	Clique<ugraph> cug(&ug, clqo::param_t());
	cug.get_param().unrolled=false;
	cug.get_param().init_order=clqo::MIN_WIDTH_KCORE;
    if(cug.set_up()==0){
        cug.run();
    }
	cug.tear_down();

    EXPECT_EQ(21,cug.get_max_clique());
}


TEST(Order, Decode_clique){

	cout<<"Order: Decode_clique------------------------"<<endl;
	ugraph ug(106);    
    ug.add_edge(1, 2);
    ug.add_edge(1, 3);
    ug.add_edge(1, 4);
	ug.add_edge(2, 3);
	ug.add_edge(78, 5);
	ug.print_data();

	ugraph ug1(ug);

	//clique 3
	Clique<ugraph> cug(&ug1, clqo::param_t());
	cug.get_param().unrolled=true;
    if(cug.set_up()==0){
        cug.run();
    }
	cug.tear_down();

    EXPECT_EQ(3,cug.get_max_clique());
	vint sol=cug.decode_first_solution();
	EXPECT_TRUE(find(sol.begin(), sol.end(), 1)!=sol.end());
	EXPECT_TRUE(find(sol.begin(), sol.end(), 2)!=sol.end());
	EXPECT_TRUE(find(sol.begin(), sol.end(), 3)!=sol.end());
	EXPECT_FALSE(find(sol.begin(), sol.end(), 4)!=sol.end());
				
	//com::stl::print_collection(cug.decode_first_solution());

	//clique 4
	ug1=ug;
	ug1.add_edge(2,4);
	ug1.add_edge(1,4);
	ug1.add_edge(1,78);
	ug1.add_edge(2,78);
	ug1.add_edge(4,78);

	cug.set_graph(&ug1);
	cug.get_param().unrolled=true;
    if(cug.set_up()==0){
        cug.run();
    }
	cug.tear_down();
	EXPECT_EQ(4,cug.get_max_clique());
	sol=cug.decode_first_solution();
	
	set<int> s(sol.begin(), sol.end());
	set<int> exp;
	exp.insert(1); exp.insert(2); exp.insert(4); exp.insert(78);
	EXPECT_EQ(exp, s);

	//com::stl::print_collection(cug.decode_first_solution());


    cout<<"--------------------------------------------------------"<<endl;

}

TEST(Order, DecodeBrock200_1){
	cout<<"Order: DecodeBrock200_1------------------------"<<endl;
	ugraph ug("brock200_1.clq");    
	ugraph ug1(ug);

	//clique 3
	Clique<ugraph> cug(&ug1, clqo::param_t());
	cug.get_param().unrolled=false;
	cug.get_param().init_order=clqo::MIN_WIDTH_MIN_TIE_STATIC;
    if(cug.set_up()==0){
        cug.run();
    }
	cug.tear_down();

    EXPECT_EQ(21,cug.get_max_clique());
	vint sol=cug.decode_first_solution();

	com::stl::print_collection(cug.decode_first_solution()); cout<<endl;
	com::stl::print_collection(cug.get_result().get_first_solution()); cout<<endl;
			
	//restore initial graph to check if it is a clique
	std::vector<int> t = cug.get_result().get_first_solution();
	EXPECT_TRUE(cug.is_clique(t));		//clique sin descodificar
	EXPECT_FALSE(cug.is_clique(sol));
	cug.set_graph(&ug);
	EXPECT_TRUE(cug.is_clique(sol));										//clique descodificado

	 cout<<"--------------------------------------------------------"<<endl;
}

TEST(Order, DecodeBrock200_2){
	cout<<"Order: DecodeBrock200_2------------------------"<<endl;
	ugraph ug("brock200_2.clq");    
	ugraph ug1(ug);

	//clique 3
	Clique<ugraph> cug(&ug1, clqo::param_t());
	cug.get_param().unrolled=false;
	cug.get_param().init_order=clqo::MIN_WIDTH_MIN_TIE_STATIC;
    if(cug.set_up()==0){
        cug.run();
    }
	cug.tear_down();

    EXPECT_EQ(12,cug.get_max_clique());
	vint sol=cug.decode_first_solution();
			
	com::stl::print_collection(cug.decode_first_solution()); cout<<endl;
	com::stl::print_collection(cug.get_result().get_first_solution()); cout<<endl;
			
	//restore initial graph to check if it is a clique
	vint temp = cug.get_result().get_first_solution();
	EXPECT_TRUE(cug.is_clique(temp));		//clique sin descodificar
	EXPECT_FALSE(cug.is_clique(sol));
	cug.set_graph(&ug);
	EXPECT_TRUE(cug.is_clique(sol));										//clique descodificado

	 cout<<"--------------------------------------------------------"<<endl;
}

TEST(Order, DecodeCliqueNone){
	cout<<"Order: DecodeCliqueNone------------------------"<<endl;
	ugraph ug("brock200_1.clq");    
	ugraph ug1(ug);

	//clique 3
	Clique<ugraph> cug(&ug1, clqo::param_t());
	cug.get_param().unrolled=false;
	cug.get_param().init_order=clqo::NONE;		//corrected bug that reversed the ordering
    if(cug.set_up()==0){
        cug.run();
    }
	cug.tear_down();

    EXPECT_EQ(21,cug.get_max_clique());
	vint sol=cug.decode_first_solution();
	EXPECT_EQ(cug.get_result().get_first_solution(),sol);

	com::stl::print_collection(cug.decode_first_solution()); cout<<endl;
	com::stl::print_collection(cug.get_result().get_first_solution()); cout<<endl;
			
	//no need to restore initial graph to check if it is a clique since no ordering has been done
	EXPECT_TRUE(cug.is_clique(sol));
	cug.set_graph(&ug);
	EXPECT_TRUE(cug.is_clique(sol));

	cout<<"--------------------------------------------------------"<<endl;
}

TEST(Order, DecodeBrock200_1_watched){
	cout<<"Order: DecodeBrock200_1_watched------------------------"<<endl;
	ugraph ug("brock200_1.clq");    
	ugraph ug1(ug);

	//clique 3
	CliqueWatched cug(&ug1, clqo::param_t());
	cug.get_param().unrolled=false;
	cug.get_param().alg=clqo::BBMC_W;
	cug.get_param().init_order=clqo::MIN_WIDTH;
    if(cug.set_up()==0){
        cug.run();
    }
	cug.tear_down();

    EXPECT_EQ(21,cug.get_max_clique());
	vint sol=cug.decode_first_solution();

	com::stl::print_collection(cug.decode_first_solution()); cout<<endl;
	com::stl::print_collection(cug.get_result().get_first_solution()); cout<<endl;
			
	//restore initial graph to check if it is a clique
	EXPECT_FALSE(cug.is_clique(sol));
	cug.set_graph(&ug);
	EXPECT_TRUE(cug.is_clique(sol));

	 cout<<"--------------------------------------------------------"<<endl;
}

TEST(Order, composite_sort){

	cout<<"Order: composite_sort------------------------"<<endl;
	ugraph ug(106);    
    ug.add_edge(1, 2);
    ug.add_edge(1, 3);
    ug.add_edge(1, 4);
	ug.add_edge(2, 3);
	ug.add_edge(2, 4);
	ug.add_edge(3, 4);
	ug.add_edge(78, 5);
	ug.print_data();			//clique 1,2,3,4

	ugraph ug1(ug);


	//composite sort
	CliqueSort<ugraph>::vpclq lord;
	lord.push_back(pair<clqo::init_order_t,gbbs::place_t>(clqo::MIN_WIDTH,gbbs::PLACE_LF));
	lord.push_back(pair<clqo::init_order_t,gbbs::place_t>(clqo::RLF_COND, gbbs::PLACE_LF));
	lord.push_back(pair<clqo::init_order_t,gbbs::place_t>(clqo::RLF, gbbs::PLACE_LF));
	lord.push_back(pair<clqo::init_order_t,gbbs::place_t>(clqo::MAX_WIDTH, gbbs::PLACE_LF));

	CliqueSort<ugraph> cs(ug1);
	Decode d;
	cs.reorder_composite(lord,d);


	//solve clique
	Clique<ugraph> cug(&ug1, clqo::param_t());
	cug.get_param().unrolled=false;
	cug.get_param().init_order=clqo::MIN_WIDTH_MIN_TIE_STATIC;
    if(cug.set_up()==0){
        cug.run();
    }
	cug.tear_down();
	EXPECT_EQ(4,cug.get_max_clique());
	

	//comprobar si la solucion es un clique en el grafo original
	vint sol=cug.decode_first_solution();
	vint decoded_sol=d.decode_list(sol);
	
	Clique<ugraph> cug1(&ug,clqo::param_t());
	EXPECT_FALSE(cug1.is_clique(sol));
	EXPECT_TRUE(cug1.is_clique(decoded_sol));
	
	com::stl::print_collection(decoded_sol); cout<<endl;
    cout<<"--------------------------------------------------------"<<endl;

}

TEST(Order, composite_sort_brock){

	cout<<"Order: composite_sort------------------------"<<endl;
	ugraph ug("brock200_1.clq");    
	ugraph ug1(ug);


	//composite sort
	CliqueSort<ugraph>::vpclq lord;
	lord.push_back(pair<clqo::init_order_t,gbbs::place_t>(clqo::MIN_WIDTH,gbbs::PLACE_FL));
	lord.push_back(pair<clqo::init_order_t,gbbs::place_t>(clqo::MIN_WIDTH_MIN_TIE_STATIC,gbbs::PLACE_LF));
	lord.push_back(pair<clqo::init_order_t,gbbs::place_t>(clqo::RLF_COND, gbbs::PLACE_LF));
	lord.push_back(pair<clqo::init_order_t,gbbs::place_t>(clqo::RLF, gbbs::PLACE_LF));
	
	CliqueSort<ugraph> cs(ug1);
	Decode d;
	cs.reorder_composite(lord,d);


	//solve clique
	Clique<ugraph> cug(&ug1, clqo::param_t());
	cug.get_param().unrolled=false;
	cug.get_param().init_order=clqo::NONE;
    if(cug.set_up()==0){
        cug.run();
    }
	cug.tear_down();
	EXPECT_EQ(21,cug.get_max_clique());
	

	//comprobar si la solucion es un clique en el grafo original
	vint sol=cug.decode_first_solution();
	vint decoded_sol=d.decode_list(sol);
	
	Clique<ugraph> cug1(&ug,clqo::param_t());
	EXPECT_FALSE(cug1.is_clique(sol));
	EXPECT_TRUE(cug1.is_clique(decoded_sol));
	
	com::stl::print_collection(decoded_sol); cout<<endl;
    cout<<"--------------------------------------------------------"<<endl;

}

TEST(Order, composite_mixed_sort_brock){

	cout<<"Order: composite_mixed_sort_brock------------------------"<<endl;
	ugraph ug("brock200_1.clq");    
	ugraph ug1(ug);


	//composite sort
	CliqueSort<ugraph>::vpclq lord;
	lord.push_back(pair<clqo::init_order_t,gbbs::place_t>(clqo::MIXED_2,gbbs::PLACE_LF));
	lord.push_back(pair<clqo::init_order_t,gbbs::place_t>(clqo::MIXED_10,gbbs::PLACE_LF));			//MIXED does not use the second parameter
	
	CliqueSort<ugraph> cs(ug1);
	Decode d;
	cs.reorder_composite(lord,d);


	//solve clique
	Clique<ugraph> cug(&ug1, clqo::param_t());
	cug.get_param().unrolled=false;
	cug.get_param().init_order=clqo::NONE;
    if(cug.set_up()==0){
        cug.run();
    }
	cug.tear_down();
	EXPECT_EQ(21,cug.get_max_clique());
	

	//comprobar si la solucion es un clique en el grafo original
	vint sol=cug.decode_first_solution();
	vint decoded_sol=d.decode_list(sol);
	
	Clique<ugraph> cug1(&ug,clqo::param_t());
	EXPECT_FALSE(cug1.is_clique(sol));
	EXPECT_TRUE(cug1.is_clique(decoded_sol));
	
	com::stl::print_collection(decoded_sol); cout<<endl;
    cout<<"--------------------------------------------------------"<<endl;

}

TEST(Order, composite_sort_sparse){

	cout<<"Order: composite_sort_sparse------------------------"<<endl;
	sparse_ugraph ug(106);  
	ug.add_edge(0, 1);
    ug.add_edge(1, 2);
    ug.add_edge(1, 3);
    ug.add_edge(1, 4);
	ug.add_edge(2, 3);
	ug.add_edge(2, 4);
	ug.add_edge(3, 4);
	ug.add_edge(78, 5);
	ug.print_data();			//clique 1,2,3,4

	sparse_ugraph ug1(ug);


	//composite sort
	CliqueSort<sparse_ugraph>::vpclq lord;
	lord.push_back(pair<clqo::init_order_t,gbbs::place_t>(clqo::RLF, gbbs::PLACE_LF));				//discarded, but the previous are effective
	lord.push_back(pair<clqo::init_order_t,gbbs::place_t>(clqo::MIN_WIDTH,gbbs::PLACE_LF));
	lord.push_back(pair<clqo::init_order_t,gbbs::place_t>(clqo::MAX_WIDTH, gbbs::PLACE_LF));
	
	CliqueSort<sparse_ugraph> cs(ug1);
	Decode d;
	cs.reorder_composite(lord,d);
	
	//solve clique
	Clique<sparse_ugraph> cug(&ug1, clqo::param_t());
	cug.get_param().unrolled=true;									//TODO check exception is unrolled is false (note sparse_graph)
	cug.get_param().init_order=clqo::MIN_WIDTH_MIN_TIE_STATIC;
    if(cug.set_up()==0){
        cug.run();
    }
	cug.tear_down();
	EXPECT_EQ(4,cug.get_max_clique());
	
	//comprobar si la solucion es un clique en el grafo original
	vint sol=cug.decode_first_solution();
	vint decoded_sol=d.decode_list(sol);
	
	Clique<sparse_ugraph> cug1(&ug,clqo::param_t());
	EXPECT_FALSE(cug1.is_clique(sol));
	EXPECT_TRUE(cug1.is_clique(decoded_sol));
	
	com::stl::print_collection(decoded_sol); cout<<endl;
    cout<<"--------------------------------------------------------"<<endl;

}

TEST(Order, color_sort){

	cout<<"Order: color_sort------------------------"<<endl;
	ugraph ug(4);    
    ug.add_edge(0,3);
    ug.add_edge(1,2);
    ug.add_edge(1,3);
	
	ug.print_data();			
	ugraph ug1(ug);


	//composite sort
	CliqueSort<ugraph> cs(ug1);
	vint cset; cset.push_back(0), cset.push_back(2); cset.push_back(2);									//two colors of size 2 (labels start at 1)
	vint res=cs.new_color_set_order(clqo::init_order_t::MAX_WIDTH, cset,gbbs::place_t::PLACE_FL);
		
	vint sol; sol.push_back(0); sol.push_back(1); sol.push_back(3); sol.push_back(2);
	EXPECT_EQ(sol, res);
	com::stl::print_collection(res); cout<<endl;

	//second test
	cset.clear();
	cset.push_back(0), cset.push_back(3); cset.push_back(1);	
	res=cs.new_color_set_order(clqo::init_order_t::MAX_WIDTH, cset,gbbs::place_t::PLACE_FL);

	sol.clear(); sol.push_back(1); sol.push_back(0); sol.push_back(2); sol.push_back(3);
	EXPECT_EQ(sol, res);
	com::stl::print_collection(res); cout<<endl;
	cout<<"--------------------------------------------------------"<<endl;
}






//
//TEST(Order, DecodeCliqueGraphFromImages){
////**** TODO- TEST CURRENTLY FAILS ****
//	cout<<"Order: DecodeCliqueGraphFromImages------------------------"<<endl;
//	ugraph ug("imgJA.clq");   
//	ug.print_data();
//	ugraph ug1(ug);
//
//	//clique 3
//	Clique<ugraph> cug(&ug1, param_t());
//	cug.get_param().unrolled=false;
//	cug.get_param().init_order=MIN_WIDTH;
//	ug1.print_data();
//    if(cug.set_up()==0){
//        cug.run();
//    }else{
//		cout<<"TRIVIAL SOLUTION FOUND"<<endl;
//	}
//	//cug.tear_down();
//
//    EXPECT_EQ(188,cug.get_max_clique());
//	vint sol=cug.decode_first_solution();
//			
//	com::stl::print_collection(cug.decode_first_solution()); cout<<endl; cout<<endl;
//	com::stl::print_collection(cug.get_result().get_first_solution()); cout<<endl;
//
//	EXPECT_EQ(sol.size(),cug.get_result().get_first_solution().size());
//			
//	//restore initial graph to check if it is a clique
//	sol =cug.get_result().get_first_solution();
//	EXPECT_TRUE(cug.is_clique(sol));										//clique sin descodificar
//	EXPECT_FALSE(cug.is_clique(sol));
//	cug.set_graph(&ug);
//	EXPECT_TRUE(cug.is_clique(sol));										//clique descodificado
//
//	cout<<"--------------------------------------------------------"<<endl;
//}

//** TODO** POSSIBLY CHANGE TO GRAPH BLOCK!
//
//TEST(Order, reorder_Brock){
//    int myints[] = {176,126,41,15,148,152,80,25,175,110,106,198,59,134,30,58,167,96,98,20,158,76,154,118,129,62,14,112,95,
//        181,162,24,171,23,144,40,138,111,179,142,186,33,105,79,84,64,157,29,67,13,125,165,61,192,115,121,94,63,32,178,164,
//        104,151,137,117,185,187,170,101,44,191,196,22,133,91,180,87,93,19,86,109,21,12,83,182,120,75,173,194,128,174,163,
//        124,54,90,50,57,108,97,18,28,177,31,53,100,147,38,49,70,132,60,150,197,27,17,11,92,141,82,48,146,43,189,89,99,42,114,
//        74,16,127,52,39,172,193,140,184,37,36,26,119,149,130,47,35,161,199,143,190,10,169,183,9,168,123,122,34,73,85,8,81,107,
//        72,71,7,6,136,160,88,153,113,69,5,166,102,4,156,145,78,77,3,66,116,139,155,103,68,131,51,2,1,56,135,0,55,46,195,45,188,65,159};
//    std::vector<int> sol1 (myints, myints + sizeof(myints) / sizeof(int) );
//    int myints2[] = {172,126,77,8,161,155,163,19,168,115,61,198,64,136,116,68,167,72,114,43,169,78,157,129,90,13,25,26,98,140,
//        154,1,158,47,148,18,137,92,173,139,185,37,75,91,82,51,152,16,96,93,133,143,86,190,107,121,102,14,6,176,76,118,151,67,110,
//        184,187,179,89,111,191,195,45,131,44,177,100,95,7,71,113,59,24,58,182,103,99,174,194,119,175,142,63,65,85,62,66,117,41,30,
//        80,180,0,2,120,146,29,53,9,145,105,162,197,23,42,38,31,135,40,69,149,11,188,178,32,87,134,49,50,123,20,15,164,193,141,186,
//        127,12,84,122,150,124,60,4,153,199,156,189,22,166,183,101,181,128,125,81,39,55,36,54,132,171,35,52,33,112,170,74,108,104,83,
//        34,144,88,3,28,147,97,57,17,70,109,138,159,56,21,130,94,106,5,48,160,27,73,46,196,79,192,10,165};
//    std::vector<int> sol2 (myints2, myints2 + sizeof(myints2) / sizeof(int) );
//    cout<<"--------------------------------------------------------"<<endl;
//
//
//    //-------------------------------------------------------------------------
//    //Ugraph
//    ugraph ug("brock200_1.clq");
//    InitOrder<ugraph> o(ug);
//    InitOrder<>::print(o.create_new_order(MIN_WIDTH));
//    EXPECT_EQ(true,o.create_new_order(MIN_WIDTH)==sol1);
//    cout<<"initial order MIN WIDTH for ugraph passed"<<endl;
//    InitOrder<>::print(o.create_new_order(MIN_WIDTH_MIN_TIE_STATIC));
//    EXPECT_EQ(true,o.create_new_order(MIN_WIDTH_MIN_TIE_STATIC)==sol2);
//    o.reorder(o.create_new_order(MIN_WIDTH_MIN_TIE_STATIC));
//    cout<<"initial order MIN WIDTH MIN TIE STATIC for ugraph passed"<<endl;
//    cout<<"--------------------------------------------------------"<<endl;
//
//
//
//    //-------------------------------------------------------------------------
//    //Sparse Ugraph
//  /*  sparse_ugraph usg("brock200_1.clq");
//    InitOrder<sparse_ugraph> os(usg);
//    InitOrder<>::print(os.create_new_order(MIN_WIDTH));
//    EXPECT_EQ(true,os.create_new_order(MIN_WIDTH)==sol1);
//    cout<<"initial order MIN WIDTH for sparse ugraph passed"<<endl;
//    InitOrder<>::print(os.create_new_order(MIN_WIDTH_MIN_TIE_STATIC));
//    EXPECT_EQ(true,os.create_new_order(MIN_WIDTH_MIN_TIE_STATIC)==sol2);
//    cout<<"initial order MIN WIDTH MIN TIE STATIC for sparse ugraph passed"<<endl;
//    os.reorder(os.create_new_order(MIN_WIDTH_MIN_TIE_STATIC));*/
//    cout<<"--------------------------------------------------------"<<endl;
//}
//


//TEST(Order_in_place, correctness){
/////////////
//// Random tests: at the moment NOT CORRECT
//
//    PrecisionTimer pt;
//    vint sol, res; 
//    double secs, secs2, col_size,col_size2;
//	int j,j2;
//
//    const int NV_INF=1000, NV_SUP=5000, INC_SIZE=500, REP_MAX=10;
//    const double DEN_INF=.1,DEN_SUP=.9, INC_DENSITY=.1;
//    for(int tam=NV_INF;tam<NV_SUP;tam+=INC_SIZE)  {
//        for(double den=DEN_INF;den<DEN_SUP;den+=INC_DENSITY){
//            for(int rep=0;rep<REP_MAX;rep++){
//                cout<<"--------------------------------------------------------"<<endl;
//               
//                //Ugraph
//                sparse_ugraph ug;
//                SparseRandomGen<>().create_ugraph(ug,tam,den);
//				sparse_ugraph ug2(ug);
//				
//				InitOrder<sparse_ugraph> o(ug);
//				InitOrder<sparse_ugraph> o2(ug2);
//				
//
//				//Reverse ordering
//                o.reorder(o.create_new_order(NONE, PLACE_LF));
//				o2.reorder_in_place(o2.create_new_order(NONE, PLACE_LF));
//			
//                //Test edge by edge (should be changed to == operator when it works for sparse graphs)
//                ASSERT_TRUE(ug.number_of_vertices()==ug2.number_of_vertices());
//				for(int i=0;i<ug.number_of_vertices();i++){
//                    sparse_bitarray neigh=ug.get_neighbors(i);
//                    sparse_bitarray neigh2=ug2.get_neighbors(i);
//                    if((neigh.init_scan(bbo::NON_DESTRUCTIVE)!=EMPTY_ELEM) && (neigh2.init_scan(bbo::NON_DESTRUCTIVE)!=EMPTY_ELEM)) {
//                        while(true){
//                            j=neigh.next_bit();
//                            j2=neigh2.next_bit();
//							if(j!=j2){
//								neigh.print(); cout<<endl;
//								neigh2.print(); cout<<endl;
//								ug.get_neighbors(j).print(); cout<<endl;
//								ug2.get_neighbors(j).print(); cout<<endl;
//								cout<<"i:"<<i<<" j:"<<j<<endl;
//							}
//                            ASSERT_TRUE(j==j2);
//                            if(j==EMPTY_ELEM)
//                                    break;                
//                        }
//                    }
//                }
//                        
//          
//				//Reorders using MIN_WIDTH
//                o.reorder(o.create_new_order(MIN_WIDTH));
//                o2.reorder_in_place(o2.create_new_order(MIN_WIDTH));
//
//                //Test edge by edge (should be changed to == operator when it works for sparse graphs)
//                ASSERT_TRUE(ug.number_of_vertices()==ug2.number_of_vertices());
//			    for(int i=0;i<ug.number_of_vertices();i++){
//                    sparse_bitarray neigh=ug.get_neighbors(i);
//                    sparse_bitarray neigh2=ug2.get_neighbors(i);
//                    if((neigh.init_scan(bbo::NON_DESTRUCTIVE)!=EMPTY_ELEM) && (neigh2.init_scan(bbo::NON_DESTRUCTIVE)!=EMPTY_ELEM)) {
//                        while(true){
//                            j=neigh.next_bit();
//                            j2=neigh2.next_bit();
//                            ASSERT_TRUE(j==j2);
//                            if(j==EMPTY_ELEM)
//                                    break;                
//                        }
//                    }
//
//                }
//
//                //tests for same color assignments to all vertices
//                cout<<"[N:"<<tam<<" p:"<<den<<" r:"<<rep<<"]"<<endl;
//            }
//        }
//    }
//}





