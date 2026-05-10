/**
  ******************************************************************************
  * @file           : button_menu.h
  * @brief          : Header for button_menu.c file.
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

#ifndef __BUTTON_MENU_H
#define __BUTTON_MENU_H

#include <stdint.h>

enum button_event {
    BUTTON_EVENT_NONE = 0,
    BUTTON_EVENT_SINGLE_PRESS,
    BUTTON_EVENT_DOUBLE_PRESS,
    BUTTON_EVENT_LONG_PRESS,
};

enum button_state {
    BUTTON_STATE_IDLE = 0,
    BUTTON_STATE_PRESSED,
    BUTTON_STATE_WAIT_DOUBLE,
    BUTTON_STATE_DOUBLE_PRESSED,
};

struct button_fsm {
    enum button_state state;
    uint32_t              timestamp_ms;
    enum button_event     pending_event;
};

void button_init(volatile struct button_fsm *fsm);
void button_isr(volatile struct button_fsm *fsm, uint8_t gpio_level, uint32_t tick_ms);
enum button_event button_tick(volatile struct button_fsm *fsm, uint32_t tick_ms);


#endif /* __BUTTON_MENU_H */
