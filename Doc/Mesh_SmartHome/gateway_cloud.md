[TOC]

#### 迅通智能家居MESH系统Gateway-Cloud对接协议：

- This document describes the protocol of the communication between xunTong gateway and a server,it is MQTT that net protocol will be  used in the gateway. For MQTT  publish and subscribe pack,a cJSON format will be used.The MQTT publish is used to report a device status in a BLE Mesh net  with some special MQTT topics,and the report process is gateway active , the server  can subscribe those special topics to get device the devices status;  also, the gateway will subscribe special MQTT topic to complete  what  the server(the web client)control the device in a BLE Mesh net.For MQTT topic,there are regular rules.See below for more details.

#### 接口信息：

|版本号|制定日期|修改记录|author|
|:----    |:---|:----- |-----   |
|0.0.1 |2019-01-10 |Initial release | Hurry |
|0.0.2 |2019-01-24 |fix some error of describing | Hurry |
|0.0.3 |2019-02-15 |修改publish以及subscribe中的topic参数中增加UserID  | Roy |
|0.0.4 |2019-02-18 |去掉了publish以及subscribe中topic的userID字段，修改subscribe的topic | Hurry |
|0.0.5|2019-02-18 |修改开关部分开关项的参数为cJson布尔变量 | Hurry |
|0.0.6|2019-02-25 |1.修改uniqAddress为int类型 <br> 2.修改uAddress为uniqAddress| Roy |
|0.0.7|2019-02-28 |1.修改设备请求和下发数据的数据包格式：增加cmd（指令类型）和value（指令下发的值）| Roy |
|0.0.8|2019-02-28 |1.修改firmVersion为cmd中的一项| Hurry|
|0.0.9|2019-03-01 |1.增加设备的health项| Hurry|
|0.1.0|2019-03-05|1.更正CTL数组元素<br>2.增加一个cmd 时间戳timeStamp用于请求服务器时间<br>3.删除lowPower,因为health中包含了低电量警告| Hurry|
|0.1.1|2019-03-25|1.增加网关OTA升级CMD： gOTA | Roy|
|0.1.2|2019-03-26|1.修复一些描述错误| Hurry|
|0.1.3|2019-03-26|1.增加心率cmd描述| Hurry|
|0.1.4|2019-03-28|1.增加心率新用户cmd（newUser）描述| Roy|
|0.1.5|2019-03-29|1.增加温度cmd<br>2.增加湿度cmd|Hurry|
|0.1.6|2019-03-29|1.增加门磁cmd|Hurry|
|0.1.7|2019-04-02|1.修改门磁cmd  true为关闭，false 为开启|Roy|

#### 域名/IP信息:

- www.freqmesh.com/47.106.164.80:1883
-  端口：1883

#### 通信方式：

- MQTT

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

#### light publish  and subscribe pack:

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
#### 参数说明:

|参数名|类型|说明|
|:-----  |:-----|-----                           |
|packId | ASCII字符串  |包ID，累加1，取之范围0---4294967295，溢出后从0开始计数 |
|uniqAddress | string  |设备UnicatAddr |
|cmd | string  | 指令类型|
|value |布尔值、String、[]、int   | 指令值|

#### cmd类型说明:
|cmd类型|类型|说明|value|
|:-----  |:-----|:-----|-----
|firmVersion | int数组  | 软件版本、硬件版本、model版本|
|power| cJSON 布尔值|true表示上电，false掉电|true、false|
|health| int|表示设备健康状态 |0表示设备正常工作；1表示设备低电量警告；2表示低电量错误，即将无法工作，其它类型错误见Mesh_v1.0.pdf 的4.2.15.1章节中的表格4.21所示|
|status| cJSON 布尔值|true表示亮灯，false灭灯 |true、false|
|lighteness| ASCII字符串| 灯的亮度|80|
|HSL| ASCII字符串| 灯的色彩 |[100,100,100]|
|CTL| ASCII字符串| 灯的色温|[100,100]|
|scene| string| 场景||
|timestamp| int| 服务器时间戳|服务器收到该cmd后，应该pub一个带有服务器时间戳的value给网关，网关pub这个cmd时，value为空，value的单位为s|
|gOTA| json对象| 网关升级信息，包含verison和URL|例如：{"version":2;"url":"https://www.freqmesh.com/package/smart_home_gateway_2.bin"}|
|BCG| json对象| 心率模组数据：{"HR":60,"RR":45,"SV":60,"HRV":60,"signal_strength":200,"status":1,"B2B":16,"B2B1":16,"B2B2":16}|
|newUser| 布尔值| true|
|temperature| int| 温度，单位为0.01摄氏度|
|humidity| int| 湿度，单位为0.01|
|doorMagnetic| bool|门磁传感器， true为关闭，false 为开启|
|log| 字符串|{"reset":1}1表示正常复位,0表示异常复位|

#### socket  pub  and subscribe pack:

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
#### 参数说明:

