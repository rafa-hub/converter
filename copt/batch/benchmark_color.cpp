#include "benchmark_color.h"
#include <sstream>
#include <iomanip>
#include <iostream>

void BkColor::Dimacs(){
///////////////////
//  132 instances
//  date_of_creation: COR-Paper 2011
//	last-updated: 29/11/16
//
// DO NOT CHANGE THE ORDER!
		
	
	add_test("1-FullIns_3.col");		
	add_test("1-FullIns_4.col");		
	add_test("1-FullIns_5.col");
	add_test("2-FullIns_3.col");		
	add_test("2-FullIns_4.col");
	add_test("2-FullIns_5.col");
	add_test("3-FullIns_3.col");
	add_test("3-FullIns_4.col");
	add_test("3-FullIns_5.col");
	add_test("4-FullIns_3.col");
	add_test("4-FullIns_4.col");
	add_test("4-FullIns_5.col");		
	add_test("5-FullIns_3.col");
	add_test("5-FullIns_4.col");

	add_test("1-Insertions_4.col");
	add_test("1-Insertions_5.col");
	add_test("1-Insertions_6.col");
	add_test("2-Insertions_3.col");
	add_test("2-Insertions_4.col");
	add_test("2-Insertions_5.col");
	add_test("3-Insertions_3.col");
	add_test("3-Insertions_4.col");
	add_test("3-Insertions_5.col");
	add_test("4-Insertions_3.col");
	add_test("4-Insertions_4.col");

	add_test("anna.col");
	add_test("david.col");
	add_test("homer.col");  		
	add_test("huck.col");
	add_test("jean.col");

	add_test("abb313GPIA.col");
	add_test("ash331GPIA.col");
	add_test("ash608GPIA.col");
	add_test("ash958GPIA.col");
		
	add_test("DSJC125.1.col");
	add_test("DSJC125.5.col");
	add_test("DSJC125.9.col");
	add_test("DSJC250.1.col");
	add_test("DSJC250.5.col");
/*	add_test("DSJC250.9.col"); */
	add_test("DSJC500.1.col");
	add_test("DSJC500.5.col");
//	add_test("DSJC500.9.col");		//not in Excel list for some reason
	add_test("DSJC1000.1.col");
/*	add_test("DSJC1000.5.col");
	add_test("DSJC1000.9.col"); */
	
    add_test("DSJR500.1.col");
	add_test("DSJR500.1c.col");
	add_test("DSJR500.5.col");

	add_test("flat300_20_0.col");
	add_test("flat300_26_0.col");
	add_test("flat300_28_0.col");
/*	add_test("flat1000_50_0.col");
	add_test("flat1000_60_0.col");
	add_test("flat1000_76_0.col");*/

	add_test("fpsol2.i.1.col");
	add_test("fpsol2.i.2.col");
	add_test("fpsol2.i.3.col");

	add_test("games120.col");

	add_test("inithx.i.1.col");
	add_test("inithx.i.2.col");
	add_test("inithx.i.3.col");

/*	add_test("latin_square_10.col");*/

	add_test("le450_5a.col");
	add_test("le450_5b.col");
	add_test("le450_5c.col");
	add_test("le450_5d.col");
	add_test("le450_15a.col");
	add_test("le450_15b.col");
	add_test("le450_15c.col");
	add_test("le450_15d.col");
	add_test("le450_25a.col");
	add_test("le450_25b.col");
	add_test("le450_25c.col");
	add_test("le450_25d.col");

	add_test("miles250.col");
	add_test("miles500.col");
	add_test("miles750.col");
	add_test("miles1000.col");
	add_test("miles1500.col");
		
	add_test("mug88_1.col");
	add_test("mug88_25.col");
	add_test("mug100_1.col");
	add_test("mug100_25.col");
		
	add_test("mulsol.i.1.col");
	add_test("mulsol.i.2.col");
	add_test("mulsol.i.3.col");
	add_test("mulsol.i.4.col");
	add_test("mulsol.i.5.col");

	add_test("myciel3.col");
	add_test("myciel4.col");
	add_test("myciel5.col");
	add_test("myciel6.col");
	add_test("myciel7.col");

	add_test("qg.order30.col");
	add_test("qg.order40.col");
	add_test("qg.order60.col");

	add_test("queen5_5.col");
	add_test("queen6_6.col");
	add_test("queen7_7.col");
	add_test("queen8_8.col");
	add_test("queen8_12.col");
	add_test("queen9_9.col");
	add_test("queen10_10.col");
	add_test("queen11_11.col");
	add_test("queen12_12.col");
	add_test("queen13_13.col");
	add_test("queen14_14.col");
	add_test("queen15_15.col");
	add_test("queen16_16.col");

	add_test("r125.1.col");
	add_test("r125.5.col");			
	add_test("r250.1.col");
	add_test("r250.5.col");
	add_test("r1000.1.col");
/*	add_test("r1000.5.col"); */

/*	add_test("r125.1c.col");		//complements of rxxx.y I presume?
	add_test("r250.1c.col");
	add_test("r1000.1c.col"); */
	
	add_test("school1.col");
	add_test("school1_nsh.col");

	add_test("wap01a.col");
	add_test("wap02a.col");
	add_test("wap03a.col");   
	add_test("wap04a.col");   
	add_test("wap05a.col");
	add_test("wap06a.col");
	add_test("wap07a.col");
	add_test("wap08a.col");

	add_test("will199GPIA.col");

	add_test("zeroin.i.1.col");
	add_test("zeroin.i.2.col");
	add_test("zeroin.i.3.col");
}

