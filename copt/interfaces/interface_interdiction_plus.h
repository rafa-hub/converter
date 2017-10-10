//interface_InterdictionPlus_plus.h is an interface to use BBMC algorithms for the clique InterdictionPlus problem
//date_of_creation: 4/11/16
//last update: 5/11/16


//IMPORTANT!!!! SWITCH PARTITION_SETUP in clique_types.h MUST BE ON				


#ifndef _INTERDICTION_PLUS_INTERFACE_H_
#define _INTERDICTION_PLUS_INTERFACE_H_

#include <vector>
#include "graph/graph.h"
#include "copt/clique/clique_infra.h"										/* deprecated */
#include "copt/clique/clique_infra_plus.h"
#include "graph/algorithms/graph_map.h"
#include "../setup.h"

using namespace std;

typedef vector<int> vint;

class InterdictionPlus{
public:										
	struct result_t{
		int w, nb_steps; 
		vint sol; 
		void print(ostream& o=cout)				{o<<"[w:"<<w<<","<<"steps:"<<nb_steps<<"]"<<endl;}
		void set(int w, int steps, vint sol /* TO CHECK REFERENCE */)	{this->w=w; this->nb_steps=steps; this->sol=sol;}	/*vint& sol (3d arg.) not compiling in LINUX: how many copies am I making?*/
	};
	
///////////
// data members
private:
	ugraph*g_ori;															//input graph-WILL NOT BE CHANGED! 			
	ugraph g;																//the one and only sorted graph
		
	CliqueInfraPlus cli;
	GraphMapSingle gm;
	vint node_stack;														//nodes to interdict					
										
public:
//////////
// public interface
	InterdictionPlus(ugraph* gout):g_ori(gout), cli(clqo::param_t()) {}		
	CliqueInfraPlus& get_alg()	 {return cli;}
	ugraph& get_graph()			 {return g;}	
	GraphMapSingle& get_map()     {return gm;}	

	void remove_nodes(int* S, int size);			/* fills node_stack */
	
	result_t init();																							
	void set_up(){;}								/* currently not used */
	result_t run();																													

protected:
	void restore_nodes		();						
};

inline	
void InterdictionPlus::remove_nodes(int* S, int size){
/////////////////
// stores forbidden nodes after mapping L->R

	node_stack.clear();
	for(int i=0; i<size; i++){
		node_stack.push_back(gm.map_l2r(S[i]));
	}

	//node_stack.assign(S, S+size);			//deprecated
}

inline
void InterdictionPlus::restore_nodes (){
	node_stack.clear();
}

inline 
InterdictionPlus::result_t InterdictionPlus::init(){
/////////////////
// called ONCE at the beginning (for all InterdictionPluss)
//
// RETURNS res.w=0 if error
// 
// Stages (note- no root partitioning): 
// 1.sorting
// 2.lower bound, upper bound
// 3. memory allocation (always, even if lb=ub)

	result_t res;
	clqo::param_t param;
	param.alg=clqo::BBMCL;
//	param.alg=clqo::BBMCL_R;
	param.init_preproc=clqo::UB;				/* no AMTS while it is not tailored for subgraphs */
	const int NV=g_ori->number_of_vertices();
	
	SETUP<CliqueInfraPlus>::init_sort(*g_ori, g, cli.get_decoder(), gm);
	cli.set_graph(&g);
	cli.set_param(param);
	
	int lb=0, ub=NV;
	SETUP<CliqueInfraPlus> st(cli);
	
	//lower bound
	Result& r= cli.get_result();
	r.clear();
	vint vset;
	lb=st.lower_bound(vset);
	r.add_solution(vset);
	r.set_LB(lb);
	
	//upper bound
	KCore<ugraph> kc(g);
	kc.kcore();
	ub=kc.get_kcore_number()+1;
	r.set_UB(ub);

	//allocation (even if lb=ub)
	clqo::search_alloc_t info;
	info.set(clqo::search_alloc_t::ALLOC_COLOR_SETS);			/* iop */
	info.remove(clqo::search_alloc_t::ALLOC_COLOR_LABELS);		/* CliqueInfraPlus specific */
	//info.size=ub;
	info.size=NV;
	if(cli.search_allocation(info)==-1){						/* error during allocation */
		LOG_ERROR("InterdictionPlus::init()-bad allocation, interdiction will fail!");
		res.w=0;
		res.nb_steps=0;
		return res;
	}

///////////////////
// solve!
	if(lb==ub){
		LOG_INFO("w:["<<lb<<"]");
		res.set(lb,0, cli.CLQParam::decode_first_solution());
		return res;
	}

	if(st.partitioning(lb)==0){
		r.set_UB(lb);
		LOG_INFO("w:["<<lb<<"]");
		res.set(lb,0, cli.decode_first_solution());
		return res;
	}	

	//TODO-run!
	cli.Clique::fix_lb(lb);							/* sets maxno -MUST BE*/
	cli.run();
	res.set(cli.get_result().get_upper_bound(),cli.get_result().number_of_steps(), cli.decode_first_solution());
		
	return res;
}

inline
InterdictionPlus::result_t InterdictionPlus::run(){
/////////////////
// runs clique algorithm for the current interdicting values in the stacks
	
	result_t res;

	SETUP<CliqueInfraPlus> st(cli);
	int NV=g.number_of_vertices();
	Result& rcli=cli.get_result();
	rcli.clear();

	if(node_stack.empty()){
		LOG_ERROR("InterdictionPlus::run()-interdicting an empty set of  nodes!");
	}

	bitarray bbf(node_stack, NV);				/*forbidden set*/
	bitarray bbsg(NV);
	bbsg.set_bit(0,NV-1);
	bbsg.erase_bit(bbf);
		
	//lower bound(interdicted nodes)
	vint vset;
	int lb=st.lower_bound(bbf,vset);
	rcli.add_solution(vset);
	rcli.set_LB(lb);	

	//upper bound(bbsg)
	KCore<ugraph> kc(g);
	kc.set_subgraph(&bbsg);
	kc.kcore();
	int ub=kc.get_kcore_number()+1;
	rcli.set_UB(ub);

	
	if(lb==ub){
		LOG_INFO("w:["<<lb<<"]");
		res.set(lb,0, cli.decode_first_solution());
		restore_nodes();
		return res;
		
	}

	if(st.partitioning(lb,bbsg)==0){
		LOG_INFO("w:["<<lb<<"]");
		res.set(lb,0, cli.decode_first_solution());
		restore_nodes();
		return res;
	}

	LOG_INFO("w:["<<lb<<","<<ub<<"]");

	cli.Clique::fix_lb(lb);		/* sets maxno -MUST BE*/
	cli.run();
	res.set(cli.get_result().get_upper_bound(),cli.get_result().number_of_steps(), cli.decode_first_solution());
	restore_nodes();
	return res;
}

#endif

