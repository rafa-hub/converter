//test_csp.cpp: some tests for new CliqueCSP_Plus class concerning CSP problems as max clique

// date_of_creation: 26/2/17
// last_update: 26/2/17


#include "gtest/gtest.h"
#include "graph/graph_gen.h"
#include "utils/common.h"
#include "graph.h"
#include "../clique/clique.h"
#include "../clique/clique_csp_plus.h"
#include "../init_csp.h"
#include "../clique//filter_csp_plus.h"
#include "../common/common_clq.h"

using namespace std;

TEST(CSP_Plus, basic){
	LOG_INFO("CSP_Plus::basic---------------------------------------------");
	
	//Ugraph
	const int NV=5;
	ugraph ug(NV);				//C1={0, 1}, C2={2, 3}, C3={4}	, all consistent values
	ug.add_edge(0, 3);
	ug.add_edge(0, 4);
	ug.add_edge(1, 2);
	ug.add_edge(2, 4);
	ug.set_name("csp_file.txt");	/* to read csp declaration file */
	InitCSP myCSP(ug);
		
	myCSP.color_db();
	EXPECT_TRUE(myCSP.get_inc_val().empty());
	EXPECT_TRUE(myCSP.is_db_sat());

	ug.remove_edge(0,3);			//make inconsistent {3}
	myCSP.reset();
	myCSP.color_db();
	EXPECT_EQ(1, myCSP.get_inc_val().size());
	EXPECT_TRUE(myCSP.is_db_sat());
	
	ug.add_edge(0,3);
	ug.remove_edge(0,4);			//make inconsistent {4}
	myCSP.reset();
	myCSP.color_db();
	myCSP.print_inc_val();
	EXPECT_EQ(1, myCSP.get_inc_val().size());
	EXPECT_FALSE(myCSP.is_db_sat());

	ug.add_edge(0,3);
	ug.remove_edge(1, 2);			//make inconsistent {2}
	myCSP.reset();
	myCSP.color_db();
	myCSP.print_db();
	myCSP.print_inc_val();
	EXPECT_EQ(2, myCSP.nb_inc_val());						//uses all values, C1={0, 1}, C2={3} inc_val: {2, 4}
	EXPECT_FALSE(myCSP.is_db_sat());				
	
	LOG_INFO("-------------------------------------------------");
}

TEST(CSP_Plus, basic_order){
	LOG_INFO("CSP_Plus::basic_order---------------------------");
	//Ugraph
	const int NV=5;
	ugraph ug(NV);									//X1={0, 1}, X2={2, 3}, X3={4}	, {2} inconsistent with C1
	ug.add_edge(0, 3);
	ug.add_edge(0, 4);
	ug.add_edge(3, 4);				
	ug.set_name("csp_file.txt");					/* to read csp declaration file */
	int NVAR=3;
	InitCSP myCSP(ug);

	//color_db: C1={0, 1}, C2={3}, C3={4}			
	myCSP.color_db();					
	EXPECT_EQ(1, myCSP.nb_inc_val());
	EXPECT_TRUE(myCSP.is_db_sat());
	myCSP.print_db();
	
	vint new_ord;
	myCSP.new_csp_order(new_ord);
	EXPECT_EQ(NV, new_ord.size());					/* ordering is always over the total number of vertices */
	EXPECT_EQ(NV-1, new_ord[2]);					/* {2} is placed at the end */
	EXPECT_EQ(2, myCSP.get_inc_val()[0]);	

	stringstream sstr;
	LOG_INFO("\nNEW_ORD MISSING INCONSISTENT VAL-----------------");
	com::stl::print_collection(new_ord, sstr);
	LOG_INFO(sstr.str());
				
	LOG_INFO("-------------------------------------------------");
}

TEST(CSP_Plus, sort_basic){
	LOG_INFO("CSP_Plus::sort_basic-----------------------------");
	//Ugraph
	const int NV=5;
	ugraph ug(NV);				//C1={0, 1}, C2={2, 3}, C3={4}	, all consistent values
	ug.add_edge(0, 3);
	ug.add_edge(0, 4);
	ug.add_edge(1, 2);
	ug.add_edge(2, 4);	
	ug.set_name("csp_file.txt");	/* to read csp declaration file */
	InitCSP myCSP(ug);

	myCSP.color_db();
	myCSP.print_db();
	
	vint new_ord;
	myCSP.new_csp_order(new_ord);
	EXPECT_EQ(NV, new_ord.size());
	stringstream sstr;
	LOG_INFO("\nNEW_ORD----------------------------------------");
	com::stl::print_collection(new_ord, sstr);
	LOG_INFO(sstr.str());
	LOG_INFO("-------------------------------------------------");
	
	LOG_INFO("-------------------------------------------------");
}

