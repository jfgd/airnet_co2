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

#include "EPD_1in54_V2.h"
#include "GUI_Paint.h"

extern volatile uint32_t g_ts_ms_last_button_pressed;
extern volatile uint32_t g_ts_ms_previous_button_pressed;
extern volatile int g_button_pressed_flag;

extern UBYTE gImage[];

extern uint32_t rtc_get_ms(void);

static int mod(int a, int b)
{
    int r = a % b;
    return r < 0 ? r + b : r;
}

enum temperature_unit {
	CONF_TEMP_CELSIUS = 0,
	CONF_TEMP_FAHRENHEIT = 1,
};

/* Only int allowed */
struct {
	int refresh_rate_sec; // = 5;
	enum temperature_unit temperature_unit;
} g_conf;


/* struct s_menu; */

/* enum menu_type { */
/* 	SUB_MENU = 1, */
/* 	SELECTION_LIST = 2, */
/* }; */

enum value_type {
	SELECT = 1,
	ACTION = 2,
	BACK = 3,
};

#define MAX_LIST_SIZE 6

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
		.name = "Temperature Unit",
		.help = "Select °C or °F",
		.type = SELECT,
		.value = (int*)&g_conf.temperature_unit,
		.items = {
			{ .value = CONF_TEMP_CELSIUS, .name = "°C" },
			{ .value = CONF_TEMP_FAHRENHEIT, .name = "°F" },
		},
	},
	{
		.name = "Exit",
		.help = "Exit configuration menu",
		.type = BACK,
	},
};



#define MENU_LENGTH (int)(sizeof(g_menu) / sizeof(g_menu[0]))

enum menu_button {
	NONE = 0,
	SHORT_PRESS = 1,
	DOUBLE_PRESS = 2,
	LONG_PRESS =3,
};

enum item_value {
	UNSELECTED = -2,
	//ENTERED = -1,
};

#define HEADER_HEIGHT 22
#define HEADER_LINE_WIDTH 2
#define SELECT_ROW_HEIGHT 22
#define SELECT_ROW_LINE_WIDTH 1
#define TEXT_OFFSET_Y 6
#define TEXT_OFFSET_X 15

static void menu_draw_base_image(void)
{
	int y = 0;

	Paint_SelectImage(gImage);
	Paint_Clear(WHITE);

	y += HEADER_HEIGHT;
	Paint_DrawLine(0, y, EPD_1IN54_V2_WIDTH, y,
		       BLACK, HEADER_LINE_WIDTH, LINE_STYLE_SOLID);

	y += HEADER_LINE_WIDTH + SELECT_ROW_HEIGHT;
	Paint_DrawLine(0, y, EPD_1IN54_V2_WIDTH, y,
		       BLACK, SELECT_ROW_LINE_WIDTH, LINE_STYLE_SOLID);
	y += SELECT_ROW_LINE_WIDTH;
	Paint_DrawRectangle(0, y, EPD_1IN54_V2_WIDTH, y + SELECT_ROW_HEIGHT,
			    BLACK, 1, DRAW_FILL_FULL);
	y += SELECT_ROW_HEIGHT;
	Paint_DrawLine(0, y, EPD_1IN54_V2_WIDTH, y,
		       BLACK, SELECT_ROW_LINE_WIDTH, LINE_STYLE_SOLID);
	y += SELECT_ROW_LINE_WIDTH + SELECT_ROW_HEIGHT;
	Paint_DrawLine(0, y, EPD_1IN54_V2_WIDTH, y,
		       BLACK, SELECT_ROW_LINE_WIDTH, LINE_STYLE_SOLID);

	Paint_DrawString_EN(0, EPD_1IN54_V2_HEIGHT - 12, "PREVIOUS: Double press", &Font12, BLACK, WHITE);
	Paint_DrawString_EN(0, EPD_1IN54_V2_HEIGHT - 12*2, "NEXT: Short press", &Font12, BLACK, WHITE);
	Paint_DrawString_EN(0, EPD_1IN54_V2_HEIGHT - 12*3, "SELECT: Long press", &Font12, BLACK, WHITE);
}


static void menu_clear(void)
{
	int y = 0;
	printf("menu: clear\n");

	Paint_ClearWindows(0, y, EPD_1IN54_V2_WIDTH,
			   y + HEADER_HEIGHT -HEADER_LINE_WIDTH, WHITE);
	y += HEADER_HEIGHT + HEADER_LINE_WIDTH;
	Paint_ClearWindows(TEXT_OFFSET_X, y + TEXT_OFFSET_Y, EPD_1IN54_V2_WIDTH,
			   y + TEXT_OFFSET_Y + Font12.Height, WHITE);
	y += SELECT_ROW_LINE_WIDTH + SELECT_ROW_HEIGHT;
	Paint_ClearWindows(TEXT_OFFSET_X, y + TEXT_OFFSET_Y, EPD_1IN54_V2_WIDTH,
			   y + TEXT_OFFSET_Y + Font12.Height, BLACK);
	y += SELECT_ROW_LINE_WIDTH + SELECT_ROW_HEIGHT;
	Paint_ClearWindows(TEXT_OFFSET_X, y + TEXT_OFFSET_Y, EPD_1IN54_V2_WIDTH,
			   y + TEXT_OFFSET_Y + Font12.Height, WHITE);
	y += SELECT_ROW_LINE_WIDTH + SELECT_ROW_HEIGHT;
	Paint_ClearWindows(TEXT_OFFSET_X, y + TEXT_OFFSET_Y, EPD_1IN54_V2_WIDTH,
			   y + TEXT_OFFSET_Y + Font12.Height, WHITE);

}


