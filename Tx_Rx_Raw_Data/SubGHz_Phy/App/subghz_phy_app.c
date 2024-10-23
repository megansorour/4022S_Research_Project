/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    subghz_phy_app.c
  * @author  MCD Application Team
  * @brief   Application of the SubGHz_Phy Middleware
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
#include "platform.h"
#include "sys_app.h"
#include "subghz_phy_app.h"
#include "radio.h"

/* USER CODE BEGIN Includes */
#include "stm32_timer.h"
#include "stm32_seq.h"
#include "utilities_def.h"
#include "data.h"
#include "compression.h"
#include <stdbool.h>
#include <stdlib.h>
/* USER CODE END Includes */

/* External variables ---------------------------------------------------------*/
/* USER CODE BEGIN EV */

/* USER CODE END EV */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* Configurations */
/*Timeout*/
//#define RX_TIMEOUT_VALUE              4000
//#define TX_TIMEOUT_VALUE              3000
/* Definitions */
#define RX_CONTINUOUS_ON              1
#define RADIO_TX                      0 /* do not change*/
#define RADIO_RX                      1 /* do not change*/
#define PRBS9_INIT                    ( ( uint16_t) 2 )

#define SF_TEST 1
#define CR_TEST  2
#define TX_POWER_TEST  3

/* Test Configurations */
/* Application buffer 255 max when USE_MODEM_LORA */
#define MAX_APP_BUFFER_SIZE              255

//Packet structure for standard data array transfer
typedef __PACKED_STRUCT
{
  uint8_t header :DATA_PACKET_HEADER;
  uint16_t pkt_no;   /*Packet number*/
  uint8_t pkt_total; /*Total packets to be transmitted*/
  uint8_t data_type;         /*Type of data being transmitted ie x,y,z,r,q,p values*/
  uint8_t data[251]; /*Array containing actual compressed sensor data*/
} data_packet;

//Packet structure for beacon sent by transmitter
typedef __PACKED_STRUCT
{
  uint8_t header :BEACON_HEADER; /*Header for beacon*/
  uint8_t node_ID;   /*ID of the buoy/node sending (eg SB1, SB2...*/
  uint16_t pkt_total; /*Total packets to be transmitted, for all data*/
  uint16_t data_log_no;         /*Name of data being transmitted ie W1, W2*/
} beacon;

//Packet structure for ACK on data packet
typedef __PACKED_STRUCT
{
  uint8_t header :ACK_HEADER;
  uint8_t node_ID;   /*ID of the buoy/node sending the ACK (eg SB1, SB2...*/
  uint8_t recip_node_ID; /* ID of intended recipient of ACK*/
  uint16_t pkt_no; /*Packet number being acknowledged*/
  //uint16_t rssi;         /*Most recent RSSI*/
} pktACK;


/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* Radio events function pointer */
static RadioEvents_t RadioEvents;

/* USER CODE BEGIN PV */
static __IO uint32_t RadioTxDone_flag = 0;
static __IO uint32_t RadioTxTimeout_flag = 0;
static __IO uint32_t RadioRxDone_flag = 0;
static __IO uint32_t RadioRxTimeout_flag = 0;
static __IO uint32_t RadioError_flag = 0;
static __IO uint32_t ACK_Received_flag = 0;
static __IO int16_t last_rx_rssi = 0;
static __IO int8_t last_rx_cfo = 0;

//uint8_t data_buffer[MAX_APP_BUFFER_SIZE];
uint8_t data_buffer[MAX_APP_BUFFER_SIZE] UTIL_MEM_ALIGN(4);
data_packet dataPacket;
uint16_t data_offset = 0;

static __IO uint16_t payloadLen = PAYLOAD_LEN;
#if (TEST_MODE == RADIO_TX)
static uint16_t payloadLenMax = MAX_APP_BUFFER_SIZE;
#endif /* TEST_MODE == RADIO_TX */

// Experiment values

uint32_t delay_on_air; //delay varies with time on air

uint32_t count_RxOk = 0;
uint32_t count_RxKo = 0;
uint32_t PER = 0;

//count total packets
static int32_t packetCnt = 0; //general, all packets
static int32_t rx_pkt = 0; //counting expected packets in transmission
//flag to signal main that tx of a data array is complete
//uint8_t array_tx_complete = 0;

//Experiments
static uint32_t spreading_factor_values[]={7, 8, 10, 12}; //for TEST_1
static uint8_t coding_rate_values[]={1, 2, 3, 4}; //for TEST_2
//static uint8_t tx_power_values[] = {10, 3, 1, 5, 14};
static uint8_t tx_power_values[] = {10, 5, 3, 1, 14};
int8_t j = 0; //parameter value index for SF, keep track of what's been tested
int8_t k = 0; //parameter value index for CR, keep track of what's been tested
int8_t l = 0; //parameter value index for Tx output power, keep track of what's been tested


