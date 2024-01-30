// Copyright (c) RealCoolEngineer. 2024. All rights reserved.
// Author: XJArsen
// Date: 2024-01-29

#ifndef INCLUDE_HTTP_HTTPREQUEST_H_
#define INCLUDE_HTTP_HTTPREQUEST_H_
#include <mysql/mysql.h>
#include <cerrno>
#include <regex>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include "Buffer.h"
class Buffer;
class HttpRequest {
  public:
    enum PARSE_STATE {
        REQUEST_LINE,
        HEADERS,
        BODY,
        FINISH,
    };

    HttpRequest();
    ~HttpRequest();

    void Init();
    bool parse(Buffer& buff);

    std::string Path() const;
    std::string& Path();
    std::string Method() const;
    std::string Version() const;
    std::string GetPost(const std::string& key) const;
    std::string GetPost(const char* key) const;

    bool IsKeepAlive() const;

  private:
    bool ParseRequestLine(const std::string& line);  // 处理请求行
    void ParseHeader(const std::string& line);       // 处理请求头
    void ParseBody(const std::string& line);         // 处理请求体

    void ParsePath();            // 处理请求路径
    void ParsePost();            // 处理Post事件
    void ParseFromUrlencoded();  // 从url种解析编码

    static bool UserVerify(const std::string& name, const std::string& pwd,
                           bool isLogin);  // 用户验证

    PARSE_STATE state;
    std::string method, path, version, body;
    std::unordered_map<std::string, std::string> header;
    std::unordered_map<std::string, std::string> post;

    static const std::unordered_set<std::string> DEFAULT_HTML;
    static std::unordered_map<std::string, int> DEFAULT_HTML_TAG;
    static int ConverHex(char ch);  // 16进制转换为10进制
};
const std::unordered_set<std::string> HttpRequest::DEFAULT_HTML{
    "/index", "/register", "/login", "/welcome", "/video", "/picture",
};

std::unordered_map<std::string, int> HttpRequest::DEFAULT_HTML_TAG{
    {"/register.html", 0},
    {"/login.html", 1},
};

#endif
