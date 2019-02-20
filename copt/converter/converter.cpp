// #include "../gtest/gtest.h"
#include "graph/graph.h"
#include "graph/algorithms/decode.h"
#include "utils/common.h"
#include "utils/logger.h"

#include "XCSP3CoreParser.h"
#include "XCSP3PrintCallbacks.h"

#include <fstream>
#include <string.h>
#include <iostream>
#include <climits>
#include <map>

//#define mipause
#define midebug
//#define mitest
#define RESTRICCION 0
#define SOPORTE 1
#define CREAR_MATRIZ 1
#ifndef BENCHMARK_PATH 
#define BENCHMARK_PATH  "/var/tmp/salida"
#define DIFERENTE 0
#define IGUAL 1
#endif

using namespace XCSP3Core;

class MiSolverPrintCallbacks: public XCSP3PrintCallbacks {

private:

	vector<string> lista_arrays;    			// Guarda la lista de arrays
	vector<string> lista_variables; 			// Guarda la lista de variables
	map<string,int> mapa_indices;				// Guarda el índice de cada variable
	
	bool is_array=false;					// PSS-determina si una varaible es un singleton o forma parte de un array

	std::map<string, int> base_array; 		// Mapa de cada array con su coordenada base
	std::map<string, int> minimo_variable; 	// Guarda el minimo del rango de las variables
	std::map<string, int> rango_variable; 	// Mapa de cada array con el rango de valores de las variables
	std::map<string, int> numero_variable;	// Mapa de cada array con el numero de instancias
										    // de variables del array

	string array_actual = "empiezo"; 	// Sirve para identificar con que array se esta trabajando
	int base_siguiente_array = 0; 		// Guarda el valor para calcular la posicion en la matriz del siguiente array
	int minimo_variables = 0;        	// Guarda el minimo valor de cada variable
	int rango_variables = 0; 			// Guarda el rango de valores de las variables de un array
	int numero_variables = 0;      		// Guarda el numero de variables de un array

	vector<vector<int>> las_tuplas;   	// Guarda las tuplas, puesto que en
									  	// buildConstraintExtensionAs() no me las pasan como argumento
	vector<int> tuplas_unarias;			// Lo mismo, pero para variables unarias

public:

	int dimension_matriz = 0; 			//Guarda la dimension definitiva de la matriz creada
	int **matriz_datos; 	// Matriz donde se almacena el resultado
	int **matriz_shadow; 	// Matriz donde se almacena el resultado
#ifdef mitest
	vector<vector<int>> matriz_check; 	// Matriz donde se almacena el resultado
#endif

	char nombre_fichero[256]; 			// Nombre del fichero XML a procesar




	void set_nombre_fichero(char *nombre) {
		strcpy(nombre_fichero, nombre);
	}



	// Escribe los resultados en un fichero
	void escribe_nombre_fichero() {
		string var;
		char *nombre_fichero_csp;

		nombre_fichero_csp = strrchr(nombre_fichero, '.');
		strcpy(nombre_fichero_csp, ".csp");
		cout << "Nombre fichero CSP: " << nombre_fichero << endl;
		ofstream fichero_salida(nombre_fichero);

#ifdef midebug
		cout<< "c Fichero creado a partir de un fichero XML que expresa un problema CSP"<< endl;
		cout << "x " << lista_variables.size() << endl;
#endif
		fichero_salida<< "c Fichero creado a partir de un fichero XML que expresa un problema CSP"<< endl;
		fichero_salida << "x " << lista_variables.size() << endl;

		for (unsigned int j = 0; j < lista_variables.size(); j++) {
			var = get_nombre(lista_variables[j]);
			fichero_salida << "v " << (j + 1) << " " << rango_variable[var]
					<< endl;

#ifdef midebug
			cout << var << endl;
			cout << "v " << (j + 1) << " " << rango_variable[var] << endl;
#endif
		}

		fichero_salida.close();
	}






	// Extrae y devuelve el indice de una variable

	int get_indice(XVariable variable) {
		string valor;
		int indice;

		valor = variable.id;
		indice=mapa_indices[valor];

#ifdef midebug
		cout << "En get_indice(), id variable: " << valor << " es: " << indice<< endl;
#endif

		return(indice);
// 		return(mapa_indices[variable.id]);  //Toda la función
	}







	//Extrae y devuelve el nombre de la variable sin indice, es decir, el nombre del array
	string get_nombre(string variable) {
		string nombre, vector;
		size_t aux1 = 0;

		nombre = variable;


		aux1 = nombre.find_first_of('[', 0);
		if(aux1!=string::npos)
			vector = nombre.substr(0, aux1);
		else{
			//singleton variable
			vector=nombre;
		}


		return vector;
	}





	//Muestra en pantalla coord base de todas las variables
	//TODO-mapping de "nombre var" con coordenada base?
	void print_coordenadas_base(){
		cout<<"IMPRIMENDO COORDENADAS BASE DE TODAS LAS VARIABLES"<<endl;
		for (vector<string>::iterator it = lista_arrays.begin();
					it != lista_arrays.end(); it++) {

			string array_var_name = get_nombre(*it);
			int row = base_array[array_var_name];
			const int NUM_VAL = rango_variable[array_var_name];

			for(int id=0; id<numero_variable[array_var_name]; id++){
				cout<<array_var_name<<"["<<id<<"]"<<" base_row:"<<row+(id*rango_variable[array_var_name])<<endl;

			}
		}
		cout<<"------------------------------------"<<endl;

		displayList(lista_arrays);
	}






	// Calcula las coordenadas base de la variable. A esto habra que sumar el orden de la
	// instancia de la variable y el valor de la coordenada de la restriccion
	// Hay que restar el minimo del rango de valores para el caso en el que no sea cero
	// Si no, se escribe fuera del rango de la matriz
	void calcula_coordenadas_base(string var_cero, string var_uno, int indice0,
									int indice1, int *coordenadas_base) {

		*coordenadas_base = base_array[var_cero] + (indice0 * rango_variable[var_cero]);
		coordenadas_base++;
		*coordenadas_base = base_array[var_uno] + (indice1 * rango_variable[var_uno]);
		
#ifdef midebug
		coordenadas_base--;
		cout << "Var cero: " << var_cero << " - indice: " << indice0 << " - Coordenada Base X: " 
		<< *coordenadas_base << endl;
		
		coordenadas_base++;
		cout << "Var uno: " << var_uno << " - indice: " << indice1 << " - Coordenada Base Y: " 
		<< *coordenadas_base << endl;
#endif

		return;
	}