uint8_t test = SF_TEST;

//int8_t button_pressed = 0; // keeps track of if button has been pressed, to move on to next test

//Tx log
uint32_t tx_packets = 0;
uint32_t retx_packets = 0;

//Changing SF, change in maximum payload
uint32_t max_buffer;
uint32_t payload_var; //5 bytes of headers/other info
uint32_t RX_TIMEOUT_VALUE;
uint32_t TX_TIMEOUT_VALUE;
#define RX_TIMEOUT_BASE 4000  // base timeout
#define RX_TIMEOUT_SF_FACTOR 1500  // additional time per SF level above 7

uint32_t time_on_air;


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/*!
 * @brief Function to be executed on Radio Tx Done event
 */
static void OnTxDone(void);

/**
  * @brief Function to be executed on Radio Rx Done event
  * @param  payload ptr of buffer received
  * @param  size buffer size
  * @param  rssi
  * @param  LoraSnr_FskCfo
  */
static void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t LoraSnr_FskCfo);

/**
  * @brief Function executed on Radio Tx Timeout event
  */
static void OnTxTimeout(void);

/**
  * @brief Function executed on Radio Rx Timeout event
  */
static void OnRxTimeout(void);

/**
  * @brief Function executed on Radio Rx Error event
  */
static void OnRxError(void);

/* USER CODE BEGIN PFP */
/**
  * @brief Packet Error Rate state machine implementation
  */
static void Rx_Process(void);

static void Rx_Timestamp(uint8_t* timestamp_pkt, uint16_t size);

static void unpack_data(uint8_t *packed_count, uint16_t *unpacked_count, uint8_t length);

static void process_timestamp(uint8_t* data_rx, uint32_t* offset, uint8_t length);

static void sendACK(uint16_t pkt_no);

static bool sendBeacon(uint16_t data_log_no, uint16_t packet_total);

static uint32_t get_timeout_value();

static bool check_timeout(uint32_t timeout);


/* USER CODE END PFP */

/* Exported functions ---------------------------------------------------------*/
void SubghzApp_Init(void)
{
  /* USER CODE BEGIN SubghzApp_Init_1 */

  /* USER CODE END SubghzApp_Init_1 */

  /* Radio initialization */
  RadioEvents.TxDone = OnTxDone;
  RadioEvents.RxDone = OnRxDone;
  RadioEvents.TxTimeout = OnTxTimeout;
  RadioEvents.RxTimeout = OnRxTimeout;
  RadioEvents.RxError = OnRxError;

  Radio.Init(&RadioEvents);

  /* USER CODE BEGIN SubghzApp_Init_2 */
	  /* Radio Set frequency */
	  Radio.SetChannel(RF_FREQUENCY);

	  data_offset = 0;


	  if (spreading_factor_values[j] < 9) {
		  max_buffer = 255;
	  }
	  else if (spreading_factor_values[j] == 9) {
		  max_buffer = 115;
	  }
	  else {
		  max_buffer = 50;
	  }

	  payload_var =  max_buffer - 5; //5 bytes of headers/other info
	  RX_TIMEOUT_VALUE = RX_TIMEOUT_BASE + (RX_TIMEOUT_SF_FACTOR * (LORA_SPREADING_FACTOR - 7));
	  TX_TIMEOUT_VALUE = RX_TIMEOUT_VALUE - RX_TIMEOUT_SF_FACTOR;

	  Radio.SetRxConfig(MODEM_LORA, LORA_BANDWIDTH, spreading_factor_values[j],
						coding_rate_values[k], 0, LORA_PREAMBLE_LENGTH,
						LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
						0, true, 0, 0, LORA_IQ_INVERSION_ON, true);

	  Radio.SetMaxPayloadLength(MODEM_LORA, max_buffer);


	  Radio.SetTxConfig(MODEM_LORA, tx_power_values[l], 0, LORA_BANDWIDTH,
			  spreading_factor_values[j], coding_rate_values[k],
						LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
						true, 0, 0, LORA_IQ_INVERSION_ON, TX_TIMEOUT_VALUE);


	  time_on_air = Radio.TimeOnAir(1, LORA_BANDWIDTH, spreading_factor_values[j], coding_rate_values[k], 8, 0, max_buffer, true);
	  delay_on_air = time_on_air/2;


	  APP_PRINTF("\nNew test. SF: %d, CR: %d, Tx Power: %d, Toa: %d \r\n", spreading_factor_values[j], coding_rate_values[k], tx_power_values[l], time_on_air);
	  //Reset experiment vals
	  tx_packets = 0;
	  retx_packets = 0;
	  count_RxOk = 0;
	  count_RxKo = 0;
	  PER = 0;

#if (TEST_MODE == RADIO_RX)
	  if (j == 3) {
		  test = CR_TEST;
		  j = 0;
	  }
	  else if (k == 3) {
		  test = TX_POWER_TEST;
		  k = 0;
	  }
#endif

//#endif /* TEST_MODE */
#if (TEST_MODE == RADIO_RX)
//	  APP_PRINTF("Rx LORA Test\r\n");
	  Radio.Rx(RX_TIMEOUT_VALUE);
#endif
//  APP_PRINTF("Start data log: 0D 0A\r\n");
  /*register task to to be run in while(1) after Radio IT*/
  UTIL_SEQ_RegTask((1 << CFG_SEQ_Task_SubGHz_Phy_App_Process), UTIL_SEQ_RFU, Rx_Process);

  /* USER CODE END SubghzApp_Init_2 */
}

