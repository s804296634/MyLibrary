

/*
notes :
        1. 以下函数实现了一个 nus 服务的数据接收 回传给 手机的功能。

        2. 采用了双缓冲的机制，主要用于 spi iic 等各种传感器数据的采集， 
    
        3. switch (useing_which_resource_data_buffer) 在 spi 或者 iic 的数据事件中断里使用。

        4. 数据的发送在 主循环中，实际项目中，可以 通过标志位同步的方式 确定是否 发送数据给手机。

  双缓冲区的优点:

         1. 数据自动在 spi 、 iic 的中断里完成 存储。
         2. 设计机制保障了 发送数据时操作的缓冲区是一个 和 后台存储数据操作的缓冲区是另一个 


           缺点：

         1. 占用的 RAM 较大。

  使用须知：

         1. 根据实际需要 调整 buffer 结构 BUFFER_UNITS_MAX_AMOUNT、COORDINATE_UNIT_LENGTH 的大小
         2.  combine_coordinate_units_to_ble_pack() 组包函数 根据实际需要更改。


*/










uint8_t ble_hvx_notify_buffer_full = 0;
                                            

static uint16_t   m_conn_handle          = BLE_CONN_HANDLE_INVALID;                 /**< Handle of the current connection. */




#define USEING_BUFFER1                         1        //macro for buffer exchange
#define USEING_BUFFER2                         2        //macro for buffer exchange

#define BUFFER_PROCESS_COMBINE_PACK            1        //macro for buffer handle process
#define BUFFER_PROCESS_BLE_TRANSMIT            2        //macro for buffer handle process

#define SPIS_RECEIVED_UNIT_LENGTH              12
       
#define BUFFER_UNITS_MAX_AMOUNT                600
#define MAX_UNITS_AMOUNT_A_FRAME 8

#define SPIS_RECEIVED_UNIT_LENGTH              12
#define COORDINATE_UNIT_LENGTH                 8       //(1byte status) + (2bytes x) + (2bytes y) + (2bytes z)         
#define BUFFER_UNITS_MAX_AMOUNT                600

typedef struct 
{ 
  uint8_t data[COORDINATE_UNIT_LENGTH];   
}coordinate_unit_type_t;

typedef struct 
{ 
  coordinate_unit_type_t units[BUFFER_UNITS_MAX_AMOUNT];
  uint32_t               units_amount;
}coordinate_info_type_t;


typedef struct 
{ 
  uint8_t   buffer1_start_send;
  uint8_t   buffer2_start_send;

  uint16_t  buffer1_out_length;
  uint16_t  buffer2_out_length;

  uint8_t   buffer1_send_all;
  uint8_t   buffer2_send_all;

//  uint8_t   buffer1_store_all;
//  uint8_t   buffer2_store_all;

  uint8_t   buffer1_handle_process;
  uint8_t   buffer2_handle_process;
}coordinate_statue_type_t;

typedef struct //struct to save information 
{ 
  uint8_t data[4096 + 1024];
  uint16_t data_length;
}array_type_t;


coordinate_info_type_t coordinate_info_buffer1;
coordinate_info_type_t coordinate_info_buffer2;
coordinate_statue_type_t coordinate_statue;

uint8_t useing_which_resource_data_buffer = USEING_BUFFER1;








/**@brief Function for handling the data from the Nordic UART Service.
 *
 * @details This function will process the data received from the Nordic UART BLE Service and send
 *          it to the UART module.
 *
 * @param[in] p_evt       Nordic UART Service event.
 */