	//for testing: x[0], 3, y[0], 1 -> true/false
	//assumes values are integers
	bool is_conflicting(string name1, string value1, string name2, string value2){
		bool res=false;
		int vindex_1, vindex_2;
		string avn_1, avn_2;
		size_t pos1, pos2;

		//data for first (var, val)
		avn_1=get_nombre(name1);
		pos1 = name1.find_first_of('[', 0);
		pos2 = name1.find_first_of(']', pos1);
		vindex_1 = std::stoi(name1.substr(pos1 + 1, pos2 - 2));
		int row_1=base_array[avn_1]+ (vindex_1 * rango_variable[avn_1]) + std::stoi(value1) -minimo_variable[avn_1];

		//data for second (var, val)
		avn_2 = get_nombre(name2);
		pos1 = name2.find_first_of('[', 0);
		pos2 = name2.find_first_of(']', pos1);
		vindex_2 = std::stoi(name2.substr(pos1 + 1, pos2 - 2));
		int row_2 = base_array[avn_2] + (vindex_2 * rango_variable[avn_2]) + std::stoi(value2) -minimo_variable[avn_2];

		return !((bool) matriz_datos[row_1][row_2]);
	}





	//removes edges corresponding to values of the same variable, from ug and matriz_datos
	//(all incompatible since a variable may only have one value)
	int remove_edges_same_var(ugraph& ug) {
//		com::stl::print_collection(miparser.lista_arrays, cout); cout<<endl;
		cout<<"REMOVING EDGES FROM VALUES OF SAME VARIABLE:-----------------"<<endl;
		for (vector<string>::iterator it = lista_arrays.begin();
				it != lista_arrays.end(); it++) {
			string array_var_name = get_nombre(*it);
			int row = base_array[array_var_name];

			const int NUM_VAL = rango_variable[array_var_name];
			const int MAX_ROWS_ARRAY_VAR = row
					+ (numero_variable[array_var_name] * NUM_VAL);
#ifdef midebug
			cout << array_var_name << " row:" << row << " range:" << NUM_VAL
					<< " nb_var:" << numero_variable[array_var_name]
					<< endl;
#endif

			while (true) {
				for (int i = row; i < (row + NUM_VAL - 1); i++) {
					for (int j = i + 1; j < (row + NUM_VAL); j++) {
						ug.remove_edge(i, j);
						matriz_datos[i][j] = 0;
						matriz_datos[j][i] = 0;
#ifdef midebug
						cout<<"edge:"<<"("<<i<<","<<j<<")";
						cout<<"var:"<<array_var_name<<" base_array:"<<base_array[array_var_name]<<endl;
						cout<<"range: "<<NUM_VAL<<" MAX ROW:"<<MAX_ROWS_ARRAY_VAR<<endl;
						cout<<"--------------------------"<<endl;
#endif
					}
				}

				//new var inside var array
				row += NUM_VAL;
				if (row >= MAX_ROWS_ARRAY_VAR)
					break;
			}
		}
		cout<<"FINISHED REMOVING EDGES FROM VALUES OF SAME VARIABLE:-----------------"<<endl;
		return 0;
	}






	// Genera la matriz
	void genera_matriz() {
		std::vector<string>::iterator lista;
		std::vector<int> fila;
		std::vector<int> fila_shadow;

		for (lista = lista_arrays.begin(); lista != lista_arrays.end();
				lista++) {
			dimension_matriz += numero_variable[*lista]
					* rango_variable[*lista];

#ifdef midebug
			cout << "array: " << *lista << endl;
			cout << "numero variables: " << numero_variable[*lista] << endl;
			cout << "rango variable: " << rango_variable[*lista] << endl;
			cout << "dimension variable: "<< numero_variable[*lista] * rango_variable[*lista] << endl;
			cout << "dimension acumulada: " << dimension_matriz << endl;
#endif
		}

		//I/O
//        com::stl::print_collection(lista_arrays);
//        cin.get();

//         com::stl::print_collection(lista_variables);
//         cin.get();
//

		// Generacion de la matriz inicializando a ceros.
		//fila.assign(dimension_matriz,1);
		//fila_shadow.assign(dimension_matriz,0);

//		for (int j = 0; j < dimension_matriz; j++) {
//			fila.push_back(1);
//			fila_shadow.push_back(0);
//		}


// 		for (int j = 0; j < dimension_matriz; j++) {
// 			matriz_datos.push_back(fila);
// 			matriz_shadow.push_back(fila_shadow);
// #ifdef mitest
// 			matriz_check.push_back(fila_shadow);
// #endif
// 		}


		matriz_datos = new int *[dimension_matriz];
    
    	for(int i = 0; i<dimension_matriz;i++)
    	{
      		matriz_datos[i] = new int[dimension_matriz];
			//cout << i << " " ;
    	}
		//cout << endl;


		matriz_shadow = new int *[dimension_matriz];
    
    	for(int i = 0; i<dimension_matriz;i++)
    	{
      		matriz_shadow[i] = new int[dimension_matriz];
    	}



#ifdef midebug
		//cout << "Matriz creada ........ \nDimension de la matriz: "	<< matriz_datos.size() << endl;
		/*ofstream fmatriz("pocholo.txt", ios::out);
		 imprime_matriz("datos",fmatriz);
		imprime_matriz("shadow",fmatriz); */
#endif
	}







	// Certificacion de que la matriz tiene la diagonal principal a cero
	void pongo_diagonal_matriz_a_cero() {
		for (int x = 0; x < dimension_matriz; x++) {
			matriz_datos[x][x] = 0;
		}
	}









	//Vuelca en pantalla la matriz, solo útil para depuración, en casos reales
	//la matriz suele ser demasiado grande
	ostream& imprime_matriz(string matriz, ostream& o=cout) {
		if (matriz == "datos") {
			//cout<<"MATRIZ DE DATOS-----------------"<<endl;
			for (int x = 0; x < dimension_matriz; x++) {
				for (int y = 0; y < dimension_matriz; y++){
					o << matriz_datos[x][y] << " ";
				}
				o << endl;
			}
			o << "\n\n" << endl;
		}
		if (matriz == "shadow") {
			//cout<<"MATRIZ SHADOW----------------"<<endl;
			for (int x = 0; x < dimension_matriz; x++){
				for (int y = 0; y < dimension_matriz; y++){
					o << matriz_shadow[x][y] << " ";
				}
				o << endl;
			}
			o << "\n\n" << endl;
		}
		return o;
	}









