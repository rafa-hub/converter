//clique_partition.h:  interface for clique partition algorithm. Compared with interface_partitition.h the algorithms are simpler. 
//                     At each step a maximum clique is found and removed from the graph. 
//					   Will also possibly include a greedy clique partition algorithm
//
//Applications: Lower bound for Clique Interdiction
//init_date: 3/10/17
//last update: 4/10/17
//
// Comments:
// 1. Only for ugraph type
// 2. Uses CliqueInfraPlus algorithm for max_clique processing of each subgraph

////////////
// Instructions of use

//switch PARTITION_SETUP in clique_types.h MUST BE ON!!!!!!!!!!!

#ifndef _PARTITION_CLIQUE_H_
#define _PARTITION_CLIQUE_H_

#include <vector>
#include "graph/graph.h"
#include "copt/clique/clique_infra_plus.h"
#include "copt/clique_sort.h"
#include "graph/algorithms/graph_map.h"
#include "copt/setup.h"
#include "./cover.h"

using namespace std;

//////////////
//
// CliquePartition
// (Computes a clique partition of maximum cliques)
//
///////////////				
class CliquePartition {
	static const int  VIEW_PROGRESS_RATE=1000;					/* show output after aprox. VIEW_PROGRESS_RATE iterations */

private:
///////////
// data members
	ugraph* g_ori;												//the input graph (WILL NOT CHANGE)				
	ugraph g;													//the one and only graph processed (will be sorted)
	
	CliqueInfraPlus cli;				
	GraphMapSingle gm;											/* for decoding-actually not really necessary*/
	cover_t<vint> part;											/* vector<vint>- the one and only partition */		

public:
	CliquePartition			(ugraph* gout):g_ori(gout), cli(clqo::param_t()) {}		
	const cover_t<vint> &	get_partition	() const						 {return part;}
	const cover_t<vint>& compute_partition	();		

	ostream& print					(ostream& o=cout);	
	bool verify();															/* does not check disjointness */

protected:
	int first_clique						();								/* includes allocation!*/
	const cover_t<vint>& next_cliques		(); 
};

inline
ostream& CliquePartition::print	(ostream& o){
	part.print(o);
	return o;	
}

inline
const cover_t<vint>& CliquePartition::compute_partition (){
/////////////////
// driver (one and only)
	Result r; /* to compute time*/
	LOG_INFO("----------STARTING PARTITIONING -----------------");
	r.tic();
				
	if(first_clique()==-1){
		part.cover.clear();
		return part;
	}
	next_cliques();

	r.toc();
	LOG_INFO("PARTITIONING FINISHED-"<<"t:"<<r.get_user_time());
	return part;
}