void BkColor::MinSumEasy(){
////////////////////
// Individual attempts less than 5s

	add_test("2-Insertions_3.col");
	add_test("3-Insertions_3.col");
	add_test("anna.col");
	add_test("david.col");
	add_test("huck.col");
	add_test("jean.col");

	add_test("DSJC125.1.col");
	add_test("DSJC125.5.col");
	add_test("flat300_20_0.col");
	add_test("flat300_26_0.col");
	add_test("flat300_28_0.col");
	add_test("fpsol2.i.1.col");
	add_test("inithx.i.1.col");
	add_test("games120.col");
	add_test("le450_15a.col");
	add_test("le450_15b.col");
	add_test("le450_15c.col");
	add_test("le450_15d.col");
	add_test("le450_25a.col");
	add_test("le450_25b.col");
	add_test("le450_25c.col");
	add_test("le450_25d.col");
	add_test("miles250.col");
	add_test("miles500.col");
	add_test("mug88_1.col");
	add_test("mug88_25.col");
	add_test("mug100_1.col");
	add_test("mug100_25.col");
	add_test("myciel3.col");
	add_test("myciel4.col");
	add_test("myciel7.col");
	add_test("qg.order30.col");
	add_test("queen5_5.col");
	add_test("queen6_6.col");
	add_test("queen7_7.col");
	add_test("queen8_8.col");
	add_test("wap06a.col");
	add_test("zeroin.i.2.col");
	add_test("zeroin.i.3.col");
}

void BkColor::MinSumHard(){
////////////////////
// Individual attempts more than 5s
	
	add_test("DSJC125.9.col");
	add_test("DSJC250.1.col");
	add_test("DSJC250.5.col");
	add_test("DSJC250.9.col");
	add_test("DSJC500.1.col");
	add_test("DSJC500.5.col");
	add_test("DSJC500.9.col");
	add_test("DSJC1000.1.col");
	add_test("myciel5.col");
	add_test("myciel6.col");
	add_test("qg.order40.col");
	add_test("qg.order60.col");
	add_test("wap05a.col");
	add_test("wap07a.col");
	add_test("wap08a.col");
}

void BkColor::Queen(){
	add_test("qg.order30.col");
	add_test("qg.order40.col");
	add_test("qg.order60.col");

	add_test("queen5_5.col");
	add_test("queen6_6.col");
	add_test("queen7_7.col");
	add_test("queen8_8.col");
	add_test("queen9_9.col");
	add_test("queen10_10.col");
	add_test("queen11_11.col");
	add_test("queen12_12.col");
	add_test("queen13_13.col");
	add_test("queen14_14.col");
	add_test("queen15_15.col");
	add_test("queen16_16.col");
}

