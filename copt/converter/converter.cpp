#include "XCSP3CoreParser.h"
#include "XCSP3PrintCallbacks.h"

#include <fstream>
#include <string.h>
#include <iostream>
#include <climits>
#include <map>
#include <math.h>

//#define mipause
//#define midebug
//#define mitest
#define BUFFER_PUNTEROS 100*1024*1024
#define DIFERENTE 0
#define IGUAL 1

#ifndef BENCHMARK_PATH 
#define BENCHMARK_PATH  "/var/tmp/salida"
#endif

using namespace XCSP3Core;

class MiSolverPrintCallbacks: public XCSP3PrintCallbacks {

private:

	struct dato {						// Estructura necesaria para la generalización de la solución binaria.
			string var;
			int contador;
			int coeficiente;
			vector <int> valores;
		};

	vector <dato> datos;				// Vector de estructuras para la generación de los vértices.
		
	map<string,int> mapa_indices;		// Guarda el índice de cada variable
	map<string,string> mapa_nombres;	// Guarda el nombre de cada variable
	
	bool is_array=false;				// PSS-determina si una varaible es un singleton o forma parte de un array
										// DEPRECATED.

	map<string, vector<int>> valores_variable_discreta;	// Guarda los valores discretos de una variable.
	string primera_variable = "Si";		// Permite calcular la base de las variables en la matriz, según se leen.
	string variable_anterior="Vacia";


	string array_actual = "empiezo"; 	// Sirve para identificar con que array se esta trabajando
	int base_siguiente_array = 0; 		// Guarda el valor para calcular la posicion en la matriz del siguiente array
	int minimo_variables = 0;        	// Guarda el minimo valor de cada variable
	int rango_variables = 0; 			// Guarda el rango de valores de las variables de un array
	int numero_variables = 0;      		// Guarda el numero de variables de un array
	vector<string> 	lista_arrays;    	// Guarda la lista de arrays
	map<string, int> base_array; 		// Mapa de cada array con su coordenada base.
	map<string, int> base_variable;		// Mapa de cada Variable con su coordenada base, debe sustituir a base_array.
	map<string, int> maximo_variable; 	// Guarda el máximo del rango de cada una de las variables.
	map<string, int> minimo_variable; 	// Guarda el minimo del rango de cada una de las variables.
	map<string, int> rango_array;	 	// Mapa de cada array con el rango de valores de las variables.
	map<string, int> rango_variable; 	// Mapa con el rango de valores de las variables.
	map<string, int> numero_variable;	// Mapa de cada array con el numero de instancias.
										// de variables del array.

	

	vector<string> lista_variables; 			// Guarda la lista de variables.
	vector<string> lista_variables_discretas;	// Guarda la lista de variables con rango discreto.
	vector<int> lista_variables_ternarias;		// Guarda la lista de variables binarizadas, 
												// en cada posición se guarda el "número" de variables.
												// Sirve para generar el fichero CSP
	int indice_var_ternarias = 0;					// Índice global de variables ternarias.
	map<int,vector<string>> nueva_super_variable;	// Nueva Super-Variable para procesar las reglas ternarias.
													// Contiene las variables como strings.


	vector <int> dimension_variables_ternarias;	// Guarda el número de tuplas posibles para cada var ternaria. 
												// Hay que darle una vuelta. DEPRECATED.

	vector<vector<int>> las_tuplas;   	// Guarda las tuplas, puesto que en
									  	// buildConstraintExtensionAs() no me las pasan como argumento
	vector<int> tamano_tuplas;			// Vector que almacena el tamaño de las tuplas: (número de tuplas). DEPRECATED.
	vector<int> tamano_total_tuplas;	// Vector que almacena el tamaño total de los elementos de las tuplas: (dimensión*número de tuplas)
										// DEPRECATED.

	
	
	
	// Datos VÉRTICES:
	int **matriz_vertices;  // Matriz donde se almacenan los punteros a los valores de las tuplas de los vértices.
	map <int,vector<int>> mapa_vertices;	// Lista de vértices.
	int indice_vertices=0;	// Índice global para indexar los vértices del grafo.
	int contador_aristas=0;				// Sirve para contar las aristas y poder generar el grafo,
	map <int , vector <int>> grafo;		// Almacena el grafo que será volcado a fichero
	vector<int> lista_vertices; // Contiene una relación entre los vértices y su rango de valores posibles. DEPRECATED.

	stack <string> pila_comparacion;		// Pila para hacer la comparación de los datos.
	char nombre_fichero[256]; 			// Nombre del fichero XML a procesar



public:






///////////////////////////////////////////////////////////////////
/////
/////  FUNCIONES AUXILIARES
/////
///////////////////////////////////////////////////////////////////







	void set_nombre_fichero(char *nombre) {
		strcpy(nombre_fichero, nombre);
	}





















	void escribe_fichero_csp() 
	{
		string var;
		char *nombre_fichero_csp;
		map <int,vector<int>>::iterator iterador_vertices,iterador_fin;

		nombre_fichero_csp = strrchr(nombre_fichero, '.');
		strcpy(nombre_fichero_csp, ".csp");
		cout << "Nombre fichero .CSP: " << nombre_fichero << endl;
		//cout << "Rango variables: " << lista_vertices.size() << endl;

		ofstream fichero_salida(nombre_fichero);

		fichero_salida<< "c Fichero creado a partir de un fichero XML que expresa un problema CSP"<< endl;
		fichero_salida << "x " << lista_variables_ternarias.size() << endl;

		

		for (unsigned int j = 0; j < lista_variables_ternarias.size(); j++)
			fichero_salida << "v " << (j + 1) << " " << lista_variables_ternarias[j]
					<< endl;
		

		fichero_salida.close();
	}

	



























