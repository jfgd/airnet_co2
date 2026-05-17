/**
  ******************************************************************************
  * @file           : menu.c
  * @brief          : Configuration menu
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

#include <string.h>
#include "EPD_1in54_V2.h"
#include "GUI_Paint.h"
#include "menu.h"
#include "button_menu.h"

extern volatile uint32_t g_ts_ms_last_button_pressed;
extern volatile uint32_t g_ts_ms_previous_button_pressed;
extern volatile int g_button_pressed_flag;
extern volatile struct button_fsm g_button_fsm;

extern UBYTE gImage[];

extern uint32_t rtc_get_ms(void);

static int mod(int a, int b)
{
    int r = a % b;
    return r < 0 ? r + b : r;
}

/* Default values */
struct conf g_conf = {
	.refresh_rate_sec = 5,
	.skin = CONF_SKIN_EMOJI,
	.debug_counter = 0,
	.debug_bat_voltage = 0,
	.temperature_unit = CONF_TEMP_CELSIUS,
};


enum value_type {
	SELECT = 1,
	ACTION = 2,
	BACK = 3,
};

#define MAX_LIST_SIZE 6
#define MAX_ITEM_NAME_LEN 32

struct item {
	int value;
	char *name;
};

struct menu_list {
	char *name;
	char *help;
	enum value_type type;
	int *value;
	struct item items[MAX_LIST_SIZE+1];
};


struct menu_list g_menu[] = {
	{
		.name = "Refresh Rate",
		.help = "Select refresh rate",
		.type = SELECT,
		.value = &g_conf.refresh_rate_sec,
		.items = {
			{ .value = 5, .name = "5 s" },
			{ .value = 10, .name = "10 s" },
			{ .value = 30, .name = "30 s" },
			{ .value = 60, .name = "1 min" },
			{ .value = 300, .name = "5 min" },
			{ .value = 600, .name = "10 min" },
		},
	},
	{
		.name = "Skin",
		.help = "Select display",
		.type = SELECT,
		.value = (int*)&g_conf.skin,
		.items = {
			{ .value = CONF_SKIN_SLIDER,
			  .name = "Slider" },
			{ .value = CONF_SKIN_EMOJI,
			  .name = "Emoji" },
			{ .value = CONF_SKIN_SLIDER_EMOJI,
			  .name = "Slider + Emoji" },
			{ .value = CONF_SKIN_SLIDER_INVERTED,
			  .name = "Slider (Inverted Color)" },
			{ .value = CONF_SKIN_EMOJI_INVERTED,
			  .name = "Emoji (Inverted Color)" },
			{ .value = CONF_SKIN_SLIDER_EMOJI_INVERTED,
			  .name = "Slider + Emoji Inv. Col" },
		},
	},
	{
		.name = "Debug Counter",
		.help = "Display a counter increasing at each refresh",
		.type = SELECT,
		.value = &g_conf.debug_counter,
		.items = {
			{ .value = 0, .name = "No" },
			{ .value = 1, .name = "Yes" },
		},
	},
	{
		.name = "Debug Battery Voltage",
		.help = "Display battery voltage in mV",
		.type = SELECT,
		.value = &g_conf.debug_bat_voltage,
		.items = {
			{ .value = 0, .name = "No" },
			{ .value = 1, .name = "Yes" },
		},
	},
	/* { */
	/* 	.name = "Temperature Unit", */
	/* 	.help = "Select °C or °F", */
	/* 	.type = SELECT, */
	/* 	.value = (int*)&g_conf.temperature_unit, */
	/* 	.items = { */
	/* 		{ .value = CONF_TEMP_CELSIUS, .name = "°C" }, */
	/* 		{ .value = CONF_TEMP_FAHRENHEIT, .name = "°F" }, */
	/* 	}, */
	/* }, */
	{
		.name = "Exit",
		.help = "Exit configuration menu",
		.type = BACK,
	},
};



#define MENU_LENGTH (int)(sizeof(g_menu) / sizeof(g_menu[0]))

enum item_value {
	UNSELECTED = -2,
	//ENTERED = -1,
};

