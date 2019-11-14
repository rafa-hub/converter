// #include "../gtest/gtest.h"
#include "graph/graph.h"
#include "graph/algorithms/decode.h"
#include "utils/common.h"
#include "utils/logger.h"
#include "utils/prec_timer.h"

#include "XCSP3CoreParser.h"
#include "XCSP3PrintCallbacks.h"
#include "XCSP3Tree.h"
#include "XCSP3TreeNode.h"

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

#define BUFFER_PUNTEROS 10*1024
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
	vector<string>	lista_variables_singleton;	// Guarda la lista de variables singleton. 
	bool is_array=false;				// PSS-determina si una varaible es un singleton o forma parte de un array

						// Todas las variables ahora se tratan como singleton. 
						// Van a quedar deprecated todas las variables antiriores.



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

	char nombre_fichero[256]; 			// Nombre del fichero XML a procesar

	vector<string> lista_variables; 			// Guarda la lista de variables.
	vector<string> lista_variables_discretas;	// Guarda la lista de variables con rango discreto.
	vector<int> lista_variables_ternarias;		// Guarda la lista de variables binarizadas, 
												// en cada posición se guarda el "número" de variables.
												// Sirve para generar el fichero CSP
	int indice_var_ternarias_con_ceros = 0;		// Va indexando las variables ternarias. (a substituir por algo más consistente)
	vector <int> dimension_variables_ternarias;	// Guarda el número de tuplas posibles para cada var ternaria.
	int **matriz_ternaria;

	int dimension_matriz = 0; 			//Guarda la dimension definitiva de la matriz creada.
	int dimension_ternaria = 0;			// Guarda la dimensión de la matriz de vértices.
	
	int **matriz_datos; 	// Matriz donde se almacena el resultado.
	//int **matriz_shadow; 	// Matriz donde se almacenan las escrituras. (Deprecated)
	
	


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
		string var;
		char *nombre_fichero_csp;
		time_t tiempo = time(NULL);

		nombre_fichero_csp = strrchr(nombre_fichero, '.');
		strcpy(nombre_fichero_csp, ".csp");
		cout << "Nombre fichero CSP: \"" << nombre_fichero << "\"" << endl;
		ofstream fichero_salida(nombre_fichero);

#ifdef midebug
		cout<< "c Fichero creado a partir de un fichero XML que expresa un problema CSP"<< endl;
		cout << "x " << lista_variables.size() << endl;
