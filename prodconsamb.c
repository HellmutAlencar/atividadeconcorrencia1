#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <pthread.h>
#include <stdlib.h>
#include <semaphore.h>

// Feito por Hellmut 21951331 e Nawe 21953261

int max;
int loops;
int *buffer1,*buffer2;

int consome1  = 0;
int produz1 = 0;
int consome2  = 0;
int produz2= 0;

int consumidorConsumiu = 0;
int ambosConsumiu = 0;
int ambosProduziu = 0;

sem_t empty1;
sem_t full1;
sem_t empty2;
sem_t full2;
sem_t mutex1;
sem_t mutex2;
pthread_mutex_t lockParaAmbos;
pthread_mutex_t lockParaConsumidor;

#define MAX (10)
int consumidores = 1;
int produtores = 1;
int nambos = 1;

void produz(int valor,int buf) {
   if(buf==1){
      buffer1[produz1] = valor;
      produz1 = (produz1+1) % max;
   }else{
      buffer2[produz2] = valor;
      produz2 = (produz2+1) % max;
   }
}

int consome(int buf) {
   int tmp; 
   if(buf==1){
      tmp = buffer1[consome1];
      consome1 = (consome1+1) %max;
   }else{
      tmp = buffer2[consome2];
      consome2 = (consome2+1) %max;
   }

   return tmp;
}

void *produtor(void *arg) {
   int i;
   for (i = 0; i < loops; i++) {
      sem_wait(&empty1);
      sem_wait(&mutex1);
      produz(i,1);
      printf("Produtor produziu em 1\n");
      sem_post(&mutex1);
      sem_post(&full1);
   }
   printf("Um produtor finalizado\n");

   return NULL;
}

void *consumidor(void *arg) {
   int tmp = 0;
   int i;
   while (1) {
      pthread_mutex_lock(&lockParaConsumidor);
      if(consumidorConsumiu == (loops * produtores)) {
         pthread_mutex_unlock(&lockParaConsumidor);
         break;
      }
      sem_wait(&full2);
      sem_wait(&mutex2);
      tmp = consome(2);
      printf("Consumidor consumiu do 2\n");
      consumidorConsumiu++;
      sem_post(&mutex2);
      sem_post(&empty2);
      pthread_mutex_unlock(&lockParaConsumidor);
   }
   printf("Um consumidor finalizado\n");
   return NULL;
}

void *ambos(void *arg) {
   // Deve acabar quando acabou todos os produtores, mas so pode acabar quando
   // nao tiver mais nada no buffer e finalizou todos os produtores
   int tmp = 0;
   int i;
   while (1) {
      pthread_mutex_lock(&lockParaAmbos);
      if(ambosProduziu == (loops * produtores) && ambosConsumiu == (loops * produtores)){
         pthread_mutex_unlock(&lockParaAmbos);
         break;
      }

      sem_wait(&full1);
      sem_wait(&mutex1);
      tmp = consome(1);
      printf("Ambos consumiu do 1\n");
      ambosConsumiu++;
      sem_post(&mutex1);
      sem_post(&empty1);

      sem_wait(&empty2);
      sem_wait(&mutex2);
      produz(tmp,2);
      printf("Ambos produziu em 2\n");
      ambosProduziu++;
      sem_post(&mutex2);
      sem_post(&full2);
      pthread_mutex_unlock(&lockParaAmbos);
   }
   printf("Um ambos finalizado\n");
   return NULL;
}

int main(int argc, char *argv[]) {
   if (argc != 6) {
      fprintf(stderr, "uso: %s <tambuffer> <loops> <produtores> <consumidores> <ambos>\n", argv[0]);
      exit(1);
   }
   max   = atoi(argv[1]);
   loops = atoi(argv[2]);
   produtores = atoi(argv[3]);
   consumidores = atoi(argv[4]);
   nambos = atoi(argv[5]);
   assert(consumidores <= MAX);

   buffer1 = (int *) malloc(max * sizeof(int));
   buffer2 = (int *) malloc(max * sizeof(int));
   int i;
   for (i = 0; i < max; i++) {
      buffer1[i] = 0;
      buffer2[i] = 0;
   }

   sem_init(&empty1, 0, MAX);
   sem_init(&full1, 0, 0);
   sem_init(&empty2, 0, MAX);
   sem_init(&full2, 0, 0);
   sem_init(&mutex1, 0, 1);
   sem_init(&mutex2, 0, 1);

   pthread_t pid[MAX], cid[MAX],aid[MAX];
   for (i = 0; i < consumidores; i++) {
      pthread_create(&cid[i], NULL, consumidor, (void *) (long long int) i); 
   }
   for (i = 0; i < nambos; i++) {
      pthread_create(&aid[i], NULL, ambos, NULL);
   }
   for (i = 0; i < produtores; i++) {
      pthread_create(&pid[i], NULL, produtor, NULL);
   }
   for (i = 0; i < consumidores; i++) {
      pthread_join(pid[i], NULL); 
      pthread_join(aid[i], NULL); 
      pthread_join(cid[i], NULL); 
   }
   return 0;
}


