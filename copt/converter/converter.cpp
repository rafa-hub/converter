// #include "../gtest/gtest.h"
#include "graph/graph_gen.h"
#include "graph/algorithms/decode.h"
#include "utils/common.h"
#include "utils/logger.h"
#include "../clique/clique.h"
#include "../clique/clique_weighted.h"
#include "../clique/clique_all_max_sol.h"

#include "XCSP3CoreParser.h"
#include "XCSP3PrintCallbacks.h"

#include <fstream>
#include <string.h>
#include <iostream>
#include <climits>
#include <map>

#define nomidebug
#define RESTRICCION 0
#define SOPORTE 1
#define CREAR_MATRIZ 1
#ifndef BENCHMARK_PATH 
    #define BENCHMARK_PATH  "/var/tmp/salida"
#endif

using namespace XCSP3Core;


class MiSolverPrintCallbacks : public  XCSP3PrintCallbacks{

    private:

        

        //Variables para poder implementar la lógica de cración de la matriz
        
        
        vector <string> lista_arrays;    // Guarda la lista de arrays
        vector <string> lista_variables; // Guarda la lista de variables

        std::map <string,int> base_array;       // Mapa de cada array con su coordenada base
        std::map <string,int> minimo_variable; // Guarda el mínimo del rango de las variables
        std::map <string,int> rango_variable;   // Mapa de cada array con el rango de valores de las variables
        std::map <string,int> numero_variable;  // Mapa de cada array con el número de instancias
                                                // de variables del array
        
        string array_actual="empiezo";  // Sirve para identificar con qué array se está trabajando
        int base_siguiente_array=0;     // Guarda el valor para calcular la posición en la matriz del siguiente array
        int minimo_variables=0;          // Guarda el mínimo valor de cada variable
        int rango_variables=0;          // Guarda el rango de valores de las variables de un array
        int numero_variables=0;         // Guarda el número de variables de un array
        
        
        vector <vector <int>> las_tuplas;   // Guarda las tuplas, puesto que en 
                                            // buildConstraintExtensionAs() no me las pasan como argumento
        
    public:

        int dimension_matriz=0;             //Guarda la dimensión definitiva de la matriz creada
        vector <vector <int>> matriz_datos; // Matriz donde se almacena el resultado
        vector <vector <int>> matriz_shadow; // Matriz donde se almacena el resultado
        
        char nombre_fichero[256]; // Nombre del fichero XML a procesar
        

        

    void set_nombre_fichero(char *nombre){
        strcpy(nombre_fichero,nombre);
    }


    void escribe_nombre_fichero(){
        string var;
        char *nombre_fichero_csp;

        nombre_fichero_csp=strrchr(nombre_fichero,'.');
        strcpy(nombre_fichero_csp,".csp");
        cout << "Nombre fichero CSP: " << nombre_fichero << endl;
        ofstream fichero_salida(nombre_fichero);

    #ifdef midebug
        cout << "c Fichero creado a partir de un fichero XML que expresa un problema CSP" << endl;
        cout << "x " << lista_variables.size() << endl;
    #endif

        fichero_salida << "c Fichero creado a partir de un fichero XML que expresa un problema CSP" << endl;
        fichero_salida << "x " << lista_variables.size() << endl;
                       
        for (unsigned int j=0; j<lista_variables.size();j++)
        {
            var=get_nombre(lista_variables[j]);
            fichero_salida << "v " << (j+1) << " " << rango_variable[var] << endl;

        #ifdef midebug
            cout << var << endl;
            cout << "v " << (j+1) << " " << rango_variable[var] << endl;
        #endif
        }
        
        fichero_salida.close();
    }


    
    // Método que extrae y devuelve el índice de una variable
    int get_indice(XVariable variable){
        string indice,valor;
        size_t aux1,aux2;
        
        valor=variable.id;

        aux1=valor.find_first_of('[',0);
        aux2=valor.find_first_of(']',aux1);
        indice=valor.substr(aux1+1,aux2-2);

    #ifdef midebug
        cout << valor << " indice: " << indice << endl;
    #endif

        return (std::stoi(indice));
    }


    // Método que extrae y devuelve el nombre de la variable sin índice, es decir, el nombre del array
    string get_nombre(string variable){
        string nombre,vector;
        size_t aux1=0;

        nombre=variable;
        
        aux1=nombre.find_first_of('[',0);
        vector=nombre.substr(0,aux1);

        return vector;
    }
   
