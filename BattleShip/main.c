#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct celda{
 int   barco;
 int   disparo;
} celda;

typedef struct usuario{
  char nombre[30];
  int wins;
} usuario;

celda *get_coord_pointer ( celda *c00, int x, int y);
int posicionar_barco(celda * c00, int long_barco, int x1, int y1, int x2, int y2);
int reset_tablero(celda *c00);
int imprimir_tablero(celda *c00, celda * c00foe);
int registro_usuario(usuario * usr, celda * c00, celda * c00foe);
int checkCoord(char * c);
int convert_coord(char * c, int *x, int *y);
int convert_coords(char *coord1s, char *coord2s, int *x1, int *x2, int *y1, int *y2);
int shoot(celda *c00, char * coords, int total_ships, usuario * usr);

//short imprimirTablero(usuario *pointer);

int main(){
  const int total_ships = 14;
  const usuario usr1, usr2;
  const celda tablero2[10][10];
  const celda tablero1[10][10];
  system("clear");

  reset_tablero(tablero1);
  reset_tablero(tablero2);
  printf("Battle Ship\n");
  printf("[Enter]:Iniciar\n");
  getchar();
  registro_usuario(&usr1, tablero1, tablero2);
  system("clear");
  printf("Pasar al siguiente jugador\n");
  printf("[Enter]:Continuar\n");
  getchar();
  registro_usuario(&usr2, tablero2, tablero1);

  do{
    //turno de usr2
    system("clear");
    printf("Turno de %s\n", usr2.nombre);
    printf("[Enter]:Continuar\n");
    getchar();
    int shoot_valido = 1;
    do{
      system("clear");
      printf("\n-----------------------------------Turno de %s------------------------------------\n", usr2.nombre);
      imprimir_tablero(tablero2, tablero1);
      char coords[4];
      printf("Coordenada de ataque:\n");
      gets(coords);
      shoot_valido = shoot(tablero1, coords, total_ships, &usr2);
      if (shoot_valido){
        printf("[Enter]:Continuar\n");
        getchar();
      }else{
        system("clear");
        printf("\n-----------------------------------Turno de %s------------------------------------\n", usr2.nombre);
        imprimir_tablero(tablero2, tablero1);
        printf("[Enter]:Continuar\n");
        getchar();
      }
    }while(shoot_valido);
    if (usr2.wins==1){
      system("clear");
      printf("%s destruyo todos los barcos enemigos", usr2.nombre);
      printf("[Enter]:Terminar\n");
      getchar();
      return 0;
    }
    //turno de usr1
    system("clear");
    printf("Turno de %s\n", usr1.nombre);
    printf("[Enter]:Continuar\n");
    getchar();
    shoot_valido = 1;
    do{
      system("clear");
      printf("\n-----------------------------------Turno de %s------------------------------------\n", usr1.nombre);
      imprimir_tablero(tablero1, tablero2);
      char coords[4];
      printf("Coordenada de ataque:\n");
      gets(coords);
      int x, y;
      shoot_valido = shoot(tablero2, coords, total_ships, &usr1);
      if (shoot_valido){
        printf("[Enter]:Continuar\n");
        getchar();
      }else{
        system("clear");
        printf("\n-----------------------------------Turno de %s------------------------------------\n", usr1.nombre);
        imprimir_tablero(tablero1, tablero2);
        printf("[Enter]:Continuar\n");
        getchar();
      }
    }while(shoot_valido);
    if (usr1.wins==1){
      system("clear");
      printf("%s destruyo todos los barcos enemigos", usr1.nombre);
      printf("[Enter]:Terminar\n");
      getchar();
      return 0;
    }

  }while(1);

return 0;
} //FIN MAIN

int reset_tablero (celda * c00){
  for (int i = 0; i < 10; i++){
    for (int j = 0;  j < 10; j++){
      celda *c = get_coord_pointer(c00, i, j);
      c->barco = 0;
      c->disparo = 0;
      //printf("%d,%d,disparo %d, barco %d, memoria %x\n",i,j,c->disparo,c->barco, &c->barco);
    }
  }
  return 1;
}

