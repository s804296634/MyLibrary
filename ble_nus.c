#include "nrf_error.h"
#include "ble_nus.h"
#include "nrf_delay.h"

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

static uint32_t Ble_Send_String(ble_nus_t * p_nus, uint8_t * p_string, uint16_t length)
{
    uint32_t err_code;
    //NRF_LOG_DEBUG("module send data to cellphone \r\n");
    //NRF_LOG_HEXDUMP_DEBUG(p_string , length);
    uint16_t length_point = length;
    do
    {		
          err_code = ble_nus_data_send(p_nus, p_string, &length_point,m_conn_handle);
          if ( (err_code != NRF_ERROR_INVALID_STATE) && (err_code != NRF_ERROR_BUSY) && (err_code != NRF_ERROR_RESOURCES))
          {
            APP_ERROR_CHECK(err_code);
          }
          if(err_code == NRF_ERROR_RESOURCES)
          {
            break;	
          }
                    
    } while (err_code == NRF_ERROR_BUSY);

    return err_code;
}

uint16_t send_data_to_smartphone(uint8_t *array_temp,uint16_t array_length)
{
    uint16_t strleng = 0;
    uint16_t send_cnt = 0;
    uint8_t  Nus_Send_Buf[20];
    uint32_t err_code;
              
    memset(Nus_Send_Buf,0,20);
    strleng = array_length;
  
   if(ble_hvx_notify_buffer_full == 0)
   {
      
      while(strleng>0)
      {
          NRF_LOG_INFO("strleng = %d\r\n",strleng);
          memset(Nus_Send_Buf,0,20);
          if(strleng > 20)
          {
              memcpy(Nus_Send_Buf , (array_temp + send_cnt) , 20);
              err_code = Ble_Send_String(&m_nus , Nus_Send_Buf ,20);              //BLE_ERROR_NO_TX_BUFFERS
              if((err_code != NRF_ERROR_INVALID_STATE) && (err_code != NRF_ERROR_RESOURCES))
              {
                APP_ERROR_CHECK(err_code);
              }
              //nrf_delay_ms(10);
              if(err_code == NRF_ERROR_RESOURCES)
              {
                ble_hvx_notify_buffer_full = 1;
                //NRF_LOG_INFO("strleng1r = %d\r\n",strleng);

               while(ble_hvx_notify_buffer_full);
               Ble_Send_String(&m_nus , Nus_Send_Buf ,20);
              //  return send_cnt;          
              }
              send_cnt = send_cnt + 20;
              strleng =  strleng - 20;
              //NRF_LOG_INFO("strleng1 = %d\r\n",strleng);
          }
          else
          {
              memcpy(Nus_Send_Buf , (array_temp + send_cnt) , strleng);
              err_code = Ble_Send_String(&m_nus , Nus_Send_Buf , strleng);        //BLE_ERROR_NO_TX_BUFFERS
              if((err_code != NRF_ERROR_INVALID_STATE) && (err_code != NRF_ERROR_RESOURCES))
              {
                APP_ERROR_CHECK(err_code);
              }
              if(err_code == NRF_ERROR_RESOURCES)
              {
                ble_hvx_notify_buffer_full = 1;
                //NRF_LOG_INFO("strleng2r = %d\r\n",strleng);
                  while(ble_hvx_notify_buffer_full);
                  Ble_Send_String(&m_nus , Nus_Send_Buf , strleng);  
               // return send_cnt;          
              }
              send_cnt += strleng;
              strleng =  0;
              //NRF_LOG_INFO("strleng2 = %d\r\n",strleng);
          }
      }
   }
   return send_cnt; 
}