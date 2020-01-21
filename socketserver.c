#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <arpa/inet.h> 
#include <semaphore.h> 
#include <fcntl.h>

#define HASH_SIZE 10007 //hash de al menos 1000 entradas
#define NAME_LEN 32
#define TYPE_LEN 32
#define BREED_LEN 16
#define PORT 3535
#define BACKLOG 2
#define MOD 256
#define LOG_SIZE 32
#define MAX_CLIENTS 32

struct in_addr ipServer;
struct sockaddr_in server,client[MAX_CLIENTS];
int clientfd[MAX_CLIENTS];
pthread_t idThreat[MAX_CLIENTS];
FILE* principal;
int* lastestId;
int arg[MAX_CLIENTS];
sem_t * semaforo;
char operation[LOG_SIZE];
char type[LOG_SIZE];


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

struct logRegister{
	char date[LOG_SIZE];
	char client[LOG_SIZE];
	char operation[LOG_SIZE];
	char type[LOG_SIZE];
};



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

void SendMascota( struct dogType * mascota,int idc )
{
	sem_wait(semaforo);
	int r = send(clientfd[idc], mascota, sizeof(struct dogType), 0);
	if( r != sizeof(struct dogType) )
	{
		perror("Error enviando mascota el cliente");
		exit(-1);
	}
	sem_post(semaforo);
}
void SendConfirmation(int confirmation ,int idc) 
{	
	sem_wait(semaforo);
	int r = send(clientfd[idc], &confirmation, sizeof(confirmation), 0);
	if( r != sizeof(confirmation) )
	{
		perror("Error enviando confirmacion al cliente");
		exit(-1);
	}
	sem_post(semaforo);
}

void addLog(char * operation,int clientIp, char * type){
	struct logRegister * log;
    log = (struct logRegister*)malloc (sizeof(struct logRegister));
    if(log == NULL){
            perror("error en el malloc del log");
            exit( -1 );
    }

    time_t t = time(NULL);
    struct tm tm = *localtime(&t);


   	char actualTime [LOG_SIZE];        
	memset( actualTime, 0, LOG_SIZE * sizeof(char));
    char times [LOG_SIZE];
    memset(times, 0, LOG_SIZE * sizeof(char));

    strcat(actualTime, "|");
    sprintf(times, "%d", tm.tm_year + 1900);
    strcat(actualTime, times);
    strcat(actualTime, "|");
    sprintf(times, "%d",tm.tm_mon + 1);
    strcat(actualTime, times);
    strcat(actualTime, "|");
    sprintf(times, "%d",tm.tm_mday);
    strcat(actualTime, times);
    strcat(actualTime, "|");
    sprintf(times, "%d",tm.tm_hour);
    strcat(actualTime, times);
    strcat(actualTime, "|");
    sprintf(times, "%d",tm.tm_min);
    strcat(actualTime, times);
    strcat(actualTime, "|");
    sprintf(times, "%d",tm.tm_sec);
    strcat(actualTime, times);
    strcat(actualTime, "|");
	
	memset(log->date, 0, LOG_SIZE);
    strcpy(log->date, actualTime);


    char ipAddr[INET_ADDRSTRLEN];
	inet_ntop( AF_INET, &(client[clientIp].sin_addr), ipAddr, INET_ADDRSTRLEN);
        
	memset(log->client, 0, LOG_SIZE);	

	strcpy(log->client, ipAddr);

	memset(log->operation,0,LOG_SIZE);
	memset(log->type, 0, LOG_SIZE);

        strcpy(log->operation, operation);
        strcpy(log->type, type);

        FILE * logFile = fopen("dataDogs.log","a+");
        if( logFile == NULL ){
		perror("error abriendo archivo log");
		exit( -1 );
	}

	int w = fwrite(log,sizeof(struct logRegister),1,logFile);	
	if (w==0){
		perror("error escritura logData");
		exit(-1);
	}
	fclose(logFile);
	free(log);

}



