/**
  ******************************************************************************
  * @file           : button_menu.c
  * @brief          : Button for menu
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

#include <stdio.h>
#include "button_menu.h"

#define BUTTON_LONG_PRESS_MS   1000U   /* Hold time to trigger a long press      */
#define BUTTON_DOUBLE_GAP_MS    300U   /* Max gap between two presses for double */

/* XXX: There is no software debouncing, assume electronic deboucing
 * circuit is good enough */

/* Functions */

static uint32_t elapsed(uint32_t now, uint32_t since)
{
    return now - since;
}

void button_init(volatile struct button_fsm *fsm)
{
    fsm->state         = BUTTON_STATE_IDLE;
    fsm->timestamp_ms  = 0U;
    fsm->pending_event = BUTTON_EVENT_NONE;
}

void button_isr(volatile struct button_fsm *fsm, uint8_t gpio_level, uint32_t tick_ms)
{
    switch (fsm->state) {

    case BUTTON_STATE_IDLE:
        if (gpio_level == 0U) {
            fsm->state        = BUTTON_STATE_PRESSED;
            fsm->timestamp_ms = tick_ms;
        }
        break;

    case BUTTON_STATE_PRESSED:
        if (gpio_level == 1U) {
            fsm->state        = BUTTON_STATE_WAIT_DOUBLE;
            fsm->timestamp_ms = tick_ms;
        }
        break;

    case BUTTON_STATE_WAIT_DOUBLE:
        if (gpio_level == 0U) {
            fsm->state        = BUTTON_STATE_DOUBLE_PRESSED;
            fsm->timestamp_ms = tick_ms;
        }
        break;

    case BUTTON_STATE_DOUBLE_PRESSED:
        if (gpio_level == 1U) {
            fsm->pending_event = BUTTON_EVENT_DOUBLE_PRESS;
            fsm->state         = BUTTON_STATE_IDLE;
        }
        break;

    default:
        fsm->state = BUTTON_STATE_IDLE;
        break;
    }

    /* printf("button isr lvl %d ms %ld\n", gpio_level, tick_ms); */
}

enum button_event button_tick(volatile struct button_fsm *fsm, uint32_t tick_ms)
{
    enum button_event evt = BUTTON_EVENT_NONE;

    if (fsm->pending_event != BUTTON_EVENT_NONE) {
        evt                = fsm->pending_event;
        fsm->pending_event = BUTTON_EVENT_NONE;
        return evt;
    }

    switch (fsm->state) {

    case BUTTON_STATE_PRESSED:
        if (elapsed(tick_ms, fsm->timestamp_ms) >= BUTTON_LONG_PRESS_MS) {
            fsm->state = BUTTON_STATE_IDLE;
            evt        = BUTTON_EVENT_LONG_PRESS;
            /* printf("BUTTON_STATE_PRESSED => long %ld - %ld (%ld)\n", tick_ms, fsm->timestamp_ms); */
        }
        break;

    case BUTTON_STATE_WAIT_DOUBLE:
        if (elapsed(tick_ms, fsm->timestamp_ms) >= BUTTON_DOUBLE_GAP_MS) {
            fsm->state = BUTTON_STATE_IDLE;
            evt        = BUTTON_EVENT_SINGLE_PRESS;
        }
        break;

    case BUTTON_STATE_IDLE:
    case BUTTON_STATE_DOUBLE_PRESSED:
    default:
        break;
    }

    return evt;
}