/* USER CODE BEGIN EF */
bool Tx_Compressed(uint8_t* compressed_data, uint32_t compressed_size_bits, int8_t data_type, int8_t sf_i, int8_t cr_i, int8_t txp_i){
	j = sf_i;
	k = cr_i;
	l = txp_i;
	SubghzApp_Init();
	APP_PRINTF("Sending data type %d", data_type);
	array_tx_complete = 0;
	//Create packet containing compressed data
	//int i = 0; //index for offset of data buffer

	uint32_t total_bytes = compressed_size_bits/8;
	APP_PRINTF("\r\nTotal bytes: %d\r\n", total_bytes);
	data_buffer[0] =  DATA_PACKET_HEADER; //Set first byte as data packet header
	data_buffer[1] = nodeID_1;
	if (total_bytes > payload_var) { //data cannot fit into one packet of 255bytes
		uint32_t total_packets = total_bytes/payload_var + 1;
		uint32_t byte_count = 0; //counting how many bytes have been sent
		int k = 0; //index for compressed data, track what's been sent

		for (int32_t j = 0; j < total_packets; j++){
			ACK_Received_flag = 0;
			data_buffer[2] = j; //1st byte: Packet no.
			data_buffer[3] = total_packets; //2nd byte: Total packets to be transmitted
			data_buffer[4] = data_type; //3rd byte: Sensor ID (ie x, y, x, r, q, p)

			if (total_bytes - k >= payload_var) {
				memcpy(&data_buffer[5], &compressed_data[k], payload_var);	//Rest of packet: Compressed sensor data
				k = k + payload_var;
				Radio.Send(data_buffer, payload_var+5);
				tx_packets++;
				while (RadioTxDone_flag != 1){
					HAL_Delay(5);
				}
				//Radio.Standby();
				APP_PRINTF("Tx no. %d sent\n\r", j);
				RadioTxDone_flag = 0;

//#if (PRINT_ALL == 1)
#if (0)
				//Printing out buffer:
				APP_PRINTF("Data buffer Tx no. %d: \n\r", j);
				for (int32_t i = 0; i < payload_var+5; i++)
				{
				  APP_PRINTF("%d ", data_buffer[i]);
				  if ((i % 16) == 15){
					APP_PRINTF("\n");}
				  HAL_Delay(10);}
				APP_PRINTF("\n\r");
#endif
				//wait for ACK
				Radio.Rx(RX_TIMEOUT_VALUE); //Wait to receive ACK
//				APP_PRINTF("Waiting for ACK.\r\n");
				uint32_t timeout = get_timeout_value();
				uint32_t num_retrans = 0; //retransmissions for this packet
				while(!ACK_Received_flag) {
					//APP_PRINTF("Waiting for ACK.");
					//if timeout, resend then rx again
					if (check_timeout(timeout)) {
						if (num_retrans > 4) {
							APP_PRINTF("Reached retransmission limit. Ending test.\r\n")
							return false;
						}
						APP_PRINTF("Rx Timeout. Sending packet %d again.\r\n", j);
						// Timeout reached, resend the packet
						Radio.Send(data_buffer, payload_var+5);
						tx_packets++;
						retx_packets++;
						num_retrans++;
						HAL_Delay(delay_on_air);
						Radio.Rx(RX_TIMEOUT_VALUE);  // Switch to Rx mode again after resending
						timeout = get_timeout_value();  // Reset timeout
					}
				}
				APP_PRINTF("ACK no. %d received. ");
				HAL_Delay(50);
				HAL_Delay(delay_on_air);
				APP_PRINTF("Next packet.\r\n", j);

			}
			//last packet of transmission
			else {
				uint32_t remaining_bytes = total_bytes - k;
				//Rest of packet
				memcpy(&data_buffer[5], &compressed_data[k], remaining_bytes);
				k = k + remaining_bytes;
				Radio.Send(data_buffer, (remaining_bytes+5));
				HAL_Delay(100); 
				while (RadioTxDone_flag != 1){
					HAL_Delay(1);
				}
				//Radio.Standby();
				APP_PRINTF("Tx no. %d sent. Final packet.\n\r", j);
				RadioTxDone_flag = 0;
				//wait for ACK
				Radio.Rx(RX_TIMEOUT_VALUE); //Wait to receive ACK
//				APP_PRINTF("Waiting for ACK.\r\n");
				uint32_t timeout = get_timeout_value();
				while(!ACK_Received_flag) {
					//APP_PRINTF("Waiting for ACK.");
					//if timeout, resend then rx again
					if (check_timeout(timeout)) {
						APP_PRINTF("Rx Timeout. Sending packet %d again.\r\n", j);
						// Timeout reached, resend the packet
						Radio.Send(data_buffer, (remaining_bytes+5));
						HAL_Delay(delay_on_air);
						Radio.Rx(RX_TIMEOUT_VALUE);  // Switch to Rx mode again after resending
						timeout = get_timeout_value();  // Reset the timeout
					}
				}
				APP_PRINTF("ACK no. %d received. Finished transmission.\r\n", j);
				APP_PRINTF("Total packets transmitted: %d, Retransmissions: %d\r\n", tx_packets, retx_packets);
				APP_PRINTF("Total ToA: %d ms \r\n", tx_packets*time_on_air);
				HAL_Delay(delay_on_air*2);
				sendBeacon(0, 0);
				HAL_Delay(delay_on_air*2);
				array_tx_complete = 1;

				//Printing out buffer:
//#if PRINT_ALL == 1
#if (0)
				APP_PRINTF("Data buffer Tx no. %d: \n\r", j);
				for (int32_t i = 0; i < remaining_bytes+5; i++)
				{
				  APP_PRINTF("%d ", data_buffer[i]);
				  if ((i % 16) == 15)
				  {
					APP_PRINTF("\n");
				  }
				  HAL_Delay(10);
				}
				APP_PRINTF("\n\r");
#endif
			}
			//HAL_Delay(3000); //for testing


			  RadioRxDone_flag = 0;
			  RadioRxTimeout_flag = 0;
			  RadioError_flag = 0;
			  RadioTxDone_flag = 0;
			  RadioTxTimeout_flag = 0;
		}
	}


	else { //if data less than 250 bytes, can transmit it all in one packet
		data_buffer[2] = 1; //only packet, 	//1st byte: Packet no.
		data_buffer[3] = 1; //2nd byte: Total packets to be transmitted
		data_buffer[4] = data_type;	//3rd byte: Sensor ID (ie x, y, x, r, q, p)
		memcpy(&data_buffer[5], compressed_data, total_bytes); 	//Rest of packet: Compressed sensor data

#if PRINT_ALL == 1
		//Printing out buffer
		APP_PRINTF("Data buffer Tx: \n\r");
		for (int32_t i = 0; i < total_bytes; i++)
		{
		  APP_PRINTF("%d ", data_buffer[i]);
		  if ((i % 16) == 15)
		  {
			APP_PRINTF("\n");
		  }
		  HAL_Delay(10);
		}
		APP_PRINTF("\n\r");
#endif
		//wait for ACK

		//memcpy(data_buffer, dataPacket, 255);
//		Radio.Send((uint8_t *)&dataPacket, (3+total_bytes));
		Radio.Send(data_buffer, (5+total_bytes)); //Real
	}
	return true;
}

