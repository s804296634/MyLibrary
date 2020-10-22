[TOC]

### 版本

| 版本号 | 变更内容 | 作者 | 时间 |
| --- | --- | --- | --- |
| 1.0.0 | - 初始版本； | Roy | 2019.06.25 |
| 1.0.1 | - 修改格式； | TrusBe | 2019.08.27 |

---

### 序言

在产品开发中，应用和固件端需要通过一个统一的协议文档来统筹开发工作，这就需要在 开发之前先将通讯协议文档书写出来，然后两端可以几乎同步的进行开发，这样既可以节 省开发时间，也可以更好的协调两端的开发工作；对于公司来说，统一的协议文档和通讯 方式，更方便项目的量化处理，协议的版本化管理，更好的协调后续的升级工作；

本文档主要协助开发人员制定硬件与软件间通讯的协议规范，对于一家公司的产品，统一 的协议规范，对于任意一方的开发人员都能够方便处理和查找问题，也便于开发过程中的交流；


### 请求方式
```
GET/POST
```


### URL

```
基本格式： scheme://host[:port#]/path/…/[?query-string]
scheme： 协议（例如： http, https）
host： 服务器的 IP 地址或者域名
port： 服务器的端口（缺省端口 80）
path： 访问资源的路径
query-string：参数，发送给 http 服务器的数据
```


### 请求头 - Header
```
Content-Type：application/json
token：客户端凭据（适用于校验安全的接口中）
```

### 请求内容-Body

基本格式： `JSON`
JOSN规范：http://jsonapi.org.cn/


### 返回内容
基本格式： `JSON`
JOSN规范：http://jsonapi.org.cn/format/

```
请求成功返回格式：
{
    "code": 0,
    "msg": "ok",        //请求成功
    "object": JSON      //自定义字段
}

请求失败返回格式：
{
    "code": errorCode,  //错误码
    "msg": errorMsg     //错误码描述
}
```


### 通用错误码
| Code | 类型 | 解释 | 说明 |
| --- | --- | --- | --- |
| 0 | Success | 请求成功 | --- |
| 500 | Error | 无效请求 | --- |
| 501 | Error | 参数不正确 | --- |
| 502 | Error | 数据不存在 | --- |
| 503 | Error | 数据已存在 | --- |


本公司保留在不通知读者的情况下，修改文档或者软件相关内容的权利，对于使用中出现的任何效果，本公司不承担任何责任。