    // Método par calcular las coordenadas base de la variable. A esto habrá que sumar el orden de la
    // instancia de la variable y el valor de la coordenada de la restricción
    // Hay que restar el mínimo del rango de valores para el caso en el que no sea cero
    // Si no, se escribe fuera del rango de la matriz
    //void calcula_coordenadas_base(XVariable var0,XVariable var1,int *coordenadas_base){
    void calcula_coordenadas_base(string var_cero,string var_uno,int indice0,int indice1,int *coordenadas_base)
    {
        int coord[2];
   
    #ifdef midebug
        cout << "Var cero: " << var_cero << " - índice: " << indice0 << endl;
        cout << "Var uno: " << var_uno << " - índice: " << indice1 << endl;
    #endif

        coord[0]=base_array[var_cero]+(indice0*rango_variable[var_cero]);
        coord[1]=base_array[var_uno]+(indice1*rango_variable[var_uno]);

        // Escribo las coordenadas base dentro del método que me invoca.
        *coordenadas_base=coord[0];
        coordenadas_base++;
        *coordenadas_base=coord[1];

        return;
    }

    // Genera la matriz 
    void genera_matriz(){
        std::vector<string>::iterator lista;
        std::vector<int> fila;
        std::vector<int> fila_shadow;

        for(lista=lista_arrays.begin();lista!=lista_arrays.end();lista++)
        {
            dimension_matriz+=numero_variable[*lista]*rango_variable[*lista];
        
        //#ifdef midebug
            cout << "array: " << *lista << endl;
            cout << "numero variables: " << numero_variable[*lista] << endl;
            cout << "rango variable: " << rango_variable[*lista] << endl;
            cout << "dimensión variable: " << numero_variable[*lista]*rango_variable[*lista] << endl;
            cout << "dimensión acumulada: " << dimension_matriz << endl;
        //#endif
        }   
    
    // Generación de la matriz inicializando a ceros.
        for(int j=0;j<dimension_matriz;j++){     // Una línea
            fila.push_back(1);
            fila_shadow.push_back(0);
        }

        for (int j=0;j<dimension_matriz;j++){    // La matriz
            matriz_datos.push_back(fila);
            matriz_shadow.push_back(fila_shadow);
        }

       
    #ifdef midebug
            cout << "Matriz creada ........ \nDimensión final de la matriz: " << matriz_datos.size() << endl;
    #endif
    }

    // Certificación de que la matriz tiene la diagonal principal a cero
    void pongo_diagonal_matriz_a_cero(){
        
        for(int x=0;x<dimension_matriz;x++)
        {
           matriz_datos[x][x] =0;
        }
        
    }


    void imprime_matriz(string matriz){

        if (matriz=="datos")
        {
        for(int x=0;x<dimension_matriz;x++)
        {
            cout << endl;
            for (int y=0;y<dimension_matriz;y++)
                cout << matriz_datos[x][y] << " ";
        }
        cout << "\n\n" << endl;
        }
    
        if (matriz=="shadow")
        {
        for(int x=0;x<dimension_matriz;x++)
        {
            cout << endl;
            for (int y=0;y<dimension_matriz;y++)
                cout << matriz_shadow[x][y] << " ";
        }
        cout << "\n\n" << endl;
        }
        
    }

