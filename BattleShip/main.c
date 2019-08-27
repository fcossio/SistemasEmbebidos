#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct celda{
 bool   barco;
 bool   disparo;
} celda;

typedef struct usuario{
  char nombre[30] ;
  celda tablero[10][10] ; //tablero
} usuario;

celda *get_coord_pointer( usuario usr, int x, int y);
//int posicionar_barco(short long_barco, short x1, short y1, short x2, short y2);
int reset_tablero(usuario usr);
void registro_usuario(usuario user); //Se debe mandar el nombre de usuario, y las coordenadas de sus 5 tipos


int main(){
  const usuario usr1, usr2;
  registro_usuario(usr1);


  reset_tablero(usr1);
return 0;
}

int reset_tablero(usuario usr){
  //borra el tablero de un usuario
  for (int i = 0; i < 10; i++){
    for (int j = 0;  j < 10; j++){
      celda * c = get_coord_pointer(usr, i, j); //Se obtiene un puntero a una coordenada
      c->barco = 0;
      c->disparo = 0;
    }
  }
}  //Se accesa los atributos de barco y disparo, y los borra.

celda *get_coord_pointer(usuario usr, int x, int y){
  //se le da un usuario y una posicion de tablero y regresa el apuntador a la celda correcta
  celda * coord = &usr.tablero;
  coord += x;
  coord += y * 10;
  return coord;
}
//*********************************

void registro_usuario(usuario user){

  printf("Escribe el nombre del usuario\n");
  gets(user.nombre);
  printf("El nombre del ususario %s se registro.\n", user.nombre);
  for(int tam=5; tam>0,tam--){

  }

}




int posicionar_barco(usuario usr, short long_barco, short x1, short y1, short x2, short y2) {
  //valida y posiciona un barco de longitud long_barco en el tablero de un usuario.
  //regresa 1 cuando todo es correcto y se posiciono el barco. 0 si algo falla
  short delta_x = x2 - x1;
  short delta_y = y2 - y1;
  //revisar que sean posiciones validas
  if( x1 < 0 || x1 > 9 || x2 < 0 || x2 > 9 || y1 < 0 || y1 > 9 || y2 < 0 || y2 > 9){
    return 0;
  }
  // revisar que sea posicion vertical u horizontal
  if (!(delta_x == 0 || delta_y == 0)){
    return 0;
  }
  //revisar que tenga la longitud del barco adecuada
  if(!(delta_x == long_barco || delta_y == long_barco)){
    return 0;
  }
  //revisar que no se hayan puesto barcos antes en esas posiciones
  int hubo_barco = 0;
  celda *c;
  if(delta_x > 0){ //la direccion es horizontal positiva
    for (int i = x1; i < x2; i++){
      c = get_coord_pointer(usr, i, y1);
      (c->barco)?hubo_barco=1;
    }
  }else if(delta_x < 0){ //la direccion es horizontal negativa

  }else if(delta_y > 0){ //la direccion es vertical positiva

  }else if(delta_y < 0){ //la direccion es vertical negativa

  }

}
