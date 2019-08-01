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
#include <math.h>

//#define mipause
#define midebug
//#define mitest
#define BUFFER_PUNTEROS 10*1024*1024
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

	vector<string> 	lista_arrays;    	// Guarda la lista de arrays
	vector<string> 	lista_variables; 	// Guarda la lista de variables
	
	int indice_var_ternarias = 0;
	
	map<string,int> mapa_indices;		// Guarda el índice de cada variable
	
	bool is_array=false;				// PSS-determina si una varaible es un singleton o forma parte de un array

	map<string, int> base_array; 		// Mapa de cada array con su coordenada base
	map<string, int> minimo_variable; 	// Guarda el minimo del rango de las variables
	map<string, int> rango_variable; 	// Mapa de cada array con el rango de valores de las variables
	map<string, int> numero_variable;	// Mapa de cada array con el numero de instancias
										    // de variables del array

	map<int,vector<string>> nueva_super_variable;	// Nueva Super-Variable para procesar las reglas ternarias.
													// Contiene las variables como strings.
	map<int,vector<string>> vertices_variable;		// Nueva Super-Variable para procesar las reglas ternarias.
													// Contiene los índices de los vértices correspondientes a la variable.
	map<int,vector<int>> tuplas_binarias;			// Contiene todas las posibles tuplas binarias.
	map<int,vector<int>> tuplas_ternarias;			// Contiene todas las posibles tuplas ternarias.

	string array_actual = "empiezo"; 	// Sirve para identificar con que array se esta trabajando
	int base_siguiente_array = 0; 		// Guarda el valor para calcular la posicion en la matriz del siguiente array
	int minimo_variables = 0;        	// Guarda el minimo valor de cada variable
	int rango_variables = 0; 			// Guarda el rango de valores de las variables de un array
	int numero_variables = 0;      		// Guarda el numero de variables de un array

	vector<vector<int>> las_tuplas;   	// Guarda las tuplas, puesto que en
									  	// buildConstraintExtensionAs() no me las pasan como argumento
	vector<int> tuplas_unarias;			// Lo mismo, pero para variables unarias
	vector<int> tamano_tuplas;			// Vector que almacena el tamaño de las tuplas: (número de tuplas)
	vector<int> tamano_total_tuplas;	// Vector que almacena el tamaño total de los elementos de las tuplas: (dimensión*número de tuplas)

	map <int,vector<int>> mapa_vertices;	// Lista de vértices.
	stack <string> pila_comparacion;				// Pila para hacer la comparación de los datos.

	
