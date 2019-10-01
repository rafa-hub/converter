// #include "../gtest/gtest.h"
#include "graph/graph.h"
#include "graph/algorithms/decode.h"
#include "utils/common.h"
#include "utils/logger.h"

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

//#define mipause
//#define midebug
//#define mitest

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
	map<string, vector<int>> variables_singleton;	// Guarda los valores discretos de una variable	
	bool is_array=false;				// PSS-determina si una varaible es un singleton o forma parte de un array

						// Todas las variables ahora se tratan como singleton. 
						// Van a quedar deprecated todas las variables antiriores.



	map<string,int> mapa_indices;		// Guarda el índice de cada variable 
	map<string, vector<int>> valores_variable_discreta;	// Guarda los valores discretos de una variable
	string primera_variable = "Si";
	string variable_anterior="Vacia";
	map<string, int> base_array; 		// Mapa de cada array con su coordenada base
	map<string, int> base_variable;		// Mapa de cada Variable con su coordenada base, debe sustituir a base_array
	map<string, int> maximo_variable; 	// Guarda el máximo del rango de cada una de las variables
	map<string, int> minimo_variable; 	// Guarda el minimo del rango de cada una de las variables
	map<string, int> rango_array;	 	// Mapa de cada array con el rango de valores de las variables
	map<string, int> rango_variable; 	// Mapa con el rango de valores de las variables
	map<string, int> numero_variable;	// Mapa de cada array con el numero de instancias
										    // de variables del array

	map<int,vector<string>> nueva_super_variable;		// Nueva Super-Variable para procesar las reglas ternarias.
														// Contiene las variables como strings.
	
	

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
	vector<int> tamano_total_tuplas;	// Vector que almacena el tamaño total de los elementos de las tuplas: (dimensión*número de tuplas)

	map <int,vector<int>> mapa_vertices;	// Lista de vértices.
	stack <string> pila_comparacion;		// Pila para hacer la comparación de los datos.

	
	
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
	int **matriz_aristas; // Matriz con todas las aristas.
	int **matriz_vertices;  // Matriz donde se almacenan los punteros a los valores de las tuplas 
							// de los vértices a procesar.
	int indice_vertices=0;	// Índice global para indexar los vértices del grafo. (a quitar)
	
	map <int , vector <int>> grafo;		// Almacena el grafo que será volcado a fichero. DEPRECATED.
	
	
	

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
			fichero_salida << "v " << (j + 1) << " " << rango_variable[lista_variables[j]]	<< endl;

#ifdef midebug
			cout << var << endl;
			cout << "v " << (j + 1) << " " << rango_variable[var] << endl;
