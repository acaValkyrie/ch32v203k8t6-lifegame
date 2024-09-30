#include <stdio.h>
#include "ch32v003fun.h"
#include "st7789.h"

// #define PIN_SCK PA5
// #define PIN_SDA PA7
// #define PIN_RES PA3
// #define PIN_DC  PA4
// #define PIN_BLK PA2

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
  char cells[x_cell_count][y_cell_count];
  char before_cells[x_cell_count][y_cell_count];

  // セルの状態の初期化
  for(int i_x = 0; i_x < x_cell_count; i_x++){
    for(int i_y = 0; i_y < y_cell_count; i_y++){
      cells[i_x][i_y] = 0;
      before_cells[i_x][i_y] = 0;
    }
  }

  // ランダムにセルを生存させる
  for(int i_x = 0; i_x < x_cell_count; i_x++){
    for(int i_y = 0; i_y < y_cell_count; i_y++){
      if(rand8() % 2 == 0){
        before_cells[i_x][i_y] = 1;
      }
    }
  }

  while(1){
    Delay_Ms(100);
    for(int i_x = 0; i_x < x_cell_count; i_x++){
      for(int i_y = 0; i_y < y_cell_count; i_y++){
        // 周囲の生存セル数を数える
        int neighbor_sum = 0;
        for(int delta_x = -1; delta_x <= 1; delta_x++){
          for(int delta_y = -1; delta_y <= 1; delta_y++){
            if(delta_x == 0 && delta_y == 0){ continue; }
            int neighbor_x = i_x + delta_x;
            int neighbor_y = i_y + delta_y;
            if(neighbor_x < 0 || neighbor_x >= x_cell_count || neighbor_y < 0 || neighbor_y >= y_cell_count){ continue;}
            neighbor_sum += before_cells[neighbor_x][neighbor_y];
          }
        }

        // セルの生死を決定する
        if(before_cells[i_x][i_y] == 1){
          if(neighbor_sum < 2 || neighbor_sum > 3){
            cells[i_x][i_y] = 0;
          }else{
            cells[i_x][i_y] = 1;
          }
        }else{
          if(neighbor_sum == 3){
            cells[i_x][i_y] = 1;
          }else{
            cells[i_x][i_y] = 0;
          }
        }
      }
    }

    // 描画
    for(int i_x = 0; i_x < x_cell_count; i_x++){
      for(int i_y = 0; i_y < y_cell_count; i_y++){
        int pos_x = i_x * cell_size;
        int pos_y = i_y * cell_size;
        if(cells[i_x][i_y] == 1){
          tft_fill_rect(pos_x, pos_y, cell_size, cell_size, CELL_COLOR);
        }else{
          tft_fill_rect(pos_x, pos_y, cell_size, cell_size, BACKGROUND_COLOR);
        }
      }
    }

    // 現在のセルの状態を保存
    for(int i_x = 0; i_x < x_cell_count; i_x++){
      for(int i_y = 0; i_y < y_cell_count; i_y++){
        before_cells[i_x][i_y] = cells[i_x][i_y];
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