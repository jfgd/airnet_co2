/**
  ******************************************************************************
  * @file           : rgb_led.c
  * @brief          : Full-color RGB LED driver on top of LPTIM1 PWM channels.
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

/*
 * The RGB LED is driven by LPTIM1 in PWM mode:
 *   - red   -> channel 3
 *   - green -> channel 4
 *   - blue  -> channel 1
 *
 * All three channels share the *same* counter/period (LPTIM_Init.Period, set
 * up once in MX_LPTIM1_Init()), so they can run at the same time and each
 * channel's compare register (CCRx) controls its own duty cycle, hence its
 * own brightness. This is what allows mixing red/green/blue into any color.
 *
 * The channels are configured with LPTIM_OCPOLARITY_LOW (see
 * MX_LPTIM1_Init()): the output idles high and is driven low from the compare
 * match up to the period rollover, and the LED lights up while the pin is
 * low. This means the "on" fraction of the period grows as the compare value
 * (Pulse) shrinks:
 *
 *   on_fraction = (Period - Pulse) / Period
 *
 * so Pulse = Period gives 0% on-time (LED off) and Pulse = 0 gives 100%
 * on-time (LED at full brightness). rgb_led_set_color() converts the
 * requested 0-255 intensities to compare values using that relationship.
 */

#include "main.h"
#include "rgb_led.h"

#define RGB_LED_CHANNEL_RED    LPTIM_CHANNEL_3
#define RGB_LED_CHANNEL_GREEN  LPTIM_CHANNEL_4
#define RGB_LED_CHANNEL_BLUE   LPTIM_CHANNEL_1

/* Intensity to apply on each color channel to interpolate through yellow. */
#define RGB_LED_CO2_MAX_INTENSITY 255U

static uint32_t rgb_led_period(void)
{
  return hlptim1.Init.Period;
}

/* Convert a 0-255 intensity to the LPTIM compare value for a channel
 * configured with LPTIM_OCPOLARITY_LOW (see file header comment). */
static uint32_t rgb_led_intensity_to_pulse(uint8_t intensity)
{
  uint32_t period = rgb_led_period();

  return period - ((uint32_t)intensity * period) / 255U;
}

static void rgb_led_set_channel(uint32_t channel, uint8_t intensity)
{
  LPTIM_OC_ConfigTypeDef sConfig = {0};

  sConfig.Pulse = rgb_led_intensity_to_pulse(intensity);
  sConfig.OCPolarity = LPTIM_OCPOLARITY_LOW;

  HAL_LPTIM_OC_ConfigChannel(&hlptim1, &sConfig, channel);
}

void rgb_led_init(void)
{
  rgb_led_set_channel(RGB_LED_CHANNEL_RED, 0);
  rgb_led_set_channel(RGB_LED_CHANNEL_GREEN, 0);
  rgb_led_set_channel(RGB_LED_CHANNEL_BLUE, 0);

  HAL_LPTIM_PWM_Start(&hlptim1, RGB_LED_CHANNEL_RED);
  HAL_LPTIM_PWM_Start(&hlptim1, RGB_LED_CHANNEL_GREEN);
  HAL_LPTIM_PWM_Start(&hlptim1, RGB_LED_CHANNEL_BLUE);
}

void rgb_led_set_color(uint8_t red, uint8_t green, uint8_t blue)
{
  rgb_led_set_channel(RGB_LED_CHANNEL_RED, red);
  rgb_led_set_channel(RGB_LED_CHANNEL_GREEN, green);
  rgb_led_set_channel(RGB_LED_CHANNEL_BLUE, blue);
}

void rgb_led_off(void)
{
  rgb_led_set_color(0, 0, 0);
}

void rgb_led_display_co2_level(int32_t co2_ppm, int32_t good_ppm, int32_t bad_ppm)
{
  int32_t range;
  int32_t position;
  uint8_t red;
  uint8_t green;

  if (bad_ppm <= good_ppm) {
    /* Degenerate configuration, avoid a division by zero below. */
    rgb_led_set_color(RGB_LED_CO2_MAX_INTENSITY, 0, 0);
    return;
  }

  if (co2_ppm <= good_ppm) {
    rgb_led_set_color(0, RGB_LED_CO2_MAX_INTENSITY, 0);
    return;
  }

  if (co2_ppm >= bad_ppm) {
    rgb_led_set_color(RGB_LED_CO2_MAX_INTENSITY, 0, 0);
    return;
  }

  range = bad_ppm - good_ppm;
  position = co2_ppm - good_ppm; /* 0 .. range */

  if (position <= (range / 2)) {
    /* Green -> Yellow: ramp red up, keep green at max. */
    red = (uint8_t)((position * 2 * RGB_LED_CO2_MAX_INTENSITY) / range);
    green = RGB_LED_CO2_MAX_INTENSITY;
  } else {
    /* Yellow -> Red: keep red at max, ramp green down. */
    int32_t position_in_upper_half = (position - (range / 2)) * 2;

    red = RGB_LED_CO2_MAX_INTENSITY;
    green = (uint8_t)(RGB_LED_CO2_MAX_INTENSITY -
                       (position_in_upper_half * RGB_LED_CO2_MAX_INTENSITY) / range);
  }

  rgb_led_set_color(red, green, 0);
}
