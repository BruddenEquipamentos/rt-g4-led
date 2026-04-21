/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2023 STMicroelectronics.
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
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>

#include "beep.h"
#include "display.h"
#include "keyboard.h"
#include "modbus/master_operations.h"
#include "modbus/mb_master.h"
#include "modbus/mb_operation.h"
#include "nvm_emb.h"
#include "panel/rt_led.h"
#include "treadmill.h"
#include "treadmill_ble.h"
#include "treadmill_motor.h"
#include "ext_i2c_kbd.h"
#include "../shared/ble/ble_protocol/ble_protocol_helper.h"
#include "../shared/ble/ble_protocol/ble_ftms_treadmill.h"
#include "lang.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
uint32_t _tim_delta = 0;
uint32_t _tim6_count = 0;

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
struct stats_time {
  uint32_t start;
  uint32_t end;
  uint32_t delta;
  uint32_t average;
  uint32_t peak;
};

struct stats {
  struct stats_time mainloop;
};

struct stats stats;
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
uint32_t PolarICValue1 = 0;
uint32_t PolarICValue2 = 0;
uint32_t PolarICDiff = 0;
uint32_t ICCapturedPolarLastTick = 0;
uint32_t ICCapturedFirstTick = 0;
uint32_t PolarBPM = 0;

uint32_t ICCapturedWheelLastTick = 0;
uint32_t WheelICValue1 = 0;
uint32_t WheelICValue2 = 0;
uint32_t WheelICDiff = 0;

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

CRC_HandleTypeDef hcrc;

I2C_HandleTypeDef hi2c1;
I2C_HandleTypeDef hi2c2;

TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim6;
TIM_HandleTypeDef htim14;
TIM_HandleTypeDef htim16;
TIM_HandleTypeDef htim17;

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart3;
DMA_HandleTypeDef hdma_usart3_rx;

osThreadId defaultTaskHandle;
osThreadId keyboardTaskHandle;
osThreadId beepTaskHandle;
osMessageQId beepEventQueueHandle;
osTimerId beepTimerHandle;
/* USER CODE BEGIN PV */
treadmill_t treadmill;
treadmill_t *pTreadmill = &treadmill;

uint8_t __new_display = 0; 

volatile uint8_t _huart3_buffer[2] = {0};
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_I2C1_Init(void);
static void MX_TIM3_Init(void);
static void MX_USART3_UART_Init(void);
static void MX_ADC1_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_TIM16_Init(void);
static void MX_CRC_Init(void);
static void MX_TIM6_Init(void);
static void MX_TIM14_Init(void);
static void MX_TIM17_Init(void);
void StartDefaultTask(void const * argument);
void KeyboardTask(void const * argument);
void BeepTask(void const * argument);
void beepTimerCallback(void const * argument);

/* USER CODE BEGIN PFP */
static void beep(beep_t beep) { xQueueSend(beepEventQueueHandle, &beep, 0); }
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
#define ADC_SAMPLE_SIZE 16
#define ADC_BUFFER_SIZE (ADC_SAMPLE_SIZE * 1)
static uint32_t adc_values[ADC_BUFFER_SIZE];  // 2 is the number of channels
uint16_t adc_hrm = 0;
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc) {
  if (hadc->Instance == ADC1) {
    int sum = 0;
    for (int i = 0; i < ADC_SAMPLE_SIZE; ++i) sum += adc_values[i];
    adc_hrm = (uint16_t)(sum / ADC_SAMPLE_SIZE);
  }
}

uint32_t adc_average = 0;
uint32_t adc_average_n = 0;
uint32_t adc_hrm_average = 0;
uint32_t adc_peak_low = 0;
uint32_t adc_peak_high = 0;
uint32_t adc_peak_delta = 0;
uint32_t adc_peak_delta_average = 0;
uint32_t adc_peak_average = 0;
uint32_t adc_peak_tick = 0;
const uint32_t ADC_THRESHOLD = 100;
int adc_below_average = 0;
uint32_t adc_new_beat = 0;
uint32_t adc_old_beat = 0;
#define BPM_BUFFER_READS (16)
uint16_t bpm_buffer_index = 0;
uint32_t bpm_buffer_reads[BPM_BUFFER_READS] = {0};
uint32_t bpm_current_value = 0;
uint32_t adc_delay_tick_ms = 0;

void bpm_reset_variables(void) {
  adc_average = 0;
  adc_average_n = 0;
  adc_below_average = 0;
  adc_new_beat = 0;
  adc_old_beat = 0;
  bpm_buffer_index = 0;
  for (int i = 0; i < BPM_BUFFER_READS; ++i) bpm_buffer_reads[i] = 0;
  bpm_current_value = 0;
  adc_delay_tick_ms = 0;
  treadmill_set_bpm_value(&treadmill, 0);
  treadmill_set_bpm_stop(&treadmill, 0);
}

static void wait_ms(const int ms) { osDelay(ms); }

int ble_uart_read(uint8_t *buffer, int max_length) {
  uint8_t byte;
  int i = 0;
  for (i = 0; i < max_length; ++i) {
    if (HAL_UART_Receive(&huart3, &byte, 1, 0) == HAL_OK) {
      buffer[i] = byte;
    } else {
      break;
    }
  }
  return i;
}

int ble_uart_write(const uint8_t *data, int length) { return HAL_UART_Transmit(&huart3, data, length, 100) == HAL_OK; }

void ble_hw_reset(void) {
  HAL_GPIO_WritePin(CMD_GPIO_Port, CMD_Pin, GPIO_PIN_RESET);
  HAL_Delay(1);
  HAL_GPIO_WritePin(CMD_GPIO_Port, CMD_Pin, GPIO_PIN_SET);
}

volatile uint8_t motor_read_buffer[32] = {0};
int motor_uart_read(uint8_t *buffer, int max_length) {
  uint8_t byte;
  int i;
  for (i = 0; i < max_length; ++i) {
    if (HAL_UART_Receive(&huart1, &byte, 1, 10) != HAL_OK) {
      memset(motor_read_buffer, 0x00, 32);
      memcpy(motor_read_buffer, buffer, i);
      break;
    } else {
      buffer[i] = byte;
    }
  }
  return i;
}

int motor_uart_write(const uint8_t *data, int length) { return HAL_UART_Transmit(&huart1, data, length, 5) == HAL_OK; }

int motor_change_frequency(const uint16_t period) {
  if (period != TIM3->ARR) {
    TIM3->ARR = period;       // auto reload register
    TIM3->CCR4 = period / 2;  // pulse
    if (period != 0)
      HAL_TIM_OC_Start_IT(&htim3, TIM_CHANNEL_4);
    else
      HAL_TIM_OC_Stop_IT(&htim3, TIM_CHANNEL_4);
  }
  return 1;
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim) {
  if (htim->Instance == TIM17) {
    if (ICCapturedPolarLastTick == 0) {
      PolarICValue1 = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
      ICCapturedPolarLastTick = HAL_GetTick();
    } else {
      PolarICValue2 = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
      ICCapturedPolarLastTick = HAL_GetTick();
      if (PolarICValue2 > PolarICValue1) {
        PolarICDiff = PolarICValue2 - PolarICValue1;
        PolarBPM = (PolarBPM * 3 + (60000 / PolarICDiff)) / 4;
      }
      PolarICValue1 = PolarICValue2;
    }
  } else if (htim->Instance == TIM14) {
    if (ICCapturedWheelLastTick == 0) {
      WheelICValue1 = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
      ICCapturedWheelLastTick = HAL_GetTick();
    } else {
      WheelICValue2 = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
      ICCapturedWheelLastTick = HAL_GetTick();
      if (WheelICValue2 > WheelICValue1) {
        WheelICDiff = WheelICValue2 - WheelICValue1;
      }
      WheelICValue1 = WheelICValue2;
    }
  }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
  if (huart == &huart3) {
    treadmill_ble_push_byte(&treadmill, _huart3_buffer[1]);
  }
}

void HAL_UART_RxHalfCpltCallback(UART_HandleTypeDef *huart) {
  if (huart == &huart3) {
    treadmill_ble_push_byte(&treadmill, _huart3_buffer[0]);
  }
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart) {
  if (huart == &huart3) {
    HAL_UART_Receive_DMA(huart, (uint8_t *)&_huart3_buffer[0], 2);
  }
}

static int treadmill_update_fms(treadmill_t *t, const uint8_t status,
		const void *param) {
#define BUFSIZE 32
	uint8_t buffer[BUFSIZE] = { 0 };
	uint8_t i = 0;

	// todo: validate status
	buffer[i++] = BLE_PROTOCOL_MESSAGE_START;
	flat_bin_to_ascii_8(buffer, &i, BUFSIZE, BP_FMS_NOTIFY);
	flat_bin_to_ascii_8(buffer, &i, BUFSIZE, status);

	// params speed or incline, for instance
	switch (status) {
	case StoppedOrPausedByTheUser:
		flat_bin_to_ascii_8(buffer, &i, BUFSIZE, *(uint8_t*) param);
		break;
	case TargetSpeedChanged:
		flat_bin_to_ascii_16(buffer, &i, BUFSIZE, *(uint16_t*) param);
		break;
	case TargetInclineChanged:
		flat_bin_to_ascii_16(buffer, &i, BUFSIZE, *(int16_t*) param);
		break;
	}

	uint8_t cs = ble_protocol_checksum(buffer, i);
	flat_bin_to_ascii_8(buffer, &i, BUFSIZE, cs);
	buffer[i++] = BLE_PROTOCOL_MESSAGE_END;

	return t->hw.ble.send_data(buffer, i);
}