	void reserva_memoria_punteros()
	{
		matriz_vertices = new int *[BUFFER_PUNTEROS];
		cout << "Creado buffer punteros y buffer de punteros a vértices con " << (BUFFER_PUNTEROS) << " posiciones." << endl;
			
	}









	




	











	

	void relleno_aristas(int primera,int segunda)
	{
		// cout << "Número de vértices U[" << primera << "]: " << mapa_vertices[primera].size() << endl;
		// cout << "Número de vértices U[" << segunda << "]: " << mapa_vertices[segunda].size() << endl;

		for (int i=0;i<mapa_vertices[primera].size();i++)
		{
			for(int j=0;j<mapa_vertices[segunda].size();j++)
			{
				//cout << "Arista ......... v(" << mapa_vertices[primera][i] << ") <-> v(" << mapa_vertices[segunda][j] << ")" << endl; 
				grafo[contador_aristas]={mapa_vertices[primera][i],mapa_vertices[segunda][j]};
				contador_aristas++;
			}
		} 
	}


















	int posicion_variable(int nueva_var,string var)
	{
		int i=0;

		for (i=0;i<lista_variables_ternarias[nueva_var];i++)
			if(var==nueva_super_variable[nueva_var][i])
			{
				//cout << "Encontrada "  << var << ", posición " << i << endl;
				break;
			}
		return i;
	}


















	void comparo_vertices(int indice_nueva_variable1, vector<int>pos_var_uno, int indice_nueva_variable2,vector<int>pos_var_dos)
	{
		stack <int> pila_resultado;
		int vertice1,vertice2;
		int bucle=0;
		int hay_arista=0;
		int tamano_comparacion = pos_var_uno.size();

		
		
		for (int i=0; i < tamano_tuplas[indice_nueva_variable1]; i++)
		{
			for(int j=0; j < tamano_tuplas[indice_nueva_variable2]; j++)
			{
				// Comparación de cada uno de los vértices
				vertice1 = mapa_vertices[indice_nueva_variable1][i];
				vertice2 = mapa_vertices[indice_nueva_variable2][j];

				cout << "\nEmpiezo la comparación ........................" << endl;
				cout << "i: " << i << " - j: " << j << endl;
				cout << "Número de variables a comparar: " << tamano_comparacion << " Vertice1: " << vertice1 << " Vertice2: " << vertice2 << endl;
				
				//pila_resultado.reset();
				for (int k=0; k < tamano_comparacion ; k++)
				{
					// cout << "posición_uno: " << pos_var_uno[k] << " valor: " << matriz_vertices[vertice1][pos_var_uno[k]] 
					// 		<< " posición_dos: " <<  pos_var_dos[k] << " valor: "<< matriz_vertices[vertice2][pos_var_dos[k]] << endl;
					cout << " valor: " << matriz_vertices[vertice1][pos_var_uno[k]] 
					 		<< " valor: "<< matriz_vertices[vertice2][pos_var_dos[k]] << endl;
					
					if(matriz_vertices[vertice1][pos_var_uno[k]] == matriz_vertices[vertice2][pos_var_dos[k]])
					{
						cout << "comparación " << k << " con exito" << endl;
						pila_resultado.push(1);
					}
					else
					{
						cout << "comparación " << k << " SIN exito" << endl;
						pila_resultado.push(0);
					}					
				}

				hay_arista = pila_resultado.top();
				bucle = pila_resultado.size();

				for (int l=0; l< bucle;l++)
				{	
					if (pila_resultado.top() == 0)
						hay_arista=0;
					pila_resultado.pop();
				}

				if(hay_arista)
				{	cout << "Hay arista entre los vertices " << vertice1 << " y " << vertice2 << endl;

					//Escribo en el grafo
					grafo[contador_aristas]={vertice1,vertice2};
					contador_aristas++;
				}
				else
					{
						cout << "Sin arista entre " << vertice1 << " y " << vertice2 <<  endl;
					}
					
			}

		}
		
	}












void  comparo_vertices_conflict(int indice_nueva_variable1, vector<int>pos_var_uno, int indice_nueva_variable2,vector<int>pos_var_dos)
	{
		stack <int> pila_resultado;
		int vertice1,vertice2;
		int hay_coincidencia=0;
		int tamano_comparacion = pos_var_uno.size();

		cout << "Número de vértices variable U[" << indice_nueva_variable1 << "]: " << mapa_vertices[indice_nueva_variable1].size() << endl;
		cout << "Número de vértices variable U[" << indice_nueva_variable2 << "]: " << mapa_vertices[indice_nueva_variable2].size() << endl;
		
		cout << "Tamaño pila comparación: " << pila_resultado.size() << endl;
		
		for (int i=0; i <mapa_vertices[indice_nueva_variable1].size(); i++)
		{
			for(int j=0; j < mapa_vertices[indice_nueva_variable2].size(); j++)
			{
				// Comparación de cada uno de los vértices
				vertice1 = mapa_vertices[indice_nueva_variable1][i];
				vertice2 = mapa_vertices[indice_nueva_variable2][j];

				cout << "\nEmpiezo la comparación ........................" << endl;
				cout << "Número de variables a comparar: " << tamano_comparacion << " Vertice1: " << vertice1 << " Vertice2: " << vertice2 << endl;
				cout << "Tamaño pila comparación: " << pila_resultado.size() << endl;
				
				//pila_resultado.clear();
				for (int k=0; k < tamano_comparacion ; k++)
				{
					cout << "posición_uno: " << pos_var_uno[k] << " valor: " << matriz_vertices[vertice1][pos_var_uno[k]] 
					 		<< " posición_dos: " <<  pos_var_dos[k] << " valor: "<< matriz_vertices[vertice2][pos_var_dos[k]] << endl;
					cout << " valor: " << matriz_vertices[vertice1][pos_var_uno[k]] 
					 		<< " valor: "<< matriz_vertices[vertice2][pos_var_dos[k]] << endl;
					
					if(matriz_vertices[vertice1][pos_var_uno[k]] == matriz_vertices[vertice2][pos_var_dos[k]])
					{
						cout << "comparación " << k << " con exito" << endl;
						pila_resultado.push(1);
					}
					else
					{
						cout << "comparación " << k << " SIN exito" << endl;
						pila_resultado.push(0);
					}					
				}

				hay_coincidencia = pila_resultado.top();
				
				for (int l=0; l< pila_resultado.size();l++)
				{	
					if (pila_resultado.top() == 0)
						hay_coincidencia=0;
					pila_resultado.pop();
				}

				if(hay_coincidencia)
				{	
					grafo[contador_aristas]={vertice1,vertice2};
					contador_aristas++;
					cout << "Regla conflict: arista " << contador_aristas << " entre los vertices " << vertice1 << " y " << vertice2 << endl;	
				}
					
			}

		}		
	}














