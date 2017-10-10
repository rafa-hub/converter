//init_csp.h: header for a specialization of InitColor to determine the inital order of vertices of a constraint graph Gc
//			  derived from a CSP problem. The input order of vertices (values of variables in the CSP) in Gc MUST BE sequential
//            with respect to values of variables (i.e. <values of X1> <values of X2> etc. where X1, X2 etc are variables in 
//			   the domain in any order 
//date of creation: 27/2/17
//last update: 7/3/17
//author: pss

#ifndef __INIT_CSP_H__
#define __INIT_CSP_H__

#include <algorithm>
#include <vector>
#include "utils/logger.h"
#include "graph/algorithms/graph_sort.h"
#include "graph/algorithms/graph_func.h"
#include "init_color_sort.h"
#include "common/common_clq.h"

using namespace std;
using namespace comclq;
typedef vector<int> vint;

///////////////////////////
//
// InitCSP class
// (to pre-process CSP derived graphs)
//
// Does not modify the graph
////////////////////////////
class InitCSP{
	
	static vint read_csp_dec(string filename);
	
	struct max_deg_first{
		max_deg_first(vint& map_out):ord(map_out){}
		bool operator()(int lhs, int rhs) const {return ord[lhs] <  ord[rhs];}  
	private:
		vint& ord;		//OLD->NEW
	};

	struct min_deg_first{														
		min_deg_first(vint& map_out):ord(map_out){}
		bool operator()(int lhs, int rhs) const {return ord[lhs] >  ord[rhs];}  
	private:
		vint& ord;		//OLD->NEW
	};

	struct biggest_first{
		bool operator()(const vint& lhs,  const vint& rhs) const  { return lhs.size() > rhs.size(); } 
	};

	struct biggest_first_only_unit{
		bool operator()(const vint& lhs,  const vint& rhs) const  { return (lhs.size() > rhs.size() && rhs.size()==1); } 
	};

	enum rlf_t {LT_NVAR=0, EQ_NVAR, GT_NVAR};
public:
	enum csp_t {CSP_ERROR=-1, CSP_OK, CSP_UNSAT};

public:
	InitCSP							(ugraph& gout);
	virtual	~InitCSP				() 						{}

	const vint& get_inc_val			()						{return inc_val;}		
	const vector<vint>& get_db		()						{return db;}	
	const vint& get_domains			()						{return domains;}	
	void reset						()						{inc_val.clear();  clean_db(); }	//testing-cannot clear domains
	int nb_inc_val					()						{return inc_val.size();}
	int nb_csp_var					()						{return NB_CSP_VAR;}

	//public for testing / should be private
	int color_db					();														//computes coloring-may filter values
											
//functionality
	csp_t run						(vint& new_ord);										//MAIN DRIVER
	int new_csp_order				(vint& new_ord);										//computes desired new order

//tests / checks
	bool is_db_sat					(); 
	bool check_domains				();
	bool check_domains				(vint& nodes_dif_var);									//*** TODO checks if nodes belong to dif var
//	bool check_iset					(bitarray& bb);

//I/O
	void print_db					();
	void print_inc_val				();
	void print_csp_dec				();														//domain sizes

private:	
	
	void add_color					(int col, int v)			{db[col].push_back(v);}
	int compute_ord					(vint& deg_ord);										//OLD to NEW
	void db_2_domains				();														//updates domains with db info

	//sort
	int sort_db						();														//sort driver
	int sort_db_by_deg				();														//sorts each color set according to compute_ord
	void sort_db_by_size			();	
	
	void clean_db					();														//for tests, cleans color sets, not color set allocation

	
public:
	//rlf_ordering
	int sort_rlf					(vint& new_ord, vint& dom_sizes, const double MIN_DENSITY=0.7);	
	int sort_rlf					(vint& new_ord, vint& dom_sizes, vint& tail_rlf_dec, const double MIN_DENSITY=0.7);	
	//void rlf_2_domains			(vint& rlf_dom);										//updates domains with rlf coloring

private:
//////////////
// data members
	ugraph& g;														//will not be changed!
	const int NV;
	int NB_CSP_VAR;
	vector<vint > db;												//[COL=1 to NB_CSP_VAR][NB_VAL], as many colors as NB_CSP_VAR
	vint inc_val;													//inconsistent values; will not go into db
	vint domains;													//[COL>=1], cardinality of values for each variable  [COL=0]->NB_CSP_VAR 
};

