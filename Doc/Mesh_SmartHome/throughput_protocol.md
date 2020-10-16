[TOC]
#### 版本：

|版本号|制定日期|修改记录|author|
|:----    |:---|:----- |-----   |
|1.0.1|2019-10-21|初始化版本|Roy|

#### 迅通智能家居MESH系统Gateway-Cloud对接协议：

本文档主要描述迅通Mesh项目，云端和网关透传Mesh指令协议；


#### 域名/IP信息:

- www.freqmesh.com/47.106.164.80:1883
-  端口：1883

#### 通信方式：

- MQTT

#### MQTT publish 参数：

|参数名|是否必须|类型|说明|
|:----    |:---|:----- |-----   |
|topic |是  |string |格式："uploadData/"+Mesh Net Key   例如：" uploadData/000000096617fb1596fa73ef0d83504b" <Br>Mesh Net Key 由手机创建 BLE Mesh 网络时生成，用于标识一个BLE Mesh 网络（一个 BLE Mesh网络有且仅有一个 Mesh Net Key）同时手机在创建一个BLE Mesh 网络会将这个Mesh Net key上报服务器|
|userName |否  |string | 取决于broker，默认为"admin"  |
|password  |否  |string | 取决于broker，默认为"password"  |
|keep alive timer  |是  |ASCII | 长连接支持、默认心跳间隔为30s，单位秒。  |

#### MQTT subscribe 参数：

|参数名|是否必须|类型|说明|
|:----    |:---|:----- |-----   |
|topic |是  |string |格式：Mesh Net Key+"/downloadData" 例如："000000096617fb1596fa73ef0d83504b/downloadData" <Br>Mesh Net Key 由手机创建 BLE Mesh 网络时生成，用于标识一个BLE Mesh 网络（一个 BLE Mesh网络有且仅有一个 Mesh Net Key）同时手机在创建一个BLE Mesh 网络会将这个Mesh Net key上报服务器|
|userName |否  |string | 取决于broker，默认为"admin"  |
|password  |否  |string | 取决于broker，默认为"password"  |
|keep alive timer  |是  |ASCII | 长连接支持、默认心跳间隔为30s，单位秒。  |

#### 云端下发指令:

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
					"cmd":"mesh",
					"value":"0103c0010004000382010a"
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
|gOTA| json对象| 网关升级信息，包含verison和URL|例如：{"version":2;"url":"https://www.freqmesh.com/package/smart_home_gateway_2.bin"}|
|mesh| 字符|Mesh透传指令|开灯：0103c0010004000382010a|


#### 网关上传数据包:

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
					"cmd":"mesh",
					"value":"0103c0010004000382010a"
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
|mesh| 字符|Mesh透传指令|开灯状态：0103c0010004000382010a|
