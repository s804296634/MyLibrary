#include "user_flash.h"
#include "nrf_fstorage.h"
#include "fds.h"
#include "app_util.h"

/**@brief Data structure of configuration data stored to flash.
*/
typedef struct 
{
  uint32_t valid;
  boot_page_type_t     boot_page;           // application data block ,which need to storage in flash.
}used_flash_config_data_t;

typedef union
{
  used_flash_config_data_t data;
  uint32_t                 padding[CEIL_DIV(sizeof(used_flash_config_data_t),4)]; // ensure the Data structure is 4-byte aligned.
}used_flash_config_t;

static used_flash_config_t m_flash_config;

static bool m_fds_initialized = false;
static bool m_fds_write_success = false;
static bool m_fds_gc_run = false;

uint32_t  user_flash_init(const boot_page_type_t * p_default_config ,boot_page_type_t ** p_config,bool *p_read_flag )
{
   uint32_t err_code = NRF_SUCCESS;
   

   // initialize FDS module.

   // find record from FDS module.

   // if exist record in flash.

   // do not exist .
        
           // 

   return err_code;
}

uint32_t  used_flash_config_store()
{
  uint32_t err_code = NRF_SUCCESS;


  return err_code;
}

uint32_t  user_flash_config_load()
{
   uint32_t err_code = NRF_SUCCESS;


   return err_code;
}
