//interface_interdiction.h is an interface to use BBMC algorithms for the clique interdiction problem
//date_of_creation: 4/11/16
//last update: 5/11/16


//**** IMPORTANT- NOT USE WITH CLIQUE_INFRA (kmin variants) BUG-RELATED TO MEMORY ISSUES IN PMAX_SAT FILTER WHEN REMOVING NODES (5/10/17)****/

#ifndef _INTERDICTION_INTERFACE_H_
#define _INTERDICTION_INTERFACE_H_

#include <vector>
#include "graph/graph.h"
#include "copt/clique/clique_infra.h" 



using namespace std;

template <typename clique_alg_t>
class Interdiction{
	static const int OK=0, MY_ERROR=-1;
	
	struct edge_t{ int u; int v; edge_t(int u, int v): u(u), v(v){}};

	typedef vector<edge_t> vedge;
	typedef vector<int> vint;

public:		
	enum alg_t{SIMPLE=0, PMAX_SAT, LARGE_SPARSE, MASSIVE_SPARSE /*whatever*/};
	enum init_mode_t{STRONG=0, MEDIUM, TRIVIAL};
	ugraph& get_graph(){return g;}		//only for manipulation purposes
	
public:
	struct result_t{int w; int nb_steps; vector<int> sol; void print(ostream& o=cout){o<<"[w:"<<w<<","<<"s:"<<nb_steps<<"]"<<endl;}/*whatever*/};
	
///////////
// data members

private:
	ugraph* pg;															//pointer to the input graph (type ugraph, unweighted)				
	ugraph g;															//copy of the input graph
	vint node_stack;
	vedge edge_stack;
	
	clique_alg_t cli;
	vint initial_sol;
	int init_ub;

	init_mode_t  init_mode;												//STRONG/MEDIUM/TRIVIAL computation of bounds during pre-processing
	alg_t			alg;												//SIMPLE, PMAX_SAT, LARGE_SPARSE, MASSIVE_SPARSE
public:
//////////
// public interface
	Interdiction(ugraph* gout):pg(gout), g(*gout), cli(&g, clqo::param_t()), alg(SIMPLE), init_ub(0), init_mode(Interdiction::TRIVIAL){}			//an ordered graph (typically by MIN_WIDTH)
	void set_alg(alg_t alg)  {this->alg=alg;}
	void set_init_mode(init_mode_t mode){init_mode=mode;}
	void set_alg_mode(alg_t alg){this->alg=alg;}
	
	void remove_nodes(int* S, int size);								//eliminates list of nodes (interdicts nodes and adds them to node_stack to restore them later);
	void remove_edges(int* heads, int* tails, int size);				//eliminates list of edged (interdicts edges and adds them to edge_stack to restore them later);
	
	int init();															//allocates memory, configures clique algorithm etc... for all possible interdictions
	void set_up();
	result_t run();														//runs clique with algorithm alg; upon termination will restore the nodes or edges that are in the stacks

protected:
	void restore_edges	();												//restores edges in graph and cleans the edge stack
	void restore_nodes	();												//restores nodes in graph and cleans the node stack
	void restore_all	()	{restore_nodes(); restore_edges();}

	int update_initial_sol();	
	int compute_initial_sol();	
};

template<typename clique_alg_t>
inline	
void Interdiction<clique_alg_t>::remove_nodes(int* S, int size){
/////////////////
//runs through the set of nodes, stores them in the stack and removes neighbors
//
//(7/2/17): nodes are not removed any more. They are removed from the solver when running the clique
// search
	
	node_stack.assign(S, S+size);			//copies the nodes	(7/2/17)

	//node_stack.clear();
	//for(int i=0; i<size; i++){
	//	node_stack.push_back(S[i]);
	//}

//	com::stl::print_collection(node_stack);
	
//	node_stack.clear();
//	for(int i=0; i<size; i++){
//		int node=S[i];
//		node_stack.push_back(node);
////		g.get_neighbors(node).erase_bit();		
//	}
}

template<typename clique_alg_t>
inline	
void Interdiction<clique_alg_t>::remove_edges(int* heads, int* tails, int size){
/////////////////
//runs through the set of nodes, stores them in the stack and removes degrees
	int u, w;
	edge_stack.clear();
	for(int i=0; i<size; i++){
		u=heads[i]; w=tails[i];
		edge_stack.push_back(edge_t(u, w));
	}

	g.remove_edge(u, w);
}

