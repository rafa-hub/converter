
// date_of_creation: 26/2/17
// last_update: 9/10/17


#include "gtest/gtest.h"
#include "graph/graph_gen.h"
#include "utils/common.h"
#include "graph.h"
#include "clique/clique.h"
#include "clique/clique_csp_plus.h"
#include "init_csp.h"
#include "clique//filter_csp_plus.h"
#include "common/common_clq.h"

#include "XCSP3CoreParser.h"
#include "XCSP3PrintCallbacks.h"

using namespace std;


	int main (int argc,char *argv[]){
	
	const int NV=5;
	XCSP3Core::XCSP3PrintCallbacks cb;
	ugraph ug(NV);

	
	if(argc!=2) 
     		throw std::runtime_error("usage: ./test xxxxcsp3instance.xml");
	try
	  {
    		XCSP3CoreParser parser(&cb);
    		parser.parse(argv[1]); // fileName is a string
  	}
 	 catch (exception &e)
  	{
    	cout.flush();
    	cerr << "\n\tUnexpected exxxception :\n";
    	cerr << "\t" << e.what() << endl;
    	exit(1);
  	}



	//Ugraph
	//C1={0, 1}, C2={2, 3}, C3={4}	, all consistent values
	
	ug.add_edge(0, 3);
	ug.add_edge(0, 4);
	ug.add_edge(1, 2);
	ug.add_edge(2, 4);
	


	return 0;
	
	}

