[TOC]

### 版本

| 版本号 | 变更内容 | 作者 | 时间 |
| --- | --- | --- | --- |
| 1.0.0 | - 首次发布； | Harden | 2019.01.24 |
| 1.0.1 | - 修改使用实例； | Harden | 2019.02.14 |
| 1.0.2 | - 将3.3透传状态包结构中的tid删掉，原因：`get message` 里面没有 `tid`，`scene setup model` 里面的 `store` 和`delete Message` 也没有 `tid`，因此难以对应；</br>- 修改4 `Message` 里面的内容； | Harden | 2019.02.21 |
| 1.1.0 | - 增加 3.1 命令包指令用于获取 `node address` 和 `netkey`； | Harden | 2019.03.06 |
| 1.1.1 | - 移植 Mindoc； | TrusBe | 2019.07.22 |


---


### DEVICE UUID格式

*表7 DEVICE UUID 格式*

| Field | Size（ Octets） | Notes |
| --- | --- | --- |
| CID | 2 | 公司 ID,设置为 `0x0059`：Nordic 或者其它 SIG Reserved 的 ID |
| PID | 1 | Bit 0-4：蓝牙广播包版本号；</br>Bit 5：是否支持 OTA ；</br>Bit 6-7 : `00` BLE4.0， `01` BLE4.2， `10` BLE5， `11` BLE5，以上默认是`01` |
| Device type | 2 | 我们自己制定，一型一号 |
| Projrct id | 2 | 关联我们上传 SVN 的项目的编号 |
| MAC地址 | 6 | 我们自己制定，一机一号 |
| VID | 2 | 软件版本号 |
| RFU | 1 | Reserved for future use |


### VENDOR MODEL

蓝牙 Mesh 网络是 Message 驱动的网络，因此蓝牙 Mesh 协议对各种常用的消息都有规定，比如 Generic OnOff 消息， Sensor 消息， Light Lighting 消息和 Time&Scene 消息， 但是由于蓝牙 Mesh 协议不可能覆盖所有类型的消息，因此需要厂家自定义消息。 蓝牙 Mesh 消息是加载在 Mesh 网络的 Access Lay，格式如下图所示：

*表1 Mesh Acess Payload 格式*

| Field Name | Size（ Octets） | Notes |
| --- | --- | --- |
| Opcode | 1，2 or 3 | Operation Code |
| Parameters | 0 to 379 | Application params |

其中 Opcode 是 1 或 2 字节的消息是 SIG 标准消息，Opcode=3 字节的是留给厂家自定义的消息， 其中第一字节的 bit7 和 bit6 是 0b11， bit5-bit0 表达 Company Identifier 对应的可以处理的消息数量，最多 64 个， 第二和第三字节是 SIG 分配的 Company Identifier（ 默认： `0xFFFE`）。 由于自定义消息常常需要指定给自定义的Model，因此这里也会使用 32bits 的 Vendor Model ID。

Mesh 协议没有完全覆盖所有的 MESSAGES， 为了简化问题， 我们会定义一组 Vendor

Model ID（ Server： `0x00591000` 和 Client： `0x00591001`） 和一组 Opcode 来处理情况， **关于 Vendor Model的具体定义参考《 迅通智能家居设备蓝牙mesh扩展协议》**

### 蓝牙 Mesh 模块串口透传

#### 命令包结构

**MCU发给模块的命令**

| 字段 | 长度 | 备注 |
| --- | --- | --- |
| Type | 1字节 | Bit7-Bit4： 版本号，默认 0000</br>Bit3-Bit0：类型， 0000-命令 |
| Cmd | 1字节 | 0x00：读取模块状态</br>0x01：设置 Device UUID</br>0x02：启动联网</br>0x03：获取 node address</br>0x04：获取 netkey</br>0x05：设置波特率(预留)</br>0x06：设置 relay feature(预留)</br>0x07：设置 proxy feature(预留)</br>0x08： |
| Lenth | 2字节 | Value 的长度 |
| Value | Variable | Cmd=0x01, Value=16 字节的 Device UUID</br>Cmd=其他， Value 字段不存在 |



