#include "nrf_fstorage.h"
#include "nrf_fstorage_sd.h"

#include "nrf_log.h"
#include "app_error.h"


#define FLASH_STA_ADDR  0x75000 
#define FLASH_END_ADDR  0x76000

/*public function*/
uint32_t user_flash_init();
uint32_t user_data_read();
uint32_t flash_erase();

/*private function*/
static void fstorage_evt_handler(nrf_fstorage_evt_t * p_evt);
static void print_flash_info(nrf_fstorage_t * p_fstorage);
static void wait_for_flash_ready(nrf_fstorage_t const * p_fstorage);
static uint32_t nrf5_flash_end_addr_get();



/* Notes:
*  use : __attribute__((aligned (4)))  complete Byte alignment.
*   
*  Useage:
*  typedef struct
*   {
*	uint8_t soft_version[2];
*	uint8_t firmware_version[3]
*  }user_data_t;
*
*  user_data_t user_configure __attribute__((aligned (4))) =
*	{
*	  .soft_version = {0x00,0x59},
*	  .firmware_version = {0x01,0x01,0x01}
*	};
*
*	user_data_t  configure_temp  __attribute__ ((aligned (4)));
*/
user_data_t  configure_temp  __attribute__ ((aligned (4)));
/*flash module needed instance */

NRF_FSTORAGE_DEF(nrf_fstorage_t fstorage) =
{
    /* Set a handler for fstorage events. */
    .evt_handler = fstorage_evt_handler,

    /* These below are the boundaries of the flash space assigned to this instance of fstorage.
     * You must set these manually, even at runtime, before nrf_fstorage_init() is called.
     * The function nrf5_flash_end_addr_get() can be used to retrieve the last address on the
     * last page of flash available to write data. */
    .start_addr = FLASH_STA_ADDR,
    .end_addr   = FLASH_END_ADDR,
};


uint32_t user_flash_init()
{

  // 1. init flash module.
   nrf_fstorage_api_t * p_fs_api;
   ret_code_t rc;

    p_fs_api = &nrf_fstorage_sd;

     rc = nrf_fstorage_init(&fstorage, p_fs_api, NULL);

     if(rc != NRF_SUCCESS)
     {
       return rc;
     }
   

    print_flash_info(&fstorage);
   return NRF_SUCCESS;
}
uint32_t user_data_read()
{
 
    uint32_t rc;
    uint8_t flash_data[1024] = {0};
    uint8_t erase_data[4] = {0xFF,0xFF,0xFF,0xFF};

  

     // 1. use API read momery buffer from flash.
    rc = nrf_fstorage_read(&fstorage,FLASH_STA_ADDR,flash_data,sizeof(flash_data));

    // 2.check if no data in flash,then  write defualt configure params into flash.
    if(rc ！= NRF_SUCCESS) 
    {  
     
        NRF_LOG_INFO("flash read error: %x.\n",rc);
    }
    else
    {
     

       if( p_data[0]== 0xFF &&
            p_data[1] == 0xFF &&
            p_data[2] == 0xFF &&
            p_data[3] == 0xFF )
      {
        NRF_LOG_INFO("do not exist data in flash.\n");
        rc = nrf_fstorage_write(&fstorage,FLASH_STA_ADDR,&user_configure,sizeof(user_configure),NULL);
        APP_ERROR_CHECK(rc);
      }
      else
      {
        // 3. copy previous configure data for application .
      	#if 0
      	  memset(configure_temp,0,sizeof(configure_temp));   	 
      	  memcpy(configure_temp,flash_data,sizeof(configure_temp)); 
        #endif 
      }
    }  

    return NRF_SUCCESS;
}
uint32_t flash_erase()
{
    uint32_t rc;
    // 1.wait previous flash operation finish.
    wait_for_flash_ready(&fstorage);

   // 2. erase all page
    NRF_LOG_DEBUG("start erase operation .\n");
   rc =  nrf_fstorage_erase(&fstorage,FLASH_STA_ADDR,1,NULL);
    APP_ERROR_CHECK(rc);

  // 3. wait for erase operation finish. 
     wait_for_flash_ready(&fstorage);
     NRF_LOG_DEBUG("erase operation done! .\n");
}








static void print_flash_info(nrf_fstorage_t * p_fstorage)
{
    NRF_LOG_INFO("========| flash info |========");
    NRF_LOG_INFO("erase unit: \t%d bytes",      p_fstorage->p_flash_info->erase_unit);
    NRF_LOG_INFO("program unit: \t%d bytes",    p_fstorage->p_flash_info->program_unit);
    NRF_LOG_INFO("==============================");
}

static void fstorage_evt_handler(nrf_fstorage_evt_t * p_evt)
{
    if (p_evt->result != NRF_SUCCESS)
    {
        NRF_LOG_INFO("--> Event received: ERROR while executing an fstorage operation.");
        return;
    }

    switch (p_evt->id)
    {
        case NRF_FSTORAGE_EVT_WRITE_RESULT:
        {
            NRF_LOG_INFO("--> Event received: wrote %d bytes at address 0x%x.",
                         p_evt->len, p_evt->addr);
        } break;

        case NRF_FSTORAGE_EVT_ERASE_RESULT:
        {
            NRF_LOG_INFO("--> Event received: erased %d page from address 0x%x.",
                         p_evt->len, p_evt->addr);
        } break;

        default:
            break;
    }
}

/**@brief   Helper function to obtain the last address on the last page of the on-chip flash that
 *          can be used to write user data.
 */
static uint32_t nrf5_flash_end_addr_get()
{
    uint32_t const bootloader_addr = BOOTLOADER_ADDRESS;
    uint32_t const page_sz         = NRF_FICR->CODEPAGESIZE;
    uint32_t const code_sz         = NRF_FICR->CODESIZE;


 NRF_LOG_DEBUG("bootloader_addr : 0x%x ",bootloader_addr);
    return (bootloader_addr != 0xFFFFFFFF ?
            bootloader_addr : (code_sz * page_sz));
}

static void wait_for_flash_ready(nrf_fstorage_t const * p_fstorage)
{
    while(nrf_fstorage_is_busy(p_fstorage))
    {
       // NRF_LOG_INFO("wait for flash instance .\n");
       //vTaskDelay(500);
    }

}