[TOC]

### 版本

| 版本号 | 变更内容 | 作者 | 时间 |
| --- | --- | --- | --- |
| 1.0.0 | - 初始版本； | gavin | 2019.07.24 |

---


### 1、描述

本文档主要描述了定时器功能的相关协议总结和应用交互相关的内容，本文档主要用到了 `Mesh_Model_Specification v1.0.pdf` 文档 `5.2.2 Scene Messgae` 相关内容；

### 2、Scene OpCodes

|  Name |  Value |
| ------------ | ------------ |
| SCENE_STATUS  |  0x5E |
|  SCENE_REGISTER_STATUS |0x8245   |
| SCENE_GET  | 0x8241  |
| SCENE_REGISTER_GET  |  0x8244 |
|  SCENE_RECALL |  0x8242 |
|  SCENE_RECALL_UNACKNOWLEDGED |  0x8243 |
| SCENE_STORE  | 0x8246  |
|  SCENE_STORE_UNACKNOWLEDGED | 0x8247  |
| SCENE_DELETE  |  0x829E |
| SCENE_DELETE_UNACKNOWLEDGED  |  0x829F |


### 3、Scene Message

#### 3.1、Scene Store

场景存储是一条确认消息，用于将元素的当前状态存储为场景，稍后可以调用。
对场景存储消息的响应是场景寄存器状态消息。下表定义了消息的结构。

| Field  | Size(octets)  | Notes  |
| ------------ | ------------ | ------------ |
|  Scene Number |   2 | 要存储的场景的编号,“场景编号”字段标识预期场景,禁止使用值0x0000。  |

#### 3.2、Scene Store Unacknowledged

一条不可靠的消息，内容见`3.1、Scene Store`.

#### 3.3、Scene Recall

场景调用是一条确认消息，可让设备回到这个场景的状态。
对场景调用消息的响应是场景状态`3.6、Scene Status`消息。下表定义了消息的结构。

| Field  | Size (octets)  | Notes  |
| ------------ | ------------ | ------------ |
|  Scene Number | 2  | 要调用的场景的编号。  |
| TID  |  1 | TID  |
| Transition Time  | 1  |  Transition Time(Optional) 见`4、Transition Time` |
|  Delay | 1  | 消息执行延迟  |




#### 3.4、Scene Recall Unacknowledged

一条不可靠的消息，内容见` 3.3、Scene Recall`.

#### 3.5、Scene Get

scene get 是一条确认消息，用于获取元素当前活动场景的当前状态，此消息没有参数。
对场景获取消息的响应是场景状态消息`3.6、Scene Status`。

#### 3.6、Scene Status

场景状态是一条未确认的消息，用于报告元素当前活动场景的当前状态。
下表定义了消息的结构。

| Field  | Size (octets)  | Notes  |
| ------------ | ------------ | ------------ |
| Status Code  | 1   | 见`5、状态码`  |
|  Current Scene | 2  |  当前场景编号 |
|  Target Scene |  2 | 目标场景编号（可选）  |
| Remaining Time  | 1  |  剩余时间，同理`4、Transition Time` |

如果存在目标场景字段，则剩余时间字段也应存在；否则，字段不应存在。

“当前场景”字段标识当前场景的场景编号。如果没有激活的场景，则当前场景字段值为0。

当一个元素正在改变场景状态时，目标场景字段标识该元素要到达的目标场景状态的目标场景编号。
当一个元素不在改变场景状态的过程中时，省略目标场景字段。
如果存在，剩余时间字段指示元素完成到元素的目标场景状态转换所需的时间。

#### 3.7、Scene Register Get

获取场景寄存器状态，这条消息没有参数。
响应是场景寄存器状态消息`3.8、Scene Register Status`。

#### 3.8、Scene Register Status

下表定义了消息的结构。

| Field  |  Size (octets) |  Notes |
| ------------ | ------------ | ------------ |
| Status Code  |  1 |  见`5、状态码` |
| Current Scene  | 2  | 当前场景  |
|  Scenes | 变量  | 元素保存的所有场景编号集合  |

#### 3.9、Scene Delete

Scene Delete  是一条可靠的消息，从元素删除一个场景，响应为`3.8、Scene Register Status `

下表定义了消息的结构。

| Field  |  Size (octets) |  Notes |
| ------------ | ------------ | ------------ |
| Scene Number|  2 |  要删除的场景编号 |

#### 3.10、Scene Delete Unacknowledged

不可靠的删除信息，内容见`3.9、Scene Delete`

### 4、Transition Time

关于`Transition Time`格式如下表

| Field  | Size (bits)  | Definition  |
| ------------ | ------------ | ------------ |
| Default Transition Number of Steps  | 6  |  The number of Steps |
|  Default Transition Step Resolution | 2  | The resolution of the Default Transition Number of Steps field  |

![](/uploads/xt_mesh/images/m_bdf5f87433e8117a91c750de741623d4_r.png)

关于`Default Transition Step Resolution`格式如下表

| Value  | Description  |
| ------------ | ------------ |
| 0b00  | The Default Transition Step Resolution is 100 milliseconds  |
| 0b01  |  The Default Transition Step Resolution is 1 second |
| 0b10  |  The Default Transition Step Resolution is 10 seconds |
| 0b11  | The Default Transition Step Resolution is 10 minutes  |

关于`Default Transition Number of Steps`格式如下表

| Value  | Description  |
| ------------ | ------------ |
| 0x00  | 一般的默认转换时间是即时的  |
| 0x01–0x3E  |  The number of steps. |
|  0x3F  |  值未知。无法将状态设置为此值，但如果转换高于0x3e或未确定，则元素可能会报告未知值。 |

- 示例

假如`Step Resolution=0b10`，`Steps=0x3E`,则`Transition Time=62*10秒=620秒`


### 5、状态码

| Value  | Description  |
| ------------ | ------------ |
| 0x00  | Success  |
|  0x01 |  Scene Register Full |
| 0x02  | Scene Not Found  |
| 0x03–0xFF  | Reserved for Future Use  |
