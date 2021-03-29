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
// #define midebug
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

	vector<string> 	lista_arrays;    	// Guarda la lista de arrays. Los arrays ya no se usan.
	bool is_array=false;				// PSS-determina si una varaible es un singleton o forma parte de un array
										// Todas las variables ahora se tratan como singleton. 
	map<string,int> mapa_indices;		// Guarda el índice de cada variable.
	map<string, vector<int>> valores_variable;	// Guarda los valores discretos de una variable.
	string primera_variable = "Si";		// Permite calcular la base de las variables en la matriz, según se leen.
	string variable_anterior="Vacia";
	map<string, int> base_array; 		// Mapa de cada array con su coordenada base.
	map<string, int> base_variable;		// Mapa de cada Variable con su coordenada base, debe sustituir a base_array.
	map<string, int> maximo_variable; 	// Guarda el máximo del rango de cada una de las variables.
	map<string, int> minimo_variable; 	// Guarda el minimo del rango de cada una de las variables.
	map<string, int> rango_array;	 	// Mapa de cada array con el rango de valores de las variables.
	map<string, int> rango_variable; 	// Mapa con el rango de valores de las variables.
	map<string, int> numero_variable;	// Mapa de cada array con el numero de instancias.
										// de variables del array.
	string array_actual = "empiezo"; 	// Sirve para identificar con que array se esta trabajando
	int base_siguiente_array = 0; 		// Guarda el valor para calcular la posicion en la matriz del siguiente array
	int minimo_variables = 0;        	// Guarda el minimo valor de cada array
	int maximo_variables = 0;        	// Guarda el minimo valor de cada array
	int rango_variables = 0; 			// Guarda el rango de valores de las variables de un array
	int numero_variables = 0;      		// Guarda el numero de variables de un array

	vector<vector<int>> las_tuplas;   	// Guarda las tuplas, puesto que en
									  	// buildConstraintExtensionAs() no me las pasan como argumento
	vector<int> tuplas_unarias;			// Lo mismo, pero para variables unarias
	vector<int> tamano_tuplas;			// Vector que almacena el tamaño de las tuplas: (número de tuplas)
	

public:

	char nombre_fichero[256]; 			// Nombre del fichero MINIZINC a procesar
	char *nombre_fichero_mzn;			// Puntero al nombre del fichero
	FILE *fichero_mzn;					// Puntero a la estructura del fichero

	vector<string> lista_variables; 			// Guarda la lista de variables.
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






#ifdef mitest
	vector<vector<int>> matriz_check; 	// Matriz donde se almacena el resultado
#endif








	void set_nombre_fichero(char *nombre) {
		strcpy(nombre_fichero, nombre);
	}




	// Escribe los resultados en un fichero
	void escribe_fichero_csp() {
		
	}





	// Genera el fichero .clq sin usar la clase UG (Undirected Graph)
	void escribe_grafo_clq()
	{
		
	}






