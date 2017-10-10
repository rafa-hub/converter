//interface_partition.h is an interface to use BBMC algorithms to find a partition of cliques in an input graph 
//Applications: SAT-PLANNING, LB for the Min Sum Coloring problem
//date_of_creation: 8/11/16
//last update: 14/02/17
//
// Current status: We have implemented a number of improvements compared to the reference paper [Wu & Hao 13] which
// never got to see the light. Specificaly we have used kcore, density, color, degree and pool of cliques to guide
// the search of EXCLIQUE [Wu & Hao 13] using an exact algorithm which finds a pool of maximum clique algorithms at each
// step. Results are comparable to EXCLIQUE and better for the wap family. There is still margin for improvement by finding
// a better heuristic which evaluates the existence of a big clique partition in the lookahead step. However, the recent
// paper [Jin & Hao 16] seems difficult to improve (it is a coloring evolutionary heuristic-CHECK) so for the time being
// I will stop developing
//
// Parameter: DEFAULT_MAX_NUM_SOL 
// Place: result.h
// Comment: Controls the number of cliques stored by CliqueALL. 
// Note: VIEW_PROGRESS should be set for the solver to stop once the maximum number of solutions have been reached.



#ifndef _PARTITION_INTERFACE_H_
#define _PARTITION_INTERFACE_H_

#include <vector>
#include "graph/graph.h"
#include "copt/clique/clique_all_max_sol.h"
#include "copt/clique/clique_infra.h"
#include "copt/clique_sort.h"


using namespace std;

////////////
//PARAMETERS

//#define DIVERSIFICATION 0			//adds random solutions when solutions exceed capacity

class Partition:public CLQParam{
	typedef vector<int> vint;
	static const int OK=0; static const int MY_ERROR=-1;
	
public:
	struct result_t{int w; int nb_steps; void print(ostream& o=cout){o<<"[w:"<<w<<","<<"s:"<<nb_steps<<"]"<<endl;}/*whatever*/};
	//struct param_t { alg_t alg; init_order_t io; /* whatever */}

private:
///////////
// data members
	const ugraph* pg;													//the real input graph				
	ugraph g;															//working copy (will be modified by cli)
	
	CliqueAll cli;				
	vector<vint> part;													//partition solution as a matrix of disjoint cliques	

	//auxiliary bitstrings for fast compatibility analysis
	bitarray bbclq1;
	bitarray bbclq2;
public:
//////////
// construction / setters and getters
	Partition						(ugraph* gout):pg(gout), g(*gout), cli(&g, clqo::param_t()), CLQParam( clqo::param_t()){/*init();*/}	
	Partition						(ugraph* gout, clqo::param_t p):pg(gout), g(*gout), cli(&g, p), CLQParam(p){/*init();*/}	
	Partition						(clqo::param_t p):pg(NULL), cli(p), CLQParam(p){}

	int set_graph					(ugraph* gout){pg=gout; g=*pg ; cli.Clique<ugraph>::set_graph(&g); return 0;}
	const vector<vint>&				get_partition() const{return part;}
	
	//main drivers
	int min_sum_col_LB				();	
	int min_sum_col_LB_heur			();	
	
	//I/O
	void print						(ostream& o=cout);	

	//batch framework
	int set_up					() {res.set_name(g.get_name()); return init(); }
	void tear_down				() {reset();}	
	void run					(); 


protected:
	int init();		//allocates memory, global 
	
	void reset()					{part.clear(); cli.get_result().clear(); cli.clear_decode();}
	void add_partition(vint p)		{part.push_back(p);}
	int compute_LB();	

	//basic interface: working well but does not use look_ahead
	vint build_and_solve_clique_graph(Result& r, ugraph& gcq);
	bool is_compatible(vint& clq1, vint& clq2);
	vint solve_clique_graph(ugraph& gcq);

	//lookahead (contains all the reasoning)
	vint build_and_solve_clique_graph_with_lookahead(Result& r, ugraph& gcq,  bitarray& bbs);
	vint solve_clique_graph_with_lookahead(ugraph& gcq, vector<vint>& clq, bitarray& bbs);		//computes all sols and evaluates the best one somehow
	vint eval(Result& rgcq, Decode& d, vector<vint>& clq, bitarray& bbs);
	double eval_pool(Result& r);
	double eval_pool(Result& r, bitarray& bbs);
	double eval_kcore(bitarray& bbr);
	vint pick_one(Result& r /*single clq*/, bitarray& bbs);
	int  pick_one(Result& r /*set of clqs*/, bitarray& bbs, vint& rsol);			//also adds cliques to partition solution 

