[TOC]

### 版本

| 版本号 | 变更内容 | 作者 | 时间 |
| --- | --- | --- | --- |
| 1.0.0 | - 初始版本； | TrusBe | 2020.10.14 |

---

### 1、背景
- 对于当前公司固件广播包内容固件的部分进行规范化；

- 由于 Android 和 iOS 的差异性，Android 可以获取到设备的 Mac 地址，而 iOS 设备不能直接获取到设备的 Mac 地址，而大部分时候，App 开发功能的时候，方便用户知道自己手中的设备，通用的做法是在广播中放置 Mac 地址，但是公司每个固件的广播包数据结构不一，没有统一的格式和位置，对于一些通用的 App （例如通用固件升级）不能兼容所有的广播包，所以需要对当前广播包内容进行标准化处理；


### 2、协议内容
#### 2.1、数据位置
- 嵌入式位置：`广播包中的厂商数据，从第 3 个字节开始。例如：" 09 ff xx xx MAC[5--0]"（xx xx 为厂商ID）`

- iOS 位置：`CBAdvertisementDataManufacturerDataKey`

- Android 位置：`result.getScanRecord().getManufacturerSpecificData("厂商数据")`


#### 2.2、规范格式
固件开发过程中广播包格式在此基础上进行扩展，特殊情况当前主要是 Beacon，其他固件请按照如下格式：

| Type | Len | Data(例) | 备注 |
| --- | --- | --- | --- |
| vId | 2 bytes | `0xFFFF` | 厂商 ID |
| mac | 6 bytes | `D10001020304` | 设备 Mac 地址 |
| fVer | 2 bytes | `0x0001` | 设备固件版本号 |
| dType | 2 bytes | `0x2001` | 设备类型码 |


### 3、Mac 地址定义
Mac 地址一般为 6 字节数据，由生产手动按照规则烧录或者直接采用蓝牙芯片自带 Mac 地址。

为了在后续不影响其他数据的位置基础上，将 Mac 放置在广播中厂商数据字段的第 `2 - 7` 个字节占位使用；


#### 3.1、App 显示 Mac 地址
App 在应用中显示 Mac 地址，按照标准显示方式：中间使用`冒号 ( : ) `分隔，例如：`D1:00:01:02:03:04`


### 4、参考
升级规范请参考：[DFU 嵌入式端DFU和 Bootload 修改规范](http://192.168.1.7:8181/docs/fw_dfu_system/fw_dfu_system-1cah2sjrf9p57 "DFU 嵌入式端DFU和 Bootload 修改规范")

版本号规范参考：[版本号规范](http://192.168.1.7:8181/docs/specification_document/specification_document-1baqsektic1b8 "版本号规范")



---

