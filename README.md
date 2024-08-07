# co_web

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

## 压测
cpu 2核 内存 8G
```shell
wrk -t2 -c800 -d20s http://127.0.0.1:8080/
Running 20s test @ http://127.0.0.1:8080/
  2 threads and 800 connections
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency    69.35ms   18.19ms 327.38ms   75.50%
    Req/Sec     5.70k     1.46k   13.89k    82.74%
  225852 requests in 20.06s, 41.35MB read
Requests/sec:  11256.87
Transfer/sec:      2.06MB

```

## Reference

[@yunwei37](https://github.com/yunwei37/co-uring-WebServer/tree/master)
[@qinguoyi](https://github.com/qinguoyi/TinyWebServer)
[@markparticle](https://github.com/markparticle/WebServer)
[@archibate](https://github.com/archibate/co_async)
[@Caturra000](https://github.com/Caturra000/io_uring-examples-cpp)