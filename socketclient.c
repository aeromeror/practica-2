#include <stdio.h>
#include <sys/types.h>          
#include <sys/socket.h>
#include <arpa/inet.h>
#include <strings.h>
#include <sys/types.h>         
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>

#define PORT 3535
#define NAME_LEN 32
#define TYPE_LEN 32
#define BREED_LEN 16

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

int clientfd,check;
struct sockaddr_in server;


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

void ingresarReg(){   
    
    struct dogType* perro;//creación del perro a ingresar
    perro = (struct dogType*)malloc(sizeof(struct dogType));
    if(perro == NULL){
        perror("error al asignar el espacio de la esctructura perro");
        exit(-1);
    }

    system("clear");
    char c;
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
    perro->idPrev = 0;
    int r = send(clientfd,perro,sizeof(struct dogType),0);
    if(r==-1){
        perror("error send");
        exit(-1);
    }
    int val;
    r = recv(clientfd, &val, sizeof(int),0);
    if( r != sizeof(int) )
	{
		perror("Error reciviendo conf");
		exit(-1);
	}
    if(val==1)
    {
        printf("comfirmado");
    }
}

void verReg(){
    system("clear");
    struct dogType* perro;
    perro = (struct dogType*)malloc(sizeof(struct dogType));
    if(perro == NULL){
        perror("error al asignar el espacio de la esctructura perro");
        exit(-1);
    }
    int tot;
    int r = recv(clientfd,&tot, sizeof(int),0);
	if( r != sizeof(tot) )
	{
		perror("Error reciviendo numero de registro");
		exit(-1);
	}
    printf("Numero total de registros: %i \n",tot);

    printf("Ingrese el numero de regitro(Id) del perro:\n");
    int val;
    scanf("%i",&val);
    r=send(clientfd,&val,sizeof(int),0);
    if( r != sizeof(val) )
	{
		perror("Error enviando reg val");
		exit(-1);
	}
    int reg;
    r = recv(clientfd,&reg, sizeof(int),0);
	if( r != sizeof(reg) )
	{
		perror("Error reciviendo numero de registro");
		exit(-1);
	}
    printf("registro numero : %d \n",reg);
    r = recv(clientfd, perro, sizeof(struct dogType),0);
	if( r != sizeof(struct dogType) )
	{
		perror("Error reciviendo mascota del cliente");
		exit(-1);
	}
    printReg(perro);
    char tecla;
    printf("si desea modificar el historial clinico presione m(modificar)+ enter \n");
    scanf(" %c",&tecla);
    int flag;
    if(tecla=='m')
    {
        flag = 1;
        r=send(clientfd,&flag,sizeof(int),0);
        if( r != sizeof(flag) )
	    {
		    perror("Error enviando reg val");
		    exit(-1);
	    }
        char hcFile [20];
        r = recv(clientfd,hcFile , sizeof(hcFile ),0);
	    if( r != sizeof(hcFile ) )
	    {
		    perror("Error reciviendo mascota del cliente");
		    exit(-1);
	    }
        system(hcFile);

    }else{
        flag = 0;
        r=send(clientfd,&flag,sizeof(int),0);
        if( r != sizeof(flag) )
	    {
		    perror("Error enviando reg val");
		    exit(-1);
	    }
    }
}

void buscarReg(){
    int r;
    system("clear");
    struct dogType* perro;
    perro = (struct dogType*)malloc(sizeof(struct dogType));
    if(perro == NULL){
        perror("error al asignar el espacio de la esctructura perro");
        exit(-1);
    }
    printf("Ingresa el nombre del perro a buscar:");
    char nombre[NAME_LEN];
    memset(nombre,0,sizeof(nombre));
    scanf("%s",nombre);
    r = send(clientfd,nombre,sizeof(nombre),0);
    if( r != sizeof(nombre) )
	{
		perror("Error enviando reg val");
		exit(-1);
	}
    int id_searched;
	while(1){
		r = recv(clientfd,&id_searched,sizeof(int),0);
		if(r == 0){
			perror("error por el connect");
			exit(-1);
		}

		if(id_searched == -1){
			break;
		}else{
			r = recv(clientfd,perro,sizeof(struct dogType),0);
			if(r == 0){
				perror("error por el connect");
				exit(-1);
			}
			printReg(perro);
		}
	} 
	free(perro);
}

void run(){
    while(1)
    {
        //menu();
        int opcion;
        printf("\n\t\tLa veterinaria de sizz\n");
        printf("Ingresa una opción del menú: \n");
        printf("1: Ingresar registro\n");
        printf("2: Ver registro\n");
        printf("3: Borrar registro\n");
        printf("4: Buscar registro\n");
        printf("5: Salir\n");
        scanf(" %d",&opcion);
        switch (opcion){
        case 1:
                check=send(clientfd,&opcion,sizeof(opcion),0);
                if(check==-1){
                    perror("error send de 1");
                    exit(-1);
                }
                ingresarReg();
                break;
        case 2:
                check=send(clientfd,&opcion,sizeof(opcion),0);
                if(check==-1){
                    perror("error send de 1");
                    exit(-1);
                }
                verReg();
                break;
        case 3:
                check=send(clientfd,&opcion,sizeof(opcion),0);
                if(check==-1){
                    perror("error send de 1");
                    exit(-1);
                }
            //borrarReg();
            break;
        case 4:
            check=send(clientfd,&opcion,sizeof(opcion),0);
                if(check==-1){
                    perror("error send de 1");
                    exit(-1);
                }
            buscarReg();
            break;
        case 5:
            check=send(clientfd,&opcion,sizeof(opcion),0);
                if(check==-1){
                    perror("error send de 1");
                    exit(-1);
                }
            exit(0);
            break;
        default:
            printf("Ingresa una opción valida (1-5)\n");
            break;
    }
    }
}


int main(){
    
    
    clientfd=socket(AF_INET,SOCK_STREAM,0);
    if(clientfd==-1){
        perror("error apertura socket");
        exit(-1);
    }
    server.sin_family=AF_INET;
    server.sin_port=htons(PORT);
    server.sin_addr.s_addr=inet_addr("127.0.0.1");
    bzero(server.sin_zero,8);
    int len = sizeof(struct sockaddr_in);
    int r;
    r=connect(clientfd,(struct sockaddr *)&server,len);
    if(r==-1){
        perror("error al conectar con servidor");
        exit(-1);
    }
    run();

    return 0;
}