    //Función que escribe en la matriz
    void escribe_en_matriz(int *coordenadas_base,vector <vector <int> > tuplas,string var_cero,string var_uno,bool support)
    {
        //vector<vector<int>>::iterator it;
        
                
        std::vector<vector <int>>::iterator itero_parejas;
        vector<int>::iterator itero_dentro_de_la_pareja;
        int coordenada_final[2];

        if (support)
            {
            cout << "Soy support ...................." << endl;
                            
            // Pongo la pareja de variables a cero, siempre que no estén ya definidas en la matriz shadow
            cout << "Pongo a cero la submatriz: " << var_cero << "["<< coordenadas_base[0] << "] - " << var_uno << "[" << coordenadas_base[1] << "]" << endl;
            for(int i=0;i<rango_variable[var_cero]; i++)
                for (int j=0; j< rango_variable[var_uno];j++)
                {
                    coordenada_final[0]=coordenadas_base[0]+i;
                    coordenada_final[1]=coordenadas_base[1]+j;
                    if(!matriz_shadow[coordenada_final[0]][coordenada_final[1]])
                        matriz_datos[coordenada_final[0]][coordenada_final[1]]=0;
                    else
                        cout << "¡¡Casi la cago!!" << endl;
                        
                    if(!matriz_shadow[coordenada_final[1]][coordenada_final[0]])
                        matriz_datos[coordenada_final[1]][coordenada_final[0]]=0;

                //#ifdef midebug
                    cout << "Coordenada Puesta a \"0\": " << coordenada_final[0] << " - " << coordenada_final[1] << endl;  
                //#endif
                } 
                    
                    // Y ahora las tuplas correspondientes a uno, anotándolo en la matriz shadow
                    
                for (itero_parejas = tuplas.begin() ; itero_parejas != tuplas.end(); ++itero_parejas)
                {    
                    itero_dentro_de_la_pareja=itero_parejas->begin();
                    
                //#ifdef midebug    
                    cout << "Primer valor Tupla: " << *itero_dentro_de_la_pareja << endl;
                //#endif
                    
                    coordenada_final[0]=coordenadas_base[0]+(*itero_dentro_de_la_pareja)-minimo_variable[var_cero];
        
                    itero_dentro_de_la_pareja++;
                //#ifdef midebug    
                    cout << "Segundo valor Tupla: " << *itero_dentro_de_la_pareja << endl;
                //#endif
                    coordenada_final[1]=coordenadas_base[1]+*(itero_dentro_de_la_pareja)-minimo_variable[var_uno];
                    
                    matriz_datos[coordenada_final[0]][coordenada_final[1]]=1;
                    matriz_shadow[coordenada_final[0]][coordenada_final[1]]=1;
                    matriz_datos[coordenada_final[1]][coordenada_final[0]]=1;
                    matriz_shadow[coordenada_final[1]][coordenada_final[0]]=1;
                //#ifdef midebug
                    cout << "Coordenada puesta a \"1\": " << coordenada_final[0] << " - " << coordenada_final[1] << endl;   
                //#endif
                }                
                    
            } 
            else
            {
               
                cout << "Soy una regla Conflict ......" << endl;
                    
                // Escribo las tuplas correspondientes a cero.
                for (itero_parejas = las_tuplas.begin() ; itero_parejas != las_tuplas.end(); ++itero_parejas)
                {
                    itero_dentro_de_la_pareja = itero_parejas->begin();
                    
                //#ifdef midebug    
                    cout << "Primer valor Tupla: " << *itero_dentro_de_la_pareja << endl;
                //#endif
   
                    coordenada_final[0]=coordenadas_base[0]+(*itero_dentro_de_la_pareja)-minimo_variable[var_cero];
            
                    itero_dentro_de_la_pareja++;
                //#ifdef midebug    
                    cout << "Segundo valor Tupla: " << *itero_dentro_de_la_pareja << endl;
                //#endif
                    coordenada_final[1]=coordenadas_base[1]+(*itero_dentro_de_la_pareja)-minimo_variable[var_uno];
                                
                    matriz_datos[coordenada_final[0]][coordenada_final[1]]=0;
                    matriz_shadow[coordenada_final[0]][coordenada_final[1]]=1;
                    matriz_datos[coordenada_final[1]][coordenada_final[0]]=0;
                    matriz_shadow[coordenada_final[1]][coordenada_final[0]]=1;
                            
                //#ifdef midebug
                    cout << "Coordenada puesta a \"0\": " << coordenada_final[0] << " - " << coordenada_final[1] << endl;   
                //#endif    
                    }
                }
    }


    
/* ==========Fin de mis funciones============================================================

=========Comienzo de las funciones que invoca el parser ===================================== */



    void beginInstance(InstanceType type){
    
    #ifdef midebug
        cout << "Empieza Instancia tipo: " << type << " ............" << endl;
    #endif
        

        //XCSP3PrintCallbacks::beginInstance(type);
    }
    

    void endInstance(){
        std::vector<string>::iterator itero;

        cout << "\nLa matriz resultante: " << endl;
        pongo_diagonal_matriz_a_cero();
        
        imprime_matriz("datos");
        cout << "-----------------------------------------------" << endl;
        cout << "-----------------------------------------------" << endl;

        imprime_matriz("shadow");

        for(itero=lista_arrays.begin();itero!=lista_arrays.end();itero++)
        {
            cout << "Array- " << *itero  << endl;
            cout << "Numero variables - " << numero_variable[*itero] << endl;
            cout << "Base dentro de la matriz - " << base_array[*itero] << endl;
            cout << "Valor base del rango - " << minimo_variable[*itero] << endl;
            cout << "Máximo valor del rango - " << rango_variable[*itero] << endl;
            cout << endl;
        }
                
        cout << "Dimension total matriz: " << dimension_matriz << endl;
        cout << endl;
        
        cout << "FIN de la instancia" << endl;

        //XCSP3PrintCallbacks::endInstance();
    }

    
    void beginVariableArray(string id){

        lista_arrays.push_back(id);
        array_actual=id;
        base_array[id]=base_siguiente_array;
        rango_variable[id]=0;
        
        numero_variables=0;
        rango_variables=0;
    
        //XCSP3PrintCallbacks::beginVariableArray(id);
    }
    