	int diversify_pool(ugraph& g, Result&, bitarray& bbs);							//*TODO

	//heuristic solution (currently only for the unweighted case) 
	Result pool(ugraph& g, bitarray& bbs, int NSOL, int min_size=1, bool explicit_removal=false, int len_improve=1000);
	bool verify_pool(bitarray& bbs, Result& r);

	//adding a maximum graph generator that produces first those with a low common neighborhood
	Result& solve_main_graph_with_reordering(bitarray& bbs);
		
	//testing
public:
	bool verify();
protected:	
	bool is_clique(vint& v);		//subgraph
	bool is_partition();			
};

inline
bool Partition::is_compatible(vint& clq1, vint& clq2){
///////////////
// returns TRUE if clq1 and clq are disjoint

	//I. computed as bitstring intersection
	bbclq1.set_bit(clq1, true);
	bbclq2.set_bit(clq2, true);
	return bbclq1.is_disjoint(bbclq2);
	
	/*with allocation
	bitarray bb1(clq1, pg->number_of_vertices());
	bitarray bb2(clq2, pg->number_of_vertices());
	return bb1.is_disjoint(bb2);*/


	//II. computed as set intersection
	/*sort(clq1.begin(), clq1.end());
	sort(clq2.begin(), clq2.end());
	vint v3;
	set_intersection(clq1.begin(), clq1.end(), clq2.begin(), clq2.end(), back_inserter(v3));
	return v3.empty();*/
}

inline
Result& Partition::solve_main_graph_with_reordering(bitarray& bbs){
//////////////////////
// maximum graph generator of subgraph bbs that produces first max-cliques with a low common neighborhood first
// Outputs pool of cliques in result object of main driving CliqueAll object cli
//
// date_of_creation:30/11/12
//
//***Experimental-does not seem to have the impact expected

	int NV=g.number_of_vertices();
	ugraph gord(g);

	//removes neighbors of vertices not in BBS
	for(int v=0; v<NV; v++){
		if(bbs.is_bit(v)) continue;
		bitarray& bbn=gord.get_neighbors(v);
		bbn.init_scan(bbo::DESTRUCTIVE);
		while(true){
			int nv=bbn.next_bit_del();
			if(nv==EMPTY_ELEM) break;
			gord.remove_edge(nv,v);
		}
	}

	//graph solving parameters
	clqo::param_t myparam;
	myparam.alg=clqo::BBMCXR_L;
	myparam.init_order=clqo::MIN_WIDTH_MIN_TIE_STATIC;		//note that nodes not in BBS will (also) be placed first (actually last)
	myparam.init_preproc=clqo::init_preproc_t::UB_HEUR;

	//reorders
	CliqueAll clqall(&gord, myparam);
	clqall.set_up();

	//reorders bbs
	vint ord=clqall.get_decoder().get_first_ordering();		//[NEW_INDEX]-->[OLD_INDEX]
	clqall.get_decoder().reverse_in_place(ord);				//[OLD_INDEX]-->[NEW_INDEX]
	
	bitarray bbs_ord(NV);
	bbs.init_scan(bbo::NON_DESTRUCTIVE);
	while(true){
		int v=bbs.next_bit();
		if(v==EMPTY_ELEM) break;
		bbs_ord.set_bit(ord[v]);
	}
	
	//runs maxclique for bbs
	clqall.set_up_subgraph(bbs_ord);
	clqall.run_subgraph_with_ordering(bbs_ord, clqo::MAX_WIDTH, gbbs::place_t::PLACE_FL);
	
	//copy decoded results to main clique driving object
	Result& rcli=cli.get_result();
	rcli.get_all_solutions()=clqall.decode_all();
	
	//VERIFIES all nodes are in bbs
	//vector<vint> lsol=rcli.get_all_solutions();
	//for(int s=0; s<lsol.size(); s++){
	//	for(int elem=0; elem<lsol[s].size(); elem++){
	//		if(!bbs.is_bit(lsol[s][elem])){
	//			LOG_ERROR("bad vertex in pool of solutions:"<<" nsol:"<<s<<" elem:"<<elem<<" val:"<<lsol[s][elem]);
	//			cin.get();
	//		}
	//	}
	//}
	
	return rcli;
}

