#include "et024006dhu.h"
#include "stdlib.h"
#include "delay.h"
#include "math.h"

void draw_impossible_polygon(uint8_t n, point2D * center, float radius, float rotation);
void act14(uint8_t valid_state, uint8_t * actual_state){
  et024006_DrawFilledRect(0, 0, ET024006_WIDTH, ET024006_HEIGHT, WHITE);
  uint8_t total_polygons = 3;
  point2D center[total_polygons];
  for(int i = 0; i<total_polygons; i++){
    center[i].x = (i+1) * ET024006_WIDTH/(total_polygons+1);
    center[i].y = ET024006_HEIGHT/2;
  }
  float rotation = 0.0;
  while(valid_state == *actual_state){
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
}
