
#include "nrf_calendar.h"
#include "sunriset.h"


/*
   1. 天文时钟能够根据 ，UTC 时间戳 和 经纬度信息计算出 日出和日落时间。 可用与街道路灯智能控制的场景中。

   2. 手机APP 同步的UTC 时间戳不需要做任何更改。
   
   3. 计算出的 日出日落时间 大约 早 14 分钟。
   
   4. 手机 APP 发送的 经纬度 为 int16 类型， 例如 ：发送北京的经纬度（116°23′17,北纬:39°54′27）  ，   longitude = 11620 ，latitudu = 3954

*/


typedef struct 
{
   uint8_t opcode;
   uint8_t key;
   uint8_t length;
   uint8_t data[];

}rx_cmd_t;
typedef struct
{
    uint8_t value;
    double longitude;
    double latitude;          
}geographic_location_t;
typedef struct
{
    uint8_t value;
    time_t dawn_time;
    time_t dust_time;          
}dawn_dust_time_t;



typedef struct __attribute((packed))
{
#if 1
    int16_t longitude;
    int16_t latitude;  
#else
    int32_t longitude;
    int32_t latitude; 

#endif
}geographic_location_set_t;

static geographic_location_t m_geographic_location = 
{
  .longitude =0,
  .latitude =0
};
static dawn_dust_time_t m_dawn_dust_time;








int   CaculateWeekDay(int y, int m, int d)
{
            if (m == 1) { m = 13; y--; }
            if (m == 2) { m = 14; y--; }
            int week = (d + 2 * m + 3 * (m + 1) / 5 + y + y / 4 - y / 100 + y / 400) % 7 + 1;
           
            return week;

}
void print_current_time()
{
   // NRF_LOG_INFO("Uncalibrated time:\t%s\r\n", nrf_cal_get_time_string(false));
    NRF_LOG_INFO("Calibrated time:\t%s\r\n", nrf_cal_get_time_string(false));

    time_t  current_time = nrf_cal_get_timestamp();

    NRF_LOG_INFO("current timestamp = %d \n",current_time);


}