TEST(CSP_Plus, frb30_15_1){
//////////////////
// tests with frb30_15_1 (derived from CSP)
// Number of vars: 30 Number of values for all vars = 15
		
	LOG_INFO("CSP_Plus::frb30_15_1--------------------------");
	
	//Ugraph
	ugraph ug("frb30-15-1.clq");
	int NV=ug.number_of_vertices();
	InitCSP myCSP(ug);
	myCSP.color_db();
	myCSP.print_db();

	//ordering
	vint new_ord;
	myCSP.new_csp_order(new_ord);
	EXPECT_EQ(NV, new_ord.size());
	stringstream sstr;
	LOG_INFO("\nNEW_ORD-------------------------------------------");
	com::stl::print_collection(new_ord, sstr);
	LOG_INFO(sstr.str());
	LOG_INFO("--------------------------------------------------");
		
	LOG_INFO("------------------------------------------------");
}


TEST(CSP_Plus,full_driver_basic){
	LOG_INFO("CSP_Plus::full_driver_basic--------------------------");
	//Ugraph
	const int NV=5;
	ugraph ug(NV);						//C1={0, 1}, C2={2, 3}, C3={4}	, all consistent values
	ug.add_edge(0, 3);
	ug.add_edge(0, 4);
	//ug.add_edge(1, 2);
	ug.add_edge(2, 4);	
	ug.add_edge(3, 4);
	ug.set_name("csp_file.txt");

	InitCSP myCSP(ug);
	
	//compute new ordering and CSP specification
	vint new_ord;
	InitCSP::csp_t res=myCSP.run(new_ord);
	EXPECT_EQ(InitCSP::CSP_OK,res);
	myCSP.print_db();
	myCSP.print_csp_dec();
			
	LOG_INFO("------------------------------------------------");
}

TEST(CSP_Plus,full_driver_frb30_15_1){
	LOG_INFO("CSP_Plus::full_driver_frb30_15_1--------------------------");
	
	ugraph ug("frb30-15-1.clq");
	InitCSP myCSP(ug);

	//compute new ordering and CSP specification
	vint new_ord;
	InitCSP::csp_t res=myCSP.run(new_ord);
	EXPECT_EQ(InitCSP::CSP_OK,res);
	myCSP.print_db();
	myCSP.print_csp_dec();
			
	LOG_INFO("------------------------------------------------");
}

TEST(CSP_Plus, integration_trim){
	LOG_INFO("CSP_Plus::integration_trim---------------------------");
	//Ugraph
	const int NV=5;
	ugraph ug(NV);									//X1={0, 1}, X2={2, 3}, X3={4}	, {2} inconsistent with C1 so  color_db: C1={0, 1, 2}, C2={3}, C3={4}
	ug.add_edge(0, 3);
	ug.add_edge(0, 4);
	ug.add_edge(3, 4);				
	ug.set_name("csp_file.txt");					/* to read csp declaration file */
	int NVAR=3;
	
	CliqueSort<ugraph> cs(ug);
	vint domains;
	bool is_unsat;
	int nb_inc_val;
	vint new_ord=cs.new_csp_order(domains, is_unsat, nb_inc_val);

	EXPECT_EQ(new_ord.size(), NV);
	EXPECT_FALSE(is_unsat);
	int total_size=0;
	for(vint::iterator it=domains.begin()+1; it!=domains.end(); it++){
		total_size+=(*it);
	}
	EXPECT_EQ(total_size,NV-1);						/* {2} from X2 is filtered*/

	stringstream sstr;
	LOG_INFO("\CSP-DEC-----------------------------------------");
	com::stl::print_collection(domains, sstr);
	LOG_INFO(sstr.str());
	LOG_INFO("--------------------------------------------------");
}

TEST(CSP_Plus, integration_frb30_15_1){
	LOG_INFO("CSP_Plus::integration_frb30_15_1-------------------");
	
	ugraph ug("frb30-15-1.clq");
	int NV=ug.number_of_vertices();

	CliqueSort<ugraph> cs(ug);
	vint domains;
	bool is_unsat;
	int nb_inc_val;
	vint new_ord=cs.new_csp_order(domains, is_unsat, nb_inc_val);				
	
	EXPECT_FALSE(is_unsat);
	EXPECT_EQ(new_ord.size(), NV);
	int total_size=0;
	for(vint::iterator it=domains.begin()+1; it!=domains.end(); it++){
		total_size+=(*it);
	}
	EXPECT_EQ(total_size,NV);
	LOG_INFO("------------------------------------------------");
}