void verReg(int idc){
    struct dogType* perro;//creación del perro a ver
    perro = (struct dogType*)malloc(sizeof(struct dogType));
    if(perro == NULL){
        perror("error al asignar el espacio de la esctructura perro");
        exit(-1);
    }
    int tot=lastPos();
	sem_wait(semaforo);
    int r=send(clientfd[idc],&tot,sizeof(int),0);
    if( r != sizeof(tot) )
	{
		perror("Error enviando numreg");
		exit(-1);
	}
	sem_post(semaforo);
    int reg;
	sem_wait(semaforo);
    r = recv(clientfd[idc],&reg, sizeof(int),0);
	if( r != sizeof(reg) )
	{
		perror("Error reciviendo numero de registro");
		exit(-1);
	}
    sem_post(semaforo);
    int val=reg;
	sem_wait(semaforo);
    r=send(clientfd[idc],&val,sizeof(int),0);
    if( r != sizeof(val) )
	{
		perror("Error enviando confirmacion al cliente");
		exit(-1);
	}
	sem_post(semaforo);
    val--;
    getReg(val,perro);
	sem_wait(semaforo);
    r=send(clientfd[idc],perro,sizeof(struct dogType),0);
    if(r==-1){
        perror("error send del perro desde server");
        exit(-1);
    }
	sem_post(semaforo);


//---------------------------------------------------------------
    
    char hcName [20];//hc stands for historia clílica
    memset(hcName,0,sizeof(hcName));
    char hcFile [20];
    memset( hcFile,0, sizeof (hcFile));

    sprintf(hcName,"%d",val);
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
    int flag;
	sem_wait(semaforo);
    r = recv(clientfd[idc],&flag, sizeof(int),0);
	if( r != sizeof(flag) )
	{
		perror("Error reciviendo numero de registro");
		exit(-1);
	}
	sem_post(semaforo);
    if(flag==1){
        r=send(clientfd[idc],hcFile,sizeof(hcFile),0);
        if( r != sizeof(hcFile) )
	    {
		    perror("Error enviando reg val");
		    exit(-1);
	    }
    }
	memset(operation,0,LOG_SIZE*sizeof(char));
	strcpy(operation,"lectura");
	memset(type,0,LOG_SIZE*sizeof(char));
	sprintf(type,"%d",reg);
	addLog(operation,idc,type);
    //presionar cualquier tecla para continuar
    free(perro);
}


void ingresarReg(int idc){
    struct dogType * mascota;
	mascota = ( struct  dogType *) malloc( sizeof ( struct dogType ) );
	if( mascota == NULL )
	{
		perror("error en el malloc de la mascota");
		exit( -1 );
	}
	sem_wait(semaforo);
	int r = recv(clientfd[idc], mascota, sizeof(struct dogType),0);
	if( r != sizeof(struct dogType) )
	{
		perror("Error reciviendo mascota del cliente");
		exit(-1);
	}
    sem_post(semaforo);
    int key = hash(mascota->nombre);
    mascota->idPrev = lastestId[key];
    lastestId[key] = lastPos();
    save(mascota);
    int val=1;
	sem_wait(semaforo);
    r=send(clientfd[idc],&val,sizeof(int),0);
    if( r != sizeof(val) )
	{
		perror("Error enviando confirmacion al cliente");
		exit(-1);
	}
	sem_post(semaforo);
	memset(operation,0,LOG_SIZE*sizeof(char));
	strcpy(operation,"inserción");
	memset(type,0,LOG_SIZE*sizeof(char));
	strcpy(type,mascota->nombre);
	addLog(operation,idc,type);
}