inline
void Partition::run () {
//////////////
// driver for batch execution
	
	res.tic();
	int lb=min_sum_col_LB_heur();
	res.toc();
	res.set_UB(lb);
	LOG_INFO("[lb:"<<res.get_upper_bound()<<","<<res.get_user_time()<<"s]");
}
	
inline
int Partition::diversify_pool(ugraph& g, Result& r, bitarray& bbs){
/////////////////////
// For a given graph and result with maximum storage, modifies result with a pool of
// heuristic AMTS cliques
//
// RETURNS -1 if Error or O if OK
	
	int MAX_SOL=r.get_MAX_NUM_SOL();
	LOG_INFO("POOL OF CLIQUES IN CLIQUE-GRAPH EXCEEDS MAXIMUM:"<<MAX_SOL);
	
	
	Result rpool=pool(g, bbs, MAX_SOL/2, 2 /* at least size 2*/);
	//Result rpool=pool(g, bbs, MAX_SOL, 2 /* at least size 2*/);
	//if(!verify_pool(bbs, rpool)){
	//	LOG_ERROR("verify_pool()-bizarre pool");
	//	return -1;
	//}
	vector<vint>& sol=rpool.get_all_solutions();
	//rpool.print_all_sol();
	//LOG_INFO("SIZE OF POOL: "<<sol.size());
	//cin.get();
	r.get_all_solutions().resize(MAX_SOL/2);		
	//r.get_all_solutions().resize(10);						//leaves a small sample
	for(int i=0; i<sol.size(); i++){
		if(r.add_solution(sol[i])==false) break;
	}

return 0;	
}

inline
Result Partition::pool(ugraph& g, bitarray& bbs, int NSOL, int min_size, bool explicit_removal, int len_improve){
/////////////////
//pool of solutions in bbs of size GT or EQ to min_size
	int NV=g.number_of_vertices();
	ugraph gc(g);	
	if(explicit_removal){
		bitarray bbrem(NV);
		bbrem.set_bit(0, NV-1);
		bbrem.erase_bit(bbs);
		gc.remove_vertices(bbrem);

		if(gc.number_of_vertices()!=bbs.popcn64()){
			LOG_ERROR("bizarre POOL");
		}

	}else{
		for(int i=0; i<NV; i++){
			if(!bbs.is_bit(i)){
				gc.get_neighbors(i).erase_bit();
				for(int j=0; j<NV; j++){
					if (j==i) continue;
					gc.remove_edge(j, i);
				}
			}
		}
	}

	//test empty graph
	if(gc.number_of_edges(false)==0) return Result();

	
	AMTSexec myAMTS2(NSOL,len_improve,1,-1);
	Result r=myAMTS2.pool_of_solutions(gc, NSOL, min_size);
	return r;
	
}


inline 
int Partition::init(){
/////////////////
// called ONCE at the begginning for each input graph: 
// allocates memory and computes lb and ub for the input subgraph 
	
	bbclq1.init(pg->number_of_vertices());
	bbclq2.init(pg->number_of_vertices());
	reset();
	clqo::param_t myparam;
	myparam.alg=clqo::BBMCXR_L;
	myparam.init_order=clqo::MIN_WIDTH;
	myparam.init_preproc=clqo::init_preproc_t::UB_HEUR;
	cli.set_param(myparam);
	cli.set_up();				
	
	return OK;
}

inline
int Partition::min_sum_col_LB(){
/////////////////
// computes clique partition
//
// returns LB for the min_sum_col problem
//
	
	int input_NV=g.number_of_vertices();
	bitarray bbs(input_NV);
	bbs.set_bit(0, input_NV-1);						//all nodes
	ugraph gclq;

	bool first_time=true;
	while(true){
		if(first_time){
			cli.run_subgraph(bbs);						/*no need for setup first time (use the strongest LBs and UBs)*/
			first_time=false;
		}else cli.run_subgraph_with_ordering(bbs, clqo::MIN_WIDTH, gbbs::place_t::PLACE_LF);	
		//vint lnodes=build_and_solve_clique_graph(cli.get_result(),gclq);	
		vint lnodes=build_and_solve_clique_graph_with_lookahead(cli.get_result(), gclq, bbs);
		bitarray bbres(lnodes, input_NV);
		bbs.erase_bit(bbres);
		if(bbs.is_empty()) break;
		cli.set_up_subgraph(bbs);					//computes strong UB and simple LB for subgraph bbs
		
	}

	//compute LB from partition
	int LB=compute_LB();
	return LB;
}