    void endVariableArray() {
        
        base_siguiente_array+=(numero_variables*rango_variables);
        numero_variable[array_actual]=numero_variables;
        rango_variable[array_actual]=rango_variables;
        minimo_variable[array_actual]=minimo_variables;

    #ifdef midebug
        cout << "Base siguiente array: " << base_siguiente_array << endl;
        cout << "Número variables: " << numero_variables << " - Rango: " << rango_variables << endl;
    #endif

        //XCSP3PrintCallbacks::endVariableArray();
    }

    void beginVariables() {
    
    #ifdef midebug
        cout << " - Comienza la declaración de variables - " << endl;
    #endif

        //XCSP3PrintCallbacks::beginVariables();
    }
    
    
    void endVariables() {

        //Escribo el fichero .csp
        escribe_nombre_fichero();

        // Genero la matriz
        genera_matriz();
        

    #ifdef midebug
        cout << " - FIN declaración variables - " << endl << endl;
    #endif

        //XCSP3PrintCallbacks::endVariables();
    }
    

    void buildVariableInteger(string id, int minValue, int maxValue) override {
               
        lista_variables.push_back(id);
        rango_variables=(maxValue-minValue)+1;
        minimo_variables=minValue;
        numero_variables++;
        cout << "Variable: " << id << " - min: " << minValue << " - max: " << maxValue << endl;
      
    #ifdef midebug  
        cout << "Array actual " << array_actual << endl;
        cout << "Rango valores: " << rango_variables << " - Instancia Variable: " << numero_variables << " - Mínimo valor Varialbe: " << minimo_variables << endl;
    #endif
    
    
       //XCSP3PrintCallbacks::buildVariableInteger(id,minValue,maxValue);
    }


    void beginConstraints() {
        

        //XCSP3PrintCallbacks::beginConstraints();
    }
    
    
    void endConstraints() {
        

        //XCSP3PrintCallbacks::endConstraints();
    }

    void buildConstraintExtension(string id, vector<XVariable *> list, vector<vector<int>> &tuples, bool support, bool hasStar) {
        
        //XVariable variable = new XVariable("var0");
        string var_cero,var_uno,var_aux;
        int indice0,indice1,indice_aux;
        int coordenadas_base[2];
        vector<vector<int>>::iterator itero_parejas;
        
        cout << "Soy buildConstraintExtension..........................................." << endl;
        
        // Guardo el valor de las tuplas por si es una restricción de grupo y para tener el mismo código en ambos métodos
        las_tuplas.clear();
        
        if (tuples.size()>0)
            for (itero_parejas = tuples.begin() ; itero_parejas != tuples.end(); ++itero_parejas)
            {
                las_tuplas.push_back(*itero_parejas);
            }

        
        cout << "Par de variables: " << (list[0]->id) << " - " << (list[1]->id) << endl; 
        
        indice0=get_indice(*(list[0]));
        indice1=get_indice(*(list[1]));

        var_cero=get_nombre(list[0]->id);
        var_uno=get_nombre(list[1]->id);

        /* if(indice0>indice1)
         {
             //cambiamos el orden para escribir en la misma zona de la matriz
             indice_aux=indice0;
             indice0=indice1;
             indice1=indice_aux;

             var_aux=var_cero;
             var_cero=var_uno;
             var_uno=var_aux;
             
             cout << "Reordeno Variables: " << var_cero << "["<<indice0 <<"] - " << var_uno << "["<<indice1<< "]" <<endl;
         } */
        
        
        if(list.size()>0)
            calcula_coordenadas_base(var_cero,var_uno,indice0,indice1,coordenadas_base);
        
            //calcula_coordenadas_base(*(list[0]),*(list[1]),coordenadas_base);
    
    #ifdef midebug
        cout << "Coordenada base calculada: " << coordenadas_base[0] << " - " << coordenadas_base[1] << endl;
    #endif

        cout << "Tamaño tuplas: " << las_tuplas.size() << endl;

        if (las_tuplas.size()>0)
            escribe_en_matriz(coordenadas_base,las_tuplas,var_cero,var_uno,support);

    #ifdef midebug
        cout << "\n ** Fin buildConstraintExtension ** " << id << endl;
    #endif
        
        //XCSP3PrintCallbacks::buildConstraintExtension(id, list,tuples,support,hasStar);
    }


