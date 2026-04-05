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

void skin_prepare(uint8_t *image)
{
  UNUSED(image);
}

void skin_update(uint8_t *image, uint16_t co2_ppm,
     uint32_t temperature, uint32_t humidity)
{
  UNUSED(image);
  static uint32_t counter = 0;
  char co2_ppm_str[STR_DISP_LEN] = {0};
  char temperature_str1[STR_DISP_LEN] = {0};
  char temperature_str2[STR_DISP_LEN] = {0};
  char humidity_str[STR_DISP_LEN] = {0};

  counter++;

  snprintf(temperature_str1, STR_DISP_LEN, "%ld.%ld C", temperature / 100,
           (temperature - ((temperature / 100)*100))/10);
  snprintf(temperature_str2, STR_DISP_LEN, "%ld.%ld C", temperature / 100,
           (temperature - ((temperature / 100)*100))/10);
#define XSTART_TEMP 10
#define YSTART_TEMP 5
  Paint_ClearWindows(XSTART_TEMP, YSTART_TEMP,
                     XSTART_TEMP + FontRobotoBold40.Width * 8,
                     YSTART_TEMP + FontRobotoBold40.Height, WHITE);
  Paint_DrawChar(XSTART_TEMP+FontRobotoBold40.Width*2+37, YSTART_TEMP, 'C',
                 &FontRobotoBold40, BLACK, WHITE);
  Paint_DrawChar(XSTART_TEMP+FontRobotoBold40.Width*2, YSTART_TEMP, '.',
                 &FontRobotoBold40, BLACK, WHITE);
  Paint_DrawNum(XSTART_TEMP, YSTART_TEMP, temperature / 100,
                &FontRobotoBold40, BLACK, WHITE);
  Paint_DrawNum(XSTART_TEMP+FontRobotoBold40.Width*2+10, YSTART_TEMP,
                (temperature - ((temperature / 100)*100))/10,
                &FontRobotoBold40, BLACK, WHITE);

  snprintf(humidity_str, STR_DISP_LEN, "%ld %%", humidity);
#define XSTART_HUMI 10
#define YSTART_HUMI 60
  Paint_ClearWindows(XSTART_HUMI, YSTART_HUMI,
                     XSTART_HUMI + FontRobotoBold40.Width * 2,
                     YSTART_HUMI + FontRobotoBold40.Height, WHITE);
  Paint_DrawString_EN(XSTART_HUMI, YSTART_HUMI, humidity_str,
                      &FontRobotoBold40, BLACK, WHITE);

  snprintf(co2_ppm_str, STR_DISP_LEN, "%02d", co2_ppm);
#define XSTART_CO2_PPM 10
#define YSTART_CO2_PPM 115
  Paint_ClearWindows(XSTART_CO2_PPM, YSTART_CO2_PPM,
                     XSTART_CO2_PPM + FontRobotoBold40.Width * 4,
                     YSTART_CO2_PPM + FontRobotoBold40.Height, WHITE);
  Paint_DrawString_EN(XSTART_CO2_PPM, YSTART_CO2_PPM, co2_ppm_str,
                      &FontRobotoBold40, BLACK, WHITE);


  printf("counter %ld\n", counter);
  Paint_ClearWindows(1, 185, 1+Font12.Width*12, 185+Font12.Height, WHITE);
  Paint_DrawNum(1, 185, counter,
                &Font12, BLACK, WHITE);

}
