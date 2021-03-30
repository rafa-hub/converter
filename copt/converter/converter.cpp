#include "XCSP3CoreParser.h"
#include "XCSP3PrintCallbacks.h"
#include "XCSP3Tree.h"
#include "XCSP3TreeNode.h"

#include "utils/prec_timer.h"

#include <fstream>
#include <string.h>
#include <iostream>
#include <climits>
#include <map>
#include <math.h>
#include <time.h>

// #define mipause
#define midebug
// #define mitest


#define EXIT_CODE_SUCCESS 0
#define EXIT_CODE_ERROR_COMMAND 1
#define EXIT_CODE_NOT_IMPLEMENTED 2
#define EXIT_CODE_NUM_VAR_EXCEEDED 4
#define LIMITE_NUM_VARIABLES 8000
#define TERNARIA 3
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

	// Variables usadas en MiniZinc
	int minimo_variables = 0;        	// Guarda el minimo valor de cada array
	int maximo_variables = 0;        	// Guarda el minimo valor de cada array
	vector<vector<int>> las_tuplas;   	// Guarda las tuplas, puesto que en
	int indice_tabla = 0;				// Índice de las tablas que se van creando
	string tabla_actual;				// Apunta a la tabla que se está utilizando en el momento
	vector<string> lista_arrays;    	// Guarda la lista de arrays. Los arrays ya no se usan.
	vector<string> lista_variables; 	// Guarda la lista de variables.
	map<string, int> rango_array;	 	// Mapa de cada array con el rango de valores de las variables.
	map<string, int> rango_variable; 	// Mapa con el rango de valores de las variables.
	map<string, int> maximo_variable; 	// Guarda el máximo del rango de cada una de las variables.
	map<string, int> minimo_variable; 	// Guarda el minimo del rango de cada una de las variables.




	// Variables históricas.
	bool is_array=false;				// PSS-determina si una varaible es un singleton o forma parte de un array
										// Todas las variables ahora se tratan como singleton. 
	map<string,int> mapa_indices;		// Guarda el índice de cada variable.
	map<string, vector<int>> valores_variable;	// Guarda los valores discretos de una variable.
	string primera_variable = "Si";		// Permite calcular la base de las variables en la matriz, según se leen.
	string variable_anterior="Vacia";
	map<string, int> base_array; 		// Mapa de cada array con su coordenada base.
	map<string, int> base_variable;		// Mapa de cada Variable con su coordenada base, debe sustituir a base_array.
	map<string, int> numero_variable;	// Mapa de cada array con el numero de instancias.
										// de variables del array.
	string array_actual = "empiezo"; 	// Sirve para identificar con que array se esta trabajando
	int base_siguiente_array = 0; 		// Guarda el valor para calcular la posicion en la matriz del siguiente array
	
	int rango_variables = 0; 			// Guarda el rango de valores de las variables de un array
	int numero_variables = 0;      		// Guarda el numero de variables de un array

	
									  	// buildConstraintExtensionAs() no me las pasan como argumento
	vector<int> tuplas_unarias;			// Lo mismo, pero para variables unarias
	vector<int> tamano_tuplas;			// Vector que almacena el tamaño de las tuplas: (número de tuplas)
	

public:

	char nombre_fichero[256]; 			// Nombre del fichero MINIZINC a procesar
	char *nombre_fichero_mzn;			// Puntero al nombre del fichero
	FILE *fichero_mzn;					// Puntero a la estructura del fichero

	
	vector<string> lista_variables_discretas;	// Guarda la lista de variables con rango discreto.
	vector<int> lista_variables_ternarias;		// Guarda la lista de variables binarizadas, 
												// en cada posición se guarda el "número" de variables.
												// Sirve para generar el fichero CSP
	// int indice_var_ternarias_con_ceros = 0;		// Va indexando las variables ternarias. (a substituir por algo más consistente)
	// vector <int> dimension_variables_ternarias;	// Guarda el número de tuplas posibles para cada var ternaria.
	int **matriz_ternaria;

	int dimension_matriz = 0; 			//Guarda la dimension definitiva de la matriz creada.
	// int dimension_ternaria = 0;			// Guarda la dimensión de la matriz de vértices.
	
	int **matriz_datos; 	// Matriz donde se almacena el resultado.
	
	
	


	//////////////////////////////////////////////////////////////
	///
	///  FIN DECLARACIÓN DE VARIABLES GLOBALES DE LA CLASE
	///
	//////////////////////////////////////////////////////////////



	void set_nombre_fichero(char *nombre) {
		strcpy(nombre_fichero, nombre);
	}




	




