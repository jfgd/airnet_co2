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

#include <stdbool.h>
#include "EPD_1in54_V2.h"
#include "GUI_Paint.h"

#define STR_DISP_LEN 16


#define XSTART_TEMP 25
#define YSTART_TEMP 0
#define XSTART_HUMI 122
#define YSTART_HUMI 0
#define XSTART_CO2_PPM 8
#define YSTART_CO2_PPM 62

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
  Paint_ClearWindows(xstart + radius, ystart, xstart + height + border,
		     ystart + height, backcolor);

  /* Clear right circle inside part */
  Paint_ClearWindows(xstart + width - height - border, ystart, xstart + width - radius,
		     ystart + height, backcolor);

  /* Top line */
  Paint_SetPixelHLine(xstart + radius, xstart + width - radius,
		      ystart - border, color); /* border = 1 */
  /* Bottom line */
  Paint_SetPixelHLine(xstart + radius, xstart + width - radius,
		      ystart + height -border, color); /* border = 1 */

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

  Paint_ClearWindows(xstart + height/2 + border + cursor_center_pos, ystart,
                      xstart + height/2 + border + cursor_intercenter
                      + cursor_center_pos,
                      ystart + height - border, color);

}

void skin_prepare(uint8_t *image)
{
  UNUSED(image);
  Paint_Clear(WHITE);

  Paint_DrawjChar(2, YSTART_TEMP, 'T', &Thermometer50, BLACK, WHITE);
  Paint_DrawjChar(XSTART_HUMI+(2*24), YSTART_TEMP+20, 'D',
		  &Droplet20, BLACK, WHITE);

  Paint_DrawString_j(XSTART_TEMP+(2*24), YSTART_TEMP+2, "°C",
                     &Digits25NotoSansSemiCondensedBold, 0, BLACK, WHITE);
  Paint_DrawString_j(XSTART_HUMI+(2*24), YSTART_HUMI+2, "%",
                     &Digits25NotoSansSemiCondensedBold, 0, BLACK, WHITE);

  Paint_DrawString_j(70, YSTART_CO2_PPM+85, "CO² ppm",
                     &CO2ppm25NotoSansMedium, 0, BLACK, WHITE);

  draw_slider_border(image, 0, 180, 200, 18, BLACK, WHITE);
}

void skin_update(uint8_t *image, uint16_t co2_ppm,
                 uint32_t temperature, uint32_t humidity, uint32_t vbat_mv,
                 bool powered)
{
  UNUSED(image);
  static uint32_t counter = 0;
  char co2_ppm_str[STR_DISP_LEN] = {0};
  char temperature_str1[STR_DISP_LEN] = {0};
  char temperature_str2[STR_DISP_LEN] = {0};
  char humidity_str[STR_DISP_LEN] = {0};
  char vbat_mv_str[STR_DISP_LEN] = {0};
  char counter_str[STR_DISP_LEN] = {0};

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
                     XSTART_CO2_PPM + Digits65NotoSansSemiCondensedBold.max_width * 4,
                     YSTART_CO2_PPM + Digits65NotoSansSemiCondensedBold.height, WHITE);
  int xstart = XSTART_CO2_PPM;
  if (co2_ppm < 1000) {
    xstart = 31;                /* Center */
  }
  Paint_DrawString_j(xstart, YSTART_CO2_PPM, co2_ppm_str,
                     &Digits65NotoSansSemiCondensedBold, 0, BLACK, WHITE);
#define MAX_SLIDER_PPM 2500
#define MIN_SLIDER_PPM 400
  static int perthousand_prev = 0;
  int perthousand = (1000 * (int)co2_ppm) / (MAX_SLIDER_PPM - MIN_SLIDER_PPM) - ((1000 * MIN_SLIDER_PPM) / (MAX_SLIDER_PPM - MIN_SLIDER_PPM));
  draw_slider_cursor(image, 0, 180, 200, 18, WHITE, BLACK, perthousand_prev);
  draw_slider_cursor(image, 0, 180, 200, 18, BLACK, WHITE, perthousand);
  perthousand_prev = perthousand;

  /* Power */
  if (powered) {
    Paint_DrawjChar(5, 130, 'L', &Lightning27, BLACK, WHITE);
  } else {
    Paint_ClearWindows(5, 130, 5+Lightning27.max_width, 130+Lightning27.height, WHITE);
  }

  /* Debug */
  printf("counter %ld\n", counter);
  snprintf(counter_str, STR_DISP_LEN, "%ld", counter);
  Paint_ClearWindows(1, 166, 1+font12.max_width*12, 166+font12.height, WHITE);
  Paint_DrawString_j(1, 166, counter_str,
		     &font12, 0, BLACK, WHITE);

  snprintf(vbat_mv_str, STR_DISP_LEN, "%ld mV", vbat_mv);
  Paint_ClearWindows(150, 166, 150+font12.max_width*7, 166+font12.height, WHITE);
  Paint_DrawString_j(150, 166, vbat_mv_str,
		     &font12, 0, BLACK, WHITE);
}
