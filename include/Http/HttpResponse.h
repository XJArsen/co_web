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

    int code_;
    bool isKeepAlive_;

    std::string path_;
    std::string srcDir_;

    char* mmFile_;
    struct stat mmFileStat_;

    static const std::unordered_map<std::string, std::string> SUFFIX_TYPE;  // 后缀类型集
    static const std::unordered_map<int, std::string> CODE_STATUS;          // 编码状态集
    static const std::unordered_map<int, std::string> CODE_PATH;            // 编码路径集
};

#endif