	//Funcion que escribe en la matriz reglas unarias
	void escribe_en_matriz_unaria(int *coordenadas_base, vector<int>& tuplas,
		string var_cero, bool support) {
		//vector<vector<int>>::iterator it;

		std::vector<int>::iterator itero_valores;
		int coordenada_final[2];

		//support

		if (support) {

			cout << "Regla SUPPORT UNARIA....." << endl;
			
			cout << "Var:" << var_cero << " min var: " << endl;


#ifdef midebug			
			cout << minimo_variable[var_cero] << endl;
#endif			


			if (tuplas.size()==0)
			{
				// No hay tuplas y es una regla support => todo a ceros
				cout << "CONJUNTO DE TUPLAS VACIO: TODO A CEROS" << endl;
				for (int i = 0; i < rango_variable[var_cero]; i++)
					for (int j = 0; j < rango_variable[var_cero]; j++) {
						coordenada_final[0] = coordenadas_base[0] + i;
						coordenada_final[1] = coordenadas_base[1] + j;
						if (!matriz_shadow[coordenada_final[0]][coordenada_final[1]]) {
#ifdef midebug
							cout << "writing-0-S en:(" << coordenada_final[0] << ","
								<< coordenada_final[1] << ")" << endl;
#endif
							matriz_datos[coordenada_final[0]][coordenada_final[1]] =0;
							matriz_datos[coordenada_final[1]][coordenada_final[0]] =0;
						}
					}
			} else {

					//La regla SUPPORT dice cuales son posibles (por tanto se ELIMINAN el RESTO de valores)

					for (itero_valores = tuplas.begin(); itero_valores != tuplas.end();
							++itero_valores) { 
						
#ifdef midebug
						cout << "Valor Tupla: " << *itero_valores
							<< endl;
#endif

						//Escritura en horizontal y vertical
						for(int i=0;i<dimension_matriz;i++)
						{
							coordenada_final[0] = coordenadas_base[0]
								+ (*itero_valores)
								- minimo_variable[var_cero];

							coordenada_final[1] =i;

							matriz_datos[coordenada_final[0]][coordenada_final[1]] = 1;
							matriz_shadow[coordenada_final[0]][coordenada_final[1]] = 1;
							matriz_datos[coordenada_final[1]][coordenada_final[0]] = 1;
							matriz_shadow[coordenada_final[1]][coordenada_final[0]] = 1;
						}


						//Escritura en matriz shadow vertical
						/* for(int i=0;i<dimension_matriz;i++)
						{	
							coordenada_final[0] = i;

							coordenada_final[1] = coordenadas_base[0]
								+ (*itero_valores)
								- minimo_variable[var_cero];

							//matriz_datos[coordenada_final[0]][coordenada_final[1]] = 1;
							matriz_shadow[coordenada_final[0]][coordenada_final[1]] = 1;
							//matriz_datos[coordenada_final[1]][coordenada_final[0]] = 1;
							matriz_shadow[coordenada_final[1]][coordenada_final[0]] = 1;
						} */


#ifdef midebug
						cout << "Coordenada base variable: "<< var_cero << "-> (" << 
						coordenadas_base[0] << "," << coordenadas_base[1] << ")" << endl;
						
						cout << "Tupla support leida-coord:(" << coordenada_final[0]
							<< "," << coordenada_final[1] << ")" << endl;
#endif
					}

					// Borro el resto de restricciones
					for (int i = 0; i < rango_variable[var_cero]; i++)
						for (int j = 0; j < dimension_matriz; j++) {
							coordenada_final[0] = coordenadas_base[0] + i;
							coordenada_final[1] = j;

#ifdef midebug
							cout << "writing-0-S en:(" << coordenada_final[0] << ","
									<< coordenada_final[1] << ")" << endl;
#endif

							if (!matriz_shadow[coordenada_final[0]][coordenada_final[1]]) {
	
								matriz_datos[coordenada_final[0]][coordenada_final[1]] =0;
								matriz_datos[coordenada_final[1]][coordenada_final[0]] =0;
							}

#ifdef midebug
							cout << "writing-0-S en:(" << coordenada_final[1] << ","
										<< coordenada_final[0] << ")" << endl;
#endif

							if (!matriz_shadow[coordenada_final[1]][coordenada_final[0]] ) {

								matriz_datos[coordenada_final[0]][coordenada_final[1]] =0;
								matriz_datos[coordenada_final[1]][coordenada_final[0]] =0;
							}
				}
			}

		} else {

			cout << "Escribiendo en la matriz una Regla CONFLICT UNARIA.........." << endl;

			// Escribo una a una las tuplas correspondientes a cero.

			for (itero_valores = tuplas_unarias.begin();
					itero_valores != tuplas_unarias.end(); ++itero_valores) {
								
				cout << "Valor Unario: " << *itero_valores
						<< endl;


			// Escribo ceros en horizontal y vertical
				coordenada_final[0] = coordenadas_base[0]
						+ (*itero_valores)
						- minimo_variable[var_cero];


				for (int i=0;i<dimension_matriz;i++)
				{
					coordenada_final[1] = i;
#ifdef midebug
					cout << "writing-0-C en:(" << coordenada_final[0] << ","
						<< coordenada_final[1] << ")" << endl;
#endif
					matriz_datos[coordenada_final[0]][coordenada_final[1]] = 0;
					matriz_datos[coordenada_final[1]][coordenada_final[0]] = 0;
				}

		
		// Escribo ceros en vertical

			/* 	coordenada_final[1] = coordenada_final[0];


				for (int i=0;i<dimension_matriz;i++)
				{
					coordenada_final[0] = i;
#ifdef midebug
					cout << "writing-0-C en:(" << coordenada_final[0] << ","
						<< coordenada_final[1] << ")" << endl;
#endif
					matriz_datos[coordenada_final[0]][coordenada_final[1]] = 0;
				} */
			


				//testing
#ifdef mitest
				if (matriz_check[coordenada_final[0]][coordenada_final[1]] ||
					matriz_check[coordenada_final[1]][coordenada_final[0]] ) {
					cout << "SOBREESCRIBIENDO EN MATRIZ DE DATOS!!!!!!!!"<< endl;
					cin.get();
				} else {
					matriz_check[coordenada_final[0]][coordenada_final[1]] = 1;
					matriz_check[coordenada_final[1]][coordenada_final[0]] = 1;
				}
#endif

			}
		}
	}














