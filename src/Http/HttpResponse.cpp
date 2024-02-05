#include "HttpResponse.h"
#include "Log.h"
HttpResponse::HttpResponse() : code(-1), mmFile(nullptr), isKeepAlive(false) {
    path = srcDir = "";
    mmFileStat = {};
}
HttpResponse::~HttpResponse() {
    UnmapFile();
}

void HttpResponse::Init(const std::string& _srcDir, std::string& _path, bool _isKeepAlive,
                        int _code) {
    if (mmFile) {
        UnmapFile();
    }

    code = _code;
    isKeepAlive = _isKeepAlive;
    path = _path;
    srcDir = _srcDir;
    mmFile = nullptr;
    mmFileStat = {};
}
void HttpResponse::MakeResponse(Buffer& buf) {
    if (stat((srcDir + path).data(), &mmFileStat) < 0 || S_ISDIR(mmFileStat.st_mode)) {
        code = 404;
    } else if (!(mmFileStat.st_mode & S_IROTH)) {
        code = 403;
    } else if (code == -1) {
        code = 200;
    }
    ErrorHtml();
    AddStateLine(buf);
    AddHeader(buf);
    AddContent(buf);
}
void HttpResponse::UnmapFile() {
    if (mmFile) {
        munmap(mmFile, mmFileStat.st_size);
        mmFile = nullptr;
    }
}
char* HttpResponse::File() {
    return mmFile;
}
size_t HttpResponse::FileLen() const {
    return mmFileStat.st_size;
}

int HttpResponse::Code() const {
    return code;
}

void HttpResponse::AddStateLine(Buffer& buf) {
    std::string status;
    if (CODE_STATUS.count(code)) {
        status = CODE_STATUS.find(code)->second;

    } else {
        code = 400;
        status = CODE_STATUS.find(400)->second;
    }
    // buf.Append("HTTP/1.1" + std::to_string(code) + " " + status + "\r\n");  // 有问题，待解决
    buf.Append("HTTP/1.1" + std::to_string(code) + status + "\r\n");  // 有问题，待解决
}
void HttpResponse::AddHeader(Buffer& buf) {
    buf.Append("Connection: ");
    if (isKeepAlive) {
        buf.Append("keep-alive\r\n");
        buf.Append("keep-alive: max=6, timeout=120\r\n");
    } else {
        buf.Append("close\r\n");
    }
    buf.Append("Content-type: " + GetFileType() + "\r\n");
}
void HttpResponse::AddContent(Buffer& buf) {
    int srcfd = open((srcDir + path).data(), O_RDONLY);
    if (srcfd < 0) {
        ErrorContent(buf, "File not found");
        return;
    }
    LOG_DEBUG("file path %s", (srcDir = path).data());

    int* mmRet = (int*)mmap(0, mmFileStat.st_size, PROT_READ, MAP_PRIVATE, srcfd, 0);
    if (*mmRet == -1) {
        ErrorContent(buf, "File not found");
        return;
    }
    mmFile = (char*)mmRet;
    close(srcfd);
    buf.Append("Content-length: " + to_string(mmFileStat.st_size) + "\r\n\r\n");
}
std::string HttpResponse::GetFileType() {
    std::string::size_type idx = path.find_last_of('.');
    if (idx == std::string::npos) {
        return "text/plain";
    }
    std::string suffiex = path.substr(idx);
    if (SUFFIX_TYPE.count(suffiex)) {
        return SUFFIX_TYPE.find(suffiex)->second;
    }
    return "text/plain";
}

void HttpResponse::ErrorHtml() {
    if (CODE_PATH.count(code)) {
        path = CODE_PATH.find(code)->second;
        stat((srcDir + path).data(), &mmFileStat);
    }
}
void HttpResponse::ErrorContent(Buffer& buf, std::string message) {
    std::string body;
    std::string status;
    body += "<html><title>Error</title>";
    body += "<body bgcolor=\"ffffff\">";
    if (CODE_STATUS.count(code) == 1) {
        status = CODE_STATUS.find(code)->second;
    } else {
        status = "Bad Request";
    }
    body += to_string(code) + " : " + status + "\n";
    body += "<p>" + message + "</p>";
    body += "<hr><em>TinyWebServer</em></body></html>";

    buf.Append("Content-length: " + to_string(body.size()) + "\r\n\r\n");
    buf.Append(body);
}

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
