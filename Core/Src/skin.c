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
#include "skin.h"

#define STR_DISP_LEN 16


#define XSTART_TEMP 19
#define YSTART_TEMP 0
#define XSTART_HUMI 122
#define YSTART_HUMI 0
#define XSTART_CO2_PPM 8
#define YSTART_CO2_PPM 62

#define WIDTH_TEMP_HUMI_FONT 27
/* = jfont_get_width(&Digits39NotoSansSemiCondensedBold, '8') */

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

static void skin_temp_rh_top_prepare(uint8_t *image, int fcolor, int bcolor)
{
  UNUSED(image);
  Paint_DrawjChar(0, YSTART_TEMP, 'T', &Thermometer39, fcolor, bcolor);
  Paint_DrawjChar(XSTART_HUMI+(2*WIDTH_TEMP_HUMI_FONT), YSTART_TEMP+20, 'D',
                  &Droplet20, fcolor, bcolor);

  Paint_DrawString_j(XSTART_TEMP+(2*WIDTH_TEMP_HUMI_FONT), YSTART_TEMP+2, "°C",
                     &Digits25NotoSansSemiCondensedBold, 0, fcolor, bcolor);
  Paint_DrawString_j(XSTART_HUMI+(2*WIDTH_TEMP_HUMI_FONT), YSTART_HUMI+2, "%",
                     &Digits25NotoSansSemiCondensedBold, 0, fcolor, bcolor);

}

static void skin_slider_prepare(uint8_t *image, int fcolor, int bcolor)
{
  UNUSED(image);
  Paint_DrawString_j(70, YSTART_CO2_PPM+78, "CO² ppm",
                     &CO2ppm25NotoSansMedium, 0, fcolor, bcolor);

  draw_slider_border(image, 0, 180, 200, 18, fcolor, bcolor);
}

static void skin_emoji_prepare(uint8_t *image, int fcolor, int bcolor)
{
  UNUSED(image);
  Paint_DrawString_j(120, YSTART_CO2_PPM+78, "CO²",
                     &CO2ppm25NotoSansMedium, 0, fcolor, bcolor);
  Paint_DrawString_j(120, YSTART_CO2_PPM+103, "ppm",
                     &CO2ppm25NotoSansMedium, 0, fcolor, bcolor);
}

static void skin_temp_rh_top_update(uint8_t *image, int fcolor, int bcolor,
                                    uint32_t temperature,
                                    uint32_t humidity)
{
  UNUSED(image);
  char temperature_str1[STR_DISP_LEN] = {0};
  char temperature_str2[STR_DISP_LEN] = {0};
  char humidity_str[STR_DISP_LEN] = {0};

  /* Temperature */
  snprintf(temperature_str1, STR_DISP_LEN, "%ld", temperature / 100);
  snprintf(temperature_str2, STR_DISP_LEN, ".%ld", (temperature - ((temperature / 100)*100))/10);
  Paint_ClearWindows(XSTART_TEMP, YSTART_TEMP,
                     XSTART_TEMP + WIDTH_TEMP_HUMI_FONT * 2,
                     YSTART_TEMP + Digits39NotoSansSemiCondensedBold.height, bcolor);
  Paint_ClearWindows(XSTART_TEMP+(2*WIDTH_TEMP_HUMI_FONT), YSTART_TEMP+20,
                     XSTART_TEMP+(2*WIDTH_TEMP_HUMI_FONT) + 11 + 6,
                     YSTART_TEMP+20 + Digits25NotoSansSemiCondensedBold.height, bcolor);
  Paint_DrawString_j(XSTART_TEMP, YSTART_TEMP, temperature_str1,
                     &Digits39NotoSansSemiCondensedBold, 0, fcolor, bcolor);
  Paint_DrawString_j(XSTART_TEMP+(2*WIDTH_TEMP_HUMI_FONT), YSTART_TEMP+20, temperature_str2,
                     &Digits25NotoSansSemiCondensedBold, 0, fcolor, bcolor);

  /* Humidity */
  snprintf(humidity_str, STR_DISP_LEN, "%ld", humidity);
  Paint_ClearWindows(XSTART_HUMI, YSTART_HUMI,
                     XSTART_HUMI + WIDTH_TEMP_HUMI_FONT * 2,
                     YSTART_HUMI + Digits39NotoSansSemiCondensedBold.height, bcolor);
  Paint_DrawString_j(XSTART_HUMI, YSTART_HUMI, humidity_str,
                     &Digits39NotoSansSemiCondensedBold, 0, fcolor, bcolor);
}