	//Funcion que escribe en la matriz reglas binarias
	void escribe_en_matriz(int *coordenadas_base, vector<vector<int> >& tuplas,
			string var_cero, string var_uno, bool support) {
		//vector<vector<int>>::iterator it;

		std::vector<vector<int>>::iterator itero_parejas;
		vector<int>::iterator itero_dentro_de_la_pareja;
		int coordenada_final[2];

		//support

		if (support) {


#ifdef midebug	
			cout << "Regla Support ...................." << endl;
			cout << "Var_0: " << var_cero << "- Var_1: " << var_uno << endl;
			cout << " min var: " << minimo_variable[var_cero] << endl;
			cout << " min var: " << minimo_variable[var_uno] << endl;
#endif

			// No hay tuplas y es una regla support => todo a ceros

			if (tuplas.size()==0)
			{
				cout << "CONJUNTO DE TUPLAS VACIO: TODO A CEROS" << endl;
				for (int i = 0; i < rango_variable[var_cero]; i++)
					for (int j = 0; j < rango_variable[var_uno]; j++) {
						coordenada_final[0] = coordenadas_base[0] + i;
						coordenada_final[1] = coordenadas_base[1] + j;
						if (!matriz_shadow[coordenada_final[0]][coordenada_final[1]]) {
#ifdef midebug
							cout << "writing-0-S en:(" << coordenada_final[0] << ","
								<< coordenada_final[1] << ")" << endl;
#endif
							matriz_datos[coordenada_final[0]][coordenada_final[1]] =0;
							matriz_datos[coordenada_final[1]][coordenada_final[0]] =0;
						}
					}
			} else {
					for (itero_parejas = tuplas.begin(); itero_parejas != tuplas.end();
							++itero_parejas) {
						itero_dentro_de_la_pareja = itero_parejas->begin();

#ifdef midebug
						cout << "Primer valor Tupla: " << *itero_dentro_de_la_pareja
							<< endl;
#endif

						coordenada_final[0] = coordenadas_base[0]
							+ (*itero_dentro_de_la_pareja)
							- minimo_variable[var_cero];

						itero_dentro_de_la_pareja++;
#ifdef midebug
						cout << "Segundo valor Tupla: " << *itero_dentro_de_la_pareja
							<< endl;
#endif
						coordenada_final[1] = coordenadas_base[1]
							+ (*itero_dentro_de_la_pareja)
							- minimo_variable[var_uno];

						//matriz_datos[coordenada_final[0]][coordenada_final[1]] = 1;
						matriz_shadow[coordenada_final[0]][coordenada_final[1]] = 1;
						//matriz_datos[coordenada_final[1]][coordenada_final[0]] = 1;
						matriz_shadow[coordenada_final[1]][coordenada_final[0]] = 1;
#ifdef midebug
						cout << "Tupla support leida-coord:(" << coordenada_final[0]
							<< "," << coordenada_final[1] << ")" << endl;
#endif
					}

					// Borro el resto de restricciones
					for (int i = 0; i < rango_variable[var_cero]; i++)
						for (int j = 0; j < rango_variable[var_uno]; j++) {
							coordenada_final[0] = coordenadas_base[0] + i;
							coordenada_final[1] = coordenadas_base[1] + j;
						if (!matriz_shadow[coordenada_final[0]][coordenada_final[1]]) {
#ifdef midebug
							cout << "writing-0-S en:(" << coordenada_final[0] << ","
								<< coordenada_final[1] << ")" << endl;
#endif
							matriz_datos[coordenada_final[0]][coordenada_final[1]] =0;
							matriz_datos[coordenada_final[1]][coordenada_final[0]] =0;

						//testing
#ifdef mitest
							if (matriz_check[coordenada_final[0]][coordenada_final[1]] ) {
								cout<<"SOBREESCRIBIENDO EN MATRIZ DE DATOS!!!!!!!!"<<endl;
								cin.get();
							} else {
							matriz_check[coordenada_final[0]][coordenada_final[1]] =1;
							}
#endif
					}

					if (!matriz_shadow[coordenada_final[1]][coordenada_final[0]] ) {
#ifdef midebug
						cout << "writing-0-S en:(" << coordenada_final[1] << ","
								<< coordenada_final[0] << ")" << endl;
#endif
						matriz_datos[coordenada_final[0]][coordenada_final[1]] =0;
						matriz_datos[coordenada_final[1]][coordenada_final[0]] =0;

						//testing
#ifdef mitest
						if (matriz_check[coordenada_final[1]][coordenada_final[0]]) {
							cout<< "SOBREESCRIBIENDO EN MATRIZ DE DATOS!!!!!!!!"<< endl;
							cin.get();
						} else {
							matriz_check[coordenada_final[1]][coordenada_final[0]] =1;
						}
#endif
					}
				}
			}

		} else {


#ifdef midebug
			cout << "Regla Conflict ......" << endl;
#endif

			// Escribo las tuplas correspondientes a cero.
			for (itero_parejas = las_tuplas.begin();
					itero_parejas != las_tuplas.end(); ++itero_parejas) {
				
				itero_dentro_de_la_pareja = itero_parejas->begin();

#ifdef midebug
				cout << "Primer valor Tupla: " << *itero_dentro_de_la_pareja
						<< endl;
#endif

				coordenada_final[0] = coordenadas_base[0]
						+ (*itero_dentro_de_la_pareja)
						- minimo_variable[var_cero];

				itero_dentro_de_la_pareja++;
#ifdef midebug
				cout << "Segundo valor Tupla: " << *itero_dentro_de_la_pareja
						<< endl;
#endif

				coordenada_final[1] = coordenadas_base[1]
						+ (*itero_dentro_de_la_pareja)
						- minimo_variable[var_uno];

#ifdef midebug
				cout << "writing-0-C en:(" << coordenada_final[0] << ","
						<< coordenada_final[1] << ")" << endl;
#endif
				matriz_datos[coordenada_final[0]][coordenada_final[1]] = 0;
				matriz_datos[coordenada_final[1]][coordenada_final[0]] = 0;

				//testing
#ifdef mitest
				if (matriz_check[coordenada_final[0]][coordenada_final[1]] ||
					matriz_check[coordenada_final[1]][coordenada_final[0]] ) {
					cout << "SOBREESCRIBIENDO EN MATRIZ DE DATOS!!!!!!!!"<< endl;
					cin.get();
				} else {
					matriz_check[coordenada_final[0]][coordenada_final[1]] = 1;
					matriz_check[coordenada_final[1]][coordenada_final[0]] = 1;
				}
#endif

			}
		}
	}






