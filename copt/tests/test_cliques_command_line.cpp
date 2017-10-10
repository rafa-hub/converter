//--------------------------------------------
// Test only for windows OS; needs passing the path of the folder as VS argument
// author: alopez
// date: 6/10/114


#include <iostream>
#include <string>
#include "gtest/gtest.h"
#include "utils/prec_timer.h"
#include "../clique/clique.h"
#include "../clique_para/clique_parallel.h"


//#define LOG_FILE  "C:\\Users\\Pablo\\Desktop\\biiwsp\\res_clique.txt"
//#define LOG_FILE  "C:\\Users\\Pablo\\Desktop\\biiwsp\\trial_set.txt"

using namespace std;

//TEST(Command_line, single_sparse_ugraph_parallel){
//		
//   extern int argc_read;
//   extern char** argv_read;
//   PrecisionTimer pt;
//   int sol;
//   double time_in_sec;
//   
//	
//   if(argc_read != 2){
//	  cout<<"filename required"<<endl;
//       return;
//   }
//   
//   string filename(argv_read[1]);
//
//   cout<<"reading--------------------------"<<endl;
//   sparse_ugraph usg(filename);
//
//   //launch parallel version
//   Clique_parallel<sparse_ugraph> cpusg(usg);
//   cout<<"Max cores HW: "<<cpusg.get_max_cores_hw()<<endl;
//   cpusg.set_cores(cpusg.get_max_cores_hw());		
//  
//   cout<<"setup--------------------------"<<endl;
//   if((sol=cpusg.set_up_unrolled())!=0){
//	  cout<<usg.get_name()<<"-solved trivially-"<<sol<<endl;
//	  return;
//   }
//
//   cout<<"search-------------------------"<<endl;
// 
//   pt.wall_tic();
//   cpusg.run_unrolled();	
//   time_in_sec=pt.wall_toc();
//
//   cout<<usg.get_name()<<"[t:"<<time_in_sec<<"]"<<"[w: "<<cpusg.get_max_clique()<<"]"<<endl;
//}

//TEST(Command_line, single_sparse_ugraph){
//	
//   extern int argc_read;
//   extern char** argv_read;
//   PrecisionTimer pt;
//   int sol;
//     	
//   if(argc_read != 2){
//	  cout<<"filename required"<<endl;
//       return;
//   }
//   
//   string filename(argv_read[1]);
//
//   cout<<"reading--------------------------"<<endl;
//   sparse_ugraph usg(filename);
//   Clique<sparse_ugraph> cusg(&usg);
//   
//   //launch serial version
//   cout<<"setup--------------------------"<<endl;
//   if((sol=cusg.set_up())>0){
//	  cout<<usg.get_name()<<"-solved trivially-"<<sol<<endl;
//	  return;
//   }
//
//   cout<<"search-------------------------"<<endl;
// 
//   pt.wall_tic();
//   cusg.run();	
//   double  time_in_sec=pt.wall_toc();
//
//   cout<<usg.get_name()<<"[t:"<<time_in_sec<<"]"<<"[w: "<<cusg.get_max_clique()<<"]"<<endl;
//}

TEST(Command_line, ugraph){
	
   extern int argc_read;
   extern char** argv_read;
   PrecisionTimer pt;
   int sol;
     	
   if(argc_read != 2){
	  cout<<"filename required"<<endl;
       return;
   }
   
   string filename(argv_read[1]);

   cout<<"reading--------------------------"<<endl;
   sparse_ugraph usg(filename);
   Clique<sparse_ugraph> cusg(&usg, clqo::param_t());
   
   //launch serial version
   cout<<"setup--------------------------"<<endl;
   if((sol=cusg.set_up())>0){
	  cout<<usg.get_name()<<"-solved trivially-"<<sol<<endl;
	  return;
   }

   cout<<"search-------------------------"<<endl;
 
   pt.wall_tic();
   cusg.run();	
   double  time_in_sec=pt.wall_toc();

   cout<<usg.get_name()<<"[t:"<<time_in_sec<<"]"<<"[w: "<<cusg.get_max_clique()<<"]"<<endl;
}