	void ejecuto_comparacion(int indice_nueva_variable1, int indice_nueva_variable2)
	{
		int tamano_pila = pila_comparacion.size();
		int tamano_comparacion = 0;
		int hay_arista=0;
		string var;
		vector<int> pos_var_uno (tamano_pila);
		vector<int> pos_var_dos (tamano_pila);

		

		for (int i=0; i< tamano_pila ;i++)
		{
			cout << "\nTamaño pila: " << pila_comparacion.size() << endl;
			var = pila_comparacion.top();
			cout << "Variable a procesar: " << var << endl;
			pos_var_uno[i] = posicion_variable(indice_nueva_variable1,var);
			pos_var_dos[i] = posicion_variable(indice_nueva_variable2,var);
			cout << "Sacamos variable de la pila: " << pila_comparacion.top();
			pila_comparacion.pop();
		}


		
		comparo_vertices(indice_nueva_variable1,pos_var_uno,indice_nueva_variable2,pos_var_dos);

	}













void ejecuto_comparacion_conflict(int indice_nueva_variable1, int indice_nueva_variable2)
	{
		int tamano_pila = pila_comparacion.size();
		int tamano_comparacion = 0;
		int hay_arista=0;
		string var;
		vector<int> pos_var_uno (tamano_pila);
		vector<int> pos_var_dos (tamano_pila);

		

		for (int i=0; i< tamano_pila ;i++)
		{
			cout << "\nTamaño pila variables a comparar: " << pila_comparacion.size() << endl;
			var = pila_comparacion.top();
			cout << "Variable a procesar: " << var << endl;
			pos_var_uno[i] = posicion_variable(indice_nueva_variable1,var);
			pos_var_dos[i] = posicion_variable(indice_nueva_variable2,var);
			cout << "Sacamos variable de la pila: " << pila_comparacion.top() << endl;
			pila_comparacion.pop();
			comparo_vertices_conflict(indice_nueva_variable1,pos_var_uno,indice_nueva_variable2,pos_var_dos);
		}

	}












	void genero_grafo_support()
	{
		int i=0,j=0,k=0,l=0;
		
		int indice=0;
		vector<string>::iterator itero_primera_variable,itero_segunda_variable;
		
		int coordenadas_base[2];
		int pos_uno=0,pos_dos=0;



		contador_aristas=0;
		cout << "\n\nGenero el grafo......................\n" << endl;
		
		
		for (k=0;k<lista_variables_ternarias.size()-1;k++)
		{
			for (i=k,j=i+1;j<lista_variables_ternarias.size();j++)
			{
				cout << "Nuevas Variables a procesar U[" << i << "] - U[" << j << "]" << endl ;
				cout << "Contador aristas: " << contador_aristas << endl;

		 		for(itero_primera_variable=nueva_super_variable[i].begin(),pos_uno=0; 
					itero_primera_variable < nueva_super_variable[i].end(); itero_primera_variable++,pos_uno++)
					{
						for(itero_segunda_variable=nueva_super_variable[j].begin(),pos_dos=0; 
							itero_segunda_variable < nueva_super_variable[j].end();itero_segunda_variable++,pos_dos++)
						{
							cout << *itero_primera_variable << "-" << *itero_segunda_variable;
							
							if(*itero_primera_variable == *itero_segunda_variable)
							{	
								pila_comparacion.push(*itero_primera_variable);
								cout << "\nVariable " << *itero_primera_variable << " a la pila." << endl;
							}
							else 
								cout << "  ";
						}
						
						cout << endl;
					}
				if(!pila_comparacion.empty())
							ejecuto_comparacion(i,j);
				else 
				{
					cout << "\n¡ATENCIÓN!-> Pongo todas las aristas entre U[" << i << "] y U[" << j << "]" << endl;
					relleno_aristas(i,j);
				}
				cout << endl;
			}
		}
	

		cout << "\n=========================================\n" << endl;

		

	}












	void genero_grafo_conflict()
	{
		int i=0,j=0,k=0,l=0;
		
		int indice=0;
		vector<string>::iterator itero_primera_variable,itero_segunda_variable;
		
		

		contador_aristas=0;
		cout << "\nGenero el grafo......................" << endl;
		
		for (k=0;k<lista_variables_ternarias.size()-1;k++)
		{
			for (i=k,j=i+1;j<lista_variables_ternarias.size();j++)
			{
				cout << "Nuevas Variables a procesar U[" << i << "] - U[" << j << "]" << endl ;
				

		 		for(itero_primera_variable=nueva_super_variable[i].begin(); 
					itero_primera_variable < nueva_super_variable[i].end(); itero_primera_variable++)
					{
						for(itero_segunda_variable=nueva_super_variable[j].begin(); 
							itero_segunda_variable < nueva_super_variable[j].end();itero_segunda_variable++)
						{
							cout << *itero_primera_variable << "-" << *itero_segunda_variable;
							
							if(*itero_primera_variable == *itero_segunda_variable)
							{	
								pila_comparacion.push(*itero_primera_variable);
								cout << "\nVariable " << *itero_primera_variable << " a la pila." << endl;
							}
							else 
							{
								cout << "  ";
							}
						}
						
						//cout << endl;
					}
				if(!pila_comparacion.empty())
							ejecuto_comparacion_conflict(i,j);
				else 
				{
					cout << "\n¡ATENCIÓN!-> Pongo todas las aristas entre U[" << i << "] y U[" << j << "]" << endl;
					relleno_aristas(i,j);
				}
				cout << "Contador aristas: " << contador_aristas << endl;
				//cout << endl;
			}
		}

	}










