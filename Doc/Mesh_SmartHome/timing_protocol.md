[TOC]

### 版本

| 版本号 | 变更内容 | 作者 | 时间 |
| --- | --- | --- | --- |
| 1.0.0 | - 初始版本； | TrusBe | 2019.07.23 |

---


### 描述

本文档主要描述了定时器功能的相关协议总结和应用交互相关的内容，本文档主要用到了 `Mesh_Model_Specification v1.0.pdf` 文档 `5.1.4.2 Schedule Register` 和 `5.2.3 Scheduler messages` 相关内容；

### 协议内容
- 主体结构请参考 `5.2.3.4 Scheduler Action Set`，`5.2.3.5 Scheduler Action Set Unacknowledged` ，`5.2.3.6 Scheduler Action Status`；

| Field | Size(bits) | Notes |
| --- | --- | --- |
| Index | 4 | - Index of the Schedule Register entry to set；</br>- `Index` 字段的有效值为 `0x00-0x0F`； </br>- `Index` 字段 `0x00` 保留，范围：`0x01-0x0F`；|
| Schedule Register | 76 | Bit field defining an entry in the Schedule Register |

- 定时参数结构（`Schedule Register`）；

| Name | Size(bits) | Notes |
| --- | --- | --- |
| Year | 7 | Scheduled year for the action (see Table 5.5) |
| Month | 12 | Scheduled month for the action (see Table 5.6) |
| Day | 5 | Scheduled day of the month for the action (see Table 5.7) |
| Hour | 5 | Scheduled hour for the action (see Table 5.8) |
| Minute | 6 | Scheduled minute for the action (see Table 5.9) |
| Second | 6 | Scheduled second for the action (see Table 5.10) |
| DayOfWeek | 7 | Scheduled days of the week for the action (see Table 5.11) |
| Action | 4 | Action to be performed at the scheduled time (see Table 5.12) |
| Transition Time | 8 | Transition time for this action (see Table 3.1.3) |
| Scene Number | 16 | Scene number to be used for some actions (see Table 5.13) |

- `Year` 参数规则

| Value | Description |
| --- | --- |
| 0x00–0x63 | 2 least significant digits of the year |
| 0x64 | Any year |
| All other values | Prohibited |

- `Month` 参数规则

| Bit | Description |
| --- | --- |
| 0 | Scheduled in January |
| 1 | Scheduled in February |
| 2 | Scheduled in March |
| 3 | Scheduled in April |
| 4 | Scheduled in May |
| 5 | Scheduled in June |
| 6 | Scheduled in July |
| 7 | Scheduled in August |
| 8 | Scheduled in September |
| 9 | Scheduled in October |
| 10 | Scheduled in November |
| 11 | Scheduled in December |

- `Day ` 参数规则

| Value | Description |
| --- | --- |
| 0x00 | Any day |
| 0x01–0x1F | Day of the month |

- `Hour  ` 参数规则

| Value | Description |
| --- | --- |
| 0x00–0x17 | Hour of the day (00 to 23 hours) |
| 0x18 | Any hour of the day |
| 0x19 | Once a day (at a random hour) |
| All other values | Prohibited |

- `Minute  ` 参数规则

| Value | Description |
| --- | --- |
| 0x00–0x3B | Minute of the hour (00 to 59) |
| 0x3C | Any minute of the hour |
| 0x3D | Every 15 minutes (minute modulo 15 is 0) (0, 15, 30, 45) |
| 0x3E | Every 20 minutes (minute modulo 20 is 0) (0, 20, 40) |
| 0x3F | Once an hour (at a random minute) |

- `Second  ` 参数规则

| Value | Description |
| --- | --- |
| 0x00–0x3B | Second of the minute (00 to 59) |
| 0x3C | Any second of the minute |
| 0x3D | Every 15 seconds (minute modulo 15 is 0) (0, 15, 30, 45) |
| 0x3E | Every 20 seconds (minute modulo 20 is 0) (0, 20, 40) |
| 0x3F | Once an minute (at a random second) |

- `DayOfWeek` 参数规则

| Bit | Description |
| --- | --- |
| 0 | Scheduled on Mondays |
| 1 | Scheduled on Tuesdays |
| 2 | Scheduled on Wednesdays |
| 3 | Scheduled on Thursdays |
| 4 | Scheduled on Fridays |
| 5 | Scheduled on Saturdays |
| 6 | Scheduled on Sundays |

- `Action  ` 参数规则

| Value | Description |
| --- | --- |
| 0x0 | Turn Off |
| 0x1 | Turn On |
| 0x2 | Scene Recall |
| 0xF | No action |
| All other values | Reserved for Future Use |

- `Scene Number ` 参数规则

| Value | Description |
| --- | --- |
| 0x0000 | No scene |
| All other values | Scene number |


### 举例

![](/uploads/xt_mesh/images/m_af51e79a998401dd3f9cb0806614aa68_r.png)


### 定时

- 定时参考需求文档和协议文档，只做修改 `Hour`、`Minute`、`DayOfWeek`、`Action`、`Scene Number`参数；

- 时间范围为 24 小时内，时间精度为分钟，秒钟默认设置为 `0x00`;

- 针对单个设备的定时，事件 `Action` 参数可以执行开、关；

- 针对群组的定时，事件 `Action` 参数可以执行开、关；

- 针对全局的定时，事件 `Action` 参数可以执行场景唤醒；

- 在事件 `Action` 参数为场景唤醒时，只能选择已经配置好的场景；


### 倒计时

- 倒计时参考需求文档和协议文档，只做修改 `Hour`、`Minute`、`Action`、`Scene Number`参数；

- 时间范围为 1 小时内，时间精度为分钟，秒钟默认设置为 `0x00`;

- 用户选择倒计时间长度，然后将时间长度增加到当前时间的基础上，产生新的定时时间；

- 针对单个设备的定时，事件 `Action` 参数都可以使用，也就是说可以执行开、关；

- 针对群组的定时，事件 `Action` 参数可以执行开、关；

- 针对场景的定时，事件 `Action` 参数可以执行场景唤醒；

- 在事件 `Action` 参数为场景唤醒时，只能选择已经配置好的场景；


---