void buscarReg(int idc){
    int r;
    struct dogType* perro;//creación del perro a ingresar
    perro = (struct dogType*)malloc(sizeof(struct dogType));
    if(perro == NULL){
        perror("error al asignar el espacio de la esctructura perro");
        exit(-1);
    }
    char nombre[NAME_LEN];
    memset(nombre,0,sizeof(nombre));
	sem_wait(semaforo);
    r = recv(clientfd[idc],nombre,NAME_LEN,0);
    if( r != NAME_LEN )
	{
		perror("Error reciviendo mascota del cliente");
		exit(-1);
	}
	sem_post(semaforo);
    int key,currId, i, equal, a, b;
    key = hash(nombre);
    currId =lastestId[key];
    
    while( currId != -1 )
	{
		getReg( currId, perro );
	
		equal = 1;

		for( i = 0; i < NAME_LEN; ++i )
		{
			a = (int) perro->nombre[i];
        	b = (int) nombre[i];
			if( a >= 'A' && a <= 'Z' ) a = 'a' + ( a - 'A' );
            if( b >= 'A' && b <= 'Z' ) b = 'a' + ( b - 'A' );
		    if( a != b ) equal = 0;
		}
		if( equal )
		{
			SendConfirmation(currId,idc );
			SendMascota( perro,idc );	
		}
		currId = perro -> idPrev;
	
    }
    free(perro);
    SendConfirmation(-1,idc);
	memset(operation,0,LOG_SIZE*sizeof(char));
	strcpy(operation,"búsqueda");
	memset(type,0,LOG_SIZE*sizeof(char));
	strcpy(type,nombre);
	addLog(operation,idc,type);
}

