#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define REGISTER_SIZE (int)1e7  //10'000.000 de estructuras a almacenar en el archivo generado
#define NAME_LEN 32
#define TYPE_LEN 32
#define BREED_LEN 16
#define TOTAL_NAMES 1717 
#define TOTAL_BREEDS 116
#define TOTAL_TYPES 13
#define HASH_SIZE 10007 //hash de al menos 1000 entradas 
#define MOD 256

struct dogType{//se declaran los atributos de la estructura segun los requisitos
	char nombre [NAME_LEN];
    char tipo [TYPE_LEN];
    int edad;
	char raza [BREED_LEN];
    int estatura;
    float peso;
   	char sexo;
	int idPrev;
};

//función hash
int hash(unsigned char *nombre){
    int hash, base, i, auxC;

        for( i = 0, hash = 0, base = 1; i < NAME_LEN; ++ i )
        {
                auxC = (int) nombre[i];
                if( auxC >= 'A' && auxC <= 'Z' ) auxC = 'a' + ( auxC - 'A' );

                hash += auxC * base;

                base *= MOD;
                base %= HASH_SIZE;

                hash %= HASH_SIZE;
        }

        return hash;
}

int main(){
    FILE* principal;
	principal=fopen("dataDogs.dat", "w+");//abrir el archivo para escribir (acá estaran las estructuras)
	if(principal == NULL){
		perror("error abriendo archivo dataDogs");
		exit(-1);
	}

//***********************RESERVAR ESPACIO EN MEMORIA**********************************//

//------------------------------------------------------------------------------------
    //iniciamos los apuntadores que corresponden a los nombres
    char** Nombres;//apuntador a cada nombre que es un apuntador(arreglo)
	Nombres = (char**)malloc(TOTAL_NAMES*sizeof(char*));//reservar espacio para los apuntadores a cada nombre  
	if(Nombres ==  NULL){
		perror("error en el malloc de nombres");	
		exit(-1);
	}
	for(int i=0;i<TOTAL_NAMES;i++){ //dejaremos en memoria el archivo a sobreescribir
	    Nombres[i]=	(char*)malloc(NAME_LEN*sizeof(char));//a cada "sub-apuntador" se le asignara el espacio para un nombre
	    if(Nombres[i]){
		    memset(Nombres[i],0,NAME_LEN*sizeof(char));//llenar con 0s el espacio de cada nombre
	    }
	}
//------------------------------------------------------------------------------------
    //iniciamos los apuntadores que corresponden a las razas
	char** Razas;
	Razas = (char**) malloc(TOTAL_BREEDS*sizeof(char*));
	if(Razas == NULL){
        perror("error en el malloc de razas");	
		exit(-1);
	}
	for(int i=0;i<TOTAL_BREEDS;i++){
	   	Razas[i] = (char*)malloc(BREED_LEN*sizeof(char));
	    if(Razas[i]){
		    memset(Razas[i],0,BREED_LEN*sizeof(char));
	    }
	}
//------------------------------------------------------------------------------------
    //iniciamos los apuntadores que corresponden a los tipos
	char** Tipos;
	Tipos = (char**)malloc(TOTAL_TYPES*sizeof(char*));
	if(Tipos == NULL){
		perror("error en el malloc de tipos");	
		exit(-1);
	}
	for(int i=0;i<TOTAL_TYPES;i++){
		Tipos[i] = (char*)malloc(TYPE_LEN*sizeof(char));
		if(Tipos[i]){
		    memset(Tipos[i],0,TYPE_LEN*sizeof(char));
		} 
	}
//------------------------------------------------------------------------------------
	int* lastestId; //actuara como nuestra tabla hash
	lastestId = (int*)malloc(HASH_SIZE*sizeof(int));//reservar espacio para la hash
	if(lastestId == NULL){
		perror("error en el malloc de la tabla hash");
	}
	memset(lastestId,-1,HASH_SIZE*sizeof(int));



//***********************PASO DE DATOS A MEMORIA**********************************//
//------------------------------------------------------------------------------------
	 //archivo sobre el cual leeremos los nombres
    char* nombresMascotas = "petNames.dat";
    FILE* nombresm = fopen(nombresMascotas,"r");//apertura archivo con nombres para lectura
	if(nombresm == NULL){
		perror("error al abrir el dat de los nombres");
		exit(-1);
	}
    char bufferName[NAME_LEN];//en este string guardaremos el nombre leido
	for(int i=0;i<TOTAL_NAMES;i++){//recorremos el archivo que tiene los nombres cada buff*i(cada nombre)
	    int check = fseek(nombresm,sizeof(bufferName)*i,SEEK_SET); 
        //sitúa el puntero de lectura/escritura de un archivo en la posición indicada
	    if(check == -1){
		    perror("error al ubicar el puntero en un nuevo nombre");
		    exit(-1);
	    }
	    check = fread(&bufferName,sizeof(bufferName),1,nombresm);
        //en la dirección de memoria de nuestro string guardamos el nombre leido
	    if(check == 0){
		    perror("error al leer el nombre");
		    exit(-1);
	    }
	    strcpy(Nombres[i],bufferName);
   	}
	fclose(nombresm);

//------------------------------------------------------------------------------------
    //leemos las razas para pasarlas al espacio de memoria que les reservamos, 
    //de igual modo que como lo hicimos con los nombres
    char* nombresRazas = "breedNames.dat";
	FILE* razas = fopen(nombresRazas,"r");
	if(razas == NULL){
		perror("error al abrir el dat de las razas");
		exit(-1);
	}
	char bufferBreed[BREED_LEN];
	for(int i=0;i<TOTAL_BREEDS;i++){
	    int check = fseek(razas,sizeof(bufferBreed)*i*2,SEEK_SET);
	    if(check == -1){
		    perror("error al ubicar el puntero en una nueva raza");
		    exit(-1);
		}
	    check = fread(&bufferBreed,sizeof(bufferBreed),1,razas);
	    if(check == 0){
		    perror("error al leer la raza");
		    exit(-1);
	    }
	    strcpy(Razas[i],bufferBreed);
    }
	fclose(razas);
	
//------------------------------------------------------------------------------------
    //leemos los tipos para pasarlos al espacio de memoria que les reservamos, 
    //de igual modo que como lo hicimos con los nombres
    char* nombresTipos = "typeNames.dat";
	FILE* tipos = fopen(nombresTipos,"r");
	if(tipos == NULL){
		perror("error al abrir el dat de los tipos");
	}
	char bufferType[TYPE_LEN];
	for(int i=0;i<TOTAL_TYPES;i++){
	    int check = fseek(tipos,sizeof(bufferType)*i,SEEK_SET);
	    if(check == -1){
		    perror("error al ubicar el puntero en un nuevo tipo");
		    exit(-1);
	    }
	    check = fread(&bufferType,sizeof(bufferType),1,tipos);
		if(check == 0){
		    perror("error al leer el tipo");
		    exit(-1);
	    }
	    strcpy(Tipos[i], bufferType);
    }
	fclose(tipos);




//**********************************GENERACIÓN DE REGISTROS****************************************//
//------------------------------------------------------------------------------------
	struct dogType* perro;
	perro = (struct dogType*)malloc(sizeof(struct dogType));
    srand(time(NULL));
    
	if(perro == NULL){
		perror("error en el malloc de la estructura a genenerar");
	}
	for(int i=0;i<REGISTER_SIZE;i++){
		int r = rand();
	    memset(perro->nombre,0,sizeof(perro->nombre));
	    strcpy(perro->nombre,Nombres[r%TOTAL_NAMES]);
	    strcpy(perro->tipo,Tipos[r%TOTAL_TYPES]);		
	    perro->edad = r%18; 	
	    strcpy(perro->raza,Razas[r%TOTAL_BREEDS]);
	    perro->estatura = r%51; 
	    perro->peso = 20.0*(((float)r/RAND_MAX) + 1);
        perro->sexo = (r%2==0)?'H':'M';
        //llenamos los datos de nuestro nuevo perro de forma aleatoria
		
	    int key = hash(perro->nombre);
	    perro->idPrev = lastestId[key]; //asignamos su valor correspondiente en la tabla
	    lastestId[key] = i;

	    //guardar en archivo principal
	    int ch;
	    ch = fwrite(perro,sizeof(struct dogType),1,principal);
	    if(ch == 0){
		    perror("error en la escritura en el archivo principal");
		    exit(-1);
	    }
    }
		
//------------------------------------------------------------------------------------	
	FILE *points;
	points = fopen("dataPointers.dat","w+"); //abrimos un archivo para escritura, este tendra los valores de nuestra tabla hash(nombres)
	if(points == NULL){
		perror("error generando apuntadores");
		exit(-1);
	}
	fwrite(lastestId,HASH_SIZE*sizeof(int),1,points);
	fclose(points);
	//printf("%d\n",lastestId[rand()%HASH_SIZE]); //verificación de que se escribio la hash correctamente

    fclose(principal);
	return 0;
}