static void skin_co2_update(uint8_t *image, int fcolor, int bcolor, uint16_t co2_ppm)
{
  UNUSED(image);
  char co2_ppm_str[STR_DISP_LEN] = {0};

  /* CO2 */
  snprintf(co2_ppm_str, STR_DISP_LEN, "%02d", co2_ppm);
  Paint_ClearWindows(XSTART_CO2_PPM, YSTART_CO2_PPM,
                     XSTART_CO2_PPM + Digits65NotoSansSemiCondensedBold.max_width * 4,
                     YSTART_CO2_PPM + Digits65NotoSansSemiCondensedBold.height, bcolor);
  int xstart = XSTART_CO2_PPM;
  if (co2_ppm < 1000) {
    xstart = 31;                /* Center */
  }
  Paint_DrawString_j(xstart, YSTART_CO2_PPM, co2_ppm_str,
                     &Digits65NotoSansSemiCondensedBold, 0, fcolor, bcolor);
}

static void skin_emoji_update(
  uint8_t *image, int fcolor, int bcolor, uint16_t co2_ppm)
{
  UNUSED(image);
  int l;

  /*  ☺️ |  😌 |  🙂 |  😐 |  🙄 |  😕 |  🙁 |  😥 |  😓 |  😣 |  😖 |  😰 */
  /*  23 |  24 |  78 |  28 |  80 |  33 |  77 |  49 |  31 |  47 |  34 |  60 */
  /*    500   650   800   900   1000   1200  1300  1400  1500  1600  2000 */

#define EMOJI_LEVELS 12
  char emoji_levels[EMOJI_LEVELS] =
    {23, 24, 78, 28, 80, 33, 77, 49, 31, 47, 34, 60};
  uint16_t co2_levels[EMOJI_LEVELS] =
    {500, 650, 800, 900, 1000, 1200, 1300, 1400, 1500, 1600, 2000, UINT16_MAX};

  for (l = 0 ; l < EMOJI_LEVELS ; l++) {
    if (co2_ppm < co2_levels[l]) {
      break;
    }
  }

  Paint_ClearWindows(15, 139, 15+EmojiFaces.max_width, 139+EmojiFaces.height, bcolor);
  Paint_DrawjChar(15, 139, emoji_levels[l], &EmojiFaces, fcolor, bcolor);
}

static void skin_power_update(uint8_t *image, int fcolor, int bcolor, bool powered)
{
  UNUSED(image);
  /* Power */
  if (powered) {
    Paint_DrawjChar(5, 130, 'L', &Lightning27, fcolor, bcolor);
  } else {
    Paint_ClearWindows(5, 130, 5+Lightning27.max_width, 130+Lightning27.height, bcolor);
  }
}

static void skin_debug_update(
  uint8_t *image, int fcolor, int bcolor, uint32_t vbat_mv,
  uint32_t counter, int debug_counter, int debug_bat_voltage)
{
  UNUSED(image);
  char vbat_mv_str[STR_DISP_LEN] = {0};
  char counter_str[STR_DISP_LEN] = {0};

  /* Debug */
  printf("counter %ld\n", counter);
  Paint_ClearWindows(1, 166, 1+font12.max_width*12, 166+font12.height, bcolor);
  if (debug_counter) {
    snprintf(counter_str, STR_DISP_LEN, "%ld", counter);
    Paint_DrawString_j(1, 166, counter_str,
                       &font12, 0, fcolor, bcolor);
  }

  Paint_ClearWindows(150, 166, 150+font12.max_width*7, 166+font12.height, bcolor);
  if (debug_bat_voltage) {
    snprintf(vbat_mv_str, STR_DISP_LEN, "%ld mV", vbat_mv);
    Paint_DrawString_j(150, 166, vbat_mv_str,
                       &font12, 0, fcolor, bcolor);
  }
}

