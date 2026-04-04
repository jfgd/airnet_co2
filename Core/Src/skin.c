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

void skin_prepare_image(uint8_t *gImage, uint16_t co2_ppm,
                        uint32_t temperature, uint32_t humidity)
{
  static int counter = 0;
  char co2_ppm_str[STR_DISP_LEN] = {0};

  counter++;

  snprintf(co2_ppm_str, STR_DISP_LEN, "%02d", co2_ppm);
#define XSTART_CO2_PPM 60
#define YSTART_CO2_PPM 50
  Paint_ClearWindows(XSTART_CO2_PPM, YSTART_CO2_PPM,
                     XSTART_CO2_PPM + FontRobotoBold40.Width * 4,
                     YSTART_CO2_PPM + FontRobotoBold40.Height, WHITE);
  Paint_DrawString_EN(XSTART_CO2_PPM, YSTART_CO2_PPM, co2_ppm_str,
                      &FontRobotoBold40, BLACK, WHITE);


  Paint_ClearWindows(1, 190, 1+Font12.Width*12, 190+Font12.Height, WHITE);
  Paint_DrawNum(1, 190, counter,
                &Font12, BLACK, WHITE);

}
