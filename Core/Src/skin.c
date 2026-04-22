/**
  ******************************************************************************
  * @file           : skin.c
  * @brief          : Display data on ePaper screen
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 jfgd.
  *
  * This program is free software: you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation, either version 3 of the License, or
  * (at your option) any later version.
  *
  * This program is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  * GNU General Public License for more details.
  *
  * You should have received a copy of the GNU General Public License
  * along with this program.  If not, see <http://www.gnu.org/licenses/>.
  *
  ******************************************************************************
  */

#include "EPD_1in54_V2.h"
#include "GUI_Paint.h"

#define STR_DISP_LEN 16


#define XSTART_TEMP 25
#define YSTART_TEMP 0
#define XSTART_HUMI 122
#define YSTART_HUMI 0
#define XSTART_CO2_PPM 8
#define YSTART_CO2_PPM 55

/* draw_slider_border()
 * Permanent slider drawing, draw cursor inside with
 * draw_slider_cursor(). xstart and ystart are top left corner of the
 * slider */
static void draw_slider_border(uint8_t *image, int xstart, int ystart,
                               int width, int height, uint16_t color,
                               uint16_t backcolor)
{
  UNUSED(image);
  int border = 1;
  int radius = height/2;

  /* Left circle */
  Paint_DrawCircle(xstart + radius + border, ystart + radius, radius,
                   color, border, DRAW_FILL_EMPTY);
  /* Right circle */
  Paint_DrawCircle(xstart + width - radius, ystart + radius, radius,
                   color, border, DRAW_FILL_EMPTY);

  /* Clear left circle inside part */
  Paint_DrawRectangle(xstart + radius, ystart, xstart + height + border,
                      ystart + height, backcolor, border, DRAW_FILL_FULL);

  /* Clear right circle inside part */
  Paint_DrawRectangle(xstart + width - height, ystart, xstart + width - radius,
                      ystart + height, backcolor, border, DRAW_FILL_FULL);

  /* Top line */
  Paint_DrawLine(xstart + radius, ystart, xstart + width - radius, ystart,
                 color, border, LINE_STYLE_SOLID);
  /* Bottom line */
  Paint_DrawLine(xstart + radius, ystart + height, xstart + width - radius,
                 ystart + height, color, border, LINE_STYLE_SOLID);

}

static void draw_slider_cursor(uint8_t *image, int xstart, int ystart,
                               int width, int height, uint16_t color,
                               uint16_t backcolor, int perthousand)
{
  UNUSED(image);
  UNUSED(backcolor);
  int border = 1;
  int radius = height/2 - border;
  int cursor_intercenter = 6;
  int usable_len = width - height - cursor_intercenter - border;

  if (perthousand > 1000) perthousand = 1000;
  if (perthousand < 0) perthousand = 0;

  int cursor_center_pos = (perthousand * usable_len) / 1000;

  Paint_DrawCircle(xstart + height/2 + border + cursor_center_pos,
                   ystart + height/2, radius,
                   color, border, DRAW_FILL_FULL);
  Paint_DrawCircle(xstart + height/2 + border + cursor_intercenter + cursor_center_pos,
                   ystart + height/2, radius,
                   color, border, DRAW_FILL_FULL);

  Paint_DrawRectangle(xstart + height/2 + border + cursor_center_pos, ystart + border,
                      xstart + height/2 + border + cursor_intercenter
                      + cursor_center_pos,
                      ystart + height, color, border, DRAW_FILL_FULL);

}

void skin_prepare(uint8_t *image)
{
  UNUSED(image);
  Paint_Clear(WHITE);

  Paint_DrawjChar(2, YSTART_TEMP, 'T', &Thermometer50, BLACK, WHITE);

  Paint_DrawString_j(XSTART_TEMP+(2*24), YSTART_TEMP+2, "°C",
                     &Digits25NotoSansSemiCondensedBold, 0, BLACK, WHITE);
  Paint_DrawString_j(XSTART_HUMI+(2*24), YSTART_HUMI+2, "%",
                     &Digits25NotoSansSemiCondensedBold, 0, BLACK, WHITE);

  Paint_DrawString_j(70, YSTART_CO2_PPM+85, "CO² ppm",
                     &CO2ppm25NotoSansMedium, 0, BLACK, WHITE);

  draw_slider_border(image, 0, 180, 200, 18, BLACK, WHITE);
}