void geographic_location_get(geographic_location_t **pp)
{
    *pp = &m_geographic_location;
}
static uint8_t set_longlatitude(void * data)
{
    

    geographic_location_set_t * p_location_set = (geographic_location_set_t *)data;
    geographic_location_t * p_location;
    geographic_location_get(&p_location);
    uint32_t err_code;
    NRF_LOG_INFO("set longtitude = %ld, latitude = %ld \n", p_location_set->longitude, p_location_set->latitude);
    
    double longitude, latitude;
    latitude = p_location_set->latitude / 100.0;
    longitude = p_location_set->longitude / 100.0;

   
    //struct tm l_tm;
    time_t current_time;
    time_t changed_time;

  
        //modify the time
    NRF_LOG_INFO( "get the location now, adapte the time\n");
    current_time = nrf_cal_get_timestamp();

    NRF_LOG_INFO("current_time = %d , old = %d, new =  %d",current_time,240 * p_location->longitude, 240 * longitude)
    changed_time = current_time - 240 * p_location->longitude + 240 * longitude;

   NRF_LOG_INFO("changed_time = %d",changed_time);
    nrf_cal_set_timestamp(changed_time);
        //l_tm = *localtime(&local_time);
    
#if 0


   time_t time = changed_time;



   struct tm * convert;
  convert = malloc(sizeof( struct tm));
   convert =  localtime(&time);
 NRF_LOG_INFO("*** year = %d , month= %d , day= %d ,  hour= %d , mintue= %d , second= %d *** ",convert->tm_year,convert->tm_mon,convert->tm_mday,convert->tm_hour,convert->tm_min,convert->tm_sec );


#endif




    p_location->longitude = longitude;
    p_location->latitude = latitude;


   

    return NRF_SUCCESS;
}
void calculate_currentday_rise_and_set_time(void * p_event_data, uint16_t event_size)
{
    double daylen;
    double rise, set;
    int    rs;
    time_t local_rise_time, local_set_time;
    time_t day_start_time;
    struct tm l_tm_rise, l_tm_set;

    time_t timestamp = *(time_t *)p_event_data;

    /* get time */
    struct tm l_tm = *localtime(&timestamp);

  
    daylen  = day_length(l_tm.tm_year + 1900, l_tm.tm_mon + 1,l_tm.tm_mday, m_geographic_location.longitude, m_geographic_location.latitude); // 计算当天的时长，从日出到日落。
    rs      = sun_rise_set(l_tm.tm_year + 1900, l_tm.tm_mon + 1, l_tm.tm_mday, m_geographic_location.longitude, m_geographic_location.latitude, &rise, &set); // 根据本地的经纬度，计算出 日出，日落时间。
    NRF_LOG_INFO( "Day length:                 " NRF_LOG_FLOAT_MARKER " hours\n", NRF_LOG_FLOAT(daylen) );
    NRF_LOG_INFO("Sun rises " NRF_LOG_FLOAT_MARKER "h UT, sets " NRF_LOG_FLOAT_MARKER "h UT\n",
          NRF_LOG_FLOAT(rise), NRF_LOG_FLOAT(set) );

    l_tm.tm_hour = 0;
    l_tm.tm_min = 0;
    l_tm.tm_sec = 0;
    day_start_time = mktime(&l_tm);
    NRF_LOG_INFO("day start time  %ld \n", day_start_time);

    local_rise_time = (double)day_start_time + rise * 3600.0 + 240.0 * m_geographic_location.longitude;  // 计算日出时的时间戳。
    local_set_time = (double)day_start_time + set * 3600.0 + 240.0 * m_geographic_location.longitude;  // 计算日落时的时间戳。
    NRF_LOG_INFO("local time sun rises  %ld, sets %ld \n", local_rise_time, local_set_time);

    m_dawn_dust_time.dawn_time = local_rise_time;  // 更新本地日出时间戳
    m_dawn_dust_time.dust_time = local_set_time;   // 更新本地日落时间戳
    m_dawn_dust_time.value = 0x41;



    l_tm_rise = *localtime(&local_rise_time);
    l_tm_set = *localtime(&local_set_time);
    NRF_LOG_INFO( "rises at %d:%d, sets at %d:%d \n", l_tm_rise.tm_hour,  l_tm_rise.tm_min,
           l_tm_set.tm_hour,  l_tm_set.tm_min);

}
geographic_location_set_t params;

void cal_init()
{
    nrf_cal_init();
    uint32_t year = 2020;
    uint32_t month = 10;
    uint32_t day = 10;
    uint32_t hour = 8;
    uint32_t minute = 22; 
    uint32_t second = 0;

    nrf_cal_set_callback(print_current_time,2);
    nrf_cal_set_time(year,month,day,hour,minute,second);



    params.longitude = 11620; // 11620
    params.latitude = 3956;  // 3956


//1602291602 

    set_longlatitude(&params);
    time_t timestamp_now = nrf_cal_get_timestamp();
    calculate_currentday_rise_and_set_time(&timestamp_now,0);
   

}
/**@brief Application main function.
 */
 #include "nrf_gpio.h"
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
    app_button_start();
    // Start execution.
    printf("\r\nUART started.\r\n");
 
    advertising_start();
    cal_init();
 
    NRF_LOG_INFO("Debug logging for UART over RTT started.");

   

   
    // Enter main loop.
    for (;;)
    {

        #ifndef FPU_INTERRUPT_MODE
        /* Clear FPSCR register and clear pending FPU interrupts. This code is base on
         * nRF5x_release_notes.txt in documentation folder. It is necessary part of code when
         * application using power saving mode and after handling FPU errors in polling mode.
         */
        __set_FPSCR(__get_FPSCR() & ~(0x0000009F));
        (void) __get_FPSCR();
        NVIC_ClearPendingIRQ(FPU_IRQn);
        #endif



        idle_state_handle();
    }
}


/**
 * @}
 */

