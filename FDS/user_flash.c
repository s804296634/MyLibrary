#include "user_flash.h"
#include "nrf_fstorage.h"
#include "fds.h"
#include "app_util.h"
#include "nrf_log.h"
#include "macros_common.h"
#include "app_scheduler.h"
/**@brief Data structure of configuration data stored to flash.
*/
typedef struct 
{
  uint32_t valid;
  user_data_t     user_data;           // application data block ,which need to storage in flash.
}used_flash_config_data_t;

typedef union
{
  used_flash_config_data_t data;
  uint32_t                 padding[CEIL_DIV(sizeof(used_flash_config_data_t),4)]; // ensure the Data structure is 4-byte aligned.
}used_flash_config_t;


static fds_record_desc_t      m_record_desc;
static used_flash_config_t    m_flash_config;

static bool m_fds_initialized = false;
static bool m_fds_write_success = false;
static bool m_fds_gc_run = false;


/**@brief Function for handling flash data storage events.
 */
static void tc_fds_evt_handler(fds_evt_t const * const p_fds_evt)
{
    switch (p_fds_evt->id)
    {
        case FDS_EVT_INIT:
            if (p_fds_evt->result == FDS_SUCCESS)
            {
                m_fds_initialized = true;
            }
            else
            {
                // Initialization failed.
                NRF_LOG_ERROR("FDS init failed!\r\n");
                APP_ERROR_CHECK_BOOL(false);
            }
            break;
        case FDS_EVT_WRITE:
            if (p_fds_evt->result == FDS_SUCCESS)
            {
                if (p_fds_evt->write.file_id == TC_FILE_ID)
                {
                    NRF_LOG_DEBUG("FDS write success! %d FileId: 0x%x RecKey:0x%x\r\n", p_fds_evt->write.is_record_updated,
                                                                                                  p_fds_evt->write.file_id,
                                                                                                  p_fds_evt->write.record_key);
                    m_fds_write_success = true;

                  NRF_LOG_INFO("FDS write success\n");
                }
            }
            else
            {
                // Initialization failed.
                NRF_LOG_ERROR("FDS write failed!\r\n");
                NRF_LOG_INFO("FDS write success\n");
                APP_ERROR_CHECK_BOOL(false);
            }
            break;
        case FDS_EVT_GC:
            if (p_fds_evt->result == FDS_SUCCESS)
            {
                NRF_LOG_INFO("garbage collect success\r\n");
                m_fds_gc_run = true;
            }
            break;
        case FDS_EVT_UPDATE:
            if (p_fds_evt->result == FDS_SUCCESS)
            {
                if (p_fds_evt->write.file_id == TC_FILE_ID)
                {
                    NRF_LOG_DEBUG("FDS update success! %d FileId: 0x%x RecKey:0x%x\r\n", p_fds_evt->write.is_record_updated,
                                                                                                  p_fds_evt->write.file_id,
                                                                                                  p_fds_evt->write.record_key);
                    //fds_update_flag_set(true);
                    NRF_LOG_INFO("FDS update success\n");
                }
            }
            else
            {
                // Initialization failed.
                NRF_LOG_ERROR("FDS update failed!\r\n");
                NRF_LOG_INFO("FDS update success\n");
                APP_ERROR_CHECK_BOOL(false);
            }
      break;
      
        default:
            NRF_LOG_DEBUG("FDS handler - %d - %d\r\n", p_fds_evt->id, p_fds_evt->result);
            APP_ERROR_CHECK(p_fds_evt->result);
            break;
    }
}


uint32_t  user_flash_init(const user_data_t * p_default_config ,user_data_t ** p_config,bool *p_read_flag )
{
   uint32_t err_code = NRF_SUCCESS;
   
   NRF_LOG_INFO("Initialization\r\n");
  
   if (p_default_config == NULL ) 
   {
     return NRF_ERROR_NULL;
   }

   // initialize FDS module.

    err_code = fds_register(tc_fds_evt_handler);
    if ( err_code != NRF_SUCCESS)
    {
      return err_code;
    }

    err_code = fds_init();
    if ( err_code != NRF_SUCCESS)
    {
      return err_code;
    }

    while(m_fds_initialized == false)
    {
       app_sched_execute();
    }

    err_code = fds_gc();
    if ( err_code != NRF_SUCCESS)
    {
      return err_code;
    }
    while (m_fds_gc_run == false)
    {
       app_sched_execute();
    }


   // find record from FDS module.
   err_code = user_flash_config_load(p_config);
   if ( err_code == FDS_ERR_NOT_FOUND)     // if exist record in flash.
   {
       fds_record_t        record;
       NRF_LOG_INFO("writing default config\r\n");
       memcpy(&m_flash_config.data.user_data,p_default_config,sizeof(user_data_t));
       m_flash_config.data.valid = TC_FLASH_CONFIG_VALID;

       record.file_id           = TC_FILE_ID;
       record.key               = TC_REC_KEY;
       record.data.p_data       = &m_flash_config;
       record.data.length_words = sizeof(used_flash_config_t)/4;

       m_fds_write_success = false;
       err_code = fds_record_write(&m_record_desc, &record);
        RETURN_IF_ERROR(err_code);

        *p_config = &m_flash_config.data.user_data;

        * p_read_flag = false;
        while(m_fds_write_success != true)
        {
            app_sched_execute();
        }
   }
   else             // do not exist .
   {
     *p_read_flag = true;
      
   }
  
   return err_code;
}

uint32_t  used_flash_config_store(user_data_t * p_data)
{
  uint32_t err_code = NRF_SUCCESS;
  fds_record_t record;
  
  NRF_LOG_INFO("storing user flash data");

  NULL_PARAM_CHECK(p_data);
  memcpy(&m_flash_config.data.user_data,p_data,sizeof(user_data_t));

  m_flash_config.data.valid = TC_FLASH_CONFIG_VALID;

  // set up record.
  record.file_id = TC_FILE_ID;
  record.key     = TC_REC_KEY;
  record.data.p_data = &m_flash_config;
  record.data.length_words = sizeof(used_flash_config_t)/4;


  err_code = fds_record_update(&m_record_desc,&record);
  if(err_code == FDS_ERR_NO_SPACE_IN_FLASH)
  {
    NRF_LOG_INFO("garbage collection\n");
    fds_gc();
    err_code = fds_record_update(&m_record_desc,&record);
  }
  RETURN_IF_ERROR(err_code);

  return err_code;
}

uint32_t  user_flash_config_load(user_data_t ** p_data)
{
   uint32_t err_code = NRF_SUCCESS;
   fds_flash_record_t  flash_record;
   fds_find_token_t    ftok;


   memset(&ftok, 0x00, sizeof(fds_find_token_t));
   NRF_LOG_INFO("Loading configuration\r\n");

   
    err_code = fds_record_find(TC_FILE_ID, TC_REC_KEY, &m_record_desc, &ftok);
    RETURN_IF_ERROR(err_code);

    err_code = fds_record_open(&m_record_desc, &flash_record);
    RETURN_IF_ERROR(err_code);

     memcpy(&m_flash_config, flash_record.p_data, sizeof(used_flash_config_t));

    err_code = fds_record_close(&m_record_desc);
    RETURN_IF_ERROR(err_code);

    *p_data = &m_flash_config.data.user_data;

    return err_code;
}