**模块回复给MCU的状态**

| 字段 | 长度 | 备注 |
| --- | --- | --- |
| Type | 1字节 | Bit7-Bit4： 版本号，默认 0000</br>Bit3-Bit0：类型， 0000-命令 |
| Cmd | 1字节 | 0x00： 模块状态</br>0x01：设置 Device UUID 状态</br>0x02： 联网状态 |
| Value | n字节 | Cmd=0x00， Value=0x00，状态正常， Value=其他，状态异常</br>Cmd=0x01 ， Value=0x00 ， 成 功 ，</br>Value=0x01， 失败</br>Cmd=0x02 ， Value=0x00，联网成功，</br>Value=0x01， 联网失败 Cmd=0x03 ， Value=node address（2字节，小端），</br>Value=0x0， 没有配网 Cmd=0x04 ， Value=netkey（16字节，大端），</br>Value=0x0， 没有配网   |

#### 透传包结构

模块联网后，外部 MCU 可以发送消息给模块，模块去控制Mesh网络的其它设备，同样，模块收到消息的响应后会把消息打包送给外部 MCU。Transparent PDU 包结构如下表所示：

| 字段 | 长度 | 备注 |
| --- | --- | --- |
| Type | 1字节 | Bit7-Bit4:版本号，默认0000</br>Bit3-Bit0:类型，0001-消息 |
| Destination address | 2字节 | 接收方地址 |
| Source address | 2字节 | 发送方地址 |
| Data length | 2字节 | Data字段的长度 |
| Data | Variable | Data字段的格式根据具体的 Message 定义决定，一般是OpCode和Parameters的结构 |

Transparent PDU 包的 Data 字段结构

| 字段 | 长度 | 备注 |
| --- | --- | --- |
| OpCode | 2字节（uint16\_t） | 消息类型，</br>1）0XXXXXXX： 1 字节消息（不包含 01111111） 比如 Scheduler Action Set 消息是 `0x60`；</br>2）10XXXXXX XXXXXXXX： 2 字节消息， 比如 Generic OnOff Set 消息是 `0x82 02`</br>3）11XXXXXX XXXXXXXX XXXXXXXX： 3 字节消息，比如 Vendor Message Get 消息是 0xC0FFFE不足两个字节补0，例如0x0060;三个字节的opcode,其中Company ID去掉，剩余一个字节补0，例如0xC0FFFE变为0x00C0 |
| Parameters | Variable | 消息参数 |

- 各个字段以小端模式（ LSB 在前）收发。
- 外部 MCU 在不知道自身 Source address 的情况下，应在对应字段填 `0x0000`，之后由模块补上。

#### 透传状态包结构
MCU 在向 Mesh 模块发送透传包时，模块响应对应透传包的状态，例如是否接收到，参数是否正确，该透传包是否超时了也没起作用等等。Transparent PDU Status 包结构如下表所示：

| 字段 | 长度 | 备注 |
| --- | --- | --- |
| Type | 1字节 | Bit7-Bit4:版本号，默认 0000</br>Bit3-Bit0:类型，0002-透传包状态 |
| ~~TID~~ | ~~1字节~~ | ~~跟发送的透传包里面的TID相对应~~ |
| OpCode | 2字节（uint16\_t） | 消息类型，</br>1）0XXXXXXX： 1 字节消息（不包含 01111111） 比如 Scheduler Action Set 消息是 `0x60` </br>2）10XXXXXX XXXXXXXX： 2 字节消息， 比如 Generic OnOff Set消息是 0x82 02 </br>3）11XXXXXX XXXXXXXX XXXXXXXX： 3 字节消息，比如 Vendor Message Get 消息是 0xC0FFFE不足两个字节补0，例如 0x0060;三个字节的 opcode ,其中 Company ID 去掉，剩余一个字节 0 ，例如 0xC00059 变为 0x00C0 |
| Status Code | 1字节（uint8\_t） | 具体定义参考《迅通智能家居设备蓝牙 mesh 扩展协议》4.4 错误码定义 |