int shoot(celda *c00, char * coords, int total_ships, usuario * usr){
  int x,y;
  int counter =0;
  if (convert_coord(coords, &x, &y)){
    printf("La coordenada %s es invalida\n", coords);
    return 1;
  }
  celda * c =get_coord_pointer(c00,x,y);
  c->disparo = 1;
  for (int i = 0; i < 10; i++){
    for (int j = 0;  j < 10; j++){
      celda *c = get_coord_pointer(c00, i, j);
      if(c->barco == 1 && c->disparo == 1){counter++;}
      printf("punto\n");
    }
  }
  if (counter == total_ships){
    usr->wins = 1;
  }
  else{
    usr->wins = 0;
  }
  return 0;
}

int imprimir_tablero(celda * c00, celda * c00foe){
  printf("|---|---|---|---|---|---|---|---|---|---|---|||---|---|---|---|---|---|---|---|---|---|---|\n");
  printf("|YOU| 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 |10 |||FOE| 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 |10 |\n");
  printf("|---|---|---|---|---|---|---|---|---|---|---|||---|---|---|---|---|---|---|---|---|---|---|\n");
  for (int i = 0; i < 10; i++){
    printf("| %c |",i+65);
    for (int j = 0;  j < 10; j++){//YOU
      celda *c = get_coord_pointer(c00, i, j);
        if(c->disparo==1){
          if (c->barco==1){printf(" X |");}else{printf(" o |");}
        }else{
          if (c->barco==1){printf(" # |");}else{printf("   |");}
        }
    }
    printf("|| %c |",i+65);
    for (int j = 0;  j < 10; j++){//YOU
      celda *c = get_coord_pointer(c00foe, i, j);
        if(c->disparo==1){
          if (c->barco==1){printf(" X |");}else{printf(" o |");}
        }else{
          printf("   |");
        }
    }
    printf("\n|---|---|---|---|---|---|---|---|---|---|---|||---|---|---|---|---|---|---|---|---|---|---|\n");
  }//If i
  printf("\n");
}//imprimir_tablero

celda * get_coord_pointer(celda *c00, int x, int y){//coordenadas no indexadas en 0
  celda *coord = c00;
  coord += x;
  coord += (y*10);
  return coord;
}

int registro_usuario(usuario * usr, celda * c00, celda * c00foe){
  printf("Escribe el nombre del jugador\n");
  gets(&usr->nombre);
  system("clear");
  for(int tam = 5; tam > 1; tam--){
    int x1, x2, y1, y2;
    char coord1s[4], coord2s[4];
    int next_ship = 1;
    do{
      int next_coord = 1;
      do{
        //system("clear");
        system("clear");
        printf("\n-----------------------------------Turno de %s------------------------------------\n", usr->nombre);
        imprimir_tablero(c00, c00foe);
        printf("Se posiciona el barco de %d espacios\n", tam);
        printf("Ingresa la coordenada 1:\n");
        gets(coord1s);
        printf("Ingresa la coordenada 2:\n");
        gets(coord2s);
        next_coord = convert_coords(coord1s, coord2s, &x1, &x2, &y1, &y2);
        if (next_coord == 0){
          next_ship = posicionar_barco(c00,tam, x1, y1, x2, y2);
          if (next_ship){
            printf("No se puede colocar el barco en esas coordenadas\n");
            getchar();
          }
        }else{
          printf("[Enter]:Continuar\n");
          getchar();
        }
      }while(next_coord);
    }while(next_ship);
  }
}

