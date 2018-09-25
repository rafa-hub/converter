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
//#define midebug
//#define mitest
#define RESTRICCION 0
#define SOPORTE 1
#define CREAR_MATRIZ 1
#ifndef BENCHMARK_PATH 
#define BENCHMARK_PATH  "/var/tmp/salida"
#endif

using namespace XCSP3Core;

class MiSolverPrintCallbacks: public XCSP3PrintCallbacks {

private:

	vector<string> lista_arrays;    		// Guarda la lista de arrays
	vector<string> lista_variables; 		// Guarda la lista de variables
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

public:

	int dimension_matriz = 0; 			//Guarda la dimension definitiva de la matriz creada
	vector<vector<int>> matriz_datos; 	// Matriz donde se almacena el resultado
	vector<vector<int>> matriz_shadow; 	// Matriz donde se almacena el resultado
#ifdef mitest
	vector<vector<int>> matriz_check; 	// Matriz donde se almacena el resultado
#endif

	char nombre_fichero[256]; 			// Nombre del fichero XML a procesar

	void set_nombre_fichero(char *nombre) {
		strcpy(nombre_fichero, nombre);
	}

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
		string indice_str, valor;
		size_t aux1, aux2;

		valor = variable.id;

		aux1 = valor.find_first_of('[', 0);
		aux2 = valor.find_first_of(']', aux1);
		if (aux1 != string::npos)
			indice_str = valor.substr(aux1 + 1, aux2 - 2);
		else {
			//singleton variable
			indice_str = "0";
		}


#ifdef midebug
		cout << valor << " indice: " << indice_str << endl;
#endif

		return (std::stoi(indice_str));
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
		cout << "Var cero: " << var_cero << " - indice: " << indice0 << endl;
		cout << "Var uno: " << var_uno << " - indice: " << indice1 << endl;
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
		fila.assign(dimension_matriz,1);
		fila_shadow.assign(dimension_matriz,0);

//		for (int j = 0; j < dimension_matriz; j++) {
//			fila.push_back(1);
//			fila_shadow.push_back(0);
//		}