### MESSAGES

#### Generic OnOff Server

##### Generic OnOff Get
没有参数

##### Generic OnOff Set
Generic OnOff Set消息结构如下图：

| Field | Size(octets) | Notes |
| --- | --- | --- |
| OnOff | 1 | The target value of the Generic OnOff state |
| TID | 1 | Transaction Identifier |
| Transition Time | 1 | Format as defined in Section 9. (Optional) |
| Delay | 1 | Message execution delay in 5 millisecond steps (C.1) |

C.1: 如果 Transition Time 字段存在, the Delay 字段也应存在; 否则这两个字段都不用存在。

##### Generic OnOff Set Unacknowledged
Generic OnOff Set Unacknowledged 消息结构如下图：

| Field | Size(octets) | Notes |
| --- | --- | --- |
| OnOff | 1 | The target value of the Generic OnOff state |
| TID | 1 | Transaction Identifier |
| Transition Time | 1 | Format as defined in Section 9. (Optional) |
| Delay | 1 | Message execution delay in 5 millisecond steps (C.1) |

C.1: 如果 Transition Time 字段存在, Delay 字段也应存在; 否则这两个字段都不用存在。

##### Generic OnOff Status
Generic OnOff Set Status 消息结构如下图：

| Field | Size(octets) | Notes |
| --- | --- | --- |
| Present OnOff | 1 | The present value of the Generic OnOff state. |
| Target OnOff | 1 | The target value of the Generic OnOff state (optional). |
| Remaining Time | 1 | Format as defined in Section 9. (C.1) |

C.1: 如果 Target OnOff 字段存在, Remaining Time 字段也应存在; 否则这两个字段都不用存在。


#### Light Lightness Server

##### Light Lightness Get
没有参数

##### Light Lightness Set

Light Lightness Set 消息结构如下图：

| Field | Size(octets) | Notes |
| --- | --- | --- |
| Lightness | 2 | The target value of the Light Lightness Actual states |
| TID | 1 | Transaction Identifier |
| Transition Time | 1 | Format as defined in Section 9. (Optional) |
| Delay | 1 | Message execution delay in 5 millisecond steps.(C.1) |

C.1: 如果Transition Time 字段存在, Delay 字段也应存在; 否则这两个字段都不用存在。

##### Light Lightness Set Unacknowledged

Light Lightness Set Unacknowledged 消息结构如下图：

| Field | Size(octets) | Notes |
| --- | --- | --- |
| Lightness | 2 | The target value of the Light Lightness Actual states |
| TID | 1 | Transaction Identifier |
| Transition Time | 1 | Format as defined in Section 9. (Optional) |
| Delay | 1 | Message execution delay in 5 millisecond steps.(C.1) |

C.1: 如果Transition Time 字段存在, Delay 字段也应存在; 否则这两个字段都不用存在。

##### Light Lightness Status

Light Lightness Status 消息结构如下图：

| **Field** | Size(octets) | Notes |
| --- | --- | --- |
| Present Lightness | 2 | The present value of the Light Lightness Actual state. |
| Target Lightness | 2 | The target value of the Light Lightness Actual state. (Optional) |
| Remaining Time | 1 | Format as defined in Section 9. (C.1) |

C.1: 如果 Target Lightness 字段存在,  Delay 字段也应存在; 否则这两个字段都不用存在。

> 其它所用到的Model及对应的Messages参考《Mesh Model》:
>《Mesh Model》4.2 Sensor messages**
>《Mesh Model》6.3 Lighting messages**
>《Mesh Model》5.2.2 Scene messages**


### 消息流和消息序列图举例

#### 消息流

Get message消息流：

> 1. 一个需要得到确认的 get message 发出;
> 2. 一个相关的 Status message 返回给发起人;

