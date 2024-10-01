#include <stdio.h>
#include "ch32v003fun.h"
#include "st7789.h"
#include <stdlib.h>
#include <math.h>

// #define PIN_SCK PA5
// #define PIN_SDA PA7
// #define PIN_RES PA3
// #define PIN_DC  PA4
// #define PIN_BLK PA2

int get_bit(long int bits, int index){
  return (bits >> index) & 1;
}

void set_bit(long int* bits, int index, int value){
  if(value){
    *bits |= ((long)1 << index);
  } else {
    *bits &= ~((long)1 << index);
  }
}

#define X_CELL_NUM 240
#define Y_CELL_NUM 240

typedef struct {
  long int data[ST7789_WIDTH * ST7789_HEIGHT / (sizeof(long int)*8)];
  int data_array_num;
  int x_num;
  int y_num;
} bit_matrix;

void create_matrix(bit_matrix* matrix, int x_num, int y_num){
  matrix->x_num = x_num;
  matrix->y_num = y_num;
  matrix->data_array_num = (int)ceil(x_num*y_num/(sizeof(long int)*8.0));
  for(int i = 0; i < matrix->data_array_num; i++){
    matrix->data[i] = 0;
  }
}

int get_matrix(bit_matrix* matrix, int x, int y){
  int index = x + y*matrix->x_num;
  int data_index = index / (sizeof(long int)*8);
  int bit_index = index % (sizeof(long int)*8);
  return get_bit(matrix->data[data_index], bit_index);
}

void set_matrix(bit_matrix* matrix, int x, int y, int value){
  int index = x + y * matrix->x_num;
  int data_index = index / (sizeof(long int)*8);
  int bit_index = index % (sizeof(long int)*8);
  set_bit(&(matrix->data[data_index]), bit_index, value);
}

uint8_t rand8(void);
#define CELL_COLOR GREEN
#define BACKGROUND_COLOR BLACK

int main(){
  SystemInit();
  funGpioInitAll();
  tft_init();

  tft_fill_rect(0, 0, ST7789_WIDTH, ST7789_HEIGHT, BACKGROUND_COLOR);

  int cell_size = 3;
  int x_cell_count = ST7789_WIDTH/cell_size;
  int y_cell_count = ST7789_HEIGHT/cell_size;
  bit_matrix cells, cells_before;
  create_matrix(&cells, X_CELL_NUM, Y_CELL_NUM);
  create_matrix(&cells_before, X_CELL_NUM, Y_CELL_NUM);

  for(int x = 0; x < X_CELL_NUM; x++){
    for(int y = 0; y < Y_CELL_NUM; y++){
      int value = rand8()%2;
      set_matrix(&cells, x, y, value);
      set_matrix(&cells_before, x, y, value);
    }
  }

  while(1){
    // Delay_Ms(100);
    for(int y = 0; y < Y_CELL_NUM; y++){
      for(int x = 0; x < X_CELL_NUM; x++){
        int count = 0;
        for(int delta_x = -1; delta_x <= 1; delta_x++){
          for(int delta_y = -1; delta_y <= 1; delta_y++){
            if(delta_x == 0 && delta_y == 0){
              continue;
            }
            if(x+delta_x < 0 || x+delta_x >= X_CELL_NUM || y+delta_y < 0 || y+delta_y >= Y_CELL_NUM){
              continue;
            }
            count += get_matrix(&cells_before, x+delta_x, y+delta_y);
          }
        }
        if(get_matrix(&cells_before, x, y)){
          if(count == 2 || count == 3){
            set_matrix(&cells, x, y, 1);
          } else {
            set_matrix(&cells, x, y, 0);
          }
        } else {
          if(count == 3){
            set_matrix(&cells, x, y, 1);
          } else {
            set_matrix(&cells, x, y, 0);
          }
        }
      }
    }

    // 描画
    for(int i_x = 0; i_x < x_cell_count; i_x++){
      for(int i_y = 0; i_y < y_cell_count; i_y++){
        int pos_x = i_x * cell_size;
        int pos_y = i_y * cell_size;
        if(get_matrix(&cells, i_x, i_y) == 1){
          tft_fill_rect(pos_x, pos_y, cell_size, cell_size, CELL_COLOR);
        }else{
          tft_fill_rect(pos_x, pos_y, cell_size, cell_size, BACKGROUND_COLOR);
        }
      }
    }

    // 現在のセルの状態を保存
    for(int y = 0; y < Y_CELL_NUM; y++){
      for(int x = 0; x < X_CELL_NUM; x++){
        set_matrix(&cells_before, x, y, get_matrix(&cells, x, y));
      }
    }
  }
}

/* White Noise Generator State */
#define NOISE_BITS      8
#define NOISE_MASK      ((1 << NOISE_BITS) - 1)
#define NOISE_POLY_TAP0 31
#define NOISE_POLY_TAP1 21
#define NOISE_POLY_TAP2 1
#define NOISE_POLY_TAP3 0
uint32_t lfsr = 1;

/*
 * random byte generator
 */
uint8_t rand8(void)
{
    uint8_t  bit;
    uint32_t new_data;

    for (bit = 0; bit < NOISE_BITS; bit++)
    {
        new_data = ((lfsr >> NOISE_POLY_TAP0) ^ (lfsr >> NOISE_POLY_TAP1) ^ (lfsr >> NOISE_POLY_TAP2) ^
                    (lfsr >> NOISE_POLY_TAP3));
        lfsr     = (lfsr << 1) | (new_data & 1);
    }

    return lfsr & NOISE_MASK;
}