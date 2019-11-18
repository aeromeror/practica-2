#include <stdio.h>
#include <wchar.h>
#include <locale.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <unistd.h>

#define NAME_LEN 32
#define TYPE_LEN 32
#define BREED_LEN 16
#define HASH_SIZE 10007 //hash de al menos 1000 entradas
#define MOD 256
FILE* principal;
int* lastestId;

struct dogType{
    char nombre[NAME_LEN];
    char tipo[TYPE_LEN];
    int edad;
    char raza[BREED_LEN];
    int estatura;
    float peso;
    char sexo;
    int idPrev;
};



/***********************************FUNCIONES AUXILIARES******************************************/
//función hash

    int hash(unsigned char *str){
    int hash, base, i, auxC;

        for( i = 0, hash = 0, base = 1; i < NAME_LEN; ++ i )
        {
                auxC = (int) str[i];
                if( auxC >= 'A' && auxC <= 'Z' ) auxC = 'a' + ( auxC - 'A' );

                hash += auxC * base;

                base *= MOD;
                base %= HASH_SIZE;

                hash %= HASH_SIZE;
        }

        return hash;


}

//posición del último registro = cantidad de registros
int lastPos(){
   int size;
   FILE* handler = fopen("dataDogs.dat", "r");
   if (handler){
       fseek(handler,0,SEEK_END);//buscar el último byte del archivo
       size = ftell(handler);//desplazamiento del primer al último byte(filesize)
       fclose(handler);
    }
    return size / sizeof(struct dogType);
}

//imprimir el registro de un perro
void printReg(struct dogType * reg){
	printf("Nombre: %s\n",reg->nombre);
	printf("Tipo: %s\n",reg->tipo);
	printf("Edad: %d\n",reg->edad);
	printf("Raza: %s\n",reg->raza);
	printf("Estatura: %d\n",reg->estatura);
	printf("Peso: %f\n",reg->peso);
	printf("Sexo: %c\n",reg->sexo);
	printf("\n\n");
}

//obtener (leer) el registro de un perro
void getReg(int reg, struct dogType* perro){
    FILE* aux;
    int check;

    aux = fopen("dataDogs.dat","r");
    if(aux == NULL){
        perror("error al abrir el archivo principal");
        exit(-1);
    }

    check = fseek(aux,reg*sizeof(struct dogType),SEEK_SET);
    if(check == -1){
        perror("error al buscar la posición del perro");
        exit(-1);
    }
    check = fread(perro,sizeof(struct dogType),1,aux);
	if (check == 0){
		perror("error al leer el perro que se busca");
		exit(-1);
	}		
	fclose(aux);
}

//funcion usada para guardar un perro en la ultima posición del datadogs
void save(struct dogType* ap){

    FILE* fd = fopen("dataDogs.dat", "a+"); 
    if(fd==NULL){
        perror("error abriendo el archivo principal");
        exit(-1);
    }   
    struct dogType* nuevoPerro = ap;
    int r;
    r = fseek(fd,0L,SEEK_END);//busca el ultimo byte en el archivo abierto
	if(r == -1){
		perror("error al buscar la posicion para añadir el perro");
		exit(-1);
	}
	r = fwrite(nuevoPerro,sizeof(struct dogType),1,fd);
	if(r == 0){
		perror("error al añadir el nuevo perro");
		exit(-1);
	}
    fclose(fd);
}




/*********************************INGRESAR********************************************/
void ingresarReg(){
    system("clear");

    char c;
    struct dogType* perro;//creación del perro a ingresar
    perro = (struct dogType*)malloc(sizeof(struct dogType));
    if(perro == NULL){
        perror("error al asignar el espacio de la esctructura perro");
        exit(-1);
    }

    //se llenan los datos del perro 
    printf("Ingresa los datos del perro\n");
    memset(perro->nombre,0,NAME_LEN*sizeof(char));
    printf("\nNombre: ");
    scanf("%s",perro->nombre);
    printf("\nTipo: ");
    scanf("%s",perro->tipo);
    printf("\nEdad: ");
    scanf("%i",&perro->edad);
    printf("\nRaza: ");
    scanf("%s",perro->raza);
    printf("\nEstatura: ");
    scanf("%i",&perro->estatura);
    printf("\nPeso: ");
    scanf("%f",&perro->peso);
    printf("\nSexo(M/F): ");
    scanf(" %c",&perro->sexo);
    printf("\n");


    int key = hash(perro->nombre);
    perro->idPrev = lastestId[key];
    lastestId[key] = lastPos();
    save(perro);
    printf("se ingreso el nuevo perro exitosamente con el número de registro: #%d",lastPos());

    //presionar cualquier tecla para continuar
    printf("\nse debe presionar cualquier tecla para continuar");
	char tecla;
	scanf(" %c",&tecla);
    free(perro);
}