public:

	vector<int> lista_variables_binarias;		// Guarda la lista de variables binarizadas binarias.
	vector<int> lista_variables_ternarias;		// Guarda la lista de variables binarizadas.

												// en cada posición se guarda el "número" de variables.
	vector <int> dimension_variables_ternarias;	// Guarda el número de tuplas posibles para cada var ternaria.

	int dimension_matriz = 0; 			//Guarda la dimension definitiva de la matriz creada
	int dimension_matriz_ternaria = 0;	//Guarda la dimension definitiva de la matriz ternaria
	
	int **matriz_datos; 	// Matriz donde se almacena el resultado.
	int **matriz_shadow; 	// Matriz donde se almacenan las escrituras.
	int **matriz_punteros;  // Matriz donde se almacenan los datos punteros a los datos ternarios.
	int **matriz_vertices;  // Matriz donde se almacenan los punteros a los valores de las tuplas de los vértices.
	int indice_vertices=0;	// Índice global para indexar los vértices del grafo.
	vector<int> lista_vertices; // Contiene una relación entre los vértices y su rango de valores posibles.

	map <int , vector <int>> grafo;		// Almacena el grafo que será volcado a fichero
	int contador_aristas=0;				// Sirve para contar las aristas y poder generar el grafo,
										// delimita el recorrido del mapa que almacena el grafo.
	
	

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









	void escribe_fichero_csp_ternario() 
	{
		string var;
		char *nombre_fichero_csp;
		map <int,vector<int>>::iterator iterador_vertices,iterador_fin;

		nombre_fichero_csp = strrchr(nombre_fichero, '.');
		strcpy(nombre_fichero_csp, ".csp");
		cout << "Nombre fichero CSP: " << nombre_fichero << endl;
		//cout << "Rango variables: " << lista_vertices.size() << endl;

		ofstream fichero_salida(nombre_fichero);

		fichero_salida<< "c Fichero creado a partir de un fichero XML que expresa un problema CSP"<< endl;
		fichero_salida << "x " << lista_variables_ternarias.size() << endl;

		

		for (unsigned int j = 0; j < lista_variables_ternarias.size(); j++)
			fichero_salida << "v " << (j + 1) << " " << dimension_variables_ternarias[j]
					<< endl;
		

		fichero_salida.close();
	}

	












	// Extrae y devuelve el indice de una variable

	int get_indice(XVariable variable) {
		string valor;
		int indice;

		valor = variable.id;
		indice = mapa_indices[valor];

#ifdef midebug
		cout << "En get_indice(), id variable: " << valor << " es: " << indice<< endl;
#endif

		return(indice);
// 		return(mapa_indices[variable.id]);  //Toda la función
	}


	int get_indice_ternario(string variable) {

		int pos_uno,pos_dos;
		string indice;

		pos_uno = variable.find('[');
		pos_dos = variable.find(']');

		//cout << "pos uno: " << pos_uno << " - pos dos: " << pos_dos << endl;
		indice = variable.substr(pos_uno+1,(pos_dos-pos_uno)-1);


		//cout << "Variable:  " << var << endl;
		return stoi(indice);
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




	void reserva_memoria_punteros()
	{
		matriz_punteros = new int *[BUFFER_PUNTEROS];
		matriz_vertices = new int *[BUFFER_PUNTEROS];
		cout << "Creado buffer punteros y buffer de punteros a vértices con " << (BUFFER_PUNTEROS) << " posiciones." << endl;
			
	}


	void genero_tuplas_binarias()
	{
		int i = 0;

		for (int j = 0; j < 2 ; j++)
		{
			for(int k = 0; k < 2; k++)
			{
				tuplas_binarias[i].push_back(j);
				tuplas_binarias[i].push_back(k);
				cout << i+1 << ":" "(" << j << "," << k << ")" << endl;	
				i++;
			}
		}

		/* for (int i=0; i < 4 ; i++)
		{
			cout << "Tupla: ";
			for (int j = 0; j < 2 ; j++)
				cout << tuplas_binarias[i][j];
			cout << endl;
		} */
	}









	void genero_tuplas_ternarias()
	{
		int i=0;
		
		for (int j = 0; j < 2 ; j++)
		{
			for (int k = 0; k < 2 ; k++)
			{
				for(int l = 0; l < 2; l++)
				{
					tuplas_ternarias[i].push_back(j);
					tuplas_ternarias[i].push_back(k);
					tuplas_ternarias[i].push_back(l);
					cout << i+1 << ":" "(" << j << "," << k << "," << l << ")" << endl;	
					i++;
				}
			}
		}

		/* for (int i=0; i < 8 ; i++)
		{
			cout << "Tupla: ";
			for (int j = 0; j < 3 ; j++)
				cout << tuplas_ternarias[i][j];
			cout << endl;
		} */


	}








	// Genera la matriz
	void genera_matriz() {
		std::vector<string>::iterator lista;

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
		//ofstream fmatriz("pocholo.txt", ios::out);
		//imprime_matriz("datos",fmatriz);
		//imprime_matriz("shadow",fmatriz); 
#endif
	}



	void genera_matriz_ternaria()
	{
		std::vector<string>::iterator lista;
		int i,j,k;

		for (lista = lista_arrays.begin(); lista != lista_arrays.end();lista++) 
		{
			dimension_matriz += numero_variable[*lista]
					* rango_variable[*lista];
		}

		
		for (k=0;k<(lista_arrays.size()-1);k++)
		{
			for(i=k,j=i+1; j<lista_arrays.size();j++)
			{
				dimension_matriz += 1;
			}
		}
	
	
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






	
	//Vuelca en pantalla la matriz, solo útil para depuración, en casos reales
	//la matriz suele ser demasiado grande
	ostream& imprime_matriz_ternaria(ostream& o=cout) {
		
		for (int i=0; i< lista_variables_ternarias.size();i++)
			{
				cout << "U[" << i << "] -> " << lista_variables_ternarias[i] << endl;
				
				for (int j=0;j<lista_variables_ternarias[i]; j++)
				{
					cout << matriz_punteros[i][j];
					if (j<lista_variables_ternarias[i]-1)
						cout << ",";
				}
				cout << endl;
				
				
			}
			cout << endl;


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



void imprimo_vertices()
	{
		cout << "Imprimo los vertices del grafo por cada nueva Nueva Variable. Número de vértices: " << indice_vertices << endl;
		
		
				
		for (int i=0; i<lista_variables_ternarias.size();i++)
		{
			
			cout << "U[" << i << "]: " << endl;
			cout << "Número de tuplas: " << tamano_tuplas[i] << endl;
			
			for (int j=0; j<lista_variables_ternarias[i]; j++)
			{
				cout << nueva_super_variable[i][j];
				if (j<(lista_variables_ternarias[i]-1))
					cout << ", ";
			}
			cout << endl;

			for (int j=0; j<tamano_tuplas[i]; j++)
			{
				cout << "v(" << ((mapa_vertices[i][j])+1) << ")" ;
				if (j<(tamano_tuplas[i]-1))
					cout <<", ";
			}
			
			cout << "\n------------------------------\n";
		}
		

	}






	

	void relleno_aristas(int primera,int segunda)
	{
		cout << "Número de vértices U[" << primera << "]: " << mapa_vertices[primera].size() << endl;
		cout << "Número de vértices U[" << segunda << "]: " << mapa_vertices[segunda].size() << endl;

		for (int i=0;i<mapa_vertices[primera].size();i++)
		{
			for(int j=0;j<mapa_vertices[segunda].size();j++)
			{
				cout << "Arista ......... v(" << mapa_vertices[primera][i] << ") <-> v(" << mapa_vertices[segunda][j] << ")" << endl; 
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
				cout << "Encontrada "  << var << ", posición " << i << endl;
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



void comparo_vertices_conflict(int indice_nueva_variable1, vector<int>pos_var_uno, int indice_nueva_variable2,vector<int>pos_var_dos)
	{
		stack <int> pila_resultado;
		int vertice1,vertice2;
		int bucle=0;
		int hay_coincidencia=0;
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

				hay_coincidencia = pila_resultado.top();
				bucle = pila_resultado.size();

				for (int l=0; l< bucle;l++)
				{	
					if (pila_resultado.top() == 0)
						hay_coincidencia=0;
					pila_resultado.pop();
				}

				if(hay_coincidencia)
				{	cout << "Regla conflict: no hay arista entre los vertices " << vertice1 << " y " << vertice2 << endl;

					//Escribo en el grafo
					
				}
				else
					{
						cout << "Regla conflict: el resto a unos " << vertice1 << " y " << vertice2 <<  endl;
						cout << "Aunque todavía no sé como hacerlo.\n";
						grafo[contador_aristas]={vertice1,vertice2};
						contador_aristas++;
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












	void genero_grafo()
	{
		int i=0,j=0,k=0,l=0;
		
		int indice=0;
		vector<string>::iterator itero_primera_variable,itero_segunda_variable;
		
		int coordenadas_base[2];
		int pos_uno=0,pos_dos=0;



		contador_aristas=0;
		cout << "\n\nGenero el fichero DIMACS con el grafo......................\n" << endl;
		
		
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




	void genero_grafo_mixto()
	{
		int i=0,j=0,k=0,l=0;
		
		int indice=0;
		vector<string>::iterator itero_primera_variable,itero_segunda_variable;
		
		int coordenadas_base[2];
		int pos_uno=0,pos_dos=0;



		contador_aristas=0;
		cout << "\n\nGenero el fichero DIMACS con el grafo......................\n" << endl;
		
		
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









	






	void imprimo_datos_grafo()
	{
		int i=0,j=0,k=0,l=0;
		vector<string>::iterator itero_dentro_variables;
		vector<string>::iterator itero_primera_variable,itero_segunda_variable;


		cout << "\nImprimo las variables y sus tuplas ........." << endl;

		cout << "\nLista variables: " << endl;
		for (int l=0;l<lista_variables_ternarias.size();l++)
		{
			cout << "U[" << l << "]: ";
			for(itero_dentro_variables=nueva_super_variable[l].begin();
					itero_dentro_variables<nueva_super_variable[l].end();itero_dentro_variables++)
					cout << *itero_dentro_variables << " ";
			cout << endl;
		}

		cout << "\nTuplas para cada variable: " << endl;
		for (k=0;k<lista_variables_ternarias.size();k++)
		{
			for(i=k,j=i+1; j<lista_variables_ternarias.size();j++)
			{
				cout << "U[" << i << "]: " << endl;

				for(itero_dentro_variables=nueva_super_variable[i].begin();
					itero_dentro_variables<nueva_super_variable[i].end();itero_dentro_variables++)
					cout << *itero_dentro_variables << " ";
				
				cout << endl;
				
				for (int h=0;h<lista_variables_ternarias[i]; h++)
				{
					cout << matriz_punteros[i][h];
					if (h<lista_variables_ternarias[i]-1)
						cout << ",";
				}
				cout << endl;

				cout << "U[" << j << "]: " << endl;

				for(itero_dentro_variables=nueva_super_variable[j].begin();
					itero_dentro_variables < nueva_super_variable[j].end() ; itero_dentro_variables++)
					cout << *itero_dentro_variables << " ";
				
				cout << endl;

				for (int h=0;h<lista_variables_ternarias[j]; h++)
				{
					cout << matriz_punteros[j][h];
					if (h<lista_variables_ternarias[j]-1)
						cout << ",";
				}
				cout << "\n" << endl;	
				

			}
			cout <<endl;
		}

		cout << "\n=========================================\n" << endl;

		for (k=0;k<lista_variables_ternarias.size()-1;k++)
		{
			for (i=k,j=i+1;j<lista_variables_ternarias.size();j++)
			{
				cout << "U[" << i << "] - U[" << j << "]" << endl ;
		 		for(itero_primera_variable=nueva_super_variable[i].begin(),itero_segunda_variable=nueva_super_variable[j].begin(); 
					itero_primera_variable < nueva_super_variable[i].end();itero_primera_variable++,itero_segunda_variable++)
					{
						cout << *itero_primera_variable << ": " << endl;
						for (l=0;l<tamano_total_tuplas[i]; l++)
						{
							cout << matriz_punteros[i][l] << " - "; 
						}
						cout << endl;
						cout << *itero_segunda_variable << ": " << endl;
						for (l=0;l<tamano_total_tuplas[j]; l++)
						{
							cout << matriz_punteros[j][l] << " - "; 
						}
						cout << endl;
					}
					
				cout << endl;
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
		ostream & terminal=cout;

		// cout <<"---------------------------------------------------"<<endl;
		// imprime_matriz_ternaria(terminal);
		// cout <<"---------------------------------------------------"<<endl;

		for (int i=0;i<lista_variables_ternarias.size();i++)
		{
			dimension_matriz_ternaria += (lista_variables_ternarias[i]);
		} 

		
		//imprimo_vertices();

		//genero_grafo();
		genero_grafo_mixto();
		escribe_grafo();


		//imprimo_datos_grafo();

		/* vector<string>::iterator itero;
		for (itero = lista_arrays.begin(); itero != lista_arrays.end();	itero++) {
			cout << "Array: " << *itero << endl;
			cout << "Numero variables: " << numero_variable[*itero] << endl;
			cout << "Fila base de la matriz: " << base_array[*itero] << endl;
			cout << "Primer valor: " << minimo_variable[*itero]<< endl;
			cout << "Número de valores: " << rango_variable[*itero]<< endl;
			cout << endl;
		}*/
		
		//cout << "Dimension total datos Ternarios: " << dimension_matriz_ternaria << endl;
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
		//escribe_nombre_fichero();
		reserva_memoria_punteros();

		// Genero la matriz
		//cout << "Genero la matriz Ternaria............." << endl;
		//genera_matriz_ternaria();
		genero_tuplas_binarias();
		genero_tuplas_ternarias();

		//cout << "Genero la matriz Binaria............." << endl;
		//genera_matriz();
		
		//cout << "Matriz generada .............." << endl;
#ifdef midebug
		//print_coordenadas_base();
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
		cout << "Regla UNARIA:" << endl;
		throw runtime_error("¡¡¡¡Funcionalidad no implementada en esta versión.");
		exit(2);
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

#ifdef mipause
        	cin.get();
#endif
	

#ifdef midebug
		cout << "\n ** Fin buildConstraintExtension ** " << id << endl;
#endif */

	}






	//Versión para restricciones binarias o superiores
	void buildConstraintExtension(string id, vector<XVariable *> list,
			vector<vector<int>> &tuples, bool support, bool hasStar) {

		int i=0,j=0;
		int rango = 0;
		string var;
		int dimension = 0;
		int numero_vertices_nueva_variable=0;
		int hay_vertice = 1;
		/* string var_cero, var_uno, var_ternaria;
		int indice0, indice1, i,j,k;
		int coordenadas_base[2];
		vector<vector<int>>::iterator itero_parejas; */
		
		
		int *puntero_ternario; 	// Puntero para recorrer la matriz ternaria
		int *puntero_vertice;	// Puntero auxiliar para recorrer la tupla de cada vértice.
								// Se inicializa un nuevo puntero de la matriz y se asigna
								// a este puntero auxiliar.

		vector<XVariable *>::iterator itero_variables;
		vector<string>::iterator itero_dentro_variables;
		vector<vector<int>>::iterator itero_tuplas;
		vector <int>::iterator itero_dentro_tuplas;



		cout<< "Parsing buildConstraintExtension..........................................."<< endl;

		// Guardo el valor de las tuplas por si es una restriccion de grupo
		las_tuplas=tuples;
		


		if (list.size() == 2){
			
			cout << "Regla BINARIA:" << endl;
			
			if (support)
			{
				throw runtime_error("Regla Support binaria, todavía no implementada.");
				exit(2);
			} else {

				cout << "Regla CONFLICT ............\n";

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
					var = get_nombre(*itero_dentro_variables);
					rango = rango_variable[var];
					cout << "Dominio valores variable: " << rango_variable[var] << endl;
				}
				cout << endl;	

				cout << "Tamaño tuplas: " << las_tuplas.size() << endl;

				lista_variables_binarias.push_back(list.size());
				lista_variables_ternarias.push_back(list.size());
				dimension = pow(rango_variable[var],list.size());
				// cout << "Rango variable: " << rango_variable[var] << " - Dimensión: " << dimension << endl;
				
				tamano_tuplas.push_back(las_tuplas.size());
				tamano_total_tuplas.push_back(las_tuplas.size()*list.size());
				
				for (int i=0; i < 4; i++)
				{
						hay_vertice = 1;

						for (itero_tuplas = las_tuplas.begin();itero_tuplas != las_tuplas.end();++itero_tuplas)
						{							
							if(tuplas_binarias[i] == *itero_tuplas)
							{
								
								hay_vertice = 0;
							}
						}

						if (hay_vertice)
						{
							cout << "Guardo el vértice: " << indice_vertices;

							matriz_vertices[indice_vertices]=new int[list.size()];

							lista_vertices.push_back(rango);
							puntero_vertice = matriz_vertices[indice_vertices];
							mapa_vertices[indice_var_ternarias].push_back(indice_vertices);
								
							cout << "\tv(" << indice_vertices << "): " ;
							cout << "(";
							
							for(int k=0; k < list.size(); k++)
							{
								matriz_vertices[indice_vertices][k] = tuplas_binarias[i][k];
								cout << matriz_vertices[indice_vertices][k];
								if (k < list.size()-1)
									cout << ",";
							}
							cout <<")" << endl;
							indice_vertices++;
							numero_vertices_nueva_variable++;
						}
				}
				
				
				}
			dimension_variables_ternarias.push_back(numero_vertices_nueva_variable);
			cout << endl;
			indice_var_ternarias++;

		}




		if(list.size() == 3)
		{
			
			cout << "Regla TERNARIA:" << endl;
			
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
					var = get_nombre(*itero_dentro_variables);
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
			} else {

				cout << "Regla CONFLICT..............\n";
				
				// Guardo las variables que componen la regla que generan la nueva supervariable
				for (itero_variables = list.begin();itero_variables < list.end();itero_variables++)
					nueva_super_variable[indice_var_ternarias].push_back((*itero_variables)->id);
				
				
				// Imprimo resultados
				cout << "U[" << indice_var_ternarias << "]: \n";
				for(itero_dentro_variables=nueva_super_variable[indice_var_ternarias].begin();
						itero_dentro_variables<nueva_super_variable[indice_var_ternarias].end();itero_dentro_variables++)
				{
					cout << "\t" << *itero_dentro_variables << " ";
					var = get_nombre(*itero_dentro_variables);
					dimension = pow(rango_variable[var],list.size());
					cout << "Dominio valores variable: " << dimension << endl;
				}

				cout << endl;
				
				cout << "Tamaño tuplas: " << las_tuplas.size() << endl;

				lista_variables_ternarias.push_back(list.size());
				dimension = pow(rango_variable[var],list.size());
				// cout << "Rango variable: " << rango_variable[var] << " - Dimensión: " << dimension << endl;
				
				tamano_tuplas.push_back(las_tuplas.size());
				tamano_total_tuplas.push_back(las_tuplas.size()*list.size());


				for (int i=0; i < 8; i++)
				{
						hay_vertice = 1;
						for (itero_tuplas = las_tuplas.begin();itero_tuplas != las_tuplas.end();++itero_tuplas)
						{
							if(tuplas_ternarias[i] == *itero_tuplas)
							{
								hay_vertice = 0;
							}
						}

						if (hay_vertice)
						{
							cout << "Guardo el vértice: " << indice_vertices;

							matriz_vertices[indice_vertices]=new int[list.size()];

							lista_vertices.push_back(rango);
							puntero_vertice = matriz_vertices[indice_vertices];
							mapa_vertices[indice_var_ternarias].push_back(indice_vertices);
								
							cout << "\tv(" << indice_vertices << "): " ;
							cout << "(";
							
							for(int k=0; k< list.size(); k++)
							{
								matriz_vertices[indice_vertices][k] = tuplas_ternarias[i][k];
								cout << matriz_vertices[indice_vertices][k];
								if (k < list.size()-1)
									cout << ",";
							}
							cout <<")" << endl;
							indice_vertices++;
							numero_vertices_nueva_variable++;
						}
				}
					
				
			}
			dimension_variables_ternarias.push_back(numero_vertices_nueva_variable);
			cout << endl;
			indice_var_ternarias++;

		}


		if (list.size() > 3){
			cout << "Regla N-ARIA > 3: " << endl;
			cout << "¡¡¡¡Funcionalidad no implementada cuando no hay reglas ternarias!!!! ........" << endl;

/* 
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
		} */
			
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
		
		int i=0,j=0,k=0;
		int rango=0;
		string var;
		int dimension=0;
		int numero_vertices_nueva_variable=0;
		int hay_vertice = 1;
		/*string var_cero, var_uno, var_aux;
		int indice0, indice1, indice_aux;
		int coordenadas_base[2];
	
		vector<vector<int>>::iterator it;
		vector<int>::iterator ite;
 */
		int *puntero_ternario; 	// Puntero para recorrer la matriz ternaria.
		int *puntero_vertice;	// Puntero auxiliar para recorrer la tupla de cada vértice.
								// Se inicializa un nuevo puntero de la matriz y se asigna
								// a este puntero auxiliar.
		
		vector<XVariable *>::iterator itero_variables;
		vector<string>::iterator itero_dentro_variables;
		vector<vector<int>>::iterator itero_tuplas;
		vector <int>::iterator itero_dentro_tuplas;

		cout<< "Parsing buildConstraintExtension  AS ........................................."<< endl;
		//cout << "Tamaño de la lista: " << list.size() << endl;
		
		
		if(list.size()==0)
		{
			throw runtime_error("Tamaño de tupla no procesado.");
			exit(2);
		}


		if (list.size() == 1){
			cout << "Regla UNARIA:" << endl;
			cout << "¡¡¡¡Funcionalidad no implementada cuando no hay reglas ternarias!!!! ........" << endl; 
			
			/* cout << "Variable Unaria: " << (list[0]->id) << endl;

			indice0 = get_indice(*(list[0]));
			indice1 = indice0;

			var_cero = get_nombre(list[0]->id);
			var_uno = var_cero;
			calcula_coordenadas_base(var_cero, var_uno, indice0, indice1,coordenadas_base);
			escribe_en_matriz_unaria(coordenadas_base, tuplas_unarias, var_cero, support); */
		
		} 

		
		
		if (list.size() == 2){
			cout << "Regla BINARIA:" << endl;
			if (support)
			{
				throw runtime_error("Regla Support binaria, todavía no implementada.");
				exit(2);
			} else {

				cout << "Regla CONFLICT ............\n";

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
					var = get_nombre(*itero_dentro_variables);
					rango = rango_variable[var];
					cout << "Dominio valores variable: " << rango_variable[var] << endl;
				}
				cout << endl;	

				cout << "Tamaño tuplas: " << las_tuplas.size() << endl;

				lista_variables_binarias.push_back(list.size());
				lista_variables_ternarias.push_back(list.size());
				dimension = pow(rango_variable[var],list.size());
				// cout << "Rango variable: " << rango_variable[var] << " - Dimensión: " << dimension << endl;
				
				tamano_tuplas.push_back(las_tuplas.size());
				tamano_total_tuplas.push_back(las_tuplas.size()*list.size());
				
				for (int i=0; i < 4; i++)
				{
						hay_vertice = 1;

						for (itero_tuplas = las_tuplas.begin();itero_tuplas != las_tuplas.end();++itero_tuplas)
						{							
							if(tuplas_binarias[i] == *itero_tuplas)
							{
								
								hay_vertice = 0;
							}
						}

						if (hay_vertice)
						{
							cout << "Guardo el vértice: " << indice_vertices;

							matriz_vertices[indice_vertices]=new int[list.size()];

							lista_vertices.push_back(rango);
							puntero_vertice = matriz_vertices[indice_vertices];
							mapa_vertices[indice_var_ternarias].push_back(indice_vertices);
								
							cout << "\tv(" << indice_vertices << "): " ;
							cout << "(";
							
							for(int k=0; k < list.size(); k++)
							{
								matriz_vertices[indice_vertices][k] = tuplas_binarias[i][k];
								cout << matriz_vertices[indice_vertices][k];
								if (k < list.size()-1)
									cout << ",";
							}
							cout <<")" << endl;
							indice_vertices++;
							numero_vertices_nueva_variable++;
						}
				}
				
				
				}
			dimension_variables_ternarias.push_back(numero_vertices_nueva_variable);
			cout << endl;
			indice_var_ternarias++;

		}
		 
		
		if (list.size() == 3)
		{
			cout << "Regla TERNARIA (AS): " << endl;

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
					var = get_nombre(*itero_dentro_variables);
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
			} else {

				cout << "Regla CONFLICT...............\n";
				
				// Guardo las variables que componen la regla que generan la nueva supervariable
				for (itero_variables = list.begin();itero_variables < list.end();itero_variables++)
					nueva_super_variable[indice_var_ternarias].push_back((*itero_variables)->id);
				
				
				// Imprimo resultados
				cout << "U[" << indice_var_ternarias << "]: \n";
				for(itero_dentro_variables=nueva_super_variable[indice_var_ternarias].begin();
						itero_dentro_variables<nueva_super_variable[indice_var_ternarias].end();itero_dentro_variables++)
				{
					cout << "\t" << *itero_dentro_variables << " ";
					var = get_nombre(*itero_dentro_variables);
					dimension = pow(rango_variable[var],list.size());
					cout << "Dominio valores variable: " << dimension << endl;
				}

				cout << endl;
				
				cout << "Tamaño tuplas: " << las_tuplas.size() << endl;

				lista_variables_ternarias.push_back(list.size());
				dimension = pow(rango_variable[var],list.size());
				// cout << "Rango variable: " << rango_variable[var] << " - Dimensión: " << dimension << endl;
				
				tamano_tuplas.push_back(las_tuplas.size());
				tamano_total_tuplas.push_back(las_tuplas.size()*list.size());
				

				
				
				
					
				for (int i=0; i < 8; i++)
				{
						hay_vertice = 1;
						for (itero_tuplas = las_tuplas.begin();itero_tuplas != las_tuplas.end();++itero_tuplas)
						{
							if(tuplas_ternarias[i] == *itero_tuplas)
							{
								hay_vertice = 0;
							}
						}

						if (hay_vertice)
						{
							cout << "Guardo el vértice: " << indice_vertices;

							matriz_vertices[indice_vertices]=new int[list.size()];

							lista_vertices.push_back(rango);
							puntero_vertice = matriz_vertices[indice_vertices];
							mapa_vertices[indice_var_ternarias].push_back(indice_vertices);
								
							cout << "\tv(" << indice_vertices << "): " ;
							cout << "(";
							
							for(int k=0; k< list.size(); k++)
							{
								matriz_vertices[indice_vertices][k] = tuplas_ternarias[i][k];
								cout << matriz_vertices[indice_vertices][k];
								if (k < list.size()-1)
									cout << ",";
							}
							cout <<")" << endl;
							indice_vertices++;
							numero_vertices_nueva_variable++;
						}
				}
					
				
			}
			dimension_variables_ternarias.push_back(numero_vertices_nueva_variable);
			cout << endl;
			indice_var_ternarias++;


			
		}

		if (list.size() > 3)
		{
			cout << "Regla N-ARIA:" << endl;
			cout << "¡¡¡¡Funcionalidad no implementada cuando no hay reglas ternarias!!!! ........" << endl; 
			
			/* displayList(list);
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
			} */
			
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
//GENERACION DEL FICHERO .csp

	miparser.escribe_fichero_csp_ternario();

	/* ostream & terminal=cout;
	miparser.imprime_matriz_ternaria(terminal); */




///////////////////
//GENERACION DE UGRAPH

	cout << "- SEGUNDA FASE -" << endl;
	cout << "Creando el fichero con el grafo ............" << endl;
	// Una vez leido el fichero y generada la matriz, se vuelca en un Grafo y se resuelve
	//ugraph ug(miparser.indice_vertices);

	cout << "Número de vérices: " << miparser.indice_vertices << " Número aristas: "
			<< miparser.contador_aristas << endl;

	/* 
	for (int i=0; i < miparser.contador_aristas;i++)
		{
			//cout <<  "e " << miparser.grafo[i][0] << " - " << miparser.grafo[i][1] << endl;
			ug.add_edge(miparser.grafo[i][0],miparser.grafo[i][1]);
		}
	 */
	/* for (int i = 0; i < (miparser.lista_variables_ternarias.size()); i++)
	{
	 	for (int j = 0; j < (miparser.lista_variables_ternarias[j]); j++)
		{
	 		cout << "Variable U[" << i << "]: "; 
	 		if (miparser.matriz_punteros[i][j] == 1) {
	 			cout << "   Valores: " << i << "," << j;
	 			ug.add_edge(i, j); 
	 		}
	 		//cout << endl;
	 	}
	 } */

	//removes incompatible edges between values of the same variable-  MUST BE!
	//miparser.remove_edges_same_var(ug);
	////////////////////

	/* ug.set_name(miparser.nombre_fichero, false);
	ug.print_data(false, cout);

	/* nombre_fichero_dimacs = strrchr(miparser.nombre_fichero, '.');
	strcpy(nombre_fichero_dimacs, ".clq");

	std::fstream f(miparser.nombre_fichero, ios::out);
	ug.write_dimacs(f);
	f.close(); */

	//salida matriz de datos
	/* ofstream fmat("log_mat.txt", ios::out);
	miparser.imprime_matriz("datos",fmat);
	fmat.close(); */

	/* cout << "\n\nEl resultado de la matriz de DATOS ......................\n " << endl;
	ostream & terminal=cout;
	miparser.imprime_matriz("datos", terminal);

	cout << "\n\nEl resultado de la matriz SHADOW ......................\n " << endl;
	miparser.imprime_matriz("shadow", terminal); */


	delete [] miparser.matriz_punteros;
	delete [] miparser.matriz_vertices;


	return 0;
}

