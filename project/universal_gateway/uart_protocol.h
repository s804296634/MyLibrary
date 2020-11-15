#ifndef UART_PROTOCOL_H
#define UART_PROTOCOL_H

#include "stdint.h"
#include "stdbool.h"

#define MAX_PAYLOAD_LENGTH 255

#define POS_CMD         0
#define POS_LENGTH      1
#define POS_CRC         2 
#define POS_PAYLOAD     3

#define SUPPORT_CMD_NUM 14



#define SET_HTTP_URL   0x01
#define SET_HTTP_PORT  0x02
#define SET_MQTT_URL   0x03
#define SET_MQTT_PORT  0x04
#define SET_TCP_URL    0x05
#define SET_TCP_PORT   0x06

#define GET_HTTP_URL  0x07
#define GET_HTTP_PORT 0x08
#define GET_MQTT_URL  0x09
#define GET_MQTT_PORT 0x0a
#define GET_TCP_URL   0x0b 
#define GET_TCP_PORT  0x0c


/** notes : cancel byte alignment of struct */
typedef struct __attribute((packed))
{
  uint8_t  cmd;
  uint16_t length;
  uint16_t crc_value;
  uint8_t  payload[MAX_PAYLOAD_LENGTH];	
}serial_data_t;

typedef uint8_t (*cmd_callback_t)(void* data, uint16_t len);


typedef struct 
{
	uint8_t         cmd;
	cmd_callback_t  cb;
}uart_cmd_table_t;


uint32_t handle_serial_data(uint8_t const * p_data , uint16_t len);

#endif