/*********************************VER********************************************/
void verReg(){
    system("clear");
    printf("numero total de registros: %i\n",lastPos());	

    struct dogType* perro;//creación del perro a ver
    perro = (struct dogType*)malloc(sizeof(struct dogType));
    if(perro == NULL){
        perror("error al asignar el espacio de la esctructura perro");
        exit(-1);
    }

    printf("Ingrese el numero de regitro(Id) del perro:\n");
    int numId;
    scanf("%d",&numId);
    numId--;//indexacion
    getReg(numId,perro);		
    printReg(perro);

    char hcName [20];//hc stands for historia clílica
    memset(hcName,0,sizeof(hcName));
    char hcFile [20];
    memset( hcFile,0, sizeof (hcFile));

    sprintf(hcName,"%d",numId);
    strcat(hcName,"hc");
    strcat(hcName,".txt");//para nombrar el hc
    strcat(hcFile,"gedit ");//para editar el txt
    strcat(hcFile, hcName);	

    if(access(hcName,F_OK) == -1){//valida si no existe el archivo

        //generamos la historia clinica del perro
	    FILE *hc = fopen(hcName,"w+");
	    int check;

	    check = fprintf(hc,"Nombre: %s\n", perro->nombre);
	    if (check < 0){
		    perror("error generando historia clinica");
		    exit(-1);
	    }

	    check = fprintf(hc,"Tipo: %s\n", perro->tipo);
	    if (check < 0){
		    perror("error generando historia clinica");
		    exit(-1);
	    }

	    check = fprintf(hc,"Edad: %d\n", perro->edad);
	    if (check < 0){
    		perror("error generando historia clinica");
		    exit(-1);
	    }
	    
        check = fprintf(hc,"Raza: %s\n", perro->raza);
	    if (check < 0){
    		perror("error generando historia clinica");
		    exit(-1);
	    }
	    
        check = fprintf(hc,"Estatura: %d\n", perro->estatura);
	    if (check < 0){
    		perror("error generando historia clinica");
		    exit(-1);
	    }
	    
        check = fprintf(hc,"Peso: %f\n", perro->peso);
	    if (check < 0){
    		perror("error generando historia clinica");
		    exit(-1);
	    }
	    check = fprintf(hc,"Sexo: %s\n", &perro->sexo);
	    if (check < 0){
    		perror("error generando historia clinica");
    		exit(-1);
    	}
    
    	check = fprintf(hc,"Historia clinica: \n");
    	if (check < 0){
		    perror("error generando historia clinica");
		    exit(-1);
	    }
	    fclose(hc);
    }

    char tecla;
    printf("si desea modificar el historial clinico presione m(modificar) \n");
    scanf(" %c",&tecla);
    if(tecla=='m')
    system(hcFile);
    free(perro);
    //presionar cualquier tecla para continuar
    printf("\nse debe presionar cualquier tecla para continuar");
    scanf(" %c",&tecla);
    free(perro);
}




/*********************************BORRAR********************************************/
void borrarReg(){

    system("clear");
    printf("numero total de registros: %i\n",lastPos());
    printf("Ingrese el id de la mascota que desea eliminar:\n");
    int regId;
    scanf("%d",&regId);
    regId--;//indexacion desde 0

    struct dogType* Final;
    Final = (struct  dogType*)malloc(sizeof(struct dogType));	
    if( Final == NULL ){
	    perror("error en el malloc de la mascota");
	    exit( -1 );
    }

    getReg(lastPos()-1,Final);
    struct dogType * Delete;
    Delete = (struct  dogType*)malloc(sizeof(struct dogType));	
    if(Delete == NULL){
	    perror("error en el malloc de la mascota");
	    exit(-1);
    }

    getReg(regId,Delete);

    int hashF, hashD;
    hashF = hash(Final->nombre);
    hashD = hash(Delete->nombre);
    struct dogType* tmp;
    tmp = (struct  dogType*)malloc(sizeof(struct dogType));	
    if(tmp == NULL){
	    perror("error en el malloc de la mascota");
	    exit(-1);
    }

    int id,idAnt;
    int idDesp = -1;
    getReg(lastestId[hashF],tmp);
    while(1){//encontrar el elemento con nombre igual al ultimo que tiene posicion > a el registerId mas cercana 
	
        idAnt = tmp->idPrev;// idAnt sera el anterior a iddesp 
	    if(tmp->idPrev <= regId){
            break;
        }
	    idDesp = idAnt;
	    getReg(tmp->idPrev,tmp);
    }

    if(id != -1){//caso en que no existe siguiente el eliminado es el ultimo
	    getReg(id,tmp);
	    tmp->idPrev = regId; // ahora tmp guarda la data completa de dicho elemento
    }

    lastestId[hashF] = Final -> idPrev;//mover el ultimo elemento a la posicion que se va a eliminar entonces el ultimo sera el anteriror
    Final->idPrev = idAnt;//final va a estar entre el Ant y el sig  
     
    //ahora arreglar id prevs de el valor hash de el que se va a eliminar  
    struct dogType * tmp2;
    tmp2 = (struct dogType*)malloc(sizeof(struct dogType));	
    if(tmp2 == NULL){
	    perror("error en el malloc de la mascota");
	    exit(-1);
    }
     int idAnt2;
     int idDesp2 = -1;
     //--------------------------------terminar
    if(hashF != hashD){	
	    getReg(lastestId[hashD],tmp2);
	    while(1){
		    if(tmp2->idPrev <= regId){
                break;
            }
		idDesp2 = tmp2 -> idPrev;
		getReg(tmp2->idPrev,tmp2);
	    }		
	    if(idDesp2 != -1){
		    getReg( idDesp2, tmp2 );
		    tmp2->idPrev = Delete->idPrev; //ahora tmp guarda la data completa del elemento idDesp2
	    }else{

		//el elemento a ser borrado es el ultimo de su lista, por tanto
		    lastestId[hashD] = Delete->idPrev;
    	}   
    }else{
	    Final -> idPrev = Delete -> idPrev;
    }

    FILE* dbTmp;
    int check;
    dbTmp = fopen("dbTmp.dat", "a+"); 

    for(int i = 0; i<lastPos()-1; ++i){
		if(i == idDesp){
			check = fwrite(tmp,sizeof(struct dogType),1,dbTmp);	
			if(check==0){
				perror("error escritura dbTmp");
				exit(-1);
			}
		}else if(i == idDesp2){
			check = fwrite(tmp2,sizeof(struct dogType),1,dbTmp);
			if (check==0){
				perror("error escritura dbTmp");
				exit(-1);
			}	
		}else if(i == regId){
			check = fwrite ( Final, sizeof( struct dogType ), 1, dbTmp );
			if (check==0){
				perror("error escritura dbTmp");
				exit(-1);
			}
		}else{
			getReg(i, Delete);
			check = fwrite(Delete,sizeof(struct dogType),1,dbTmp); 
			if(check==0){
				perror("error escritura dbTmp");
				exit(-1);
			}
		}
	}

	fclose(dbTmp); 
    int status;
	status = remove("dataDogs.dat");	
	if(status != 0){
		perror("Error eliminando la base de datos");
		exit( -1 );
	}

	status = rename("dbTmp.dat","dataDogs.dat");
	if(status != 0){
		perror("Error en el renombramiento de la base de datos");
		exit(-1);
	}

	free(Final);
	free(Delete);
	free(tmp);
	free(tmp2);

	printf("\nLa mascota ha sido eliminada, ingrese un caracter continuar");
	char tecla;
	scanf("%s",&tecla);	
}