#endif
		}

		fichero_salida.close();
	}












	void escribe_fichero_csp_ternario() 
	{
		string var;
		char *nombre_fichero_csp;
		map <int,vector<int>>::iterator iterador_vertices,iterador_fin;

		nombre_fichero_csp = strrchr(nombre_fichero, '.');
		strcpy(nombre_fichero_csp, ".csp");
		cout << "Nombre fichero CSP: " << nombre_fichero << endl;
		

		ofstream fichero_salida(nombre_fichero);

		fichero_salida<< "c Fichero creado a partir de un fichero XML que expresa un problema CSP"<< endl;
		fichero_salida << "x " << nueva_super_variable.size() << endl;

		

		for (unsigned int j = 0; j < nueva_super_variable.size(); j++)
			fichero_salida << "v " << (j + 1) << " " << dimension_variables_ternarias[j]
					<< endl;
		

		fichero_salida.close();
	}

	











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















	//Extrae y devuelve el nombre de la variable sin indice, es decir, el nombre del array
	// Deprecated, a dejar de usar
	string get_nombre(string variable) {
		string nombre, vector;
		size_t aux1 = 0;

		nombre = variable;

		if (es_singleton(nombre))
		{
			return nombre;
		}

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
		/* coordenadas_base--;
		cout << "Var cero: " << var_cero << " - indice: " << indice0 << " - Coordenada Base X: " 
		<< *coordenadas_base << endl;
		
		coordenadas_base++;
		cout << "Var uno: " << var_uno << " - indice: " << indice1 << " - Coordenada Base Y: " 
		<< *coordenadas_base << endl; */
#endif

		return;
	}









	void calcula_coordenadas_ternarias(int indice1, int indice2, int *coordenadas_base) 
	{
		int coord_X=0,coord_Y=0;
		
		coordenadas_base[0] = tamano_total_tuplas[indice1];
		coordenadas_base[1] = tamano_total_tuplas[indice2];

		for (int i=0; i < indice1; i++)
			coord_X += tamano_total_tuplas[i];

		coordenadas_base[0]=coord_X;

		cout << "\nCalculando ..............."<< endl;
		cout << "X: " << coordenadas_base[0] << " - Y: " << coordenadas_base[1] << endl;



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














	void reserva_memoria_punteros()
	{
		matriz_vertices = new int *[BUFFER_PUNTEROS];
		cout << "Creado buffer punteros y buffer de punteros a vértices con " << (BUFFER_PUNTEROS) << " posiciones." << endl;
			
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
		for (int i=0;i<dimension_matriz;i++)
		{
			matriz_datos[i]=new int[dimension_matriz];
		}
		
		//matriz_datos[0]= new int[dimension_matriz*dimension_matriz];
    
		//matriz_shadow = new int *[dimension_matriz];
		//matriz_shadow[0]= new int[dimension_matriz*dimension_matriz];


    	// for(int i = 1; i<dimension_matriz;i++)
    	// {
      	// 	matriz_datos[i] = matriz_datos[i-1]+dimension_matriz;
		// 	//matriz_shadow[i] = matriz_shadow[i-1]+dimension_matriz;
    	// }

		for (int i=0; i< dimension_matriz;i++)
		{
			for (int j=0;j<dimension_matriz;j++)
			{
				matriz_datos[i][j]=1 ;
				//matriz_shadow[i][j]=1;
			}
		}


#ifdef midebug
		//ofstream fmatriz("pocholo.txt", ios::out);
		//imprime_matriz("datos",fmatriz);
		//imprime_matriz("shadow",fmatriz); 
#endif
	}











	void genera_matriz_ternaria()
	{
		// Ahora mismo solo funciona para matrices ternarias
		int dimension=3;
		int fila=0;
		int contador=0;
		vector<string>::iterator itero;

		
		for (int i=0 ; i < (lista_variables.size()-2); i++)
		{
			for(int j=i+1; j < (lista_variables.size()-1) ; j++)
			{
				for(int k = j+1; k < lista_variables.size(); k++)
				{
					
					//cout << "Variable: " << i <<  " - " << j << " - " << k << endl;
					//cout << lista_variables[i] <<  " - " << lista_variables[j] << " - " << lista_variables[k] << endl;
					nueva_super_variable[contador] = {lista_variables[i],lista_variables[j],lista_variables[k]};
					fila = pow(rango_variable[lista_variables[i]],dimension);
					dimension_variables_ternarias.push_back(fila);
					//cout << "Tamaño fila: " << fila << endl; 
					matriz_vertices[contador] = new int [fila];
					for(int l=0; l < fila; l++)
						matriz_vertices[contador][l] = 1;
					contador++;	
				}
			}
		}

		cout << "Número total de supervariables: " << contador << endl;
		
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
		/* if (matriz == "shadow") {
			//cout<<"MATRIZ SHADOW----------------"<<endl;
			for (int x = 0; x < dimension_matriz; x++){
				for (int y = 0; y < dimension_matriz; y++){
					o << matriz_shadow[x][y] << " ";
				}
				o << endl;
			}
			o << "\n\n" << endl;
		} */
		return o;
	}






	
	










	//Funcion que escribe en la matriz reglas unarias. Hay que adaptarla a 
	// no usar get_indice() o get_nombre()
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
						//if (!matriz_shadow[coordenada_final[0]][coordenada_final[1]]) {
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
							//matriz_shadow[coordenada_final[0]][coordenada_final[1]] = 1;
							matriz_datos[coordenada_final[1]][coordenada_final[0]] = 1;
							//matriz_shadow[coordenada_final[1]][coordenada_final[0]] = 1;
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

							//if (!matriz_shadow[coordenada_final[0]][coordenada_final[1]]) {
	
								matriz_datos[coordenada_final[0]][coordenada_final[1]] =0;
								matriz_datos[coordenada_final[1]][coordenada_final[0]] =0;
							//}

#ifdef midebug
							cout << "writing-0-S en:(" << coordenada_final[1] << ","
										<< coordenada_final[0] << ")" << endl;
#endif

							//if (!matriz_shadow[coordenada_final[1]][coordenada_final[0]] ) {

								matriz_datos[coordenada_final[0]][coordenada_final[1]] =0;
								matriz_datos[coordenada_final[1]][coordenada_final[0]] =0;
							//}
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













	//Funcion que escribe en la matriz reglas binarias
	void escribe_en_matriz(int *coordenadas_base, vector<vector<int> >& tuplas,
			string var_cero, string var_uno, bool support) {
		//vector<vector<int>>::iterator it;

		vector<vector<int>>::iterator itero_parejas;
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
						//if (!matriz_shadow[coordenada_final[0]][coordenada_final[1]]) {
#ifdef midebug
							cout << "writing-0-S en:(" << coordenada_final[0] << ","
								<< coordenada_final[1] << ")" << endl;
#endif
							matriz_datos[coordenada_final[0]][coordenada_final[1]] =0;
							matriz_datos[coordenada_final[1]][coordenada_final[0]] =0;
						//}
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
						//matriz_shadow[coordenada_final[0]][coordenada_final[1]] = 1;
						//matriz_datos[coordenada_final[1]][coordenada_final[0]] = 1;
						//matriz_shadow[coordenada_final[1]][coordenada_final[0]] = 1;
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
					//	if (!matriz_shadow[coordenada_final[0]][coordenada_final[1]]) {
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
					//}

					//if (!matriz_shadow[coordenada_final[1]][coordenada_final[0]] ) {
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
					//}
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









	// Para el caso de reglas intensionales binarias, escribe en la matriz que fue creada
	// para reglas extensionales binarias.

	void escribe_en_matriz_intensional(int *coordenadas_base, string var_cero, string var_uno,int i, int j)
	{
		int coordenada_final[2];

	#ifdef midebug
			cout << "Coordenadas_base: " << coordenadas_base[0] << " - " << coordenadas_base[1] << endl;
			cout << "Variables: " << var_cero << " - " << var_uno << endl;
			cout << "Índices: " << i << " - " << j << endl; 
			cout << "Valores donde escribo: " << i+coordenadas_base[0] << " - " << j+coordenadas_base[1] << endl;
	#endif


		coordenada_final[0] = coordenadas_base[0]+i;
		coordenada_final[1] = coordenadas_base[1]+j;
		
		//cout << "Es aquíiiii ..................\n";
		matriz_datos[coordenada_final[0]][coordenada_final[1]] = 1;
		matriz_datos[coordenada_final[1]][coordenada_final[0]] = 1;
		//cout << "Síiiii ..................\n";
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
	// Hay que adaptarla para el uso de las nuevas funciones.(nueva_escribe_en_matriz())
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
				//cout << "i: " << i << " - j: " << j;

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







	void escribe_grafo()
	{
		string var;
		char *nombre_fichero_csp;
		int contador_vertices=0;

		nombre_fichero_csp = strrchr(nombre_fichero, '.');
		strcpy(nombre_fichero_csp, ".clq");
		cout << "Nombre fichero .CLQ: " << nombre_fichero << endl;
		
		ofstream fichero_salida(nombre_fichero);

		// Cuento el número de aristas del grafo

		for (unsigned int i = 0; i < dimension_variables_ternarias.size(); i++)
		{
			for (unsigned int j=0; j < dimension_variables_ternarias[i] ; j++)
			{
				//cout << "Número de variables en la fila: " << dimension_variables_ternarias[i] << endl;
				if (matriz_vertices[i][j] == 1)
				{
					contador_vertices++;
				}
			}	
		}


		fichero_salida << "c Fichero creado a partir de un fichero XML que expresa un problema CSP"<< endl;
		fichero_salida << "c " << nombre_fichero << endl;
		fichero_salida << "p " << dimension_variables_ternarias.size() << " " << contador_vertices  <<  endl;

		// for (unsigned int j = 0; j < lista_variables_ternarias.size(); j++)
		// 	fichero_salida << "e " << grafo[j][0] << " " << grafo[j][1] << endl;

		cout << "Filas Matriz a volcar (Número de Supervariables): " << dimension_variables_ternarias.size() << endl;
		
		
		for (unsigned int i = 0; i < dimension_variables_ternarias.size(); i++)
		{
			for (unsigned int j=0; j < dimension_variables_ternarias[i] ; j++)
			{
				//cout << "Número de variables en la fila: " << dimension_variables_ternarias[i] << endl;
				if (matriz_vertices[i][j] == 1)
				{
					fichero_salida << "e " << i+1 << " " << j+1 << endl;
					//cout << "e " << i+1 << " " << j+1 << endl;
				}
			}	
		}
		fichero_salida << endl;
		fichero_salida.close();


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
	void endVariableArray() {

		base_siguiente_array += (numero_variables * rango_variables);
		numero_variable[array_actual] = numero_variables;
		rango_array[array_actual] = rango_variables;
		minimo_variable[array_actual] = minimo_variables;

		is_array=false;

#ifdef midebug
		cout << "Base siguiente array: " << base_siguiente_array << endl;
		cout << "Numero variables: " << numero_variables << " - Rango: "
				<< rango_array << endl;
#endif

	}








	// Comienza el proceso de variables. De momento no se hace nada.
	void beginVariables() {

		// Reserva espacio para los punteros cuando el fichero tiene reglas
		// ternarias.
		reserva_memoria_punteros();


//#ifdef midebug
		cout << "COMIENZA la declaracion de variables............. " << endl;
//#endif

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
		print_coordenadas_base();
		cout << " - FIN declaracion variables - " << endl << endl;
#ifdef mipause
		cin.get();
#endif
#endif
	}










	//PSS calls here alsp for variables with singleton values (<var id="x0"> -1 <\var> )
	void buildVariableInteger(string id, int minValue, int maxValue) override {
		
		//cout << "Primera Variable: " << primera_variable << " - Variable anterior: " << variable_anterior << endl;

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
				
		mapa_indices[id]=numero_variables;

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
		
		cout << "Variable: " << id << " indice: "<< (numero_variables-1)
			<< " Número variable: " << numero_variables 
			<< " - min: " << minValue << " - max: "<< maxValue << endl;


		//cout << "Variable: " << id << " indice: "<< (numero_variables-1) << " - min: " << minValue << " - max: "
		//		<< maxValue << " - Base variable en la matriz: " << base_variable[id] << " - Rango: " << rango_variable[id] << endl;

		
	}








	//called for stand-alone values independent of a range: we assume they DO belong to a range
	void buildVariableInteger(string id, vector<int> &values) override {

		vector<int>::iterator itero_values;

		lista_variables.push_back(id);
		lista_variables_discretas.push_back(id);
		rango_variable[id] = values.size();

		rango_variables = values.size();
		minimo_variables = values.front(); 		/*TODO-extend to non-index values */
		maximo_variables = values.back();
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
	void buildConstraintExtension(string id, XVariable *variable, vector<int> &tuples, bool support, bool hasStar) {
		
		
		cout << "Regla UNARIA:" << endl;
		cout << "Hay que implementar........." << endl;

		cout << "Variable: " << variable->id << endl;

		cout << "Número tuplas: " << tuples.size() << endl;
		cout << "Tuplas: ";
		for (int i = 0; i < tuples.size(); i++)
		{
			cout << tuples[i] << " " << endl;			
		} 


		cout << endl;
		/* string var_cero, var_uno, var_aux;
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
*/

	}








	//Versión para restricciones binarias o superiores
	void buildConstraintExtension(string id, vector<XVariable *> list,
			vector<vector<int>> &tuples, bool support, bool hasStar) {

		int i=0,j=0,k=0;
		int contador=0;
		int rango=0;
		int fila=0;
		string var;
		vector<int> auxiliar;
		stack <int> pila_comparacion;
		
		int indice0, indice1,indice_aux;;
		int dimension;
		string var_cero, var_uno, var_aux;
		int coordenadas_base[2];
		vector<vector<int>>::iterator itero_parejas;
		
		
		int *puntero_ternario; 	// Puntero para recorrer la matriz ternaria
		int *puntero_vertice;	// Puntero auxiliar para recorrer la tupla de cada vértice.
								// Se inicializa un nuevo puntero de la matriz y se asigna
								// a este puntero auxiliar.

		vector<XVariable *>::iterator itero_variables;
		vector<string>::iterator itero_dentro_variables;
		vector<vector<int>>::iterator itero_tuplas;
		vector <int>::iterator itero_dentro_tuplas;

		vector <string> super_variable;
		int id_supervariable=0;
		


		cout<< "Parsing buildConstraintExtension..........................................."<< endl;

		// Guardo el valor de las tuplas por si es una restriccion de grupo
		las_tuplas=tuples;
		
		cout << "Número variables: " << list.size() << endl;
		
		
		if(list.size()==0)
		{
			throw runtime_error("Tamaño cero de tupla, hay algún error, no procesado.");
			exit(2);
		}



		if (list.size() == 1){
			cout << "Regla UNARIA:" << endl;
			cout << "¡¡¡¡Funcionalidad no implementada cuando hay reglas ternarias!!!! ........" << endl; 
			
			cout << "Variable Unaria: " << (list[0]->id) << endl;

			indice0 = get_indice(*(list[0]));
			indice1 = indice0;

			var_cero = get_nombre(list[0]->id);
			var_uno = var_cero;
			calcula_coordenadas_base(var_cero, var_uno, indice0, indice1,coordenadas_base);
			escribe_en_matriz_unaria(coordenadas_base, tuplas_unarias, var_cero, support);
		
		} 

		if (list.size() == 2){
			cout << "Regla BINARIA:" << endl;
			cout << "Hay que implementar........." << endl;
			displayList(list);
			cout << endl;
			
			/*cout << "Par de variables: " << (list[0]->id) << " - " << (list[1]->id)	<< endl;
			//cout <<  "Coordenada base nueva: " << base_variable[list[0]->id] << " - " << base_variable[list[1]->id] << endl;

			nueva_escribe_en_matriz(las_tuplas,list[0]->id,list[1]->id,support);
			//escribe_en_matriz(coordenadas_base, las_tuplas, var_cero, var_uno, support);  */
		}


		if(list.size() == 3)
		{
			
			cout << "Regla TERNARIA:" << endl;

			super_variable.clear();
			//displayList(list);
			for (itero_variables = list.begin();itero_variables < list.end();itero_variables++)
			{
				cout << (*itero_variables)->id << " - " ;
				super_variable.push_back((*itero_variables)->id);
				
			}

			cout << "\nNúmero de Supervariables: " << nueva_super_variable.size() << endl;

			for(int i=0; i < nueva_super_variable.size(); i++)
			{
				while(!pila_comparacion.empty())
					pila_comparacion.pop();	
				for(int j=0; j<3 ; j++)
				{
					for (int k=0; k<3; k++)
						{
							if (nueva_super_variable[i][j] == super_variable[k])
							{
								pila_comparacion.push(1);
								//cout << nueva_super_variable[i][j] << " - " << super_variable[k] << " Pila: " << pila_comparacion.size() << endl;
							}
						}
				}
				//cout << "Fin comparación = " << pila_comparacion.size() << endl;

				if (pila_comparacion.size() == list.size())
				{
					id_supervariable = i;
					cout << "Coincide con la Nueva Supervariable: " << id_supervariable << endl; 
					break;
				}
			}

			contador=0;

			// Genero los valores de los vértices	
			// Y Pongo los valores de las tuplas a 0
			for (int j=0; j < rango_variable[nueva_super_variable[id_supervariable][0]]; j++)
				for (int k=0 ; k < rango_variable[nueva_super_variable[id_supervariable][1]]; k++)
					for (int h=0; h < rango_variable[nueva_super_variable[id_supervariable][2]]; h++)
					{
						auxiliar.clear();
						auxiliar.push_back(j);
						auxiliar.push_back(k);
						auxiliar.push_back(h);
						cout << "Contador: " << contador << endl;
						cout << auxiliar[0] << " " << auxiliar[1] << " " << auxiliar[2] << endl;
						for (auto superit=las_tuplas.begin();superit!=las_tuplas.end();superit++)
						{
							if (*superit == auxiliar)
							{
								cout << "Es un 0 ...............\n";
								matriz_vertices[id_supervariable][contador]=0;
							}
						}
					contador++;

					}
		
			cout << endl;
			cout << "Fila de la variable Ternaria: " << id_supervariable << endl;
			for (int i=0; i < fila; i++)
				cout << matriz_vertices[id_supervariable][i] << " - " ;
			cout << endl;

			indice_var_ternarias_con_ceros++;



		}


		if (list.size() > 3){
			cout << "Regla N-ARIA > 3: " << endl;
			throw runtime_error("¡¡¡¡Funcionalidad no implementada cuando hay reglas ternarias!!!! ........");
			exit(2); 
		} 

	}











	//Versión para restricciones Unarias y Binarias.
	void buildConstraintExtensionAs(string id, vector<XVariable *> list,
			bool support, bool hasStar) {
		
		int i=0,j=0,k=0;
		int contador=0;
		int rango=0;
		int fila=0;
		string var;
		vector<int> auxiliar;
		stack <int> pila_comparacion;
		
		
		int dimension;
		string var_cero, var_uno, var_aux;
		int indice0, indice1, indice_aux;
		int coordenadas_base[2];
	
		vector<vector<int>>::iterator it;
		vector<int>::iterator ite;
 
		int *puntero_ternario; 	// Puntero para recorrer la matriz ternaria.
		int *puntero_vertice;	// Puntero auxiliar para recorrer la tupla de cada vértice.
								// Se inicializa un nuevo puntero de la matriz y se asigna
								// a este puntero auxiliar.
		
		vector<XVariable *>::iterator itero_variables;
		vector<string>::iterator itero_dentro_variables;
		vector<vector<int>>::iterator itero_tuplas;
		vector <int>::iterator itero_dentro_tuplas;

		vector <string> super_variable;
		int id_supervariable=0;
		

		cout<< "Parsing buildConstraintExtension  AS ........................................."<< endl;
		//cout << "Tamaño de la lista: " << list.size() << endl;
		
		
		
		if(list.size()==0)
		{
			throw runtime_error("Tamaño cero de tupla, hay algún error, no procesado.");
			exit(2);
		}



		if (list.size() == 1){
			cout << "Regla UNARIA:" << endl;
			cout << "¡¡¡¡Funcionalidad no implementada cuando hay reglas ternarias!!!! ........" << endl; 
			
			cout << "Variable Unaria: " << (list[0]->id) << endl;

			indice0 = get_indice(*(list[0]));
			indice1 = indice0;

			var_cero = get_nombre(list[0]->id);
			var_uno = var_cero;
			calcula_coordenadas_base(var_cero, var_uno, indice0, indice1,coordenadas_base);
			escribe_en_matriz_unaria(coordenadas_base, tuplas_unarias, var_cero, support);
		
		} 

		if (list.size() == 2){
			cout << "Regla BINARIA:" << endl;
			cout << "Hay que implementar........." << endl;
			displayList(list);
			cout << endl;
			
			/*cout << "Par de variables: " << (list[0]->id) << " - " << (list[1]->id)	<< endl;
			//cout <<  "Coordenada base nueva: " << base_variable[list[0]->id] << " - " << base_variable[list[1]->id] << endl;

			nueva_escribe_en_matriz(las_tuplas,list[0]->id,list[1]->id,support);
			//escribe_en_matriz(coordenadas_base, las_tuplas, var_cero, var_uno, support);  */
		}


		if(list.size() == 3)
		{
			
			cout << "Regla TERNARIA:" << endl;

			super_variable.clear();
			//displayList(list);
			for (itero_variables = list.begin();itero_variables < list.end();itero_variables++)
			{
				cout << (*itero_variables)->id << " - " ;
				super_variable.push_back((*itero_variables)->id);
				
			}

			cout << "\nNúmero de Supervariables: " << nueva_super_variable.size() << endl;

			for(int i=0; i < nueva_super_variable.size(); i++)
			{
				while(!pila_comparacion.empty())
					pila_comparacion.pop();	
				for(int j=0; j<3 ; j++)
				{
					for (int k=0; k<3; k++)
						{
							if (nueva_super_variable[i][j] == super_variable[k])
							{
								pila_comparacion.push(1);
								//cout << nueva_super_variable[i][j] << " - " << super_variable[k] << " Pila: " << pila_comparacion.size() << endl;
							}
						}
				}
				//cout << "Fin comparación = " << pila_comparacion.size() << endl;

				if (pila_comparacion.size() == list.size())
				{
					id_supervariable = i;
					cout << "Coincide con la Nueva Supervariable: " << id_supervariable << endl; 
					break;
				}
			}

			contador=0;

			// Genero los valores de los vértices	
			// Y Pongo los valores de las tuplas a 0
			for (int j=0; j < rango_variable[nueva_super_variable[id_supervariable][0]]; j++)
				for (int k=0 ; k < rango_variable[nueva_super_variable[id_supervariable][1]]; k++)
					for (int h=0; h < rango_variable[nueva_super_variable[id_supervariable][2]]; h++)
					{
						auxiliar.clear();
						auxiliar.push_back(j);
						auxiliar.push_back(k);
						auxiliar.push_back(h);
						cout << "Contador: " << contador << endl;
						cout << auxiliar[0] << " " << auxiliar[1] << " " << auxiliar[2] << endl;
						for (auto superit=las_tuplas.begin();superit!=las_tuplas.end();superit++)
						{
							if (*superit == auxiliar)
							{
								cout << "Es un 0 ...............\n";
								matriz_vertices[id_supervariable][contador]=0;
							}
						}
					contador++;

					}
		
			cout << endl;
			cout << "Fila de la variable Ternaria: " << id_supervariable << endl;
			for (int i=0; i < fila; i++)
				cout << matriz_vertices[id_supervariable][i] << " - " ;
			cout << endl;

			indice_var_ternarias_con_ceros++;



		}


		if (list.size() > 3){
			cout << "Regla N-ARIA > 3: " << endl;
			throw runtime_error("¡¡¡¡Funcionalidad no implementada cuando hay reglas ternarias!!!! ........");
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
    	
		int indice0,indice1;
		string var_cero, var_uno;
		int coordenadas_base[2];
		int i,j,k;
		int REGLA;

		REGLA=DIFERENTE;		
		//cout << "\n   Mi allDiff constraint " << id << "Tamaño de la tupla: "<< list.size() << endl;
		//displayList(list);
		
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












	////////////////////
	//
	// PROCESSING FORMULAS
	//
	///////////////////


	void buildConstraintPrimitive(string id, OrderType orden, XVariable *x, int k, XVariable *y) {
    	string var_cero,var_uno;
		int rango_cero,rango_uno,indice0,indice1;
    	int dimension=2; 
		int coordenadas_base[2]={0,0};
		int *punt_auxiliar;
		int coordenadas_final[2]={0,0};

		punt_auxiliar=coordenadas_base;

	//cout << "\nFórmula simple.............. \n  " << id;
			
	#ifdef midebug
			cout << "\nFórmula simple..............   " << id;
			cout << endl;
			cout << "\n   OPERACIONES BINARIAS............... Order Type: " << orden <<endl;
	#endif

		cout << "Mi gran polla ........\n";

		var_cero=get_nombre(x->id);
		var_uno=get_nombre(y->id);
		indice0=get_indice_ternario(x->id);
		indice1=get_indice_ternario(y->id);
		rango_cero=rango_variable[var_cero];
		rango_uno=rango_variable[var_uno];

		//calcula_coordenadas_base(var_cero, var_uno, indice0, indice1,coordenadas_base);
		*punt_auxiliar = base_array[var_cero] + (indice0 * rango_variable[var_cero]);
		punt_auxiliar++;
		*punt_auxiliar = base_array[var_uno] + (indice1 * rango_variable[var_uno]);

		cout << var_cero << "[" << indice0 << "] - " << var_uno << "[" << indice1 << "] : Operación: " << orden << endl; 


	#ifdef midebug
			cout << "Var uno: " << var_cero << "- Índice: " << indice0 << " - Rango: " << rango_cero << 
				" - Var dos: " << var_uno << "- Índice: " << indice1 << " Rango: " << rango_uno << endl;
	#endif
		
		switch(orden)
		{
			case (LE):
				cout << "Less or Equal (" << orden << ")" << endl;
				for (int i=0; i<rango_cero;i++)
					for (int j=0;j<rango_uno;j++)
						if (i>j)
						{
							//escribe_en_matriz_intensional(coordenadas_base, var_cero, var_uno,i,j)
							coordenadas_final[0] = coordenadas_base[0]+i;
							coordenadas_final[1] = coordenadas_base[1]+j;					
							matriz_datos[coordenadas_final[0]][coordenadas_final[1]] = 0;
							matriz_datos[coordenadas_final[1]][coordenadas_final[0]] = 0;

						}
				break;
			case (LT):
				cout << "Less Than (" << orden << ")" << endl;
				for (int i=0; i<rango_cero;i++)
					for (int j=0;j<rango_uno;j++)
						if (i>=j)
						{
							//escribe_en_matriz_intensional(coordenadas_base, var_cero, var_uno,i,j);
							coordenadas_final[0] = coordenadas_base[0]+i;
							coordenadas_final[1] = coordenadas_base[1]+j;
							
							matriz_datos[coordenadas_final[0]][coordenadas_final[1]] = 0;
							matriz_datos[coordenadas_final[1]][coordenadas_final[0]] = 0;
						}
				break;
			case (GE):
				cout << "Greater or Equal (" << orden << ")" << endl;
				for (int i=0; i<rango_cero;i++)
					for (int j=0;j<rango_uno;j++)
						if (i<j)
						{
							//escribe_en_matriz_intensional(coordenadas_base, var_cero, var_uno,i,j);
							coordenadas_final[0] = coordenadas_base[0]+i;
							coordenadas_final[1] = coordenadas_base[1]+j;
							
							matriz_datos[coordenadas_final[0]][coordenadas_final[1]] = 0;
							matriz_datos[coordenadas_final[1]][coordenadas_final[0]] = 0;
						}
				break;
			case (GT):
				cout << "Greater Than (" << orden << ")" << endl;
				for (int i=0; i<rango_cero;i++)
					for (int j=0;j<rango_uno;j++)
						if (i<=j)
						{
							//escribe_en_matriz_intensional(coordenadas_base, var_cero, var_uno,i,j);
							coordenadas_final[0] = coordenadas_base[0]+i;
							coordenadas_final[1] = coordenadas_base[1]+j;
							
							matriz_datos[coordenadas_final[0]][coordenadas_final[1]] = 0;
							matriz_datos[coordenadas_final[1]][coordenadas_final[0]] = 0;
						}
				break;
			case (IN):
				cout << "Contenido en (" << orden << ")" << endl;
				cout << "Pendiente de implementar\n";
				break;
			case (EQ):
				cout << "Equal (" << orden << ")" << endl;
				for (int i=0; i<rango_cero;i++)
					for (int j=0;j<rango_uno;j++)
						if (i!=j)
						{
							//escribe_en_matriz_intensional(coordenadas_base, var_cero, var_uno,i,j);
							coordenadas_final[0] = coordenadas_base[0]+i;
							coordenadas_final[1] = coordenadas_base[1]+j;
							
							matriz_datos[coordenadas_final[0]][coordenadas_final[1]] = 0;
							matriz_datos[coordenadas_final[1]][coordenadas_final[0]] = 0;
						}
				break;
			case (NE):
				cout << "Not Equal (" << orden << ")" << endl;
				for (int i=0; i<rango_cero;i++)
					for (int j=0;j<rango_uno;j++)
						if (i==j)
						{
							//cout << "i: " << i << " - " << "j: " << j << endl;
							//escribe_en_matriz_intensional(coordenadas_base, var_cero, var_uno,i,j);
							coordenadas_final[0] = coordenadas_base[0]+i;
							coordenadas_final[1] = coordenadas_base[1]+j;
							//cout << "Escribo en coordenadas: " << coordenadas_final[0] << " - " << coordenadas_final[1] << endl;
							
							matriz_datos[coordenadas_final[0]][coordenadas_final[1]] = 0;
							matriz_datos[coordenadas_final[1]][coordenadas_final[0]] = 0;
						}					
				break;
			} 
		}












  	void buildConstraintIntension(string id, Tree *tree) {
		vector<string> variable;
		vector<int> rango;
    	map<string, int> tupla;
		string var_cero,var_uno;
		int indice0,indice1;
		int resultado=0; 
		int coordenadas_base[2];
		int *punt_auxiliar;
		int coordenadas_final[2];

		punt_auxiliar=coordenadas_base;
		

    	//cout << "\nFórmula compleja..............   \n";
    	//tree->prefixe();
				

		for(int i=0;i<tree->arity();i++)
    	{
     		variable.push_back(tree->listOfVariables[i]);
			rango.push_back(rango_variable[get_nombre(tree->listOfVariables[i])]);
    	}

		

		/* for(int i=0;i<tree->arity();i++)
		{	
			cout << "Variable: " << variable[i] << " - Rango de valores: " << rango[i] << endl;
		} 

		cout << endl; */

		


		if (tree->arity()==2)
		{
			var_cero=get_nombre(variable[0]);
			var_uno=get_nombre(variable[1]);
			
			indice0=get_indice_ternario(variable[0]);
			indice1=get_indice_ternario(variable[1]);

			cout << variable[0] <<  " - " << variable[1] << endl;
			//cout << var_cero << ": " << indice0 << " - " << var_uno << ": " << indice1 << endl;
			//calcula_coordenadas_base(var_cero, var_uno, indice0, indice1,coordenadas_base);
			
			*punt_auxiliar = base_array[var_cero] + (indice0 * rango_variable[var_cero]);
			punt_auxiliar++;
			*punt_auxiliar = base_array[var_uno] + (indice1 * rango_variable[var_uno]);	
			
		
			for (int i=0; i<rango[0];i++)
			{
				for(int j=0;j<rango[1];j++)
				{
					//cout << variable[0] << ": " << i << " - " << variable[1] << ": " << j << endl;
					//cout << var_cero << ": " << indice0 << " - " << var_uno << ": " << indice1 << endl;
					tupla[variable[0]]=i;
					tupla[variable[1]]=j;
					
					resultado = tree->evaluate(tupla);
					
		#ifdef midebug
					tree->prefixe();
					cout << "=  " << resultado << endl << endl;
		#endif

					if (!resultado)
					{		
						//escribe_en_matriz_intensional(coordenadas_base, var_cero, var_uno,i,j);
						coordenadas_final[0] = coordenadas_base[0]+i;
						coordenadas_final[1] = coordenadas_base[1]+j;	
						
						matriz_datos[coordenadas_final[0]][coordenadas_final[1]] = 0;
						matriz_datos[coordenadas_final[1]][coordenadas_final[0]] = 0;
					}
				}
			}
		}
    
    	

    	cout << "\n";
	}









//////////////////////////////////
//
// 	MÁS INTENSIONAL
//
//////////////////////////////////

void buildConstraintSum(string id, vector<XVariable *> &list, vector<int> &coeffs, XCondition &cond)
{

	XCSP3PrintCallbacks::buildConstraintSum(id,list,coeffs,cond);
}







void beginConstraints() {
    cout << "\nComienza la declaración de las Restricciones (Constraints) ..............\n" << endl;
	//genera_matriz_ternaria();
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

		
		// Pongo a cero todo
  
		for(k=0,itero1 = list1.begin(); itero1 != list1.end();itero1++)
		{
			cout << "Variable1: " << (*itero1)->id << endl;
			for (itero2 = list2.begin(); itero2 != list2.end(); itero2++)
			{
				cout << "Variable2: " << (*itero2)->id << " " << endl;
				coordenada_base[0] = base_variable[(*itero1)->id];
				coordenada_base[1] = base_variable[(*itero2)->id];

				for(int i=0; i < rango_variable[(*itero1)->id]; i++)
					for (int j=0; j < rango_variable[(*itero2)->id]; j++)
					{
						coordenada_final[0] = coordenada_base[0] + i;
						coordenada_final[1] = coordenada_base[1] + j;
						
						cout << "Coordenadas finales[" << k << "]: " << coordenada_final[0] << " - " 
							<< coordenada_final[1] << endl;
						k++;
						matriz_datos[coordenada_final[0]][coordenada_final[1]] = 0;
						matriz_datos[coordenada_final[1]][coordenada_final[0]] = 0;
					}	
			}
			cout << endl; 
		} 

 


/* 

		for(itero1 = list1.begin(); itero1 != list1.end();itero1++)
		{
			for (itero2 = list2.begin(); itero2 != list2.end(); itero2++)
			{
				cout << "lista1: " << (*itero1)->id << " - Rango: " << rango_variable[(*itero1)->id] 
					<< " - Base en Matriz: " << base_variable[(*itero1)->id];
				cout << " -.- lista2: " << (*itero2)->id << " - Rango: " << rango_variable[(*itero2)->id] 
					<< " - Base en Matriz: " << base_variable[(*itero2)->id] << endl;
					
			} 
		} 
 */

		// Pongo a Uno el canal
		
    
		for (k=0, itero1 = list1.begin(), i=0; itero1 != list1.end();itero1++,i++)
		{
			
			coordenada_base[0] = base_variable[(*itero1)->id];
			
			for (itero2 = list2.begin(), j = 0; itero2 != list2.end(); itero2++, j++)
			{
				coordenada_final[0] = coordenada_base[0]+j;
				coordenada_final[1] = base_variable[(*itero2)->id]+i;

				cout << "Var1: " << (*itero1)->id << " - Var2: " << (*itero2)->id << " - ";
				cout << "Coordenadas finales[" << k << "]: " << coordenada_final[0] << " - " << coordenada_final[1] << endl;
				
				k++;
					
				matriz_datos[coordenada_final[0]][coordenada_final[1]] = 1;
				matriz_datos[coordenada_final[1]][coordenada_final[0]] = 1;
			}
			
		}
  
 
		cout << endl;

	}









	//////////////////////////////////
	//
	// 	PROCESANDO REGLAS SLIDE
	//
	//////////////////////////////////


	void beginSlide(string id, bool) {
		cout << "   start slide CON POLLÓN." << id << endl;
	}













	void endSlide() {
		cout << "   end slide  SIN POLLÓN." << endl;
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
//GENERACION DEL FICHERO .csp

	cout << "\nCreando el fichero (.csp) .....................\n";
	miparser.escribe_fichero_csp();
	//miparser.escribe_fichero_csp_ternario(); // Para cuando hay binarización.





///////////////////
//GENERACION DE UGRAPH

	
	cout << "Creando el fichero DIMACS con el grafo (.clq) ............" << endl;
	miparser.escribe_grafo();
	
	
	// Una vez leido el fichero y generada la matriz, se vuelca en un Grafo a fichero

	/* dimension = miparser.indice_var_ternarias_con_ceros * miparser.dimension_variables_ternarias[0];

	ugraph ug(dimension);

	cout << "Número de aristas: " << miparser.indice_var_ternarias_con_ceros*miparser.dimension_variables_ternarias[0] << 
	       " Dimensión total: " << dimension <<endl; 
	
	for (int i=0; i < miparser.indice_var_ternarias_con_ceros; i++)
		for(int j=0; j < miparser.dimension_variables_ternarias[i]; j++)
		{
			if (miparser.matriz_vertices[i][j] == 1)
			{
				//cout << i << "," << j << " ";
				ug.add_edge(i,j);
			}
		} 
	cout << endl; */
	




	//cout << "La dimensión de la Matriz BINARIA: " << miparser.dimension_matriz << endl;
	// Escribir matriz intensional BINARIA
	/* ugraph ug(miparser.dimension_matriz);
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
	//removes incompatible edges between values of the same variable-  MUST BE!
	//miparser.pongo_diagonal_matriz_a_cero();
	//miparser.remove_edges_same_var(ug);
	////////////////////
	*/
	
	
	/* ug.set_name(miparser.nombre_fichero, false);
	

	nombre_fichero_dimacs = strrchr(miparser.nombre_fichero, '.');
	strcpy(nombre_fichero_dimacs, ".clq");

	cout << "Escribiendo el grafo resultante al fichero " << miparser.nombre_fichero <<  " .......................\n\n\n";

	std::fstream f(miparser.nombre_fichero, ios::out);
	ug.write_dimacs(f);
	f.close();
 */
	
	//salida matriz de datos
	ofstream fmat("log_mat.txt", ios::out);
	miparser.imprime_matriz("datos",fmat);
	fmat.close();

	cout << "\n\nEl resultado de la matriz de DATOS ......................\n " << endl;
	ostream & terminal=cout;
	miparser.imprime_matriz("datos", terminal);

	/*cout << "\n\nEl resultado de la matriz SHADOW ......................\n " << endl;
	miparser.imprime_matriz("shadow", terminal); */

	
    // Para reglas binarias
    //delete [] miparser.matriz_datos;
	
	// Para reglas n-arias
	delete [] miparser.matriz_vertices;

	return 0;
}