	//Funcion que escribe en la matriz reglas ternarias
	void escribe_en_matriz_ternaria(int *coordenadas_base, vector<vector<int> >& tuplas,
			string var_cero, string var_uno, int orden0, int orden1, bool support) 
	{
		std::vector<vector<int>>::iterator itero_tuplas;
		vector<vector<int>> valores_tupla;
		vector <int> primera_tupla;
		int coordenada_final[2];

		

#ifdef midebug
		cout << "Pareja de la Regla n-aria, índices: " << orden0 << "," << orden1  
		<< "--> support: " << support << endl;
#endif
		
		for (itero_tuplas = tuplas.begin(); itero_tuplas != tuplas.end(); ++itero_tuplas)
		{
			primera_tupla.clear();
			valores_tupla.clear();

			primera_tupla.push_back((*itero_tuplas)[orden0]);
			primera_tupla.push_back((*itero_tuplas)[orden1]);

			valores_tupla.push_back(primera_tupla);

#ifdef midebug			
			cout << "Valores Tupla n-aria a binaria: " << valores_tupla[0] [0] << "," << primera_tupla[0]
			<< " - " << valores_tupla[0][1] << "," << primera_tupla[1] << endl;
#endif

			escribe_en_matriz(coordenadas_base,valores_tupla,var_cero,var_uno,support);
		}

	}









	//Funcion que escribe en la matriz una regla AllEqual o AllDifferent
	void  escribe_regla_all(int *coordenadas_base, string var_cero, string var_uno, int REGLA)
	{
		int i=0,j=0;
		int coordenada_final[2];

	if(REGLA==DIFERENTE)
	{
		for (i=minimo_variable[var_cero];i<(rango_variable[var_cero]+minimo_variable[var_cero]);i++)
		{	
			for (j=minimo_variable[var_uno];j<(rango_variable[var_uno]+minimo_variable[var_uno]);j++)
			{
#ifdef midebug
				cout << "i: " << i << " - j: " << j;
#endif
					if(i!=j)
					{
#ifdef midebug
						cout << "  -->  Son diferentes " ;
#endif
						coordenada_final[0]=coordenadas_base[0]+i;
						coordenada_final[1]=coordenadas_base[1]+j;
						matriz_datos[coordenada_final[0]][coordenada_final[1]] = 1;
						matriz_datos[coordenada_final[1]][coordenada_final[0]] = 1;
					}
					else {
						coordenada_final[0]=coordenadas_base[0]+i;
						coordenada_final[1]=coordenadas_base[1]+j;
						//if(matriz_datos[coordenada_final[0]][coordenada_final[1]] == 0 || matriz_datos[coordenada_final[1]][coordenada_final[0]] == 0)
						//	throw std::runtime_error("Error: UNA REGLA AllDifferent ESTÁ INTENTANDO ESCRIBIR EN UNA PARTE DE LA MATRIZ PREVIAMENTE ESCRITA");
						matriz_datos[coordenada_final[0]][coordenada_final[1]] = 0;
						matriz_datos[coordenada_final[1]][coordenada_final[0]] = 0;

					}
#ifdef midebug					
					cout << endl;
#endif
			}
				
		}
	}

	if(REGLA==IGUAL)
	{
		for (i=minimo_variable[var_cero];i<(rango_variable[var_cero]+minimo_variable[var_cero]);i++)
		{	
			for (j=minimo_variable[var_uno];j<(rango_variable[var_uno]+minimo_variable[var_uno]);j++)
			{
				cout << "i: " << i << " - j: " << j;

					if(i==j)
					{
#ifdef midebug
						cout << "  -->  Son iguales " ;
#endif
						coordenada_final[0]=coordenadas_base[0]+i;
						coordenada_final[1]=coordenadas_base[1]+j;
						matriz_datos[coordenada_final[0]][coordenada_final[1]] = 1;
						matriz_datos[coordenada_final[1]][coordenada_final[0]] = 1;
					}
					else {
						coordenada_final[0]=coordenadas_base[0]+i;
						coordenada_final[1]=coordenadas_base[1]+j;
						//if(matriz_datos[coordenada_final[0]][coordenada_final[1]] == 0 || matriz_datos[coordenada_final[1]][coordenada_final[0]] == 0)
						//	throw std::runtime_error("Error: UNA REGLA AllEqual ESTÁ INTENTANDO ESCRIBIR EN UNA PARTE DE LA MATRIZ PREVIAMENTE ESCRITA");
						matriz_datos[coordenada_final[0]][coordenada_final[1]] = 0;
						matriz_datos[coordenada_final[1]][coordenada_final[0]] = 0;

					}
#ifdef midebug	
					cout << endl;
#endif
			}
				
		}
	}
		
	}












/////////////////////////////////////////////
	/* ==========Fin de mis funciones============================================================

	 =========Comienzo de las funciones que invoca el parser ===================================== */
////////////////////////////////////////////





	void beginInstance(InstanceType type) {

#ifdef midebug
		cout << "Empieza Instancia tipo: " << type << " ............" << endl;
#endif

		//XCSP3PrintCallbacks::beginInstance(type);
	}









	void endInstance() {
		//pongo_diagonal_matriz_a_cero();

		//I/O: Nota-la matriz de datos no est� terminada todavia
		//Hay que eliminar las relaciones entra valores de la misma variable
		//TODO-cambiar la l�gica y hacerlo aqui

		cout <<"---------------------------------------------------"<<endl;
		std::vector<string>::iterator itero;
		for (itero = lista_arrays.begin(); itero != lista_arrays.end();	itero++) {
			cout << "Array: " << *itero << endl;
			cout << "Numero variables: " << numero_variable[*itero] << endl;
			cout << "Fila base de la matriz: " << base_array[*itero] << endl;
			cout << "Primer valor: " << minimo_variable[*itero]<< endl;
			cout << "Número de valores: " << rango_variable[*itero]<< endl;
			cout << endl;
		}

		cout << "Dimension total matriz: " << dimension_matriz << endl;
		cout << endl;
		cout << "FIN del parsing----------------" << endl;

		
	}