void borrarReg(int idc){
    int reg,r;
	//pasar la cantidad de registros actuales
	int total=lastPos();
	sem_wait(semaforo);
	r = send(clientfd[idc],&total,sizeof(total),0);
    if( r != sizeof(total))
    {
        perror("Error al enviar cantidad de registros");
        exit(-1);
    }
	sem_post(semaforo);
	sem_wait(semaforo);
    r = recv(clientfd[idc],&reg,sizeof(reg),0);
    if( r != sizeof(reg))
    {
        perror("Error al recibir el registro a eliminar");
        exit(-1);
    }
	sem_post(semaforo);
	struct dogType * mascotaFinal;
	mascotaFinal = ( struct  dogType *) malloc( sizeof ( struct dogType ) );	
	if( mascotaFinal == NULL )
	{
		perror("error en el malloc de la mascota");
		exit( -1 );
	}
	getReg( total-1, mascotaFinal );
	struct dogType * mascotaDelete;
	mascotaDelete = ( struct  dogType *) malloc( sizeof ( struct dogType ) );	
	if( mascotaDelete == NULL )
	{
		perror("error en el malloc de la mascota");
		exit( -1 );
	}
	getReg( reg, mascotaDelete ); //aca recibe reg
	// a partir de este punto comienza lo feo	
	int hashF, hashD;
	hashF = hash( mascotaFinal -> nombre );
	hashD = hash( mascotaDelete -> nombre );

	struct dogType * tmp;
	tmp = ( struct  dogType *) malloc( sizeof ( struct dogType ) );	
	if( tmp  == NULL )
	{
		perror("error en el malloc de la mascota");
		exit( -1 );
	}
	
	int id, idAnt,idDesp;
	id = -1;
	getReg( lastestId[hashF], tmp );  //aca es lastestID
	while( 1 )
	{
		idAnt = tmp -> idPrev;
		if( tmp -> idPrev <= reg ) //aca es reg
			break;
		idDesp = idAnt;
		getReg( tmp -> idPrev, tmp );
	}	
	
	// id guarda el indice del primer elemento que pertenece a la lista de HashFinal y que se encuentra despues del elemento a ser borrado
	// si este elemento no existe se id vale -1 ( cuando la posicion borrada es la ultima )
	// para este elemento su prevId se modificara ya que antes la lista era de la forma:  ..... idAnt, id, ..... lastID[hashF]
	// ahora la lista pasa a ser de la forma: ..... id Prev, ultimaMascota(posicion eliminada), id, ......

	if( id != -1 )	
	{
		getReg( id, tmp );
		tmp -> idPrev = reg; // ahora tmp guarda la data completa de dicho elemento      //aca es reg
	}

	// el head de la lista HashFinal, se re direcciona al que iba antes del ultimo elemento       
	lastestId[hashF] = mascotaFinal -> idPrev;                                                        

	//ahora posicionando a la mascota final en la posicion eliminada(registerId) se apunta como elemento anterior a idPrev 
	mascotaFinal -> idPrev = idAnt;	

	struct dogType * tmp2;
	tmp2 = ( struct  dogType *) malloc( sizeof ( struct dogType ) );	
	if( tmp2  == NULL )
	{
		perror("error en el malloc de la mascota");
		exit( -1 );
	}
	
	int idAnt2;
    int idDesp2 = -1;

	if( hashF != hashD )
	{	
		getReg( lastestId[hashD], tmp2 );                
		while( 1 )
		{
			if( tmp2 -> idPrev <= reg )          //aca es reg
				break;
			idDesp2 = tmp2 -> idPrev;
			getReg(tmp2->idPrev,tmp2);
		}	
		
		// id2 guarda el indice del primer elemento que apunta al elemento a ser borrado o -1 si el elemento borrado es el ultimo de su lista
		// la lista hashDelete era de la forma: ......( mascotaDelete -> prevId ), registerId, id2, .....
		// ahora es de la forma: ...... ( mascotaDelete -> prevId ), id2, .....
		
				
		if(idDesp2 != -1){
		    getReg( idDesp2, tmp2 );
		    tmp2->idPrev = mascotaDelete->idPrev; //ahora tmp guarda la data completa del elemento idDesp2
	    }else{

		//el elemento a ser borrado es el ultimo de su lista, por tanto
		    lastestId[hashD] = mascotaDelete->idPrev;
    	}   
    }else{
	    mascotaFinal -> idPrev = mascotaDelete -> idPrev;
    }

		
	int status;
	
	FILE * dbTmp;
	int check;
	dbTmp = fopen("dbTmp.dat", "a+");
	
	int i, tot;
	i = 0, tot = lastPos() - 1;

	for( i = 0; i < tot; ++ i )
	{
		if( i == id ){
			check = fwrite ( tmp, sizeof( struct dogType ), 1, dbTmp );	
			if (check==0)
			{
				perror("error escritura dbTmp");
				exit(-1);
			}
		} 
		else if( i == idDesp2 ){
			check = fwrite ( tmp2, sizeof( struct dogType ), 1, dbTmp );
			if (check==0)
			{
				perror("error escritura dbTmp");
				exit(-1);
			}	
		}
		else if( i == reg ){   //aca es reg
			check = fwrite ( mascotaFinal, sizeof( struct dogType ), 1, dbTmp );
			if (check==0)
			{
				perror("error escritura dbTmp");
				exit(-1);
			}
		} 
		else
		{
			getReg( i, mascotaDelete );
			check  = fwrite( mascotaDelete, sizeof ( struct dogType ), 1, dbTmp ); 
			if (check==0)
			{
				perror("error escritura dbTmp");
				exit(-1);
			}
		}
	}
	fclose(dbTmp); 

	status = remove( "dataDogs.dat" );	
	if( status != 0 )
	{
		perror( "Error eliminando la base de datos" );
		exit( -1 );
	}

	status = rename( "dbTmp.dat", "dataDogs.dat" );
	if( status != 0 )
	{
		perror( "Error en el renombramiento de la base de datos" );
		exit( -1 );
	}

	//Re escritura de historias clinicas
	//NO SE SI BORRAR LA HISTORIA CLINICA ESTE BIEN ASDADASDVDFBTRBMDLKFMVDJFKVNDJFKVNDFJVKNDFVJKNFDVJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJVSDKJVNSDVKSJD

	// Primero se elimina la historia clinica de la mascota registerId
	unsigned char hcNameDelete [22], hcNameLast [22];
	memset( hcNameDelete,0, sizeof (hcNameDelete));
	memset( hcNameLast, 0, sizeof (hcNameLast) );

	sprintf(hcNameDelete, "%d", reg + 1);  //aca es reg
	strcat(hcNameDelete,"hc");
	strcat(hcNameDelete,".txt");

		
	sprintf(hcNameLast, "%d", lastPos() + 1 );
	strcat(hcNameLast,"hc");
	strcat(hcNameLast,".txt");

	if( access( hcNameDelete, F_OK ) != -1 ) { 
		status = remove( hcNameDelete );	
		if( status != 0 )
		{
			perror( "Error eliminando historia clinica" );
			exit( -1 );

		}
	}	

	//Ahora se renombra la historia clinica del ultimo	
	
	if( access( hcNameLast, F_OK ) != -1 ) {
		status = rename( hcNameLast, hcNameDelete );
		if( status != 0 )
		{
			perror( "Error en el renombramiento de la historio clinica" );
			exit( -1 );
		}
		
	}

	free(mascotaFinal);
	free(mascotaDelete);
	free(tmp);
	free(tmp2);

	SendConfirmation(1,idc); 

	memset(operation,0,LOG_SIZE*sizeof(char));
	strcpy(operation,"borrar");
	memset(type,0,LOG_SIZE*sizeof(char));
	sprintf(type,"%d",reg);
	addLog(operation,idc,type);
}