void BkColor::Random(){

	
	add_test("r125.1.col");
	add_test("r125.5.col");			
	add_test("r250.1.col");
	add_test("r250.5.col");
	add_test("r1000.1.col");
	/*	add_test("r1000.5.col"); */

	/*	add_test("r125.1c.col");		//complements of rxxx.y I presume?
	add_test("r250.1c.col");
	add_test("r1000.1c.col"); */

}

void BkColor::Mulsol(){
	add_test("mulsol.i.1.col");
	add_test("mulsol.i.2.col");
	add_test("mulsol.i.3.col");
	add_test("mulsol.i.4.col");
	add_test("mulsol.i.5.col");
}

void BkColor::Miles(){
	 add_test("miles250.col");  
	 add_test("miles500.col");  
	 add_test("miles750.col"); 
	 add_test("miles1000.col");   
	 add_test("miles1500.col");  

}

void BkColor::Mug(){
	add_test("mug88_1.col");
	add_test("mug88_25.col");
	add_test("mug100_1.col");
	add_test("mug100_25.col");
}

void BkColor::Wap(){
	add_test("wap05a.col");
	add_test("wap06a.col");
	add_test("wap07a.col");
	add_test("wap08a.col");
}

void BkColor::Insertions(){
	add_test("2-Insertions_3.col");
	add_test("3-Insertions_3.col");
}

void BkColor::Init(){
	add_test("inithx.i.1.col");   
	add_test("inithx.i.2.col");   
	add_test("inithx.i.3.col");
}

void BkColor::Zeroin(){
	add_test("zeroin.i.2.col");
	add_test("zeroin.i.3.col");
}

void BkColor::Dsjc(){
	add_test("DSJC125.9.col");
	add_test("DSJC250.5.col");
	return;

	add_test("DSJC125.1.col");
	add_test("DSJC125.5.col");
	add_test("DSJC125.9.col");
/*	add_test("DSJC250.1.col");
	add_test("DSJC250.5.col");
	add_test("DSJC250.9.col");
	add_test("DSJC500.1.col");
	*add_test("DSJC500.5.col");
	add_test("DSJC500.9.col");*/
}

void BkColor::Le4(){
	add_test("le450_5a.col");
	add_test("le450_5b.col");
	add_test("le450_5c.col");
	add_test("le450_5d.col");
	add_test("le450_15a.col");
	add_test("le450_15b.col");
	add_test("le450_15c.col");
	add_test("le450_15d.col");
	add_test("le450_25a.col");
	add_test("le450_25b.col");
	add_test("le450_25c.col");
	add_test("le450_25d.col");
}

void BkColor::Flat(){
	add_test("flat1000_50_0.col");
	add_test("flat1000_60_0.col");
	add_test("flat1000_76_0.col");
	return;

	add_test("flat300_20_0.col");
	add_test("flat300_26_0.col");
	add_test("flat300_28_0.col");
	add_test("flat1000_50_0.col");
	add_test("flat1000_60_0.col");
	add_test("flat1000_76_0.col");
}

