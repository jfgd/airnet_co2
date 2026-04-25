/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>

#include "skin.h"
#include "menu.h"

#include "stcc4_i2c.h"
#include "sensirion_i2c_hal.h"
#include "sensirion_common.h"

#include "EPD_1in54_V2.h"
#include "GUI_Paint.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

extern uint32_t rtc_get_ms(void);
#ifdef DEBUG_PRINT
#define TS(f)                                                           \
  {                                                                     \
    uint32_t _ts_ms = rtc_get_ms();                                     \
    f;                                                                  \
    printf("Duration %s %s:%d : %ld ms\n", #f, __FILE_NAME__, __LINE__, \
           rtc_get_ms() - _ts_ms);                                      \
  }
#else
#define TS(f)  f;
#endif


/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;

I2C_HandleTypeDef hi2c1;

RTC_HandleTypeDef hrtc;

SPI_HandleTypeDef hspi1;

TIM_HandleTypeDef htim3;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */

volatile uint32_t g_ts_ms_last_button_pressed = 0;
volatile uint32_t g_ts_ms_previous_button_pressed = 0;
volatile int g_button_pressed_flag = 0;

uint8_t gImage[5000];

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ADC1_Init(void);
static void MX_I2C1_Init(void);
static void MX_RTC_Init(void);
static void MX_SPI1_Init(void);
static void MX_TIM3_Init(void);
static void MX_USART1_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

#ifdef DEBUG_PRINT
int __io_putchar(int ch)
{
  HAL_UART_Transmit(&huart1, (uint8_t *) &ch, 1, 1000);
  return ch;
}
#endif  /* DEBUG_PRINT */

static inline void led_red_on(void)
{
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
}

static inline void led_red_off(void)
{
  HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_1);
}

static inline void led_green_on(void)
{
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
}

static inline void led_green_off(void)
{
  HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_2);
}

static inline void led_yellow_on(void)
{
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3);
}

static inline void led_yellow_off(void)
{
  HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_3);
}

static void led_roll(int per_led_delay_ms, int iterations)
{
  for (int i = 0 ; i < iterations ; i++) {
    led_red_on();
    HAL_Delay(per_led_delay_ms);
    led_green_on();
    led_red_off();
    HAL_Delay(per_led_delay_ms);
    led_yellow_on();
    led_green_off();
    HAL_Delay(per_led_delay_ms);
    led_yellow_off();
  }
}

static inline void epd_power_on(void)
{
#ifndef DEBUG_NO_EPD
  HAL_GPIO_WritePin(EPD_ENABLE_GPIO_Port, EPD_ENABLE_Pin, GPIO_PIN_SET);
#endif  /* not DEBUG_NO_EPD */
}

static inline void epd_power_off(void)
{
#ifndef DEBUG_NO_EPD
  HAL_GPIO_WritePin(EPD_ENABLE_GPIO_Port, EPD_ENABLE_Pin, GPIO_PIN_RESET);
#endif  /* not DEBUG_NO_EPD */
}

static void epd_wait_busy_led(void)
{
  /* LOW: idle, HIGH: busy */
  while(HAL_GPIO_ReadPin(EPD_BUSY_GPIO_Port, EPD_BUSY_Pin) == 1) {
      led_roll(100, 1);
  }
}

uint32_t rtc_get_tick(void)
{
  RTC_TimeTypeDef time;
  if (HAL_RTC_GetTime(&hrtc, &time, 0) != HAL_OK) {
    printf("Error get time\n");
  }
  return UINT32_MAX - time.SubSeconds;
}

uint32_t rtc_ticks_to_ms(uint32_t ticks)
{
  return ticks * ((1000 * (127 + 1)) / 32000);
}

uint32_t rtc_get_ms(void)
{
  return rtc_ticks_to_ms(rtc_get_tick());
}

void HAL_GPIO_EXTI_Falling_Callback(uint16_t GPIO_Pin)
{
  if (GPIO_Pin == BUTTON_Pin) {
    g_ts_ms_previous_button_pressed = g_ts_ms_last_button_pressed;
    g_ts_ms_last_button_pressed = rtc_get_ms();
    g_button_pressed_flag = 1;
    printf("button pressed %ld %ld\n", g_ts_ms_last_button_pressed, g_ts_ms_previous_button_pressed);
  }
}