	void genero_grafo_sum()
	{

		int i=0,j=0,k=0,l=0;
		
		contador_aristas=0;
		cout << "\nGenero el grafo......................" << endl;
		

		for (k=0;k<lista_variables_ternarias.size()-1;k++)
		{
			for (i=k,j=i+1;j<lista_variables_ternarias.size();j++)
			{
				//cout << "Pongo todas las aristas entre U[" << i << "] y U[" << j << "]" << endl;
				relleno_aristas(i,j);
				
				//cout << "Contador aristas: " << contador_aristas << endl;
				
			}
		}



	}





















	void escribe_grafo()
	{
		string var;
		char *nombre_fichero_csp;

		nombre_fichero_csp = strrchr(nombre_fichero, '.');
		strcpy(nombre_fichero_csp, ".clq");
		cout << "Nombre fichero .CLQ: " << nombre_fichero << endl;
		
		ofstream fichero_salida(nombre_fichero);

		
		fichero_salida << "c Fichero creado a partir de un fichero XML que expresa un problema CSP"<< endl;
		fichero_salida << "c " << nombre_fichero << endl;
		fichero_salida << "p	edge\t" << indice_vertices << "\t" << contador_aristas << endl;

		for (unsigned int j = 0; j < grafo.size(); j++)
			fichero_salida << "e " << grafo[j][0]+1 << " " << grafo[j][1]+1 << endl;
					
		fichero_salida << endl;
		fichero_salida.close();
	}









	






	















////////////////////////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////
///		Fin de mis funciones
//
//	 	Comienzo de las funciones que invoca el parser 
////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////




//////////////////////
//
//  COMIENZO Y FIN DE LA INSTANCIA
//
//////////////////////




	void beginInstance(InstanceType type) 
	{
		cout << "Empieza Instancia................ " << type << endl;
	}




	void endInstance() {
		
		cout << "FIN del parsing ............." << endl;

		// Generación del grafo:
		
		// genero_grafo_support();
		// genero_grafo_conflict();
		genero_grafo_sum();


		cout << "Escribiendo los ficheros ........" << endl;
		cout << "Número de vérices: " << indice_vertices << " Número aristas: "
			<< contador_aristas << endl;	
		escribe_grafo();
		escribe_fichero_csp();

		// Liberando la memoria.	
		delete [] matriz_vertices;		
	}










//////////////////////
//
//  COMIENZO Y FIN DE LAS RESTRICCIONES
//
//////////////////////







	void beginConstraints()
	{
    	cout << "\nDeclaración de las Restricciones................" << endl << endl;
	}








	void endConstraints()
	{
    	cout << "\nFin de la declaración de las Restricciones..............." << endl << endl;
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


		cout << "Fin Grupo .......\n" << endl;
		
	}