inline
bool InitCSP::check_domains (){
/////////////////////
// Checks file csp domain specification. 
// 
// RETURNS TRUE if every variable is indeed a
// feasible color set over the range of values 
// declared in the specification file

	if(domains.empty()){
		LOG_ERROR("InitCSP::check_domains ()-over empty domain");
		return false;
	}
	bitarray bb(NV);
	
	int offset=0;
	for(int var=1; var<domains.size(); var++){
		bb.erase_bit();
		bb.set_bit(offset, offset+domains[var]-1);
		offset+=domains[var];
		if(gfunc::is_iset(g, bb)==false) 
						return false;
	}
	return true;
}

inline
bool InitCSP::check_domains	(vint& nodes_dif_var){
//////////////////////////////
// RETURNS TRUE is all nodes belong to different variables
//
// date_of_creation: 12/5/17 (plane to Stockholm)
// 
// REMARKS: 
// 1. domains must be available
// 2. does not assume any order in nodes_dif_var
// 3. potential source of inefficiency in big graphs: determines variable ranges 
//
	int v_first=0, v_last;

	//1. generates ranges 
	vector<range_t> RANGES;
	RANGES.assign(NV,range_t());
	for(vint::iterator it=domains.begin()+1; it!=domains.end(); it++){
		v_last=v_first+*it-1;
		for(int v=v_first; v<=v_last; v++){
			RANGES[v].vl=v_first;
			RANGES[v].vh=v_last;
		}
		v_first=v_last+1;
	}
	
	//I/O.TEST
	/*for(vector<range_t>::iterator it=RANGES.begin(); it!=RANGES.end(); it++){
		it->print();
		cin.get();
	}	*/
	/////////////
		
	//2. for each node check if the remaining nodes are out of the range O(N*N) operation
	for(vint::iterator it=nodes_dif_var.begin(); it!=nodes_dif_var.end(); it++){
		v_first=RANGES[*it].vl; v_last=RANGES[*it].vh;
		if(v_first!=v_last){
			LOG_INFO("InitCSP::check_domains-SIZE OF DOMAIN NOT SINGLE");
			for(vint::iterator it1=it+1; it1!=nodes_dif_var.end(); it1++){
				if(*it1>=v_first && *it1<=v_last){
					LOG_INFO("InitCSP::check_domains-UNSAT CSP");
					return false;
				}
			}
		}
	}

	//3. check if tail forms a clique [OPTIONAL-MUST BE BY DEFINITION OF TAIL]
	/*if(!gfunc::is_clique(g,nodes_dif_var)){
		LOG_INFO("InitCSP::check_domains-UNSAT CSP but bizarre");
		return false;
	}*/
		
	return true;
}


inline
InitCSP::InitCSP(ugraph& gout):g(gout), NB_CSP_VAR(-1), NV(g.number_of_vertices()){
//////////////////
// reads data from csp specification
	
	string str_name=g.get_name();
	string str=g.get_path();
	if(!str.empty()) str+=str_name;
	else str=str_name;
	
	size_t pos=str.find_last_of(".");
	str.replace(pos+1, string::npos, "csp");
	domains=InitCSP::read_csp_dec(str);

	if(domains.empty()){
		LOG_ERROR("InitCSP-error during construction when reading csp declaration file");
		return;
	}
	
	if(!check_domains()){
		LOG_ERROR("InitCSP-error during construction, csp var-val are not isets");
		domains.clear();
		return;
	}
	
	//all
	NB_CSP_VAR=domains.front();
	db.resize(NB_CSP_VAR+1);

	//TEST
	/*vint ord,dom;
	int res=sort_rlf(ord,dom);
	if(res==EQ_NVAR){
		LOG_INFO("RLF is optimal");
	}else if(res==LT_NVAR){
		LOG_INFO("RLF-CSP is UNSAT");
	}
	cin.get();*/
}

inline 
InitCSP::csp_t InitCSP::run(vint& new_ord){
///////////////////
// Driver for CSP order computation
// I.  sets_db by greedy coloring (removes inconsistent values)
// II. reads csp_specification from file if is_csp_dec=TRUE
// III.trims db if is_csp_dec=TRUE (removes more inconsistent values)
// 
// RETURNS CSP_OK, CSP_ERROR, CSP_UNSAT
//
// PARAMS: is_csp_dec-reads csp specifications from file
		
	if(color_db()==InitCSP::CSP_ERROR) return InitCSP::CSP_ERROR;
	//trim_db();
	
	if(!is_db_sat()) {
		LOG_INFO("InitCSP::run-CSP_UNSATISFIABLE");
		return CSP_UNSAT;
	}
	if(new_csp_order(new_ord)==InitCSP::CSP_ERROR) return InitCSP::CSP_ERROR;
	return InitCSP::CSP_OK;
}