static void skin_slider_update(uint8_t *image, int fcolor, int bcolor,
                               uint16_t co2_ppm)
{
#define MAX_SLIDER_PPM 2500
#define MIN_SLIDER_PPM 400
  static int perthousand_prev = 0;
  int perthousand = (1000 * (int)co2_ppm) / (MAX_SLIDER_PPM - MIN_SLIDER_PPM) - ((1000 * MIN_SLIDER_PPM) / (MAX_SLIDER_PPM - MIN_SLIDER_PPM));
  draw_slider_cursor(image, 0, 180, 200, 18, bcolor, fcolor, perthousand_prev);
  draw_slider_cursor(image, 0, 180, 200, 18, fcolor, bcolor, perthousand);
  perthousand_prev = perthousand;
}

void skin_prepare(enum conf_skin_value skin, uint8_t *image)
{
  int fcolor, bcolor;

  switch (skin) {
  case CONF_SKIN_SLIDER_INVERTED:
  case CONF_SKIN_EMOJI_INVERTED:
    bcolor = BLACK;
    fcolor = WHITE;
    break;
  case CONF_SKIN_SLIDER:
  case CONF_SKIN_EMOJI:
  default:
    bcolor = WHITE;
    fcolor = BLACK;
    break;
  }
  Paint_Clear(bcolor);


  switch (skin) {

  case CONF_SKIN_SLIDER:
  case CONF_SKIN_SLIDER_INVERTED:
  default:
    skin_temp_rh_top_prepare(image, fcolor, bcolor);
    skin_slider_prepare(image, fcolor, bcolor);
    break;

  case CONF_SKIN_EMOJI:
  case CONF_SKIN_EMOJI_INVERTED:
    skin_temp_rh_top_prepare(image, fcolor, bcolor);
    skin_emoji_prepare(image, fcolor, bcolor);
    break;
  }
}

void skin_update(enum conf_skin_value skin, uint8_t *image, uint16_t co2_ppm,
                 uint32_t temperature, uint32_t humidity, uint32_t vbat_mv,
                 bool powered, int debug_counter, int debug_bat_voltage)
{
  UNUSED(image);
  UNUSED(skin);
  int fcolor, bcolor;
  static uint32_t counter = 0;


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

  switch (skin) {
  case CONF_SKIN_SLIDER_INVERTED:
  case CONF_SKIN_EMOJI_INVERTED:
    bcolor = BLACK;
    fcolor = WHITE;
    break;
  case CONF_SKIN_SLIDER:
  case CONF_SKIN_EMOJI:
  default:
    bcolor = WHITE;
    fcolor = BLACK;
    break;
  }


  switch (skin) {

  case CONF_SKIN_SLIDER:
  case CONF_SKIN_SLIDER_INVERTED:
  default:
    skin_temp_rh_top_update(image, fcolor, bcolor, temperature, humidity);
    skin_co2_update(image, fcolor, bcolor, co2_ppm);
    skin_power_update(image, fcolor, bcolor, powered);
    skin_slider_update(image, fcolor, bcolor, co2_ppm);
    skin_debug_update(image, fcolor, bcolor, vbat_mv, counter,
                       debug_counter, debug_bat_voltage);
    break;

  case CONF_SKIN_EMOJI:
  case CONF_SKIN_EMOJI_INVERTED:
    skin_temp_rh_top_update(image, fcolor, bcolor, temperature, humidity);
    skin_co2_update(image, fcolor, bcolor, co2_ppm);
    skin_power_update(image, fcolor, bcolor, powered);
    skin_emoji_update(image, fcolor, bcolor, co2_ppm);
    /* skin_debug_update(image, fcolor, bcolor, vbat_mv, counter, */
    /*                   debug_counter, debug_bat_voltage); */
    break;

  case CONF_SKIN_SLIDER_EMOJI:
  case CONF_SKIN_SLIDER_EMOJI_INVERTED:
    skin_temp_rh_top_update(image, fcolor, bcolor, temperature, humidity);
    skin_co2_update(image, fcolor, bcolor, co2_ppm);
    skin_power_update(image, fcolor, bcolor, powered);
    skin_emoji_update(image, fcolor, bcolor, co2_ppm);
    skin_slider_update(image, fcolor, bcolor, co2_ppm);
    break;
  }

}