inline
int Partition::min_sum_col_LB_heur(){
/////////////////
// computes a clique partition
//
// returns LB for the min_sum_col problem
//	
	int input_NV=g.number_of_vertices();
	bitarray bbs(input_NV);
	bbs.set_bit(0, input_NV-1);						//all nodes
	ugraph gclq;
	

	bool first_time=true;
	while(true){
		if(first_time){
			//cli.run();	
			cli.run_subgraph(bbs);						/*no need for setup first time (use the strongest LBs and UBs)*/
			first_time=false;
		}else {
			//cli.run_subgraph_with_ordering(bbs, clqo::MAX_WIDTH, gbbs::place_t::PLACE_FL);
			solve_main_graph_with_reordering(bbs);

		}	
			
		Result& r=cli.get_result();
	
#ifdef DIVERSIFICATION
		int MAXSOL=r.get_MAX_NUM_SOL();
		if( (r.get_number_of_solutions()>= (MAXSOL-1)) && r.get_first_solution().size()>1 ){
			LOG_INFO("DIVERSIFYING POOL FOR CLIQUES----------");
			cin.get();
			diversify_pool(g, r, bbs);
			LOG_INFO("----------------------------------------------------------");
		}
#endif
	
		vint lnodes=build_and_solve_clique_graph_with_lookahead(r, gclq, bbs);
		
		bitarray bbres(lnodes, input_NV);
		bbs.erase_bit(bbres);
		if(bbs.is_empty()) break;
		cli.set_up_subgraph(bbs);					//computes strong UB and simple LB for subgraph bbs
	}
	

	//compute LB from partition
	int LB=compute_LB();
	return LB;
}



inline
vint Partition::build_and_solve_clique_graph(Result& r, ugraph& gcq){

	int SOL=r.get_number_of_solutions();
	vector<vint>& non_dec_sol=r.get_all_solutions();
	//***random_shuffle(non_dec_sol.begin(), non_dec_sol.end());
	

////////////
//quick tests for special situations: for 1-cliques
	vint res;
	if(SOL==1){
		//only one clique: no need to test compatibility
		res=non_dec_sol[0];
		add_partition(cli.get_decoder().decode_list(res));
		return res;
	}else if(r.get_upper_bound()==1){
		//all 1-cliques
		for(int i=0; i<non_dec_sol.size(); i++){
			add_partition(cli.get_decoder().decode_list(non_dec_sol[i]));
			res.insert(res.end(),non_dec_sol[i].begin(), non_dec_sol[i].end());
		}
		return res;
	}

///////////////////////////	
//creates compatibility graph and solves it
	gcq.init(SOL);
	for(int i=0; i<SOL-1; i++){
		for(int j=i+1; j<SOL; j++){
			if(is_compatible(non_dec_sol[i], non_dec_sol[j])) 
				gcq.add_edge(i, j);
		}
	}

	if(gcq.number_of_edges()!=0){
		vint gcqsol=solve_clique_graph(gcq);
		//returns set of vertices of the partition
		for(int i=0; i<gcqsol.size(); i++){
			add_partition(cli.get_decoder().decode_list(non_dec_sol[gcqsol[i]]));
			res.insert(res.end(),non_dec_sol[gcqsol[i]].begin(), non_dec_sol[gcqsol[i]].end());
		}
	}else{
		//no clique compatible: records first solution only
		res=non_dec_sol[0];
		add_partition(cli.get_decoder().decode_list(non_dec_sol[0]));
	}
		
	return res;
}