inline
void InitCSP::db_2_domains(){
//////////////////
// updates domains with current db
//
// COMMENTS: use when the final order of the db is decided to store
//           final variable domains (already trimmed)

	domains.assign(NB_CSP_VAR+1,-1);
	for(int c=1; c<=NB_CSP_VAR; c++){
		domains[c]=db[c].size();
	}
	domains[0]=NB_CSP_VAR;
}

//inline
//void InitCSP::rlf_2_domains(vint& rlf_dom){
////////////////////
//// updates domains determined by RLF sorting
//
//	//domains.assign(NB_CSP_VAR+1,-1);
//	//domains[0]=NB_CSP_VAR;
//	copy(rlf_dom.begin(), rlf_dom.end(), domains.begin()+1);
//	domains[0]=NB_CSP_VAR;	
//}



inline
vint InitCSP::read_csp_dec(string filename){
///////////////
// parses a csp-dec format file
// date: 28/02/17
//
// RETURNS vector[NUM_VAR /*1 based */]->number of values or EMPTY vector if error
//          currently vector[0]=NUMBER_OF_VARIABLES
//
// COMMENTS- I. line error checking missing II. primitive check
		
	char line [255], token;
	int NVAR=0,  var_curr=0, nb_val, nb_var=0;
	vint res; 
	ifstream f(filename, ifstream::out);
	if(!f){
		LOG_INFO("InitCSP::read_csp_dec()-cannot open:"<<filename);
		return res;
	}
	
	//parsing all lines
	while(!f.eof()){
		f.getline(line, 255);
		if(line[0]=='\0' || line[0]=='\n') continue;
		
		//read token: protocol
		stringstream sstr(line);
		sstr>>token;
		switch(token){
		case 'c':
		//	cout<<line;
			break;
		case 'x':
			sstr>>NVAR;
			res.push_back(NVAR);		/* first element is the number of variables */
			break;
		case 'v':
			sstr>>var_curr>>nb_val;
			res.push_back(nb_val);
			nb_var++;
			break;
		default:
			LOG_ERROR("InitCSP::read_csp_dec()-error in csp protocol");
			vint vempty;
			swap(res, vempty);
			return res;
		}
			
		//exit on error
		//if(error_flag) break;
	}

	//check
	if(nb_var!=NVAR){
		LOG_ERROR("InitCSP::read_csp_dec()-number of variables read incorrect");
		vint v_empty;
		swap(res, v_empty);
	}

	//cout<<filename<<endl;
	return res;
}

inline
int InitCSP::new_csp_order(vint& new_ord){
////////////////
// full driver which computes the new ordering:
// I.  color classes internally ordered by non increasing deg (degenerate based)
// II. color_classes sorted according to non increasing size
//
// TO BE CALLED AFTER color_db()
//
// date: 27/02/17
//
// RETURNS 0 OK, -1 ERROR  (new_ord will be empty)
//
// COMMENTS: degree ordering is min-width based and is done prior to any computation
//			 in particular: colors are sorted by size AFTERWARDS
//
// POSSIBLE IMPROVEMENTS: sort colors by size first and then do the sorting by degree
//                        refer to that sorting to order color classes internally

	//swap(new_ord, vint());
	if(sort_db()==-1) return InitCSP::CSP_ERROR;		/* outputs db*/
	new_ord.assign(g.number_of_vertices(), -1);
	int k=0;
	
	for(int c=1; c<=NB_CSP_VAR; c++){
		if(!db[c].empty()){
			for(vint::iterator it=db[c].begin(); it!=db[c].end(); it++){
				new_ord[(*it)]=k++;
			}
		}else {
			LOG_INFO("InitCSP::new_csp_order-not consistent, empty variables");
			vint vempty;
			swap(new_ord, vempty);
			return InitCSP::CSP_ERROR;
		}
	}

	//fill remaining vertices at the end of the ordering
	if(!inc_val.empty()){
		int offset=NV-inc_val.size();
		for( vint::const_iterator it=inc_val.begin(); it!=inc_val.end(); it++){
			new_ord[(*it)]=offset++;
		}
	}
	
	db_2_domains();		//stores current ranges of variables
	return 0;
}