/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
  static const uint32_t APPLICATION_ADDRESS = 0x8005000;
  SCB->VTOR = APPLICATION_ADDRESS;
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */
  SET_BIT(SYSCFG->CFGR1, SYSCFG_CFGR1_UCPD1_STROBE);
  SET_BIT(SYSCFG->CFGR1, SYSCFG_CFGR1_UCPD2_STROBE);
  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_I2C1_Init();
  MX_TIM3_Init();
  MX_USART3_UART_Init();
  MX_ADC1_Init();
  MX_USART1_UART_Init();
  MX_TIM16_Init();
  MX_CRC_Init();
  MX_TIM6_Init();
  MX_TIM14_Init();
//  MX_TIM17_Init(); //Lucas Polar: remove to avoid ghost BPM
  MX_I2C2_Init();
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* USER CODE BEGIN RTOS_MUTEX */

  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* Create the timer(s) */
  /* definition and creation of beepTimer */
  osTimerDef(beepTimer, beepTimerCallback);
  beepTimerHandle = osTimerCreate(osTimer(beepTimer), osTimerOnce, NULL);

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* definition and creation of beepEventQueue */
  osMessageQDef(beepEventQueue, 1, uint16_t);
  beepEventQueueHandle = osMessageCreate(osMessageQ(beepEventQueue), NULL);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of defaultTask */
  osThreadDef(defaultTask, StartDefaultTask, osPriorityAboveNormal, 0, 640);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  /* definition and creation of keyboardTask */
  osThreadDef(keyboardTask, KeyboardTask, osPriorityNormal, 0, 128);
  keyboardTaskHandle = osThreadCreate(osThread(keyboardTask), NULL);

  /* definition and creation of beepTask */
  osThreadDef(beepTask, BeepTask, osPriorityLow, 0, 128);
  beepTaskHandle = osThreadCreate(osThread(beepTask), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  NVM_Setup();
  HAL_FLASH_Unlock();
  __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_OPTVERR);
  while (__HAL_FLASH_GET_FLAG(FLASH_FLAG_OPTVERR) != RESET)
    ;

  if (nvm.NVM_Init() != NVM_STATUS_OK) {
	  nvm.NVM_Format();
  }

  // nvm.NVM_WriteVariable32bits(NVM_U32_TOTAL_DISTANCE_METERS, 36141000);
//    nvm.NVM_WriteVariable32bits(NVM_U32_TOTAL_TIME_SECONDS, 0);
  //  nvm.NVM_WriteVariable32bits(NVM_U32_LUB_DISTANCE_PARAM, 1500);
  //  nvm.NVM_WriteVariable32bits(NVM_U32_LUB_DISTANCE, 0);

  //  nvm.NVM_WriteVariable32bits(NVM_U32_TOTAL_DISTANCE_METERS, 10000);
  //  nvm.NVM_WriteVariable32bits(NVM_U32_TOTAL_TIME_SECONDS, 3600);

  treadmill_setup(&treadmill);
  treadmill.get_time_ms = HAL_GetTick;
  treadmill.hw.beep = beep;

  treadmill.on_update_fms_callback = treadmill_update_fms;

  treadmill.hw.ble.get_tick = HAL_GetTick;
  treadmill.hw.ble.send_data = ble_uart_write;
  treadmill.hw.ble.recv_data = ble_uart_read;
  treadmill.hw.ble.reset = ble_hw_reset;

  treadmill.hw.inverter.send_data = motor_uart_write;
  treadmill.hw.inverter.recv_data = motor_uart_read;
  treadmill.hw.wait_ms = wait_ms;

  treadmill.hw.inverter.change_frequency = motor_change_frequency;

  display_init();

  HAL_ADCEx_Calibration_Start(&hadc1);

  HAL_TIM_IC_Start_IT(&htim17, TIM_CHANNEL_1);

  HAL_TIM_IC_Start_IT(&htim14, TIM_CHANNEL_1);

  HAL_UART_Receive_DMA(&huart3, (uint8_t *)&_huart3_buffer[0], 2);

  /* USER CODE END RTOS_THREADS */

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1) {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
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
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
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

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
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
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc1.Init.LowPowerAutoWait = DISABLE;
  hadc1.Init.LowPowerAutoPowerOff = DISABLE;
  hadc1.Init.ContinuousConvMode = ENABLE;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.DMAContinuousRequests = ENABLE;
  hadc1.Init.Overrun = ADC_OVR_DATA_PRESERVED;
  hadc1.Init.SamplingTimeCommon1 = ADC_SAMPLETIME_79CYCLES_5;
  hadc1.Init.SamplingTimeCommon2 = ADC_SAMPLETIME_79CYCLES_5;
  hadc1.Init.OversamplingMode = DISABLE;
  hadc1.Init.TriggerFrequencyMode = ADC_TRIGGER_FREQ_HIGH;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_5;
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
  * @brief CRC Initialization Function
  * @param None
  * @retval None
  */
static void MX_CRC_Init(void)
{

  /* USER CODE BEGIN CRC_Init 0 */

  /* USER CODE END CRC_Init 0 */

  /* USER CODE BEGIN CRC_Init 1 */

  /* USER CODE END CRC_Init 1 */
  hcrc.Instance = CRC;
  hcrc.Init.DefaultPolynomialUse = DEFAULT_POLYNOMIAL_ENABLE;
  hcrc.Init.DefaultInitValueUse = DEFAULT_INIT_VALUE_ENABLE;
  hcrc.Init.InputDataInversionMode = CRC_INPUTDATA_INVERSION_NONE;
  hcrc.Init.OutputDataInversionMode = CRC_OUTPUTDATA_INVERSION_DISABLE;
  hcrc.InputDataFormat = CRC_INPUTDATA_FORMAT_BYTES;
  if (HAL_CRC_Init(&hcrc) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CRC_Init 2 */

  /* USER CODE END CRC_Init 2 */

}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

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
  * @brief I2C2 Initialization Function
  * @param None
  * @retval None
  */
void MX_I2C2_Init(void)
{

  /* USER CODE BEGIN I2C2_Init 0 */

  /* USER CODE END I2C2_Init 0 */

  /* USER CODE BEGIN I2C2_Init 1 */

  /* USER CODE END I2C2_Init 1 */
  hi2c2.Instance = I2C2;
  hi2c2.Init.Timing = 0x00303D5B;
  hi2c2.Init.OwnAddress1 = 0;
  hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c2.Init.OwnAddress2 = 0;
  hi2c2.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c2) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c2, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c2, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C2_Init 2 */

  /* USER CODE END I2C2_Init 2 */

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
  htim3.Init.Prescaler = 27;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 65535;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_OC_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_TOGGLE;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_LOW;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_OC_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_4) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */
  // HAL_TIM_OC_Start_IT(&htim3, TIM_CHANNEL_4);
  /* USER CODE END TIM3_Init 2 */
  HAL_TIM_MspPostInit(&htim3);

}

/**
  * @brief TIM6 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM6_Init(void)
{

  /* USER CODE BEGIN TIM6_Init 0 */

  /* USER CODE END TIM6_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM6_Init 1 */

  /* USER CODE END TIM6_Init 1 */
  htim6.Instance = TIM6;
  htim6.Init.Prescaler = 15;
  htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim6.Init.Period = 999;
  htim6.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim6) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim6, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM6_Init 2 */

  /* USER CODE END TIM6_Init 2 */

}