Set message消息流：

> 1. 一个 Set message 发出;
> 2. 收到消息，更新状态;
> 3. 如果在步骤(1)中 Set message 需要确认，一个相应的 status message 返回给发起人;

##### 状态异步变化或者收到一个非确认的消息

当状态异步地改变，一个非确认的 Status 消息会发布到对应 Model 的发布地址。

##### 当状态被一个改变状态的可靠的消息改变

当状态被一个改变状态的可靠的消息改变，一个相应的无需确认的的 Status message 会返回给发起者。此外，另一个同样的无需确认的的 Status message 会发给对应 model 的发布地址，以此通知潜在的订阅者状态已经改变。

##### 根据状态改变的情况发布消息

如果一个 model 支持发布并且发布地址有效，它需要根据状态改变的情况发布消息，规则如下：

- 一个恰当的 Status 消息应该在状态转换结束后立即予以发布。

- 如果转换时间大于等于 `2s`，建议在开始转换的1s之内发布一个额外的 Status 消息。

- 状态转换结束后，一个元素应当定期发布 Status 信息。发布周期是根据 Model;

Publication Set message 的 Model publish Period 状态来设定。推荐的发布周期为 `30s`。`0` 值意味着不用周期性发布消息。

注意：当状态转换涉及多个束缚状态，由实现者决定发布哪一个最恰当的 Status 消息。推荐使用能包括被改变状态的完整信息的单条消息。

#### 消息序列图举例

本节显示了一些示例消息序列图（MSCs）。此处模块作为 client 的角色。

##### Generic OnOff Get

下面的消息序列图显示了一个客户端使用需要确认的Generic OnOff Get消息获得对等元素的状态。服务器以一个关联的Generic OnOff Statu的消息响应。
<div align="center"><img width="500" height="150" src="http://192.168.1.7:8181/uploads/xt_mesh/images/m_db8724333fe00d3bda204cb9907ea927_r.png"></div>

##### Generic OnOff Set

下面的消息序列图显示了一个客户端使用需要确认的Generic OnOff Set消息设置对等元素的状态。服务器以一个关联的Generic OnOff Statu消息响应，同时向配置为model的发布地址的组地址发布一个Generic OnOff Statu消息。
<div align="center"><img width="500" height="150" src="http://192.168.1.7:8181/uploads/xt_mesh/images/m_f6859ab238ddcb269a972e2e371a9664_r.png"></div>

##### Generic OnOff Set Unacknowledged

下面的消息序列图显示了一个客户端设置对等元素的状态。没有消息回应给发起者，但是服务器会向model的发布地址发布一个Status消息。
<div align="center"><img width="500" height="150" src="http://192.168.1.7:8181/uploads/xt_mesh/images/m_f31722bdebcbf6eb4b6f5113f81443b7_r.png"></div>


### 摘要信息

#### Messages 摘要

<div align="center"><img width="600" height="750" src="http://192.168.1.7:8181/uploads/xt_mesh/images/m_d2cdba8f5d5ad00bbbc901b1f2648b77_r.png"></div>

#### Model ID

| Model Name | SIG Model ID |
| --- | --- |
| Generic OnOff Server | 0x1000 |
| Generic OnOff Client | 0x1001 |
| Light Lightness Server | 0x1300 |
| Light Lightness Client | 0x1302 |
| Light CTL Server | 0x1303 |
| Light CTL Client | 0x1305 |
| Light HSL Server | 0x1307 |
| Light HSL Client | 0x1309 |
| Sensor Server | 0x1100 |
| Sensor Setup Server | 0x1101 |
| Sensor Client        | 0x1102 |
| Scene Server | 0x1203 |
| Scene Setup Server | 0x1204 |
| Scene Client | 0x1205 |


### Transition Time

Transition Time格式：

| Field | Size(bits) | Definition |
| --- | --- | --- |
| Transition Number of Steps | 6 | The number of Steps |
| Transition Step Resolution | 2 | The resolution of the Transition Number of Steps field |