inline
int InitCSP::sort_db(){
/////////
// driver for all sort operations
	if(sort_db_by_deg()==-1) return InitCSP::CSP_ERROR;
	sort_db_by_size();

	return 0;
}

inline
int InitCSP::sort_rlf (vint& new_ord, vint& domain_sizes, const double MIN_DENSITY){
//////////////////////
// tests with RLF Sort
//
// PARAMS: new_ord: the new rlf ordering[OLD_INDEX]->NEW_INDEX (or EMPTY if conditions are not met)
//         domain_sizes: sizes of the RLF coloring, 0 BASED (i.e. domain_sizes[0] is the size of the first color class etc.)  
//
// RETURNS the difference between the nb of CSP var and the nb of colors obtained by RLF
// Note: A 0 indicates an optimal coloring, <0 the problem is trivially UNSAT, >0 coloring is not valid 
//       (according to the simplifications used in the main CSP as MCP solver)
//        
// date of creation: 30/04/17

	bool is_good_tail;
	
	if(g.density()<=MIN_DENSITY){
		LOG_INFO("RLF coloring not possible becuase density is less than:"<<MIN_DENSITY);
		vint empty_v;
		swap(new_ord, empty_v);
		swap(domain_sizes, empty_v);
		return 1;
	}
			
	InitColorSort cs(g);

	//TESTING
	//cs.recursiveLargestFirst_INC_CSP(vint(), NB_CSP_VAR);

	domain_sizes=cs.recursiveLargestFirst_INC(new_ord, is_good_tail);
	int nb_var_expected=domain_sizes.size();

	//I/O: optional
	LOG_INFO("-----------------------------");
	stringstream sstr;
	com::stl::print_collection(new_ord, sstr); sstr<<endl;
	com::stl::print_collection(domain_sizes, sstr); sstr<<"["<<nb_var_expected<<"]"<<endl;
	LOG_INFO(sstr.str());
	LOG_INFO("-----------------------------");
	
	return(nb_var_expected-NB_CSP_VAR);	
}

inline
int InitCSP::sort_rlf (vint& new_ord, vint& domain_sizes, vint& tail_rlf_dec, const double MIN_DENSITY){
//////////////////////
// tests with RLF Sort
//
// PARAMS: new_ord: the new rlf ordering[OLD_INDEX]->NEW_INDEX (or EMPTY if conditions are not met)
//         domain_sizes: sizes of the RLF coloring, 0 BASED (i.e. domain_sizes[0] is the size of the first color class etc.)
//         tail_rlf_dec: list of nodes that make up the RFL tail of the coloring, referred to the original graph
//
// RETURNS the difference between the nb of CSP var and the nb of colors obtained by RLF
// Note: A 0 indicates an optimal coloring, <0 the problem is trivially UNSAT, >0 coloring is not valid 
//       (according to the simplifications used in the main CSP as MCP solver)
//        
// date of creation: 30/04/17

	bool is_good_tail;
	
	if(g.density()<=MIN_DENSITY){
		LOG_INFO("RLF coloring not possible becuase density is less than:"<<MIN_DENSITY);
		vint empty_v;
		swap(new_ord, empty_v);
		swap(domain_sizes, empty_v);
		return 1;
	}
			
	InitColorSort cs(g);

	//TESTING
	//cs.recursiveLargestFirst_INC_CSP(vint(), NB_CSP_VAR);
	
	domain_sizes=cs.recursiveLargestFirst_INC(new_ord, tail_rlf_dec, is_good_tail);
	int nb_var_expected=domain_sizes.size();

	//I/O: optional
	LOG_INFO("-----------------------------");
	stringstream sstr;
	com::stl::print_collection(new_ord, sstr); sstr<<endl;
	com::stl::print_collection(domain_sizes, sstr); sstr<<"["<<nb_var_expected<<"]"<<endl;
	LOG_INFO(sstr.str());
	LOG_INFO("-----------------------------");
	
	return(nb_var_expected-NB_CSP_VAR);	
}

inline
void InitCSP::sort_db_by_size(){
/////////////
// color sets with largest size first

	stable_sort(db.begin()+1, db.end(), biggest_first());
	//stable_sort(db.begin()+1, db.end(), biggest_first_only_unit());
}