static void menu_draw_main(int menu_idx, int item_idx)
{
	int y = 0;
	struct menu_list *m;
	struct item *itm;
	int items_len = 0;

	if (item_idx == UNSELECTED) {
		/* Main MENU */
		if (menu_idx < 0 || menu_idx > MENU_LENGTH) {
			printf("ERROR: WRONG menu_idx %d\n", menu_idx);
			return;
		}
		Paint_DrawString_EN(70, y, "MENU", &Font20, BLACK, WHITE);

		/* Select wheel */
		y += HEADER_HEIGHT + HEADER_LINE_WIDTH;
		printf("menu: menu_idx %d UNSLECTED\n", menu_idx);
		m = &g_menu[mod(menu_idx - 1, MENU_LENGTH)];
		Paint_DrawString_EN(TEXT_OFFSET_X, y + TEXT_OFFSET_Y, m->name,
				    &Font12, BLACK, WHITE);
		y += SELECT_ROW_LINE_WIDTH + SELECT_ROW_HEIGHT;
		m = &g_menu[menu_idx];
		Paint_DrawString_EN(TEXT_OFFSET_X, y + TEXT_OFFSET_Y, m->name,
				    &Font12, WHITE, BLACK);
		y += SELECT_ROW_LINE_WIDTH + SELECT_ROW_HEIGHT;
		m = &g_menu[mod(menu_idx + 1, MENU_LENGTH)];
		Paint_DrawString_EN(TEXT_OFFSET_X, y + TEXT_OFFSET_Y, m->name,
				    &Font12, BLACK, WHITE);

		/* Help */
		y += SELECT_ROW_LINE_WIDTH + SELECT_ROW_HEIGHT;
		m = &g_menu[menu_idx];
		Paint_DrawString_EN(TEXT_OFFSET_X, y+ TEXT_OFFSET_Y,
				    g_menu[menu_idx].help,
				    &Font12, BLACK, WHITE);
	} else {
		if (menu_idx < 0 || menu_idx > MENU_LENGTH) {
			printf("ERROR: WRONG menu_idx %d\n", menu_idx);
			return;
		}

		Paint_DrawString_EN(2, y, g_menu[menu_idx].name,
				    &Font20, BLACK, WHITE);

		items_len = (sizeof(g_menu[menu_idx].items) / sizeof(g_menu[menu_idx].items[0]));
		//itm = &g_menu[menu_idx].items[item_idx];

		/* Select wheel */
		y += HEADER_HEIGHT + HEADER_LINE_WIDTH;
		printf("menu: menu_idx %d item_idx %d\n", menu_idx, item_idx);
		itm = &g_menu[menu_idx].items[mod(item_idx - 1, items_len)];
		Paint_DrawString_EN(TEXT_OFFSET_X, y + TEXT_OFFSET_Y, itm->name,
				    &Font12, BLACK, WHITE);
		y += SELECT_ROW_LINE_WIDTH + SELECT_ROW_HEIGHT;
		itm = &g_menu[menu_idx].items[item_idx];
		Paint_DrawString_EN(TEXT_OFFSET_X, y + TEXT_OFFSET_Y, itm->name,
				    &Font12, WHITE, BLACK);
		y += SELECT_ROW_LINE_WIDTH + SELECT_ROW_HEIGHT;
		itm = &g_menu[menu_idx].items[mod(item_idx + 1, items_len)];
		Paint_DrawString_EN(TEXT_OFFSET_X, y + TEXT_OFFSET_Y, itm->name,
				    &Font12, BLACK, WHITE);
	}
}

enum menu_handle_ret {
	NEED_REFRESH = 1,
	NO_REFRESH = 2,
	EXIT = 3,
};

static enum menu_handle_ret menu_handle_button_pressed(
	enum menu_button button_pressed,
	int *menu_idx, int *item_idx)
{
	printf("menu: handle %d %d %d\n", button_pressed, *menu_idx, *item_idx);
	if (*item_idx == UNSELECTED) {
		/* Main MENU */
		if (button_pressed == SHORT_PRESS) {
			*menu_idx = (*menu_idx + 1) % MENU_LENGTH;
			return NEED_REFRESH;
		} else if (button_pressed == LONG_PRESS) {
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
		int items_len = (sizeof(g_menu[*menu_idx].items) / sizeof(g_menu[*menu_idx].items[0]));
		if (button_pressed == SHORT_PRESS) {
			*item_idx = (*item_idx + 1) % items_len;
			return NEED_REFRESH;
		} else if (button_pressed == LONG_PRESS) {
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
	enum menu_button button_pressed = NONE;
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
		if (g_button_pressed_flag) {
			button_pressed = SHORT_PRESS;
			uint32_t press_time_ms = 0;
			g_button_pressed_flag = 0;
			while (BUTTON_GPIO_STATE() == 0) {
				press_time_ms =
					rtc_get_ms() - g_ts_ms_last_button_pressed;
				if (press_time_ms > 800) {
					button_pressed = LONG_PRESS;
					printf("menu: Long press button %ld\n",
					       press_time_ms);
					break;
				}
				HAL_Delay(10);
			}
			printf("menu: BUTTON %d\n", button_pressed);
		}

		if (button_pressed) {
			ret = menu_handle_button_pressed(
				button_pressed, &menu_idx, &item_idx);
			if (ret == NEED_REFRESH) {
				menu_clear();
				menu_draw_main(menu_idx, item_idx);
				EPD_1IN54_V2_DisplayPart(gImage);
			} else if (ret == EXIT) {
				break;
			}
			button_pressed = NONE;
		}

		if (rtc_get_ms() - g_ts_ms_last_button_pressed > 30000) {
			printf("Too long in menu exiting\n");
			break;
		}
	}
	Paint_Clear(WHITE);
}