void skin_update(uint8_t *image, uint16_t co2_ppm,
                 uint32_t temperature, uint32_t humidity, uint32_t vbat_mv)
{
  UNUSED(image);
  static uint32_t counter = 0;
  char co2_ppm_str[STR_DISP_LEN] = {0};
  char temperature_str1[STR_DISP_LEN] = {0};
  char temperature_str2[STR_DISP_LEN] = {0};
  char humidity_str[STR_DISP_LEN] = {0};
  char vbat_mv_str[STR_DISP_LEN] = {0};

  if (co2_ppm > 10000) {
    co2_ppm = 9999;
  }
  if (temperature > 10000) {
    temperature = 9999;
  }
  if (humidity > 100) {
    humidity = 99;
  }

  counter++;

  /* Temperature */
  snprintf(temperature_str1, STR_DISP_LEN, "%ld", temperature / 100);
  snprintf(temperature_str2, STR_DISP_LEN, ".%ld", (temperature - ((temperature / 100)*100))/10);
  Paint_ClearWindows(XSTART_TEMP, YSTART_TEMP,
                     XSTART_TEMP + 24 * 2,
                     YSTART_TEMP + Digits50NotoSansSemiCondensedBold.height, WHITE);
  Paint_ClearWindows(XSTART_TEMP+(2*24), YSTART_TEMP+20,
                     XSTART_TEMP+(2*24) + 11 + 6,
                     YSTART_TEMP+20 + Digits25NotoSansSemiCondensedBold.height, WHITE);
  Paint_DrawString_j(XSTART_TEMP, YSTART_TEMP, temperature_str1,
                     &Digits50NotoSansSemiCondensedBold, 0, BLACK, WHITE);
  Paint_DrawString_j(XSTART_TEMP+(2*24), YSTART_TEMP+20, temperature_str2,
                     &Digits25NotoSansSemiCondensedBold, 0, BLACK, WHITE);

  /* Humidity */
  snprintf(humidity_str, STR_DISP_LEN, "%ld", humidity);
  Paint_ClearWindows(XSTART_HUMI, YSTART_HUMI,
                     XSTART_HUMI + 24 * 2,
                     YSTART_HUMI + Digits50NotoSansSemiCondensedBold.height, WHITE);
  Paint_DrawString_j(XSTART_HUMI, YSTART_HUMI, humidity_str,
                     &Digits50NotoSansSemiCondensedBold, 0, BLACK, WHITE);

  /* CO2 */
  snprintf(co2_ppm_str, STR_DISP_LEN, "%02d", co2_ppm);
  Paint_ClearWindows(XSTART_CO2_PPM, YSTART_CO2_PPM,
                     XSTART_CO2_PPM + Digits90NotoSansSemiCondensedBold.max_width * 4,
                     YSTART_CO2_PPM + Digits90NotoSansSemiCondensedBold.height, WHITE);
  int xstart = XSTART_CO2_PPM;
  if (co2_ppm < 1000) {
    xstart = 31;                /* Center */
  }
  Paint_DrawString_j(xstart, YSTART_CO2_PPM, co2_ppm_str,
                     &Digits90NotoSansSemiCondensedBold, 0, BLACK, WHITE);
#define MAX_SLIDER_PPM 2500
#define MIN_SLIDER_PPM 400
  static int perthousand_prev = 0;
  int perthousand = (1000 * (int)co2_ppm) / (MAX_SLIDER_PPM - MIN_SLIDER_PPM) - ((1000 * MIN_SLIDER_PPM) / (MAX_SLIDER_PPM - MIN_SLIDER_PPM));
  draw_slider_cursor(image, 0, 180, 200, 18, WHITE, BLACK, perthousand_prev);
  draw_slider_cursor(image, 0, 180, 200, 18, BLACK, WHITE, perthousand);
  perthousand_prev = perthousand;


  /* Debug */
  printf("counter %ld\n", counter);
  Paint_ClearWindows(1, 166, 1+Font12.Width*12, 166+Font12.Height, WHITE);
  Paint_DrawNum(1, 166, counter,
                &Font12, BLACK, WHITE);

  snprintf(vbat_mv_str, STR_DISP_LEN, "%ld mV", vbat_mv);
  Paint_ClearWindows(150, 166, 150+Font12.Width*4, 166+Font12.Height, WHITE);
  Paint_DrawString_EN(150, 166, vbat_mv_str,
                      &Font12, BLACK, WHITE);
}