- 当选择 `100ms` 的步进分辨率，时间的范围为 `0` 到 `6.2s`；

- 当选择 `1s` 的步进分辨率，时间的范围为 `0` 到 `62s`；

- 当选择 `10s` 的步进分辨率，时间的范围为 `0` 到 `620s(10.5minutes)`；

- 当选择 `10min` 的步进分辨率，时间的范围为 `0` 到 `620 minutes(10.5hours)`；

Transition Time的计算公式：

`Transition Time = Transition Step Resolution * Transition Number of Steps`


#### Transition Step Resolution

下表描述了Value的值所代表的具体时间。

| Value | Description |
| --- | --- |
| 0b00 | The Transition Step Resolution is 100 milliseconds |
| 0b01 | The Transition Step Resolution is 1 second |
| 0b10 | The Transition Step Resolution is 10 seconds |
| 0b11 | The Transition Step Resolution is 10 minutes |


#### Transition Number of Steps

下表描述了Value的值所代表的具体步进。

| **Value** | **Description** |
| --- | --- |
| 0x00 | The Generic Default Transition Time is immediate. |
| 0x01–0x3E | The number of steps. |
| 0x3F | The value is unknown. The state cannot be set to this value, but an element may report an unknown value if a transition is higher than 0x3E or not determined. |


### 使用实例

#### Generic OnOff Set消息开灯

- 外部MCU通过串口给模块发送透传包：

```
01 0400 fe7f 0400 0282 0100
Type                    :01
Destination address     :0400
Source address          :fe7f
Data length             :0400
Opcode                  :0282
Params:                 :0100
```
Mesh模块收到后，向dest addr(0x0004)发送Generic OnOff Set消息;

Dest addr设备回应Generic OnOff Status消息

- Mesh模块收到消息后，模块通过串口给MCU发送透传包：

```
01 0100 0400 0300 0482 01
Type                    :01
Destination address     :0100
Source address          :0400
Data length             :0300
Opcode                  :0482
Params:                 :01
```

因为 MCU 打包发的是 Set 的消息，有回应，如果长时间没收到 status 消息打包的回应（ MCU 应该比对发出的 destination addr 和收到的 source addr 是否一致），MCU 则做相应超时处理，应该收到响应才能发送下一条 Set 打包消息，依此类推。


#### Generic OnOff Set Unacknowledged 消息开灯

- 外部MCU通过串口给模块发送透传包：

```
01 01c0 0100 0400 0382 0100
Type                    :01
Destination address     :01c0
Source address          :0100
Data length             :0400
Opcode                  :0382
Params:                 :0100
```

Mesh 模块收到后，向 dest addr (`0xC001`)发送 Generic OnOff Set Unacknowledged 消息

因为 MCU 打包发的是 Set Unacknowledged 的消息，没有回应。

#### Generic OnOff Get 获取灯的状态

- 外部 MCU 通过串口给模块发送透传包：

```
01 0400 0100 0200 0182
Type                    :01
Destination address     :0400
Source address          :0100
Data length             :0200
Opcode                  :0182
Params:                 :NULL
```

Mesh模块收到后，向 dest addr(`0x0004`)发送 Generic OnOff Get 消息

Dest addr 设备回应 Generic OnOff Status 消息

- Mesh 模块收到消息后，通过串口给 MCU 发送透传包：

```
01 0100 0400 0300 0482 01
Type                    :01
Destination address     :0100
Source address          :0400
Data length             :0300
Opcode                  :0482
Params:                 :01
```

#### 获取beacon的所有参数（Vendor Message Get）

```
01 0100 fe7f 0500 d000 00 cf04 
```

#### 设置beacon的所有参数（Vendor Message Set）

```
01 0400 fe7f 1D00 d100 cf04 00 1800 0104 0304
ff12233445566778899AABBCCDDEEFF0 C3 00 F401
```


---