#define HEADER_HEIGHT 22
#define HEADER_LINE_WIDTH 3
#define SELECT_ROW_HEIGHT 22
#define SELECT_ROW_LINE_WIDTH 1
#define ARROW_OFFSET_X 2
#define TEXT_OFFSET_Y 6
#define TEXT_OFFSET_X 15
#define TEXT_OFFSET_HELP_X 5

static void menu_draw_base_image(void)
{
	int y = 0;

	Paint_SelectImage(gImage);
	Paint_Clear(WHITE);

	y += HEADER_HEIGHT;
	Paint_DrawLine(0, y, EPD_1IN54_V2_WIDTH, y,
		       BLACK, HEADER_LINE_WIDTH, LINE_STYLE_SOLID);

	y += HEADER_LINE_WIDTH;
	Paint_DrawjChar(ARROW_OFFSET_X, y + TEXT_OFFSET_Y, 0xA1, &font12,
			BLACK, WHITE); /* "▲" */
	y +=  SELECT_ROW_HEIGHT;
	Paint_DrawLine(0, y, EPD_1IN54_V2_WIDTH, y,
		       BLACK, SELECT_ROW_LINE_WIDTH, LINE_STYLE_SOLID);

	y += SELECT_ROW_LINE_WIDTH;
	Paint_DrawRectangle(0, y, EPD_1IN54_V2_WIDTH, y + SELECT_ROW_HEIGHT,
			    BLACK, 1, DRAW_FILL_FULL);
	Paint_DrawjChar(ARROW_OFFSET_X, y + TEXT_OFFSET_Y, 0xA2, &font12,
			WHITE, BLACK); /* "▶" */

	y += SELECT_ROW_HEIGHT;
	Paint_DrawLine(0, y, EPD_1IN54_V2_WIDTH, y,
		       BLACK, SELECT_ROW_LINE_WIDTH, LINE_STYLE_SOLID);
	y += SELECT_ROW_LINE_WIDTH;
	Paint_DrawjChar(ARROW_OFFSET_X, y + TEXT_OFFSET_Y, 0xA3, &font12,
			BLACK, WHITE); /* "▼" */
	y += SELECT_ROW_HEIGHT;
	Paint_DrawLine(0, y, EPD_1IN54_V2_WIDTH, y,
		       BLACK, SELECT_ROW_LINE_WIDTH, LINE_STYLE_SOLID);

	Paint_DrawString_j(0, EPD_1IN54_V2_HEIGHT - 12,
			   "\xC2\xA1PREVIOUS: Double press", &font12, 0, BLACK, WHITE);
	Paint_DrawString_j(0, EPD_1IN54_V2_HEIGHT - 12*2,
			   "\xC2\xA3NEXT: Short press", &font12, 0, BLACK, WHITE);
	Paint_DrawString_j(0, EPD_1IN54_V2_HEIGHT - 12*3,
			   "\xC2\xA2SELECT: Long press", &font12, 0, BLACK, WHITE);
}


static void menu_clear(void)
{
	int y = 0;
	printf("menu: clear\n");

	/* Title */
	Paint_ClearWindows(0, y, EPD_1IN54_V2_WIDTH,
			   y + HEADER_HEIGHT -HEADER_LINE_WIDTH, WHITE);

	/* Wheel 1st row */
	y += HEADER_HEIGHT + HEADER_LINE_WIDTH;
	Paint_ClearWindows(TEXT_OFFSET_X, y + TEXT_OFFSET_Y, EPD_1IN54_V2_WIDTH,
			   y + TEXT_OFFSET_Y + font12.height, WHITE);

	/* Wheel 2nd row */
	y += SELECT_ROW_LINE_WIDTH + SELECT_ROW_HEIGHT;
	Paint_ClearWindows(TEXT_OFFSET_X, y + TEXT_OFFSET_Y, EPD_1IN54_V2_WIDTH,
			   y + TEXT_OFFSET_Y + font12.height, BLACK);

	/* Wheel 3rd row */
	y += SELECT_ROW_LINE_WIDTH + SELECT_ROW_HEIGHT;
	Paint_ClearWindows(TEXT_OFFSET_X, y + TEXT_OFFSET_Y, EPD_1IN54_V2_WIDTH,
			   y + TEXT_OFFSET_Y + font12.height, WHITE);

	/* Help zone */
	y += SELECT_ROW_LINE_WIDTH + SELECT_ROW_HEIGHT;
	Paint_ClearWindows(TEXT_OFFSET_HELP_X, y + TEXT_OFFSET_Y, EPD_1IN54_V2_WIDTH,
			   y + TEXT_OFFSET_Y + font12.height*2, WHITE);

}