static void draw_logo(uint8_t *image) {
  UNUSED(image);
  int y_pos = 50;
  Paint_DrawString_j(4, y_pos, "AirNet", &Airnet40NotoSansSemiCondensedBoldItalic,
                     -3, BLACK, WHITE);
  Paint_DrawString_j(111, y_pos-7, "CO²", &CO253NotoSansExtraBold,
                     -2, BLACK, WHITE);
}

void DEV_SPI_WriteByte(UBYTE value)
{
#ifndef DEBUG_NO_EPD
  HAL_StatusTypeDef status;
  status = HAL_SPI_Transmit(&hspi1, &value, 1, 1000);
  if (status != HAL_OK) {
    printf("Error DEV_SPI_WriteByte HAL_SPI_Transmit %d\n", status);
  }
#else
  UNUSED(value);
#endif  /* not DEBUG_NO_EPD */
}

static void enter_stop2(void)
{
  /* TODO: Set GPIO to analog */
  HAL_SuspendTick();
  HAL_PWREx_EnableUltraLowPowerMode();
  HAL_PWREx_EnterSTOP2Mode(PWR_STOPENTRY_WFI);

  /* Stop 2 */

  SystemClock_Config();
  HAL_ResumeTick();
  /* TODO: reconfigure GPIO */
}

