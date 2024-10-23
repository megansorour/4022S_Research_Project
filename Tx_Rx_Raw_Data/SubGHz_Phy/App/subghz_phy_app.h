/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    subghz_phy_app.h
  * @author  MCD Application Team
  * @brief   Header of application of the SubGHz_Phy Middleware
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SUBGHZ_PHY_APP_H__
#define __SUBGHZ_PHY_APP_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdbool.h>
#include <stdint.h>
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
// Set to 1 to print everything.
#define PRINT_ALL 0
#define DEBUG_PRINT 0
#define TEST_MODE                     RADIO_TX

/* MODEM type: one shall be 1 the other shall be 0 */
#define USE_MODEM_LORA  1
#define USE_MODEM_FSK   0

#define RF_FREQUENCY                                433000000 /* Hz */

#ifndef TX_OUTPUT_POWER   /* please, to change this value, redefine it in USER CODE SECTION */
#define TX_OUTPUT_POWER                             10        /* dBm */
#endif /* TX_OUTPUT_POWER */

#if (( USE_MODEM_LORA == 1 ) && ( USE_MODEM_FSK == 0 ))
#define LORA_BANDWIDTH                              0         /* [0: 125 kHz, 1: 250 kHz, 2: 500 kHz, 3: Reserved] */
#define LORA_SPREADING_FACTOR                       7         /* [SF7..SF12] */
#define LORA_CODINGRATE                             1         /* [1: 4/5, 2: 4/6, 3: 4/7, 4: 4/8] */
#define LORA_PREAMBLE_LENGTH                        8         /* Same for Tx and Rx */
#define LORA_SYMBOL_TIMEOUT                         5         /* Symbols */
#define LORA_FIX_LENGTH_PAYLOAD_ON                  false
#define LORA_IQ_INVERSION_ON                        false
#define VERSION 									2

#elif (( USE_MODEM_LORA == 0 ) && ( USE_MODEM_FSK == 1 ))

#define FSK_FDEV                                    25000     /* Hz */
#define FSK_DATARATE                                50000     /* bps */
#define FSK_BANDWIDTH                               50000     /* Hz */
#define FSK_PREAMBLE_LENGTH                         5         /* Same for Tx and Rx */
#define FSK_FIX_LENGTH_PAYLOAD_ON                   false

#else
#error "Please define a modem in the compiler subghz_phy_app.h."
#endif /* USE_MODEM_LORA | USE_MODEM_FSK */

#define PAYLOAD_LEN                                 255

//Unique parameters of node
#define nodeID 										1
#define dataName 									1

//Node IDs
#define nodeID_1 										1
#define nodeID_2 										2
#define nodeID_3 										3

//Packet headers
#define BEACON_HEADER 1
#define DATA_PACKET_HEADER 2
#define ACK_HEADER 3
#define TIMESTAMP_HEADER 4

typedef struct {
  uint8_t header :TIMESTAMP_HEADER;
  uint8_t* encoded_vals_h;
  uint8_t* encoded_count_h;
  uint8_t h_length;
  uint8_t* encoded_vals_m;
  uint8_t* encoded_count_m;
  uint8_t m_length;
  uint8_t* encoded_vals_s;
  uint8_t* encoded_count_s;
  uint8_t s_length;
} timestamp_packet;


/* USER CODE BEGIN EC */
//Global flag, transmission of one data array complete
extern uint8_t array_tx_complete;

/* USER CODE END EC */

/* External variables --------------------------------------------------------*/
/* USER CODE BEGIN EV */

/* USER CODE END EV */

/* Exported macros -----------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
/**
  * @brief  Init Subghz Application
  */
void SubghzApp_Init(void);

/* USER CODE BEGIN EFP */

/*
 * Transmits compressed array of data. Used in main. Returns true if success
 */

bool Tx_Compressed(uint8_t* compressed_data, uint32_t compressed_size_bits, int8_t data_type, int8_t sf_i, int8_t cr_i, int8_t txp_i);

/*
 * Transmits compressed array of data. Used in main. Returns true if success
 */

bool Tx_Compressed_Timestamp(timestamp_packet* TimestampPkt);


/*
 * For receiving compressed data
 * Input array/s to store data
 */

void Rx_Compressed(uint8_t* compressed_data, uint16_t size);

/* USER CODE END EFP */

#ifdef __cplusplus
}
#endif

#endif /*__SUBGHZ_PHY_APP_H__*/
