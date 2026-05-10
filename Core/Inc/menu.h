/**
  ******************************************************************************
  * @file           : menu.h
  * @brief          : Header for menu.c file.
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

#ifndef __MENU_H
#define __MENU_H


enum conf_temperature_unit {
	CONF_TEMP_CELSIUS = 0,
	CONF_TEMP_FAHRENHEIT = 1,
};

enum conf_skin_value {
	CONF_SKIN_SLIDER = 0,
	CONF_SKIN_SLIDER_INVERTED,
	CONF_SKIN_EMOJI,
	CONF_SKIN_EMOJI_INVERTED,
	CONF_SKIN_SLIDER_EMOJI,
	CONF_SKIN_SLIDER_EMOJI_INVERTED,
};

/* Only int allowed */
struct conf {
	int refresh_rate_sec;
	enum conf_skin_value skin;
	int debug_counter;
	int debug_bat_voltage;
	enum conf_temperature_unit temperature_unit;
};

/* Global configuration */
extern struct conf g_conf;

void menu_enter(void);

#endif /* __MENU_H */