void Rx_Unpack(uint8_t* compressed_data, uint16_t size) {
//Packet stored in data_rx
	//uint32_t i = 0; //index to track position in packet
	//UTIL_SEQ_ClrEvt(CFG_SEQ_Task_SubGHz_Phy_App_Process);
//	Radio.Standby();
	uint8_t sender_ID = data_rx[1];
	uint8_t packet_no = data_rx[2];
	uint8_t total_packets = data_rx[3] - 1;
	uint8_t data_type = data_rx[4];
//	APP_PRINTF("Packet no: %u, Rx expected packet: %d\r\n", packet_no, rx_pkt);
	APP_PRINTF("%d,%d,%d,%u,%d\r\n", last_rx_rssi, last_rx_cfo, size, packet_no, rx_pkt);
	if (packet_no != rx_pkt) { //Already received this packet
		sendACK((uint16_t)packet_no); //resend ACK to sender
		while (RadioTxDone_flag != 1){
				HAL_Delay(5);
			}
		return;
	}
	else if (packet_no > rx_pkt) { //Incorrect packet
		APP_PRINTF("Missing packet %d\r\n", rx_pkt);
	}
	//Establish what type of data it is
#if (0)
	if (packet_no == 0) {
#if (DEBUG_PRINT == 1)
		APP_PRINTF("Total packets: %d\r\n", total_packets);
#endif

		switch (data_type){
		case (TYPE_X):
			APP_PRINTF("x_vals =\r\n");
			HAL_Delay(10);
			break;
		case (TYPE_Y):
			APP_PRINTF("y_vals =\r\n");
			HAL_Delay(10);
			break;
		case (TYPE_Z):
			APP_PRINTF("z_vals =\r\n");
			HAL_Delay(10);
			break;
		case (TYPE_R):
			APP_PRINTF("r_vals =\r\n");
			HAL_Delay(10);
			break;
		case (TYPE_Q):
			APP_PRINTF("q_vals =\r\n");
			HAL_Delay(10);
			break;
		case (TYPE_P):
			APP_PRINTF("p_vals =\r\n");
			HAL_Delay(10);
			break;
		}
	}
#endif
	memcpy(compressed_data, &data_rx[5], size-5);

	int data_size = size - 5;
	//Instead of writing to file, print to usart
	for (int32_t i = 0; i < data_size; i += 16) {
	    // Print out in chunks of 16, or less if the remaining data is smaller than 16
	    if ((i + 16) <= data_size) {
	        APP_PRINTF("0AQ %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
	            compressed_data[i], compressed_data[i+1], compressed_data[i+2], compressed_data[i+3],
	            compressed_data[i+4], compressed_data[i+5], compressed_data[i+6], compressed_data[i+7],
	            compressed_data[i+8], compressed_data[i+9], compressed_data[i+10], compressed_data[i+11],
	            compressed_data[i+12], compressed_data[i+13], compressed_data[i+14], compressed_data[i+15]);
	    } else {
	        // print remaining elements that are less than 16
	    	APP_PRINTF("0AQ ")
	        for (int j = i; j < data_size; j++) {
	            APP_PRINTF("%d ", compressed_data[j]);
	        }
//	        if ((data_size) < max_buffer-20) {
//	        	HAL_Delay(100); //add delay for very small packet
//	        	HAL_Delay(delay_on_air/2);
//	        }
	        APP_PRINTF("\n");  // Newline after the last set
	    }
	    HAL_Delay(5);
	}
	APP_PRINTF("\n");
	HAL_Delay(5);
//	HAL_Delay(delay_on_air); //delay added for Tx to print buffer
	rx_pkt++; //increment packet count for this transmission
	sendACK((uint16_t)rx_pkt); //send ACK to sender
	while (RadioTxDone_flag != 1){
		HAL_Delay(5);
	}
#if (DEBUG_PRINT == 1)
	APP_PRINTF("Ack no. %d sent.\r\n", packet_no);
#endif
	if (packet_no == total_packets) { //less than 250 bytes in packet, aka final packet
#if (DEBUG_PRINT == 1)
		APP_PRINTF("End of data array.\r\n\n");
#endif
		rx_pkt = 0; //reset packet count for transmission
	}
}

