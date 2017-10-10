/* mnts.h */
#ifndef MNTS_H
#define MNTS_H

using namespace std;

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <string.h>
#include <time.h>
#include <ctime>
#include <vector>
#include <string.h>

//clique dependencies
#include "graph/graph.h"

class mnts{
public:
//char * File_Name;
int **Edge;   // adjacent matrix
int *vectex;
int *funch;
int *address;
int *tabuin;
int Max_Vtx,  Max_Iter; 
int f;
int fbest;
int *adjaclen;
int **adjacMatrix;
int *cruset;
int len;
int tm1;
int tm2;
int *C0;
int *C1;
int *We;
int *BC;
int len0;
int len1;
int *TC1;
//int Iter;
int TABUL;
int Wf;
int Wbest;
int *FC1;
int *Tbest;
int *TTbest;
int Waim;
//int Titer;
double starting_time, finishing_time, avg_time;
int len_best;
int Iteration[ 100 ];
double time_used[ 100 ];
int len_used[ 100 ];
int W_used[ 100 ];
char outfilename[30];
int len_improve;
//int len_time;
int Wmode;
//int TABUL0 = 5;


// section 0, initiaze
template<class T>
mnts( T *g, int Waim ,
			  int Wmode ,
			  int len_improve );
~mnts();

int randomInt( int n );
void clearGamma();
int selectC0( int Iter);
int WselectC0(int Iter );
int expand(int SelN);
int selectC1(int Iter );
int WselectC1( int Iter);
int plateau( int SelN, int Iter );
int Mumi_Weigt();
int backtract(int Iter);
void verify();
//void Output();
};
int tabu( int Max_Iter , mnts &m);
template <typename T>
int Max_Tabu(T *g);
/**
 *
 * @param g graph
 * @param _Waim The objective weight value
 * @param _Wmode The value for Waim related to the way of allocating weights to vertices
 * @param _len_improve The search depth
 */
template<class T>
mnts::mnts(T *g, int _Waim ,
			  int _Wmode ,
			  int _len_improve )
{
	TABUL = 7;
	len_best = 0;
    Waim = _Waim;
	Wmode = _Wmode;
	len_improve = _len_improve;

     int nb_vtx=0, nb_edg=-1, max_edg=0;
     int x1, x2;
	 Max_Vtx = g->number_of_vertices();
	 nb_edg = g->number_of_edges();

//	  cout << "Number of vertexes = " << Max_Vtx << endl;
//	  cout << "Number of edges = " << nb_edg << endl;

	  vectex = new int [Max_Vtx];
	  funch  = new int [Max_Vtx];
	  address= new int [Max_Vtx];
	  tabuin = new int [Max_Vtx];
	  adjaclen= new int[Max_Vtx];
	  C0 = new int[Max_Vtx];
	  C1 = new int[Max_Vtx];
	  TC1= new int[Max_Vtx];
	  We = new int[Max_Vtx];
	  BC = new int[Max_Vtx];
	  FC1= new int[Max_Vtx];
	  adjacMatrix = new int*[Max_Vtx];
	  cruset = new int [2000];
	  Tbest = new int[Max_Vtx];
	  TTbest = new int[Max_Vtx];

	  for( int x = 0 ; x < Max_Vtx ; x++ )
	  {
		   vectex[x]  = x;
		   address[x] = x;
	  }
	  Edge = new int*[ Max_Vtx ];

	  for (int x = 0 ; x < Max_Vtx ; x++ )
	  {
		  Edge[x] = new int[Max_Vtx];
		  adjacMatrix[x] = new int[Max_Vtx];
	  }

	  for ( int x=0; x<Max_Vtx; x++ )
		for ( int y=0; y<Max_Vtx; y++ )
		  Edge[x][y] = 0;
//starts inserting edges

		for(int i=0; i<g->number_of_vertices()-1; i++){
			for(int j=i+1; j<g->number_of_vertices(); j++){
				if(g->is_edge(i,j)){
					//o<<"["<<i<<"]"<<"--"<<"["<<j<<"]"<<endl;
					Edge[i][j]=Edge[j][i]=1;
					max_edg++;
				}
			}
		}


//	cout << "Density = " << (float) max_edg/(Max_Vtx*(Max_Vtx-1)) << endl;

     if ( 0 && max_edg != nb_edg )
     {
           cout << "### Error de lecture du graphe, nbre aretes : annonce="
                 << nb_edg << ", lu=" << max_edg  << endl;
           exit(0);
     }
     //cout<< " 1 " <<endl;
     for( int x=0 ; x<Max_Vtx; x++ )
       for( int y=0 ; y<Max_Vtx; y++ )
         Edge[x][y] = 1 - Edge[x][y];

	 //cout<< " 2 " <<endl;

     for( int x=0 ; x<Max_Vtx; x++ )
         Edge[x][x] = 0;

     //cout<< " 3 " <<endl;

     for( int x=0 ; x<Max_Vtx; x++ )
     {
         adjaclen[x] = 0;
         for( int y=0; y<Max_Vtx; y++ )
         {
                 if( Edge[x][y] == 1 )
                 {
                    adjacMatrix[x][adjaclen[x]] = y;
                    adjaclen[x]++;
                 }
         }
     }

     //cout<< " 4 " <<endl;
	 if(g->is_weighted_v()){
		  for( int x = 0; x < Max_Vtx; x++ ){
			   We[ x ] = g->get_wv(x);
			   BC[ x ] = 0;					//***?
		  }
	 }else{
		 //will generate weights depending on wmode. 
		 //wmode=1 is the unweighted version
		 for( int x = 0; x < Max_Vtx; x++ ) {
			 We[ x ] = (x+1)%Wmode + 1;
			 BC[ x ] = 0;

			 //We[ x ] = 1;
			 //We[ x ] = ( rand() % 10 ) + 1;
		 }
	 }

     //cout<< " 5 " <<endl;

//     FIC.close();
}

template<class T>
int Max_Tabu(T *g){
     int i, j, k, l, m, lbest;
     lbest = 0;
     int len_W;

//     int lenbest = 0;
//     int Titer = 0;
//     int M_iter = 0;
//     starting_time = (double)clock();
 	int len_time = (int (100000 / 10000) ) + 1;

//#pragma omp parallel shared(lbest) private(i)
//    {
//#pragma omp for schedule(dynamic) nowait
     for( i = 0; i < len_time; i++ ){
        mnts m_mnts(g,50,1,10000);
    	//waim wmode len_improve


         l = tabu(m_mnts.len_improve,m_mnts);
         //M_iter = M_iter + Iter;
         if( l > lbest )
         {
           lbest = l;
//         finishing_time = (double)clock();
           //Titer = M_iter;
           len_W = m_mnts.len_best;
         }

         //if( l == Waim )
         //  return lbest;
         //cout << " l = " << l << " i = " << i << endl;
     }
 //   }
     return lbest;
}
#endif


