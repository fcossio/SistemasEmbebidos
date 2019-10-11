#include "et024006dhu.h"
#include "stdlib.h"
#include "delay.h"
#include "math.h"
typedef struct point2D{
 float x;
 float y;
} point2D;

void draw_impossible_polygon(uint8_t n, point2D * center, float radius, float rotation);
void draw_point2D(point2D * p);
void draw_line2D(point2D * p1, point2D * p2);
float euclidian_distance(point2D * p1, point2D * p2);
float to_deg(float radians);
float to_rad(float degrees);
void act14(void){
  et024006_DrawFilledRect(0, 0, ET024006_WIDTH, ET024006_HEIGHT, WHITE);
  uint8_t total_polygons = 3;
  point2D center[total_polygons];
  for(int i = 0; i<total_polygons; i++){
    center[i].x = (i+1) * ET024006_WIDTH/(total_polygons+1);
    center[i].y = ET024006_HEIGHT/2;
  }
  float rotation = 0.0;
  while(1){

    for(int i = 0; i<total_polygons; i++){
      et024006_DrawFilledRect(center[i].x-(ET024006_WIDTH/total_polygons+1)/2, 0, ET024006_WIDTH/total_polygons+1, ET024006_HEIGHT, WHITE);
      draw_impossible_polygon( i+3, &center[i], 18-(total_polygons-i), rotation);
    }
    rotation += 1;
    delay_ms(33);
  }
}

void draw_impossible_polygon(uint8_t n, point2D * center, float radius, float rotation){
  float center_angle = 360.0/n;
  float corner_angle = (180.0 - center_angle)/2;
  //initial points
  point2D p[n][4];
  for (int i=0; i<n; i++){
    p[i][0].x = center->x + radius * sin(to_rad((i*360.0)/n + rotation));
    p[i][0].y = center->y + radius * cos(to_rad((i*360.0)/n + rotation));
    draw_point2D(&p[i][0]);
  }

  for (int j=1; j<4; j++){
    for (int i=0; i<n; i++){
      float angle = (center_angle * (i + j) + corner_angle) ;
      float dp = euclidian_distance(&p[i][j-1], &p[(i+1)%n][j-1]);
      if (j==3)
        dp = euclidian_distance(&p[i][j-2], &p[(i+1)%n][j-2]) *0.83/(1+(n-2)/15.0);
      p[i][j].x = p[i][j-1].x + 1.2*dp * sin(to_rad(angle + rotation));
      p[i][j].y = p[i][j-1].y + 1.2*dp * cos(to_rad(angle + rotation));
      draw_line2D(&p[i][j-1], &p[i][j]);
    }
  }
  for (int i=0; i<n; i++){
    draw_line2D(&p[i][3], &p[(i+1)%n][2]);
  }
  // for (int i=0; i<n; i++){
  //   float angle = center_angle * (i + 1) + corner_angle;
  //   p[i][2].x = p[i][1].x + 2.5*radius * sin(to_rad(angle));
  //   p[i][2].y = p[i][1].y + 2.5*radius * cos(to_rad(angle));
  //   draw_line2D(&p[i][0], &p[i][1]);
  //   draw_point2D(&p[i][1]);
  //   delay_ms(100);
  // }
}
void draw_point2D(point2D * p){
  if (p->x >= 0.0 && p->x <= 320.4 && p->y >= 0.0 && p->y <= 240.4){
    et024006_DrawPixel( round(p->x), round(p->y), color16(63,000,63));
  }
}
void draw_line2D(point2D * p1, point2D * p2){
  if (p1->x >= 0.0 && p1->x <= 320.4 && p1->y >= 0.0 && p1->y <= 240.4){
    if (p2->x >= 0.0 && p2->x <= 320.4 && p2->y >= 0.0 && p2->y <= 240.4){
      et024006_DrawLine(round(p1->x), round(p1->y), round(p2->x), round(p2->y), BLACK);
    }
  }
}
float euclidian_distance(point2D * p1, point2D * p2){
  return ( sqrt(pow(p1->x - p2->x,2) + pow(p1->y - p2->y,2)) );
}
float to_deg(float radians) {
    return (radians * (180.0 / M_PI));
}
float to_rad(float degrees) {
    return ( (degrees * M_PI) / 180.0);
}