inline
vint Partition::build_and_solve_clique_graph_with_lookahead(Result& r, ugraph& gcq, bitarray& bbs){

	int SOL=r.get_number_of_solutions();
	vector<vint>& non_dec_sol=r.get_all_solutions();
	//***random_shuffle(non_dec_sol.begin(), non_dec_sol.end());


	////////////
	//quick tests for special situations: for 1-cliques
	vint res;
	if(SOL==1){
		//only one clique: no need to test compatibility
		res=non_dec_sol[0];
		add_partition(cli.get_decoder().decode_list(res));
		return res;
	}else if(r.get_upper_bound()==1){
		//all 1-cliques
		for(int i=0; i<non_dec_sol.size(); i++){
			add_partition(cli.get_decoder().decode_list(non_dec_sol[i]));
			res.insert(res.end(),non_dec_sol[i].begin(), non_dec_sol[i].end());
		}
		return res;
	}

	///////////////////////////	
	//creates compatibility graph and solves it
	gcq.init(SOL);
	for(int i=0; i<SOL-1; i++){
		for(int j=i+1; j<SOL; j++){
			if(is_compatible(non_dec_sol[i], non_dec_sol[j])) 
				gcq.add_edge(i, j);
		}
	}

	if(gcq.number_of_edges()!=0){
		LOG_INFO("SOLVING COMPATIBILITY GRAPH AND SELECTING BEST CLIQUE PARTITION---------------------");
		//cin.get();
		res=solve_clique_graph_with_lookahead(gcq, non_dec_sol, bbs);		/*partition updated inside*/
		LOG_INFO("-------------------------------------------------------------------------------------");
	}else{
		//all-incompatible, choose best according to lookahead
		res=pick_one(r,bbs);
		add_partition(cli.get_decoder().decode_list(res));
	}
		
	return res;

}

inline
vint Partition::pick_one(Result& r, bitarray& bbs){
////////////////
// pick best value of solutions in r according to the resulting 
// subgraph {bbs}\{sol}
		
	vint res;
	vector<vint>& sol=r.get_all_solutions();
	int NB_SOL;
	if( (NB_SOL=sol.size())==1 )  return sol[0];
	 	
	//more than one sol, choose best
	bitarray bbsol(g.number_of_vertices());
	bitarray bbrem(g.number_of_vertices());
//	InitColor<ugraph> ic(g);

	//evaluate values for all criteria for first solution
	int index=0;
	bool flag_found=false;
	while(index<NB_SOL){
		bbsol.set_bit(sol[index], true);
		ERASE(bbs, bbsol, bbrem);
		if(!bbrem.is_empty()){
			res=sol[index];
			flag_found=true;
			index++;  //used for later loop
		//	bbrem.print(); cout<<endl;
			break;
		}
		index++;
	}

	if(flag_found){
		double max_den=g.Graph<bitarray>::density(bbrem), den;
		double min_pc=bbrem.popcn64(), pc;
		//double max_den=g.number_of_edges(bbrem), den;
		double max_kcd=eval_kcore(bbrem), kcd;
		double max_po=0.0, po;
		Result rpool=pool(g, bbrem, 50, 2);
		if(rpool.get_number_of_solutions()>0){
			max_po=eval_pool(rpool/*,bbs*/);
		}
		

		//int max_col=ic.greedyIndependentSetColoring(bbrem), col;
		
		//loop over remaining solutions (must be at least one)
		for(int s=index; s<sol.size(); s++){
			bbsol.set_bit(sol[s], true);
			ERASE(bbs, bbsol, bbrem);
			//	bbrem.print(); cout<<endl;			

				//CRITERIA: I-kcore, II.density, III. pool of 50 sol
				/*kcd=eval_kcore(bbrem);
				if(max_kcd<kcd){
					max_kcd=kcd;
					res=sol[s];
				}else if(max_kcd==kcd){*/


		/*	col=ic.greedyIndependentSetColoring(bbrem);
			if(max_col<col){
				LOG_ERROR("COLOR RULE FIRED");
				max_col=col;
				res=sol[s];
				
			}else if(max_col==col){*/

				den=g.Graph<bitarray>::density(bbrem);
				//	}else den=g.number_of_edges(bbrem);
				if(max_den<den){
					max_den=den;
					res=sol[s];
				}else if(max_den==den){
					if(den!=0.0){
						Result rpool=pool(g, bbrem, 50, 2);
						if(rpool.get_number_of_solutions()>0){
							po=eval_pool(rpool/*,bbs*/);
							if(max_po<po){
								LOG_ERROR("POOL CRITERIA FIRED");
								max_po=po;
								res=sol[s];
							}
						}
					}else{
						//all singletons
						pc=bbrem.popcn64();
						if(min_pc>pc){
							LOG_ERROR("SIZE OF POPULATION FIRED");
							min_pc=pc;
							res=sol[s];
						}
					}
				}
		//	}


		}	
	}else return sol[0];		//resulting subgraph bbs\bb is always empty
	
	return res;
}