bool Tx_Compressed_Timestamp(timestamp_packet* TimestampPkt) {
	uint32_t data_length = (TimestampPkt->h_length)*3 + (TimestampPkt->m_length)*3 + (TimestampPkt->s_length)*3 + 6;
	//Create data buffer packet
	//if (data_length <= 255) { will always be less than 255? Bc wave event 30min
		uint16_t offset = 0;
		data_buffer[0] = TimestampPkt->header;
		data_buffer[1] = 0;
		data_buffer[2] = 1;
		data_buffer[3] = TimestampPkt->h_length; //must x3 on rx side
		offset = 4;
		memcpy(&data_buffer[offset], TimestampPkt->encoded_vals_h, TimestampPkt->h_length);
		offset = offset + TimestampPkt->h_length;
		memcpy(&data_buffer[offset], TimestampPkt->encoded_count_h, TimestampPkt->h_length*2);
		offset = offset + TimestampPkt->h_length*2;

		data_buffer[offset] = TimestampPkt->m_length;
		offset++;
		memcpy(&data_buffer[offset], TimestampPkt->encoded_vals_m, TimestampPkt->m_length);
		offset = offset + TimestampPkt->m_length;
		memcpy(&data_buffer[offset], TimestampPkt->encoded_count_m, TimestampPkt->m_length*2);
		offset = offset + TimestampPkt->m_length*2;

		data_buffer[offset] = TimestampPkt->s_length;
		offset++;
		memcpy(&data_buffer[offset], TimestampPkt->encoded_vals_s, TimestampPkt->s_length);
		offset = offset + TimestampPkt->s_length;
		memcpy(&data_buffer[offset], TimestampPkt->encoded_count_s, TimestampPkt->s_length*2);
		offset = offset + TimestampPkt->s_length*2;

//		//Print out data buffer
//		for (uint32_t i = 0; i < data_length; i++) {
//			APP_PRINTF("Data buffer[%d]: %u\r\n", i, data_buffer[i]);
//			HAL_Delay(10);
//		}
		//Send timestamp packet
		Radio.Send(data_buffer, data_length);
		HAL_Delay(delay_on_air);
		while (RadioTxDone_flag != 1){
			HAL_Delay(1);
		}
		//Radio.Standby();
		APP_PRINTF("Tx timestamp sent.\n\r");
		RadioTxDone_flag = 0;
		//wait for ACK
		Radio.Rx(RX_TIMEOUT_VALUE); //Wait to receive ACK
		APP_PRINTF("Waiting for ACK.\r\n");
		uint32_t timeout = get_timeout_value();
		while(!ACK_Received_flag) {
		//APP_PRINTF("Waiting for ACK.");
		//if timeout, resend then rx again
		if (check_timeout(timeout)) {
			APP_PRINTF("Rx Timeout. Sending timestamp packet again.\r\n");
			// Timeout reached, resend the packet
			Radio.Send(data_buffer, data_length);
			HAL_Delay(delay_on_air);
			Radio.Rx(RX_TIMEOUT_VALUE);  // Switch to Rx mode again after resending
			timeout = get_timeout_value();  // Reset the timeout
			}
		}
		APP_TPRINTF("ACK timestamp received.\r\n");

		return true;
}

