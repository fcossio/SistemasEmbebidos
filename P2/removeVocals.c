#include <stdio.h>

void shrink(char *a);
void bubbleSort(char *a);
void selectionSort(char *a);
void binarySearch ();
int main(void){
  char frase[100];
  printf("Delete vocals and numbers from string\n");
  gets(frase);
  char * a = frase;
  while(*a){
    if (*a=='a' || *a=='e' || *a=='i' || *a=='o' || *a=='u' || *a=='A' || *a=='E' || *a=='I' || *a=='O' || *a=='U') {
      shrink(a);
      a--;
    }
    if (*a >= 0x30 && *a <=0x39) {
      shrink(a);
      a--;
    }
    a++;
  }
  printf("%s\n", frase);
  return 0;
}

void shrink(char *a){
  while(*a){
    *a = *(a+1);
    a++;
  }
}
void bubbleSort(char *a){

}
void selectionSort(char *a){

}
void binarySearch(){
  
}