	// Se invoca cuando se comienza a procesar un array.
	// Se resetean los contadores para poder llevar registro del
	// tamaño del array y del rango de las variables.
	void beginVariableArray(string id) {

#ifdef midebug
		cout << "Empiezo con el Array, reseteo los valores para el array:  " << id << endl;
#endif

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
		//minimo_variable[array_actual] = minimo_variables;

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
















	
	void endVariables() {

		// Reservo memoria para guardar los punteros que referenciarán los vértices
		// de las nuevas Super Variables binarizadas.
		reserva_memoria_punteros();

	}












	//PSS calls here alsp for variables with singleton values (<var id="x0"> -1 <\var> )
	void buildVariableInteger(string id, int minValue, int maxValue) override {

		lista_variables.push_back(id);
		mapa_indices[id] = numero_variables;
		mapa_nombres[id] = id;
		rango_variables = (maxValue - minValue) + 1;
		cout << "ID: " << id << " - " ;
		rango_variable[id] = rango_variables;
		minimo_variables = minValue;
		minimo_variable[id] = minValue;
		maximo_variable[id] = maxValue;					
		numero_variables++;
		cout << "Variable: " << id << " indice var: "<< (numero_variables-1) << " - min: " << minValue << " - max: "
				<< maxValue << endl;

	}










	//called for stand-alone values independent of a range: we assume they DO belong to a range
	void buildVariableInteger(string id, vector<int> &values) override {

		vector<int>::iterator itero_values;

		lista_variables.push_back(id);
		lista_variables_discretas.push_back(id);
		rango_variable[id] = values.size();

		rango_variables = values.size();
		minimo_variable[id] = values.front(); 		
		maximo_variable[id] = values.back();
		mapa_indices[id] = numero_variables;
		numero_variables++;



		for (int i=0; i< values.size();i++)
		{
			valores_variable_discreta[id].push_back(values[i]);
		}


		cout << "Variable: " << id << " - min: " << values[0] << " - max: "
		 		<< values.back() << " - Índice: " << mapa_indices[id] << " - Rango: " << rango_variable[id] <<  endl;

		cout << "Valores: ";

		for (int i=0; i< values.size();i++)
		{
			cout << valores_variable_discreta[id][i] << " ";
		}
		cout << endl;
	}

















	//Versión para Restricciones UNARIAS
	void buildConstraintExtension(string id, XVariable *variable, vector<int> &tuples, bool support, bool hasStar)
	{
		
		cout << "Regla UNARIA:" << endl;
		cout << "Pendiente de escribir la versión binarizada.\n";
		throw runtime_error("\nFuncionalidad no implementada en esta versión.\n");
		exit(2);
	}


















	//Versión para restricciones binarias o superiores
	void buildConstraintExtension(string id, vector<XVariable *> list,
			vector<vector<int>> &tuples, bool support, bool hasStar)
	{
		int i=0,j=0;
		int tamano_lista = 0,tamano_valores = 0;
		int numero_vertices_nueva_variable=0;
		int hay_vertice = 1;
		
		vector <int> temporal;			
		dato aux;

		vector <dato>::iterator itero_datos;
		vector<XVariable *>::iterator itero_variables;
		vector<string>::iterator itero_dentro_variables;
		vector<vector<int>>::iterator itero_tuplas;
		vector <int>::iterator itero_dentro_tuplas;

		cout<< "Parsing buildConstraintExtension..........................................."<< endl;

		// Guardo el valor de las tuplas por si es una restriccion de grupo
		las_tuplas=tuples;
		
		if(list.size()==0)
		{
			throw runtime_error("Tamaño cero de tupla, hay algún error, no procesado.");
			exit(2);
		}



		if (list.size() == 1)
		{
			cout << "Regla UNARIA:" << endl;
			throw runtime_error("El Parser debería haber llamado a otra función ........");
			exit(2);
		} 




		// Creo la nueva supervariable correspondiente a la regla.
		cout << "\nNueva supervariable .......... U[" << indice_var_ternarias << "]"  << endl;
		for (itero_variables = list.begin();itero_variables != list.end();itero_variables++)
		{
				cout << (*itero_variables)->id << " - " ;
				nueva_super_variable[indice_var_ternarias].push_back((*itero_variables)->id);
		}

		lista_variables_ternarias.push_back(list.size());	// Guardo las variables ternarias. (Número de variables binarizadas)
		dimension_variables_ternarias.push_back(las_tuplas.size()); // Guardo las tuplas de cada variable.
		tamano_lista = list.size();							// Tamaño tuplas.
		tamano_valores = rango_variable[list[0]->id];  		// Tamaño datos.

		cout << "Número Variables: " << tamano_lista << " - Rango de valores: " << tamano_valores << endl;

		// Reseteo el vector datos para generar la nueva tanda de vértices.
		datos.clear();

		// Relleno la estructura de datos que luego voy a recorrer.
		for(int i = 0; i < list.size(); i++)
		{
			aux.var = list[i]->id;
			aux.contador = 0;
			aux.valores.clear();
			for (int j = minimo_variable[list[i]->id]; j <= maximo_variable[list[i]->id]; j++)
				aux.valores.push_back(j);

			datos.push_back(aux);
		}
		
		// Datos base creados
		// Ahora se generan los vértices
		
		if (!support)
		{
			
			cout << "Regla conflict...................\n";
			cout << "Genero los vértices.......... \n";

			while (datos.back().contador < tamano_valores)
			{
			// Genero los vértices.
			// y Comparo los vértices descarto los conflicto.

			temporal.clear();

			for(itero_datos = datos.begin(); itero_datos != datos.end(); itero_datos++)
			{
				temporal.push_back(itero_datos->valores[itero_datos->contador]);
			}
			
			// En el caso de conflict, si el vértice es "conflicto", no se genera.
			hay_vertice = 1;
			for (itero_tuplas = las_tuplas.begin();itero_tuplas != las_tuplas.end();++itero_tuplas)
			{
				if(temporal == *itero_tuplas)
				{
				hay_vertice = 0;
				}
			}
			if (hay_vertice)
			{
				matriz_vertices[indice_vertices]=new int[list.size()];

				cout << "(";
				for(int k=0; k < list.size(); k++)
				{
					matriz_vertices[indice_vertices][k] = temporal[k];
					cout << matriz_vertices[indice_vertices][k];
					if (k < list.size()-1)
						cout << ",";
				}
				cout <<") Vertice: " << indice_vertices << endl;

				mapa_vertices[indice_var_ternarias].push_back(indice_vertices);
				indice_vertices++;
			}

			// Gestiono los contadores para ir generando las tuplas.
			
			datos.begin()->contador++;

			for (int i=0; i < tamano_lista; i++)
			{
				if (datos.back().contador == tamano_valores)
					break;

				if(datos[i].contador == tamano_valores)
				{
					datos[i].contador = 0; 
					datos[i+1].contador++;
				}
			}
			} 
			indice_var_ternarias++;
		}
	}











	//Versión para restricciones Unarias y Binarias.
	void buildConstraintExtensionAs(string id, vector<XVariable *> list,
			bool support, bool hasStar) 
	{
		int i=0,j=0,k=0;
		int rango=0;
		string var;
		int dimension=0;
		int tamano_lista = 0,tamano_valores = 0;
		int numero_vertices_nueva_variable=0;
		int hay_vertice = 1;
		
		int *puntero_ternario; 	// Puntero para recorrer la matriz ternaria.
		int *puntero_vertice;	// Puntero auxiliar para recorrer la tupla de cada vértice.
								// Se inicializa un nuevo puntero de la matriz y se asigna
								// a este puntero auxiliar.
		
		vector <int> temporal;			
		dato aux;

		vector <dato>::iterator itero_datos;
		vector<XVariable *>::iterator itero_variables;
		vector<string>::iterator itero_dentro_variables;
		vector<vector<int>>::iterator itero_tuplas;
		vector <int>::iterator itero_dentro_tuplas;



		cout<< "Parsing buildConstraintExtension  AS ........................................."<< endl;
		
		if(list.size()==0)
		{
			throw runtime_error("Tamaño cero de tupla, hay algún error, no procesado.");
			exit(2);
		}


		if (list.size() == 1)
		{
			cout << "Regla UNARIA:" << endl;
			throw runtime_error("El Parser debería haber llamado a otra función ........");
			exit(2);
		} 


		// Creo la nueva supervariable correspondiente a la regla.
		cout << "\nNueva supervariable .......... U[" << indice_var_ternarias << "]"  << endl;
		for (itero_variables = list.begin();itero_variables != list.end();itero_variables++)
		{
				//cout << (*itero_variables)->id << " - " ;
				nueva_super_variable[indice_var_ternarias].push_back((*itero_variables)->id);
		}

		lista_variables_ternarias.push_back(list.size());	// Guardo las variables ternarias. (Número de variables binarizadas)
		dimension_variables_ternarias.push_back(las_tuplas.size()); // Guardo las tuplas de cada variable.
		tamano_lista = list.size();							// Tamaño tuplas.
		tamano_valores = rango_variable[list[0]->id]; 	 	// Tamaño datos.

		cout << "Número Variables: " << tamano_lista << " - Rango de valores: " << tamano_valores << endl;

		// Reseteo el vector datos para generar la nueva tanda de vértices.
		datos.clear();

		for(int i = 0; i < list.size(); i++)
		{
			aux.var = list[i]->id;
			aux.contador = 0;
			aux.valores.clear();
			for (int j = minimo_variable[list[i]->id]; j <= maximo_variable[list[i]->id]; j++)
				aux.valores.push_back(j);

			datos.push_back(aux);
		}

		if (!support)
		{
			
			cout << "Regla conflict...................\n";
			cout << "Genero los vértices.......... \n";

			while (datos.back().contador < tamano_valores)
			{
			// Genero los vértices.
			// y Comparo los vértices descarto los conflicto.

			temporal.clear();

			for(itero_datos = datos.begin(); itero_datos != datos.end(); itero_datos++)
			{
				temporal.push_back(itero_datos->valores[itero_datos->contador]);
			}

			// En el caso de conflict, si el vértice es "conflicto", no se genera.
			hay_vertice = 1;
			for (itero_tuplas = las_tuplas.begin();itero_tuplas != las_tuplas.end();++itero_tuplas)
			{
				if(temporal == *itero_tuplas)
				{
				hay_vertice = 0;
				}
			}

			if (hay_vertice)
			{
				matriz_vertices[indice_vertices]=new int[list.size()];

				cout << "(";
				for(int k=0; k < list.size(); k++)
				{
					matriz_vertices[indice_vertices][k] = temporal[k];
					cout << matriz_vertices[indice_vertices][k];
					if (k < list.size()-1)
						cout << ",";
				}
				cout <<") Vertice: " << indice_vertices << endl;
				
				mapa_vertices[indice_var_ternarias].push_back(indice_vertices);
				indice_vertices++;
			}

			// Gestiono los contadores para ir generando las tuplas.
			
			datos.begin()->contador++;

			for (int i=0; i < tamano_lista; i++)
			{
				if (datos.back().contador == tamano_valores)
					break;

				if(datos[i].contador == tamano_valores)
				{
					datos[i].contador = 0; 
					datos[i+1].contador++;
				}
			}
			} 
			indice_var_ternarias++;
		}

				
		if (support)
		{
			//displayList(list);
			for (itero_variables = list.begin();itero_variables < list.end();itero_variables++)
			{
				//cout << (*itero_variables)->id << " - " ;
				nueva_super_variable[indice_var_ternarias].push_back((*itero_variables)->id);
			}


			cout << "U[" << indice_var_ternarias << "]: \n";
			for(itero_dentro_variables=nueva_super_variable[indice_var_ternarias].begin();
					itero_dentro_variables<nueva_super_variable[indice_var_ternarias].end();itero_dentro_variables++)
			{
				cout << "\t" << *itero_dentro_variables << " ";
				var = *itero_dentro_variables;
				rango = rango_variable[var];
				cout << "Dominio valores variable: " << rango_variable[var] << endl;
			}

			cout << endl;
				
			cout << "Tamaño tuplas y nuevo número de vértices: " << las_tuplas.size() << endl;

			lista_variables_ternarias.push_back(list.size());
			dimension = pow(rango_variable[var],list.size());
			cout << "Rango variable: " << rango_variable[var] << " - Dimensión: " << dimension << endl;
			dimension_variables_ternarias.push_back(las_tuplas.size());
			tamano_tuplas.push_back(las_tuplas.size());
			tamano_total_tuplas.push_back(las_tuplas.size()*list.size());
				
			j=0;
			for (itero_tuplas = las_tuplas.begin();itero_tuplas != las_tuplas.end();++itero_tuplas)
			{
				itero_dentro_tuplas = itero_tuplas->begin();
				matriz_vertices[indice_vertices]=new int[list.size()];
				
				lista_vertices.push_back(rango);
				puntero_vertice = matriz_vertices[indice_vertices];

				mapa_vertices[indice_var_ternarias].push_back(indice_vertices);
					
				cout << "\tv(" << indice_vertices << "): " ;
				cout << "(";
					
				for(int i=0; i<itero_tuplas->size(); i++)
				{
					cout << *itero_dentro_tuplas;
					if (i<list.size()-1)
						cout << ",";
					*puntero_vertice=*itero_dentro_tuplas;
					puntero_vertice++;
					itero_dentro_tuplas++;
				}
				cout <<")" << endl;
				indice_vertices++;
				j++;
			}
		}
			
	}














////////////////////
//
// PROCESSING ALL DIFFERENT
//
///////////////////



	// Para restricciones AllDifferent. En esta rama de código todavía NO IMPLEMENTADA.
	void buildConstraintAlldifferent(string id, vector<XVariable *> &list)
	{
    	
		int REGLA;

		REGLA=DIFERENTE;		
		
		cout << "\n   Mi AllDifferent constraint " << id << "Tamaño de la tupla: "<< list.size() << endl;
		cout << "Pendiente de escribir la versión binarizada.\n";

		
	}










	void buildConstraintAlldifferentMatrix(string id, vector<vector<XVariable *>> &matrix) {
  		cout << "\n  ¡Mi!  allDiff matrix constraint" << id << endl;
   		for(unsigned int i = 0 ; i < matrix.size() ; i++) {
        	cout << "    i:    " << i << "  ";
        	displayList(matrix[i]);
    	}
		cout << "Pendiente de escribir la versión binarizada.\n";
	}











	void buildConstraintAlldifferentList(string id, vector<vector<XVariable *>> &lists) {
    	cout << "\n  ¡Mi!  allDiff list constraint" << id << endl;
    	for(unsigned int i = 0 ; i < (lists.size() < 4 ? lists.size() : 3) ; i++) {
        	cout << "        ";
        	displayList(lists[i]);
    	}
		cout << "Pendiente de escribir la versión binarizada.\n";
	}









////////////////////
//
// PROCESSING ALL EQUAL
//
///////////////////


	void buildConstraintAllEqual(string id, vector<XVariable *> &list) {

		int REGLA;

		REGLA=IGUAL;	
		
		cout << "\n   Mi allEqual constraint " << id << "Tamaño de la tupla: "<< list.size() << endl;
		cout << "Pendiente de escribir la versión binarizada.\n";
		
	}











////////////////////
//
// PROCESSING SUMAS
//
///////////////////





	// Para restricciones con suma sin coeficientes.
	void buildConstraintSum(string, vector<XVariable *> &list, XCondition &cond)
	{
    
		vector <dato>::iterator itero_datos;
		vector<XVariable *>::iterator itero_variables;
		
		int suma=0;
		vector <int> temporal;			
		dato aux;

		int tamano_lista = 0,tamano_valores = 0;
		int numero_vertices_nueva_variable=0;
		int hay_vertice = 1;

		// Temporal
		int operacion=0;

		
		cout << "SUMA ....................\n";
		
		cout << "Nueva supervariable .......... U[" << indice_var_ternarias << "]"  << endl;
		for (itero_variables = list.begin();itero_variables != list.end();itero_variables++)
			nueva_super_variable[indice_var_ternarias].push_back((*itero_variables)->id);
		

		lista_variables_ternarias.push_back(list.size());	// Guardo las variables ternarias. (Número de variables binarizadas)
		tamano_lista = list.size();							// Tamaño tuplas.
		tamano_valores = rango_variable[list[0]->id];  		// Tamaño datos.

		// cout << "Número Variables: " << tamano_lista << " - Rango de valores: " << tamano_valores << endl;

		// Reseteo el vector datos que almacena la lista de variables con sus valores
		// para generar la nueva tanda de vértices.
		datos.clear();

		// Relleno la estructura de datos que luego voy a recorrer.
		for(int i = 0; i < list.size(); i++)
		{
			aux.var = list[i]->id;
			aux.contador = 0;
			aux.valores.clear();
			for (int j = minimo_variable[list[i]->id]; j <= maximo_variable[list[i]->id]; j++)
			{
				aux.valores.push_back(j);
				// cout << "Valor variable: " << list[i]->id << " " << j << " - ";
			}
			// cout << endl;

			datos.push_back(aux);
		}

		// Bucle para generar los vértices.
		while (datos.back().contador < tamano_valores)
		{

		// Genero los vértices.
		// y solo guardo los que cumplen la condición.

			temporal.clear();
			suma = 0;
			hay_vertice = 0;

			for(itero_datos = datos.begin(); itero_datos != datos.end(); itero_datos++)
			{
				suma += itero_datos->valores[itero_datos->contador];
				temporal.push_back(itero_datos->valores[itero_datos->contador]);
			}

			switch(cond.op)
			{
				case (LE):
					if (suma <= cond.val)
					{
						hay_vertice = 1;
						cout << "Suma " << suma << " menor o igual que la condición " << cond.val << "." << endl;
					}
					break;

				case (LT):
					if (suma < cond.val)
					{
						hay_vertice = 1;
						cout << "Suma " << suma << " menor que la condición " << cond.val << "." << endl;
					}
					break;
					
				case (GE):
					if (suma >= cond.val)
					{
						hay_vertice = 1;
						// cout << "Suma " << suma << " mayor o igual que la condición " << cond.val << "." << endl;
					}
					break;
				
				case (GT):
					if (suma > cond.val)
					{
						hay_vertice = 1;
						cout << "Suma " << suma << " mayor que la condición " << cond.val << "." << endl;
					}
					break;

				case (IN):
					cout << "Contenido en (" << operacion << ")" << endl;
					throw runtime_error("En Constraint SUM, operación \"IN\" no implementada todavía.\n");
					break;

				case (EQ):
					if (suma == cond.val)
					{
						hay_vertice = 1;
						cout << "Suma " << suma << " igual a la condición " << cond.val << "." << endl;
					}
					break;

				case (NE):
					if (suma == cond.val)
					{
						hay_vertice = 1;
						cout << "Suma " << suma << " no es igual a la condición " << cond.val << "." << endl;
					}
			}

			// En el caso de que se cumpla la condición, se crea el vértice correspondiente.
			if (hay_vertice)
			{
				matriz_vertices[indice_vertices]=new int[list.size()];

				// cout << "(";
				for(int k=0; k < list.size(); k++)
				{
					matriz_vertices[indice_vertices][k] = temporal[k];
					// cout << matriz_vertices[indice_vertices][k];
					// if (k < list.size()-1)
						// cout << ",";
				}
				// cout <<") Vertice: " << indice_vertices << endl;

				mapa_vertices[indice_var_ternarias].push_back(indice_vertices);
				indice_vertices++;	
			}		

			// Gestiono los contadores para ir generando las tuplas.
			datos.begin()->contador++;

			for (int i=0; i < tamano_lista; i++)
			{
				if (datos.back().contador == tamano_valores)
					break;

				if(datos[i].contador == tamano_valores)
				{
					datos[i].contador = 0; 
					datos[i+1].contador++;
				}
			}
		} 	

	indice_var_ternarias++;
	// cout << endl;	
	}














	// Para restricciones con suma con coeficientes.
	void buildConstraintSum(string, vector<XVariable *> &list, vector<int> &coeffs, XCondition &cond) {
    
		vector <dato>::iterator itero_datos;
		vector<XVariable *>::iterator itero_variables;
	
		int suma=0;
		vector <int> valores,coeficientes,temporal;			
		dato aux;

		int tamano_lista = 0,tamano_valores = 0;
		int numero_vertices_nueva_variable=0;
		int hay_vertice = 1;

		// Temporal
		int operacion=0;
	
	
		cout << "SUMA con PESOS..................\n";
		

		cout << "Nueva supervariable .......... U[" << indice_var_ternarias << "]"  << endl;
		for (itero_variables = list.begin();itero_variables != list.end();itero_variables++)
			nueva_super_variable[indice_var_ternarias].push_back((*itero_variables)->id);
		

		lista_variables_ternarias.push_back(list.size());	// Guardo las variables ternarias. (Número de variables binarizadas)
		tamano_lista = list.size();							// Tamaño tuplas.
		tamano_valores = rango_variable[list[0]->id];  		// Tamaño datos.

		// cout << "Número Variables: " << tamano_lista << " - Rango de valores: " << tamano_valores << endl;

		// Reseteo el vector datos que almacena la lista de variables con sus valores
		// para generar la nueva tanda de vértices.
		datos.clear();

		// Relleno la estructura de datos que luego voy a recorrer.
		for(int i = 0; i < list.size(); i++)
		{
			aux.var = list[i]->id;
			aux.contador = 0;
			aux.valores.clear();

			// Pensado exclusivamente para rangos de valores únicos y continuos
			for (int j = minimo_variable[list[i]->id]; j <= maximo_variable[list[i]->id]; j++)
			{
				aux.valores.push_back(j);
			//	cout << "Valor variable: " << list[i]->id << " " << j << " - ";
			}
			// cout << endl;

			aux.coeficiente = coeffs[i];
			//cout << "Coeficiente: " << coeffs[i] << endl;
			
			datos.push_back(aux);
		}
		// cout << endl;


		// Bucle para generar los vértices.
		while (datos.back().contador < tamano_valores)
		{

			// Genero los vértices.
			// y solo guardo los que cumplen la condición.

			temporal.clear();
			suma = 0;
			hay_vertice = 0;

			for(itero_datos = datos.begin(); itero_datos != datos.end(); itero_datos++)
			{
				//cout << "Coeficiente: " << itero_datos->coeficiente << " - Valor: " << itero_datos->valores[itero_datos->contador] << endl;
 				suma += itero_datos->coeficiente*itero_datos->valores[itero_datos->contador];
				temporal.push_back(itero_datos->valores[itero_datos->contador]);
			}

			//cout << "Resultado suma: " << suma << endl;

			switch(cond.op)
			{
				case (LE):
					if (suma <= cond.val)
					{
						hay_vertice = 1;
						cout << "Suma " << suma << " menor o igual que la condición " << cond.val << "." << endl;
					}
					break;

				case (LT):
					if (suma < cond.val)
					{
						hay_vertice = 1;
						cout << "Suma " << suma << " menor que la condición " << cond.val << "." << endl;
					}
					break;
					
				case (GE):
					if (suma >= cond.val)
					{
						hay_vertice = 1;
						// cout << "Suma " << suma << " mayor o igual que la condición " << cond.val << "." << endl;
					}
					break;
				
				case (GT):
					if (suma > cond.val)
					{
						hay_vertice = 1;
						cout << "Suma " << suma << " mayor que la condición " << cond.val << "." << endl;
					}
					break;

				case (IN):
					cout << "Contenido en (" << operacion << ")" << endl;
					throw runtime_error("En Constraint SUM, operación \"IN\" no implementada todavía.\n");
					break;

				case (EQ):
					if (suma == cond.val)
					{
						hay_vertice = 1;
						cout << "Suma " << suma << " igual a la condición " << cond.val << "." << endl;
					}
					break;

				case (NE):
					if (suma == cond.val)
					{
						hay_vertice = 1;
						cout << "Suma " << suma << " no es igual a la condición " << cond.val << "." << endl;
					}
			}

			// En el caso de que se cumpla la condición, se crea el vértice correspondiente.
			if (hay_vertice)
			{
				matriz_vertices[indice_vertices]=new int[list.size()];

				// cout << "(";
				for(int k=0; k < list.size(); k++)
				{
					matriz_vertices[indice_vertices][k] = temporal[k];
					// cout << matriz_vertices[indice_vertices][k];
					// if (k < list.size()-1)
					//	cout << ",";
				}
				//cout <<") Vertice: " << indice_vertices << endl;

				mapa_vertices[indice_var_ternarias].push_back(indice_vertices);
				indice_vertices++;	
			}		

			// Gestiono los contadores para ir generando las tuplas.
			datos.begin()->contador++;

			for (int i=0; i < tamano_lista; i++)
			{
				if (datos.back().contador == tamano_valores)
					break;

				if(datos[i].contador == tamano_valores)
				{
					datos[i].contador = 0; 
					datos[i+1].contador++;
				}
			}
		} 	

	indice_var_ternarias++;
	}



















	void buildConstraintIntension(string id, string expr) {
    cout << "\n    SOY intension constraint : " << id << " : " << expr << endl;
	}


	












	void buildConstraintPrimitive(string id, OrderType, XVariable *x, int k, XVariable *y) {
    cout << "\n   VIVESOY intension constraint " << id << ": " << x->id << (k >= 0 ? "+" : "") << k << " op " << y->id << endl;
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
    	cout << "\n    2) TOPE channel constraint" << endl;
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

	return 0;
}