	// Se invoca cuando se comienza a procesar un array.
	// Se resetean los contadores para poder llevar registro del
	// tamaño del array y del rango de las variables.
	void beginVariableArray(string id) {

		cout << "Empiezo con el Array, reseteo los valores para el array:  " << id << endl;

		lista_arrays.push_back(id);
		array_actual = id;
		base_array[id] = base_siguiente_array;
		rango_variable[id] = 0;

		numero_variables = 0;
		rango_variables = 0;


		is_array=true;

		
	}









	// Se invoca cuando se termina de procesar todas las variables de un array.
	// En este momento se actualizan las variables globales.
	// Con esa información se realiza el cálculo para poder escribir en la matriz.
	void endVariableArray() {

		base_siguiente_array += (numero_variables * rango_variables);
		numero_variable[array_actual] = numero_variables;
		rango_variable[array_actual] = rango_variables;
		minimo_variable[array_actual] = minimo_variables;

		is_array=false;

#ifdef midebug
		cout << "Base siguiente array: " << base_siguiente_array << endl;
		cout << "Numero variables: " << numero_variables << " - Rango: "
				<< rango_variables << endl;
#endif

	}








	// Comienza el proceso de variables. De momento no se hace nada.
	void beginVariables() {


#ifdef midebug
		cout << " - Comienza la declaracion de variables - " << endl;
#endif

	}










	// Se invoca al terminar de procesar las variables.
	// Escribe el fichero .csp que contiene todas las variables con sus rangos.
	// Genera la matriz, que una vez escrita, servirá para generar el grafo.
	void endVariables() {

		//Escribo el fichero .csp
		escribe_nombre_fichero();

		// Genero la matriz
		cout << "Genero la matriz ............." << endl;
		genera_matriz();
		cout << "Matriz generada .............." << endl;
#ifdef midebug
		print_coordenadas_base();
		cout << " - FIN declaracion variables - " << endl << endl;
#ifdef mipause
		cin.get();
#endif
#endif
	}








	//PSS calls here alsp for variables with singleton values (<var id="x0"> -1 <\var> )
	void buildVariableInteger(string id, int minValue, int maxValue) override {

		lista_variables.push_back(id);
		mapa_indices[id]=numero_variables;
		rango_variables = (maxValue - minValue) + 1;
		minimo_variables = minValue;					/*TODO-hay variables (singleton) con valor -1!!*/
		numero_variables++;
		cout << "Variable: " << id << " indice var: "<< (numero_variables-1) << " - min: " << minValue << " - max: "
				<< maxValue << endl;

		//PSS-treats the case of singleton variables
		if(!is_array){						/* variable extension to arrays: dirty */
			cout << "¡¡¡ Soy Singelton !!!" << endl;
			lista_arrays.push_back(id);
			base_array[id] = base_siguiente_array;
			numero_variables=1;

			base_siguiente_array += rango_variables;
			numero_variable[id] = 1;
			rango_variable[id] = rango_variables;
			minimo_variable[id] = minimo_variables;
		}


#ifdef midebug
		cout << "Array actual " << array_actual << endl;
		cout << "Rango valores: " << rango_variables
				<< " - Instancia Variable: " << (numero_variables-1)
				<< " - Minimo valor Variable: " << minimo_variables << endl;
#endif

	}









	//called for stand-alone values independent of a range: we assume they DO belong to a range
	void buildVariableInteger(string id, vector<int> &values) override {

		lista_variables.push_back(id);
		rango_variables = values.size();
		minimo_variables = values.front(); 		/*TODO-extend to non-index values */
		mapa_indices[id]=numero_variables;
		numero_variables++;

		cout << "Variable: " << id << " - min: " << values[0] << " - max: "
				<< values.back() << " Índice: " << mapa_indices[id] <<  endl;


		//treats the case of singleton variables
		if (!is_array) { /* variable extension to arrays: dirty */
			cout << "¡¡¡ Soy Singelton !!!" << endl;
			lista_arrays.push_back(id);
			base_array[id] = base_siguiente_array;
			numero_variables = 1;

			base_siguiente_array += rango_variables;
			numero_variable[id] = 1;
			rango_variable[id] = rango_variables;
			minimo_variable[id] = minimo_variables;
		}


//		cout << "   Variable con valores discretos: " << id << " : ";
//    	cout << "        ";
//    	displayList(values);
	}






	//Versión para Restricciones UNARIAS
	void buildConstraintExtension(string id, XVariable *variable, vector<int> &tuples, bool support, bool hasStar) {
		string var_cero, var_uno, var_aux;
		int indice0, indice1, indice_aux;
		int direccion;
		int coordenadas_base[2];
		vector<vector<int>>::iterator itero_parejas;

    	cout << "\n  Extension-Constraint UNARIA en buildConstraintExtension(): " << id << endl;
    	cout << "       regla " << (support ? "support" : "conflict") << ", número tuplas: " << tuples.size() 
			<< " star: " << hasStar << endl;



		tuplas_unarias=tuples;
		cout << "La variables: " << (variable->id)	<< endl;

		indice0 = get_indice(*variable);
		indice1 = 0;
		var_cero = get_nombre(variable->id);
		var_uno = lista_variables[0];
		
		
		calcula_coordenadas_base(var_cero, var_uno, indice0, indice1,coordenadas_base);
		
		#ifdef midebug
		cout << "Coordenada base calculada: " << coordenadas_base[0] << " - "
				<< coordenadas_base[1] << endl;
		#endif
		escribe_en_matriz_unaria(coordenadas_base, tuplas_unarias, var_cero, support);

#ifdef midebug
		cout << "Tama�o tuplas: " << tuplas_unarias.size() << endl;
#endif

		
#ifdef midebug
			if (support)
				cout << "escribiendo support en: " << "(" << var_cero << ","
						<< var_uno << ")" << endl;
			else
				cout << "escribiendo conflict en: " << "(" << var_cero << ","
						<< var_uno << ")" << endl;
#endif

#ifdef mipause
        	cin.get();
#endif
	

#ifdef midebug
		cout << "\n ** Fin buildConstraintExtension ** " << id << endl;
#endif

	}