void crea_fichero_mzn()
	{
		string var;
		
		
		// Procedo a escribir el fichero.
		nombre_fichero_mzn = strrchr(nombre_fichero, '.');
		strcpy(nombre_fichero_mzn, ".mzn");
		fichero_mzn = fopen(nombre_fichero,"w");
		cout << "Nombre fichero .MZN: " << nombre_fichero << endl;
		
		// Escribo la cabecera del fichero.

		fprintf(fichero_mzn,"%\tFichero en formato MINIZINC creado a partir de un fichero XCSP3\n\n");	
		fprintf(fichero_mzn,"include \"table.mzn\";\n\n\n");
		fprintf(fichero_mzn,"%\tDeclaración de variables: \n\n");
		
	}







	void escribe_fichero_mzn(string texto)
	{

		// fprintf(fichero_mzn,texto);

	}




	void cierra_fichero_mzn()
	{
		fclose(fichero_mzn);
	}








	
	//removes edges corresponding to values of the same variable, from ug and matriz_datos
	//(all incompatible since a variable may only have one value)
	void mi_remove_edges_same_var()
	{
		
	}

	// Genera la matriz
	void genera_matriz() {

	}


	// Certificacion de que la matriz tiene la diagonal principal a cero
	void pongo_diagonal_matriz_a_cero() {
	}

	ostream& imprime_matriz(string matriz, ostream& o=cout) {
		return o;
	}


	// REVISAR SI HAY QUE VOLVER A IMPLEMENTARLA.
	// Funcion que escribe en la matriz reglas unarias. Hay que adaptarla a 
	// no usar get_indice() o get_nombre()
	void escribe_en_matriz_unaria(vector<int>& tuplas,string var_unaria, bool support)
	{
	}


	void nueva_escribe_en_matriz(vector<vector<int> >& tuplas,string var_cero, string var_uno, bool support) 
	{

	}



	// Para el caso de reglas intensionales binarias, escribe 1 en la matriz 
	void escribe_1_en_matriz(string var_cero, string var_uno,int i, int j)
	{
	}


	// Para el caso de reglas intensionales binarias, escribe 0 en la matriz 
	void escribe_0_en_matriz(string var_cero, string var_uno,int i, int j)
	{
	}

	//Funcion que escribe en la matriz una regla AllEqual o AllDifferent
	// Hay que adaptarla para el uso de las nuevas funciones.(nueva_escribe_en_matriz())
	void  escribe_regla_all(string var_cero, string var_uno, int REGLA)
	{
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

		lista_arrays.push_back(id);
		

		array_actual = id;
		base_array[id] = base_siguiente_array;
		rango_array[id] = 0;

		numero_variables = 0;
		rango_variables = 0;


		is_array=true;

		
	}











	// Se invoca cuando se termina de procesar todas las variables de un array.
	// En este momento se actualizan las variables globales.
	// Con esa información se realiza el cálculo para poder escribir en la matriz.
	// Deprecated, ahora se calcula de otra manera.
	void endVariableArray() {

		base_siguiente_array += (numero_variables * rango_variables);
		numero_variable[array_actual] = numero_variables;
		rango_array[array_actual] = rango_variables;
		minimo_variable[array_actual] = minimo_variables;

		is_array=false;

		cout << "Fin array .........." << endl;

#ifdef midebug
/* 		cout << "Base siguiente array: " << base_siguiente_array << endl;
		cout << "Numero variables: " << numero_variables << " - Rango: "
				<< rango_array << endl; */
#endif

	}











	// Comienza el proceso de variables. De momento no se hace nada.
	void beginVariables() {

		
		cout << "Creo el fichero MiniZinc ................" << endl;

		crea_fichero_mzn();


#ifdef midebug
		cout << "COMIENZA la declaracion de variables............. " << endl;
#endif

	}














	// Se invoca al terminar de procesar las variables.
	// Escribe el fichero .csp que contiene todas las variables con sus rangos.
	// Genera la matriz, que una vez escrita, servirá para generar el grafo.
	void endVariables() {

		//Escribo el fichero .csp
		//escribe_fichero_csp();
		

		
		//cout << "Genero la matriz Binaria............." << endl;
		genera_matriz();
		//cout << "Dimensión de la matriz: " << dimension_matriz << endl;		
		//cout << "Matriz generada .............." << endl;


#ifdef midebug
		cout << " - FIN declaracion variables - " << endl << endl;
#ifdef mipause
		cin.get();
#endif
#endif
	}















	void buildVariableInteger(string id, int minValue, int maxValue) override {
		
		// cout << "Primera Variable: " << primera_variable;

		if (primera_variable == "Si")
		{
			
			base_variable[id] = 0;
			primera_variable = "No";
		}
		else
		{
			variable_anterior = lista_variables.back();
		}

		// cout << " - Variable anterior: " << variable_anterior << endl;
		
		lista_variables.push_back(id);			
		mapa_indices[id]=numero_variables;

		for (int i = minValue; i <= maxValue; i++)
		{
			valores_variable[id].push_back(i);
		}

		// Para tratar los arrays actualmente deprecated, sin uso
		rango_variables = (maxValue - minValue) + 1;
		minimo_variables = minValue;			
		numero_variables++;
		
		// Para tratar cada variable de manera individual
		rango_variable[id] = (maxValue - minValue) + 1;
		maximo_variable[id] = maxValue;
		minimo_variable[id] = minValue;


		if (primera_variable == "No")	
			base_variable[id] = base_variable[variable_anterior] + rango_variable[variable_anterior];
		
		
		// cout << "Variable: " << id << " indice: "<< (numero_variables-1) << " - min: " << minValue << " - max: "
		//		<< maxValue << " - Base variable en la matriz: " << base_variable[id] << " - Rango: " << rango_variable[id] << endl;

		}














	//called for stand-alone values independent of a range: we assume they DO belong to a range
	void buildVariableInteger(string id, vector<int> &values) override {

		vector<int>::iterator itero_values;

		if (primera_variable == "Si")
		{
			
			base_variable[id] = 0;
			primera_variable = "No";
		}
		else
		{
			variable_anterior = lista_variables.back();
		}


		lista_variables.push_back(id);
		lista_variables_discretas.push_back(id);
		rango_variable[id] = values.size();

		rango_variables = values.size();
		minimo_variables = values.front(); 		/*TODO-extend to non-index values */
		maximo_variables = values.back();
		mapa_indices[id] = numero_variables;
		numero_variables++;

		if (primera_variable == "No")	
			base_variable[id] = base_variable[variable_anterior] + rango_variable[variable_anterior];


		for (int i=0; i< values.size();i++)
		{
			valores_variable[id].push_back(values[i]);
		}

		cout << "Variable: " << id 	<< " - Base variable en la matriz: " << base_variable[id]
			 << " - Rango: " << rango_variable[id] << endl;

		cout  << " - min: " << values[0] << " - max: "
		 		<< values.back() << " - Índice: " << mapa_indices[id] << " - Rango: " << rango_variable[id] <<  endl;

		cout << "Valores: ";

		for (int i=0; i< values.size();i++)
		{
			cout << valores_variable[id][i] << " ";
		}
		cout << endl;
	}














	//Versión para Restricciones UNARIAS
	void buildConstraintExtension(string id, XVariable *variable, vector<int> &tuples, bool support, bool hasStar) {
		
		
		cout << "Regla UNARIA: "<< support << endl;
		escribe_en_matriz_unaria(tuples,variable->id,support);
	}

















	//Versión para restricciones binarias o superiores
	void buildConstraintExtension(string id, vector<XVariable *> list,
		vector<vector<int>> &tuples, bool support, bool hasStar) {

		vector<XVariable *>::iterator itero;
		
		cout<< "Parsing buildConstraintExtension..........................................."<< endl;

		// Guardo el valor de las tuplas por si es una restriccion de grupo
		las_tuplas=tuples;


	#ifdef mydebug
		cout << "Tamaño de la lista: " << list.size() << endl;
		cout << "Tamaño tuplas: " << las_tuplas.size() << endl;
		for (itero = list.begin(); itero != list.end(); itero++)
		{
			cout << (*itero)->id << endl;
		}
	#endif
	
		
		// cout << "Fin variables." << endl;		
		
		if (list.size() == 2){
			
			cout << "Regla BINARIA:" << endl;
			nueva_escribe_en_matriz(las_tuplas,list[0]->id,list[1]->id,support);

	#ifdef mydebug
			cout << "Par de variables: " << (list[0]->id) << " - " << (list[1]->id)	<< endl;
			cout <<  "Coordenada base nueva: " << base_variable[list[0]->id] << " - " << base_variable[list[1]->id] << endl;
	#endif
		}
		else {
			cout << "Tamaño de la regla: " << list.size() << endl;
			throw runtime_error("ERROR: Tamaño de Regla no procesado con esta versión del generador de grafos.");
			exit(EXIT_CODE_NOT_IMPLEMENTED);
		}

	}














	//Versión para restricciones Unarias y Binarias.
	void buildConstraintExtensionAs(string id, vector<XVariable *> list,
			bool support, bool hasStar) {
		
		vector<XVariable *>::iterator itero;

		cout<< "Parsing buildConstraintExtension  AS ........................................."<< endl;

	#ifdef mydebug
		cout << "Tamaño de la lista: " << list.size() << endl;
		cout << "Tamaño tuplas: " << las_tuplas.size() << endl;
		for (itero = list.begin(); itero != list.end(); itero++)
		{
			cout << (*itero)->id << endl;
		}
	#endif
		
		if (list.size() == 2)
		{
			cout << "Regla BINARIA:" << endl;
			nueva_escribe_en_matriz(las_tuplas,list[0]->id,list[1]->id,support);

	#ifdef mydebug
			cout << "Par de variables: " << (list[0]->id) << " - " << (list[1]->id)	<< endl;
			cout <<  "Coordenada base nueva: " << base_variable[list[0]->id] << " - " << base_variable[list[1]->id] << endl;
	#endif
		}
		else {
			cout << "Tamaño de la regla: " << list.size() << endl;
			throw runtime_error("ERROR: Tamaño no procesado con esta versión del generador de grafos.");
			exit(EXIT_CODE_NOT_IMPLEMENTED);
		}

	}






















