/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "spi.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include <stdio.h>

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define MCP2515_CS_PORT GPIOB
#define MCP2515_CS_PIN  GPIO_PIN_6

#define MCP_RESET         0xC0
#define MCP_READ          0x03
#define MCP_WRITE         0x02
#define MCP_BITMOD        0x05
#define MCP_RTS_TX0       0x81

#define MCP_CANSTAT       0x0E
#define MCP_CANCTRL       0x0F
#define MCP_CNF3          0x28
#define MCP_CNF2          0x29
#define MCP_CNF1          0x2A
#define MCP_CANINTE       0x2B
#define MCP_CANINTF       0x2C
#define MCP_TXB0CTRL      0x30
#define MCP_TXB0SIDH      0x31
#define MCP_TXB0SIDL      0x32
#define MCP_TXB0DLC       0x35
#define MCP_TXB0D0        0x36

#define MCP_RXB0CTRL      0x60

#define MCP_MODE_NORMAL   0x00
#define MCP_MODE_CONFIG   0x80
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* USER CODE BEGIN PV */
uint32_t adc_raw = 0;
uint16_t throttle = 0;
uint8_t canstat = 0;
uint8_t canctrl = 0;
/* USER CODE END PV */
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void MCP2515_Select(void);
void MCP2515_Unselect(void);
void MCP2515_Reset(void);
uint8_t MCP2515_ReadRegister(uint8_t address);
void MCP2515_WriteRegister(uint8_t address, uint8_t data);
void MCP2515_BitModify(uint8_t address, uint8_t mask, uint8_t data);
void MCP2515_SetMode(uint8_t mode);
void MCP2515_Init_125kbps_8MHz(void);
void MCP2515_SendThrottle(uint16_t throttle_value);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/* USER CODE BEGIN 0 */
int __io_putchar(int ch)
{
    HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
    return ch;
}
void MCP2515_Select(void)
{
    HAL_GPIO_WritePin(MCP2515_CS_PORT, MCP2515_CS_PIN, GPIO_PIN_RESET);
}

void MCP2515_Unselect(void)
{
    HAL_GPIO_WritePin(MCP2515_CS_PORT, MCP2515_CS_PIN, GPIO_PIN_SET);
}

void MCP2515_Reset(void)
{
    uint8_t cmd = MCP_RESET;

    MCP2515_Select();
    HAL_SPI_Transmit(&hspi1, &cmd, 1, HAL_MAX_DELAY);
    MCP2515_Unselect();

    HAL_Delay(10);
}

uint8_t MCP2515_ReadRegister(uint8_t address)
{
    uint8_t tx[3];
    uint8_t rx[3];
    uint8_t value;

    tx[0] = MCP_READ;
    tx[1] = address;
    tx[2] = 0x00;

    MCP2515_Select();
    HAL_SPI_TransmitReceive(&hspi1, tx, rx, 3, HAL_MAX_DELAY);
    MCP2515_Unselect();

    value = rx[2];
    return value;
}

void MCP2515_WriteRegister(uint8_t address, uint8_t data)
{
    uint8_t tx[3];

    tx[0] = MCP_WRITE;
    tx[1] = address;
    tx[2] = data;

    MCP2515_Select();
    HAL_SPI_Transmit(&hspi1, tx, 3, HAL_MAX_DELAY);
    MCP2515_Unselect();
}
void MCP2515_BitModify(uint8_t address, uint8_t mask, uint8_t data)
{
    uint8_t tx[4];

    tx[0] = MCP_BITMOD;
    tx[1] = address;
    tx[2] = mask;
    tx[3] = data;

    MCP2515_Select();
    HAL_SPI_Transmit(&hspi1, tx, 4, HAL_MAX_DELAY);
    MCP2515_Unselect();
}

void MCP2515_SetMode(uint8_t mode)
{
    MCP2515_BitModify(MCP_CANCTRL, 0xE0, mode);

    HAL_Delay(10);
}

void MCP2515_Init_125kbps_8MHz(void)
{
    MCP2515_Reset();
    HAL_Delay(10);

    MCP2515_SetMode(MCP_MODE_CONFIG);

    /* 125 kbps @ 8 MHz */
    MCP2515_WriteRegister(MCP_CNF1, 0x01);
    MCP2515_WriteRegister(MCP_CNF2, 0xB1);
    MCP2515_WriteRegister(MCP_CNF3, 0x05);

    /* Accept all messages on RXB0 */
    MCP2515_WriteRegister(MCP_RXB0CTRL, 0x60);

    /* Disable interrupts for now */
    MCP2515_WriteRegister(MCP_CANINTE, 0x00);
    MCP2515_WriteRegister(MCP_CANINTF, 0x00);

    /* Clear TX buffer control */
    MCP2515_WriteRegister(MCP_TXB0CTRL, 0x00);

    MCP2515_SetMode(MCP_MODE_NORMAL);
}

void MCP2515_SendThrottle(uint16_t throttle_value)
{
    uint8_t txdata[2];
    uint8_t rts = MCP_RTS_TX0;

    txdata[0] = (uint8_t)(throttle_value >> 8);
    txdata[1] = (uint8_t)(throttle_value & 0xFF);

    /* Standard ID = 0x100 */
    MCP2515_WriteRegister(MCP_TXB0SIDH, 0x20);  // 0x100 >> 3
    MCP2515_WriteRegister(MCP_TXB0SIDL, 0x00);

    /* Data length = 2 */
    MCP2515_WriteRegister(MCP_TXB0DLC, 0x02);

    /* Load data bytes */
    MCP2515_WriteRegister(MCP_TXB0D0, txdata[0]);
    MCP2515_WriteRegister(MCP_TXB0D0 + 1, txdata[1]);

    /* Request to send TX buffer 0 */
    MCP2515_Select();
    HAL_SPI_Transmit(&hspi1, &rts, 1, HAL_MAX_DELAY);
    MCP2515_Unselect();
}
/* USER CODE END 0 */
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

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
  MX_USART2_UART_Init();
  MX_ADC1_Init();
  MX_SPI1_Init();
  /* USER CODE BEGIN 2 */
  printf("STM32 ADC + MCP2515 TX test started\r\n");

  MCP2515_Unselect();
  HAL_Delay(10);

  MCP2515_Init_125kbps_8MHz();

  canstat = MCP2515_ReadRegister(MCP_CANSTAT);
  canctrl = MCP2515_ReadRegister(MCP_CANCTRL);

  printf("CANSTAT = 0x%02X\r\n", canstat);
  printf("CANCTRL = 0x%02X\r\n", canctrl);
  printf("MCP2515 initialized at 125 kbps (8 MHz crystal)\r\n");
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {HAL_ADC_Start(&hadc1);
  HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY);
  adc_raw = HAL_ADC_GetValue(&hadc1);
  HAL_ADC_Stop(&hadc1);

  throttle = (uint16_t)((adc_raw * 1000) / 4095);

  MCP2515_SendThrottle(throttle);

  printf("ADC raw = %lu | Throttle = %u | CAN TX sent\r\n", adc_raw, throttle);

  HAL_Delay(200);
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
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
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
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
