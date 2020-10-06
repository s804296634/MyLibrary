#include "nrf_error.h"
#include "ble_nus.h"

uint8_t ble_hvx_notify_buffer_full = 0;
static void nus_data_handler(ble_nus_evt_t * p_evt)
{   
   switch (p_evt->type)
   {
      case BLE_NUS_EVT_TX_RDY:
           ble_hvx_notify_buffer_full = 0;      
           //NRF_LOG_INFO("hvx notify buffer is empty \r\n");
      break;

      case BLE_NUS_EVT_COMM_STARTED:// enable cccd
           NRF_LOG_INFO("cccd enable \r\n");
      break;      

      case BLE_NUS_EVT_RX_DATA:
           NRF_LOG_INFO("Received data from Client \r\n");
           // NRF_LOG_HEXDUMP_INFO(p_evt->params.rx_data.p_data, p_evt->params.rx_data.length); 
           // analyse_client_command(p_evt->params.rx_data.p_data ,p_evt->params.rx_data.length);
      break;    
   }
}

static uint32_t ble_send_string(ble_nus_t * p_nus , uint8_t *p_string, uint16_t length)
{
	uint32_t err_code;

	uint16_t len = length;

	do
	{
		err_code = ble_nus_string_send(p_nus,p_string,&len);
		if( (err_code != NRF_ERROR_INVALID_STATE) && (err_code != NRF_ERROR_BUSY) && (err_code != NRF_ERROR_RESOURCES))
		{

			APP_ERROR_CHECK(err_code);
		}
		if(err_code == NRF_ERROR_RESOURCES)
        {
            break;	
        }


	}while（err_code == NRF_ERROR_BUSY）

   return err_code;
}

uint16_t send_data_to_smartphone(uint8_t * p_data,uint16_t data_length)
{
	uint16_t str_len = 0;
	uint16_t send_cnt = 0;

	uint8_t nus_send_buf[20] = {0};
	uint32_t err_code;

	str_len = data_length;

	if ( ble_hvx_notify_buffer_full == 0 )
	{
		while( str_len > 0)
		{
           memset(nus_send_buf,0,sizeof(nus_send_buf));
           if ( str_len > 20)
           {
           	   memcpy(nus_send_buf, p_data + send_cnt, 20);
           	   err_code = ble_send_string(&nus,nus_send_buf,str_len);
           	  if((err_code != NRF_ERROR_INVALID_STATE) && (err_code != NRF_ERROR_RESOURCES))
              {
                APP_ERROR_CHECK(err_code);
              }
              if(err_code == NRF_ERROR_RESOURCES)
              {
                ble_hvx_notify_buffer_full = 1;
                //NRF_LOG_INFO("strleng1r = %d\r\n",strleng);
                return send_cnt;          
              }

              send_cnt += 20;
              str_len  -= 20;
           }
           else
           {
           	 memcpy(nus_send_buf,p_data + send_cnt,str_len);
           	 err_code = ble_send_string(&nus,nus_send_buf,str_len);
           	 if((err_code != NRF_ERROR_INVALID_STATE) && (err_code != NRF_ERROR_RESOURCES))
              {
                APP_ERROR_CHECK(err_code);
              }
              if(err_code == NRF_ERROR_RESOURCES)
              {
                ble_hvx_notify_buffer_full = 1;
                //NRF_LOG_INFO("strleng1r = %d\r\n",strleng);
                return send_cnt;          
              }

              send_cnt += str_len;
              str_len   = 0;

           }
		}
	}
	
	return send_cnt;

}