[TOC]

### 版本

| 版本号 | 变更内容 | 作者 | 时间       |
| ------ | -------- | ---- | ---------- |
| 1.0.0  | 初始版本 | HB   | 2020.11.06 |



### 简述

- esp32 通用网关用于扫描网关附近的蓝牙设备，将其**广播包数据** 和 **rssi**  值上传到 云端服务器。



### 计划

1. 理解和掌握 esp32 的编译系统，make 和 Cmake 的使用方法，能够新建项目。
2. 掌握 esp32 BLE的主机例程，实现扫描并打印附近 BLE 设备的 mac 和 广播包。
3. 掌握 BLE 从机例程，实现一个**自定义透传服务**，服务中包含 **Tx** 和 **Rx**特性
4. 实现主从一体，合并 以上两条的功能。
5. 掌握 wifi 的例程，能连接上手机热点或附近的 WiFi。
6. 掌握 HTTP 、MQTT 的例程，能通过网络协议上传数据到云端。
7. 掌握 esp32 OTA ，能实现空中升级。
8. 了解 esp32  的flash布局，掌握 flash 操作相关API ，能保存 配置数据掉电不丢失。
9. 制定手机和网关的通讯协议。
10. 制定网关和云端的通讯协议。



### 云端上报格式

- JSON 包格式

```c
beacons=[
  {
   "devicename":"xBeacon",
   "type":"ibeacon",
   "devicemac":"D1010800012E",
   "gatewaymac":"D11215000109",
   "rssi":-64,
   "data1":"0201061AFF4C000215FDA50693A4E24FB1AFCFC6EB0764782527723D07BB",
   "data2":"080978426561636F6E14162915500B11012C0131D1010800012E27723D07"
   }
]

[
{"devicename":"",
"type":"rsp",
"dMac":"CF0821000003",
"gMac":"D11215000109",
"rssi":"-88",
"data1":"",
"data2":""},
{"devicename":"",
86",data1":"0201061AFF4C000215FDA50693A4E24FB1AFCFC6EB0764782527723CDCBB",
"data2":""},

{"devicename":"",
"type":"ibeacon",
"dMac":"D2110600055E","gMac":"D11215000109","rssi":"-90",
"data1":"0201061AFF4C000215A000FAA00047005A00526D6F62696B65C6430F67BB","data2":""},
{"devicename":"","type":"ibeacon",
"dMac":"D2110600055A",
"gMac":"D11215000109",
"rssi":"-81",
"data1":"0201061AFF4C000215A000FAA00047005A00526D6F62696B65C6430F63BB","data2":""},

{"devicename":"",
"type":"ibeacon",
"dMac":"D2082400022B",
"gMac":"D11215000109",
"rssi":"-85",
"data1":"0201061AFF4C000215A000FAA00047005A00526D6F62696B65C6430A03BB",
"data2":""
}
]

```



