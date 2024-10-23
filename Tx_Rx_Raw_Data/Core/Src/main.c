/* USER CODE BEGIN Header */
/** Author: Megan Sorour, 2024
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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
#include "dma.h"
#include "app_subghz_phy.h"
#include "gpio.h"
#include "usart.h"
#include "sys_app.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "data.h"
#include "compression.h"
#include "subghz_phy_app.h"
#include "usart.h"
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <stdlib.h>
#include <stdarg.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

//Compressed sensor data arrays
uint8_t compressed_x[(NUM_ENTRIES)*2]; //compressed sensor data, x. number of entries
uint8_t compressed_y[(NUM_ENTRIES)*2]; //compressed sensor data, x. number of entries
uint8_t compressed_z[(NUM_ENTRIES)*2]; //compressed sensor data, x. number of entries
uint8_t compressed_r[(NUM_ENTRIES)*2]; //compressed sensor data, x. number of entries
uint8_t compressed_q[(NUM_ENTRIES)*2]; //compressed sensor data, x. number of entries
uint8_t compressed_p[(NUM_ENTRIES)*2]; //compressed sensor data, x. number of entries

//Compressed sensor data array size (in bits)
uint32_t compressed_size_bits_x;
uint32_t compressed_size_bits_y;
uint32_t compressed_size_bits_z;
uint32_t compressed_size_bits_r;
uint32_t compressed_size_bits_q;
uint32_t compressed_size_bits_p;

//Compressed time data dynamic arrays
DynamicArray_8 encoded_vals_h; //Dynamically sized array to store RLE time values (8 bit)
DynamicArray_16 encoded_count_h; //Dynamically sized array to store run length of values (16 bit)
DynamicArray_8 encoded_vals_m; //Dynamically sized array to store RLE time values (8 bit)
DynamicArray_16 encoded_count_m; //Dynamically sized array to store run length of values (16 bit)
DynamicArray_8 encoded_vals_s; //Dynamically sized array to store RLE time values (8 bit)
DynamicArray_16 encoded_count_s; //Dynamically sized array to store run length of values (16 bit)

timestamp_packet TimestampPkt;

uint32_t encode_size = NUM_ENTRIES; //number of entries in data log. Same for all

uint32_t startTime, endTime, elapsedTime; //for timing

uint8_t array_tx_complete = 0;

int16_t decode[NUM_ENTRIES];
uint8_t *map = 0; //no Huffman map used


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

// Function to verify compression algorithm
bool verify(int16_t *dec, int16_t *enc, uint32_t size);
void compression_demo(void);

void printmsg(char* format, ...);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

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
  MX_DMA_Init();
  MX_SubGHz_Phy_Init();


#if(1) //If this node is acting as the transmitter

#if (1) // If for experimentation



  array_tx_complete = 0;
  //Create DataEntry
  DataEntry W1;
  W1.node_ID = nodeID_1;
  W1.data_name = dataName;

  //compress all raw data: (x values)
  compression_demo();

  //assign data to type
  W1.x_vals = compressed_x;
  W1.y_vals = compressed_y;
  W1.z_vals = compressed_z;
  W1.r_vals = compressed_r;
  W1.q_vals = compressed_q;
  W1.p_vals = compressed_p;

//  //Spreading Factor Test
//  startTime = HAL_GetTick();
//  Tx_Compressed(W1.x_vals, compressed_size_bits_x, TYPE_X, 0 , 0 , 0);
//  endTime = HAL_GetTick();
//  elapsedTime = endTime - startTime;
//  APP_PRINTF("Test 1 elapsed: %d ms \r\n", elapsedTime)
//  HAL_Delay(300);
//
//  startTime = HAL_GetTick();
//  Tx_Compressed(W1.x_vals, compressed_size_bits_x, TYPE_X, 1 , 0 , 1);
//  endTime = HAL_GetTick();
//  elapsedTime = endTime - startTime;
//  APP_PRINTF("Test 2 elapsed: %d ms\r\n", elapsedTime)
//  HAL_Delay(300);
//
//  startTime = HAL_GetTick();
//  Tx_Compressed(W1.x_vals, compressed_size_bits_x, TYPE_X, 2 , 0 , 2);
//  endTime = HAL_GetTick();
//  elapsedTime = endTime - startTime;
//  APP_PRINTF("Test 3 elapsed: %d ms\r\n", elapsedTime)
//  HAL_Delay(300);
//
//  startTime = HAL_GetTick();
//  Tx_Compressed(W1.x_vals, compressed_size_bits_x, TYPE_X, 3 , 0 , 3);
//  endTime = HAL_GetTick();
//  elapsedTime = endTime - startTime;
//  APP_PRINTF("Test 4 elapsed: %d ms\r\n", elapsedTime)
//  HAL_Delay(300);

//  //Coding Rate Test
//  startTime = HAL_GetTick();
//  Tx_Compressed(W1.x_vals, compressed_size_bits_x, TYPE_X, 0 , 1 , 0);
//  endTime = HAL_GetTick();
//  elapsedTime = endTime - startTime;
//  APP_PRINTF("Test 5 elapsed: %d ms\r\n", elapsedTime)
//  HAL_Delay(300);
//
//  startTime = HAL_GetTick();
//  Tx_Compressed(W1.x_vals, compressed_size_bits_x, TYPE_X, 0 , 2 , 0);
//  endTime = HAL_GetTick();
//  elapsedTime = endTime - startTime;
//  APP_PRINTF("Test 6 elapsed: %d ms\r\n", elapsedTime)
//  HAL_Delay(300);
//
//  startTime = HAL_GetTick();
//  Tx_Compressed(W1.x_vals, compressed_size_bits_x, TYPE_X, 0 , 3 , 0);
//  endTime = HAL_GetTick();
//  elapsedTime = endTime - startTime;
//  APP_PRINTF("Test 7 elapsed: %d ms\r\n", elapsedTime)
//  HAL_Delay(300);
//
  //Tx Ouput Power Test
  startTime = HAL_GetTick();
  Tx_Compressed(W1.x_vals, compressed_size_bits_x, TYPE_X, 0 , 0 , 0);
  endTime = HAL_GetTick();
  elapsedTime = endTime - startTime;
  APP_PRINTF("Test 8 elapsed: %d ms\r\n", elapsedTime)
  HAL_Delay(300);

  startTime = HAL_GetTick();
  Tx_Compressed(W1.x_vals, compressed_size_bits_x, TYPE_X, 0 , 0 , 1);
  endTime = HAL_GetTick();
  elapsedTime = endTime - startTime;
  APP_PRINTF("Test 9 elapsed: %d ms\r\n", elapsedTime)
  HAL_Delay(300);


  startTime = HAL_GetTick();
  Tx_Compressed(W1.x_vals, compressed_size_bits_x, TYPE_X, 0 , 0 , 2);
  endTime = HAL_GetTick();
  elapsedTime = endTime - startTime;
  APP_PRINTF("Test 10 elapsed: %d ms\r\n", elapsedTime)
  HAL_Delay(300);


  startTime = HAL_GetTick();
  Tx_Compressed(W1.x_vals, compressed_size_bits_x, TYPE_X, 0 , 0 , 3);
  endTime = HAL_GetTick();
  elapsedTime = endTime - startTime;
  APP_PRINTF("Test 11 elapsed: %d ms\r\n", elapsedTime)
  HAL_Delay(300);


  startTime = HAL_GetTick();
  Tx_Compressed(W1.x_vals, compressed_size_bits_x, TYPE_X, 0 , 0 , 4);
  endTime = HAL_GetTick();
  elapsedTime = endTime - startTime;
  APP_PRINTF("Test 12 elapsed: %d ms\r\n", elapsedTime);
  APP_PRINTF("Finished transmission of data log %u\r\n", dataName);
#endif

#if (0) //If for transmission of entire wave data log
 timestamp_compression_demo();

 Tx_Compressed_Timestamp(&TimestampPkt);
 HAL_Delay(200);
 
 Tx_Compressed(W1.x_vals, compressed_size_bits_x, TYPE_X, 0 , 0 , 0);
 HAL_Delay(200);

 Tx_Compressed(W1.y_vals, compressed_size_bits_y, TYPE_Y, 0 , 0 , 0);
 HAL_Delay(200);

 Tx_Compressed(W1.z_vals, compressed_size_bits_z, TYPE_Z, 0 , 0 , 0);
 HAL_Delay(200);

 Tx_Compressed(W1.r_vals, compressed_size_bits_r, TYPE_R, 0 , 0 , 0);
 HAL_Delay(200);

 Tx_Compressed(W1.q_vals, compressed_size_bits_q, TYPE_Q, 0 , 0 , 0);
 HAL_Delay(200);

 Tx_Compressed(W1.p_vals, compressed_size_bits_p, TYPE_P, 0 , 0 , 0);
 HAL_Delay(200);

  APP_PRINTF("Finished transmission of data log %u\r\n", dataName);

#endif
#endif


  /* USER CODE END 2 */
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

    /* USER CODE END WHILE */
    MX_SubGHz_Phy_Process();
	  //transmit compressed_x

	  //just compress the data in init section

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

  /** Configure LSE Drive Capability
  */
  HAL_PWR_EnableBkUpAccess();
  __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSE|RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = RCC_MSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_11;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure the SYSCLKSource, HCLK, PCLK1 and PCLK2 clocks dividers
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK3|RCC_CLOCKTYPE_HCLK
                              |RCC_CLOCKTYPE_SYSCLK|RCC_CLOCKTYPE_PCLK1
                              |RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.AHBCLK3Divider = RCC_SYSCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

