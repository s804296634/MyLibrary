#include "uart_protocol.h"

#include "nrf_log.h"

static uint8_t get_http_url(void* data, uint16_t len);
static uint8_t get_http_port(void* data, uint16_t len);
static uint8_t get_mqtt_port(void* data, uint16_t len);
static uint8_t get_mqtt_topic(void* data, uint16_t len);
static uint8_t get_mqtt_url(void* data, uint16_t len);
static uint8_t get_mqtt_port(void* data, uint16_t len);
static uint8_t get_tcp_url(void* data, uint16_t len);
static uint8_t get_tcp_port(void* data, uint16_t len);

static uint8_t set_http_url(void* data, uint16_t len);
static uint8_t set_http_port(void* data, uint16_t len);
static uint8_t set_mqtt_topic(void* data, uint16_t len);
static uint8_t set_mqtt_url(void* data, uint16_t len);
static uint8_t set_mqtt_port(void* data, uint16_t len);
static uint8_t set_mqtt_port(void* data, uint16_t len);
static uint8_t set_tcp_url(void* data, uint16_t len);
static uint8_t set_tcp_port(void* data, uint16_t len);

#define SET_HTTP_URL   0x01
#define SET_HTTP_PORT  0x02
#define SET_MQTT_TOPIC   0x03
#define SET_MQTT_URL   0x03
#define SET_MQTT_PORT  0x04
#define SET_TCP_URL    0x05
#define SET_TCP_PORT   0x06

#define GET_HTTP_URL  0x07
#define GET_HTTP_PORT 0x08
#define GET_MQTT_TOPIC   0x03
#define GET_MQTT_URL  0x09
#define GET_MQTT_PORT 0x0a
#define GET_TCP_URL   0x0b 
#define GET_TCP_PORT  0x0c

static const uart_cmd_table_t m_cmd_table[SUPPORT_CMD_NUM] = 
{
{.cmd =SET_HTTP_URL,  .cb = set_http_url },
{.cmd =SET_HTTP_PORT,  .cb = set_http_port },
{.cmd =SET_MQTT_TOPIC,  .cb = set_mqtt_topic },
{.cmd =SET_MQTT_URL,  .cb = set_mqtt_url },
{.cmd =SET_MQTT_PORT,  .cb = set_mqtt_port },
{.cmd =SET_TCP_URL,  .cb = set_tcp_url },
{.cmd =SET_TCP_PORT,  .cb = set_tcp_port },

{.cmd =GET_HTTP_URL,  .cb = get_http_url },
{.cmd =GET_HTTP_PORT,  .cb = get_http_port },
{.cmd =GET_MQTT_TOPIC,  .cb = get_mqtt_topic },
{.cmd =GET_MQTT_URL,  .cb = get_mqtt_url },
{.cmd =GET_MQTT_PORT,  .cb = get_mqtt_port },
{.cmd =GET_TCP_URL,  .cb = get_tcp_url },
{.cmd =GET_TCP_PORT,  .cb = get_tcp_port }




};

static uint8_t get_http_url(void* data, uint16_t len)
{
    NRF_LOG_INFO(" get http post url"); 
}
static uint8_t get_http_port(void* data, uint16_t len)
{
    NRF_LOG_INFO(" get http port "); 
}
static uint8_t get_mqtt_topic(void* data, uint16_t len)
{

}
static uint8_t get_mqtt_url(void* data, uint16_t len)
{

}
static uint8_t get_mqtt_port(void* data, uint16_t len)
{

}
static uint8_t get_tcp_url(void* data, uint16_t len)
{

}
static uint8_t get_tcp_port(void* data, uint16_t len)
{

}

static uint8_t set_http_url(void* data, uint16_t len)
{

}
static uint8_t set_http_port(void* data, uint16_t len)
{

}
static uint8_t set_mqtt_topic(void* data, uint16_t len)
{

}
static uint8_t set_mqtt_url(void* data, uint16_t len)
{

}
static uint8_t set_mqtt_port(void* data, uint16_t len)
{

}
static uint8_t set_tcp_url(void* data, uint16_t len)
{

}
static uint8_t set_tcp_port(void* data, uint16_t len)
{

}
static uint16_t crc16_compute(uint8_t const * p_data, uint32_t size, uint16_t const * p_crc)
{
    uint16_t crc = (p_crc == NULL) ? 0xFFFF : *p_crc;

    for (uint32_t i = 0; i < size; i++)
    {
        crc  = (uint8_t)(crc >> 8) | (crc << 8);
        crc ^= p_data[i];
        crc ^= (uint8_t)(crc & 0xFF) >> 4;
        crc ^= (crc << 8) << 4;
        crc ^= ((crc & 0xFF) << 4) << 1;
    }

    return crc;
}
static bool crc_verify_success(uint16_t crc,uint16_t len,uint8_t const * p_data)
{
    uint16_t computed_crc;

    computed_crc = crc16_compute(p_data,POS_CRC,NULL);
    computed_crc = crc16_compute(p_data + POS_PAYLOAD,len -POS_PAYLOAD ,&computed_crc );
    return ( computed_crc == crc );

}

uint32_t handle_serial_data(uint8_t const * p_data , uint16_t len)
{
      uint32_t err_code = 0x00; 

      serial_data_t * p_serial = (serial_data_t * )p_data;

      NRF_LOG_HEXDUMP_INFO(p_data,len);
      NRF_LOG_HEXDUMP_INFO(p_serial,len);
      NRF_LOG_INFO("cmd : %d",p_serial->cmd);
      NRF_LOG_INFO("len : %d",p_serial->length);
      NRF_LOG_INFO("crc : %d",p_serial->crc_value);

      if( p_serial->length > MAX_PAYLOAD_LENGTH)
      {
        NRF_LOG_INFO("error ,out of length :%d", p_serial->length);         
        return 0x01;
      }
      if(p_data == NULL)
      {
        return 0x02;
      }


     
     #if 0
     if ( !crc_verify_success(p_serial->crc_value,p_serial->length,p_data) )
     {
        NRF_LOG_INFO("crc fail\n");
        return  0x00;
     }
     #endif

     for(uint8_t i = 0 ; i <= SUPPORT_CMD_NUM - 1 ; i++ )
     {
        if(m_cmd_table[i].cmd ==  p_serial->cmd &&  m_cmd_table[i].cb != NULL)
        {
            m_cmd_table[i].cb(p_serial->payload,p_serial->length);
        }
     }
     NRF_LOG_INFO("unsupported command \n");
     return 0x03;
}