int convert_coords(char * coord1s, char * coord2s, int *x1, int *x2, int *y1, int *y2){
  if (convert_coord(coord1s, x1, y1)){
    printf("La coordenada %s es invalida\n", coord1s);
    return 1;
  }
  if (convert_coord(coord2s, x2, y2)){
    printf("La coordenada %s es invalida\n", coord2s);
    return 1;
  }
  return 0;
}
int checkCoord(char * c){
  if (!((*c >= 'A' && *c <= 'J') || (*c >= 'a' && *c <= 'j'))){
    return 1;
  }
  char *c2 = c+1, *c3 = c+2;
  if (!(*c2 >= '1' && *c2 <= '9')){
    return 2;
  }
  if (*c2 == '1'){
    if (!(*c3=='0'||*c3=='\0')){
      return 3;
    };
  }
  return 0;
}
int convert_coord(char * c, int *x, int *y){
  if (checkCoord(c)==0){
    char aux = *c;
    if (aux >= 97) {//Cosas ASCII
      aux -= 32;
    }
    *x = aux - 65;
    if(*(c+1) =='1'){
      if (*(c+2)=='\0'){
        *y = 0;
      }else{
        *y = 9;
      }
    }else{
      *y = *(c+1) - 49;
    }
    return 0;
  }else{
    return 1;
  }
}

int posicionar_barco(celda * c00, int long_barco, int x1, int y1, int x2, int y2) {
  //valida y posiciona un barco de longitud long_barco en el tablero de un usuario.
  //regresa 1 cuando todo es correcto y se posiciono el barco. 0 si algo falla
  //printf("Posicionar barco x1: %d, x2: %d, y1: %d, y2: %d, c00:%x \n",x1,x2,y1,y2,c00);

  int delta_x = x2 - x1;
  int delta_y = y2 - y1;

  //printf("Deltax %d, Deltay %d\n",delta_x, delta_y);
  //revisar que sean posiciones validas
  if( x1 < 0 || x1 > 9 || x2 < 0 || x2 > 9 || y1 < 0 || y1 > 9 || y2 < 0 || y2 > 9){
    printf("Las posiciones x1 %d, x2 %d, y1 %d, y2 %d son invalidas \n",x1,x2,y1,y2);
    return 1;
  }
  // revisar que sea posicion vertical u horizontal
  if (!(delta_x == 0 || delta_y == 0)){
    printf("No se esta colocando horizontal o vertical: dx %d, dy %d\n", delta_x, delta_y);
    return 2;
  }
  //revisar que tenga la longitud del barco adecuada
  if(!(delta_x == long_barco-1 || delta_y == long_barco-1)){
    return 3;
    printf("La longitud del barco no coincide\n");
  }
  //revisar que no se hayan puesto barcos antes en esas posiciones
  int hubo_barco = 0;
  celda *c;
  if(delta_x > 0){ //la direccion es horizontal positiva
    for (int i = x1; i <= x2; i++){
      c = get_coord_pointer(c00, i, y1);
      if (c->barco){
        hubo_barco=1;
        // printf("dx>0 colision en %d,%d,%x\n", i,y1,c);
      };
    }
  }else if(delta_x < 0){ //la direccion es horizontal negativa
    for (int i = x2; i >= x1; i--){
      c = get_coord_pointer(c00, i, y1);
      if (c->barco){
        hubo_barco=1;
        // printf("dx<0 colision en %d,%d,%x\n", i,y1,c);
      };
      int aux = x1;
      x1 = x2;
      x2 = aux;
    }
  }else if(delta_y > 0){ //la direccion es vertical positiva
    for (int i = y1; i <= y2; i++){
      c = get_coord_pointer(c00, x1, i);
      if (c->barco){
        hubo_barco=1;
        // printf("dy>0 colision en %d,%d,%x\n", x1,i,c);
      };
    }
  }else if(delta_y < 0){ //la direccion es vertical negativa
    for (int i = y2; i >= y1; i--){
      c = get_coord_pointer(c00, x1, i);
      if (c->barco){
        hubo_barco=1;
        // printf("dy<0 colision en %d,%d,%x\n", x1,i,c);
      };
      int aux = y1;
      y1 = y2;
      y2 = aux;
    }
  }
  if (hubo_barco){
    printf("No se pueden poner 2 barcos en el mismo lugar\n");
    return 4;
  }
  for (int i = x1; i <= x2; i++){
    for (int j = y1; j <= y2; j++){
      c = get_coord_pointer(c00, i, j);
      c->barco = 1;
      // printf("barco en %d,%d, %x, %d\n", i,j,c,c->barco);
    }
  }

  return 0;
}
