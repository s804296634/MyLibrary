
/*
@echo off
nrfjprog.exe --family nRF52 --memwr 0x10001088 --val 0x%str:~0,6%
nrfjprog.exe --family nRF52 --memwr 0x1000108C --val 0x%str:~6,6%

*/
void set_local_mac()
{

  /* 1.read mac address from  register.*/ 
  
    gateway_product_info.mac[2]=(NRF_UICR->CUSTOMER[2]>>0);
    gateway_product_info.mac[1]=(NRF_UICR->CUSTOMER[2]>>8);
    gateway_product_info.mac[0]=(NRF_UICR->CUSTOMER[2]>>16);

    gateway_product_info.mac[5]=(NRF_UICR->CUSTOMER[3]>>0);
    gateway_product_info.mac[4]=(NRF_UICR->CUSTOMER[3]>>8);
    gateway_product_info.mac[3]=(NRF_UICR->CUSTOMER[3]>>16);

  /* 2. call softdevice API set Local mac address. */  
  
    ble_gap_addr_t addr;
    memset(&addr, 0, sizeof(addr));
    err_code = sd_ble_gap_addr_get(&addr);
    APP_ERROR_CHECK(err_code);	
    addr.addr[0]=gateway_product_info.mac[5];
    addr.addr[1]=gateway_product_info.mac[4];
    addr.addr[2]=gateway_product_info.mac[3];
    addr.addr[3]=gateway_product_info.mac[2];
    addr.addr[4]=gateway_product_info.mac[1];
    addr.addr[5]=gateway_product_info.mac[0];
    
    if((addr.addr[5]>>4)>=12)  /* 2.1 check the validity of address. */ 
    {				
        err_code = sd_ble_gap_addr_set(&addr);
        APP_ERROR_CHECK(err_code);
    }
}