//compressing functions
bool verify(int16_t *dec, int16_t *enc, uint32_t size)
{
    bool match = true;
    for(size_t pos = 0 ; pos < size ; pos++)
    {
        if(dec[pos] != enc[pos])
        {
            APP_PRINTF("Invalid byte pos %lu (%d <> %d)\r\n",pos,dec[pos],enc[pos]);
            match = false;
            break;
        }
    }

    return match;
}

void compression_demo(void)
{
    float comp_rate = dhc_compress_evaluate(x_vals,encode_size,&compressed_size_bits_x,map); //evaluating what size of compressed data would be

    if(comp_rate > 0)
    {
        HAL_Delay(10);

        startTime =  HAL_GetTick();
        dhc_compress(compressed_x,&compressed_size_bits_x,x_vals,encode_size,map);

        dhc_compress(compressed_y,&compressed_size_bits_y,y_vals,encode_size,map);

        dhc_compress(compressed_z,&compressed_size_bits_z,z_vals,encode_size,map);

        dhc_compress(compressed_r,&compressed_size_bits_r,r_vals,encode_size,map);

        dhc_compress(compressed_q,&compressed_size_bits_q,q_vals,encode_size,map);

        dhc_compress(compressed_p,&compressed_size_bits_p,p_vals,encode_size,map);

        endTime = HAL_GetTick();
        elapsedTime = endTime - startTime;
        APP_PRINTF("Compression completed in: %u ms\r\n", elapsedTime);
        uint32_t data_log_bits = compressed_size_bits_x + compressed_size_bits_y + compressed_size_bits_z + compressed_size_bits_r + compressed_size_bits_q + compressed_size_bits_p;

        APP_PRINTF("Size before compression: %d bytes, after compression: %d bytes\r\n", encode_size*2*6, data_log_bits/8);

//#if (PRINT_ALL == 1)
//        //Printing compressed data
//        APP_PRINTF("Compressed data: \r\n");
//        APP_PRINTF("Data Type %d \r\n", TYPE_X);
//     	for (int32_t i = 0; i < compressed_size_bits_x/8; i++)
//     		{
//				APP_PRINTF("%d ", compressed_x[i]);
//				if ((i % 16) == 15)
//					{
//					APP_PRINTF("\n");
//					}
//				HAL_Delay(10);
//     		}
//     	APP_PRINTF("\n\r");
//        APP_PRINTF("Data Type %d", TYPE_Y);
//     	for (int32_t i = 0; i < compressed_size_bits_y/8; i++)
//     		{
//				APP_PRINTF("%d ", compressed_y[i]);
//				if ((i % 16) == 15)
//					{
//					APP_PRINTF("\n");
//					}
//				HAL_Delay(10);
//     		}
     	APP_PRINTF("\n\r");
//#endif

    }
    else
    {
        APP_PRINTF("Compressor will not reduce the data ");
    }
}