#endif
		
		fichero_salida<< "c Fichero creado a partir de un fichero XML \n" 
			<< "c que expresa un problema CSP:  " << PrecisionTimer::local_timestamp()  << endl;
		fichero_salida << "x " << lista_variables.size() << endl;

		for (unsigned int j = 0; j < lista_variables.size(); j++)
		{
			fichero_salida << "v " << (j + 1) << " " << rango_variable[lista_variables[j]]	<< endl;

#ifdef midebug
			cout << lista_variables[j] << endl;
			cout << "v " << (j + 1) << " " << rango_variable[lista_variables[j]] << endl;
#endif
		}

		fichero_salida.close();
	}















	// Función deprecated, ahora se tratan todas las variables como variables individuales
	int es_singleton(string variable)
	{
		if(lista_variables_singleton.size()>0)
		{
			for (int i=0;i<lista_variables_singleton.size();i++)
			{
				if (lista_variables_singleton[i]==variable)
					return 1;
			}
		}
		return 0;
	}














	// Extrae y devuelve el indice de una variable. Deprecated, a dejar de usar.
	// Se han dejado de usar los arrays.
	int get_indice(XVariable variable) {
		string valor;
		int indice;

		valor = variable.id;
		if (es_singleton(valor))
		{
			return 1;
		}
		indice = mapa_indices[valor];

#ifdef midebug
		cout << "En get_indice(), id variable: " << valor << " es: " << indice<< endl;
#endif

		return(indice);
// 		return(mapa_indices[variable.id]);  //Toda la función
	}















	// Extrae y devuelve el indice de una variable. Deprecated, a dejar de usar.
	// Se han dejado de usar los arrays.
	int get_indice_ternario(string variable) {

		int pos_uno,pos_dos;
		string indice;
		
		if (variable.find(']') != std::string::npos)
		{
			pos_uno = variable.find('[');
			pos_dos = variable.find(']');

			//cout << "pos uno: " << pos_uno << " - pos dos: " << pos_dos << endl;
			indice = variable.substr(pos_uno+1,(pos_dos-pos_uno)-1);

			return stoi(indice);
		}
		else{
			// Es singleton:
			return 1;
			//cout << "Variable porculera: " << variable << endl;
			//throw runtime_error("Variable singleton, todavía no implementado.");
		}
	}















	








	




	//removes edges corresponding to values of the same variable, from ug and matriz_datos
	//(all incompatible since a variable may only have one value)
	int remove_edges_same_var(ugraph& ug) {
//		com::stl::print_collection(miparser.lista_arrays, cout); cout<<endl;
		cout<<"REMOVING EDGES FROM VALUES OF SAME VARIABLE:-----------------"<<endl;
		for (vector<string>::iterator it = lista_variables.begin();
				it != lista_variables.end(); it++) {
			
			int row = base_variable[*it];

			const int NUM_VAL = rango_variable[*it];
			const int MAX_ROWS_ARRAY_VAR = row + NUM_VAL;
#ifdef midebug
			// cout << array_var_name << " row:" << row << " range:" << NUM_VAL
			// 		<< " nb_var:" << numero_variable[array_var_name]
			// 		<< endl;
#endif

			while (true) {
				for (int i = row; i < (row + NUM_VAL - 1); i++) {
					for (int j = i + 1; j < (row + NUM_VAL); j++) {
						ug.remove_edge(i, j);
						matriz_datos[i][j] = 0;
						matriz_datos[j][i] = 0;
#ifdef midebug
/* 						cout<<"edge:"<<"("<<i<<","<<j<<")";
						cout<<"var:"<<array_var_name<<" base_array:"<<base_array[array_var_name]<<endl;
						cout<<"range: "<<NUM_VAL<<" MAX ROW:"<<MAX_ROWS_ARRAY_VAR<<endl;
						cout<<"--------------------------"<<endl; */
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
		vector<string>::iterator lista;

		

		for (lista = lista_variables.begin(); lista != lista_variables.end(); lista++)
				{
					dimension_matriz += rango_variable[*lista];
					cout << "Variable: " << *lista << endl;
					cout << "Rango variable: " << rango_variable[*lista] << endl;
					cout << "Dimensión acumulada: " << dimension_matriz << endl;
				}		

			

		matriz_datos = new int* [dimension_matriz];
		cout << "Espacio para punteros asignado ..................." << endl;
		for (int i=0;i<dimension_matriz;i++)
		{
			matriz_datos[i]=new int[dimension_matriz];
		}
		
		//matriz_datos[0]= new int[dimension_matriz*dimension_matriz];
    	// for(int i = 1; i<dimension_matriz;i++)
    	// {
      	// 	matriz_datos[i] = matriz_datos[i-1]+dimension_matriz;
		// }

		// Inicializo los valores de la matriz a 1.
		for (int i=0; i< dimension_matriz;i++)
			for (int j=0;j<dimension_matriz;j++)
				matriz_datos[i][j]=1 ;


#ifdef midebug
		//ofstream fmatriz("pocholo.txt", ios::out);
		//imprime_matriz("datos",fmatriz);
		//imprime_matriz("shadow",fmatriz); 
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
		
		return o;
	}






	
	









	// REVISAR SI HAY QUE VOLVER A IMPLEMENTARLA.
	// Funcion que escribe en la matriz reglas unarias. Hay que adaptarla a 
	// no usar get_indice() o get_nombre()
	void escribe_en_matriz_unaria(vector<int>& tuplas,string var_unaria, bool support)
	{
		std::vector<int>::iterator itero_valores;
		int coordenada_final[2];



		if (support) 
		{
			//cout << "Regla SUPPORT UNARIA....." << endl;

#ifdef midebug	
		/* 	cout << "Var:" << variable << " min var: "
					<< minimo_variable[variable] << endl; */
#endif	

			if (tuplas.size()==0)
			{
				// No hay tuplas y es una regla support => todo a ceros
				cout << "CONJUNTO DE TUPLAS VACIO: TODO A CEROS" << endl;
				for (int i = 0; i < rango_variable[var_unaria]; i++)
					for (int j = 0; j < rango_variable[var_unaria]; j++) {
						coordenada_final[0] = base_variable[var_unaria] + i;
						coordenada_final[1] = base_variable[var_unaria] + j;
#ifdef midebug
							cout << "writing-0-S en:(" << coordenada_final[0] << ","
								<< coordenada_final[1] << ")" << endl;
#endif
						matriz_datos[coordenada_final[0]][coordenada_final[1]] =0;
						matriz_datos[coordenada_final[1]][coordenada_final[0]] =0;
						//}
					}
			} else {
					//La regla SUPPORT dice cuales son posibles (por tanto se ELIMINAN el RESTO de valores)

					cout << "Escribiendo en la matriz una Regla SUPPORT UNARIA.........." << endl;

					for (itero_valores = tuplas.begin(); itero_valores != tuplas.end();
							++itero_valores)
					{ 
						
#ifdef midebug
						cout << "Valor Tupla: " << *itero_valores
							<< endl;
#endif

						//Escritura en horizontal y vertical
						for(int i=0;i<dimension_matriz;i++)
						{
							// REVISAR ESTA FÓRMULA SI HAY QUE VOLVER A IMPLEMENTAR ESTA FUNCIÓN.
							coordenada_final[0] = base_variable[var_unaria]
								+ (*itero_valores)
								- minimo_variable[var_unaria];

							coordenada_final[1] =i;

							matriz_datos[coordenada_final[0]][coordenada_final[1]] = 1;
							matriz_datos[coordenada_final[1]][coordenada_final[0]] = 1;
						}

#ifdef midebug
						/* cout << "Coordenada base variable: "<< variable << "-> (" << 
						coordenadas_base[0] << "," << coordenadas_base[1] << ")" << endl;
						
						cout << "Tupla support leida-coord:(" << coordenada_final[0]
							<< "," << coordenada_final[1] << ")" << endl; */
#endif
					}

					// Borro el resto de restricciones
					for (int i = 0; i < rango_variable[var_unaria]; i++)
						for (int j = 0; j < dimension_matriz; j++) {
							coordenada_final[0] = base_variable[var_unaria] + i;
							coordenada_final[1] = j;
							matriz_datos[coordenada_final[0]][coordenada_final[1]] =0;
							matriz_datos[coordenada_final[1]][coordenada_final[0]] =0;
							
#ifdef midebug
							cout << "writing-0-S en:(" << coordenada_final[1] << ","
										<< coordenada_final[0] << ")" << endl;
#endif
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
				coordenada_final[0] = base_variable[var_unaria]
						+ (*itero_valores)
						- minimo_variable[var_unaria];


				for (int i=0;i<dimension_matriz;i++)
				{
					coordenada_final[1] = i;
					matriz_datos[coordenada_final[0]][coordenada_final[1]] = 0;
					matriz_datos[coordenada_final[1]][coordenada_final[0]] = 0;
#ifdef midebug
					cout << "writing-0-C en:(" << coordenada_final[0] << ","
						<< coordenada_final[1] << ")" << endl;
#endif
				}

		
			// Escribo ceros en vertical

			 	coordenada_final[1] = coordenada_final[0];

				for (int i=0;i<dimension_matriz;i++)
				{
					coordenada_final[0] = i;
					matriz_datos[coordenada_final[0]][coordenada_final[1]] = 0;
#ifdef midebug
					cout << "writing-0-C en:(" << coordenada_final[0] << ","
						<< coordenada_final[1] << ")" << endl;
#endif
					
				} 
			
			}
		}
	}














void nueva_escribe_en_matriz(vector<vector<int> >& tuplas,string var_cero, string var_uno, bool support) 
{
		vector<vector<int>>::iterator itero_parejas;
		vector<int>::iterator itero_dentro_de_la_pareja;
		int coordenada_final[2];

		//support

		if (support)
		{
			if (tuplas.size()==0)
			{
				cout << "CONJUNTO DE TUPLAS VACIO: TODO A CEROS" << endl;
				for (int i = 0; i < rango_variable[var_cero]; i++)
					for (int j = 0; j < rango_variable[var_uno]; j++) 
					{
						coordenada_final[0] = base_variable[var_cero] + i;
						coordenada_final[1] = base_variable[var_uno] + j;
						
#ifdef midebug
							cout << "writing-0-S en:(" << coordenada_final[0] << ","
								<< coordenada_final[1] << ")" << endl;
#endif
							matriz_datos[coordenada_final[0]][coordenada_final[1]] =0;
							matriz_datos[coordenada_final[1]][coordenada_final[0]] =0;	
					}
			
		
			} else {
					// Borro el resto de restricciones
					for (int i = 0; i < rango_variable[var_cero]; i++)
						for (int j = 0; j < rango_variable[var_uno]; j++) 
						{
							coordenada_final[0] = base_variable[var_cero] + i;
							coordenada_final[1] = base_variable[var_uno] + j;
#ifdef midebug
							cout << "writing-0-S en:(" << coordenada_final[0] << ","
								<< coordenada_final[1] << ")" << endl;
#endif
							matriz_datos[coordenada_final[0]][coordenada_final[1]] =0;
							matriz_datos[coordenada_final[1]][coordenada_final[0]] =0;

#ifdef midebug
						cout << "writing-0-S en:(" << coordenada_final[1] << ","
								<< coordenada_final[0] << ")" << endl;
#endif
						}

				// Y escribo las reglas support
				for (itero_parejas = tuplas.begin(); itero_parejas != tuplas.end();++itero_parejas) 
				{
						itero_dentro_de_la_pareja = itero_parejas->begin();

#ifdef midebug
						cout << "Primer valor Tupla: " << *itero_dentro_de_la_pareja
							<< endl;
#endif

						coordenada_final[0] = base_variable[var_cero]
							+ (*itero_dentro_de_la_pareja)
							- minimo_variable[var_cero];

						itero_dentro_de_la_pareja++;
#ifdef midebug
						cout << "Segundo valor Tupla: " << *itero_dentro_de_la_pareja
							<< endl;
#endif
						coordenada_final[1] = base_variable[var_uno]
							+ (*itero_dentro_de_la_pareja)
							- minimo_variable[var_uno];

						matriz_datos[coordenada_final[0]][coordenada_final[1]] = 1;
						matriz_datos[coordenada_final[1]][coordenada_final[0]] = 1;					
#ifdef midebug
						cout << "Tupla support leida-coord:(" << coordenada_final[0]
							<< "," << coordenada_final[1] << ")" << endl;
#endif			
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

					coordenada_final[0] = base_variable[var_cero]
							+ (*itero_dentro_de_la_pareja)
							- minimo_variable[var_cero];

					itero_dentro_de_la_pareja++;
#ifdef midebug
				cout << "Segundo valor Tupla: " << *itero_dentro_de_la_pareja
						<< endl;
#endif

					coordenada_final[1] = base_variable[var_uno]
							+ (*itero_dentro_de_la_pareja)
							- minimo_variable[var_uno];

#ifdef midebug
				cout << "writing-0-C en:(" << coordenada_final[0] << ","
						<< coordenada_final[1] << ")" << endl;
#endif
					matriz_datos[coordenada_final[0]][coordenada_final[1]] = 0;
					matriz_datos[coordenada_final[1]][coordenada_final[0]] = 0;

			}
		}
}




















	// Para el caso de reglas intensionales binarias, escribe 1 en la matriz 
	void escribe_1__en_matriz(string var_cero, string var_uno,int i, int j)
	{
		int coordenada_final[2];

	#ifdef midebug
			cout << "Escribo 1 en: " << base_variable[var_cero]+i << " - " << j+base_variable[var_uno]+j << endl;
	#endif
		coordenada_final[0] = base_variable[var_cero]+i;
		coordenada_final[1] = base_variable[var_uno]+j;
		matriz_datos[coordenada_final[0]][coordenada_final[1]] = 1;
		matriz_datos[coordenada_final[1]][coordenada_final[0]] = 1;
		
	}
















	// Para el caso de reglas intensionales binarias, escribe 0 en la matriz 
	void escribe_0_en_matriz(string var_cero, string var_uno,int i, int j)
	{
		int coordenada_final[2];

#ifdef midebug
			cout << "Escribo 0 en: " << base_variable[var_cero]+i << " - " << j+base_variable[var_uno]+j << endl;
#endif
		coordenada_final[0] = base_variable[var_cero]+i;
		coordenada_final[1] = base_variable[var_uno]+j;
		matriz_datos[coordenada_final[0]][coordenada_final[1]] = 0;
		matriz_datos[coordenada_final[1]][coordenada_final[0]] = 0;
	}









	





	//Funcion que escribe en la matriz una regla AllEqual o AllDifferent
	// Hay que adaptarla para el uso de las nuevas funciones.(nueva_escribe_en_matriz())
	void  escribe_regla_all(string var_cero, string var_uno, int REGLA)
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
						coordenada_final[0]=base_variable[var_cero]+i;
						coordenada_final[1]=base_variable[var_uno]+j;
						matriz_datos[coordenada_final[0]][coordenada_final[1]] = 1;
						matriz_datos[coordenada_final[1]][coordenada_final[0]] = 1;
					}
					else {
						coordenada_final[0]=base_variable[var_cero]+i;
						coordenada_final[1]=base_variable[var_uno]+j;
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
				//cout << "i: " << i << " - j: " << j;

					if(i==j)
					{
#ifdef midebug
						cout << "  -->  Son iguales " ;
#endif
						coordenada_final[0]=base_variable[var_cero]+i;
						coordenada_final[1]=base_variable[var_uno]+j;
						matriz_datos[coordenada_final[0]][coordenada_final[1]] = 1;
						matriz_datos[coordenada_final[1]][coordenada_final[0]] = 1;
					}
					else {
						coordenada_final[0]=base_variable[var_cero]+i;
						coordenada_final[1]=base_variable[var_uno]+j;
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
//	 ==========Fin de mis funciones=============================
//
//	 =========Comienzo de las funciones que invoca el parser ===
////////////////////////////////////////////






	void beginInstance(InstanceType type) {

#ifdef midebug
		cout << "Empieza Instancia tipo: " << type << endl;
#endif

		//XCSP3PrintCallbacks::beginInstance(type);
	}













	void endInstance() {
		

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
		if (numero_variables == 1)
			lista_variables_singleton.push_back(array_actual);
		rango_array[array_actual] = rango_variables;
		minimo_variable[array_actual] = minimo_variables;

		is_array=false;

#ifdef midebug
/* 		cout << "Base siguiente array: " << base_siguiente_array << endl;
		cout << "Numero variables: " << numero_variables << " - Rango: "
				<< rango_array << endl; */
#endif

	}











	// Comienza el proceso de variables. De momento no se hace nada.
	void beginVariables() {

		
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















	//PSS calls here alsp for variables with singleton values (<var id="x0"> -1 <\var> )
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

		for (int i = minValue; i<= maxValue; i++)
		{
			valores_variable[id].push_back(i);
		}

		// Para tratar los arrays actualmente deprecated, sin uso
		rango_variables = (maxValue - minValue) + 1;
		minimo_variables = minValue;					/*TODO-hay variables (singleton) con valor -1!!*/
		numero_variables++;
		
		// Para tratar cada variable de manera individual
		rango_variable[id] = (maxValue - minValue) + 1;
		maximo_variable[id] = maxValue;
		minimo_variable[id] = minValue;


		if (primera_variable == "No")	
			base_variable[id] = base_variable[variable_anterior] + rango_variable[variable_anterior];
		
		
		cout << "Variable: " << id << " indice: "<< (numero_variables-1) << " - min: " << minValue << " - max: "
				<< maxValue << " - Base variable en la matriz: " << base_variable[id] << " - Rango: " << rango_variable[id] << endl;

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
		
		
		cout << "Regla UNARIA: " << endl;
		escribe_en_matriz_unaria(tuples,variable->id,support);
	}

















	//Versión para restricciones binarias o superiores
	void buildConstraintExtension(string id, vector<XVariable *> list,
		vector<vector<int>> &tuples, bool support, bool hasStar) {
		
		cout<< "Parsing buildConstraintExtension..........................................."<< endl;

		// Guardo el valor de las tuplas por si es una restriccion de grupo
		las_tuplas=tuples;


	#ifdef mydebug
		cout << "Tamaño de la lista: " << list.size() << endl;
		cout << "Tamaño tuplas: " << las_tuplas.size() << endl;
	#endif
	
		
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
			throw runtime_error("ERROR: Tamaño no procesado con esta versión del generador de grafos.");
			exit(2);
		}

	}














	//Versión para restricciones Unarias y Binarias.
	void buildConstraintExtensionAs(string id, vector<XVariable *> list,
			bool support, bool hasStar) {
		
		cout<< "Parsing buildConstraintExtension  AS ........................................."<< endl;

	#ifdef mydebug
		cout << "Tamaño de la lista: " << list.size() << endl;
		cout << "Tamaño tuplas: " << las_tuplas.size() << endl;
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
			exit(2);
		}

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
    	
		int i=0,j=0,k=0;
		int REGLA;

		REGLA=DIFERENTE;		
		cout << "\n   Mi allDiff constraint " << id << "Tamaño de la regla: "<< list.size() << endl;

		if (list.size() != 2)
		{
			cout << "Tamaño de la regla: " << list.size() << endl;
			throw runtime_error("ERROR: Tamaño no procesado con esta versión del generador de grafos.");
			exit(2);
		}
		
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












	////////////////////
	//
	// PROCESSING FORMULAS
	//
	///////////////////


	void buildConstraintPrimitive(string id, OrderType orden, XVariable *x, int k, XVariable *y) {
    	string var_cero,var_uno;
		int dimension=2; 
		int coordenadas_final[2]={0,0};


		cout << "\nFórmula simple.............. \n  " << id;
			
	#ifdef midebug
			cout << "\n   OPERACIONES BINARIAS............... Order Type: " << orden <<endl;
			cout << "PRIMITIVA .....................\n";
			cout << x->id << " base: " << base_variable[x->id] << " - " << y->id << " base: " 
				<< base_variable[y->id] << " : Operación: " << orden << endl; 
	#endif
		
		switch(orden)
		{
			case (LE):
				cout << "Less or Equal (" << orden << ")" << endl;
				for (int i = 0; i< rango_variable[x->id]; i++)
					for (int j = 0; j < rango_variable[y->id]; j++)
						if (!(valores_variable[x->id][i] <= valores_variable[y->id][j]))
							escribe_0_en_matriz(x->id,y->id,i,j);
				break;
			case (LT):
				cout << "Less Than (" << orden << ")" << endl;
				for (int i = 0; i< rango_variable[x->id]; i++)
					for (int j = 0; j < rango_variable[y->id]; j++)
						if (!(valores_variable[x->id][i] < valores_variable[y->id][j]))
							escribe_0_en_matriz(x->id,y->id,i,j); 
				break;
			case (GE):
				cout << "Greater or Equal (" << orden << ")" << endl;
				for (int i = 0; i< rango_variable[x->id]; i++)
					for (int j = 0; j < rango_variable[y->id]; j++)
						if (!(valores_variable[x->id][i] >= valores_variable[y->id][j]))
							escribe_0_en_matriz(x->id,y->id,i,j);
				break;
			case (GT):
				cout << "Greater Than (" << orden << ")" << endl;
				for (int i = 0; i< rango_variable[x->id]; i++)
					for (int j = 0; j < rango_variable[y->id]; j++)
						if (!(valores_variable[x->id][i] > valores_variable[y->id][j]))
							escribe_0_en_matriz(x->id,y->id,i,j);
				break;
			
			case (IN):
				cout << "Contenido en (" << orden << ")" << endl;
				throw runtime_error("Pendiente de implementar ........\n");
				exit(2); 
				
			case (EQ):
				cout << "Equal (" << orden << ")" << endl;
				for (int i = 0; i< rango_variable[x->id]; i++)
					for (int j = 0; j < rango_variable[y->id]; j++)
						if (!(valores_variable[x->id][i] == valores_variable[y->id][j]))
							escribe_0_en_matriz(x->id,y->id,i,j);
				break;
			case (NE):
				cout << "Not Equal (" << orden << ")" << endl;
				for (int i = 0; i< rango_variable[x->id]; i++)
					for (int j = 0; j < rango_variable[y->id]; j++)
					{
						//cout << "Valores: " << valores_variable[x->id][i] << " " << valores_variable[y->id][j] << endl;
						if (!(valores_variable[x->id][i] != valores_variable[y->id][j]))
						{
						//	cout << " ¡SON IGUALES! Escribo cero \n";
							escribe_0_en_matriz(x->id,y->id,i,j);
						}
					}
		} 
	}














  	void buildConstraintIntension(string id, Tree *tree) {
		vector<string> variable;
		vector<int> rango;
    	map<string, int> tupla;
		int resultado=0; 
		int coordenadas_final[2];
		

    	cout << "\nFórmula compleja de orden: " << tree->arity() << " ..............   \n";
    	// tree->prefixe();

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
			exit(2); 

		}
    	
	}









//////////////////////////////////
//
// 	MÁS INTENSIONAL
//
//////////////////////////////////




	void buildConstraintPrimitive(string id, XVariable *x, bool in, int min, int max) {
			cout << "\n  FOLLÓN constraint  " << id << ":"<< x->id << (in ? " in " : " not in ") << min << ".." << max <<"\n";
	}














	void beginConstraints() {
		cout << "\nComienza la declaración de las Restricciones (Constraints) ..............\n" << endl;
		
	}










	void endConstraints() {
		cout << "Fin declaración Constraints .................." << endl << endl;
	}
	
	










//////////////////////////////////
//
// 	PROCESANDO REGLAS CHANNEL
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
		vector<XVariable *>::iterator itero1,itero2;
		int coordenada_final[2],coordenada_base[2];
		int i = 0,j = 0, k=0;

		
		

		cout << "\n   Restricción de \"CANAL\": " << endl;
		cout << "        list1 ";
		displayList(list1);
		cout << "        list2 ";
		displayList(list2);

		
		// Pongo a cero las variables del canal

		cout << "Poniendo a cero las variables del canal ............\n";
  
		for(k=0,itero1 = list1.begin(); itero1 != list1.end();itero1++)
		{
			//cout << "Variable1: " << (*itero1)->id << endl;
			for (itero2 = list2.begin(); itero2 != list2.end(); itero2++)
			{
				//cout << "Variable2: " << (*itero2)->id << " " << endl;
				coordenada_base[0] = base_variable[(*itero1)->id];
				coordenada_base[1] = base_variable[(*itero2)->id];

				for(int i=0; i < rango_variable[(*itero1)->id]; i++)
					for (int j=0; j < rango_variable[(*itero2)->id]; j++)
					{
						coordenada_final[0] = coordenada_base[0] + i;
						coordenada_final[1] = coordenada_base[1] + j;	
						// cout << "Coordenadas finales[" << k << "]: " << coordenada_final[0] << " - " << coordenada_final[1] << endl;
						k++;
						matriz_datos[coordenada_final[0]][coordenada_final[1]] = 0;
						matriz_datos[coordenada_final[1]][coordenada_final[0]] = 0;
					}	
			}
			//cout << endl; 
		} 

		// Pongo a Uno el canal    
		for (k=0, itero1 = list1.begin(), i=0; itero1 != list1.end();itero1++,i++)
		{
			
			coordenada_base[0] = base_variable[(*itero1)->id];
			
			for (itero2 = list2.begin(), j = 0; itero2 != list2.end(); itero2++, j++)
			{
				coordenada_final[0] = coordenada_base[0]+j;
				coordenada_final[1] = base_variable[(*itero2)->id]+i;
				//cout << "Var1: " << (*itero1)->id << " - Var2: " << (*itero2)->id << " - ";
				//cout << "Coordenadas finales[" << k << "]: " << coordenada_final[0] << " - " << coordenada_final[1] << endl;
				
				k++;
					
				matriz_datos[coordenada_final[0]][coordenada_final[1]] = 1;
				matriz_datos[coordenada_final[1]][coordenada_final[0]] = 1;
			}
			
		}
		//cout << endl;
	}











	//////////////////////////////////
	//
	// 	PROCESANDO REGLAS SLIDE
	//
	//////////////////////////////////


	void beginSlide(string id, bool) {
		cout << "Empieza un slide.............." << id << endl;
	}













	void endSlide() {
		cout << "\nFin slide..............." << endl;
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
	char *nombre_fichero_dimacs;
	int dimension = 0;
	time_t hora = time(NULL);

	

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


	cout << "\n\n- SEGUNDA FASE -" << endl;

///////////////////
// GENERACION DEL FICHERO .csp

	cout << "\nCreando el fichero (.csp) .....................\n";
	miparser.escribe_fichero_csp();
	

///////////////////
// GENERACION DE UGRAPH

	
	cout << "Creando el fichero DIMACS con el grafo (.clq) ............" << endl;
		
	// Una vez leido el fichero y generada la matriz, se vuelca en un Grafo a fichero

	printf("%s",ctime(&hora));
	cout << "La dimensión de la Matriz BINARIA: " << miparser.dimension_matriz << endl;
	cout << "Generando el grafo en memoria .................." << endl;
	
	// Escribir matriz intensional BINARIA
	ugraph ug(miparser.dimension_matriz);
	for (int i=0;i< miparser.dimension_matriz-1;i++)
	{
		for (int j=i+1;j<miparser.dimension_matriz;j++)
		{
			
			if (miparser.matriz_datos[i][j]==1)
			{
				ug.add_edge(i,j);
			}
				
		}
	}  

	// Removes incompatible edges between values of the same variable-  MUST BE!
	// miparser.pongo_diagonal_matriz_a_cero();
	// miparser.remove_edges_same_var(ug);
	ug.set_name(miparser.nombre_fichero, false);
	
	nombre_fichero_dimacs = strrchr(miparser.nombre_fichero, '.');
	strcpy(nombre_fichero_dimacs, ".clq");

	printf("%s",ctime(&hora));

	
	cout << "Escribiendo el grafo resultante al fichero \"" << miparser.nombre_fichero 
		<<  "\" .........\n";

	const clock_t comienzo = clock();


	fstream f(miparser.nombre_fichero, ios::out);
	ug.write_dimacs(f);
	f.close();
	
	cout << "Tiempo empleado en escribir el fichero: " << float( clock () - comienzo ) /  CLOCKS_PER_SEC 
		<< " segundos." << endl;
 
	//salida matriz de datos
 	/* ofstream fmat("log_mat.txt", ios::out);
	miparser.imprime_matriz("datos",fmat);
	fmat.close(); */

	//cout << "\n\nEl resultado de la matriz de DATOS ......................\n " << endl;
	//ostream & terminal=cout;
	//miparser.imprime_matriz("datos", terminal);

	/*cout << "\n\nEl resultado de la matriz SHADOW ......................\n " << endl;
	miparser.imprime_matriz("shadow", terminal); */

	
    // Para reglas binarias
    delete [] miparser.matriz_datos;
	
	// Para reglas n-arias
	
	return 0;
}