|参数名|类型|说明|
|:-----  |:-----|-----                           |
|packId | ASCII字符串  |包ID，累加1，取之范围0---4294967295，溢出后从0开始计数 |
|uniqAddress | string  |设备UnicatAddr |
|cmd | string  | 指令类型|
|value |布尔值、String、[]、int   | 指令值|

#### cmd类型说明:
|cmd类型|类型|说明|value|
|:-----  |:-----|:-----|-----
|firmVersion | int数组  | 软件版本、硬件版本、model版本|
|power| cJSON 布尔值|true表示上电，false掉电|true、false|
|health| int|表示设备健康状态 |0表示设备正常工作；1表示设备低电量警告；2表示低电量错误，即将无法工作，其它类型错误见Mesh_v1.0.pdf 的4.2.15.1章节中的表格4.21所示|
|status| cJSON 布尔值|true表示亮灯，false灭灯 |true、false|
|scene| string| 场景||
|timestamp| int| 服务器时间戳|服务器收到该cmd后，应该pub一个带有服务器时间戳的value给网关，网关pub这个cmd时，value为空，value的单位为s|


#### sensor pub  and subscribe pack:

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
#### 参数说明:
|参数名|类型|说明|
|:-----  |:-----|-----                           |
|packId | ASCII字符串  |包ID，累加1，取之范围0---4294967295，溢出后从0开始计数 |
|uniqAddress | string  |设备UnicatAddr |
|cmd | string  | 指令类型|
|value |布尔值、String、[]、int   | 指令值|

#### cmd类型说明:
|cmd类型|类型|说明|value|
|:-----  |:-----|:-----|-----
|firmVersion | int数组  | 软件版本、硬件版本、model版本|
|power| cJSON 布尔值|true表示上电，false掉电|true、false|
|health| int|表示设备健康状态 |0表示设备正常工作；1表示设备低电量警告；2表示低电量错误，即将无法工作，其它类型错误见Mesh_v1.0.pdf 的4.2.15.1章节中的表格4.21所示|
|status| cJSON 布尔值|true表示亮灯，false灭灯 |true、false|
|parameter| string| 传感器参数,可能是温湿度也可能是报警信息等|
|scene| string| 场景||
|timestamp| int| 服务器时间戳|服务器收到该cmd后，应该pub一个带有服务器时间戳的value给网关，网关pub这个cmd时，value为空，value的单位为s|

#### switch pub  and subscribe pack:

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
#### 参数说明:

|参数名|类型|说明|
|:-----  |:-----|-----                           |
|packId | ASCII字符串  |包ID，累加1，取之范围0---4294967295，溢出后从0开始计数 |
|uniqAddress | string  |设备UnicatAddr |
|cmd | string  | 指令类型|
|value |布尔值、String、[]、int   | 指令值|

#### cmd类型说明:
|cmd类型|类型|说明|value|
|:-----  |:-----|:-----|-----
|firmVersion | int数组  | 软件版本、硬件版本、model版本|
|power| cJSON 布尔值|true表示上电，false掉电|true、false|
|health| int|表示设备健康状态 |0表示设备正常工作；1表示设备低电量警告；2表示低电量错误，即将无法工作，其它类型错误见Mesh_v1.0.pdf 的4.2.15.1章节中的表格4.21所示|
|status| cJSON 布尔值|true表示亮灯，false灭灯 |true、false|
|parameter| string| 传感器参数,可能是温湿度也可能是报警信息等|
|scene| string| 场景||
|timestamp| int| 服务器时间戳|服务器收到该cmd后，应该pub一个带有服务器时间戳的value给网关，网关pub这个cmd时，value为空，value的单位为s|

####  lowPDevice pub  and subscribe pack:

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

#### 参数说明:

|参数名|类型|说明|
|:-----  |:-----|-----                           |
|packId | ASCII字符串  |包ID，累加1，取之范围0---4294967295，溢出后从0开始计数 |
|uniqAddress | string  |设备UnicatAddr |
|cmd | string  | 指令类型|
|value |布尔值、String、[]、int   | 指令值|

#### cmd类型说明:
|cmd类型|类型|说明|value|
|:-----  |:-----|:-----|-----
|firmVersion | int数组  | 软件版本、硬件版本、model版本|
|power| cJSON 布尔值|true表示上电，false掉电|true、false|
|health| int|表示设备健康状态 |0表示设备正常工作；1表示设备低电量警告；2表示低电量错误，即将无法工作，其它类型错误见Mesh_v1.0.pdf 的4.2.15.1章节中的表格4.21所示|
|status| cJSON 布尔值|true表示亮灯，false灭灯 |true、false|
|parameter| string| 传感器参数,可能是温湿度也可能是报警信息等|
|scene| string| 场景||
|timestamp| int| 服务器时间戳|服务器收到该cmd后，应该pub一个带有服务器时间戳的value给网关，网关pub这个cmd时，value为空，value的单位为s||
#### 备注:
