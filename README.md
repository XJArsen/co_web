# webServer

## Function

* 使用状态机解析HTTP请求报文，支持解析GET和POST请求
* 访问服务器数据库实现web端用户注册、登录功能，可以请求服务器图片和视频文件
* 实现同步/异步日志系统，记录服务器运行状态

## Environment

* Ubuntu 22.04
* C++20(g++13)
* MySql 8.0

## TODO
* 利用co_web 通过协程改造提高并发，使用io_uring的异步框架
* 替换MySQL, 计划使用PostgreSQL提高性能
* 支持websocket

## Reference

[@yunwei37](https://github.com/yunwei37/co-uring-WebServer/tree/master)
[@qinguoyi](https://github.com/qinguoyi/TinyWebServer)
[@markparticle](https://github.com/markparticle/WebServer)
[@archibate](https://github.com/archibate/co_async)
[@Caturra000](https://github.com/Caturra000/io_uring-examples-cpp)