inline
const cover_t<vint> & CliquePartition::next_cliques () {
//////////////
// computes remaining maximum cliques (except the first one already in 'part')
		
	SETUP<CliqueInfraPlus> st(cli);
	const int NV=g.number_of_vertices();
	bitarray bbsg(NV);											/* remaining subproblem */
	bbsg.set_bit(0, NV-1);											
	bitarray bbf(cli.get_result().get_first_solution(), NV);    /* nodes removed */
	bbsg.erase_bit(bbf);
	int PC_LEVEL=NV;	
	vint vsol;
			
	int lb=0, ub=NV, pc=bbsg.popcn64();
	while(!bbsg.is_empty()){
		
		//lower bound for(bbs)
		Result& r= cli.get_result();
		ub=r.get_upper_bound();									/* ub in the past iteration is also an ub for this iteration */
		r.clear_all_solutions();								/* does not clear UB- used in the next iteration */
		vint vset;
		lb=st.lower_bound(bbf,vset);
		r.add_solution(vset);
		r.set_LB(lb);

		if(lb==ub){												/* catches w=1, possibly w=2 */
		//	LOG_INFO("LB=PREVIOUS UB!");
		//	LOG_INFO("w:["<<lb<<"]");
			vint& clq=r.get_first_solution();
			for(int i=0; i<clq.size(); i++){					/* manual erase */
				bbsg.erase_bit(clq[i]);
				bbf.set_bit(clq[i]);
				pc--;
			}
			cli.decode_first_solution(vsol);
			part.add_set(vsol);				
			continue;
		}
		
		//upper bound
		KCore<ugraph> kc(g);
		kc.set_subgraph(&bbsg);
		kc.kcore();
		ub=min<int>(ub,kc.get_kcore_number()+1);
		r.set_UB(ub);
		
		if(lb==ub){
		//	LOG_INFO("w:["<<lb<<"]");
			vint& clq=r.get_first_solution();
			for(int i=0; i<clq.size(); i++){					/* manual erase */
				bbsg.erase_bit(clq[i]);
				bbf.set_bit(clq[i]);
				pc--;
			}	
					
			cli.decode_first_solution(vsol);
			part.add_set(vsol);				
			continue;
		}

		if(st.partitioning(lb, bbsg)==0){
		//	LOG_INFO("w:["<<lb<<"]");
			vint& clq=r.get_first_solution();												
			for(int i=0; i<clq.size(); i++){					/* manual erase */
				bbsg.erase_bit(clq[i]);	
				bbf.set_bit(clq[i]);
				pc--;
			}

			cli.decode_first_solution(vsol);
			part.add_set(vsol);	
			continue;
		}
					
		cli.Clique::fix_lb(lb);									/* sets maxno -MUST BE*/
		cli.run();
		vint& clq=r.get_first_solution();
		for(int i=0; i<clq.size(); i++){						/* manual erase */
			bbsg.erase_bit(clq[i]);
			bbf.set_bit(clq[i]);
			pc--;
		}	

		cli.decode_first_solution(vsol);
		part.add_set(vsol);		
		
		//I/O
		if(pc<PC_LEVEL){
			LOG_INFO("n="<<pc);
			PC_LEVEL-=CliquePartition::VIEW_PROGRESS_RATE;		
		}
	}
	
	return part;
}

inline 
int CliquePartition::first_clique(){
/////////////////
// allocates memory, sorts graph and solves maximum clique for input graph
//
// COMMENTS
// 1.stores max clique as first element of 'part'

	part.cover.clear();				
	clqo::param_t param;
	param.alg=clqo::BBMCL;
	param.init_preproc=clqo::UB;							/* no UB_HEUR! while AMTS is not tailored for subgraphs */
	vint vsol;

	const int NV=g.number_of_vertices();
	SETUP<CliqueInfraPlus>::init_sort(*g_ori, g, cli.get_decoder(),gm, SETUP<CliqueInfraPlus>::MIN_WIDTH); /*also MIN_WIDTH_BIG*/
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
	info.set(clqo::search_alloc_t::ALLOC_COLOR_SETS);						/* iop */
	info.remove(clqo::search_alloc_t::ALLOC_COLOR_LABELS);					/* CliqueInfraPlus specific */
	info.size=ub;
	if(cli.search_allocation(info)==-1){									/* error during allocation */
		LOG_ERROR("InterdictionPlus::init()-bad allocation, interdiction will fail!");
		return -1;
	}

//////////////////
// solve!
	if(lb==ub){
		cli.decode_first_solution(vsol);
		part.add_set(vsol);		
		return 0;
	}

	if(st.partitioning(lb)==0){
		cli.decode_first_solution(vsol);
		part.add_set(vsol);	
		LOG_INFO("w:["<<lb<<"]");
		return 0;
	}	

	//TODO-run!
	cli.Clique<CliqueInfraPlus::g_type>::fix_lb(lb);							/* sets maxno -MUST BE*/
	cli.run();
	cli.decode_first_solution(vsol);
	part.add_set(vsol);	
	
	return 0;  //ok
}

inline
bool CliquePartition::verify(){
	return part.is_clq_partition(*g_ori);
}


#endif