static void read_data_and_draw(int display)
{
  uint32_t ts_ms;
  uint32_t ts_ms_start;
  int16_t err = 0;

  uint16_t co2_ppm = 0;
  uint32_t temperature = 0;
  uint32_t humidity = 0;
  uint32_t vbat_mv = 0;

  int16_t co2_concentration_raw = 0;
  uint16_t temperature_raw = 0;
  uint16_t relative_humidity_raw = 0;
  uint16_t sensor_status_raw = 0;
  uint32_t adc_val;

  ts_ms_start = rtc_get_ms();   /* t = 0 ms */

  /* Exit STCC4 sleep */
  err = stcc4_exit_sleep_mode_nowait(); /* Must wait 5 ms */
  ts_ms = rtc_get_ms();
  if (err != NO_ERROR) {
    printf("Error stcc4_exit_sleep_mode\n");
    Error_Handler();
  }

  /* Start reading bat voltage */
  if (HAL_ADC_Start(&hadc1) != HAL_OK)
  {
      Error_Handler();
  }

  HAL_ADC_PollForConversion(&hadc1, 10000);
  adc_val = HAL_ADC_GetValue(&hadc1);
  vbat_mv = (adc_val * 3300) / 4095;
  printf("adc %ld %ld mV %ld ms\n", adc_val, vbat_mv,rtc_get_ms());
  HAL_ADC_Stop(&hadc1);

  /* Wait exit sleep done */
  while (rtc_get_ms() - ts_ms < 5) {
    printf("wait exit sleep %ld\n", rtc_get_ms() - ts_ms);
  }
  /* t ~= 5 ms */

  /* Launch measure single shot */
  err = stcc4_measure_single_shot_nowait(); /* Must wait 500 ms */
  ts_ms = rtc_get_ms();
  if (err != NO_ERROR) {
    printf("Error stcc4_measure_single_shot\n");
    Error_Handler();
  }

  if (display) {
    TS(EPD_1IN54_V2_Init_Partial()); /* 524 ms Wake up */
  }

  /* Wait measure single done */
  while (rtc_get_ms() - ts_ms < 500) {
    printf("wait measure single %ld\n", rtc_get_ms() - ts_ms);
    HAL_Delay(25);
  }
  /* t ~= 530 ms */
  printf("t = %ld\n", rtc_get_ms() - ts_ms_start);

  /* Read measurement */
  err = stcc4_read_measurement_raw(&co2_concentration_raw, &temperature_raw,
                                   &relative_humidity_raw, &sensor_status_raw);
  if (err != NO_ERROR) {
#ifndef DEBUG_NO_SENSORS
    printf("Error stcc4_read_measurement_raw retrying in 150ms\n");
    HAL_Delay(150);
    err = stcc4_read_measurement_raw(&co2_concentration_raw, &temperature_raw,
                                     &relative_humidity_raw, &sensor_status_raw);
    if (err != NO_ERROR) {
      printf("Error stcc4_read_measurement_raw\n");
      Error_Handler();
    }
#endif    /*  not DEBUG_NO_SENSORS */
  }
  printf("t = %ld\n", rtc_get_ms() - ts_ms_start);

  /* Sleep sensor */
  err = stcc4_enter_sleep_mode_nowait(); /* Must wait 1 ms */
  if (err != NO_ERROR) {
    printf("Error stcc4_enter_sleep_mode\n");
  }

  co2_ppm = co2_concentration_raw;
  temperature = ((175 * (uint32_t)temperature_raw) / 655) - 4500; /* in c°C */
  humidity = ((125 * (uint32_t)relative_humidity_raw) / 65535) - 6;
  printf("sensor: co2 is %02dppm.\n", co2_ppm);
  printf("sensor: temperature is %ld cC, 0x%lx\n", temperature, temperature);
  printf("sensor: humidity is %ld, 0x%lx\n", humidity, humidity);


  TS(skin_update(gImage, co2_ppm, temperature, humidity, vbat_mv)); /* 260 ms ! */

  printf("Loop duration : %ld ms\n", rtc_get_ms() - ts_ms_start);

}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  uint32_t ts_ms_start = 0;
  int16_t err = 0;
  uint32_t stcc4_product_id;
  uint64_t stcc4_sn;

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_ADC1_Init();
  MX_I2C1_Init();
  MX_RTC_Init();
  MX_SPI1_Init();
  MX_TIM3_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */

  printf("\n\nHello from AirNet CO2 %ld ms\n", rtc_get_ms());

  stcc4_init(STCC4_I2C_ADDR_64);

  if (stcc4_stop_continuous_measurement_nowait() != NO_ERROR) { /* 1200 ms wait */
    printf("error executing stop_continuous_measurement()\n");
    Error_Handler();
  }
  ts_ms_start = rtc_get_ms();
  /* Need to wait 1200 ms before next stcc4 cmd */

  /* Prepare image */
  Paint_NewImage(gImage, EPD_1IN54_V2_WIDTH, EPD_1IN54_V2_HEIGHT, 270, WHITE);
  Paint_SelectImage(gImage);
  Paint_Clear(WHITE);
  draw_logo(gImage);
  Paint_DrawString_j((EPD_1IN54_V2_WIDTH-(sizeof(VERSION)-1)*font12.max_width)/2,
		     130, VERSION, &font12, 0, BLACK, WHITE);

  /* Start screen */
  epd_power_on();
  printf("EPD init %ld ms\n", rtc_get_ms());
  EPD_1IN54_V2_Init();          /* 448 ms */
  EPD_1IN54_V2_DisplayAsync(gImage); /* 216 ms, 2040 ms read busy ok*/

  /* Wait STCC4 ready */
  while (rtc_get_ms() - ts_ms_start < 1200) {
    printf("STCC4 not ready %ld\n", rtc_get_ms() - ts_ms_start);
    led_roll(100, 1);
  }

  /* Now STCC4 should be ready */
  err = stcc4_get_product_id(&stcc4_product_id, &stcc4_sn);
  if (err != NO_ERROR) {
#ifndef DEBUG_NO_SENSORS
    printf("error reading stcc4 product_id\n");
    Error_Handler();
#endif    /*  not DEBUG_NO_SENSORS */
  }
  printf("stcc4 pid 0x%lx 0x%lx\n", stcc4_product_id, (uint32_t)stcc4_sn);

  /* Wait for screen to finish display image */
  epd_wait_busy_led();
  /* We are now around 2528 ms after ts_ms_start */

  /* Wait a bit more so image is visible */
  while (rtc_get_ms() < 4000) {
    led_roll(100, 1);
  }

  /* Prepare data display */
  TS(skin_prepare(gImage));     /* 36 ms */

  /* Do a first read and display */
  read_data_and_draw(0);

  /* Schedule RTC wake up */
  /* TODO */

  /* Display */
  EPD_1IN54_V2_DisplayPartBaseImage(gImage);

  enter_stop2();

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  printf("\n\n");
  while (1)
  {
    int long_press = 0;
    if (g_button_pressed_flag) {
      uint32_t press_time_ms = 0;
      g_button_pressed_flag = 0;
      while (BUTTON_GPIO_STATE() == 0) {
        press_time_ms = rtc_get_ms() - g_ts_ms_last_button_pressed;
        if (press_time_ms > 800) {
          long_press = 1;
          printf("Long press button %ld\n", press_time_ms);
          break;
        }
        HAL_Delay(10);
      }
    }
    if (long_press) {
      menu_enter();
      continue;
    }
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    /* uint32_t start_ms = rtc_get_ms(); */
    /* if ((start_ms - ts_ms_sensors_read) < 4900) { */
    /*   printf("Looped to soon skip\n"); */
    /*   HAL_Delay(200); */
    /*   continue; */
    /* } */

    printf("\n\n");

    read_data_and_draw(1);

    epd_power_on();
    //EPD_1IN54_V2_Init_Partial(); /* Wake up */
    printf("EPD init partial done %ld ms\n", rtc_get_ms());
    uint32_t ts_disp_start = rtc_get_ms();
    EPD_1IN54_V2_DisplayPart(gImage); /* 812 ms */
    printf("EPD display done %ld ms (%ld ms)\n", rtc_get_ms(),
	   rtc_get_ms() - ts_disp_start);
    printf("EPD sleep %ld ms\n", rtc_get_ms());
    EPD_1IN54_V2_Sleep();
    //epd_power_off();

    /* Going to sleep */
    enter_stop2();

  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE2);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV1;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc1.Init.LowPowerAutoWait = DISABLE;
  hadc1.Init.LowPowerAutoPowerOff = DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.Overrun = ADC_OVR_DATA_PRESERVED;
  hadc1.Init.SamplingTimeCommon1 = ADC_SAMPLETIME_1CYCLE_5;
  hadc1.Init.SamplingTimeCommon2 = ADC_SAMPLETIME_1CYCLE_5;
  hadc1.Init.OversamplingMode = DISABLE;
  hadc1.Init.TriggerFrequencyMode = ADC_TRIGGER_FREQ_HIGH;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_4;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLINGTIME_COMMON_1;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