template<typename clique_alg_t>
inline
void Interdiction<clique_alg_t>::restore_nodes (){
	//for(int i=0; i<node_stack.size(); i++){
	//	g.get_neighbors(node_stack[i]).set_bit(pg->get_neighbors(node_stack[i]));
	//}
	node_stack.clear();
}

template<typename clique_alg_t>
inline
void Interdiction<clique_alg_t>::restore_edges (){
	for(int i=0; i<edge_stack.size(); i++){
		edge_t e=edge_stack[i];
		if(pg->is_edge(e.u, e.v)) 
				g.add_edge(e.u, e.v);
	}
	edge_stack.clear();
}

template<typename clique_alg_t>
inline 
int Interdiction<clique_alg_t>::init(){
/////////////////
// called ONCE at the beginning (for all interdictions)
// RETURNS -1 if error, 0 if ok
	
	clqo::param_t param;
	const int NV=g.number_of_vertices();

	param.init_order=clqo::init_order_t::NONE;				//graph is already ordered
	switch(alg){
	case SIMPLE:
		param.alg=clqo::BBMC;								//(for Clique<ugraph>) 
		break;
	case PMAX_SAT:
	//	param.alg=clqo::BBMCRL_KMIN;						//(for CliqueInfra) 
	//	param.alg=clqo::BBMCR_KMIN;							//(for CliqueInfra) 
		param.alg=clqo::BBMCR;

		break;
	/*case LARGE_SPARSE:
	case MASSIVE_SPARSE:*/
	default:
		LOG_ERROR("Interdiction<clique_alg_t>::init()-Unkwnown algorithm");
		return MY_ERROR;
	}

	cli.set_param(param);									//clique lower and upper bound are not taken from param
			
	//initial upper bound 
	InitColorUB c(g);
	int* rcol=new int[NV];

	LOG_INFO("Interdiction<clique_alg_t>::init()-Computing initial UB");
	switch(init_mode){
	case STRONG:
		c.Compute_UB_enhanced_last(rcol);
		break;
	case MEDIUM:
		c.Compute_incUB(rcol);
		break;
	case TRIVIAL:
		c.Compute_trivial_UB(rcol);
		break;
	default:
		LOG_ERROR("Interdiction<clique_alg_t>::init()-incorrect initial bound");
		return MY_ERROR;
	}

	int max=0;
	for(int i=0; i<NV; i++){
		if(max<rcol[i])
				max=rcol[i];
	}
		
	cli.get_result().set_UB(max);
	init_ub=max;						//stores initial UB; remains fixed from now on

	//allocate memory taking into account the UB
	LOG_INFO("Interdiction<clique_alg_t>::init()-allocating memory based on UB:"<<init_ub);
	clqo::search_alloc_t info;
	info.set(clqo::search_alloc_t::ALLOC_COLOR_SETS);
	info.size=init_ub;
	if(cli.search_allocation(info)==-1){
		LOG_ERROR("Interdiction<clique_alg_t>::init()-Error during allocation");
		return MY_ERROR;
	}

	//copy the root coloring
	int* root_col=cli.get_root_coloring();
	for(int v=0; v<NV; v++){
		root_col[v]=rcol[v];
	}
	delete [] rcol; rcol=NULL;


	//initial feasible solution-lb (with AMTS)
	LOG_INFO("Interdiction<clique_alg_t>::init()-computing initial solution AMTS");
	compute_initial_sol();
		
	return OK;
}