/* USER CODE END EF */

/* Private functions ---------------------------------------------------------*/
static void OnTxDone(void)
{
  /* USER CODE BEGIN OnTxDone */
  RadioTxDone_flag = 1;
#if (DEBUG_PRINT == 1)
  APP_PRINTF("OnTxDone\r\n");
#endif

//#if (TEST_MODE == RADIO_RX)
//  APP_PRINTF("Rx says TxDone\r\n");
//#endif
  //UTIL_SEQ_SetTask((1 << CFG_SEQ_Task_SubGHz_Phy_App_Process), CFG_SEQ_Prio_0);
  /* USER CODE END OnTxDone */
}

static void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t LoraSnr_FskCfo)
{
  /* USER CODE BEGIN OnRxDone */
  last_rx_rssi = rssi;
  last_rx_cfo = LoraSnr_FskCfo;
  memcpy(data_rx, payload, size); //data_rx uint8, payload uint8
  /* Set Rxdone flag */
  RadioRxDone_flag = 1;
  payloadLen = size;
#if (TEST_MODE == RADIO_TX)
  //cast payload to correct structure
  pktACK* ACK = (pktACK*)data_rx;
  if (ACK->header ==  ACK_HEADER){
	  ACK_Received_flag = 1;
  }
#endif
#if (TEST_MODE == RADIO_RX)
  UTIL_SEQ_SetTask((1 << CFG_SEQ_Task_SubGHz_Phy_App_Process), CFG_SEQ_Prio_0);
  //Rx_Process();
#endif

  /* USER CODE END OnRxDone */
}

static void OnTxTimeout(void)
{
  /* USER CODE BEGIN OnTxTimeout */
   RadioTxTimeout_flag = 1;
   APP_PRINTF("TxTimeout\r\n");
  /* Run process in background*/
  //UTIL_SEQ_SetTask((1 << CFG_SEQ_Task_SubGHz_Phy_App_Process), CFG_SEQ_Prio_0);
  /* USER CODE END OnTxTimeout */
}

static void OnRxTimeout(void)
{
  /* USER CODE BEGIN OnRxTimeout */
  RadioRxTimeout_flag = 1;
  /* Run Per process in background*/
	#if (TEST_MODE == RADIO_TX)
	  APP_PRINTF("Tx says RxTimeout\r\n");
#elif (TEST_MODE == RADIO_RX)
  UTIL_SEQ_SetTask((1 << CFG_SEQ_Task_SubGHz_Phy_App_Process), CFG_SEQ_Prio_0);
	  //Rx_Process();
#endif
  /* USER CODE END OnRxTimeout */
}

static void OnRxError(void)
{
  /* USER CODE BEGIN OnRxError */
  RadioError_flag = 1;
  /* Run Per process in background*/
  UTIL_SEQ_SetTask((1 << CFG_SEQ_Task_SubGHz_Phy_App_Process), CFG_SEQ_Prio_0);
  //Rx_Process();
  /* USER CODE END OnRxError */
}