inline
int Partition::pick_one(Result& r, bitarray& bbs, vint& rsol){
////////////////
// pick best value of solutions in r according to the resulting 
// subgraph {bbs}\{sol}
//
// Compared with the other pick_one, here we add the separate cliques
// to the solution set
//
// RETURNS pointer to rsol in r
		
	rsol.clear();
	vector<vint>& sol=r.get_all_solutions();
	int NB_SOL, rindex;
	if( (NB_SOL=sol.size())==1 ){
		rsol=sol[0];
		return 0;
	}
	
	bitarray bbsol(g.number_of_vertices());
	bitarray bbrem(g.number_of_vertices());
	//InitColor<ugraph> ic(g);

	//evaluate values for all criteria for first solution
	int index=0;
	bool flag_found=false;
	while(index<NB_SOL){
		bbsol.set_bit(sol[index], true);
		ERASE(bbs, bbsol, bbrem);
		if(!bbrem.is_empty()){
		//	bbrem.print();
			rsol=sol[index];
			rindex=index;
			flag_found=true;
			index++;
			break;
		}
		index++;
	}


	if(flag_found){
		//bbrem non-empty: evaluate current sol
		double max_den=g.Graph<bitarray>::density(bbrem), den;
		double min_pc=bbrem.popcn64(), pc;
		double max_kcd=eval_kcore(bbrem), kcd;
		double max_po=0.0, po;
		Result rpool=pool(g, bbrem, 50, 2);
		if(rpool.get_number_of_solutions()>0){
			//LOG_ERROR("SIZE OF POOL: "<<rpool.get_number_of_solutions());
			max_po=eval_pool(rpool/*,bbs*/);
		}


		
		
	//	int max_col=ic.greedyIndependentSetColoring(bbrem), col;
		
		//loop over remaining solutions (must be at least one)
		for(int s=index; s<sol.size(); s++){
			bbsol.set_bit(sol[s], true);
			ERASE(bbs, bbsol, bbrem);
		//	bbrem.print();





			//CRITERIA: I-kcore, II.density, III. pool of 50 sol
		/*	kcd=eval_kcore(bbrem);
			if(max_kcd<kcd){
				max_kcd=kcd;
				rsol=sol[s];
				rindex=s;
			}else if(max_kcd==kcd){*/

		/*	col=ic.greedyIndependentSetColoring(bbrem);
			if(max_col<col){
				LOG_ERROR("COLOR RULE FIRED");
				max_col=col;
				rsol=sol[s];
				rindex=s;
			}else if(max_col==col){*/
				den=g.Graph<bitarray>::density(bbrem);
				if(max_den<den){
					max_den=den;
					rsol=sol[s];
					rindex=s;
				}else if(max_den==den){
					if(den!=0.0){
						Result rpool=pool(g, bbrem, 50, 2);
						if(rpool.get_number_of_solutions()>0){
							po=eval_pool(rpool/*,bbs*/);
							if(max_po<po){
								LOG_ERROR("POOL CRITERIA FIRED");
								max_po=po;
								rsol=sol[s];
								rindex=s;
							}
						}
					}else{ //all isolani
						pc=bbrem.popcn64();
						if(min_pc>pc){
							LOG_ERROR("SIZE OF POPULATION FIRED");
							min_pc=pc;
							rsol=sol[s];
						}
					}
				}
		//	}
			

		}	
	}else{//resulting subgraph bbs\bb is always empty
		rsol=sol[0];
		return 0;
	}
	
	return rindex;
}


