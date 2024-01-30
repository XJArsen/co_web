// Copyright (c) RealCoolEngineer. 2024. All rights reserved.
// Author: XJArsen
// Date: 2024-01-29

#ifndef INCLUDE_HTTP_HTTPRESPONSE_H_
#define INCLUDE_HTTP_HTTPRESPONSE_H_

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string>
#include <unordered_map>
#include "Buffer.h"
class Buffer;
class HttpResponse {
  public:
    HttpResponse();
    ~HttpResponse();

    void Init(const std::string& _srcDir, std::string& _path, bool _isKeepAlive = false,
              int _code = -1);
    void MakeResponse(Buffer&);
    void UnmapFile();
    char* File();
    size_t FileLen() const;
    void ErrorContent(Buffer&, std::string);
    int Code() const;

  private:
    void AddStateLine(Buffer&);
    void AddHeader(Buffer&);
    void AddContent(Buffer&);

    void ErrorHtml();
    std::string GetFileType();

    int code;
    bool isKeepAlive;

    std::string path;
    std::string srcDir;

    char* mmFile;
    struct stat mmFileStat;

    static const std::unordered_map<std::string, std::string> SUFFIX_TYPE;  // 后缀类型集
    static const std::unordered_map<int, std::string> CODE_STATUS;          // 编码状态集
    static const std::unordered_map<int, std::string> CODE_PATH;            // 编码路径集
};

const std::unordered_map<std::string, std::string> HttpResponse::SUFFIX_TYPE = {
    {".html", "text/html"},
    {".xml", "text/xml"},
    {".xhtml", "application/xhtml+xml"},
    {".txt", "text/plain"},
    {".rtf", "application/rtf"},
    {".pdf", "application/pdf"},
    {".word", "application/nsword"},
    {".png", "image/png"},
    {".gif", "image/gif"},
    {".jpg", "image/jpeg"},
    {".jpeg", "image/jpeg"},
    {".au", "audio/basic"},
    {".mpeg", "video/mpeg"},
    {".mpg", "video/mpeg"},
    {".avi", "video/x-msvideo"},
    {".gz", "application/x-gzip"},
    {".tar", "application/x-tar"},
    {".css", "text/css "},
    {".js", "text/javascript "},
};

const std::unordered_map<int, std::string> HttpResponse::CODE_STATUS = {
    {200, "OK"},
    {400, "Bad Request"},
    {403, "Forbidden"},
    {404, "Not Found"},
};

const std::unordered_map<int, std::string> HttpResponse::CODE_PATH = {
    {400, "/400.html"},
    {403, "/403.html"},
    {404, "/404.html"},
};

#endif