	//Versión para restricciones binarias
	void buildConstraintExtension(string id, vector<XVariable *> list,
			vector<vector<int>> &tuples, bool support, bool hasStar) {

		string var_cero, var_uno;
		int indice0, indice1, i,j,k;
		int coordenadas_base[2];
		vector<vector<int>>::iterator itero_parejas;

		cout<< "Parsing buildConstraintExtension..........................................."<< endl;

		// Guardo el valor de las tuplas por si es una restriccion de grupo
		las_tuplas=tuples;
		
		cout << "Número variables: " << list.size() << endl;


		if (list.size() == 2){
			cout << "Par de variables: " << (list[0]->id) << " - " << (list[1]->id)	<< endl;

			indice0 = get_indice(*(list[0]));
			indice1 = get_indice(*(list[1]));
			var_cero = get_nombre(list[0]->id);
			var_uno = get_nombre(list[1]->id);
			calcula_coordenadas_base(var_cero, var_uno, indice0, indice1,coordenadas_base);
			escribe_en_matriz(coordenadas_base, las_tuplas, var_cero, var_uno, support);
		}

		if (list.size() >= 3){

			displayList(list);
			

			for (k=0;k<(list.size()-1);k++)
			{
				for(i=k,j=i+1; j<list.size();j++)
				{
					cout << "Pareja: " << list[i]->id << " , " << list[j]->id << endl;

					indice0 = get_indice(*(list[i]));
					indice1 = get_indice(*(list[j]));
	#ifdef midebug
					cout << "Índices: " << indice0 << " - " << i << " , " << indice1 << " - " << j << endl;
	#endif
					var_cero = get_nombre(list[i]->id);
					var_uno = get_nombre(list[j]->id);

					calcula_coordenadas_base(var_cero, var_uno, indice0, indice1,coordenadas_base);
					escribe_en_matriz_ternaria(coordenadas_base, las_tuplas, 
					var_cero, var_uno, i, j,support);
			}
		}
			
		} 

/* #ifdef midebug
		cout << "Coordenada base calculada: " << coordenadas_base[0] << " - "
				<< coordenadas_base[1] << endl;
#endif

#ifdef midebug
		cout << "Tama�o tuplas: " << las_tuplas.size() << endl;
#endif

		
#ifdef midebug
			if (support)
				cout << "escribiendo support en: " << "(" << var_cero << ","
						<< var_uno << ")" << endl;
			else
				cout << "escribiendo conflict en: " << "(" << var_cero << ","
						<< var_uno << ")" << endl;
#endif
#ifdef mipause
        	cin.get();
#endif
		
		

#ifdef midebug
		cout << "\n ** Fin buildConstraintExtension ** " << id << endl;
#endif */

	}







	//Versión para restricciones Unarias y Binarias.
	void buildConstraintExtensionAs(string id, vector<XVariable *> list,
			bool support, bool hasStar) {
		
		int i,j,k;
		string var_cero, var_uno, var_aux;
		int indice0, indice1, indice_aux;
		int coordenadas_base[2];

		vector<vector<int>>::iterator it;
		vector<int>::iterator ite;


		cout<< "Parsing buildConstraintExtension  AS ........................................."<< endl;
		cout << "Tamaño de la lista: " << list.size() << endl;
		displayList(list);
		
		if(list.size()==0)
		{
			throw runtime_error("Tamaño de tupla no procesado.");
			exit(2);
		}


		if (list.size() == 1){
			cout << "Variable Unaria: " << (list[0]->id) << endl;

			indice0 = get_indice(*(list[0]));
			indice1 = indice0;

			var_cero = get_nombre(list[0]->id);
			var_uno = var_cero;
			calcula_coordenadas_base(var_cero, var_uno, indice0, indice1,coordenadas_base);
			escribe_en_matriz_unaria(coordenadas_base, tuplas_unarias, var_cero, support);
		
		} 

		
		
		if (list.size() == 2){
			cout << "Par de variables: " << (list[0]->id) << " - " << (list[1]->id)	<< endl;

			indice0 = get_indice(*(list[0]));
			indice1 = get_indice(*(list[1]));

			var_cero = get_nombre(list[0]->id);
			var_uno = get_nombre(list[1]->id);

			calcula_coordenadas_base(var_cero, var_uno, indice0, indice1,coordenadas_base);
			
			cout << "Escribo binaria" << endl;
			escribe_en_matriz(coordenadas_base, las_tuplas, var_cero, var_uno,
					support);
		} 
		
		if (list.size() >= 3)
		{
			//displayList(list);
			

			for (k=0;k<(list.size()-1);k++)
			{
				for(i=k,j=i+1; j<list.size();j++)
				{
					cout << "Pareja: " << list[i]->id << " , " << list[j]->id << endl;

					indice0 = get_indice(*(list[i]));
					indice1 = get_indice(*(list[j]));
	#ifdef midebug
					cout << "Índices: " << indice0 << " , " << indice1 << endl;
	#endif
					var_cero = get_nombre(list[i]->id);
					var_uno = get_nombre(list[j]->id);

					calcula_coordenadas_base(var_cero, var_uno, indice0, indice1,coordenadas_base);
					escribe_en_matriz_ternaria(coordenadas_base,las_tuplas, var_cero, var_uno,i,j,support);
				}
			}
			
		} 
		
/* 		

#ifdef midebug
		cout << "Coordenada base calculada: " << coordenadas_base[0] << " - "
				<< coordenadas_base[1] << endl;
#endif

#ifdef midebug
		cout << "Tamaño tuplas: " << las_tuplas.size() << endl;
		cout << "Tamaño tuplas individuales: " << tuplas_unarias.size() << endl;
#endif

		
#ifdef midebug
			if (support)
				cout << "escribiendo support en: " << "(" << var_cero << ","
						<< var_uno << ")" << endl;
			else
				cout << "escribiendo conflict en: " << "(" << var_cero << ","
						<< var_uno << ")" << endl;
#endif

#ifdef mipause
        	cin.get();
#endif
			

#ifdef midebug
		cout << "\n ** Fin buildConstraintExtensionAS ** " << id << endl;
#endif */

		
	}







////////////////////
//
// PROCESSING Group
//
///////////////////

// Sin uso de momento

	void beginGroup(string id) {


		cout << "Comienzo Grupo ....... " << id << endl;


		
	}





	void endGroup() {


		cout << "Fin Grupo .......\n\n " << endl;
		
	}













////////////////////
//
// PROCESSING ALL DIFFERENT
//
///////////////////