		for (int j = 0; j < dimension_matriz; j++) {
			matriz_datos.push_back(fila);
			matriz_shadow.push_back(fila_shadow);
#ifdef mitest
			matriz_check.push_back(fila_shadow);
#endif
		}

#ifdef midebug
		cout << "Matriz creada ........ \nDimension de la matriz: "	<< matriz_datos.size() << endl;
#endif
	}

	// Certificacion de que la matriz tiene la diagonal principal a cero
	void pongo_diagonal_matriz_a_cero() {
		for (int x = 0; x < dimension_matriz; x++) {
			matriz_datos[x][x] = 0;
		}
	}

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

	//Funcion que escribe en la matriz
	void escribe_en_matriz(int *coordenadas_base, vector<vector<int> >& tuplas,
			string var_cero, string var_uno, bool support) {
		//vector<vector<int>>::iterator it;

		std::vector<vector<int>>::iterator itero_parejas;
		vector<int>::iterator itero_dentro_de_la_pareja;
		int coordenada_final[2];

		//support

		if (support) {

			cout << "Soy support ...................." << endl;
			cout << "Var_0:" << var_cero << " min var: "
					<< minimo_variable[var_cero] << endl;
			cout << "Var_1:" << var_uno << " min var: "
					<< minimo_variable[var_uno] << endl;

			// No hay tuplas y es una regla support => todo a ceros

			if (tuplas.size()==0)
			{
				cout << "CONJUNTO DE TUPLAS VACIO: TODO A CEROS" << endl;
				for (int i = 0; i < rango_variable[var_cero]; i++)
					for (int j = 0; j < rango_variable[var_uno]; j++) {
						coordenada_final[0] = coordenadas_base[0] + i;
						coordenada_final[1] = coordenadas_base[1] + j;
						if (!matriz_shadow[coordenada_final[0]][coordenada_final[1]]) {
//#ifdef midebug
							cout << "writing-0-S en:(" << coordenada_final[0] << ","
								<< coordenada_final[1] << ")" << endl;
//#endif
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

			cout << "Soy una regla Conflict ......" << endl;

			// Escribo las tuplas correspondientes a cero.
			for (itero_parejas = las_tuplas.begin();
					itero_parejas != las_tuplas.end(); ++itero_parejas) {
				
				itero_dentro_de_la_pareja = itero_parejas->begin();

				//#ifdef midebug
				cout << "Primer valor Tupla: " << *itero_dentro_de_la_pareja
						<< endl;
				//#endif

				coordenada_final[0] = coordenadas_base[0]
						+ (*itero_dentro_de_la_pareja)
						- minimo_variable[var_cero];

				itero_dentro_de_la_pareja++;
				//#ifdef midebug
				cout << "Segundo valor Tupla: " << *itero_dentro_de_la_pareja
						<< endl;
				//#endif
				coordenada_final[1] = coordenadas_base[1]
						+ (*itero_dentro_de_la_pareja)
						- minimo_variable[var_uno];
//#ifdef midebug
				cout << "writing-0-C en:(" << coordenada_final[0] << ","
						<< coordenada_final[1] << ")" << endl;
//#endif
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
		pongo_diagonal_matriz_a_cero();

		//I/O: Nota-la matriz de datos no est� terminada todavia
		//Hay que eliminar las relaciones entra valores de la misma variable
		//TODO-cambiar la l�gica y hacerlo aqui

		//	cout << "\nLa matriz resultante: " << endl;
			imprime_matriz("datos");
		//	cout << "-----------------------------------------------" << endl;
		//	cout << "-----------------------------------------------" << endl;
			imprime_matriz("shadow");

		cout <<"---------------------------------------------------"<<endl;
		std::vector<string>::iterator itero;
		for (itero = lista_arrays.begin(); itero != lista_arrays.end();	itero++) {
			cout << "Array: " << *itero << endl;
			cout << "Numero variables: " << numero_variable[*itero] << endl;
			cout << "Fila base de la matriz: " << base_array[*itero] << endl;
			cout << "Primer valor: " << minimo_variable[*itero]<< endl;
			cout << "N�mero de valores: " << rango_variable[*itero]<< endl;
			cout << endl;
		}

		cout << "Dimension total matriz: " << dimension_matriz << endl;
		cout << endl;
		cout << "FIN del parsing----------------" << endl;

		//XCSP3PrintCallbacks::endInstance();
	}

	void beginVariableArray(string id) {

		lista_arrays.push_back(id);
		array_actual = id;
		base_array[id] = base_siguiente_array;
		rango_variable[id] = 0;

		numero_variables = 0;
		rango_variables = 0;


		is_array=true;

		//XCSP3PrintCallbacks::beginVariableArray(id);
	}

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

		//XCSP3PrintCallbacks::endVariableArray();
	}

	void beginVariables() {


#ifdef midebug
		cout << " - Comienza la declaracion de variables - " << endl;
#endif

		//XCSP3PrintCallbacks::beginVariables();
	}

	void endVariables() {

		//Escribo el fichero .csp
		escribe_nombre_fichero();

		// Genero la matriz
		genera_matriz();

#ifdef midebug
		print_coordenadas_base();
		cout << " - FIN declaracion variables - " << endl << endl;
#ifdef mipause
		cin.get();
#endif
#endif

		//XCSP3PrintCallbacks::endVariables();
	}

	//PSS calls here alsp for variables with singleton values (<var id="x0"> -1 <\var> )
	void buildVariableInteger(string id, int minValue, int maxValue) override {

		lista_variables.push_back(id);
		rango_variables = (maxValue - minValue) + 1;
		minimo_variables = minValue;					/*TODO-hay variables (singleton) con valor -1!!*/
		numero_variables++;
		cout << "Variable: " << id << " - min: " << minValue << " - max: "
				<< maxValue << endl;

		//PSS-treats the case of singleton variables
		if(!is_array){						/* variable extension to arrays: dirty */
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
				<< " - Instancia Variable: " << numero_variables
				<< " - Minimo valor Variable: " << minimo_variables << endl;
#endif

		//XCSP3PrintCallbacks::buildVariableInteger(id,minValue,maxValue);
	}

	//called for stand-alone values independent of a range: we assume they DO belong to a range
	void buildVariableInteger(string id, vector<int> &values) override {

		lista_variables.push_back(id);
		rango_variables = values.size();
		minimo_variables = values.front(); 		/*TODO-extend to non-index values */
		numero_variables++;

//#ifdef mydebug
		cout << "Variable: " << id << " - min: " << values[0] << " - max: "
				<< values.back() << endl;
//#endif

		//treats the case of singleton variables
		if (!is_array) { /* variable extension to arrays: dirty */
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

	void beginConstraints() {

		//XCSP3PrintCallbacks::beginConstraints();
	}

	void endConstraints() {

		//XCSP3PrintCallbacks::endConstraints();
	}

	void buildConstraintExtension(string id, vector<XVariable *> list,
			vector<vector<int>> &tuples, bool support, bool hasStar) {

		string var_cero, var_uno, var_aux;
		int indice0, indice1, indice_aux;
		int coordenadas_base[2];
		vector<vector<int>>::iterator itero_parejas;

		cout<< "Parsing buildConstraintExtension..........................................."<< endl;

		// Guardo el valor de las tuplas por si es una restriccion de grupo
		las_tuplas=tuples;
		cout << "Par de variables: " << (list[0]->id) << " - " << (list[1]->id)	<< endl;

		indice0 = get_indice(*(list[0]));
		indice1 = get_indice(*(list[1]));
		var_cero = get_nombre(list[0]->id);
		var_uno = get_nombre(list[1]->id);


		if (list.size() == 2){
			calcula_coordenadas_base(var_cero, var_uno, indice0, indice1,coordenadas_base);
		} else{
			throw std::runtime_error("Error, este código sólo funciona con relaciones binarias");
			exit(2);
		}



#ifdef midebug
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
		escribe_en_matriz(coordenadas_base, las_tuplas, var_cero, var_uno,
					support);
		

#ifdef midebug
		cout << "\n ** Fin buildConstraintExtension ** " << id << endl;
#endif

		//XCSP3PrintCallbacks::buildConstraintExtension(id, list,tuples,support,hasStar);
	}

	void buildConstraintExtensionAs(string id, vector<XVariable *> list,
			bool support, bool hasStar) {

		string var_cero, var_uno, var_aux;
		int indice0, indice1, indice_aux;
		int coordenadas_base[2];

		vector<vector<int>>::iterator it;
		vector<int>::iterator ite;

		cout<< "Parsing buildConstraintExtension  AS ........................................."<< endl;
		cout << "Par de variables: " << (list[0]->id) << " - " << (list[1]->id)	<< endl;

		indice0 = get_indice(*(list[0]));
		indice1 = get_indice(*(list[1]));

		var_cero = get_nombre(list[0]->id);
		var_uno = get_nombre(list[1]->id);

		if (list.size() == 2){
			calcula_coordenadas_base(var_cero, var_uno, indice0, indice1,coordenadas_base);
		} else{
			throw std::runtime_error("Error, este código sólo funciona con relaciones binarias");
			exit(2);
		}
		

		//calcula_coordenadas_base(*(list[0]),*(list[1]),coordenadas_base);

#ifdef midebug
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
		escribe_en_matriz(coordenadas_base, las_tuplas, var_cero, var_uno,
					support);
		

#ifdef midebug
		cout << "\n ** Fin buildConstraintExtensionAS ** " << id << endl;
#endif

		//XCSP3PrintCallbacks::buildConstraintExtensionAs(id,list,support,hasStar);
	}

	void beginGroup(string id) {

#ifdef midebug
		cout << "Comienzo Grupo ....... " << id << endl;
#endif

		//XCSP3PrintCallbacks::beginGroup(id);
	}

	void endGroup() {

#ifdef midebug
		cout << "Fin Grupo .......\n\n " << endl;
#endif

		//XCSP3PrintCallbacks::endGroup();
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


	cout << "\nPrueba de que compila ................. \n" << endl;


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
	miparser.remove_edges_same_var(ug);
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
	fmat.close();


/////////////////////////////////////////////
//TESTING:TODO-use test framework
//Instance: driveslogw-01c
//A-CONFLICTING CONSTRAINTS

#ifdef mitest
	string v1, val1, v2, val2;
	v1 = "x[2]", val1 = "1", v2 = "x[1]", val2 = "1";
	if (miparser.is_conflicting(v1, val1, v2, val2)) {
		cout << "CONFLICT:" << "(" << v1 << "=" << val1 << ")" << "(" << v2
				<< "=" << val2 << ")" << endl;
	}
	v1 = "x[31]", val1 = "1", v2 = "x[20]", val2 = "1";
	if (miparser.is_conflicting(v1, val1, v2, val2)) {
		cout << "CONFLICT:" << "(" << v1 << "=" << val1 << ")" << "(" << v2
				<< "=" << val2 << ")" << endl;
	}
	v1 = "x[24]", val1 = "1", v2 = "x[22]", val2 = "1";
	if (miparser.is_conflicting(v1, val1, v2, val2)) {
		cout << "CONFLICT:" << "(" << v1 << "=" << val1 << ")" << "(" << v2
				<< "=" << val2 << ")" << endl;
	}
	v1 = "x[27]", val1 = "1", v2 = "x[22]", val2 = "1";
	if (miparser.is_conflicting(v1, val1, v2, val2)) {
		cout << "CONFLICT:" << "(" << v1 << "=" << val1 << ")" << "(" << v2
				<< "=" << val2 << ")" << endl;
	}
	//non-conflicts
	v1 = "x[27]", val1 = "1", v2 = "x[22]", val2 = "0";
	if (miparser.is_conflicting(v1, val1, v2, val2)) {
		cout << "CONFLICT:" << "(" << v1 << "=" << val1 << ")" << "(" << v2
				<< "=" << val2 << ")" << endl;
	}
	v1 = "x[24]", val1 = "1", v2 = "x[22]", val2 = "0";
	if (miparser.is_conflicting(v1, val1, v2, val2)) {
		cout << "CONFLICT:" << "(" << v1 << "=" << val1 << ")" << "(" << v2
				<< "=" << val2 << ")" << endl;
	}




//B-SUPPORTS
	v1 = "z[4]", val1 = "0", v2 = "y[5]", val2 = "0";
	if (!miparser.is_conflicting(v1, val1, v2, val2)) {
		cout << "SUPPORT:" << "(" << v1 << "=" << val1 << ")" << "(" << v2
				<< "=" << val2 << ")" << endl;
	}
	v1 = "z[4]", val1 = "0", v2 = "y[5]", val2 = "1";
	if (!miparser.is_conflicting(v1, val1, v2, val2)) {
		cout << "SUPPORT:" << "(" << v1 << "=" << val1 << ")" << "(" << v2
				<< "=" << val2 << ")" << endl;
	}
	v1 = "z[4]", val1 = "0", v2 = "y[5]", val2 = "2";
	if (!miparser.is_conflicting(v1, val1, v2, val2)) {
		cout << "SUPPORT:" << "(" << v1 << "=" << val1 << ")" << "(" << v2
				<< "=" << val2 << ")" << endl;
	}
	v1 = "z[4]", val1 = "1", v2 = "y[5]", val2 = "0";
	if (!miparser.is_conflicting(v1, val1, v2, val2)) {
		cout << "SUPPORT:" << "(" << v1 << "=" << val1 << ")" << "(" << v2
				<< "=" << val2 << ")" << endl;
	}
	v1 = "z[4]", val1 = "2", v2 = "y[5]", val2 = "0";
	if (!miparser.is_conflicting(v1, val1, v2, val2)) {
		cout << "SUPPORT:" << "(" << v1 << "=" << val1 << ")" << "(" << v2
				<< "=" << val2 << ")" << endl;
	}
	v1 = "z[4]", val1 = "3", v2 = "y[5]", val2 = "0";
	if (!miparser.is_conflicting(v1, val1, v2, val2)) {
		cout << "SUPPORT:" << "(" << v1 << "=" << val1 << ")" << "(" << v2
				<< "=" << val2 << ")" << endl;
	}

	v1 = "z[2]", val1 = "0", v2 = "z[1]", val2 = "0";
	if (!miparser.is_conflicting(v1, val1, v2, val2)) {
		cout << "SUPPORT:" << "(" << v1 << "=" << val1 << ")" << "(" << v2
				<< "=" << val2 << ")" << endl;
	}

	v1 = "z[2]", val1 = "0", v2 = "z[1]", val2 = "1";
	if (!miparser.is_conflicting(v1, val1, v2, val2)) {
		cout << "SUPPORT:" << "(" << v1 << "=" << val1 << ")" << "(" << v2
				<< "=" << val2 << ")" << endl;
	}

	v1 = "z[2]", val1 = "0", v2 = "z[1]", val2 = "2";
	if (!miparser.is_conflicting(v1, val1, v2, val2)) {
		cout << "SUPPORT:" << "(" << v1 << "=" << val1 << ")" << "(" << v2
				<< "=" << val2 << ")" << endl;
	}

	v1 = "z[2]", val1 = "0", v2 = "z[1]", val2 = "3";
	if (!miparser.is_conflicting(v1, val1, v2, val2)) {
		cout << "SUPPORT:" << "(" << v1 << "=" << val1 << ")" << "(" << v2
				<< "=" << val2 << ")" << endl;
	}

	v1 = "z[2]", val1 = "1", v2 = "z[1]", val2 = "0";
	if (!miparser.is_conflicting(v1, val1, v2, val2)) {
		cout << "SUPPORT:" << "(" << v1 << "=" << val1 << ")" << "(" << v2
				<< "=" << val2 << ")" << endl;
	}

	v1 = "z[2]", val1 = "2", v2 = "z[1]", val2 = "0";
	if (!miparser.is_conflicting(v1, val1, v2, val2)) {
		cout << "SUPPORT:" << "(" << v1 << "=" << val1 << ")" << "(" << v2
				<< "=" << val2 << ")" << endl;
	}

	v1 = "z[2]", val1 = "3", v2 = "z[1]", val2 = "0";
	if (!miparser.is_conflicting(v1, val1, v2, val2)) {
		cout << "SUPPORT:" << "(" << v1 << "=" << val1 << ")" << "(" << v2
				<< "=" << val2 << ")" << endl;
	}

	//non-support
	v1 = "z[4]", val1 = "3", v2 = "y[5]", val2 = "1";
	if (!miparser.is_conflicting(v1, val1, v2, val2)) {
		cout << "SUPPORT:" << "(" << v1 << "=" << val1 << ")" << "(" << v2
				<< "=" << val2 << ")" << endl;
	}

	v1 = "z[2]", val1 = "1", v2 = "z[1]", val2 = "1";
	if (!miparser.is_conflicting(v1, val1, v2, val2)) {
		cout << "SUPPORT:" << "(" << v1 << "=" << val1 << ")" << "(" << v2
				<< "=" << val2 << ")" << endl;
	}

	v1 = "z[2]", val1 = "2", v2 = "z[1]", val2 = "1";
	if (!miparser.is_conflicting(v1, val1, v2, val2)) {
		cout << "SUPPORT:" << "(" << v1 << "=" << val1 << ")" << "(" << v2
				<< "=" << val2 << ")" << endl;
	}


//C-Incompatible values of same variable
	v1 = "z[0]", val1 = "0", v2 = "z[0]", val2 = "1";
	if (miparser.is_conflicting(v1, val1, v2, val2)) {
		cout << "CONFLICT:" << "(" << v1 << "=" << val1 << ")" << "(" << v2
				<< "=" << val2 << ")" << endl;
	}
	v1 = "z[0]", val1 = "0", v2 = "z[0]", val2 = "2";
	if (miparser.is_conflicting(v1, val1, v2, val2)) {
		cout << "CONFLICT:" << "(" << v1 << "=" << val1 << ")" << "(" << v2
				<< "=" << val2 << ")" << endl;
	}
	v1 = "z[0]", val1 = "0", v2 = "z[0]", val2 = "3";
	if (miparser.is_conflicting(v1, val1, v2, val2)) {
		cout << "CONFLICT:" << "(" << v1 << "=" << val1 << ")" << "(" << v2
				<< "=" << val2 << ")" << endl;
	}
	v1 = "z[0]", val1 = "1", v2 = "z[0]", val2 = "2";
	if (miparser.is_conflicting(v1, val1, v2, val2)) {
		cout << "CONFLICT:" << "(" << v1 << "=" << val1 << ")" << "(" << v2
				<< "=" << val2 << ")" << endl;
	}
	v1 = "z[0]", val1 = "1", v2 = "z[0]", val2 = "3";
	if (miparser.is_conflicting(v1, val1, v2, val2)) {
		cout << "CONFLICT:" << "(" << v1 << "=" << val1 << ")" << "(" << v2
				<< "=" << val2 << ")" << endl;
	}
	v1 = "z[0]", val1 = "2", v2 = "z[0]", val2 = "3";
	if (miparser.is_conflicting(v1, val1, v2, val2)) {
		cout << "CONFLICT:" << "(" << v1 << "=" << val1 << ")" << "(" << v2
				<< "=" << val2 << ")" << endl;
	}
	v1 = "x[0]", val1 = "0", v2 = "x[0]", val2 = "1";
	if (miparser.is_conflicting(v1, val1, v2, val2)) {
		cout << "CONFLICT:" << "(" << v1 << "=" << val1 << ")" << "(" << v2
				<< "=" << val2 << ")" << endl;
	}
	v1 = "y[0]", val1 = "2", v2 = "y[0]", val2 = "1";
	if (miparser.is_conflicting(v1, val1, v2, val2)) {
		cout << "CONFLICT:" << "(" << v1 << "=" << val1 << ")" << "(" << v2
				<< "=" << val2 << ")" << endl;
	}
#endif

	return 0;
}