void unpack_data(uint8_t *packed_count, uint16_t *unpacked_count, uint8_t length) {
    for (size_t i = 0; i < length; i++) {
        // Unpack two 8-bit values into one 16-bit value
        unpacked_count[i] = (packed_count[2 * i] << 8) | packed_count[2 * i + 1];
        //APP_PRINTF("Unpacked Value %d: %u\r\n", i, unpacked_count[i]);
    }
}

void timestamp_compression_demo(void){

	uint32_t time_total_before_size = (encode_size)*3;
	APP_PRINTF("Original size of timestamp array: %d bytes\r\n", time_total_before_size);

	startTime =  HAL_GetTick();
	compress_timestamp(hours, encode_size, &encoded_vals_h, &encoded_count_h);
	//HAL_Delay(10);
	compress_timestamp(minutes, encode_size, &encoded_vals_m, &encoded_count_m);
	//HAL_Delay(10);
	compress_timestamp(seconds, encode_size, &encoded_vals_s, &encoded_count_s);
	endTime = HAL_GetTick();
	elapsedTime = endTime - startTime;

	APP_PRINTF("Compression completed in: %u ms\r\n", elapsedTime);
//#if (PRINT_ALL == 1)
	APP_PRINTF("Hours:\r\n");
	for (size_t i = 0; i < encoded_vals_h.size; i ++) {
			APP_PRINTF("Value: %d ", encoded_vals_h.data[i]);
			  APP_PRINTF("Count: %d\n", encoded_count_h.data[i]);
			  HAL_Delay(10);
		}
	APP_PRINTF("Minutes:\r\n");
	for (size_t i = 0; i < encoded_vals_m.size; i ++) {
			APP_PRINTF("Value: %d ", encoded_vals_m.data[i]);
			  APP_PRINTF("Count: %d\n", encoded_count_m.data[i]);
			  HAL_Delay(10);
		}
	APP_PRINTF("Seconds:\r\n");
	for (size_t i = 0; i < encoded_vals_s.size; i ++) {
			APP_PRINTF("Value: %d ", encoded_vals_s.data[i]);
			  APP_PRINTF("Count: %d\n", encoded_count_s.data[i]);
			  HAL_Delay(10);
		}
//#endif


	uint32_t time_total_compressed_size = (encoded_vals_h.size + encoded_count_h.size*2) + (encoded_vals_m.size + encoded_count_m.size*2) + (encoded_vals_s.size + encoded_count_s.size*2);
	APP_PRINTF("Size of timestamp array after compression: %d bytes\n", time_total_compressed_size);

	//Write data to timestamp packet type, casting all to uint8_t array for transmission
	TimestampPkt.header = TIMESTAMP_HEADER;
//Hours
	TimestampPkt.encoded_vals_h = encoded_vals_h.data;
	TimestampPkt.h_length = encoded_vals_h.size;

	//Packing 16 bit array into 8 bit array for radio transfer
	uint8_t* encoded_count_h_8 = (uint8_t*)malloc(encoded_count_h.size * 2 * sizeof(uint8_t));
	for (size_t i = 0; i < encoded_count_h.size; i++) {
		encoded_count_h_8[2 * i]     = (uint8_t)(encoded_count_h.data[i] >> 8);      // High byte
		encoded_count_h_8[2 * i + 1] = (uint8_t)(encoded_count_h.data[i] & 0xFF);    // Low byte
	    }
	TimestampPkt.encoded_count_h = encoded_count_h_8; //length of count_8 is double that of vals

//Minutes
	TimestampPkt.encoded_vals_m = encoded_vals_m.data;
	TimestampPkt.m_length = encoded_vals_m.size;

	uint8_t* encoded_count_m_8 = (uint8_t*)malloc(encoded_count_m.size * 2 * sizeof(uint8_t));
	for (size_t i = 0; i < encoded_count_h.size; i++) {
		encoded_count_m_8[2 * i]     = (uint8_t)(encoded_count_m.data[i] >> 8);      // High byte
		encoded_count_m_8[2 * i + 1] = (uint8_t)(encoded_count_m.data[i] & 0xFF);    // Low byte
	    }
	TimestampPkt.encoded_count_m = encoded_count_m_8; //length of count_8 is double that of vals

//Seconds
	TimestampPkt.encoded_vals_s = encoded_vals_s.data;
	TimestampPkt.s_length = encoded_vals_s.size;

	uint8_t* encoded_count_s_8 = (uint8_t*)malloc(encoded_count_s.size * 2 * sizeof(uint8_t));
	for (size_t i = 0; i < encoded_count_s.size; i++) {
		encoded_count_s_8[2 * i]     = (uint8_t)(encoded_count_s.data[i] >> 8);      // High byte
		encoded_count_s_8[2 * i + 1] = (uint8_t)(encoded_count_s.data[i] & 0xFF);    // Low byte
	    }
	TimestampPkt.encoded_count_s = encoded_count_s_8; //length of count_8 is double that of vals
	//APP_PRINTF("Size s 8 bit: %d\r\n", encoded_count_s.size * 2 * sizeof(uint8_t));

//#if (PRINT_ALL == 1)
//    for (uint32_t i = 0; i < encoded_count_s.size*2; i++) {
//        APP_PRINTF("packed_vals[%u] = %u\n", (unsigned int)i, encoded_count_s_8[i]);
//        HAL_Delay(10);
//    }
//#endif
    uint16_t unpacked[25];
	//Unpack for debug
    unpack_data(encoded_count_s_8, unpacked, TimestampPkt.s_length);

	// Free allocated memory
	freeArray_8(&encoded_vals_h);
	freeArray_16(&encoded_count_h);
	freeArray_8(&encoded_vals_m);
	freeArray_16(&encoded_count_m);
	freeArray_8(&encoded_vals_s);
	freeArray_16(&encoded_count_s);
}

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
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

//Debug Print (from Michael)
//void printmsg(char *format,...) {
//    char str[80];
//
//    /*Extract the the argument list using VA apis */
//    va_list args;
//    va_start(args, format);
//    vsprintf(str, format,args);
//    HAL_UART_Transmit(&huart1,(uint8_t *)str, strlen(str),HAL_MAX_DELAY);
//    va_end(args);
//}