void crea_fichero_mzn()
	{
		string var = nombre_fichero;
		string myText;
		
		
		// Procedo a escribir el fichero.
		nombre_fichero_mzn = strrchr(nombre_fichero, '.');
		strcpy(nombre_fichero_mzn, ".mzn");
		fichero_mzn = fopen(nombre_fichero,"w");
		cout << "Nombre fichero .MZN: " << nombre_fichero << endl;
		
		// Escribo la cabecera del fichero.

		myText = "% \\ Fichero en formato MINIZINC creado a partir del fichero XCSP3 " + var + "\n\n";

		fprintf(fichero_mzn,myText.c_str());	
		fprintf(fichero_mzn,"include \"table.mzn\";\n\n\n");
		fprintf(fichero_mzn,"% \\ Declaración de variables: \n\n");
		
	}







	void escribe_fichero_mzn(string texto)
	{

		fprintf(fichero_mzn,texto.c_str());

	}






	void cierra_fichero_mzn()
	{
		int i=0;
		string aux;

		fprintf(fichero_mzn,"\n\nsolve satisfy;\n\n");
		fprintf(fichero_mzn,"output [\"black-hole: \",");
		for (i=0;i<lista_arrays.size();i++){
			aux = "show(" + lista_arrays[i] + "),";
			fprintf(fichero_mzn, aux.c_str());
		}

		fprintf(fichero_mzn,"\"\\n\"];\n");

		fclose(fichero_mzn);
	}