TEST(CSP_Plus_framework, simplification){
	LOG_INFO("CSP_Plus_framework::simplification---------------------------------------------");
	
	//Ugraph
	const int NV=7;
	ugraph ug(NV);				//C1={0}, C2={1, 2}, C3={3, 4}, C4={5,6} , {2} not C1
	ug.add_edge(0, 1);
	ug.add_edge(0, 3);
	ug.add_edge(0, 4);
	ug.add_edge(0, 5);
	ug.add_edge(0, 6);
	ug.add_edge(1, 3);
	ug.add_edge(1, 4);
	ug.add_edge(1, 5);
	ug.add_edge(1, 6);
	ug.add_edge(2, 4);
	ug.add_edge(2, 6);
	ug.add_edge(3, 5);
	ug.add_edge(3, 6);
	ug.add_edge(4, 6);

	ug.set_name("csp_file_2.txt");	/* to read csp declaration file */

	//solve it with framework
	clqo::param_t param;
	param.init_order=clqo::CSP;
	param.alg=clqo::BBMC_CSP;
	CliqueCSP_Plus c(&ug, param);
	int sol=c.set_up();
	if(!sol) c.run();	
	EXPECT_EQ(4, c.get_max_clique());		//solution
	EXPECT_EQ(1, c.get_NB_INC_VAL());

	LOG_INFO("------------------------------------------------");
}


TEST(CSP_Plus_framework, simplification_2){
	LOG_INFO("CSP_Plus_framework::simplification_2------------");
	
	//Ugraph
	const int NV=6;
	ugraph ug(NV);				//C1={0}, C2={1}, C3={2, 3}, C4={4,5} , {3} not C2, {5} not C1
	ug.add_edge(4, 2);
	ug.add_edge(5, 3);
	ug.add_edge(4, 1);
	ug.add_edge(5, 1);
	ug.add_edge(4, 0);
	ug.add_edge(2, 1);
	ug.add_edge(2, 0);
	ug.add_edge(3, 1);
	ug.add_edge(1, 0);
	ug.set_name("csp_file_3.txt");	/* to read csp declaration file */

	//solve it with framework
	clqo::param_t param;
	param.init_order=clqo::CSP;
	param.alg=clqo::BBMC_CSP;
	CliqueCSP_Plus c(&ug, param);
	int sol=c.set_up();
	if(!sol) c.run();	
	EXPECT_EQ(4, c.get_max_clique());		//solution
	EXPECT_EQ(2, c.get_NB_INC_VAL());		//{3} {5}

	LOG_INFO("------------------------------------------------");
}

TEST(CSP_Plus_framework, frb_30_15){
	LOG_INFO("CSP_Plus_framework::frb_30_15-------------------");
	
	//Ugraph
	ugraph ug("frb30-15-1.clq");				
	
	//solve it with framework
	clqo::param_t param;
	param.init_order=clqo::CSP;
	param.alg=clqo::BBMC_CSP;
	CliqueCSP_Plus c(&ug, param);
	int sol=c.set_up();
	if(!sol) c.run();		
	EXPECT_EQ(30, c.get_max_clique());		//solution
  	EXPECT_EQ(0, c.get_NB_INC_VAL());

	LOG_INFO("------------------------------------------------");
}


TEST(CSP_Plus, Filter_CSP){
	LOG_INFO("CSP_Plus::Filter_CSP---------------------------------------------");
	
	//Ugraph
	const int NV=5;
	ugraph ug(NV);				//C1={0, 1}, C2={2, 3}, C3={4}	, all consistent values
	ug.add_edge(0, 3);
	ug.add_edge(0, 4);
	ug.add_edge(1, 2);
	ug.add_edge(2, 4);
	ug.set_name("csp_file.txt");	/* to read csp declaration file */
	//InitCSP myCSP(ug);

	FilterCSP_Plus f(&ug);
	/*comclq::range_t r;*/
		
	//myCSP.color_db();
	//EXPECT_TRUE(myCSP.get_inc_val().empty());
	//EXPECT_TRUE(myCSP.is_db_sat());

	//ug.remove_edge(0,3);			//make inconsistent {3}
	//myCSP.reset();
	//myCSP.color_db();
	//EXPECT_EQ(1, myCSP.get_inc_val().size());
	//EXPECT_TRUE(myCSP.is_db_sat());
	//
	//ug.add_edge(0,3);
	//ug.remove_edge(0,4);			//make inconsistent {4}
	//myCSP.reset();
	//myCSP.color_db();
	//myCSP.print_inc_val();
	//EXPECT_EQ(1, myCSP.get_inc_val().size());
	//EXPECT_FALSE(myCSP.is_db_sat());

	//ug.add_edge(0,3);
	//ug.remove_edge(1, 2);			//make inconsistent {2}
	//myCSP.reset();
	//myCSP.color_db();
	//myCSP.print_db();
	//myCSP.print_inc_val();
	//EXPECT_EQ(2, myCSP.nb_inc_val());						//uses all values, C1={0, 1}, C2={3} inc_val: {2, 4}
	//EXPECT_FALSE(myCSP.is_db_sat());				
	
	LOG_INFO("-------------------------------------------------");
}


//** TEST SIMPLE DE INCONSISTENCIA DEL CSP