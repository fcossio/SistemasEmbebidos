

//#include <unistd.h>
#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define TAIL_LENGTH 5

typedef enum {EXEC,WAIT,DONE} STATE;
// EXEC = 0; WAIT = 1; DONE = 2
typedef struct {
  char id ;
  int priority;
  int duration;
  double execute_time;
  STATE state;
} Tarea;


int set_task(Tarea * tsk, char id, int priority, int duration, int state);
double delay (int time);
int priority_order (Tarea * tail);
int print_table(Tarea * tail);
void bubbleSort(short *salida, int longitud);
void imprimirArreglo(short *arreglo, short longitud);
//int finish (Tarea * tail);

int main(){
  Tarea task_tail[5];

  set_task(&task_tail[0],'a',1,3,WAIT);
  set_task(&task_tail[1],'b',1,5,WAIT);
  set_task(&task_tail[2],'c',4,2,WAIT);
  set_task(&task_tail[3],'d',3,5,WAIT);
  set_task(&task_tail[4],'e',5,3,WAIT);

  priority_order (task_tail);
  print_table(&task_tail[0]);
  int current_task = 0;
  do{
    task_tail[current_task].state = EXEC;
    for ( int t = 0; t < task_tail[current_task].duration; t++){
        double delta_time = delay(1000);
        task_tail[current_task].execute_time += delta_time;
        print_table(&task_tail[0]);
    }
    task_tail[current_task].state = DONE;
    current_task ++;
  }while (current_task < 6);
  print_table(&task_tail[0]);
  return 0;
}

double delay(int seconds){ //la duracion es def. por el usuario ejecut esta fn cada 1seg
  clock_t init_time, end_time;
  double final_time;
  init_time = clock(); //Comienza a contar
  Sleep(seconds);
  end_time = clock(); //Tiempo que se tomo hasta este punto
  final_time = (double) (end_time - init_time)/CLOCKS_PER_SEC;
  return final_time;
} //fin de fn

int set_task(Tarea * tsk, char id, int priority, int duration, int state){
  tsk->id = id;
  tsk->priority = priority;
  tsk->duration = duration;
  tsk->state = state;
  tsk->execute_time = 0.0;
  return 0;
}//set_task

int priority_order (Tarea * tail){
  int direccion = tail;
  short sortedInts[TAIL_LENGTH];
  Tarea aux;
  for (int i=0; i<TAIL_LENGTH; i++){
    sortedInts[i]=tail->priority;
    tail++;
  }//for
  tail = direccion;
  bubbleSort(sortedInts, TAIL_LENGTH);


  Tarea * taili = tail;
  for (int i=0; i<TAIL_LENGTH; i++){ //sortedInts
    Tarea * tailj = tail;
    for (int j=0; j<TAIL_LENGTH; j++){ //tail
      if (tailj->priority == sortedInts[i]){
        aux = *taili;
        *taili = *tailj;
        *tailj = aux;
      }//if
    tailj ++;
    }//for
  taili ++;
  }//for
  return 0;
}//priority_order

int print_table(Tarea * tail){
  const char* getStateName(STATE state)
  {
     switch (state)
     {
        case WAIT: return "WAIT\0";
        case EXEC: return "EXEC\0";
        case DONE: return "DONE\0";
     }
  }
  system("cls");
  printf(" ----------------------------------------------\n");
  printf("| ID | priority | duration | state | exec_time |\n");
  printf(" ----------------------------------------------\n");
  for(int i = 0; i < TAIL_LENGTH; i++){
    printf("  %c       %d          %d        %s    %.03f    \n",
            tail->id,
            tail->priority,
            tail->duration,
            getStateName(tail->state),
            tail->execute_time);
    tail++;
  }
  return 0;
}

void bubbleSort(short *salida, int longitud){
    short aux1=0, aux2=0, cont1=0, cont2=0;
    int direccion = salida;
    while(cont2 < longitud-1){
        do{ aux1 = *salida;
            salida++;
            if (*salida < aux1){
                aux2 = *salida;
                *salida = aux1;salida--;
                *salida = aux2;salida++;
            }//if
            cont1++;
        }while (cont1 < longitud-cont2-1);
        salida = direccion;
        cont2++; cont1=0;
    }//While
}//bubbleSort
