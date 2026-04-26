/**
  ******************************************************************************
  * @file           : skin.h
  * @brief          : Header for skin.c file.
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

#ifndef __SKIN_H
#define __SKIN_H

#include <stdbool.h>

void skin_prepare(uint8_t *gImage);

void skin_update(uint8_t *gImage, uint16_t co2_ppm,
		 uint32_t temperature, uint32_t humidity, uint32_t vbat_mv,
		 bool powered, int debug_counter, int debug_bat_voltage);

#endif /* __SKIN_H */