/* USER CODE BEGIN PrFD */
//State entered after beacon received, waiting for packets
static void Rx_Process(void){
	//APP_PRINTF("Rx Running");
	  data_offset = 0;
	  #if (TEST_MODE == RADIO_RX)
//#if (DEBUG_PRINT == 1)
	  APP_PRINTF("Rx%d,", packetCnt);
//#endif

	  if (RadioRxDone_flag == 1) {
#if (DEBUG_PRINT == 1)
		  APP_PRINTF("Size of received packet: %d\r\n", payloadLen);
#endif
		  APP_PRINTF("OnRxDone,");
		//Radio.Standby();
		//Read received packet header, go to corresponding function
		//APP_PRINTF("Data_Rx header: %d\r\n", data_rx[0]);
		switch (data_rx[0]) {
		case BEACON_HEADER:
//			APP_PRINTF("j: %d, k: %d, l: %d \r\n", j, k, l);
			//Update any parameters?
			//Setting pipeline based on buoy ID, data name?
			//Set to a receiving data state
			//For experiment: increment to next testing values
			if (test == SF_TEST) {
				j++;
				SubghzApp_Init();
			}
			else if (test == CR_TEST) {
				k++;
				SubghzApp_Init();
			}
			else if (test == TX_POWER_TEST) {
				l++;
				SubghzApp_Init();
			}

			break;

		case DATA_PACKET_HEADER:
			Rx_Unpack(data_rx, payloadLen); //Unpack received data
			break;
//		case ACK_HEADER:
		case TIMESTAMP_HEADER:
			Rx_Timestamp(data_rx, payloadLen); //Unpack received timestamp packet
			break;
//
		default:
#if (DEBUG_PRINT == 1)
			APP_PRINTF("Unknown packet received.\r\n");
			break;
#endif

		}
		int16_t rssi = last_rx_rssi;
		int8_t cfo = last_rx_cfo;
//		HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_SET); /* LED_GREEN */
		//APP_PRINTF("OnRxDone,");
		//APP_PRINTF("%d,%d,%d,", rssi, cfo, payloadLen);
	#if 0 //can remove if, to print received packet
		// data_rx printing
		APP_PRINTF("data_rx=0x \n\r");
		for (int32_t i = 0; i < size; i++)
		{
		  APP_PRINTF("%02X ", data_rx[i]);
		  if ((i % 16) == 15)
		  {
			APP_PRINTF("\n\r");
		  }
		  HAL_Delay(10);
		}
		APP_PRINTF("\n\r");
#endif /* 0 */
	  }
	  else if (RadioRxTimeout_flag == 1)
	  {
		APP_PRINTF("OnRxTimeout,NA,NA,NA,NA,NA\r\n");
	  }
	  else if (RadioError_flag == 1)
	  {
		APP_PRINTF("OnRxError,NA,NA,NA,NA,NA\r\n");
	  }
	  /*check flag*/
	  if ((RadioRxTimeout_flag == 1) || (RadioError_flag == 1))
	  {
		count_RxKo++;
	  }
	  if (RadioRxDone_flag == 1)
	  {
		count_RxOk++;
	  }

#endif
	  /* Reset timeout flag */
	  RadioRxDone_flag = 0;
	  RadioRxTimeout_flag = 0;
	  RadioError_flag = 0;
	  RadioTxDone_flag = 0;
	  RadioTxTimeout_flag = 0;

//	  HAL_Delay(100);

	  Radio.Rx(RX_TIMEOUT_VALUE);

	  /* This delay is only to give enough time to allow DMA to empty APP_PRINTF queue*/
	  //HAL_Delay(500);

	  RadioError_flag = 0;
	  packetCnt++;
	  PER = (100 * (count_RxKo)) / (count_RxKo + count_RxOk);
	  APP_PRINTF("%d\r\n", PER);
	  APP_PRINTF("\r\n");

}

void unpack_data(uint8_t *packed_count, uint16_t *unpacked_count, uint8_t length) {
    for (size_t i = 0; i < length; i++) {
        // Unpack two 8-bit values into one 16-bit value
        unpacked_count[i] = (packed_count[2 * i] << 8) | packed_count[2 * i + 1];
//Debugging statements
//        APP_PRINTF("Packed count i(%d): %u, Packed count i+1(%d): %u\r\n", 2*i, packed_count[2*i], 2*i+1, packed_count[2*i+1]);
//        HAL_Delay(10);
//        APP_PRINTF("Unpacked Value %d: %u\r\n", i, unpacked_count[i]);
//        HAL_Delay(10);
    }
}