void BkColor::DimacsPreProcessable(){
///////////////////////
// Dimacs color instances which if preprocessed initially can be reduced
	 
	add_test("4-FullIns_3.col");   
	add_test("5-FullIns_3.col");  
	add_test("anna.col");
	add_test("david.col"); 
	add_test("huck.col");   
	add_test("jean.col");   
	  
	add_test("ash331GPIA.col ");  
	add_test("ash608GPIA.col");   
	add_test("ash958GPIA.col ");  
	add_test("abb313GPIA.col"); 
	add_test("DSJR500.1.col");   
	add_test("DSJR500.1c.col");   
	add_test("DSJR500.5.col");   
	add_test("fpsol2.i.1.col");   
	add_test("fpsol2.i.2.col");   
	add_test("fpsol2.i.3.col");  
	add_test("games120.col");   
	add_test("inithx.i.1.col");   
	add_test("inithx.i.2.col");   
	add_test("inithx.i.3.col"); 
	add_test("le450_5d.col"); 
	add_test("le450_15a.col ");  
	add_test("le450_15d.col");   
	add_test("le450_25a.col"); 
  	add_test("miles250.col");  
	add_test("miles500.col");  
	add_test("miles750.col"); 
	add_test("miles1000.col");   
	add_test("miles1500.col");  
	add_test("mulsol.i.1.col");   
	add_test("mulsol.i.2.col");   
	add_test("mulsol.i.3.col");   
	add_test("mulsol.i.4.col");   
	add_test("mulsol.i.5.col");   
	add_test("r125.1.col ");  
	add_test("r125.5.col"); 
	add_test("r250.1.col ");  
	add_test("r1000.1.col");   
	add_test("r1000.5.col");  
	add_test("r125.1c.col");
	add_test("r250.1c.col"); 
	add_test("r1000.1c.col");  
	add_test("school1.col");   
	add_test("school1_nsh.col"); 
	add_test("wap01a.col ");  
	add_test("wap02a.col");   
	add_test("wap03a.col ");   
	add_test("wap04a.col");    
	add_test("wap05a.col"); 
	add_test("wap06a.col");   
	add_test("wap07a.col");   
	add_test("wap08a.col"); 
	add_test("will199GPIA.col ");  
	add_test("zeroin.i.1.col");   
	add_test("zeroin.i.2.col");   
	add_test("zeroin.i.3.col");   

}

void BkColor::EasyDimacs(){
	add_test("1-FullIns_3.col");		
	add_test("1-FullIns_4.col");		//New solved (filter merging)
	add_test("2-FullIns_3.col");		//New solved (filter merging)
	
	add_test("2-Insertions_3.col");
	add_test("3-Insertions_3.col");
	
	add_test("anna.col");
	add_test("david.col");
	add_test("homer.col");			//El problema es el el LB es malo, comparado con Trick y compañia
	add_test("huck.col");
	add_test("jean.col");
	
	add_test("ash331GPIA.col");

	add_test("DSJC125.1.col");

	add_test("DSJR500.1.col");

	add_test("le450_25a.col");
	add_test("le450_25b.col");

	add_test("miles250.col");
	add_test("miles500.col");
	add_test("miles750.col");
	add_test("miles1500.col");

	
	add_test("mulsol.i.1.col");
	add_test("mulsol.i.2.col");
	add_test("mulsol.i.3.col");
	add_test("mulsol.i.4.col");
	add_test("mulsol.i.5.col");

	add_test("myciel3.col");
	add_test("myciel4.col");
	add_test("myciel5.col");

	add_test("qg.order30.col");
	add_test("queen5_5.col");
	add_test("queen6_6.col");
	add_test("queen7_7.col");
	add_test("queen8_8.col");		//Dsatur Trick no resuelve (segun Malaguti)
	add_test("queen8_12.col");

	add_test("r125.1.col");
	add_test("r250.1.col");
	add_test("r1000.1.col");
	add_test("r125.1c.col");
	add_test("r250.1c.col");

	add_test("school1.col");
	
	add_test("zeroin.i.1.col");
	add_test("zeroin.i.2.col");
	add_test("zeroin.i.3.col");
}