/*********************************BUSCAR********************************************/
void buscarReg(){
    system("clear");
    printf("Ingresa el nombre del perro a buscar:");
    struct dogType* perro;//creación del perro a ingresar
    perro = (struct dogType*)malloc(sizeof(struct dogType));
    if(perro == NULL){
        perror("error al asignar el espacio de la esctructura perro");
        exit(-1);
    }
    char nombre[NAME_LEN];
    memset(nombre,0,sizeof(nombre));
    scanf("%s",nombre);
    int key = hash(nombre); 
    int lastId =lastestId[key];
    int count=0;
    while( lastId != -1 ){//recorremos todas las estructuras con esa key, hasta que esa posición este con el valor inicial de la hash(-1)
        getReg(lastId, perro);
        if(strcasecmp(nombre,(perro->nombre))==0){
            printf("\n");
            printf("%d\n",++lastId);
            printReg(perro);
            count ++; 
        }
        lastId = perro->idPrev;
    }
    printf(" total encontrados %i",count);
    free(perro);
    printf("\nBusqueda terminada, presiona cualquier tecla para continuar");
    char tecla;
    scanf("%s",&tecla);
}

/*********************************MENÚ********************************************/
void menu(){
    char opcion;
    setlocale(LC_ALL, "");
    printf("\n\t\tLa veterinaria de sizz\n");
    printf("Ingresa una opción del menú: \n");
    printf("1: Ingresar registro\n");
    printf("2: Ver registro\n");
    printf("3: Borrar registro\n");
    printf("4: Buscar registro\n");
    printf("5: Salir\n");
    scanf(" %c",&opcion);
    switch (opcion){
        case '1':
            ingresarReg();
            break;
        case '2':
            verReg();
            break;
        case '3':
            borrarReg();
            break;
        case '4':
            buscarReg();
            break;
        case '5':
            exit(0);
            break;
        default:
            printf("Ingresa una opción valida (1-5)\n");
            break;
    }
}




/**********************************MAIN*******************************************/
int main(){
    
    FILE* principal;
	principal=fopen("dataDogs.dat", "r+");//abrir el archivo generado
	if(principal == NULL){
		perror("error abriendo archivo dataDogs");
		exit(-1);
	}

    
	lastestId=malloc(HASH_SIZE*sizeof(int));
    if(lastestId == NULL){
        perror("error en el malloc de la hash");
        exit(-1);
    }
    memset(lastestId,-1,HASH_SIZE*sizeof(int));

	FILE *points;
    int r;
	points = fopen("dataPointers.dat","r");
	if(points == NULL){
		perror("error abriendo los datos de la hash");
		exit(-1);
	}
    fread(lastestId,HASH_SIZE*sizeof(int),1,points);
    for(;;){menu();}
    return 0;
}