inline
vint Partition::solve_clique_graph(ugraph& gcq){
//////////////////
// solves max clique un gcq and returns the vertices which make up the partial partition
// 
// REMARKS: 
// 1. uses the current best BBMC configuration (12/11/16)
// 2. updates partition data member with each clique separately
//

	clqo::param_t myparam;
	myparam.alg=clqo::BBMCRL_KMIN;
	myparam.init_preproc=clqo::UB_HEUR;
	myparam.init_order=clqo::MAX_WIDTH;				//more compatibility, more disjointness
	
	CliqueInfra ci(&gcq, myparam);
	if(ci.set_up()==0){
		ci.run();
	}
	
	return ci.decode_first_solution();
}

inline
vint Partition::solve_clique_graph_with_lookahead(ugraph& gcq, vector<vint>& clq, bitarray& bbs){
/////////////////
// solves clique graph and evaluates solutions to maximize density
//
// RETURNS a set of vertices in the bbs space which make up the (partial) partition of cliques

	clqo::param_t myparam;
	myparam.alg=clqo::BBMCXR_L;
	myparam.init_preproc=clqo::UB_HEUR;
	myparam.init_order=clqo::MAX_WIDTH;	
	
	CliqueAll ci(&gcq, myparam);
	ci.set_up();
	ci.run();

	int nbsol=ci.get_result().get_number_of_solutions();
	
	//one solution processing
	if(nbsol==1){
		vint res;
		vint gcqsol=ci.decode_first_solution();
		for(int i=0; i<gcqsol.size(); i++){
			add_partition(cli.get_decoder().decode_list(clq[gcqsol[i]]));
			res.insert(res.end(),clq[gcqsol[i]].begin(), clq[gcqsol[i]].end());
		}
		return res;
	}
	
	
#ifdef DIVERSIFICATION
	int maxsol=ci.get_result().get_MAX_NUM_SOL();
	if(nbsol>= (maxsol-1)){
		LOG_INFO("DIVERSIFYING POOL FOR CLIQUE COMPATIBILITY GRAPH----------");
		cin.get();
		diversify_pool(gcq, ci.get_result(), bbs);
		LOG_INFO("----------------------------------------------------------");
	}
#endif

	return eval(ci.get_result(), ci.CLQParam::get_decoder(), clq, bbs);
}

inline
int Partition::compute_LB(){
////////////////////
// Sum of Euler sums of each partial parition
	int LB=0;
	for(int p=0; p<part.size(); p++){
		int size=part[p].size();
		LB+=size*(size+1)/2;
	}
	
	LOG_INFO("size of partition: "<<part.size());
	cin.get();

	return LB;
}

inline
void Partition::print(ostream& o){
	int size=part.size();
	for(int s=0; s<size; s++){
		com::stl::print_collection(part[s], o); 
		if(s!=(size-1)) o<<endl;
	}
}

inline
bool Partition::verify(){
	
	for(int i=0; i<part.size(); i++){
		if(!is_clique(part[i])){
			LOG_ERROR("Partition::verify()-clique: "<<i<<" is not a clique");
			return false;
		}
	}
		
	if(!is_partition()){
		LOG_ERROR("Partition::verify()-incorrect partition");
		return false;
	}

	return true;
}

inline
bool Partition::is_clique(vint& v){
////////////////////
// compares against input graph proper

	for(int i=0; i<v.size()-1; i++)
		for(int j=i+1; j<v.size(); j++){
			if(!pg->is_edge(v[i], v[j])){
				return false;
			}
		}

	return true;
}

inline
bool Partition::is_partition(){
////////////////////
// Only checks if it is a cover

	if(part.empty()){
		LOG_INFO("Partition::is_partition() for empty partition");
		return true;
	}	

	//***count: pigeon hole principle 
	int count=0;
	for(int i=0; i<part.size(); i++){
		count+=part[i].size();
	}
	if(count!=pg->number_of_vertices()){
		LOG_ERROR("Partition::is_partition()-bizarre count:"<<count<<" NV:"<<pg->number_of_vertices());
		print(); cout<<endl;
		vint lv;
		for(int i=0; i<part.size();i++){
			lv.insert(lv.end(), part[i].begin(), part[i].end());
		}
		sort(lv.begin(), lv.end());
		com::stl::print_collection(lv);

	}

	bitarray bbv(pg->number_of_vertices());
	bbv.set_bit(0,pg->number_of_vertices()-1);
	for(int i=0; i<part.size(); i++)
		for(int j=0; j<part[i].size(); j++){
			bbv.erase_bit(part[i][j]);
		}

	if(!bbv.is_empty()){
		LOG_ERROR("Partition::is_partition()-incorrect partition");
		bbv.print(); cout<<endl;
		return false;
	}
	return true;
}