/**
  * @brief TIM14 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM14_Init(void)
{

  /* USER CODE BEGIN TIM14_Init 0 */

  /* USER CODE END TIM14_Init 0 */

  TIM_IC_InitTypeDef sConfigIC = {0};

  /* USER CODE BEGIN TIM14_Init 1 */

  /* USER CODE END TIM14_Init 1 */
  htim14.Instance = TIM14;
  htim14.Init.Prescaler = 15999;
  htim14.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim14.Init.Period = 65535;
  htim14.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim14.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim14) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_IC_Init(&htim14) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
  sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
  sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
  sConfigIC.ICFilter = 0;
  if (HAL_TIM_IC_ConfigChannel(&htim14, &sConfigIC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM14_Init 2 */

  /* USER CODE END TIM14_Init 2 */

}

/**
  * @brief TIM16 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM16_Init(void)
{

  /* USER CODE BEGIN TIM16_Init 0 */

  /* USER CODE END TIM16_Init 0 */

  TIM_OC_InitTypeDef sConfigOC = {0};
  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

  /* USER CODE BEGIN TIM16_Init 1 */

  /* USER CODE END TIM16_Init 1 */
  htim16.Instance = TIM16;
  htim16.Init.Prescaler = 200;
  htim16.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim16.Init.Period = 40;
  htim16.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim16.Init.RepetitionCounter = 0;
  htim16.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim16) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim16) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 10;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if (HAL_TIM_PWM_ConfigChannel(&htim16, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
  sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
  sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
  sBreakDeadTimeConfig.DeadTime = 0;
  sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
  sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
  sBreakDeadTimeConfig.BreakFilter = 0;
  sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
  if (HAL_TIMEx_ConfigBreakDeadTime(&htim16, &sBreakDeadTimeConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM16_Init 2 */

  /* USER CODE END TIM16_Init 2 */
  HAL_TIM_MspPostInit(&htim16);

}

/**
  * @brief TIM17 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM17_Init(void)
{

  /* USER CODE BEGIN TIM17_Init 0 */

  /* USER CODE END TIM17_Init 0 */

  /* USER CODE BEGIN TIM17_Init 1 */

  /* USER CODE END TIM17_Init 1 */
  htim17.Instance = TIM17;
  htim17.Init.Prescaler = 15999;
  htim17.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim17.Init.Period = 65535;
  htim17.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim17.Init.RepetitionCounter = 0;
  htim17.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim17) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM17_Init 2 */

  /* USER CODE END TIM17_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 9600;
  huart1.Init.WordLength = UART_WORDLENGTH_9B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_EVEN;
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
  if (HAL_UARTEx_EnableFifoMode(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief USART3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART3_UART_Init(void)
{

  /* USER CODE BEGIN USART3_Init 0 */

  /* USER CODE END USART3_Init 0 */

  /* USER CODE BEGIN USART3_Init 1 */

  /* USER CODE END USART3_Init 1 */
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 115200;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  huart3.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart3.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart3.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */

  /* USER CODE END USART3_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 3, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);
  /* DMA1_Channel2_3_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel2_3_IRQn, 3, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel2_3_IRQn);

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
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, LED_Pin|TS10_Pin|TS11_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(RESET_N_GPIO_Port, RESET_N_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, CMD_Pin|W_HW_Pin|TS08_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(TS09_GPIO_Port, TS09_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, TS06_Pin|TS07_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(CONTR_GPIO_Port, CONTR_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : LED_Pin TS10_Pin TS11_Pin */
  GPIO_InitStruct.Pin = LED_Pin|TS10_Pin|TS11_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : INC_MINUS_Pin SP_MINUS_Pin SP_PLUS_Pin INC_PLUS_Pin
                           TS05_Pin TS00_Pin */
  GPIO_InitStruct.Pin = INC_MINUS_Pin|SP_MINUS_Pin|SP_PLUS_Pin|INC_PLUS_Pin
                          |TS05_Pin|TS00_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  
  /*Configure GPIO pins : RESET_N_Pin TS09_Pin */
  GPIO_InitStruct.Pin = RESET_N_Pin|TS09_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : CMD_Pin W_HW_Pin TS08_Pin CONTR_Pin */
  GPIO_InitStruct.Pin = CMD_Pin|W_HW_Pin|TS08_Pin|CONTR_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : LO_PLUS_Pin LO_MINUS_Pin TS03_Pin TS04_Pin
                           EMG_Pin */
  GPIO_InitStruct.Pin = LO_PLUS_Pin|LO_MINUS_Pin|TS03_Pin|TS04_Pin
                          |EMG_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : INT_SC_Pin */
  GPIO_InitStruct.Pin = INT_SC_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(INT_SC_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : TS01_Pin TS02_Pin */
  GPIO_InitStruct.Pin = TS01_Pin|TS02_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pins : TS06_Pin TS07_Pin */
  GPIO_InitStruct.Pin = TS06_Pin|TS07_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI4_15_IRQn, 3, 0);
  HAL_NVIC_EnableIRQ(EXTI4_15_IRQn);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartDefaultTask */
/**
 * @brief  Function implementing the defaultTask thread.
 * @param  argument: Not used
 * @retval None
 */
GPIO_PinState _lo_minus = RESET;
GPIO_PinState _lo_plus = RESET;
GPIO_PinState _emg = RESET;

static inline void bpm_read_handler(void) {
  static uint32_t hg_hold_tick = 0;
  {  // HANDGRIP
    _lo_minus = HAL_GPIO_ReadPin(LO_MINUS_GPIO_Port, LO_MINUS_Pin);
    _lo_plus = HAL_GPIO_ReadPin(LO_PLUS_GPIO_Port, LO_PLUS_Pin);
    if (HAL_GPIO_ReadPin(LO_MINUS_GPIO_Port, LO_MINUS_Pin) == GPIO_PIN_RESET &&
        HAL_GPIO_ReadPin(LO_PLUS_GPIO_Port, LO_PLUS_Pin) == GPIO_PIN_RESET) {
      if (hg_hold_tick == 0) {
        hg_hold_tick = treadmill.get_time_ms();
      } else if (HAL_GPIO_ReadPin(CONTR_GPIO_Port, CONTR_Pin) == GPIO_PIN_RESET) {
        if (treadmill.get_time_ms() - hg_hold_tick >= 2000) {
          HAL_GPIO_WritePin(CONTR_GPIO_Port, CONTR_Pin, GPIO_PIN_SET);
          HAL_ADC_Start_DMA(&hadc1, adc_values, ADC_BUFFER_SIZE);
          HAL_TIM_Base_Start_IT(&htim6);
          if (PolarBPM == 0) treadmill_set_bpm_read(&treadmill);
          adc_delay_tick_ms = HAL_GetTick();
        }
      } else {
        hg_hold_tick = treadmill.get_time_ms();
      }
    } else {
      if (hg_hold_tick) {
        if (treadmill.get_time_ms() - hg_hold_tick >= 5000) {
          HAL_GPIO_WritePin(CONTR_GPIO_Port, CONTR_Pin, GPIO_PIN_RESET);
          HAL_ADC_Stop_DMA(&hadc1);
          HAL_TIM_Base_Stop_IT(&htim6);
          adc_hrm = 0;
          hg_hold_tick = 0;
          bpm_reset_variables();
          if (PolarBPM == 0) {
            treadmill_set_bpm_stop(&treadmill, 1);
          }
        }
      }
    }
  }
  {  // POLAR
    if (ICCapturedPolarLastTick) {
      uint32_t t = HAL_GetTick() - ICCapturedPolarLastTick;
      if (PolarBPM < 40 && PolarBPM > 240) {
        treadmill_set_bpm_read(&treadmill);
      } else if (t >= 10000) {
        PolarICDiff = 0;
        PolarBPM = 0;
        ICCapturedPolarLastTick = 0;
        treadmill_set_bpm_stop(&treadmill, 1);
      } else {
        treadmill_set_bpm_value(&treadmill, (uint8_t)PolarBPM);
      }
    }
  }
  {
    // RODA //////////////
    if (treadmill.scene == SCENE_TRAINING) {
      if (ICCapturedWheelLastTick == 0)
        ICCapturedWheelLastTick = HAL_GetTick();
      else if (HAL_GetTick() - ICCapturedWheelLastTick >= 5000) {
       treadmill_stop_procedure(&treadmill);
       treadmill_set_dm_message(&treadmill, get_string(&treadmill, STR_WHEEL), 3000, 0);
       ICCapturedWheelLastTick = 0;
      }
    } else {
    	ICCapturedWheelLastTick = 0;
    }
  }
}

/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void const * argument)
{
  /* USER CODE BEGIN 5 */
  /* Infinite loop */
  for (;;) {
    stats.mainloop.start = HAL_GetTick();

//    enable_shortcut_keys_reading = 1;

    bpm_read_handler();

    treadmill_ble_process(&treadmill);

    treadmill_motor_process(&treadmill);

    while (treadmill.ms < HAL_GetTick()) {
      treadmill.ms += 10;
      treadmill_update(&treadmill);
      display_update();
    }

    rt_led_render(&treadmill);

    stats.mainloop.end = HAL_GetTick();
    stats.mainloop.delta = (stats.mainloop.end - stats.mainloop.start);
    stats.mainloop.average = (stats.mainloop.average + stats.mainloop.delta) / 2;
    if (stats.mainloop.peak < stats.mainloop.delta) {
      stats.mainloop.peak = stats.mainloop.delta;
    }

    osDelay(10);
  }
  /* USER CODE END 5 */
}

/* USER CODE BEGIN Header_KeyboardTask */
/**
 * @brief Function implementing the keyboardTask thread.
 * @param argument: Not used
 * @retval None
 */

#define MAIN_BURST_WINDOW_MS 1000u
#define MAIN_BURST_MAX 8u
#define HG_BURST_WINDOW_MS 1000u
#define HG_BURST_MAX 8u
#define HG_START_STOP_CONFIG_HOLD_MS 2000u

typedef enum keyboard_source_group {
  KEYBOARD_SOURCE_GROUP_MAIN = 0,
  KEYBOARD_SOURCE_GROUP_HG_SPEED,
  KEYBOARD_SOURCE_GROUP_HG_START_STOP,
} keyboard_source_group_t;

struct key_source_health {
  uint8_t pressed;
  kbd_key_t last_key;
  uint32_t press_start_ms;
  uint32_t burst_window_start_ms;
  uint8_t burst_count;
};

struct c2v_pending_shortcut {
  uint8_t active;
  kbd_key_t key;
  uint32_t selected_time_ms;
};

static struct key_source_health g_key_source_health[KEYBOARD_INPUT_SOURCE_TOTAL] = {0};
static struct c2v_pending_shortcut g_c2v_pending_shortcut = {0};
static uint8_t g_c2v_start_prompt_active = 0;
static volatile uint8_t g_main_plus_minus_combo_pressed = 0;
static volatile uint8_t g_hg_plus_minus_combo_pressed = 0;

static void keyboard_reset_source_health(keyboard_input_source_t source) {
  if (source >= KEYBOARD_INPUT_SOURCE_TOTAL) return;

  g_key_source_health[source].pressed = 0;
  g_key_source_health[source].last_key = KEY_NONE;
  g_key_source_health[source].press_start_ms = 0;
  g_key_source_health[source].burst_window_start_ms = 0;
  g_key_source_health[source].burst_count = 0;
}

static uint8_t keyboard_is_error_clear_combo_active(void) {
  if (treadmill.scene != SCENE_IDLE) return 0;

  return (uint8_t)(g_main_plus_minus_combo_pressed || g_hg_plus_minus_combo_pressed ||
                   ext_i2c_kbd_is_clear_combo_pressed());
}

static uint8_t keyboard_is_error_clear_combo_key(keyboard_input_source_t source, kbd_key_t key) {
  if (source == KEYBOARD_INPUT_SOURCE_MAIN_MATRIX || source == KEYBOARD_INPUT_SOURCE_MAIN_I2C) {
    return (key == KEY_SPEED_INCREMENT || key == KEY_SPEED_DECREMENT);
  }

  if (source == KEYBOARD_INPUT_SOURCE_HG_SPEED) {
    return (key == KEY_EXT_KBD_SPEED_INC || key == KEY_EXT_KBD_SPEED_DEC);
  }

  return 0;
}

static uint8_t keyboard_should_suppress_fault_for_clear_combo(keyboard_input_source_t source, kbd_key_t key) {
  return (keyboard_is_error_clear_combo_active() && keyboard_is_error_clear_combo_key(source, key));
}

static keyboard_source_group_t keyboard_source_group_from_input(keyboard_input_source_t source) {
  switch (source) {
    case KEYBOARD_INPUT_SOURCE_MAIN_MATRIX:
    case KEYBOARD_INPUT_SOURCE_MAIN_I2C:
      return KEYBOARD_SOURCE_GROUP_MAIN;
    case KEYBOARD_INPUT_SOURCE_HG_SPEED:
      return KEYBOARD_SOURCE_GROUP_HG_SPEED;
    case KEYBOARD_INPUT_SOURCE_HG_START_STOP:
      return KEYBOARD_SOURCE_GROUP_HG_START_STOP;
    default:
      return KEYBOARD_SOURCE_GROUP_MAIN;
  }
}

static uint8_t keyboard_source_disable_mask_from_group(keyboard_source_group_t group) {
  switch (group) {
    case KEYBOARD_SOURCE_GROUP_MAIN:
      return KEYBOARD_SOURCE_MASK_MAIN;
    case KEYBOARD_SOURCE_GROUP_HG_SPEED:
      return KEYBOARD_SOURCE_MASK_HG_SPEED;
    case KEYBOARD_SOURCE_GROUP_HG_START_STOP:
      return KEYBOARD_SOURCE_MASK_HG_START_STOP;
    default:
      return 0;
  }
}

static uint32_t keyboard_stuck_threshold_ms(keyboard_source_group_t group) {
  (void)group;
  return KEYBOARD_STUCK_THRESHOLD_MS;
}

static uint32_t keyboard_burst_window_ms(keyboard_source_group_t group) {
  if (group == KEYBOARD_SOURCE_GROUP_MAIN) {
    return MAIN_BURST_WINDOW_MS;
  }
  return HG_BURST_WINDOW_MS;
}

static uint8_t keyboard_burst_max(keyboard_source_group_t group) {
  if (group == KEYBOARD_SOURCE_GROUP_MAIN) {
    return MAIN_BURST_MAX;
  }
  return HG_BURST_MAX;
}

static keyboard_error_code_t keyboard_stuck_fault_code(keyboard_source_group_t group) {
  switch (group) {
    case KEYBOARD_SOURCE_GROUP_MAIN:
      return KEYBOARD_ERROR_MAIN_STUCK;
    case KEYBOARD_SOURCE_GROUP_HG_SPEED:
      return KEYBOARD_ERROR_HG_SPEED_STUCK;
    case KEYBOARD_SOURCE_GROUP_HG_START_STOP:
      return KEYBOARD_ERROR_HG_START_STOP_STUCK;
    default:
      return KEYBOARD_ERROR_NONE;
  }
}

static keyboard_error_code_t keyboard_burst_fault_code(keyboard_source_group_t group) {
  switch (group) {
    case KEYBOARD_SOURCE_GROUP_MAIN:
      return KEYBOARD_ERROR_MAIN_BURST;
    case KEYBOARD_SOURCE_GROUP_HG_SPEED:
      return KEYBOARD_ERROR_HG_SPEED_BURST;
    case KEYBOARD_SOURCE_GROUP_HG_START_STOP:
      return KEYBOARD_ERROR_HG_START_STOP_BURST;
    default:
      return KEYBOARD_ERROR_NONE;
  }
}

static uint8_t keyboard_is_main_matrix_speed_shortcut_release_action(kbd_key_t key) {
  return (key == KEY_SPEED_6 || key == KEY_SPEED_9 || key == KEY_SPEED_12 || key == KEY_SPEED_15);
}

static uint8_t keyboard_is_main_i2c_speed_shortcut_release_action(kbd_key_t key) {
  return (key == KEY_EXT_KBD_SPEED_6 || key == KEY_EXT_KBD_SPEED_9 || key == KEY_EXT_KBD_SPEED_12 ||
          key == KEY_EXT_KBD_SPEED_15);
}

static uint8_t keyboard_is_speed_shortcut_release_action(keyboard_input_source_t source, kbd_key_t key) {
  if (source == KEYBOARD_INPUT_SOURCE_MAIN_MATRIX) {
    return keyboard_is_main_matrix_speed_shortcut_release_action(key);
  }
  if (source == KEYBOARD_INPUT_SOURCE_MAIN_I2C) {
    return keyboard_is_main_i2c_speed_shortcut_release_action(key);
  }
  return 0;
}

static uint16_t keyboard_speed_shortcut_target_speed(kbd_key_t key) {
  switch (key) {
    case KEY_SPEED_6:
    case KEY_EXT_KBD_SPEED_6:
      return 600u;
    case KEY_SPEED_9:
    case KEY_EXT_KBD_SPEED_9:
      return 900u;
    case KEY_SPEED_12:
    case KEY_EXT_KBD_SPEED_12:
      return 1200u;
    case KEY_SPEED_15:
    case KEY_EXT_KBD_SPEED_15:
      return 1500u;
    default:
      return 0u;
  }
}

static uint8_t keyboard_should_dispatch_shortcut_on_press(kbd_key_t key) {
  if (treadmill.config.c2v != RT_LED_CONFIG_C2V_DISABLE) return 0;
  if (treadmill.scene != SCENE_TRAINING) return 0;

  uint16_t shortcut_target_speed = keyboard_speed_shortcut_target_speed(key);
  if (shortcut_target_speed == 0u) return 0;

  return (shortcut_target_speed < treadmill.training.speed);
}

static uint8_t keyboard_is_speed_shortcut_release_behavior(keyboard_input_source_t source, kbd_key_t key) {
  if (!keyboard_is_speed_shortcut_release_action(source, key)) {
    return 0;
  }

  if (keyboard_should_dispatch_shortcut_on_press(key)) {
    return 0;
  }

  return 1;
}

static uint8_t keyboard_is_start_release_action(keyboard_input_source_t source, kbd_key_t key) {
  if ((source == KEYBOARD_INPUT_SOURCE_MAIN_MATRIX || source == KEYBOARD_INPUT_SOURCE_MAIN_I2C) &&
      key == KEY_START) {
    return 1;
  }

  if (source == KEYBOARD_INPUT_SOURCE_HG_START_STOP && key == KEY_EXT_KBD_START) {
    return 1;
  }

  return 0;
}

static uint8_t keyboard_is_release_action(keyboard_input_source_t source, kbd_key_t key) {
  return (keyboard_is_speed_shortcut_release_behavior(source, key) || keyboard_is_start_release_action(source, key));
}

static uint8_t keyboard_is_speed_shortcut_key(keyboard_input_source_t source, kbd_key_t key) {
  if (keyboard_is_speed_shortcut_release_action(source, key)) {
    return 1;
  }

  if (source == KEYBOARD_INPUT_SOURCE_HG_SPEED &&
      (key == KEY_EXT_KBD_SPEED_INC || key == KEY_EXT_KBD_SPEED_DEC)) {
    return 1;
  }

  return 0;
}

static uint8_t keyboard_should_check_stuck(keyboard_input_source_t source, kbd_key_t key) {
  if (treadmill.scene == SCENE_IDLE) {
    return keyboard_is_speed_shortcut_key(source, key);
  }

  if (treadmill.scene == SCENE_TRAINING && treadmill.config.c2v == RT_LED_CONFIG_C2V_DISABLE) {
    return (key != KEY_NONE);
  }

  return 0;
}

static uint8_t keyboard_is_stop_key(kbd_key_t key) {
  return (key == KEY_STOP || key == KEY_EXT_KBD_STOP);
}

static uint8_t keyboard_is_config_key(kbd_key_t key) {
  return (key == KEY_CONFIG);
}

static uint8_t keyboard_is_config_navigation_start_key(kbd_key_t key) {
  return (key == KEY_START || key == KEY_EXT_KBD_START);
}

static uint8_t keyboard_is_source_key_stuck_now(keyboard_input_source_t source, kbd_key_t key) {
  if (source >= KEYBOARD_INPUT_SOURCE_TOTAL) return 0;

  struct key_source_health *source_health = &g_key_source_health[source];
  if (!source_health->pressed || source_health->last_key != key || source_health->press_start_ms == 0) {
    return 0;
  }

  keyboard_source_group_t group = keyboard_source_group_from_input(source);
  uint32_t held_ms = HAL_GetTick() - source_health->press_start_ms;
  return (held_ms >= keyboard_stuck_threshold_ms(group));
}

static uint8_t keyboard_is_config_errors_browse_key(kbd_key_t key) {
  return (key == KEY_SPEED_INCREMENT);
}

static uint8_t keyboard_c2v_enabled(void) {
  return (treadmill.config.c2v == RT_LED_CONFIG_C2V_ENABLE);
}

static uint8_t keyboard_is_c2v_shortcut_key(kbd_key_t key) {
  return (key == KEY_SPEED_6 || key == KEY_SPEED_9 || key == KEY_SPEED_12 || key == KEY_SPEED_15 ||
          key == KEY_EXT_KBD_SPEED_6 || key == KEY_EXT_KBD_SPEED_9 || key == KEY_EXT_KBD_SPEED_12 ||
          key == KEY_EXT_KBD_SPEED_15);
}

static void keyboard_show_c2v_start_prompt(void) {
  treadmill_set_dm_message(&treadmill, "START", C2V_CONFIRM_WINDOW_MS, 0);
  g_c2v_start_prompt_active = 1;
}

static void keyboard_clear_c2v_start_prompt(void) {
  if (!g_c2v_start_prompt_active) return;

  if (treadmill.message.tick && strcmp(treadmill.message.data, "START") == 0) {
    treadmill_clear_dm_message(&treadmill);
  }

  g_c2v_start_prompt_active = 0;
}

static void keyboard_clear_c2v_pending_shortcut(void) {
  keyboard_clear_c2v_start_prompt();
  treadmill.aux.c2v_shortcut_sd_feedback_active = 0;
  treadmill.aux.c2v_shortcut_sd_feedback_start_ms = 0;
  treadmill.aux.c2v_shortcut_sd_feedback_speed = 0;
  g_c2v_pending_shortcut.active = 0;
  g_c2v_pending_shortcut.key = KEY_NONE;
  g_c2v_pending_shortcut.selected_time_ms = 0;
}

static uint8_t keyboard_should_use_c2v_for_shortcut(kbd_key_t key) {
  if (!keyboard_c2v_enabled() || treadmill.scene != SCENE_TRAINING || !keyboard_is_c2v_shortcut_key(key)) {
    return 0;
  }

  uint16_t shortcut_target_speed = keyboard_speed_shortcut_target_speed(key);
  if (shortcut_target_speed == 0u) return 0;

  // C2V=1: START is used only to confirm shortcut speed increments.
  return (shortcut_target_speed > treadmill.training.speed);
}

static void keyboard_c2v_update(uint32_t now) {
  if (!g_c2v_pending_shortcut.active) return;

  if (!keyboard_c2v_enabled() || treadmill.scene != SCENE_TRAINING ||
      (now - g_c2v_pending_shortcut.selected_time_ms) > C2V_CONFIRM_WINDOW_MS) {
    keyboard_clear_c2v_pending_shortcut();
  }
}

static void keyboard_store_c2v_pending_shortcut(kbd_key_t key, uint32_t now) {
  uint16_t shortcut_target_speed = keyboard_speed_shortcut_target_speed(key);
  g_c2v_pending_shortcut.active = 1;
  g_c2v_pending_shortcut.key = key;
  g_c2v_pending_shortcut.selected_time_ms = now;
  treadmill.aux.c2v_shortcut_sd_feedback_active = 1;
  treadmill.aux.c2v_shortcut_sd_feedback_start_ms = now;
  treadmill.aux.c2v_shortcut_sd_feedback_speed = shortcut_target_speed;
  keyboard_show_c2v_start_prompt();
}

static uint8_t keyboard_try_confirm_c2v_shortcut(uint32_t now) {
  if (!g_c2v_pending_shortcut.active) return 0;
  if ((now - g_c2v_pending_shortcut.selected_time_ms) > C2V_CONFIRM_WINDOW_MS) {
    keyboard_clear_c2v_pending_shortcut();
    return 0;
  }

  kbd_key_t confirmed_key = g_c2v_pending_shortcut.key;
  keyboard_clear_c2v_pending_shortcut();
  treadmill_key_press(&treadmill, confirmed_key);
  return 1;
}

static void keyboard_service_idle_keyboard_fault_clear_combo(void) {
  static uint32_t combo_pressed_tick = 0;
  static uint8_t combo_action_done = 0;

  if (treadmill.scene != SCENE_IDLE) {
    combo_pressed_tick = 0;
    combo_action_done = 0;
    return;
  }

  uint8_t combo_pressed_now = keyboard_is_error_clear_combo_active();
  if (!combo_pressed_now) {
    combo_pressed_tick = 0;
    combo_action_done = 0;
    return;
  }

  uint32_t now = HAL_GetTick();
  if (combo_pressed_tick == 0) {
    combo_pressed_tick = now;
  }

  if (!combo_action_done && (now - combo_pressed_tick) >= KEYBOARD_FAULT_CLEAR_HOLD_MS) {
    treadmill_clear_keyboard_error_code_and_unlock(&treadmill);
    keyboard_reset_source_health(KEYBOARD_INPUT_SOURCE_MAIN_MATRIX);
    keyboard_reset_source_health(KEYBOARD_INPUT_SOURCE_MAIN_I2C);
    keyboard_reset_source_health(KEYBOARD_INPUT_SOURCE_HG_SPEED);
    treadmill.hw.beep(BEEP_KEYPRESS_ACK);
    combo_action_done = 1;
  }
}

void keyboard_route_key_event(keyboard_input_source_t source, kbd_key_t key) {
  if (source >= KEYBOARD_INPUT_SOURCE_TOTAL || key == KEY_NONE) return;
  keyboard_c2v_update(HAL_GetTick());

  if (keyboard_should_suppress_fault_for_clear_combo(source, key)) {
    keyboard_reset_source_health(source);
    return;
  }

  keyboard_source_group_t group = keyboard_source_group_from_input(source);
  uint8_t disable_mask = keyboard_source_disable_mask_from_group(group);
  if (disable_mask == 0) return;

  if (treadmill.hw.keyboard.disabled_sources_mask & disable_mask) {
    // Safety rule: STOP and CONFIG must never be disabled by keyboard fault latches.
    // In CONFIG scene, START is available to navigate to ERR reset only when that START is not stuck.
    // In CONFIG/ERR, allow principal '+' to browse stored errors during maintenance.
    if (keyboard_is_stop_key(key) || keyboard_is_config_key(key) ||
        (treadmill.scene == SCENE_CONFIG && keyboard_is_config_navigation_start_key(key) &&
         !keyboard_is_source_key_stuck_now(source, key)) ||
        (treadmill.scene == SCENE_CONFIG && treadmill.config.option == CONFIG_OPTION_ERRORS &&
         keyboard_is_config_errors_browse_key(key))) {
      if (keyboard_is_release_action(source, key)) {
        struct key_source_health *source_health = &g_key_source_health[source];
        if (!source_health->pressed || source_health->last_key != key) {
          source_health->pressed = 1;
          source_health->last_key = key;
          source_health->press_start_ms = HAL_GetTick();
        }
      } else {
        treadmill_key_press(&treadmill, key);
      }
    }
    return;
  }

  uint32_t now = HAL_GetTick();
  struct key_source_health *source_health = &g_key_source_health[source];
  uint8_t should_check_stuck = keyboard_should_check_stuck(source, key);
  uint8_t is_new_press = (!source_health->pressed || source_health->last_key != key);

  if (is_new_press) {
    source_health->pressed = 1;
    source_health->last_key = key;
    source_health->press_start_ms = now;

    if (source_health->burst_window_start_ms == 0 ||
        (now - source_health->burst_window_start_ms) > keyboard_burst_window_ms(group)) {
      source_health->burst_window_start_ms = now;
      source_health->burst_count = 1;
    } else if (source_health->burst_count < 255u) {
      source_health->burst_count++;
    }

    if (source_health->burst_count > keyboard_burst_max(group)) {
      treadmill.hw.keyboard.disabled_sources_mask |= disable_mask;
      treadmill_register_keyboard_error(&treadmill, keyboard_burst_fault_code(group));
      return;
    }
  } else if (should_check_stuck && source_health->press_start_ms > 0 &&
             (now - source_health->press_start_ms) >= keyboard_stuck_threshold_ms(group)) {
    treadmill.hw.keyboard.disabled_sources_mask |= disable_mask;
    treadmill_register_keyboard_error(&treadmill, keyboard_stuck_fault_code(group));
    return;
  }

  // Safety behavior: selected keys (speed shortcuts/start) are applied only on key release.
  if (keyboard_is_release_action(source, key)) {
    return;
  }

  if (keyboard_should_dispatch_shortcut_on_press(key) && !is_new_press) {
    return;
  }

  treadmill_key_press(&treadmill, key);
}

static void keyboard_route_key_hold(keyboard_input_source_t source, kbd_key_t key) {
  if (source >= KEYBOARD_INPUT_SOURCE_TOTAL || key == KEY_NONE) return;
  if (keyboard_should_suppress_fault_for_clear_combo(source, key)) {
    keyboard_reset_source_health(source);
    return;
  }
  if (!keyboard_should_check_stuck(source, key)) return;

  keyboard_source_group_t group = keyboard_source_group_from_input(source);
  uint8_t disable_mask = keyboard_source_disable_mask_from_group(group);
  if (disable_mask == 0) return;
  if (treadmill.hw.keyboard.disabled_sources_mask & disable_mask) return;

  struct key_source_health *source_health = &g_key_source_health[source];
  if (!source_health->pressed || source_health->last_key != key || source_health->press_start_ms == 0) return;

  uint32_t now = HAL_GetTick();
  if ((now - source_health->press_start_ms) >= keyboard_stuck_threshold_ms(group)) {
    treadmill.hw.keyboard.disabled_sources_mask |= disable_mask;
    treadmill_register_keyboard_error(&treadmill, keyboard_stuck_fault_code(group));
  }
}

void keyboard_route_key_release(keyboard_input_source_t source) {
  if (source >= KEYBOARD_INPUT_SOURCE_TOTAL) return;
  uint32_t now = HAL_GetTick();
  keyboard_c2v_update(now);
  struct key_source_health *source_health = &g_key_source_health[source];
  kbd_key_t last_key = source_health->last_key;
  keyboard_source_group_t group = keyboard_source_group_from_input(source);
  uint32_t held_ms = 0;
  if (source_health->pressed && source_health->press_start_ms > 0) {
    held_ms = now - source_health->press_start_ms;
  }

  if (source_health->pressed && keyboard_should_check_stuck(source, last_key) &&
      held_ms >= keyboard_stuck_threshold_ms(group)) {
    uint8_t disable_mask = keyboard_source_disable_mask_from_group(group);
    if (disable_mask != 0 && (treadmill.hw.keyboard.disabled_sources_mask & disable_mask) == 0) {
      treadmill.hw.keyboard.disabled_sources_mask |= disable_mask;
      treadmill_register_keyboard_error(&treadmill, keyboard_stuck_fault_code(group));
    }
    source_health->pressed = 0;
    source_health->last_key = KEY_NONE;
    source_health->press_start_ms = 0;
    return;
  }

  if (source_health->pressed && keyboard_is_speed_shortcut_release_behavior(source, last_key)) {
    // During training, long-pressed speed shortcuts are ignored instead of acting/sticking.
    if (treadmill.scene == SCENE_TRAINING && treadmill.config.c2v == RT_LED_CONFIG_C2V_DISABLE &&
        held_ms >= KEYBOARD_SHORTCUT_LONG_PRESS_IGNORE_MS) {
      source_health->pressed = 0;
      source_health->last_key = KEY_NONE;
      source_health->press_start_ms = 0;
      return;
    }
  }

  if (source_health->pressed && keyboard_is_release_action(source, last_key)) {
    uint8_t disable_mask = keyboard_source_disable_mask_from_group(keyboard_source_group_from_input(source));
    uint8_t can_dispatch = ((treadmill.hw.keyboard.disabled_sources_mask & disable_mask) == 0);

    if (!can_dispatch && treadmill.scene == SCENE_CONFIG && keyboard_is_config_navigation_start_key(last_key) &&
        !keyboard_is_source_key_stuck_now(source, last_key)) {
      can_dispatch = 1;
    }

    if (can_dispatch) {
      if (keyboard_should_use_c2v_for_shortcut(last_key)) {
        keyboard_store_c2v_pending_shortcut(last_key, now);
      } else if (keyboard_is_start_release_action(source, last_key) && keyboard_c2v_enabled() &&
                 treadmill.scene == SCENE_TRAINING && keyboard_try_confirm_c2v_shortcut(now)) {
        // START acts as confirmation for pending speed shortcut in C2V mode.
      } else {
        if (keyboard_c2v_enabled() && treadmill.scene == SCENE_TRAINING && keyboard_is_c2v_shortcut_key(last_key) &&
            g_c2v_pending_shortcut.active) {
          // Selecting a direct shortcut while another one is pending cancels the old confirmation window.
          keyboard_clear_c2v_pending_shortcut();
        }
        treadmill_key_press(&treadmill, last_key);
      }
    }
  }

  source_health->pressed = 0;
  source_health->last_key = KEY_NONE;
  source_health->press_start_ms = 0;
}

#define SP_SPD_INC_PIN SP_PLUS_Pin
#define SP_SPD_DEC_PIN SP_MINUS_Pin
#define SP_START_PIN INC_MINUS_Pin
#define SP_STOP_PIN INC_PLUS_Pin

#define SP_SPD_INC_PORT SP_PLUS_GPIO_Port
#define SP_SPD_DEC_PORT SP_MINUS_GPIO_Port
#define SP_START_PORT INC_MINUS_GPIO_Port
#define SP_STOP_PORT INC_PLUS_GPIO_Port

static void configure_shortcut_speed_keys_to_output_high(void) {
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin = SP_SPD_INC_PIN;	
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = SP_SPD_DEC_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	// GPIO_InitStruct.Pin = SP_SPD_DEC_PIN|SP_START_PIN|SP_STOP_PIN;
	// GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	// GPIO_InitStruct.Pull = GPIO_NOPULL;
	// GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	// HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

	/* Output 3.3V (HIGH) to charge capacitors or stabilize external circuit */
	HAL_GPIO_WritePin(SP_SPD_INC_PORT, SP_SPD_INC_PIN, GPIO_PIN_SET);
	HAL_GPIO_WritePin(SP_SPD_DEC_PORT, SP_SPD_DEC_PIN, GPIO_PIN_SET);
}

static void configure_shortcut_speed_keys_to_input(void) {
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin = SP_SPD_INC_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	/* Speed field not needed for input mode, but clearing it for clarity */
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = SP_SPD_DEC_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	// GPIO_InitStruct.Pin = SP_SPD_DEC_PIN|SP_START_PIN|SP_STOP_PIN;
	// GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
	// GPIO_InitStruct.Pull = GPIO_PULLUP;
	// GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	// HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);
}

static void configure_shortcut_start_stop_keys_to_output(void) {
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin = SP_START_PIN | SP_STOP_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

struct key_shortcut_state {
	uint32_t tick_since_high;
	uint32_t tick_since_low;

	uint32_t delta_high;
	uint32_t delta_low;

	uint8_t pressed;

	uint32_t count;

	uint32_t last_press_time;  // Timestamp do último press (para throttling de 1 segundo)
};

struct key_shortcut_state keys_shortcut[KEY_SHORTCUT_COUNT] = {0};
struct ext_start_stop_combo_state {
  uint8_t active;
  uint8_t config_sent;
  uint32_t pressed_tick;
};

static struct ext_start_stop_combo_state g_ext_start_stop_combo = {0};

static void clear_shortcut_state(enum key_shortcut shortcut_index) {
  keys_shortcut[shortcut_index] = (struct key_shortcut_state){0};
}

static uint8_t handle_external_start_stop_config_combo(void) {
  uint8_t start_pressed = (HAL_GPIO_ReadPin(INC_MINUS_GPIO_Port, INC_MINUS_Pin) == GPIO_PIN_RESET);
  uint8_t stop_pressed = (HAL_GPIO_ReadPin(INC_PLUS_GPIO_Port, INC_PLUS_Pin) == GPIO_PIN_RESET);
  uint8_t both_pressed = (uint8_t)(start_pressed && stop_pressed);
  uint32_t now = HAL_GetTick();

  if (both_pressed) {
    if (!g_ext_start_stop_combo.active) {
      g_ext_start_stop_combo.active = 1;
      g_ext_start_stop_combo.config_sent = 0;
      g_ext_start_stop_combo.pressed_tick = now;
      clear_shortcut_state(KEY_SHORTCUT_INC_DEC);
      clear_shortcut_state(KEY_SHORTCUT_INC_INC);
    }

    if (!g_ext_start_stop_combo.config_sent &&
        (now - g_ext_start_stop_combo.pressed_tick) >= HG_START_STOP_CONFIG_HOLD_MS) {
      keyboard_route_key_event(KEYBOARD_INPUT_SOURCE_HG_START_STOP, KEY_CONFIG);
      g_ext_start_stop_combo.config_sent = 1;
    }

    return 1;
  }

  if (g_ext_start_stop_combo.active) {
    g_ext_start_stop_combo.active = 0;
    g_ext_start_stop_combo.config_sent = 0;
    g_ext_start_stop_combo.pressed_tick = 0;
    clear_shortcut_state(KEY_SHORTCUT_INC_DEC);
    clear_shortcut_state(KEY_SHORTCUT_INC_INC);
    keyboard_route_key_release(KEYBOARD_INPUT_SOURCE_HG_START_STOP);
    return 1;
  }

  return 0;
}

static keyboard_input_source_t shortcut_index_to_input_source(enum key_shortcut shortcut_index) {
  switch (shortcut_index) {
    case KEY_SHORTCUT_SPD_DEC:
    case KEY_SHORTCUT_SPD_INC:
      return KEYBOARD_INPUT_SOURCE_HG_SPEED;
    case KEY_SHORTCUT_INC_DEC:
    case KEY_SHORTCUT_INC_INC:
      return KEYBOARD_INPUT_SOURCE_HG_START_STOP;
    default:
      return KEYBOARD_INPUT_SOURCE_HG_SPEED;
  }
}

static void update_handgrip_release_state(keyboard_input_source_t source) {
  static uint8_t hg_speed_pressed = 0;
  static uint8_t hg_start_stop_pressed = 0;
  uint8_t any_pressed = 0;
  uint8_t *pressed_state = NULL;

  if (source == KEYBOARD_INPUT_SOURCE_HG_SPEED) {
    any_pressed = (HAL_GPIO_ReadPin(SP_MINUS_GPIO_Port, SP_MINUS_Pin) == GPIO_PIN_RESET) ||
                  (HAL_GPIO_ReadPin(SP_PLUS_GPIO_Port, SP_PLUS_Pin) == GPIO_PIN_RESET);
    pressed_state = &hg_speed_pressed;
  } else if (source == KEYBOARD_INPUT_SOURCE_HG_START_STOP) {
    any_pressed = (HAL_GPIO_ReadPin(INC_MINUS_GPIO_Port, INC_MINUS_Pin) == GPIO_PIN_RESET) ||
                  (HAL_GPIO_ReadPin(INC_PLUS_GPIO_Port, INC_PLUS_Pin) == GPIO_PIN_RESET);
    pressed_state = &hg_start_stop_pressed;
  }

  if (pressed_state == NULL) return;

  if (any_pressed) {
    *pressed_state = 1;
  } else if (*pressed_state) {
    keyboard_route_key_release(source);
    *pressed_state = 0;
  }
}

void update_handle_shortcut_keys(GPIO_TypeDef *port, uint16_t pin, enum key_shortcut shortcut_index, kbd_key_t action_key) {
	GPIO_PinState state = HAL_GPIO_ReadPin(port, pin);
  keyboard_input_source_t source = shortcut_index_to_input_source(shortcut_index);
	if (keys_shortcut[shortcut_index].tick_since_high == 0) {
		if (state == GPIO_PIN_SET && HAL_GetTick() > 0) {
			keys_shortcut[shortcut_index].tick_since_high = HAL_GetTick();
			keys_shortcut[shortcut_index].delta_high = 0;
		}
	} 

	if (keys_shortcut[shortcut_index].tick_since_low == 0) {
		if (state == GPIO_PIN_RESET) {
			keys_shortcut[shortcut_index].tick_since_low = HAL_GetTick();
			keys_shortcut[shortcut_index].delta_low = 0;
		}
	}

	if (keys_shortcut[shortcut_index].delta_high == 0) {
		if (state == GPIO_PIN_RESET && keys_shortcut[shortcut_index].tick_since_high > 0) {
			keys_shortcut[shortcut_index].delta_high = HAL_GetTick() - keys_shortcut[shortcut_index].tick_since_high;
			keys_shortcut[shortcut_index].tick_since_high = 0;
		}
	}

	if (state == GPIO_PIN_RESET) {								
		keys_shortcut[shortcut_index].tick_since_high = 0;
		keys_shortcut[shortcut_index].delta_low = HAL_GetTick() - keys_shortcut[shortcut_index].tick_since_low;		
		if (keys_shortcut[shortcut_index].delta_high > 0 && keys_shortcut[shortcut_index].delta_high >= 50
			&& keys_shortcut[shortcut_index].delta_low  > 0 && keys_shortcut[shortcut_index].delta_low >= 50) {
			
			// Throttle para KEY_EXT_KBD_SPEED_INC: apenas permite a cada 1 segundo
			uint32_t current_time = HAL_GetTick();
			int allow_press = 1;
			
			if (action_key == KEY_EXT_KBD_SPEED_INC) {
				if (keys_shortcut[shortcut_index].last_press_time == 0) {
					// Primeiro press, sempre permite
					allow_press = 1;
				} else if (current_time - keys_shortcut[shortcut_index].last_press_time < 1000) {
					// Menos de 1 segundo desde o último press, não permite
					allow_press = 0;
				}
			}
			
			if (allow_press) {
				keyboard_route_key_event(source, action_key);
				keys_shortcut[shortcut_index].count++;
				keys_shortcut[shortcut_index].last_press_time = current_time;
			}
			
			keys_shortcut[shortcut_index].delta_high = 0;
			keys_shortcut[shortcut_index].delta_low  = 0;
		}

    // Keep monitoring continuous press to detect stuck even if no new edge event is generated.
    keyboard_route_key_hold(source, action_key);
	} else {		
		keys_shortcut[shortcut_index].tick_since_low = 0;
		keys_shortcut[shortcut_index].delta_low = 0;
	}

  update_handgrip_release_state(source);
} 

int enable_shortcut_keys_reading = 0;
void handle_shortcut_keys(void) {}

// static void handle_shortcut_keys(void) {
//   static uint32_t debounce = 0;
// #define SHORTCUT_ACKED (0xffffffff)
//   if (treadmill.scene != SCENE_TRAINING){
//     if (HAL_GPIO_ReadPin(INC_MINUS_GPIO_Port, INC_MINUS_Pin) == GPIO_PIN_RESET) {
//       if (debounce == 0) debounce = HAL_GetTick();
//       if (debounce != SHORTCUT_ACKED && HAL_GetTick() - debounce > 50) {
//         debounce = SHORTCUT_ACKED;

//         // changed behavior - start
//         treadmill_key_press(&treadmill, KEY_START);
//         treadmill.hw.beep(BEEP_KEYPRESS_ACK);
//       }

//     } else if (HAL_GPIO_ReadPin(INC_PLUS_GPIO_Port, INC_PLUS_Pin) == GPIO_PIN_RESET) {
//       if (debounce == 0) debounce = HAL_GetTick();
//       if (debounce != SHORTCUT_ACKED && HAL_GetTick() - debounce > 50) {
//         debounce = SHORTCUT_ACKED;
//         // changed behavior - stop 
//         treadmill_key_press(&treadmill, KEY_STOP);
//         treadmill.hw.beep(BEEP_KEYPRESS_ACK);
//       }
//     }
//     else if (HAL_GPIO_ReadPin(SP_MINUS_GPIO_Port, SP_MINUS_Pin) == GPIO_PIN_RESET) {
//       if (debounce == 0) debounce = HAL_GetTick();
//       if (debounce != SHORTCUT_ACKED && HAL_GetTick() - debounce > 50) {
//         debounce = SHORTCUT_ACKED;
//         treadmill.hw.beep(BEEP_KEYPRESS_ACK);
//         treadmill_key_press(&treadmill, KEY_EXT_KBD_SPEED_DEC);
//       }

//     } else if (HAL_GPIO_ReadPin(SP_PLUS_GPIO_Port, SP_PLUS_Pin) == GPIO_PIN_RESET) {
//       if (debounce == 0) debounce = HAL_GetTick();
//       if (debounce != SHORTCUT_ACKED && HAL_GetTick() - debounce > 50) {
//         debounce = SHORTCUT_ACKED;
//         treadmill.hw.beep(BEEP_KEYPRESS_ACK);
//         treadmill_key_press(&treadmill, KEY_EXT_KBD_SPEED_INC);
//       }
//     }
//     else{
//       debounce = 0;
//     }
//   }
//   else {
//     if (HAL_GPIO_ReadPin(SP_MINUS_GPIO_Port, SP_MINUS_Pin) == GPIO_PIN_RESET) {
//       if (debounce == 0) debounce = HAL_GetTick();
//       if (debounce != SHORTCUT_ACKED && HAL_GetTick() - debounce > 50) {
//         debounce = SHORTCUT_ACKED;
//         treadmill.hw.beep(BEEP_KEYPRESS_ACK);
//         treadmill_key_press(&treadmill, KEY_EXT_KBD_SPEED_DEC);
//       }

//     } else if (HAL_GPIO_ReadPin(SP_PLUS_GPIO_Port, SP_PLUS_Pin) == GPIO_PIN_RESET) {
//       if (debounce == 0) debounce = HAL_GetTick();
//       if (debounce != SHORTCUT_ACKED && HAL_GetTick() - debounce > 50) {
//         debounce = SHORTCUT_ACKED;
//         treadmill.hw.beep(BEEP_KEYPRESS_ACK);
//         treadmill_key_press(&treadmill, KEY_EXT_KBD_SPEED_INC);
//       }

//     } else if (HAL_GPIO_ReadPin(INC_MINUS_GPIO_Port, INC_MINUS_Pin) == GPIO_PIN_RESET) {
//       if (debounce == 0) debounce = HAL_GetTick();
//       if (debounce != SHORTCUT_ACKED && HAL_GetTick() - debounce > 50) {
//         debounce = SHORTCUT_ACKED;

//         treadmill.hw.beep(BEEP_KEYPRESS_ACK);
//         // changed behavior - start
//         treadmill_key_press(&treadmill, KEY_START);
//       }

//     } else if (HAL_GPIO_ReadPin(INC_PLUS_GPIO_Port, INC_PLUS_Pin) == GPIO_PIN_RESET) {
//       if (debounce == 0) debounce = HAL_GetTick();
//       if (debounce != SHORTCUT_ACKED && HAL_GetTick() - debounce > 50) {
//         debounce = SHORTCUT_ACKED;
//         // changed behavior - stop 
//         treadmill.hw.beep(BEEP_KEYPRESS_ACK);
//         treadmill_key_press(&treadmill, KEY_STOP);
//       }

//     } else {
//       debounce = 0;
//     }
//   }
// }

static void handle_emergency(void) {
  static uint32_t debounce = 0;
#define EMG_ACKED (0xffffffff)
  _emg = HAL_GPIO_ReadPin(EMG_GPIO_Port, EMG_Pin);
  if (_emg) {
    if (debounce == 0) debounce = HAL_GetTick();
    if (debounce != EMG_ACKED && HAL_GetTick() - debounce > 40) {
      treadmill_emergency_pressed(&treadmill);
      debounce = EMG_ACKED;
    }
  } else {
    debounce = 0;
  }
}

static inline int handle_combined_or_single_key_press(enum key singles, enum key combined, enum key pressed,
                                               enum key *previous, uint32_t *key_tick) {
  if (pressed & (singles | combined)) {
    if (*key_tick == 0) {
      *key_tick = HAL_GetTick();
    } else if (HAL_GetTick() - *key_tick >= 50) {
      if (pressed != combined && *previous == KEY_NONE) {
        keyboard_route_key_event(KEYBOARD_INPUT_SOURCE_MAIN_MATRIX, pressed);
        *previous = pressed;
        return 1;
      } else {
        if (pressed == combined) {
          if (HAL_GetTick() - *key_tick >= 2000) {
            if (pressed != *previous) keyboard_route_key_event(KEYBOARD_INPUT_SOURCE_MAIN_MATRIX, pressed);
            *previous = pressed;
          }
          return 1;
        } else {
          if (!(singles & (KEY_SPEED_INCREMENT | KEY_SPEED_DECREMENT))) return 1;
        }
      }
    }
  }
  return 0;
}

static inline void handle_key_press(void) {
  static enum key pressed = KEY_NONE;
  static enum key previous = KEY_NONE;
  static uint32_t key_tick = 0;
  static uint8_t slow_repeat_count = 0;

  pressed = get_key_pressed();
  g_main_plus_minus_combo_pressed = ((pressed & KEY_LUB) == KEY_LUB) ? 1u : 0u;

  if (handle_combined_or_single_key_press((KEY_START | KEY_STOP), KEY_CONFIG, pressed, &previous, &key_tick)) {
    return;
  }

  if (handle_combined_or_single_key_press((KEY_SPEED_INCREMENT | KEY_SPEED_DECREMENT), KEY_LUB, pressed, &previous,
                                          &key_tick)) {
    return;
  }

  if (pressed != KEY_NONE && pressed != previous) {
    keyboard_route_key_event(KEYBOARD_INPUT_SOURCE_MAIN_MATRIX, pressed);
    key_tick = 0;
    slow_repeat_count = 0;
    previous = pressed;
  } else if (pressed != KEY_NONE) {
    if (slow_repeat_count <= 10) {
      osDelay(25);
    } else if (slow_repeat_count < 21) {
      osDelay(200);
    } else {
      osDelay(75);
    }
    if (slow_repeat_count < 255) slow_repeat_count++;
    if (slow_repeat_count <= 10) return;
    keyboard_route_key_event(KEYBOARD_INPUT_SOURCE_MAIN_MATRIX, pressed);
  } else {
    key_tick = 0;
    previous = pressed;
    slow_repeat_count = 0;
    keyboard_route_key_release(KEYBOARD_INPUT_SOURCE_MAIN_MATRIX);
    treadmill_key_press_clear(&treadmill);
  }
}

/* USER CODE END Header_KeyboardTask */
void KeyboardTask(void const * argument)
{
  /* USER CODE BEGIN KeyboardTask */
  /* Infinite loop */
  for (;;) {
    handle_emergency();
    enable_shortcut_keys_reading = 1;
    // handle_shortcut_keys();
    handle_key_press();
    ext_i2c_kbd_handler();
    keyboard_service_idle_keyboard_fault_clear_combo();
    osDelay(1);  // 2 ms
  }
  /* USER CODE END KeyboardTask */
}

/* USER CODE BEGIN Header_BeepTask */
/**
 * @brief Function implementing the beepTask thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_BeepTask */
void BeepTask(void const * argument)
{
  /* USER CODE BEGIN BeepTask */
  beep_t beep;

  beep = BEEP_KEYPRESS_ACK;
  xQueueSend(beepEventQueueHandle, &beep, 0);
  /* Infinite loop */
  for (;;) {
    if (xQueueReceive(beepEventQueueHandle, &beep, portMAX_DELAY) == pdPASS) {
      // continue; //debug only
      
      HAL_TIM_PWM_Stop(&htim16, TIM_CHANNEL_1);
      switch (beep) {
        case BEEP_KEYPRESS_ACK: {
           TIM16->CNT = 0;
           TIM16->PSC = 120;
           TIM16->ARR = 40;
           TIM16->CCR2 = 10;
           HAL_TIM_PWM_Start(&htim16, TIM_CHANNEL_1);
           osTimerStart(beepTimerHandle, pdMS_TO_TICKS(100));
          break;
        }
        case BEEP_KEYPRESS_NAK: {
           TIM16->CNT = 0;
           TIM16->PSC = 600;
           TIM16->ARR = 40;
           TIM16->CCR2 = 20;
           HAL_TIM_PWM_Start(&htim16, TIM_CHANNEL_1);
           osTimerStart(beepTimerHandle, pdMS_TO_TICKS(100));
          break;
        }
        case BEEP_COUNTDOWN_COUNT: {
           TIM16->CNT = 0;
           TIM16->PSC = 120 * 2;
           TIM16->ARR = 40;
           TIM16->CCR2 = 20;
           HAL_TIM_PWM_Start(&htim16, TIM_CHANNEL_1);
           osTimerStart(beepTimerHandle, pdMS_TO_TICKS(250));
          break;
        }
        case BEEP_COUNTDOWN_END: {
           TIM16->CNT = 0;
           TIM16->PSC = 110 * 2;
           TIM16->ARR = 40;
           TIM16->CCR2 = 20;
           HAL_TIM_PWM_Start(&htim16, TIM_CHANNEL_1);
           osTimerStart(beepTimerHandle, pdMS_TO_TICKS(500));
          break;
        }
        case BEEP_REST: {
           TIM16->CNT = 0;
           TIM16->PSC = 120;
           TIM16->ARR = 40;
           TIM16->CCR2 = 2;
           HAL_TIM_PWM_Start(&htim16, TIM_CHANNEL_1);
           osTimerStart(beepTimerHandle, pdMS_TO_TICKS(50));
          break;
        }
        case BEEP_PROGRAM_FINISHED: {
           TIM16->CNT = 0;
           TIM16->PSC = 110;
           TIM16->ARR = 40;
           TIM16->CCR2 = 20;
           HAL_TIM_PWM_Start(&htim16, TIM_CHANNEL_1);
           osTimerStart(beepTimerHandle, pdMS_TO_TICKS(500));
          break;
        }
        case BEEP_EMERGENCY: {
           TIM16->CNT = 0;
           TIM16->PSC = 110;
           TIM16->ARR = 40;
           TIM16->CCR2 = 20;
           HAL_TIM_PWM_Start(&htim16, TIM_CHANNEL_1);
           osTimerStart(beepTimerHandle, pdMS_TO_TICKS(1000));
          break;
        }
      }
    }
    osDelay(1);
  }
  /* USER CODE END BeepTask */
}

/* beepTimerCallback function */
void beepTimerCallback(void const * argument)
{
  /* USER CODE BEGIN beepTimerCallback */
  HAL_TIM_PWM_Stop(&htim16, TIM_CHANNEL_1);
  /* USER CODE END beepTimerCallback */
}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM1 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM1) {
    HAL_IncTick();

    extern int enable_shortcut_keys_reading;
    if (enable_shortcut_keys_reading == 1) {
      uint8_t hg_sp_plus_pressed = (HAL_GPIO_ReadPin(SP_PLUS_GPIO_Port, SP_PLUS_Pin) == GPIO_PIN_RESET);
      uint8_t hg_sp_minus_pressed = (HAL_GPIO_ReadPin(SP_MINUS_GPIO_Port, SP_MINUS_Pin) == GPIO_PIN_RESET);
      g_hg_plus_minus_combo_pressed = (uint8_t)(hg_sp_plus_pressed && hg_sp_minus_pressed);

      uint8_t ext_start_stop_combo = handle_external_start_stop_config_combo();

      // put external shortcut keys filter logic here.
      // Call for each shortcut key (using GPIO definitions from main.h):
      // SPD_INC: SP_MINUS_GPIO_Port/SP_MINUS_Pin -> KEY_EXT_KBD_SPEED_INC
      update_handle_shortcut_keys(SP_MINUS_GPIO_Port, SP_PLUS_Pin, KEY_SHORTCUT_SPD_INC, KEY_EXT_KBD_SPEED_INC);
      // SPD_DEC: SP_PLUS_GPIO_Port/SP_PLUS_Pin -> KEY_EXT_KBD_SPEED_DEC
      update_handle_shortcut_keys(SP_PLUS_GPIO_Port, SP_MINUS_Pin, KEY_SHORTCUT_SPD_DEC, KEY_EXT_KBD_SPEED_DEC);
      if (!ext_start_stop_combo) {
        // START: INC_MINUS_GPIO_Port/INC_MINUS_Pin -> KEY_EXT_KBD_START
        update_handle_shortcut_keys(INC_MINUS_GPIO_Port, INC_MINUS_Pin, KEY_SHORTCUT_INC_DEC, KEY_EXT_KBD_START);
        // STOP: INC_PLUS_GPIO_Port/INC_PLUS_Pin -> KEY_EXT_KBD_STOP
        update_handle_shortcut_keys(INC_PLUS_GPIO_Port, INC_PLUS_Pin, KEY_SHORTCUT_INC_INC, KEY_EXT_KBD_STOP);
      }
    } else {
      g_hg_plus_minus_combo_pressed = 0;
    }
  }
  /* USER CODE BEGIN Callback 1 */
  else if (htim->Instance == TIM6) {    

	if (HAL_GPIO_ReadPin(LO_MINUS_GPIO_Port, LO_MINUS_Pin) == GPIO_PIN_SET ||
	  HAL_GPIO_ReadPin(LO_PLUS_GPIO_Port, LO_PLUS_Pin) == GPIO_PIN_SET) {
	  return;
	}

	if (uwTick - adc_peak_tick >= 1000) {
		adc_peak_delta = adc_peak_high - adc_peak_low;
		adc_peak_delta_average  = (adc_peak_delta_average*4 + adc_peak_delta)/5;
		adc_peak_low = adc_average_n;
		adc_peak_high = 0;
		adc_peak_tick = uwTick;
	} else {
		if (adc_below_average) {
		  if (adc_hrm < adc_peak_low) {
			  adc_peak_low = adc_hrm;
		  }
		} else {
		  if (adc_hrm > adc_peak_high) {
			  adc_peak_high = adc_hrm;
		  }
		}
	}

	uint32_t threshold = adc_average_n + ADC_THRESHOLD;
	if (adc_peak_delta_average < adc_peak_high && adc_peak_delta_average >= threshold) {
		threshold = adc_peak_delta_average / 3;
	}

    adc_average = ((adc_average * 255) + (adc_hrm * 1000)) / 256;
    adc_average_n = adc_average / 1000;

    if (adc_hrm > (adc_average_n + ADC_THRESHOLD) && adc_below_average) {
      adc_new_beat = HAL_GetTick();
      uint32_t diff = adc_new_beat - adc_old_beat;
      adc_below_average = 0;

      if (diff < 250) {
        // BPM > 240 bpm
      } else {

  	    if (adc_hrm > adc_peak_high) {
		  adc_peak_high = adc_hrm;
	    }

        uint32_t current_bpm = 60000 / diff;

        uint32_t bpm_totals = 0;
        for (int i = 0; i < BPM_BUFFER_READS; ++i) {
          bpm_totals += bpm_buffer_reads[i];
        }

        bpm_current_value = bpm_totals / BPM_BUFFER_READS;

        bpm_buffer_reads[bpm_buffer_index] = ((bpm_current_value * 1) + current_bpm) / 2;
        bpm_buffer_index = (bpm_buffer_index + 1) % BPM_BUFFER_READS;

        adc_old_beat = adc_new_beat;

        // if not reading PolarBPM, use Handgrip
        if (PolarBPM == 0 && adc_new_beat - adc_delay_tick_ms > 2000) {
          if (bpm_current_value < 40) {
            treadmill_set_bpm_read(&treadmill);
          } else {
            treadmill_set_bpm_value(&treadmill, (uint8_t)bpm_current_value);
          }

        }

      }
    } else if (adc_hrm < adc_average_n) {
      adc_below_average = 1;
    }
  }
  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1) {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
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
  /* User can add his own implementation to report the file name and line
     number, ex: printf("Wrong parameters value: file %s on line %d\r\n", file,
     line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