static int items_len(struct item *items)
{
	int len = 0;

	for (int i = 0 ; i < MAX_LIST_SIZE ; i++) {
		if (items[i].name != NULL) {
			len++;
		} else {
			break;
		}
	}
	return len;
}

static void wheel_draw_text_row(int row, char *str)
{
	int y = HEADER_HEIGHT + HEADER_LINE_WIDTH; /* Wheel 1st row */
	int fcolor = BLACK, bcolor = WHITE;

	if (row < 1 || row > 3) {
		printf("Error wrong row num %d\n", row);
	}

	if (row >= 2) {
		/* Wheel 2nd row */
		y += SELECT_ROW_LINE_WIDTH + SELECT_ROW_HEIGHT;
	}

	if (row == 2) {
		fcolor = WHITE;
		bcolor = BLACK;
	}

	if (row >= 3) {
		/* Wheel 3rd row */
		y += SELECT_ROW_LINE_WIDTH + SELECT_ROW_HEIGHT;
	}

	if (str == NULL) {
		str = "Back \xC2\xA5";
	}

	Paint_DrawString_j(TEXT_OFFSET_X, y + TEXT_OFFSET_Y, str,
			   &font12, 0, fcolor, bcolor);
}

static void wheel_draw_text(char *row1, char *row2, char *row3)
{
	wheel_draw_text_row(1, row1);
	wheel_draw_text_row(2, row2);
	wheel_draw_text_row(3, row3);
}

static void menu_draw_main(int menu_idx, int item_idx)
{
	int y = 0;
	int ilen = 0;

	if (item_idx == UNSELECTED) {
		/* Main MENU */
		if (menu_idx < 0 || menu_idx > MENU_LENGTH) {
			printf("ERROR: WRONG menu_idx %d\n", menu_idx);
			return;
		}
		Paint_DrawString_EN(70, y, "MENU", &Font20, BLACK, WHITE);

		/* Select wheel */
		printf("menu: menu_idx %d UNSELECTED\n", menu_idx);
		wheel_draw_text(
			g_menu[mod(menu_idx - 1, MENU_LENGTH)].name,
			g_menu[menu_idx].name,
			g_menu[mod(menu_idx + 1, MENU_LENGTH)].name);

	} else {
		/* Sub menu */
		if (menu_idx < 0 || menu_idx > MENU_LENGTH) {
			printf("ERROR: WRONG menu_idx %d\n", menu_idx);
			return;
		}

		Paint_DrawString_j(2, y, g_menu[menu_idx].name,
				   &font12, 0, BLACK, WHITE);

		ilen = items_len(g_menu[menu_idx].items) + 1;
		printf("items len %d\n", ilen);

		/* Select wheel */
		printf("menu: menu_idx %d item_idx %d\n", menu_idx, item_idx);
		for (int i = -1 ; i <= 1 ; i++) {
			int idx = mod(item_idx + i, ilen);
			char sname[MAX_ITEM_NAME_LEN] = {0};
			char *name = g_menu[menu_idx].items[idx].name;
			if (name && (*g_menu[menu_idx].value
				     == g_menu[menu_idx].items[idx].value)) {
				/* Item selected add '✔' */
				strncpy(sname, name, MAX_ITEM_NAME_LEN);
				strncat(sname, " \xC2\xA4", MAX_ITEM_NAME_LEN-2);
				name = sname;
			}
			wheel_draw_text_row(i + 2, name);
		}
	}

	/* Help */
	y += HEADER_HEIGHT + HEADER_LINE_WIDTH;
	y += 3*SELECT_ROW_LINE_WIDTH + 3*SELECT_ROW_HEIGHT;
	Paint_DrawString_j(TEXT_OFFSET_HELP_X, y+ TEXT_OFFSET_Y,
			   g_menu[menu_idx].help,
			   &font12, 0, BLACK, WHITE);
}

