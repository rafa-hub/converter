//covern.h:  interface a general purpose cover type cover_t. 
//                     
//Applications: Clique Partitioning
//init_date: 7/10/17
//last update: 7/10/17
//
// /* TODO-extend functionality to a partition of bitstrings */

#ifndef _GENERAL_COVER_TYPE_H_
#define _GENERAL_COVER_TYPE_H_

#include <vector>
#include "graph/graph.h"

using namespace std;

template <class set_t>										/* type of individual collection of nodes */
struct cover_t{
////////////////
// General template for cover/partitioning problems	
//
	typedef vector<set_t> vset;
	vset cover;
/////////////
// interface
	int number_of_elements();								 /* aggregate sum */
	void add_set (set_t& s)	{cover.push_back(s);}
	bool operator!() const	{return cover.empty();}			  /*empty test*/
	
	bool is_cover (int N);
	bool is_partition (int N);

	template<class Graph_t>
	bool is_clq_cover(const Graph_t& g);			

	template<class Graph_t>
	bool is_clq_partition(const Graph_t& g);		
	
	//I/O
	ostream& print(ostream& o=cout);
private:
	bool pigeon_hole_test_cover(int N);
	bool pigeon_hole_test_partition(int N);	
};

template <typename set_t>
int cover_t<set_t>::number_of_elements(){
	int nb=0;
	for(int i=0; i<cover.size(); i++){
		nb+=cover[i].size();
	}
	return nb;
}

template <typename set_t>
bool cover_t<set_t>::pigeon_hole_test_cover(int N){
	int nb_elem=number_of_elements();
		
	if(nb_elem<N) return false;
	return true;	
}

template <typename set_t>
bool cover_t<set_t>::pigeon_hole_test_partition(int N){
	int nb_elem=number_of_elements();
		
	if(nb_elem!=N) return false;
	return true;	
}

template <typename set_t>
template <class Graph_t>
inline
bool cover_t<set_t>::is_clq_cover(const Graph_t& g){
	if(!pigeon_hole_test_cover(g.number_of_vertices())){
		return false;
	}		
	//pigeon hole check using a bitstring
	const int N=g.number_of_vertices();
	bitarray bbv(N);
	bbv.set_bit(0,N-1);
	for(int i=0; i<cover.size(); i++){
		if(!gfunc::is_clique_edge_based(g,cover[i])) {
			LOG_INFO("cover_t<set_t>::is_clq_cover()not_a_clique");
			return false;		
		}
		for(int j=0; j<cover[i].size(); j++){
			bbv.erase_bit(cover[i][j]);
		}
	}
	if(!bbv.is_empty()){
		LOG_INFO("cover_t<set_t>::is_cover()-does not cover all nodes");
		return false;
	}
	return true;
}

template <typename set_t>	
template <class Graph_t>
inline
bool cover_t<set_t>::is_clq_partition(const Graph_t& g){
	if(!pigeon_hole_test_partition(g.number_of_vertices())){
		return false;
	}	
	//pigeon hole check using a bitstring
	const int N=g.number_of_vertices();
	bitarray bbv(N);
	for(int i=0; i<cover.size(); i++){
		if(!gfunc::is_clique_edge_based(g,cover[i])) {
			LOG_INFO("cover_t<set_t>::is_clq_partition()not_a_clique");
			return false;		
		}
		for(int j=0; j<cover[i].size(); j++){
			if(bbv.is_bit(cover[i][j])){			/* checks if the element is already covered */
				return false;				
			}
			bbv.set_bit(cover[i][j]);
		}
	}
	if(bbv.popcn64()!=N){
		LOG_INFO("cover_t<set_t>::is_clq_partition()-does not cover all nodes");
		return false;
	}
	return true;
}

template <typename set_t>
inline
ostream& cover_t<set_t>::print(ostream& o){
	for(int i=0; i<cover.size(); i++){
		com::stl::print_collection(cover[i], o);
	//	cover[i].print(o); 
		o<<endl;
	}
	return o;
}

template <>
inline
ostream& cover_t<bitarray>::print(ostream& o){
	for(int i=0; i<cover.size(); i++){		
		cover[i].print(o); 
		o<<endl;
	}
	return o;
}

template <typename set_t>	
inline
bool cover_t<set_t>::is_cover(int N){
////////////////
// tests cover for elements 0-(N-1)

	if(!pigeon_hole_test_cover(N)){
		return false;
	}
	
	//pigeon hole check using a bitstring
	bitarray bbv(N);
	bbv.set_bit(0,N-1);
	for(int i=0; i<cover.size(); i++)
		for(int j=0; j<cover[i].size(); j++){
			bbv.erase_bit(cover[i][j]);
		}

	if(!bbv.is_empty()){
		LOG_INFO("cover_t<set_t>::is_cover()-does not cover all nodes");
		return false;
	}
	return true;
}

template <typename set_t>	
inline
bool cover_t<set_t>::is_partition(int N){
////////////////
// tests partition for elements 0-(N-1)

	if(!pigeon_hole_test_partition(N)){
		return false;
	}
	
	//pigeon hole check using a bitstring
	bitarray bbv(N);
	for(int i=0; i<cover.size(); i++){
		for(int j=0; j<cover[i].size(); j++){
			if(bbv.is_bit(cover[i][j])){			/* checks if the element is already covered */
				return false;				
			}
			bbv.set_bit(cover[i][j]);
		}
	}

	if(bbv.popcn64()!=N){
		LOG_INFO("cover_t<set_t>::is_partition-does not cover all nodes");
		return false;
	}
	return true;
}

#endif