inline
int InitCSP::sort_db_by_deg(){
/////////////
// sorts EACH COLOR CLASS according to min_width_last 

	vint deg_ord(NV, -1);
	if(compute_ord(deg_ord)==InitCSP::CSP_ERROR){
		LOG_INFO("InitCSP::sort_db()-impossible to sort db");		
		return InitCSP::CSP_ERROR;
	}
		
	//max_deg_first pred(deg_ord);	
	min_deg_first pred(deg_ord);		/* reverses ordering */
	
	for(int c=1; c<=NB_CSP_VAR; c++)
		stable_sort(db[c].begin(), db[c].end(), pred);

	return 0;
}

inline
int InitCSP::compute_ord(vint& deg_ord){
////////////////
// computes a min_width ordering LAST to FIRST
// with (as usual) deg_ord[OLD_VERTEX]->NEW_VERTEX
//
// RETURNS  0 ok, -1 ERROR

	GraphSort<ugraph> gs(g);
	deg_ord=gs.new_order(gbbs::sort_t::MIN_DEG_DEGEN_TIE_STATIC,gbbs::place_t::PLACE_LF);

	if(deg_ord.size()!=NV) return InitCSP::CSP_ERROR;
	else return 0;
}

inline
bool InitCSP::is_db_sat(){
////////////
// RETURNS TRUE if CSP is SATISFIABLE, FALSE UNSAT
	for(int c=1; c<=NB_CSP_VAR; c++){
		if(db[c].empty()){
			LOG_INFO("color: "<<c<<" empty");
			return false;
		}
	}
	
	return true;
}

inline
void InitCSP::clean_db(){
///////////////
// cleans color sets (does NOT erase initial color allocation which should be fixed)
	for(vector<vint>::iterator it=db.begin(); it!=db.end(); it++){
		it->clear();
	}
}

inline
int InitCSP::color_db(){
/////////////
// sequential greedy coloring of consistent vertices (values) --> to db 
// inconsistent vertices(values) stored in inc_val (will not go into db)
//
// PARAMS: is_csp_dec=true -> reads csp declaration file
//
// RETURNS: number of inconsistent bva vertices (0 if no inconsistent values are found), -1 if ERROR
		
	int pc=NV, col=1, v=EMPTY_ELEM, w=EMPTY_ELEM,from=EMPTY_ELEM, offset=0;
	bool first_color=true;
	bitarray bb_unsel(NV), bb_sel(NV);
	//bitarray bb_inc_val(NV);

	//initial values
	clean_db();
	vint vempty;
	swap(inc_val,vempty); 
	bb_unsel.set_bit(0,NV-1);
	
	//algorithm
	while(true){
		bb_sel=bb_unsel;
		bb_sel.init_scan(bbo::DESTRUCTIVE);		
		first_color=true;
		while(true){
			if((v=bb_sel.next_bit_del(from, bb_unsel))==EMPTY_ELEM) 
				break;

			//update initial vertex offset
			if(first_color){
				first_color=false;
				offset=v+domains[col];	
			}else if(v>=offset){
					LOG_PRINT("["<<v<<","<<"col:"<<col<<","<<"vmin:"<<offset-domains[col]<<","<<"vmax:"<<offset-1<<"]");
					inc_val.push_back(v);
					if((--pc)==0)	
						return inc_val.size();
					continue;
			}

			// consecutive vertices inside the domain
			add_color(col, v);
			if((--pc)==0)	
				return inc_val.size();
												
			//computes next vertex of the current color class
			bb_sel.erase_block(from, g.get_neighbors(v));
		}
	++col;
	}
	
	return InitCSP::CSP_ERROR;					//ERROR: should not reach here
}


inline 
void InitCSP::print_db(){
	LOG_INFO("DB COLORS-----------------");
	stringstream sstr("");
		
	for(int i=1;i<db.size(); i++){
		com::stl::print_collection(db[i], sstr);
		LOG_INFO(sstr.str());
		sstr.str("");
		sstr.clear();
	}
	
	LOG_INFO("-------------------------");
}

inline 
void InitCSP::print_inc_val(){
	LOG_INFO("INCONSISTENT VALUES-------");
	stringstream sstr("");
	com::stl::print_collection(inc_val, sstr);
	LOG_INFO(sstr.str());
	LOG_INFO("-------------------------");
}

inline 
void InitCSP::print_csp_dec(){
	LOG_INFO("CSP-DEC-------");
	stringstream sstr("");
	com::stl::print_collection(domains, sstr);
	LOG_INFO(sstr.str());
	LOG_INFO("-------------------------");
}

#endif 