template<typename clique_alg_t>
inline
typename Interdiction<clique_alg_t>::result_t Interdiction<clique_alg_t>::run(){
/////////////////
// runs clique algorithm for the current interdicting values in the stacks
//
// currently leaves UB as in the input graph

	 result_t res;
	 update_initial_sol();
	 cli.get_result().add_solution(initial_sol);					/* sets feasible solution as final solution, in case it is not improved */
	 cli.get_result().set_UB(init_ub);								/* sets UB of the original graph */

	 //*** conditionally update initial upper bounds (currently left as in the original graph)
	 set_up();
	 int lb=cli.get_result().get_lower_bound();
	 int ub=cli.get_result().get_upper_bound();
	
	/*cout<<"LB: "<<lb<<endl;
	  cout<<"UB: "<<ub<<endl;*/
	 cli.fix_lb(lb);	
	 if(ub>lb){
		cli.run();
	 }else{
		cli.get_result().set_UB(cli.get_result().get_lower_bound());
	 }
	 
	 res.w=cli.get_result().get_upper_bound();
	 res.nb_steps=cli.get_result().number_of_steps();
	 res.sol=cli.decode_first_solution();
	
	 //cout<<"\n[w:"<<c.get_result().get_upper_bound()<<"]"<<"["<<c.get_result().number_of_steps()<< "]"<<endl;
	 
	 restore_all();				//restores graph context
	 return res;
}
template<typename clique_alg_t>
inline
int Interdiction<clique_alg_t>::update_initial_sol(){
//////////////////
// 1. compares initial solution with node or edge stack and computes a lower bound based on
// the amount of nodes interdicted from the initial solution
// 2. stores initial solution (without the removed nodes) as best solution
//
//
// ***TODO: edge interdiction


	//interdicted nodes
	vector<int> feasible_sol;
	vector<int> bitstring(g.number_of_vertices(), 0);
	for(int i=0; i<node_stack.size(); i++){
		bitstring[node_stack[i]]=1;
	}

	/*cout<<endl;
	com::stl::print_collection(node_stack);

	cout<<endl;
	com::stl::print_collection(initial_sol);*/

	
	int counter=0;
	bool flag_recompute=false;
	for(int i=0; i<initial_sol.size(); i++){
		if(bitstring[initial_sol[i]]==1){
			counter++;
		}else{
			feasible_sol.push_back(initial_sol[i]);
		}
	}
	
	
	//cout<<"COMMON NODES INITIAL SOLUTION:"<<counter<<endl;
	int lb=	initial_sol.size()-counter;
	cli.fix_lb(lb);		
	cli.get_result().set_LB(lb);
	cli.get_result().clear_all_solutions();
	cli.get_result().add_solution(feasible_sol);
	return counter;
}

template<typename clique_alg_t>
inline
int Interdiction<clique_alg_t>::compute_initial_sol(){
//////////////
// RETURNS size of initial feasible solution

	//init_ub with lean memory storage: finds a simple initial clique iteratively
	if(init_mode==TRIVIAL){
		const int NV=g.number_of_vertices();
		initial_sol.clear();
		bitarray bb(NV); 
		bb.set_bit(0, NV-1);
		bb.init_scan(bbo::DESTRUCTIVE);
		while(true){
			int v=bb.next_bit_del();
			if(v==EMPTY_ELEM) break;
			initial_sol.push_back(v);
			bb&=g.get_neighbors(v);
		}
		cli.get_result().add_solution(initial_sol);	
	}else{

		//AMTS manages a lot of memory
		AMTSexec a(RESTARTS, ITERATIONS_PER_RESTART);
		cli.get_result().set_LB(a.run(g, true));							/* stores size of feasible solution */
		initial_sol=a.get_nodes();
		cli.get_result().add_solution(initial_sol);							/* stores nodes of feasible solution */	
	}

	return initial_sol.size();
}

template<typename clique_alg_t>
inline
void Interdiction<clique_alg_t>::set_up(){
///////////////
// updates initial nodes and edges
	bitarray& bbr=cli.Clique<ugraph>::get_root_bitstring();
	Clique<ugraph>::nodelist_t& lr = cli.Clique<ugraph>::get_root_nodelist();

	int NV=g.number_of_vertices();
	bbr.set_bit(0, NV-1);
	
	//nodes
	if(!node_stack.empty()){
		
		for(int i=0; i<node_stack.size(); i++){
			int node=node_stack[i];
			bbr.erase_bit(node);
		}

		/*cout<<"root bistring"<<endl;
		bbr.print();*/

		vector<int> bitstring(NV, 0);
		for(int i=0; i<node_stack.size(); i++){
			bitstring[node_stack[i]]=1;
		}

	
		lr.index=-1;
		for(int node=0; node<NV; node++){
			if(bitstring[node]==0)
				lr.nodos[++lr.index]=node;
		}


		/*cout<<"list of nodes at root: "<<endl;
		for(int i=0; i<=lr.index; i++){
			cout<<lr.nodos[i]<<" ";
		}
		cout<<"-------------------- "<<endl;*/
	}else{
		lr.index=-1;
		for(int node=0; node<NV; node++){
			lr.nodos[++lr.index]=node;
		}
	}


	//edges
	for(int i=0; i<edge_stack.size(); i++){
		edge_t e=edge_stack[i];
		g.remove_edge(e.u, e.v);
	}

	
	cout<<endl<<"-------------"<<endl;


}

#endif