/////////////////////////////////////////////
//	 ==========Fin de mis funciones=============================
//
//	 =========Comienzo de las funciones que invoca el parser ===
////////////////////////////////////////////







	// Se invoca cuando se comienza a procesar un array.
	// Se resetean los contadores para poder llevar registro del
	// tamaño del array y del rango de las variables.
	void beginVariableArray(string id) {

		cout << "Empiezo con el Array, reseteo los valores para el array:  " << id << endl;
		array_actual = id;
		lista_arrays.push_back(id);
		numero_variables = 0;
	}



	// Se invoca cuando se termina de procesar todas las variables de un array.
	// En este momento se actualizan las variables globales.
	// Con esa información se realiza el cálculo para poder escribir en la matriz.
	// Deprecated, ahora se calcula de otra manera.
	void endVariableArray() {

		string var_line = "array[0..";

		
		cout << "Fin array .......... " << array_actual  << endl;
		cout << "\t Número variables array .......... " << numero_variables  << endl;
		cout << "\t Rango:  " << minimo_variables << ".." << maximo_variables << endl;
		
		var_line = var_line + to_string(numero_variables-1) +"] of var " + to_string(minimo_variables) + ".." 
			+ to_string(maximo_variables) + ": " + array_actual + ";\n";
		
		rango_array[array_actual] = (maximo_variables - minimo_variables)+1;
		
		escribe_fichero_mzn(var_line);

	}











	// Comienza el proceso de variables. De momento no se hace nada.
	void beginVariables() {

		cout << "Empiezo con las Variables  ................" << endl;
		
	}










	// Se invoca al terminar de procesar las variables.
	// Escribe el fichero .csp que contiene todas las variables con sus rangos.
	// Genera la matriz, que una vez escrita, servirá para generar el grafo.
	void endVariables() {

		
		cout << " - FIN declaracion variables - " << endl << endl;

		escribe_fichero_mzn("\n\n");
	}













	void buildVariableInteger(string id, int minValue, int maxValue) override {
		
		cout << "Variable: " << id << " - Min value: " << minValue << " - Max vlue: " << maxValue << endl;
		numero_variables++;
		lista_variables.push_back(id);

		minimo_variables = minValue;
		maximo_variables = maxValue;
		rango_variable[id] = (maxValue-minValue)+1;
		maximo_variable[id] = maxValue;
		minimo_variable[id] = minValue;
		}














	//called for stand-alone values independent of a range: we assume they DO belong to a range
	void buildVariableInteger(string id, vector<int> &values) override {
		cout << "- ¡VARIABLES CON VALORES DISCRETOS! -" << endl;
		
	}










	//Versión para Restricciones UNARIAS
	void buildConstraintExtension(string id, XVariable *variable, vector<int> &tuples, bool support, bool hasStar) {
		cout << "Constraint UNARIA .............." << endl;
	}








	//Versión para restricciones binarias o superiores
	void buildConstraintExtension(string id, vector<XVariable *> list,
		vector<vector<int>> &tuples, bool support, bool hasStar) {

		vector<XVariable *>::iterator itero;
		vector<vector<int>>::iterator itero_parejas;
		vector<int>::iterator itero_dentro_de_la_pareja;
		string aux,auxTuplas,auxArray,auxGenero;
		

		cout<< "Parsing buildConstraintExtension..........................................."<< endl;

		cout << "Support: " << support << endl;

		
		if (support)
		{
			aux = "constraint table([";
			cout << "SUPPORT Rule........" << endl;
		} else 
		{
			aux = "constraint not table([";
			cout << "CONFLICT Rule........" << endl;
		}

		
		cout << "Tamaño de la lista: " << list.size() << endl;
		cout << "Tamaño tuplas: " << las_tuplas.size() << endl;

		tabla_actual = "table_" + to_string(indice_tabla);
		indice_tabla++;

		
		// Declaración de la tabla
		auxArray = "\narray[1.." + to_string(tuples.size()) + ", 1..2] of int: " + tabla_actual + ";\n" ;
		escribe_fichero_mzn(auxArray);


		// Creación línea Constraint
		itero = list.begin();
		aux = aux + (*itero)->id + ",";
		
		itero++;
		aux = aux + (*itero)->id + "], "+ tabla_actual + ");\n";
		
		cout << aux << endl;
		escribe_fichero_mzn(aux);

		// Creación de la tabla con las tuplas
		aux = tabla_actual + " = array2d(1.."+ to_string(tuples.size()) + ", 1..2, [\n";
		escribe_fichero_mzn(aux);

		for (itero_parejas = tuples.begin(); itero_parejas != tuples.end();++itero_parejas) 
				{
						itero_dentro_de_la_pareja = itero_parejas->begin();
						#ifdef midebug
							cout << "\tPrimer valor Tupla: " << *itero_dentro_de_la_pareja
							<< endl;

						#endif

						auxTuplas = to_string(*itero_dentro_de_la_pareja) + ",";
						escribe_fichero_mzn(auxTuplas);
						itero_dentro_de_la_pareja++;
					
						#ifdef midebug
							cout << "\tSegundo valor Tupla: " << *itero_dentro_de_la_pareja
								<< endl;
						#endif

						auxTuplas = to_string(*itero_dentro_de_la_pareja) + ",";
						escribe_fichero_mzn(auxTuplas);
					}
		aux = "]);\n";
		escribe_fichero_mzn(aux);

		if (support && (tuples.size() == 0)) 
		{
			// Declaración de la tabla
			auxArray = "\narray[1.." + to_string(rango_variable[list[0]->id]*rango_variable[list[1]->id]) +
				", 1..2] of int: " + tabla_actual + ";\n" ;
			escribe_fichero_mzn(auxArray);

			auxGenero = tabla_actual + " = array2d(1.."+ to_string(rango_variable[list[0]->id]*rango_variable[list[1]->id]) + ", 1..2, [\n";
			escribe_fichero_mzn(auxGenero);

			cout << "\tMinimo var " << list[0]->id << ": " << minimo_variable[list[0]->id] << endl;
			cout << "\tMinimo var " << list[1]->id << ": " << minimo_variable[list[1]->id] << endl;

			cout << "\tMaximo var " << list[0]->id << ": " << maximo_variable[list[0]->id] << endl;
			cout << "\tMaximo var " << list[1]->id << ": " << maximo_variable[list[1]->id] << endl;

			cout << endl << endl;
			cout << "TUPLAS GENERADAS: " << endl;

			for (int i=minimo_variable[list[0]->id]; i <= maximo_variable[list[0]->id]; i++)
				for (int j=minimo_variable[list[1]->id]; j <= maximo_variable[list[1]->id]; j++)
				{
					auxTuplas = to_string(i) + "," + to_string(j) + ",";
				#ifdef midebug
					cout << "\t" << i << " - " << j << endl;
				#endif
					escribe_fichero_mzn(auxTuplas);
				}
			cout << endl << endl;
			auxGenero = "]);\n";
			escribe_fichero_mzn(auxGenero);
		}
	}














	//Versión para restricciones Unarias y Binarias.
	void buildConstraintExtensionAs(string id, vector<XVariable *> list,
			bool support, bool hasStar) {
		
		vector<XVariable *>::iterator itero;
		string aux;


		cout<< "\nParsing buildConstraintExtension  AS ........................................."<< endl;
		cout << "Support: " << support << endl;

		
		if (support)
		{
			aux = "constraint table([";
			cout << "SUPPORT Rule........" << endl;
		} else 
		{
			aux = "constraint not table([";
			cout << "CONFLICT Rule........" << endl;
		}


		cout << "Tamaño de la lista: " << list.size() << endl;
		cout << "Tamaño tuplas: " << las_tuplas.size() << endl;
		cout << "Tabla actual: " << tabla_actual << endl;


		
		itero = list.begin();
		aux = aux + (*itero)->id + ",";
		
		itero++;
		aux = aux + (*itero)->id + "], " + tabla_actual + ");\n";
		
		cout << aux << endl;
		escribe_fichero_mzn(aux);

	}






