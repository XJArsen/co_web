# webServer

## TODO
* 逐步使用协程替代回调,优化读写
* 将Reactor模型替代为Proactor模型，引入io_using作为异步io

## Function

* 使用状态机解析HTTP请求报文，支持解析GET和POST请求
* 访问服务器数据库实现web端用户注册、登录功能，可以请求服务器图片和视频文件
* 实现同步/异步日志系统，记录服务器运行状态

## Environment

* Ubuntu 22.04
* C++20
* MySql 8.0


## Thanks

Linux高性能服务器编程，游双著.

[@qinguoyi](https://github.com/qinguoyi/TinyWebServer)

[@markparticle](https://github.com/markparticle/WebServer)