void * run(void * ap){
    int idc = *(int*)ap;
	int doing=1;
    while(doing==1){
        int op;
        int r = recv(clientfd[idc], &op, sizeof(op),0);
	    if(r == 0)
	    {
		    perror("error por el connect");
		    exit(-1);
	    }
        switch (op)
        {
            case 1:
                printf("insertar");
                ingresarReg(idc);
                break;
            case 2:
                printf("ver");
                verReg(idc);
                break;
            case 3:
                printf("borrar");
                borrarReg(idc);
                break;
            case 4:
                printf("buscar");
                buscarReg(idc);
                break;
            case 5:
                doing= 0;
                break;
            default:
                printf("Ingresa una opción valida (1-5)\n");
                break;
        }
    }
    FILE *points;
        int  check;
        points = fopen("dataPointers.dat","w+");
        if(points == NULL)
		{	
                perror("error generando apuntadores");
                exit(-1);
        }

        check = fwrite( lastestId, HASH_SIZE * sizeof(int), 1, points );
        if( check == 0 )
		{
			perror("error en la escritura de apuntadores");	
			exit( -1 );
		}
	fclose(points);
	pthread_join(idThreat[idc],NULL);
    close(clientfd[idc]);
    clientfd[idc]=0;
}


int main(){
	semaforo = sem_open("semaforo_name", O_CREAT, 0700, MAX_CLIENTS);
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

//-----------------------------------------------------------------------------------------------
    int serverfd;
    socklen_t len;

    serverfd = socket(AF_INET,SOCK_STREAM,0);
    if(serverfd==-1){
        perror("error creando el socket");
    }
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr=INADDR_ANY; //indicarle al server que escuche por cualquier interfaz de red
    ipServer = server.sin_addr;
    bzero((server.sin_zero),8);
    len=sizeof(server);
    //r = setsockopt(serverfd);
    r = bind(serverfd,(struct sockaddr*)&server,len);
    if(r==-1){
        perror("error al nombrar el socket");
    }
    r = listen(serverfd,BACKLOG);
    if(r==-1){
        perror("error al escuchar el tipo de socket");
    }
	for( int i = 0; i < MAX_CLIENTS; ++ i ){
		arg[i] = i;
	}
	int idx=0;
	while (1)
	{
		socklen_t tamaClient = 0;
		clientfd[idx] = accept(serverfd,(struct sockaddr *)&client[idx],&tamaClient);
    	if(clientfd[idx]==-1){
        	perror("error al esperar al cliente");
    	}
		pthread_create(&(idThreat[idx]), NULL, run, (void*)&arg[idx]);
		if(idx == MAX_CLIENTS){
			break;
		}
		++idx;
	}	
	sem_close(semaforo);
    sem_unlink("semaforo_name");
    close(serverfd);
    return 0;
}