///////////////////////////
//
// PROCESSING ALL DIFFERENT
//
///////////////////////////



	void buildConstraintAlldifferent(string id, vector<XVariable *> &list) {
    	
		int i=0,j=0,k=0;
		int REGLA;

		REGLA=DIFERENTE;		
		cout << "\n   Mi allDiff constraint " << id << "Tamaño de la regla: "<< list.size() << endl;

		/* if (list.size() != 2)
		{
			cout << "Tamaño de la regla: " << list.size() << endl;
			throw runtime_error("ERROR: Tamaño no procesado con esta versión del generador de grafos.");
			exit(EXIT_CODE_NOT_IMPLEMENTED);
		} */
		
		for (k=0;k<(list.size()-1);k++)
			for(i=k,j=i+1; j<list.size();j++)
			{
				escribe_regla_all(list[i]->id,list[j]->id,REGLA);
	
	#ifdef midebug
				cout << "Pareja: " << list[i]->id << " , " << list[j]->id << endl;
	#endif
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
    	
		int i=0,j=0,k=0;
		int REGLA;

		REGLA=IGUAL;
		cout << "\n   Mi allEqual constraint " << id << "Tamaño de la tupla: "<< list.size() << endl;
		
		
		for (k=0;k<(list.size()-1);k++)
		{
			for(i=k,j=i+1; j<list.size();j++)
			{
	#ifdef midebug
				cout << "Pareja: " << list[i]->id << " , " << list[j]->id << endl;
	#endif
				escribe_regla_all(list[i]->id,list[j]->id,REGLA);
			}
		}
	}












	////////////////////////////////////
	//
	// PROCESSING FORMULAS INTENSIONALES
	//
	////////////////////////////////////



	void buildConstraintPrimitive(string id, OrderType orden, XVariable *x, int k, XVariable *y) {
    	

		cout << "\nFórmula simple.............. \n  " << id;
		// cout << "\nVERSIÓN ESPECÍFICA .............. \n  " << id;
			
	// #ifdef midebug
			cout << "\n   OPERACIONES BINARIAS ............... Order Type: " << orden <<endl;
			cout << x->id << " base: " << base_variable[x->id] << " - Rango: " << rango_variable[x->id] << endl; 
			cout << "Entero :" << k << endl;
			cout << y->id << " base: " << base_variable[y->id] << " - Rango: " << rango_variable[x->id] << endl; 
			cout <<  " : Operación: " << orden << endl; 
	// #endif
		
		switch(orden)
		{
			case (LE):
				cout << "Less or Equal (" << orden << ")" << endl;
				for (int i = 0; i < rango_variable[x->id]; i++)
					for (int j = 0; j < rango_variable[y->id]; j++)
						if (!(valores_variable[x->id][i] + k <= valores_variable[y->id][j])) // (+ k), versión específica para familia Scheduling
							escribe_0_en_matriz(x->id,y->id,i,j);
				break;
			case (LT):
				cout << "Less Than (" << orden << ")" << endl;
				for (int i = 0; i < rango_variable[x->id]; i++)
					for (int j = 0; j < rango_variable[y->id]; j++)
						if (!(valores_variable[x->id][i] < valores_variable[y->id][j]))
							escribe_0_en_matriz(x->id,y->id,i,j);
				break;
			case (GE):
				cout << "Greater or Equal (" << orden << ")" << endl;
				for (int i = 0; i < rango_variable[x->id]; i++)
					for (int j = 0; j < rango_variable[y->id]; j++)
						if (!(valores_variable[x->id][i] >= valores_variable[y->id][j]))
							escribe_0_en_matriz(x->id,y->id,i,j); 
				break;
			case (GT):
				cout << "Greater Than (" << orden << ")" << endl;
				for (int i = 0; i < rango_variable[x->id]; i++)
					for (int j = 0; j < rango_variable[y->id]; j++)
						if (!(valores_variable[x->id][i] > valores_variable[y->id][j]))
							escribe_0_en_matriz(x->id,y->id,i,j);
				break;
			
			case (IN):
				cout << "Contenido en (" << orden << ")" << endl;
				throw runtime_error("Pendiente de implementar ........\n");
				exit(EXIT_CODE_NOT_IMPLEMENTED); 
				
			case (EQ):
				cout << "Equal (" << orden << ")" << endl;
				for (int i = 0; i < rango_variable[x->id]; i++)
					for (int j = 0; j < rango_variable[y->id]; j++)
						if (!(valores_variable[x->id][i] == valores_variable[y->id][j]))
							escribe_0_en_matriz(x->id,y->id,i,j);
				break;
			case (NE):
				cout << "Not Equal (" << orden << ")" << endl;
				for (int i = 0; i < rango_variable[x->id]; i++)
					for (int j = 0; j < rango_variable[y->id]; j++)
						if (!(valores_variable[x->id][i] != valores_variable[y->id][j]))
							escribe_0_en_matriz(x->id,y->id,i,j);
		} 
	}














  	void buildConstraintIntension(string id, Tree *tree) {
		vector<string> variable;
		vector<int> rango;
    	map<string, int> tupla;
		int resultado=0; 
		int coordenadas_final[2];
		

    	cout << "\nFórmula compleja de orden: " << tree->arity() << " ..............   \n";
    	
		for(int i=0;i<tree->arity();i++)
    	{
     		variable.push_back(tree->listOfVariables[i]);
			rango.push_back(rango_variable[tree->listOfVariables[i]]);

	#ifdef mydebug
			cout << tree->listOfVariables[i] << " ";
	#endif
    	}

		cout << endl;

		if (tree->arity() == 2)
		{
			for (int i=0;  i < rango[0]; i++)
			{
				for(int j=0; j < rango[1]; j++)
				{
					tupla[variable[0]]=valores_variable[variable[0]][i];
					tupla[variable[1]]=valores_variable[variable[1]][j];
					resultado = tree->evaluate(tupla);				
	#ifdef midebug
					cout << "valores: " << valores_variable[variable[0]][i] << " " << valores_variable[variable[1]][j] << " ";
					cout << endl;
					tree->prefixe();
					cout << "=  " << resultado << endl;
	#endif
					if (!resultado)
						escribe_0_en_matriz(variable[0],variable[1],i,j);			
				}
			}
		}
		else
		{
			throw runtime_error("FÓRMULA con más de dos variables, NO IMPLEMENTADO en esta versión del generador de grafos........");
			exit(EXIT_CODE_NOT_IMPLEMENTED); 

		}
    	
	}









//////////////////////////////////
//
// 	MÁS INTENSIONAL
//
//////////////////////////////////




	void buildConstraintPrimitive(string id, XVariable *x, bool in, int min, int max) {
		
		int coordenada_final[2];

		cout << "\n  Constraint simple," << x->id 
			<< (in ? " in " : " not in ") << min << ".." << max <<"\n";
		
		cout << "Para el fichero Sadeh, son todo unos.\n";


#ifdef midebug
			cout << "Escribo 0 en: " << base_variable[x->id] << " - " << base_variable[x->id] << endl;
#endif
		// PENDIENTE DE IMPLEMENTAR, SOLO VÁLIDA PARA FICHEROS SADEH.
		coordenada_final[0] = base_variable[x->id]+min;
		coordenada_final[1] = base_variable[x->id]+max;
		matriz_datos[coordenada_final[0]][coordenada_final[1]] = 1;
		matriz_datos[coordenada_final[1]][coordenada_final[0]] = 1; 
	}










//////////////////////////////////
//
// 	PROCESANDO REGLAS NEL
//
//////////////////////////////////




	// string id, vector<XVariable *> &list, int startIndex, XVariable *value
	void buildConstraintChannel(string, vector<XVariable *> &list, int, XVariable *value)
	{
    	cout << "\n    chachacha - channel constraint" << endl;
    	cout << "        ";
    	displayList(list);
    	cout << "        value: " << *value << endl;
	}















	// string id, vector<XVariable *> &list1, int startIndex1, vector<XVariable *> &list2, int startIndex2
	void buildConstraintChannel(string, vector<XVariable *> &list1, int, vector<XVariable *> &list2, int) {
		
		int coordenada_final[2],coordenada_base[2];
		int i = 0,j = 0, k=0;
				
		cout << "\n   Restricción de \"CANAL\": " << endl;
		cout << "        list1 ";
		displayList(list1);
		cout << "        list2 ";
		displayList(list2);
		cout << endl;

		for(int i = 0; i < list1.size(); i++)
		{
			// cout << "Variable1: " << list1[i]->id << endl;
			coordenada_base[0] = base_variable[list1[i]->id];

			for (int j = 0; j < list1.size(); j++)
			{	
				

				// cout << "Variable2: " << list2[i]->id << endl;
				// cout << "Pongo el cero de la columna " << j << endl;

				coordenada_base[1] = base_variable[list2[j]->id];

				coordenada_final[0] = coordenada_base[0]+j;
				coordenada_final[1] = coordenada_base[1]+i;

				matriz_datos[coordenada_final[0]][coordenada_final[1]] = 0;
				matriz_datos[coordenada_final[1]][coordenada_final[0]] = 0;

			}	
				
			
		}


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

		//XCSP3PrintCallbacks::beginInstance(type);
	}











	void endInstance() 
	{
				

		cout << endl;
		cout << "FIN del parsing----------------" << endl;

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
    delete [] miparser.matriz_datos;
	
	
	exit(EXIT_CODE_SUCCESS);
}