#ifdef DEBUG_NO_SENSORS
  /* Put I2C pin in analog to save power */
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Pin = GPIO_PIN_9|GPIO_PIN_10;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  return;
#endif /* DEBUG_NO_SENSORS */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = 0x00303D5B;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief RTC Initialization Function
  * @param None
  * @retval None
  */
static void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */

  /** Initialize RTC Only
  */
  hrtc.Instance = RTC;
  hrtc.Init.AsynchPrediv = 127;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutRemap = RTC_OUTPUT_REMAP_NONE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  hrtc.Init.OutPutPullUp = RTC_OUTPUT_PULLUP_NONE;
  hrtc.Init.BinMode = RTC_BINARY_ONLY;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }

  /** Enable the WakeUp
  */
  if (HAL_RTCEx_SetWakeUpTimer_IT(&hrtc, 5, RTC_WAKEUPCLOCK_CK_SPRE_16BITS, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RTC_Init 2 */

  /* USER CODE END RTC_Init 2 */

}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

#ifdef DEBUG_NO_EPD
  /* Put SPI pin + in analog to save power */
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Pin = GPIO_PIN_1|GPIO_PIN_7|EPD_ENABLE_Pin|EPD_CS_Pin|EPD_DC_Pin|EPD_RST_Pin|EPD_BUSY_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  return;
#endif /* DEBUG_NO_EPD */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_1LINE;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 7;
  hspi1.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  hspi1.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 0;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 65535;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 6500;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.Pulse = 1000;
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.Pulse = 6500;
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */
  HAL_TIM_MspPostInit(&htim3);

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

#ifndef DEBUG_PRINT
    return;
#endif  /* not DEBUG_PRINT */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart1, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart1, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, EPD_ENABLE_Pin|EPD_CS_Pin|EPD_DC_Pin|EPD_RST_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : EPD_ENABLE_Pin EPD_CS_Pin EPD_DC_Pin EPD_RST_Pin */
  GPIO_InitStruct.Pin = EPD_ENABLE_Pin|EPD_CS_Pin|EPD_DC_Pin|EPD_RST_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : EPD_BUSY_Pin nPGOOD_Pin */
  GPIO_InitStruct.Pin = EPD_BUSY_Pin|nPGOOD_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : BUTTON_Pin */
  GPIO_InitStruct.Pin = BUTTON_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(BUTTON_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI4_15_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI4_15_IRQn);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
    led_red_on();
    HAL_Delay(300);
    led_red_off();
    HAL_Delay(300);
    printf("error handler\n");
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
