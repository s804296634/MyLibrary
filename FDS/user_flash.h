#ifndef  _USER_FLASH_
#define  _USER_FLASH_

#include <stdint.h>
#include <stdbool.h>


#define COORDINATE_PAGE_MAX_AMOUNT   256

typedef struct //specific coordinate page information 
{	
  uint32_t unique_id;
  uint32_t start_adress;
  uint32_t units_amount;//units amount
}coordinate_page_info_type_t;

typedef struct //struct of coordinate storage relevant parameters ,the length is stable 4KB,update when new page close,delete page.
{  
  uint32_t  available_adress;
  uint32_t  clear_adress;
  int       current_page_amount;
  uint32_t  left_storage_low;//storage of the mx25 is less than %30
  uint32_t  no_space;
  uint32_t  current_id;
  coordinate_page_info_type_t info[COORDINATE_PAGE_MAX_AMOUNT];
}boot_page_type_t;




uint32_t  user_flash_init(const boot_page_type_t * p_default_config ,boot_page_type_t ** p_config,bool *p_read_flag);

uint32_t  used_flash_config_store();

uint32_t  user_flash_config_load();

#endif