///////////////////////////////////
//
// COMIENZO Y FIN DE RESTRICCIONES
//
///////////////////////////////////





	void beginConstraints() {
		cout << "\nComienza la declaración de las Restricciones (Constraints) ..............\n" << endl;
		
	}










	void endConstraints() {
		cout << "Fin declaración Constraints .................." << endl << endl;
	}
	
	



////////////////////////
//
// PROCESSING GROUP
//
////////////////////////

// Sin uso de momento

	void beginGroup(string id) {


		cout << "Comienzo Grupo ....... " << id << endl;


		
	}





	void endGroup() {


		cout << "Fin Grupo .......\n\n " << endl;
		
	}














////////////////////////
//
// PROCESSING INSNTACE
//
////////////////////////



	void beginInstance(InstanceType type)
	{

#ifdef midebug
		cout << "Empieza Instancia tipo: " << type << endl;
#endif

		cout << "Creo el fichero MiniZinc ................" << endl;

		crea_fichero_mzn();

		//XCSP3PrintCallbacks::beginInstance(type);
	}











	void endInstance() 
	{
				

		cout << endl;
		cout << "FIN del parsing, closing file ----------------" << endl;

		cierra_fichero_mzn();


		
		
	}










	
//////////////////////////
//// Fin de la clase MiParser
//////////////////////////
	
};










///////////////////
//
// THE ONE AND ONLY MAIN
//
///////////////////

int main(int argc, char **argv) {
	MiSolverPrintCallbacks miparser;
	char *nombre_fichero_mzn;
	int dimension = 0;
	

	

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
		exit(EXIT_CODE_ERROR_COMMAND);
	}



	//salida matriz de datos
 	/* ofstream fmat("log_mat.txt", ios::out);
	miparser.imprime_matriz("datos",fmat);
	fmat.flush(); */

	/* ostream terminal(cout.rdbuf());
	miparser.imprime_matriz("datos",terminal);
	terminal.flush(); */
		
    // Liberamos memoria
    // delete [] miparser.matriz_datos;
	
	
	exit(EXIT_CODE_SUCCESS);
}

