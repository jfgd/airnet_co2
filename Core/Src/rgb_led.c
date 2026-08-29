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
 *   - red   -> channel 3 (PB3, LPTIM1_CH3, AF2)
 *   - green -> channel 4 (PB4, LPTIM1_CH4, AF1)
 *   - blue  -> channel 1 (PB2, LPTIM1_CH1, AF1)
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
 * so Pulse = 0 gives 100% on-time (LED at full brightness).
 *
 * A Pulse value equal to (or above) Period is meant to give 0% on-time, but
 * in practice a compare value that lands exactly on (or never reaches) the
 * auto-reload value still let a faint, permanent glow through on all three
 * channels (a PWM edge case at very low duty cycle / low PWM frequency).
 * To guarantee the LED is genuinely and fully off, a channel at 0 intensity
 * is entirely disconnected from LPTIM1 (PWM stopped) and its GPIO is
 * switched to a plain push-pull output forced to the inactive level (HIGH),
 * instead of relying on the PWM compare logic.
 */

#include "main.h"
#include "rgb_led.h"

#define RGB_LED_CHANNEL_RED    LPTIM_CHANNEL_3
#define RGB_LED_CHANNEL_GREEN  LPTIM_CHANNEL_4
#define RGB_LED_CHANNEL_BLUE   LPTIM_CHANNEL_1

/* Intensity to apply on each color channel to interpolate through yellow. */
#define RGB_LED_CO2_MAX_INTENSITY 255U

struct rgb_led_channel {
  uint32_t lptim_channel;
  GPIO_TypeDef *gpio_port;
  uint16_t gpio_pin;
  uint32_t gpio_af;
  uint8_t current_intensity; /* 0 means the channel is disconnected from
                               * LPTIM1 and its GPIO is forced HIGH (off). */
};

static struct rgb_led_channel rgb_led_channels[3] = {
  { RGB_LED_CHANNEL_RED,   NULL, 0, 0, 0 },
  { RGB_LED_CHANNEL_GREEN, NULL, 0, 0, 0 },
  { RGB_LED_CHANNEL_BLUE,  NULL, 0, 0, 0 },
};

static uint32_t rgb_led_period(void)
{
  return hlptim1.Init.Period;
}

/* Convert a 0-255 intensity to the LPTIM compare value for a channel
 * configured with LPTIM_OCPOLARITY_LOW (see file header comment). Only
 * meant to be used for intensity > 0: intensity == 0 is handled by fully
 * disconnecting the channel from LPTIM1 instead (see rgb_led_channel_set()).
 */
static uint32_t rgb_led_intensity_to_pulse(uint8_t intensity)
{
  uint32_t period = rgb_led_period();

  return period - ((uint32_t)intensity * period) / 255U;
}

/* Stop the LPTIM PWM output on this channel and drive its GPIO directly as
 * a plain push-pull output forced to the inactive level (HIGH), so the LED
 * segment is unambiguously and fully off. */
static void rgb_led_channel_force_off(struct rgb_led_channel *ch)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  HAL_LPTIM_PWM_Stop(&hlptim1, ch->lptim_channel);

  GPIO_InitStruct.Pin = ch->gpio_pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(ch->gpio_port, &GPIO_InitStruct);
  HAL_GPIO_WritePin(ch->gpio_port, ch->gpio_pin, GPIO_PIN_SET);

  ch->current_intensity = 0U;
}

/* Reconnect this channel's GPIO to LPTIM1's PWM output (AF mode) and (re)set
 * its duty cycle to reflect the requested intensity (1-255). */
static void rgb_led_channel_set_intensity(struct rgb_led_channel *ch, uint8_t intensity)
{
  LPTIM_OC_ConfigTypeDef sConfig = {0};

  if (ch->current_intensity == 0U) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Pin = ch->gpio_pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = ch->gpio_af;
    HAL_GPIO_Init(ch->gpio_port, &GPIO_InitStruct);
  }

  sConfig.Pulse = rgb_led_intensity_to_pulse(intensity);
  sConfig.OCPolarity = LPTIM_OCPOLARITY_LOW;
  HAL_LPTIM_OC_ConfigChannel(&hlptim1, &sConfig, ch->lptim_channel);

  if (ch->current_intensity == 0U) {
    HAL_LPTIM_PWM_Start(&hlptim1, ch->lptim_channel);
  }

  ch->current_intensity = intensity;
}

static void rgb_led_channel_set(struct rgb_led_channel *ch, uint8_t intensity)
{
  if (intensity == 0U) {
    if (ch->current_intensity != 0U) {
      rgb_led_channel_force_off(ch);
    }
  } else {
    rgb_led_channel_set_intensity(ch, intensity);
  }
}

void rgb_led_init(void)
{
  rgb_led_channels[0].gpio_port = LED_R_GPIO_Port;
  rgb_led_channels[0].gpio_pin  = LED_R_Pin;
  rgb_led_channels[0].gpio_af   = GPIO_AF2_LPTIM1;

  rgb_led_channels[1].gpio_port = LED_G_GPIO_Port;
  rgb_led_channels[1].gpio_pin  = LED_G_Pin;
  rgb_led_channels[1].gpio_af   = GPIO_AF1_LPTIM1;

  rgb_led_channels[2].gpio_port = LED_B_GPIO_Port;
  rgb_led_channels[2].gpio_pin  = LED_B_Pin;
  rgb_led_channels[2].gpio_af   = GPIO_AF1_LPTIM1;

  /* current_intensity starts at 0 in the static initializer, but the
   * channels are still wired to LPTIM1 in AF mode and not yet started at
   * this point (see MX_LPTIM1_Init() / HAL_LPTIM_MspPostInit()), which
   * would leave the pin state undefined/floating. Force a known, fully off
   * state on all three channels. */
  rgb_led_channel_force_off(&rgb_led_channels[0]);
  rgb_led_channel_force_off(&rgb_led_channels[1]);
  rgb_led_channel_force_off(&rgb_led_channels[2]);
}

void rgb_led_set_color(uint8_t red, uint8_t green, uint8_t blue)
{
  rgb_led_channel_set(&rgb_led_channels[0], red);
  rgb_led_channel_set(&rgb_led_channels[1], green);
  rgb_led_channel_set(&rgb_led_channels[2], blue);
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

  /* Linear cross-fade from green to red, red + green always adding up to
   * RGB_LED_CO2_MAX_INTENSITY so the LED's overall brightness stays
   * constant across the whole gradient (yellow is just as bright as pure
   * green or pure red, not twice as bright). */
  red = (uint8_t)((position * RGB_LED_CO2_MAX_INTENSITY) / range);
  green = (uint8_t)(RGB_LED_CO2_MAX_INTENSITY - red);

  rgb_led_set_color(red, green, 0);
}