	void buildConstraintAlldifferent(string id, vector<XVariable *> &list) {
    	
		int indice0,indice1;
		string var_cero, var_uno;
		int coordenadas_base[2];
		int i,j,k;
		int REGLA;

		REGLA=DIFERENTE;		
		cout << "\n   Mi allDiff constraint " << id << "Tamaño de la tupla: "<< list.size() << endl;
		displayList(list);
		
		for (k=0;k<(list.size()-1);k++)
		{
			for(i=k,j=i+1; j<list.size();j++)
			{
#ifdef midebug
				cout << "Pareja: " << list[i]->id << " , " << list[j]->id << endl;
#endif
				indice0 = get_indice(*(list[i]));
				indice1 = get_indice(*(list[j]));
				var_cero = get_nombre(list[i]->id);
				var_uno = get_nombre(list[j]->id);

				calcula_coordenadas_base(var_cero, var_uno, indice0, indice1,coordenadas_base);
				escribe_regla_all(coordenadas_base,var_cero,var_uno,REGLA);
			}
		}


		
	}







	void buildConstraintAlldifferentMatrix(string id, vector<vector<XVariable *>> &matrix) {
  		cout << "\n  ¡Mi!  allDiff matrix constraint" << id << endl;
   		for(unsigned int i = 0 ; i < matrix.size() ; i++) {
        	cout << "    i:    " << i << "  ";
        	displayList(matrix[i]);
    	}
	}






	void buildConstraintAlldifferentList(string id, vector<vector<XVariable *>> &lists) {
    	cout << "\n  ¡Mi!  allDiff list constraint" << id << endl;
    	for(unsigned int i = 0 ; i < (lists.size() < 4 ? lists.size() : 3) ; i++) {
        	cout << "        ";
        	displayList(lists[i]);
    	}
	}









////////////////////
//
// PROCESSING ALL EQUAL
//
///////////////////


	void buildConstraintAllEqual(string id, vector<XVariable *> &list) {
    	
		int indice0,indice1;
		string var_cero, var_uno;
		int coordenadas_base[2];
		int i,j,k;
		int REGLA;

		REGLA=IGUAL;
		cout << "\n   Mi allEqual constraint " << id << "Tamaño de la tupla: "<< list.size() << endl;
		
		
		for (k=0;k<(list.size()-1);k++)
		{
			for(i=k,j=i+1; j<list.size();j++)
			{
				cout << "Pareja: " << list[i]->id << " , " << list[j]->id << endl;

				indice0 = get_indice(*(list[i]));
				indice1 = get_indice(*(list[j]));
				var_cero = get_nombre(list[i]->id);
				var_uno = get_nombre(list[j]->id);

				calcula_coordenadas_base(var_cero, var_uno, indice0, indice1,coordenadas_base);
				escribe_regla_all(coordenadas_base,var_cero,var_uno,REGLA);
			}
		}


		



	}




	// string id, vector<XVariable *> &list, int startIndex
	void buildConstraintChannel(string, vector<XVariable *> &list, int startIndex) {
    	cout << "\n   1) channel constraint" << endl;
    	cout << "        ";
    	displayList(list);
    	cout << "        Start index : " << startIndex << endl;
	}


	// string id, vector<XVariable *> &list1, int startIndex1, vector<XVariable *> &list2, int startIndex2
	void buildConstraintChannel(string, vector<XVariable *> &list1, int, vector<XVariable *> &list2, int) {
    	cout << "\n    2) channel constraint" << endl;
    	cout << "        list1 ";
    	displayList(list1);
    	cout << "        list2 ";
    	displayList(list2);
	}


	// string id, vector<XVariable *> &list, int startIndex, XVariable *value
	void buildConstraintChannel(string, vector<XVariable *> &list, int, XVariable *value) {
    	cout << "\n   3) channel constraint" << endl;
    	cout << "        ";
    	displayList(list);
    	cout << "        value: " << *value << endl;
	}


	// string id, vector<XVariable *> &list, vector<int> &values
	void buildConstraintInstantiation(string, vector<XVariable *> &list, vector<int> &values) {
    	cout << "\n   Mi instantiation constraint" << endl;
    	cout << "        list:";
    	displayList(list);
    	cout << "        values:";
    	displayList(values);

	}


	
};










///////////////////
//
// THE ONE AND ONLY MAIN
//
///////////////////

int main(int argc, char **argv) {
	MiSolverPrintCallbacks miparser;
	char *nombre_fichero_dimacs;

	

	if (argc != 2) {
		throw std::runtime_error("usage: ./csp xcsp3instance.xml");
		return 0;
	}

	miparser.set_nombre_fichero(argv[1]);

/////////////////
//PARSING
	try {
		XCSP3CoreParser parser(&miparser);
		parser.parse(argv[1]); // fileName is a string
	} catch (exception &e) {
		cout.flush();
		cerr << "\n\tUnexpectedd exxception: \n";
		cerr << "\t" << e.what() << endl;
		exit(1);
	}
///////////////////
//GENERACION DE UGRAPH

	// Una vez leido el fichero y generada la matriz, se vuelca en un Grafo y se resuelve
	ugraph ug(miparser.dimension_matriz);

	for (int i = 0; i < (miparser.dimension_matriz - 1); i++){
		for (int j = i + 1; j < miparser.dimension_matriz; j++) {
			if (miparser.matriz_datos[i][j] == 1) {
				ug.add_edge(i, j);
			}
		}
	}

	//removes incompatible edges between values of the same variable-  MUST BE!
	//miparser.remove_edges_same_var(ug);
	////////////////////

	ug.set_name(miparser.nombre_fichero, false);
	ug.print_data(false /* from scratch*/, cout);

	nombre_fichero_dimacs = strrchr(miparser.nombre_fichero, '.');
	strcpy(nombre_fichero_dimacs, ".clq");

	std::fstream f(miparser.nombre_fichero, ios::out);
	ug.write_dimacs(f);
	f.close();

	//salida matriz de datos
	ofstream fmat("log_mat.txt", ios::out);
	miparser.imprime_matriz("datos", fmat);
	miparser.imprime_matriz("datos",fmat);
	fmat.close();

	/* cout << "\n\nEl resultado de la matriz de DATOS ......................\n " << endl;
	ostream & terminal=cout;
	miparser.imprime_matriz("datos", terminal);

	cout << "\n\nEl resultado de la matriz SHADOW ......................\n " << endl;
	miparser.imprime_matriz("shadow", terminal); */



	return 0;
}