/**@snippet [Handling the data received over BLE] */
static void nus_data_handler(ble_nus_evt_t * p_evt)
{

    if (p_evt->type == BLE_NUS_EVT_RX_DATA)
    {
        uint32_t err_code;

       // NRF_LOG_DEBUG("Received data from BLE NUS. Writing data on UART.");
       // NRF_LOG_HEXDUMP_INFO(p_evt->params.rx_data.p_data, p_evt->params.rx_data.length);



//        for (uint32_t i = 0; i < p_evt->params.rx_data.length; i++)
//        {
//            do
//            {
//                err_code = app_uart_put(p_evt->params.rx_data.p_data[i]);
//                if ((err_code != NRF_SUCCESS) && (err_code != NRF_ERROR_BUSY))
//                {
//                    NRF_LOG_ERROR("Failed receiving NUS message. Error 0x%x. ", err_code);
//                    APP_ERROR_CHECK(err_code);
//                }
//            } while (err_code == NRF_ERROR_BUSY);
//        }
//        if (p_evt->params.rx_data.p_data[p_evt->params.rx_data.length - 1] == '\r')
//        {
//            while (app_uart_put('\n') == NRF_ERROR_BUSY);
//        }
               switch (useing_which_resource_data_buffer)//match which buffer be available
              {
               case USEING_BUFFER1:
                 NRF_LOG_INFO(" using buffer1");
                 memcpy(coordinate_info_buffer1.units[coordinate_info_buffer1.units_amount].data , p_evt->params.rx_data.p_data , p_evt->params.rx_data.length);
                 coordinate_info_buffer1.units_amount++;                  
                 if(coordinate_info_buffer1.units_amount >= BUFFER_UNITS_MAX_AMOUNT)//limit amount increase constantly
                 {
                   coordinate_info_buffer1.units_amount = 0;
                   NRF_LOG_INFO("1overflow \r\n");
                 }
                 if(coordinate_statue.buffer2_send_all)//always check another buffer if available ,if true ,replace current buffer that store coordinate info  
                 {
                  coordinate_statue.buffer2_send_all =0;
                  coordinate_statue.buffer1_start_send =1;//start to send buffer1 data
                  //coordinate_statue.buffer1_store_all =0;
                  useing_which_resource_data_buffer = USEING_BUFFER2;                  
                 }                
               break;

               case USEING_BUFFER2:
                    NRF_LOG_INFO(" using buffer2");
                 memcpy(coordinate_info_buffer2.units[coordinate_info_buffer2.units_amount].data , p_evt->params.rx_data.p_data , p_evt->params.rx_data.length);
                 coordinate_info_buffer2.units_amount++;
                 if(coordinate_info_buffer2.units_amount >= BUFFER_UNITS_MAX_AMOUNT)//limit amount increase constantly
                 {
                   coordinate_info_buffer2.units_amount = 0;
                   NRF_LOG_INFO("2overflow \r\n");
                 }
                 if(coordinate_statue.buffer1_send_all)//always check another buffer if available ,if true ,replace current buffer that store coordinate info
                 {                   
                  coordinate_statue.buffer1_send_all =0;
                  coordinate_statue.buffer2_start_send =1; //start to send buffer2 data
                  //coordinate_statue.buffer2_store_all =0;
                  useing_which_resource_data_buffer = USEING_BUFFER1;                  
                 }
               break;

               default :
                  NRF_LOG_INFO("switch error3 \r\n");
               break;
              }

    }

      switch(p_evt->type)
    {
        case BLE_NUS_EVT_COMM_STARTED:
                      
                      send = 1;
        break;
         case BLE_NUS_EVT_COMM_STOPPED:

                send = 0;
          break;

         case BLE_NUS_EVT_TX_RDY:

            ble_hvx_notify_buffer_full = 0;

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
#include "nrf_delay.h"
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


static void coordinate_buffer_parameter_init(void)//assignment initial value to parameters about coordinate buffer 
{
  coordinate_statue.buffer1_start_send =0;
  coordinate_statue.buffer2_start_send =0;
  coordinate_statue.buffer1_out_length =0;
  coordinate_statue.buffer2_out_length =0;
  coordinate_statue.buffer1_send_all   =0;
  coordinate_statue.buffer2_send_all   =1;
  //coordinate_statue.buffer1_store_all  =0;
  //coordinate_statue.buffer2_store_all  =0;
  coordinate_statue.buffer1_handle_process =BUFFER_PROCESS_COMBINE_PACK;
  coordinate_statue.buffer2_handle_process =BUFFER_PROCESS_COMBINE_PACK;
  useing_which_resource_data_buffer = USEING_BUFFER1;
    
//  memset(&coordinate_info_buffer1 , 0,sizeof(coordinate_info_buffer1));
//  memset(&coordinate_info_buffer2 , 0,sizeof(coordinate_info_buffer2));
}

uint32_t combine_coordinate_units_to_ble_pack(
coordinate_info_type_t   *coordinate_info_temp,
array_type_t              *coordinate_ble_pack_temp)
{
  uint32_t err_code = NRF_SUCCESS;
  uint16_t units_index =0;  
  uint16_t units_amount_temp  = coordinate_info_temp->units_amount;
  uint8_t  *coordinate_ble_pack_point = &coordinate_ble_pack_temp->data[0];
  uint16_t pack_length =0;

NRF_LOG_INFO("units_amount_temp = %d\r\n",units_amount_temp);


  while(units_amount_temp)
  {
    
    if(units_amount_temp > MAX_UNITS_AMOUNT_A_FRAME)
    {
    
      
      memcpy(coordinate_ble_pack_point , coordinate_info_temp->units[units_index].data ,(COORDINATE_UNIT_LENGTH * MAX_UNITS_AMOUNT_A_FRAME));//copy a coordinate unit to ble pack
      coordinate_ble_pack_point += (COORDINATE_UNIT_LENGTH * MAX_UNITS_AMOUNT_A_FRAME);//ble pack point increase
      pack_length += (COORDINATE_UNIT_LENGTH * MAX_UNITS_AMOUNT_A_FRAME);//ble pack point increase
      
      units_index += MAX_UNITS_AMOUNT_A_FRAME;//unit index increase
      units_amount_temp -= MAX_UNITS_AMOUNT_A_FRAME;//units amount dicrease      
    }
    else
    {
//      *coordinate_ble_pack_point = (COORDINATE_UNIT_LENGTH * units_amount_temp);//parameter length in a fame
//      coordinate_ble_pack_point++;
//      pack_length++;
      memcpy(coordinate_ble_pack_point , coordinate_info_temp->units[units_index].data ,(COORDINATE_UNIT_LENGTH * units_amount_temp));//copy a coordinate unit to ble pack
      coordinate_ble_pack_point += (COORDINATE_UNIT_LENGTH * units_amount_temp);//ble pack point increase
      pack_length += (COORDINATE_UNIT_LENGTH * units_amount_temp);//ble pack point increase
      units_index += units_amount_temp;//unit index increase
      units_amount_temp = 0;//units amount dicrease   
    }  
  
   //NRF_LOG_INFO("pack_length =%d\r\n",pack_length);
   } 
 
  NRF_LOG_HEXDUMP_INFO(&coordinate_ble_pack_temp->data[0],pack_length);
   coordinate_ble_pack_temp->data_length =  pack_length;
   NRF_LOG_INFO("ble pack length =%d\r\n",coordinate_ble_pack_temp->data_length);  

  
  return err_code;
} 
/**@brief Application main function.
 */
int main(void)
{
    bool erase_bonds;

    // Initialize.
    uart_init();
    log_init();
    timers_init();
    buttons_leds_init(&erase_bonds);
    power_management_init();
    ble_stack_init();
    gap_params_init();
    gatt_init();
    services_init();
    advertising_init();
    conn_params_init();
   // fds_test();
  //fstorage_test();
//   array_type_t ble_pack_temp;
//   uint16_t    data_temp_length;

    coordinate_info_type_t  history_coordinate_array;  /*<   600 * 7     */
    array_type_t            coordinate_ble_pack_temp;  /*<  4096 + 1024     */
    uint16_t                data_temp_length;   
  

    
 
    // Start execution.
   
  
    coordinate_buffer_parameter_init();
    advertising_start();
   uint16_t length = 0;
    // Enter main loop.
    uint32_t err_code;
   // NRF_LOG_INFO("sizeof(coordinate_info_buffer1.units) = %d",sizeof(coordinate_info_buffer1.units));
    for (;;)
    {
        idle_state_handle();


        if(coordinate_statue.buffer1_start_send)
        {
          switch (coordinate_statue.buffer1_handle_process)//match which buffer handle process
          {
           case BUFFER_PROCESS_COMBINE_PACK:
              
                err_code = combine_coordinate_units_to_ble_pack(
                                        &coordinate_info_buffer1,
                                        &coordinate_ble_pack_temp
                                       );
                APP_ERROR_CHECK(err_code);


                coordinate_statue.buffer1_handle_process = BUFFER_PROCESS_BLE_TRANSMIT;
                coordinate_info_buffer1.units_amount =0;
                memset(coordinate_info_buffer1.units,0,sizeof(coordinate_info_buffer1.units));
           break;

           case BUFFER_PROCESS_BLE_TRANSMIT:
                
                   data_temp_length = 0;
                   data_temp_length = send_data_to_smartphone(&coordinate_ble_pack_temp.data[coordinate_statue.buffer1_out_length] , coordinate_ble_pack_temp.data_length);
                   coordinate_ble_pack_temp.data_length -= data_temp_length;
                   coordinate_statue.buffer1_out_length += data_temp_length;                   
                   NRF_LOG_INFO(" buffer1_start_send :  %d \r\n",data_temp_length);

                 if(coordinate_ble_pack_temp.data_length ==0)// return to defualt status.
                 {
                  coordinate_statue.buffer1_out_length = 0;
                  coordinate_statue.buffer1_send_all   = 1;
                  coordinate_statue.buffer1_start_send = 0;
                  coordinate_statue.buffer1_handle_process = BUFFER_PROCESS_COMBINE_PACK;
                 } 
           break;

           default :
                 NRF_LOG_INFO("buffer1 error \r\n");
           break;
          }      
        }

        if(coordinate_statue.buffer2_start_send)
        {
          switch (coordinate_statue.buffer2_handle_process)//match which buffer handle process
          {
           case BUFFER_PROCESS_COMBINE_PACK:
                   
                err_code = combine_coordinate_units_to_ble_pack(
                                       
                                        &coordinate_info_buffer2,
                                        &coordinate_ble_pack_temp);
                APP_ERROR_CHECK(err_code);
                coordinate_statue.buffer2_handle_process = BUFFER_PROCESS_BLE_TRANSMIT;
                coordinate_info_buffer2.units_amount =0;

                 memset(coordinate_info_buffer2.units,0,sizeof(coordinate_info_buffer1.units));
           break;

           case BUFFER_PROCESS_BLE_TRANSMIT:
              
                  data_temp_length = 0;
                  data_temp_length = send_data_to_smartphone(&coordinate_ble_pack_temp.data[coordinate_statue.buffer2_out_length] , coordinate_ble_pack_temp.data_length);
                  coordinate_ble_pack_temp.data_length -= data_temp_length;
                  coordinate_statue.buffer2_out_length += data_temp_length;
              
                 NRF_LOG_INFO(" buffer2_start_send :  %d \r\n",data_temp_length);
                if(coordinate_ble_pack_temp.data_length ==0)
                {
                   coordinate_statue.buffer2_out_length = 0;
                   coordinate_statue.buffer2_send_all   = 1;
                   coordinate_statue.buffer2_start_send = 0;
                   coordinate_statue.buffer2_handle_process = BUFFER_PROCESS_COMBINE_PACK;
                }
           break;

           default :
                NRF_LOG_INFO("buffer2 error \r\n");
           break;
          }      
        }

    }
}


/**
 * @}
 */