void process_timestamp(uint8_t* data_rx, uint32_t* offset, uint8_t length) {
	//Allocating memory for data arrays
	uint8_t* compressed_data_vals = (uint8_t *) malloc(length * sizeof(uint8_t));
	uint8_t* compressed_data_count_8 = (uint8_t *) malloc(length *2* sizeof(uint8_t));
	uint16_t* compressed_data_count = (uint16_t *) malloc((length) * sizeof(uint16_t));

//	APP_PRINTF("Length of data count 16 = %d \r\n", sizeof(compressed_data_count));
//	APP_PRINTF("Length of data count 8 = %d \r\n", sizeof(compressed_data_count_8));
//	APP_PRINTF("Length = %d \r\n", length);

	memcpy(compressed_data_vals, &data_rx[*offset], length);
	*offset += length;

	memcpy(compressed_data_count_8, &data_rx[*offset], length*2);
	*offset += length*2;

//	for (size_t i = 0; i < length*2; i++) {
//		APP_PRINTF("8 bit Count %d: %u\r\n", i, compressed_data_count_8[i]);
//		HAL_Delay(20);
//	}
	HAL_Delay(delay_on_air);
	//Pack 8-bit count array into 16-bit
	unpack_data(compressed_data_count_8, compressed_data_count, length);
	for (size_t i = 0; i < length; i++) {
		APP_PRINTF("Val %d: %u, Count: %u\r\n", i, compressed_data_vals[i], compressed_data_count[i]);
		HAL_Delay(20);
	}

	free(compressed_data_vals);
	free(compressed_data_count_8);
	free(compressed_data_count);
}

void Rx_Timestamp(uint8_t* data_rx, uint16_t size) {
	uint32_t offset = 0;
//	uint8_t* compressed_data_vals; //Array to hold compressed data values as it is unpacked
//	uint8_t* compressed_data_count_8; //Array to hold compressed data counts as it is unpacked
//	uint16_t* compressed_data_count;
//	uint8_t packet_no = data_rx[1];
//	uint8_t total_packets = data_rx[2] - 1;
	uint8_t length_h = data_rx[3];
	offset = 4;

	APP_PRINTF("H:\r\n");
	process_timestamp(data_rx, &offset, length_h);
	APP_PRINTF("\r\n");
	HAL_Delay(10);

	uint8_t length_m = data_rx[offset];

	offset++;
	HAL_Delay(10);

	HAL_Delay(10);
	APP_PRINTF("M:\r\n");
	process_timestamp(data_rx, &offset, length_m);
	APP_PRINTF("\r\n");
	HAL_Delay(10);

	HAL_Delay(10);
	uint8_t length_s = data_rx[offset];
	offset++;
	APP_PRINTF("S:\r\n");
	process_timestamp(data_rx, &offset, length_s);
	HAL_Delay(10);
	APP_PRINTF("\r\n");
	HAL_Delay(10);
	sendACK(0);
	while (RadioTxDone_flag != 1){
		HAL_Delay(5);
	}
}


//#if (TEST_MODE == RADIO_TX)
bool sendBeacon(uint16_t data_log_no, uint16_t packet_total) {
//	beacon Beacon; //instantiate beacon packet
//	Beacon.node_ID = nodeID_1;
//	Beacon.data_log_no = dataName;
//	Beacon.pkt_total = packet_total;
//	Radio.Send((uint8_t *)&Beacon, 4);
	data_buffer[0] = BEACON_HEADER;
	Radio.Send(data_buffer, 4);
	APP_PRINTF("Beacon sent.\r\n");

	return true;
}

uint32_t get_timeout_value() {
    // Return the current time plus a timeout duration (e.g., 5000 ms = 5 seconds)
    return HAL_GetTick() + RX_TIMEOUT_VALUE;  // Timeout duration of 5 seconds
}

bool check_timeout(uint32_t timeout) {
    // Compare the current time with the timeout value
    return HAL_GetTick() > timeout;
}


//#elif (TEST_MODE == RADIO_RX)

void sendACK(uint16_t pkt_no) {
	pktACK ACK; //Instantiate new ACK packet
	ACK.header = ACK_HEADER;
	ACK.node_ID = nodeID_2; //ID of this node
	ACK.recip_node_ID = nodeID_1; //ID of intended recipient (ie Tx Node)
	ACK.pkt_no = pkt_no;
	//ACK.rssi = rssi;
	Radio.Send((uint8_t *)&ACK, sizeof(pktACK)); //Send ACK packet
	HAL_Delay(delay_on_air/2);
//	APP_PRINTF("Sent ACK %d\r\n", pkt_no-1);

}



//#endif
/* USER CODE END PrFD */