void BkColor::NonTrivialMalagutiDimacs(){
	add_test("1-FullIns_4.col");		//New solved (filter merging)
	add_test("1-FullIns_5.col");
	add_test("2-FullIns_3.col");		//New solved (filter merging)
	add_test("2-FullIns_4.col");
	add_test("2-FullIns_5.col");
	add_test("3-FullIns_3.col");
	add_test("3-FullIns_4.col");
	add_test("3-FullIns_5.col");
	add_test("4-FullIns_3.col");
	add_test("4-FullIns_4.col");
//	add_test("4-FullIns_5.col");  //Problemas de memoria
	add_test("5-FullIns_3.col");
	add_test("5-FullIns_4.col");

	add_test("1-Insertions_4.col");
	add_test("1-Insertions_5.col");
	add_test("1-Insertions_6.col");
	add_test("2-Insertions_3.col");
	add_test("2-Insertions_4.col");
	add_test("2-Insertions_5.col");
	add_test("3-Insertions_3.col");
	add_test("3-Insertions_4.col");
	add_test("3-Insertions_5.col");
	add_test("4-Insertions_3.col");
	add_test("4-Insertions_4.col");
	
	add_test("abb313GPIA.col");
	add_test("ash331GPIA.col");
	add_test("ash608GPIA.col");
	add_test("ash958GPIA.col");
		
	add_test("DSJC125.1.col");
	add_test("DSJC125.5.col");
	add_test("DSJC125.9.col");
	add_test("DSJC250.1.col");
	add_test("DSJC250.5.col");
	add_test("DSJC250.9.col");
	add_test("DSJC500.1.col");
	add_test("DSJC500.5.col");
//	add_test("DSJC500.9.col");		//Me da problemas al leer. Entender
	add_test("DSJC1000.1.col");
	add_test("DSJC1000.5.col");
	add_test("DSJC1000.9.col");

//	add_test("DSJR500.1.col");		//Trivial
	add_test("DSJR500.1c.col");
	add_test("DSJR500.5.col");
		
	add_test("fpsol2.i.1.col");
	add_test("fpsol2.i.2.col");
	add_test("fpsol2.i.3.col");
	
	add_test("le450_5a.col");
	add_test("le450_5b.col");
//	add_test("le450_5c.col");		//Trivial
	add_test("le450_5d.col");
	add_test("le450_15a.col");
	add_test("le450_15b.col");
	add_test("le450_15c.col");
	add_test("le450_15d.col");
//	add_test("le450_25a.col");
//	add_test("le450_25b.col");
	add_test("le450_25c.col");
	add_test("le450_25d.col");
	
	
	add_test("miles1000.col");
	add_test("miles1500.col");
	
	add_test("mug88_1.col");
	add_test("mug88_25.col");
	add_test("mug100_1.col");
	add_test("mug100_25.col");
	
/*  add_test("myciel6.col");
	add_test("myciel7.col");  */

	add_test("qg.order40.col");
	add_test("qg.order60.col");

	add_test("queen8_8.col");
	add_test("queen8_12.col");
	add_test("queen9_9.col");
	add_test("queen10_10.col");
	add_test("queen11_11.col");
//	add_test("queen12_12.col");
	add_test("queen13_13.col");
	add_test("queen14_14.col");
	add_test("queen15_15.col");
	add_test("queen16_16.col");
	
	add_test("wap01a.col");
	add_test("wap02a.col");
	add_test("wap03a.col");		//Problemas de memoria
	add_test("wap04a.col");
	add_test("wap05a.col");
	add_test("wap06a.col");
	add_test("wap07a.col");
	add_test("wap08a.col");

	add_test("will199GPIA.col");
}

void BkColor::MyRandomBenchmark(){
	stringstream name;

	int size=60;
	/*for(float j=.1 ; j<=.91; j+=.1){
		for(int k=0; k<50; k++){
			name.str("");
			name<<"\\"<<size<<"\\";		//Path
			name<<"r"<<size<<"_"<<setprecision(0)<<100*j<<"."<<k<<".txt";
			add_test(name.str().c_str());
		}
	}*/

	//70
	for(int i=70; i<76; i+=5){
		for(float j=.3 ; j<=.71; j+=.1){
			for(int k=0; k<50; k++){
				name.str("");
				name<<"\\"<<i<<"\\";		//Path
				name<<"r"<<i<<"_"<<setprecision(0)<<100*j<<"."<<k<<".txt";
				add_test(name.str().c_str());
			}
		}
	}


	
	//Instancias 70-75
	/*for(int i=75; i<80; i+=5){
		for(float j=.1 ; j<=.91; j+=.1){
			for(int k=0; k<50; k++){
				name.str("");
				name<<"\\"<<i<<"\\";		//Path
				name<<"r"<<i<<"_"<<setprecision(0)<<100*j<<"."<<k<<".txt";
				add_test(name.str().c_str());
			}
		}	
	}*/
	

	//Instancias 80 hasta .3
	/*size=80;
	for(float j=.1 ; j<=.31; j+=.1){
		for(int k=0; k<50; k++){
			name.str("");
			name<<"\\"<<size<<"\\";		//Path
			name<<"r"<<size<<"_"<<setprecision(0)<<100*j<<"."<<k<<".txt";
			add_test(name.str().c_str());
		}
	}*/

}