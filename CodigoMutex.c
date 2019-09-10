#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h> 
#include <sys/types.h>
#include <sys/syscall.h>
#include <semaphore.h>


#define TAM_BUFFER 5
#define NUM_THREADS 8
#define NUM_TREBALLS 100000
typedef struct{
	int rango_desde;
	int rango_hasta;
} trabajo_t;

typedef struct {
	int pos_lectura;
	int pos_escritura;
	int num_trabajos;
	trabajo_t trabajo[TAM_BUFFER]; 
} buffer_trabajo_t;

pthread_t thread[NUM_THREADS];
buffer_trabajo_t buffer;
pthread_mutex_t mutex;
pthread_mutex_t mutex2;

void buffer_trabajos_inicializar(buffer_trabajo_t* buffer_trabajos){

	buffer_trabajos->pos_lectura=0;
	buffer_trabajos->pos_escritura=0;
	buffer_trabajos->num_trabajos=0;
}



int buffer_trabajos_lleno(buffer_trabajo_t* buffer_trabajos){

int retorno;

	if(buffer_trabajos->num_trabajos == TAM_BUFFER){

	retorno = 1;

	}else{

	retorno=0;

	}
return retorno;
}


int buffer_trabajos_vacio(buffer_trabajo_t* buffer_trabajos){

int retorno;

	if(buffer_trabajos->num_trabajos == 0){

	retorno = 1;

	}else{

	retorno=0;

	}
return retorno;
}


void buffer_trabajos_encolar(buffer_trabajo_t* buffer_trabajos, trabajo_t* trabajo){

	
	buffer_trabajos->trabajo[buffer_trabajos->pos_escritura] = *trabajo;
	buffer_trabajos->num_trabajos++;
	buffer_trabajos->pos_escritura = (buffer_trabajos->pos_escritura + 1) % TAM_BUFFER;
	

}

void buffer_trabajos_atender(buffer_trabajo_t* buffer_trabajos, trabajo_t* trabajo){

	
	*trabajo = buffer_trabajos->trabajo[buffer_trabajos->pos_lectura];
	buffer_trabajos->num_trabajos--;
	buffer_trabajos->pos_lectura = (buffer_trabajos->pos_lectura + 1) % TAM_BUFFER;



}

int calcularprimo(int numero)
{
	int retorno=0;
	int a=0;
	for(int i=1;i<=numero;i++)
	{
   		if(numero%i==0) 
		{
   		 	a++;
		}
	}
	if (a==2)
	{
	retorno=1;
	}
	return retorno;
}


void primo(trabajo_t* trabajo, int threadid){

int i,a;
a=0; 
int numero=trabajo->rango_desde;


printf("[Thread %d] Primos en [%d,%d]:",threadid, trabajo->rango_desde,trabajo->rango_hasta);
while(numero<=trabajo->rango_hasta){
	
	if (calcularprimo(numero) == 1)
	{
		printf("%d ", numero);
	}
	numero=numero+1;
	
}
printf("\n");

}





void enviar_trabajo(buffer_trabajo_t* buffer, trabajo_t* trabajo){

	pthread_mutex_lock(&mutex);
	while(buffer_trabajos_lleno(buffer)==1)
	{
		pthread_mutex_unlock(&mutex);
		pthread_mutex_lock(&mutex);

	}
	buffer_trabajos_encolar(buffer,trabajo);

	pthread_mutex_unlock(&mutex);
}




void recibir_trabajo(trabajo_t* trabajo){

	
	pthread_mutex_lock(&mutex);

	while(buffer_trabajos_vacio(&buffer)==1)
	{
		pthread_mutex_unlock(&mutex);
		pthread_mutex_lock(&mutex);

	}
	buffer_trabajos_atender(&buffer, trabajo);
	pthread_mutex_unlock(&mutex);
	


}

void* FuncionSlave(int threadid){
	trabajo_t trabajo;
	int seguir =1;

	
	while(seguir == 1){

		recibir_trabajo(&trabajo);
		pthread_mutex_lock(&mutex2);
		if(trabajo.rango_desde == -1){
		pthread_mutex_unlock(&mutex2);
		break;}
		else{
		primo(&trabajo,threadid);
		}
		pthread_mutex_unlock(&mutex2);

		
	}
	
	pthread_exit(NULL);
}


//Crear 5 trabajos extra
void CrearTrabajos(buffer_trabajo_t* buffer){
	int z;
	
	
	for(z=1;z<NUM_TREBALLS;z+=10){
		trabajo_t trabajo;
		trabajo.rango_desde=z;
		trabajo.rango_hasta=z+9;
		enviar_trabajo(buffer,&trabajo);
		}
	for(z=0;z<=NUM_THREADS;z++){
		trabajo_t trabajo;
		trabajo.rango_desde=-1;
		trabajo.rango_hasta=-1;
		enviar_trabajo(buffer,&trabajo);
		}
	}
	
	
	





int main(int argc,char** argv) {
// Crear trabajos
	buffer_trabajos_inicializar(&buffer);
	pthread_mutex_init(&mutex,NULL);
	pthread_mutex_init(&mutex2,NULL);
	
	long int i;
	for (i=0;i<NUM_THREADS;++i) {

		pthread_create(thread+i,NULL,(void* (*)(void*))FuncionSlave,(void*)(i));
	}
	CrearTrabajos(&buffer);
	
// Esperar threads
	for (i=0;i<NUM_THREADS;i++) {
		pthread_join(thread[i],NULL);
		printf("\n Eliminando thread %ld",i);

	}
return 0;
}