    void buildConstraintExtensionAs(string id, vector<XVariable *> list, bool support, bool hasStar) {
       
        string var_cero,var_uno,var_aux;
        int indice0,indice1,indice_aux;
        int coordenadas_base[2];
        

        vector<vector<int>>::iterator it;
        vector<int>::iterator ite;

        cout << "Soy buildConstraintExtension  AS ........................................." << endl;

        cout << "Par de variables: " << (list[0]->id) << " - " << (list[1]->id) << endl;

        indice0=get_indice(*(list[0]));
        indice1=get_indice(*(list[1]));

        var_cero=get_nombre(list[0]->id);
        var_uno=get_nombre(list[1]->id);

        /* if(indice0>indice1)
         {
             //cambiamos el orden para escribir en la misma zona de la matriz
             indice_aux=indice0;
             indice0=indice1;
             indice1=indice_aux;

             var_aux=var_cero;
             var_cero=var_uno;
             var_uno=var_aux;

             cout << "Reordeno Variables: " << var_cero << "["<<indice0 <<"] - " << var_uno << "["<<indice1<< "]" <<endl;
         } */

        if(list.size()>0)
            calcula_coordenadas_base(var_cero,var_uno,indice0,indice1,coordenadas_base);
        
            //calcula_coordenadas_base(*(list[0]),*(list[1]),coordenadas_base);

    
    #ifdef midebug
        cout << "Coordenada base calculada: " << coordenadas_base[0] << " - " << coordenadas_base[1] << endl;
    #endif    

    
        cout << "Tamaño tuplas: " << las_tuplas.size() << endl;

        if (las_tuplas.size()>0)
            escribe_en_matriz(coordenadas_base,las_tuplas,var_cero,var_uno,support);

    #ifdef midebug
        cout << "\n ** Fin buildConstraintExtensionAS ** " << id << endl;
    #endif


        //XCSP3PrintCallbacks::buildConstraintExtensionAs(id,list,support,hasStar);
    }


    void beginGroup(string id)  {
    
    #ifdef midebug
            cout << "Comienzo Grupo ....... " << id <<endl;
    #endif
    
            //XCSP3PrintCallbacks::beginGroup(id);
         }
    
    
    void endGroup() {
    
    #ifdef midebug
            cout << "Grupo FIN .......\n\n " <<endl;
    #endif
        
            //XCSP3PrintCallbacks::endGroup();
        }

};


int main(int argc,char **argv) {
    MiSolverPrintCallbacks miparser;
    char *nombre_fichero_dimacs;
    
            
   
    if(argc!=2){ 
        throw std::runtime_error("usage: ./csp xcsp3instance.xml");
        return 0;
    }

    miparser.set_nombre_fichero(argv[1]);
    
    
  
    try
    {
        XCSP3CoreParser parser(&miparser);
        parser.parse(argv[1]); // fileName is a string
    }
    catch (exception &e)
    {
        cout.flush();
        cerr << "\n\tUnexpectedd exxception: \n";
        cerr << "\t" << e.what() << endl;
        exit(1);
    }

    // Una vez leido el fichero y generada la matriz, se vuelca en un Grafo y se resuelve
    // Habrá que incluir aquí la generación del fichero Dimacs

    ugraph ug(miparser.dimension_matriz);

    for(int i=0;i<miparser.dimension_matriz;i++)
        for(int j=i+1;j<miparser.dimension_matriz-1;j++)
            if(miparser.matriz_datos[i][j]==1){
                ug.add_edge(i,j);
                //cout << "Añado edge(" << i << "," << j << ")" << endl;
            }
    
    
        
    nombre_fichero_dimacs=strrchr(miparser.nombre_fichero,'.');
    strcpy(nombre_fichero_dimacs,".clq");

    cout << "Nombre dimacs: " << miparser.nombre_fichero << endl;
    
    std::fstream f(miparser.nombre_fichero,ios::out);            
    ug.write_dimacs(f);
    f.close();

    /*  clqo::param_t parametros;
	parametros.alg=clqo::BBMCXR_L;
	parametros.init_preproc=clqo::init_preproc_t::UB;
	CliqueAll cug(&ug, parametros);
	cug.set_up();
	cug.run(); 	 */	


    return 0;
}

