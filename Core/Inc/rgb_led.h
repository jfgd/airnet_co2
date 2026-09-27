/**
  ******************************************************************************
  * @file           : rgb_led.h
  * @brief          : Header for rgb_led.c file.
  *                   Full-color RGB LED driver on top of LPTIM1 PWM channels.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 Jeremy Fanguède.
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

#ifndef __RGB_LED_H
#define __RGB_LED_H

#include <stdint.h>

/* Must be called once, after MX_LPTIM1_Init(), before any other rgb_led_*()
 * call. Puts the three PWM channels (red/green/blue) in a known, fully off
 * state and enables their output pins. */
void rgb_led_init(void);

/* Global user brightness, in percent of the requested intensity, applied on
 * top of every rgb_led_set_color() / rgb_led_display_co2_level() call. */
void rgb_led_set_brightness(uint8_t percent);

/* Set the LED to an arbitrary RGB color, each component in [0, 255]. */
void rgb_led_set_color(uint8_t red, uint8_t green, uint8_t blue);

/* Turn the LED fully off. */
void rgb_led_off(void);

/* Compute the green -> yellow -> red color for a given CO2 level (in ppm) and
 * apply it to the LED. Below good_ppm the LED is solid green, above bad_ppm
 * the LED is solid red, in between the color is linearly interpolated
 * through yellow. */
void rgb_led_display_co2_level(int32_t co2_ppm, int32_t good_ppm, int32_t bad_ppm);

#endif /* __RGB_LED_H */