inline
vint Partition::eval(Result& rgcq, Decode& d, vector<vint>& clq, bitarray& bbs){
/////////////////
// evaluates each possible set of disjoint cliques with different criteria
// (currently maximizes degree in the remaining subgraph to be partitioned)
//
// REMARK
// solutions in rgcq should NOT be 1-cliques

	//decode all solutions (*** OPTIMIZE: too many copies; currently not working)
	Result r; 
	vint lv, dec_sol, rsol;
	vector<vint>& sol=rgcq.get_all_solutions();
	for(int i=0; i<sol.size(); i++){
		dec_sol=d.decode_list(sol[i]);
		lv.clear();
		for(int i=0; i<dec_sol.size(); i++){
			lv.insert(lv.end(),clq[dec_sol[i]].begin(),clq[dec_sol[i]].end());
		}
		r.add_solution(lv);
	}

	//verify
	//vector<vint>& ver_sol=r.get_all_solutions();
	//for(int i=0; i<ver_sol.size(); i++){
	//	for(int j=0; j<ver_sol[i].size(); j++){
	//		if(!bbs.is_bit(ver_sol[i][j])){
	//			LOG_ERROR("BIZARRE RESULT");
	//			cin.get();
	//		}
	//	}
	//}

	//evaluates and choose one solution in r
	//r.print_all_sol();
	int psol=pick_one(r, bbs, rsol);

	//decodes solution chosen and adds it to the final partition of cliques
	dec_sol=d.decode_list(sol[psol]);
	for(int i=0; i<dec_sol.size(); i++)
		add_partition(cli.get_decoder().decode_list(clq[dec_sol[i]]));
	return rsol;
}

bool Partition::verify_pool(bitarray& bbs, Result& r){
	vector<vint>& sol=r.get_all_solutions();
	for(int s=0; s<sol.size(); s++){
		vint& clq=sol[s];
		for(int i=0; i<clq.size(); i++){
			if(!bbs.is_bit(clq[i])){
				LOG_ERROR("bad member of pool");
				com::stl::print_collection(clq);
				cout<<endl<<"bbs:"<<endl;
				bbs.print();
				//cin.get();
				return false;
			}
		}
	}
	return true;
}

double Partition::eval_pool(Result& r){
/////////////////
// average size of pool
	int size=0;
	vector<vint>& sol=r.get_all_solutions();
	for(int s=0; s<sol.size(); s++){
		size+=sol[s].size();
	}

	return size/sol.size();		//can be 0
}


double Partition::eval_pool(Result& r, bitarray& bbs){
///////////////////////
// Returns the number of vertices covered by the pool
// 
// REMARKS: in the tests: not working properly

	int size=0, NV=bbs.popcn64();
	bitarray bbt(bbs);
	bbt.set_bit(0, NV-1);
	bitarray bbsol(bbs);

	vector<vint>& sol=r.get_all_solutions();
	for(int s=0; s<sol.size(); s++){
		bbsol.set_bit(sol[s],true);
		bbt.erase_bit(bbsol);
	}


	return NV-bbt.popcn64();
}

double Partition::eval_kcore(bitarray& bbr){
	KCore<ugraph> k(g, &bbr);
	k.kcore();

	int kmax=0, kmin=0;
	int KN=k.get_kcore_number();

	if(KN>3){
		double HALF_KN=KN/2;
		for(int i=HALF_KN; i<=KN; i++){
			kmax+=k.get_kcore_size(i);
		}
		for(int i=0; i<HALF_KN; i++){
			kmin+=k.get_kcore_size(i);
		}
		if(kmin!=0)
		//	return max(kmax-kmin,0);
			return kmax/(float)kmin;
		else return kmax;
	}else if (KN==0) return 0.0;		//all singletons
	else{
		//not singletons but kcore at most 3
		kmax=k.get_kcore_size(KN);
		kmin=k.get_kcore_size(0);
		if(kmin!=0)
			//return max(kmax-kmin,0);
			return kmax/(float)kmin;
		else return kmax;
	}

	return kmax;		//should not reach here
}

#endif

