[TOC]
版分管理

|版本号|制定日期|修改记录|author|
|:----    |:---|:----- |-----   |
|0.0.1 |2019-04-01 |Initial release | Hurry |
|0.0.2 |2019-07-01 |1:增加log字段 | Hurry |

# Mesh网关简介

迅通智能家庭Mesh网关由两部分构成，一部分是Mesh模组；另一部分是WIFI模组；本文主要介绍WIFI模组的功能和功能实现的软件架构。

- WIFI模组和Mesh模组使用uart通信
- WIFI模组使用MQTT与云端通信。

# 硬件部分

WIFI模组使用乐鑫的ESP8666模组 [ESP-WROOM-02D](https://www.espressif.com/zh-hans/products/hardware/esp-wroom-02/resources).
- 硬件引脚连接：[ESP-WROOM-02D](https://www.espressif.com/zh-hans/products/hardware/esp-wroom-02/resources)模组的5(IO13)、6(IO15)）、9(GND)分别对应接Mesh模组的TXD、RXD、GND引脚。
- 波特率为115200bps.

# 软件部分

##环境安装和代码调试
开发 [ESP-WROOM-02D](https://www.espressif.com/zh-hans/products/hardware/esp-wroom-02/resources)没有使用官方的AT固件，而是使用官方的[SDK](https://github.com/espressif/ESP8266_RTOS_SDK)进行二次开发.开发环境安装及编译、下载、调试方法见https://github.com/espressif/ESP8266_RTOS_SDK/blob/master/README.md

##代码协议

 [ESP-WROOM-02D](https://www.espressif.com/zh-hans/products/hardware/esp-wroom-02/resources)使用uart和Mesh模组通信，uart通信和普通uart通信相同，不再赘述。和云端通信的协议为MQTT
 #### 域名/IP信息:
- www.xxxxxxxx.com/47.106.164.80
-  端口：1883
#### MQTT publish 参数：

|参数名|是否必须|类型|说明|
|:----    |:---|:----- |-----   |
|topic |是  |string |格式："uploadData/"+Mesh Net Key   例如：" uploadData/000000024489EF0DA1E29393E9B4D741" <Br>Mesh Net Key 由手机创建 BLE Mesh 网络时生成，用于标识一个BLE Mesh 网络（一个 BLE Mesh网络有且仅有一个 Mesh Net Key）同时手机在创建一个BLE Mesh 网络会将这个Mesh Net key上报服务器|
|userName |否  |string | 取决于broker，默认为"admin"  |
|password  |否  |string | 取决于broker，默认为"password"  |
|keep alive timer  |是  |ASCII | 长连接支持、默认心跳间隔为30s，单位秒。  |
#### MQTT subscribe 参数：

|参数名|是否必须|类型|说明|
|:----    |:---|:----- |-----   |
|topic |是  |string |格式：Mesh Net Key+"/downloadData" 例如："000000024489EF0DA1E29393E9B4D741/downloadData" <Br>Mesh Net Key 由手机创建 BLE Mesh 网络时生成，用于标识一个BLE Mesh 网络（一个 BLE Mesh网络有且仅有一个 Mesh Net Key）同时手机在创建一个BLE Mesh 网络会将这个Mesh Net key上报服务器|
|userName |否  |string | 取决于broker，默认为"admin"  |
|password  |否  |string | 取决于broker，默认为"password"  |
|keep alive timer  |是  |ASCII | 长连接支持、默认心跳间隔为30s，单位秒。  |
 
 MQTT通信包基础协议为json，比如上报Mesh网络中的节点的power状态：
```
{
      "packId":1,     
      "gateway":
				{
					"uniqAddress":2,
					"firmVersion":1
				},
      "device":
				{
					"uniqAddress":3,
					"cmd":"power",
					"value":true
				}
}
```
 详细通信协议见http://192.168.1.7:8181/docs/xt_mesh/xt_mesh-1b3i34k8oanrl
 
 ##软件架构
 二次开发使用freeRTOS作为软件基础平台，各个模块分割成单独的任务。
 
 ![软件架构图](http://192.168.1.7:8181/uploads/meshGateway-wifi/images/m_917bd6feccbe68ad1346b5c1d5090c3f_r.png "软件架构图")

在Mesh网关的WIFI模组中共有6个任务：
- uart rx 任务 将收到的数据通过队列消息传送到mqtt 任务，mqtt 任务将这些消息数据组包publish到云端服务器；uart rx 任务发送任务通知到WIFI test 任务执行WIFI测试；WIFI 测试的结果通过队列消息传送到uart tx 任务。
- mqtt订阅任务将订阅到云端信息解析后通过队列消息送到uart tx 任务；同时如果解析到云端服务器要执行固件更新，则发送任务通知到ota 任务。

##关键部分代码

#mqtt任务中 pub的处理
<br>不断查询任务队列，对于有效的数据，组json包，发送到云端服务器，详细代码如下：

```c
uint32_t device_status_queue_read(mesh_device_t* device_status)
{    
      uint32_t err_code = NRF_ERROR_NULL;

      if(xQueueReceive(device_status_queue, (void*)device_status, MESSAGE_STATUS_QUEUE_TIMEOUT) == pdTRUE)
      {
            err_code = 0; 
      }
      
      return  err_code;    
}

char* mqtt_pub_pack_handler(uint32_t package_id)
{
    char  *ret = NULL;
    mesh_device_t device_status;

    cJSON *m_object = cJSON_CreateObject();
    if (m_object == NULL)
    {
      goto end;
    }

    if(device_status_queue_read(&device_status) != 0)
    {
      goto end;
    }  

    cJSON_AddNumberToObject(m_object, JSON_PACKAGE_ID, package_id);

    cJSON *s_object1 = cJSON_CreateObject();
    if (s_object1 == NULL)
    {
        goto end;
    }

    cJSON_AddItemToObject(m_object, JSON_GATEWAY_OBJECT, s_object1);

    cJSON_AddNumberToObject(s_object1, JSON_GATEWAY_UNIQUE_ADDRESS, customer_configure.unique_address);

    cJSON_AddNumberToObject(s_object1, JSON_GATEWAY_FIRMWARE_VERSION, firmware_version);

    cJSON *s_object2 = cJSON_CreateObject();
    if (s_object2 == NULL)
    {
        goto end;
    }
   
    cJSON_AddItemToObject(m_object, JSON_DEVICE_OBJECT, s_object2);

    cJSON_AddNumberToObject(s_object2, JSON_DEVICE_UNIQUE_ADDRESS, device_status.sour_add);

    switch (device_status.cmd.cmd)//commamd or stasus 
    {      
      case DEVICE_CMD_COMMON_IS_POWER:            
            cJSON_AddItemToObject(s_object2, JSON_DEVICE_CMD, cJSON_CreateString(JSON_DEVICE_CMD_POWER));
            cJSON_AddBoolToObject(s_object2, JSON_DEVICE_CMD_VALUE, device_status.cmd.cmd_value.true_or_false);
            break;

      case DEVICE_CMD_COMMON_FIRMWARE:            
            cJSON_AddItemToObject(s_object2, JSON_DEVICE_CMD, cJSON_CreateString(JSON_DEVICE_FIRMWARE_VERSION));
            int firmware[3];
            firmware[0] = device_status.cmd.cmd_value.data[0] + (device_status.cmd.cmd_value.data[1]*256);
            firmware[1] = device_status.cmd.cmd_value.data[2] + (device_status.cmd.cmd_value.data[3]*256);
            firmware[2] = device_status.cmd.cmd_value.data[4];
            cJSON_AddItemToObject(s_object2, JSON_DEVICE_CMD_VALUE, cJSON_CreateIntArray(firmware, 3));
            break;

      case DEVICE_CMD_COMMON_TIMESTAMP:            
            cJSON_AddItemToObject(s_object2, JSON_DEVICE_CMD, cJSON_CreateString(JSON_DEVICE_CMD_TIMESTAMP));
            cJSON_AddItemToObject(s_object2, JSON_DEVICE_CMD_VALUE, cJSON_CreateNull());
            break;

      case DEVICE_CMD_HEALTH_STATUS:            
            cJSON_AddItemToObject(s_object2, JSON_DEVICE_CMD, cJSON_CreateString(JSON_DEVICE_CMD_HEALTH_STATUS));
            switch (device_status.cmd.cmd_value.health_status)//commamd or stasus 
            {      
              case HEALTH_STATUS_NORMAL:
                    cJSON_AddNumberToObject(s_object2, JSON_DEVICE_CMD_VALUE, HEALTH_STATUS_NORMAL);
                    break;

              case HEALTH_STATUS_LOW_POWER_WARNING:
                    cJSON_AddNumberToObject(s_object2, JSON_DEVICE_CMD_VALUE, HEALTH_STATUS_LOW_POWER_WARNING);
                    break;

              case HEALTH_STATUS_LOW_POWER_ERROR:
                    cJSON_AddNumberToObject(s_object2, JSON_DEVICE_CMD_VALUE, HEALTH_STATUS_LOW_POWER_ERROR);
                    break;

              default :
                    goto end;
                    break;              
            }
            break;

      case DEVICE_CMD_COMMON_ONOFF_STATUS:            
            cJSON_AddItemToObject(s_object2, JSON_DEVICE_CMD, cJSON_CreateString(JSON_DEVICE_CMD_ONOFF_STATUS));
            cJSON_AddBoolToObject(s_object2, JSON_DEVICE_CMD_VALUE, device_status.cmd.cmd_value.true_or_false);
            break;

      case DEVICE_CMD_LIGHT_LIGHTENESS:            
            cJSON_AddItemToObject(s_object2, JSON_DEVICE_CMD, cJSON_CreateString(JSON_DEVICE_CMD_LIGTH_LIGHTENESS));
            uint16_t light_lighteness = (device_status.cmd.cmd_value.data[0] + (device_status.cmd.cmd_value.data[1]*256));
            cJSON_AddNumberToObject(s_object2, JSON_DEVICE_CMD_VALUE, light_lighteness);
            break;

      case DEVICE_CMD_LIGHT_CTL:            
            cJSON_AddItemToObject(s_object2, JSON_DEVICE_CMD, cJSON_CreateString(JSON_DEVICE_CMDLIGTH_CTL));
            int light_ctl[2];
            light_ctl[0] = device_status.cmd.cmd_value.data[0] + (device_status.cmd.cmd_value.data[1]*256);
            light_ctl[1] = device_status.cmd.cmd_value.data[2] + (device_status.cmd.cmd_value.data[3]*256);
            cJSON_AddItemToObject(s_object2, JSON_DEVICE_CMD_VALUE, cJSON_CreateIntArray(light_ctl, 2));
            break;

      case DEVICE_CMD_LIGHT_HSL:            
            cJSON_AddItemToObject(s_object2, JSON_DEVICE_CMD, cJSON_CreateString(JSON_DEVICE_CMDLIGTH_HSL));
            int light_hsl[3];
            light_hsl[0] = device_status.cmd.cmd_value.data[0] + (device_status.cmd.cmd_value.data[1]*256);
            light_hsl[1] = device_status.cmd.cmd_value.data[2] + (device_status.cmd.cmd_value.data[3]*256);
            light_hsl[2] = device_status.cmd.cmd_value.data[4] + (device_status.cmd.cmd_value.data[5]*256);
            cJSON_AddItemToObject(s_object2, JSON_DEVICE_CMD_VALUE, cJSON_CreateIntArray(light_hsl, 3));
            break;

      case DEVICE_CMD_SENSOR_TEMPETATURE:            
            cJSON_AddItemToObject(s_object2, JSON_DEVICE_CMD, cJSON_CreateString(JSON_DEVICE_CMD_SENSOR_TEMPERATURE));
            uint16_t s_temperature = (device_status.cmd.cmd_value.data[0] + (device_status.cmd.cmd_value.data[1]*256));
            cJSON_AddNumberToObject(s_object2, JSON_DEVICE_CMD_VALUE, s_temperature);
            break;

      case DEVICE_CMD_SENSOR_HUMITIDY:            
            cJSON_AddItemToObject(s_object2, JSON_DEVICE_CMD, cJSON_CreateString(JSON_DEVICE_CMD_SENSOR_HUMITIDY));
            uint16_t s_humitidy = (device_status.cmd.cmd_value.data[0] + (device_status.cmd.cmd_value.data[1]*256));
            cJSON_AddNumberToObject(s_object2, JSON_DEVICE_CMD_VALUE, s_humitidy);
            break;

      case DEVICE_CMD_DOOR_MAGNETIC:            
            cJSON_AddItemToObject(s_object2, JSON_DEVICE_CMD, cJSON_CreateString(JSON_DEVICE_CMD_DOOR_MAGNETIC));
            cJSON_AddBoolToObject(s_object2, JSON_DEVICE_CMD_VALUE, device_status.cmd.cmd_value.true_or_false);
            break;

      default ://response an error code
            goto end;
            break;
    }

    ret = cJSON_Print(m_object);
    if (ret == NULL) 
    {
        goto end;     
    }

    end:
    cJSON_Delete(m_object);
    return ret;
}
```
#sub 处理：
<br>sub使用MQTTYield()函数，然后通过回调得到和处理订阅的信息，订阅到的信息解析后，将需要写入Mesh模组的uart控制指令写入uart tx 队列。详细代码如下：

```c
...............
...............
rc = MQTTYield(&client, MQTT_YELD_INTERVAL_MS);
if (rc != 0) 
{
	break;
}

static void messageArrived(MessageData *data)
{
    ESP_LOGI(TAG, "Message arrived[len:%u]: %.*s", \
           data->message->payloadlen, data->message->payloadlen, (char *)data->message->payload);
    mqtt_sub_message_payload_handler((char *)data->message->payload, data->message->payloadlen);    
}

void mqtt_sub_message_payload_handler(char* payload, uint16_t payload_length)
{  
  static uint8_t message_id =0;
  mesh_message_t p_message;

  cJSON *json = cJSON_Parse(payload);
  cJSON *m_device_node = NULL;
  m_device_node = cJSON_GetObjectItem(json, JSON_DEVICE_OBJECT);  
  
  if(m_device_node != NULL && m_device_node->type == cJSON_Object)
  {
    ESP_LOGI(TAG, "found device object");   
     
    p_message.version_and_type.version = (0x00 & 0xF0);//stable
    p_message.version_and_type.type    = (0x01 & 0x0F);//stable    
    p_message.sour_add                 = 0x0001;//stable for gateway client  

    cJSON *s_node_1 = NULL;
    s_node_1 = cJSON_GetObjectItem(m_device_node, JSON_DEVICE_UNIQUE_ADDRESS);
    if(s_node_1 != NULL && s_node_1->type == cJSON_Number)
    {
      p_message.dest_add             = s_node_1->valuedouble; 
      ESP_LOGI(TAG, "dest add =%04X", p_message.dest_add); 
    }
    else
    {
      goto exit;//error do nothing
    }
    bool dest_add_group = false;
    uint16_t min_group_add = 0xC000;
    if(p_message.dest_add >= min_group_add)
    {
      dest_add_group = true;
    } 
    s_node_1 = cJSON_GetObjectItem(m_device_node, JSON_DEVICE_CMD);
    if(s_node_1 != NULL && s_node_1->type == cJSON_String)
    {
      char string_array[64];

      memset(string_array, 0, sizeof(string_array));
      memcpy(string_array, s_node_1->valuestring, strlen(s_node_1->valuestring));
      if(memcmp(string_array, JSON_DEVICE_CMD_ONOFF_STATUS, strlen(JSON_DEVICE_CMD_ONOFF_STATUS)) == 0)
      {
        s_node_1 = cJSON_GetObjectItem(m_device_node, JSON_DEVICE_CMD_VALUE);
        if(s_node_1 != NULL && s_node_1->type ==  cJSON_True)
        {
          p_message.data.parameter[0] = 1;            
        }
        else if (s_node_1 != NULL && s_node_1->type ==  cJSON_False) {
          p_message.data.parameter[0] = 0;
        }
        else
        {
          goto exit;
        }
        ESP_LOGI(TAG, "onoff status =%d", p_message.data.parameter[0]);
        p_message.data.tid = message_id;
        p_message.data.parameter_lenght = 1;
        p_message.data.opcode      =  MEAH_LIGHT_OPCODE_ONOFF_SET;
        if(dest_add_group)
        {
          p_message.data.opcode      =  MEAH_LIGHT_OPCODE_ONOFF_UNACK;
        }        
        p_message.data_length      = (p_message.data.parameter_lenght + 
                                      MESH_MESSAGE_TID_LENGTH +
                                      MESH_MESSAGE_OPCODE_LENGTH);
      }
      else if(memcmp(string_array, JSON_DEVICE_CMD_LIGTH_LIGHTENESS, strlen(JSON_DEVICE_CMD_LIGTH_LIGHTENESS)) == 0)
      {
        uint16_t light_lighteness;
        s_node_1 = cJSON_GetObjectItem(m_device_node, JSON_DEVICE_CMD_VALUE);
        if(s_node_1 != NULL && s_node_1->type ==  cJSON_Number)
        {
          light_lighteness = s_node_1->valuedouble;                     
        }
        else
        {
          goto exit;
        }
        ESP_LOGI(TAG, "lighteness =%d", light_lighteness);
        p_message.data.parameter[0] = (uint8_t)light_lighteness;
        p_message.data.parameter[1] = (uint8_t)(light_lighteness >> 8); 
        p_message.data.tid = message_id;
        p_message.data.parameter_lenght = 2;
        p_message.data.opcode      =  MEAH_LIGHT_OPCODE_LIGHTENESS;
        if(dest_add_group)
        {
          p_message.data.opcode      =  MEAH_LIGHT_OPCODE_LIGHTENESS_UNACK;
        }
        p_message.data_length      = (p_message.data.parameter_lenght + 
                                      MESH_MESSAGE_TID_LENGTH +
                                      MESH_MESSAGE_OPCODE_LENGTH);
      }
      else if(memcmp(string_array, JSON_DEVICE_CMDLIGTH_CTL, strlen(JSON_DEVICE_CMDLIGTH_CTL)) == 0)
      {
        uint16_t light_ctl[2];
        s_node_1 = cJSON_GetObjectItem(m_device_node, JSON_DEVICE_CMD_VALUE);
        if(s_node_1 != NULL && s_node_1->type ==  cJSON_Array)
        {          
          cJSON *s_node_2 = NULL;

          s_node_2 = cJSON_GetArrayItem(s_node_1, 0);
          if(s_node_2 != NULL && s_node_2->type == cJSON_Number)
          {
            light_ctl[0] = s_node_2->valuedouble;
          }
          else
          {
            goto exit;
          }          
          s_node_2 = cJSON_GetArrayItem(s_node_1, 1);
          if(s_node_2 != NULL && s_node_2->type == cJSON_Number)
          {
            light_ctl[1] = s_node_2->valuedouble;
          }
          else
          {
            goto exit;
          }
        }
        else
        {
          goto exit;
        }
        ESP_LOGI(TAG, "light ctl =%d,%d,%d", light_ctl[0], light_ctl[1], light_ctl[2]);
        p_message.data.parameter[0] = (uint8_t)light_ctl[0];
        p_message.data.parameter[1] = (uint8_t)(light_ctl[0] >> 8);
        p_message.data.parameter[2] = (uint8_t)light_ctl[1];
        p_message.data.parameter[3] = (uint8_t)(light_ctl[1] >> 8);  
        p_message.data.tid = message_id;
        p_message.data.parameter_lenght = 4;
        p_message.data.opcode      =  MEAH_LIGHT_OPCODE_CTL;
        if(dest_add_group)
        {
          p_message.data.opcode      =  MEAH_LIGHT_OPCODE_CTL_UNACK;
        }
        p_message.data_length      = (p_message.data.parameter_lenght + 
                                      MESH_MESSAGE_TID_LENGTH +
                                      MESH_MESSAGE_OPCODE_LENGTH);
      }
      else if(memcmp(string_array, JSON_DEVICE_CMDLIGTH_HSL, strlen(JSON_DEVICE_CMDLIGTH_HSL)) == 0)
      {
        uint16_t light_hsl[3];
        s_node_1 = cJSON_GetObjectItem(m_device_node, JSON_DEVICE_CMD_VALUE);
        if(s_node_1 != NULL && s_node_1->type ==  cJSON_Array)
        {          
          cJSON *s_node_2 = NULL;

          s_node_2 = cJSON_GetArrayItem(s_node_1, 0);
          if(s_node_2 != NULL && s_node_2->type == cJSON_Number)
          {
            light_hsl[0] = s_node_2->valuedouble;
          }
          else
          {
            goto exit;
          }          
          s_node_2 = cJSON_GetArrayItem(s_node_1, 1);
          if(s_node_2 != NULL && s_node_2->type == cJSON_Number)
          {
            light_hsl[1] = s_node_2->valuedouble;
          }
          else
          {
            goto exit;
          }
          s_node_2 = cJSON_GetArrayItem(s_node_1, 2);
          if(s_node_2 != NULL && s_node_2->type == cJSON_Number)
          {
            light_hsl[2] = s_node_2->valuedouble;
          }
          else
          {
            goto exit;
          }
        }
        else
        {
          goto exit;
        }
        ESP_LOGI(TAG, "light ctl =%d,%d,%d", light_hsl[0], light_hsl[1], light_hsl[2]);
        p_message.data.parameter[0] = (uint8_t)light_hsl[0];
        p_message.data.parameter[1] = (uint8_t)(light_hsl[0] >> 8);
        p_message.data.parameter[2] = (uint8_t)light_hsl[1];
        p_message.data.parameter[3] = (uint8_t)(light_hsl[1] >> 8); 
        p_message.data.parameter[4] = (uint8_t)light_hsl[2];
        p_message.data.parameter[5] = (uint8_t)(light_hsl[2] >> 8); 
        p_message.data.tid = message_id;
        p_message.data.parameter_lenght = 6;
        p_message.data.opcode      =  MEAH_LIGHT_OPCODE_HSL;
        if(dest_add_group)
        {
          p_message.data.opcode      =  MEAH_LIGHT_OPCODE_HSL_UNACK;
        }
        p_message.data_length      = (p_message.data.parameter_lenght + 
                                      MESH_MESSAGE_TID_LENGTH +
                                      MESH_MESSAGE_OPCODE_LENGTH);
      }
      else if(memcmp(string_array, JSON_DEVICE_CMD_TIMESTAMP, strlen(JSON_DEVICE_CMD_TIMESTAMP)) == 0)
      {
        uint32_t p_timestamp;
        s_node_1 = cJSON_GetObjectItem(m_device_node, JSON_DEVICE_CMD_VALUE);
        if(s_node_1 != NULL && s_node_1->type ==  cJSON_Number)
        {
          p_timestamp = s_node_1->valuedouble;
          update_timestamp(p_timestamp);                       
        }
        else
        {
          goto exit;
        }
        ESP_LOGI(TAG, "timestamp =%d", p_timestamp);
      }
      else if(memcmp(string_array, JSON_DEVICE_CMD_GOTA, strlen(JSON_DEVICE_CMD_GOTA)) == 0)
      {     
        s_node_1 = cJSON_GetObjectItem(m_device_node, JSON_DEVICE_CMD_VALUE);
        if(s_node_1 != NULL && s_node_1->type ==  cJSON_Object)
        { 
          cJSON *s_node_2 = NULL;
          s_node_2 = cJSON_GetObjectItem(s_node_1, JSON_DEVICE_VALUE_GOTA_VERSION);
          if(s_node_2 != NULL && s_node_2->type ==  cJSON_Number)
          { 
            uint32_t p_version;
            p_version = s_node_2->valuedouble;
            if(p_version > firmware_version)
            {
              ESP_LOGI(TAG, "firmware version = %d", p_version);
              s_node_2 = cJSON_GetObjectItem(s_node_1, JSON_DEVICE_VALUE_GOTA_URL);
              if(s_node_2 != NULL && s_node_2->type ==  cJSON_String)
              {
                memset(customer_configure.ota_info.url, 0, sizeof(customer_configure.ota_info.url));
                memcpy(customer_configure.ota_info.url, s_node_2->valuestring, strlen(s_node_2->valuestring));
                ESP_LOGI(TAG, "ota url %s", customer_configure.ota_info.url);
                ota_notify(GATEWAY_WIFI_OTA_FLAG);
              }              
            } 
          }
           
        }
        else
        {
          goto exit;
        }        
      }
      else
      {
        goto exit;
      }
    }
    else
    {
      goto exit;//error do nothing
    }
  }
  else
  {
    ESP_LOGI(TAG, "sub pack annlysis error");
    goto exit;//error do nothing
  }
  
  ESP_LOGI(TAG, "sub pack annlysis ok");
  
  message_uart_tx_queue_write(&p_message);  
  
  message_id++;

  exit:
  cJSON_Delete(json);
}
............
............
```
#OTA任务，OTA用到了https，因此，需要导入证书，证书内容如下：
```c
-----BEGIN CERTIFICATE-----
MIIFkDCCBHigAwIBAgIQC55HD0dvj5lOWTFFp8fSfTANBgkqhkiG9w0BAQsFADBu
MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3
d3cuZGlnaWNlcnQuY29tMS0wKwYDVQQDEyRFbmNyeXB0aW9uIEV2ZXJ5d2hlcmUg
RFYgVExTIENBIC0gRzEwHhcNMTkwMjIyMDAwMDAwWhcNMjAwMjIyMTIwMDAwWjAX
MRUwEwYDVQQDEwxmcmVxbWVzaC5jb20wggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAw
ggEKAoIBAQCEpOvTaeMzjLsmGfezUNR4y0z8WdMVZ9h4IRtaD4sPuhvTZ/0ATpV+
ueAMWvpeGBuwRIsj9dAgUIhAvXcK0zc3U+ixXCcUyDc60GRffSPJEgNeHR0PpPxy
ppKVg3xtODn6v8bR2mJefFGpkdTbVcyZUi69eMPDnX4tKdkzYwOXXz6XIwPkCSab
pwE2j+1E+M4v3qhxTb13sK534PQi/oYIRPfKKV0Laesp8oNZPVAqA3W1oX42eq6J
AIaDfvcrqD9zARY1bYqbu5ATDwkqfL3C3Ukd/wox2/9ckira65vk2oW2jGpu/8ss
a2bUZv4cWfQYLAzQV6s9Iem3jbx4F9Q9AgMBAAGjggJ/MIICezAfBgNVHSMEGDAW
gBRVdE+yck/1YLpQ0dfmUVyaAYca1zAdBgNVHQ4EFgQUHvoMFczlLCmZsTAi6X6e
mGOLxigwKQYDVR0RBCIwIIIMZnJlcW1lc2guY29tghB3d3cuZnJlcW1lc2guY29t
MA4GA1UdDwEB/wQEAwIFoDAdBgNVHSUEFjAUBggrBgEFBQcDAQYIKwYBBQUHAwIw
TAYDVR0gBEUwQzA3BglghkgBhv1sAQIwKjAoBggrBgEFBQcCARYcaHR0cHM6Ly93
d3cuZGlnaWNlcnQuY29tL0NQUzAIBgZngQwBAgEwfQYIKwYBBQUHAQEEcTBvMCEG
CCsGAQUFBzABhhVodHRwOi8vb2NzcC5kY29jc3AuY24wSgYIKwYBBQUHMAKGPmh0
dHA6Ly9jYWNlcnRzLmRpZ2ljZXJ0LmNvbS9FbmNyeXB0aW9uRXZlcnl3aGVyZURW
VExTQ0EtRzEuY3J0MAkGA1UdEwQCMAAwggEFBgorBgEEAdZ5AgQCBIH2BIHzAPEA
dgCkuQmQtBhYFIe7E6LMZ3AKPDWYBPkb37jjd80OyA3cEAAAAWkT8FypAAAEAwBH
MEUCIDWdSZxf5Vp4ZZfpRgVP/vWdOhffUFy7Bi9iTmOEMqhOAiEA1U9WSGpwC5c2
EqvQkFldUeWOLoxoZygqskX9yYn03pUAdwCHdb/nWXz4jEOZX73zbv9WjUdWNv9K
tWDBtOr/XqCDDwAAAWkT8F33AAAEAwBIMEYCIQDSkAl059bnkWMpYX+N4KvXUSow
zq+f0eCXLuHOJCGjsQIhAPB0c7IRsnoaVCOHp1m3x6J3MN0wMYBdKSUgAoUeao7c
MA0GCSqGSIb3DQEBCwUAA4IBAQCE7G5atq/UNaGXgJm7m8n8rPrTBuSTtU0QibLc
3Wvm5Wa+x7J5aL3++Y2ykcdiUnXZTE0ZMUjerMb/ALwCn7opwSM9Iwfv81DTfo+f
vMyhed1vqH+tTfF80KLaxdSrxocuxWCYiJ+GH7ESTT4l099iW+yxk89jepvwnk/I
t9lGvDMMXZWKcrgM7HSgx2gt+5buaPodU6YNiPJhrka2d83BhukMb7WNiIq2uIKS
nrRulHaD5DR2h/F4LlNcYQpbcTB9ywuOx9KTEd6YZDgrPbiNRjwQQGPZD64PeabG
w9GHwI3bgHjw7CdCQSayKSuWs9P/ySvQmoMxoJJwFQ+bbCCw
-----END CERTIFICATE-----
-----BEGIN CERTIFICATE-----
MIIEqjCCA5KgAwIBAgIQAnmsRYvBskWr+YBTzSybsTANBgkqhkiG9w0BAQsFADBh
MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3
d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBD
QTAeFw0xNzExMjcxMjQ2MTBaFw0yNzExMjcxMjQ2MTBaMG4xCzAJBgNVBAYTAlVT
MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxGTAXBgNVBAsTEHd3dy5kaWdpY2VydC5j
b20xLTArBgNVBAMTJEVuY3J5cHRpb24gRXZlcnl3aGVyZSBEViBUTFMgQ0EgLSBH
MTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALPeP6wkab41dyQh6mKc
oHqt3jRIxW5MDvf9QyiOR7VfFwK656es0UFiIb74N9pRntzF1UgYzDGu3ppZVMdo
lbxhm6dWS9OK/lFehKNT0OYI9aqk6F+U7cA6jxSC+iDBPXwdF4rs3KRyp3aQn6pj
pp1yr7IB6Y4zv72Ee/PlZ/6rK6InC6WpK0nPVOYR7n9iDuPe1E4IxUMBH/T33+3h
yuH3dvfgiWUOUkjdpMbyxX+XNle5uEIiyBsi4IvbcTCh8ruifCIi5mDXkZrnMT8n
wfYCV6v6kDdXkbgGRLKsR4pucbJtbKqIkUGxuZI2t7pfewKRc5nWecvDBZf3+p1M
pA8CAwEAAaOCAU8wggFLMB0GA1UdDgQWBBRVdE+yck/1YLpQ0dfmUVyaAYca1zAf
BgNVHSMEGDAWgBQD3lA1VtFMu2bwo+IbG8OXsj3RVTAOBgNVHQ8BAf8EBAMCAYYw
HQYDVR0lBBYwFAYIKwYBBQUHAwEGCCsGAQUFBwMCMBIGA1UdEwEB/wQIMAYBAf8C
AQAwNAYIKwYBBQUHAQEEKDAmMCQGCCsGAQUFBzABhhhodHRwOi8vb2NzcC5kaWdp
Y2VydC5jb20wQgYDVR0fBDswOTA3oDWgM4YxaHR0cDovL2NybDMuZGlnaWNlcnQu
Y29tL0RpZ2lDZXJ0R2xvYmFsUm9vdENBLmNybDBMBgNVHSAERTBDMDcGCWCGSAGG
/WwBAjAqMCgGCCsGAQUFBwIBFhxodHRwczovL3d3dy5kaWdpY2VydC5jb20vQ1BT
MAgGBmeBDAECATANBgkqhkiG9w0BAQsFAAOCAQEAK3Gp6/aGq7aBZsxf/oQ+TD/B
SwW3AU4ETK+GQf2kFzYZkby5SFrHdPomunx2HBzViUchGoofGgg7gHW0W3MlQAXW
M0r5LUvStcr82QDWYNPaUy4taCQmyaJ+VB+6wxHstSigOlSNF2a6vg4rgexixeiV
4YSB03Yqp2t3TeZHM9ESfkus74nQyW7pRGezj+TC44xCagCQQOzzNmzEAP2SnCrJ
sNE2DpRVMnL8J6xBRdjmOsC3N6cQuKuRXbzByVBjCqAA8t1L0I+9wXJerLPyErjy
rMKWaBFLmfK/AHNF4ZihwPGOc7w6UHczBZXH5RFzJNnww+WnKuTPI0HfnVH8lg==
-----END CERTIFICATE-----
```
https任务代码如下：

```c
static void wifi_test_task(void *pvParameters)
{
    uint32_t     dest_add;

    while(1)
    {
        dest_add = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        if(dest_add != 0)
        {
            ESP_LOGI(TAG, "enter wifi test task");

            vTaskSuspend(mqtt_client_task_handle);
            uart_disable_rx_intr(EX_UART_NUM);
            vTaskSuspend(uart0_receive_event_task_handle);
            if(network.my_socket >=0)
            {
                network.disconnect(&network);
            }

            ESP_ERROR_CHECK(esp_wifi_disconnect());      

            EventBits_t uxBits;
            wifi_config_t wifi_config;
            
            memset(&wifi_config, 0, sizeof(wifi_config));
            memcpy(wifi_config.sta.ssid, customer_configure.wifi_ssid, strlen(customer_configure.wifi_ssid));
            memcpy(wifi_config.sta.password, customer_configure.wifi_password, strlen(customer_configure.wifi_password));

            ESP_LOGI(TAG, "test SSID %s...", wifi_config.sta.ssid);
            ESP_LOGI(TAG, "test passowrd %s...", wifi_config.sta.password);
            //ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
            ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
            ESP_ERROR_CHECK(esp_wifi_connect());

            xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
            uxBits = xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT, false, true, WIFI_TEST_WAIT_TIME_MS);
            if((uxBits & CONNECTED_BIT) == CONNECTED_BIT)
            {
                flash_store_t s_flash_store;

                memset(&s_flash_store, 0, sizeof(s_flash_store));
                memcpy(s_flash_store.wifi_ssid, customer_configure.wifi_ssid, strlen(customer_configure.wifi_ssid));
                memcpy(s_flash_store.wifi_password, customer_configure.wifi_password, strlen(customer_configure.wifi_password));

                ESP_ERROR_CHECK(spi_flash_erase_sector(CUSTOM_STORAGE_START_ADDRESS / SPI_FLASH_SEC_SIZE));
                ESP_ERROR_CHECK(spi_flash_write(CUSTOM_STORAGE_START_ADDRESS, (void*)&s_flash_store, sizeof(s_flash_store))); 
                
                uint8_t test_result[1] = {0};
                give_wifi_test_rsp(WIFI_TEST_result_RSP, (uint16_t)dest_add, 0, test_result, 1);

                ESP_LOGI(TAG, "test success and store parameter");
 
            }
            else
            {
                uint8_t test_result[1] = {1};
                give_wifi_test_rsp(WIFI_TEST_result_RSP, (uint16_t)dest_add, 0, test_result, 1);
                ESP_LOGI(TAG, "wifi test fail, ssid and password");
            }
            vTaskDelay(FLASH_STORE_WAIT_TIME_MS);
            esp_restart();            
        }       
    }
}
```
#wifi测试任务代码：
<br>```c
static void wifi_test_task(void *pvParameters)
{
    uint32_t     dest_add;

    while(1)
    {
        dest_add = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        if(dest_add != 0)
        {
            ESP_LOGI(TAG, "enter wifi test task");

            vTaskSuspend(mqtt_client_task_handle);
            uart_disable_rx_intr(EX_UART_NUM);
            vTaskSuspend(uart0_receive_event_task_handle);
            if(network.my_socket >=0)
            {
                network.disconnect(&network);
            }

            ESP_ERROR_CHECK(esp_wifi_disconnect());      

            EventBits_t uxBits;
            wifi_config_t wifi_config;
            
            memset(&wifi_config, 0, sizeof(wifi_config));
            memcpy(wifi_config.sta.ssid, customer_configure.wifi_ssid, strlen(customer_configure.wifi_ssid));
            memcpy(wifi_config.sta.password, customer_configure.wifi_password, strlen(customer_configure.wifi_password));

            ESP_LOGI(TAG, "test SSID %s...", wifi_config.sta.ssid);
            ESP_LOGI(TAG, "test passowrd %s...", wifi_config.sta.password);
            //ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
            ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
            ESP_ERROR_CHECK(esp_wifi_connect());

            xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
            uxBits = xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT, false, true, WIFI_TEST_WAIT_TIME_MS);
            if((uxBits & CONNECTED_BIT) == CONNECTED_BIT)
            {
                flash_store_t s_flash_store;

                memset(&s_flash_store, 0, sizeof(s_flash_store));
                memcpy(s_flash_store.wifi_ssid, customer_configure.wifi_ssid, strlen(customer_configure.wifi_ssid));
                memcpy(s_flash_store.wifi_password, customer_configure.wifi_password, strlen(customer_configure.wifi_password));

                ESP_ERROR_CHECK(spi_flash_erase_sector(CUSTOM_STORAGE_START_ADDRESS / SPI_FLASH_SEC_SIZE));
                ESP_ERROR_CHECK(spi_flash_write(CUSTOM_STORAGE_START_ADDRESS, (void*)&s_flash_store, sizeof(s_flash_store))); 
                
                uint8_t test_result[1] = {0};
                give_wifi_test_rsp(WIFI_TEST_result_RSP, (uint16_t)dest_add, 0, test_result, 1);

                ESP_LOGI(TAG, "test success and store parameter");
 
            }
            else
            {
                uint8_t test_result[1] = {1};
                give_wifi_test_rsp(WIFI_TEST_result_RSP, (uint16_t)dest_add, 0, test_result, 1);
                ESP_LOGI(TAG, "wifi test fail, ssid and password");
            }
            vTaskDelay(FLASH_STORE_WAIT_TIME_MS);
            esp_restart();            
        }       
    }
}

```

# 关于作者

一个不纯粹的攻城s，一个不自由的 man 。
