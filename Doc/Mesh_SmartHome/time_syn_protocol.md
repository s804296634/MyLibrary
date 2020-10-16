[TOC]

### 版本

| 版本号 | 变更内容 | 作者 | 时间 |
| --- | --- | --- | --- |
| 1.0.0 | - 初始版本； | TrusBe | 2019.06.23 |

---
### 1 描述

本文档主要描述了定时器功能的相关协议总结和应用交互相关的内容，本文档主要用到了 `Mesh_Model_Specification v1.0.1.pdf` 文档 `5.2.1 Time messages` 相关内容；


### 2 功能设计

- 设备支持计时功能；

- APP 、服务器同步本地时间到设备；

- APP、服务器校准设备时间；


### 3 协议内容
#### 3.1 Time Get

获取设备当前的计时时间，返回内容 `3.3 Time Status`；

#### 3.2 Time Set

为设备配置、校准当前当地时间；

| Field | Size (bits) | Notes |
| --- | --- | --- |
| TAI Seconds | 40 | The current TAI time in seconds |
| ~~Subsecond~~ | ~~8~~ | ~~The sub-second time in units of 1/256 th second~~ |
| ~~Uncertainty~~ | ~~8~~ | ~~The estimated uncertainty in 10 millisecond steps~~ |
| ~~Time Authority~~ | ~~1~~ | ~~0 = No Time Authority, 1 = Time Authority~~ |
| ~~TAI-UTC Delta~~ | ~~15~~ | ~~Current difference between TAI and UTC in seconds~~ |
| ~~Time Zone Offset~~ | ~~8~~ | ~~The local time zone offset in 15-minute increments~~ |

**注意：**
> -  时间配置、校准指令使用 UTC 时间戳；
> - 因为该指令设置较为复杂，而我们需要的功能暂时用不到其他参数，所以除了第一个时间戳参数，其他参数备用，可以置空；


#### 3.3 Time Status

获取设备当前时间信息返回，内容为：

| Field | Size (bits) | Notes |
| --- | --- | --- |
| TAI Seconds | 40 | The current TAI time in seconds |
| ~~Subsecond~~ | ~~8~~ | ~~The sub-second time in units of 1/256 th second~~ |
| ~~Uncertainty~~ | ~~8~~ | ~~The estimated uncertainty in 10 millisecond steps~~ |
| ~~Time Authority~~ | ~~1~~ | ~~0 = No Time Authority, 1 = Time Authority~~ |
| ~~TAI-UTC Delta~~ | ~~15~~ | ~~Current difference between TAI and UTC in seconds~~ |
| ~~Time Zone Offset~~ | ~~8~~ | ~~The local time zone offset in 15-minute increments~~ |

**注意：**
> -  时间内容使用 UTC 时间戳；
> - 该指令只用到了时间戳参数，所以只需检测时间戳即可；


---
### 免责申明
深圳市蓝科迅通科技有限公司随附提供的软件或文档资料旨在提供给您（本公司的客户） 使用，仅限于且只能在本公司销售的产品上使用。

该软件或文档资料为本公司所有，并受适用的版权法保护。版权所有。如有违反，将面临 相关适用法律的刑事制裁，并承担违背此许可的条款和条件的民事责任。

本公司保留在不通知读者的情况下，修改文档或者软件相关内容的权利，对于使用中出现 的任何效果，本公司不承担任何责任。