enum menu_handle_ret {
	NEED_REFRESH = 1,
	NO_REFRESH = 2,
	EXIT = 3,
};

static enum menu_handle_ret menu_handle_button_pressed(
	enum button_event button_event,
	int *menu_idx, int *item_idx)
{
	printf("menu: handle %d %d %d\n", button_event, *menu_idx, *item_idx);
	if (*item_idx == UNSELECTED) {
		/* Main MENU */
		if (button_event == BUTTON_EVENT_SINGLE_PRESS) {
			*menu_idx = mod(*menu_idx + 1, MENU_LENGTH);
			return NEED_REFRESH;
		} else if (button_event == BUTTON_EVENT_DOUBLE_PRESS) {
			*menu_idx = mod(*menu_idx - 1, MENU_LENGTH);
			return NEED_REFRESH;
		} else if (button_event == BUTTON_EVENT_LONG_PRESS) {
			switch(g_menu[*menu_idx].type) {
			case BACK:
				return EXIT;
			default:
				printf("enter menu\n");
				*item_idx = 0 ; //ENTERED; /* Enter submenu */
			}
			return NEED_REFRESH;
		}
	} else {
		int ilen = items_len(g_menu[*menu_idx].items) + 1;
		if (button_event == BUTTON_EVENT_SINGLE_PRESS) {
			*item_idx = mod(*item_idx + 1, ilen);
			return NEED_REFRESH;
		} else if (button_event == BUTTON_EVENT_DOUBLE_PRESS) {
			*item_idx = mod(*item_idx - 1, ilen);
			return NEED_REFRESH;
		} else if (button_event == BUTTON_EVENT_LONG_PRESS) {
			if (g_menu[*menu_idx].items[*item_idx].name == NULL) {
				/* Special for "back" */
				*item_idx = UNSELECTED;
				return NEED_REFRESH;
			}
			printf("value selected: %d written on %p\n", g_menu[*menu_idx].items[*item_idx].value, g_menu[*menu_idx].value);
			*(g_menu[*menu_idx].value) = g_menu[*menu_idx].items[*item_idx].value;
			*item_idx = UNSELECTED;
			return NEED_REFRESH;
		}
	}

	return NO_REFRESH;
}

void menu_enter(void)
{
	int menu_idx = 0;
	int item_idx = UNSELECTED;
	enum menu_handle_ret ret;

	button_init(&g_button_fsm);
	printf("menu: Init EPD %ld ms %p %ld\n", rtc_get_ms(), gImage, g_ts_ms_last_button_pressed);
	EPD_1IN54_V2_Init();
	//EPD_1IN54_V2_Clear();

	menu_draw_base_image();
	menu_draw_main(menu_idx, item_idx);
	EPD_1IN54_V2_DisplayPartBaseImage(gImage);
	EPD_1IN54_V2_Init_Partial();
	//EPD_1IN54_V2_Display(gImage);

	/* menu_draw_main(menu_idx, item_idx); */
	/* EPD_1IN54_V2_DisplayPart(gImage); */

	while (1) {
		enum button_event evt = button_tick(&g_button_fsm, rtc_get_ms());

		if (evt != BUTTON_EVENT_NONE) {
			if (evt == BUTTON_EVENT_SINGLE_PRESS) {
				printf("button SHORT PRESS fsm %d\n",
				       g_button_fsm.state);
			} else if (evt == BUTTON_EVENT_DOUBLE_PRESS){
				printf("button DOUBLE PRESS fsm %d\n",
				       g_button_fsm.state);
			} else if (evt == BUTTON_EVENT_LONG_PRESS){
				printf("button LONG PRESS fsm %d\n",
				       g_button_fsm.state);
			}
			ret = menu_handle_button_pressed(
				evt, &menu_idx, &item_idx);
			if (ret == NEED_REFRESH) {
				menu_clear();
				menu_draw_main(menu_idx, item_idx);
				EPD_1IN54_V2_DisplayPart(gImage);
			} else if (ret == EXIT) {
				break;
			}
		} else {
			HAL_Delay(10);
		}

		if (rtc_get_ms() - g_ts_ms_last_button_pressed > 30000) {
			printf("Too long in menu exiting\n");
			break;
		}
	}
	Paint_Clear(WHITE);
}
