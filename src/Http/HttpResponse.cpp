#include "HttpResponse.h"
#include "Log.h"
HttpResponse::HttpResponse() : code_(-1), mmFile_(nullptr), isKeepAlive_(false) {
    path_ = srcDir_ = "";
    mmFileStat_ = {};
}
HttpResponse::~HttpResponse() {
    UnmapFile();
}

void HttpResponse::Init(const std::string& _srcDir, std::string& _path, bool _isKeepAlive,
                        int _code) {
    if (mmFile_) {
        UnmapFile();
    }

    code_ = _code;
    isKeepAlive_ = _isKeepAlive;
    path_ = _path;
    srcDir_ = _srcDir;
    mmFile_ = nullptr;
    mmFileStat_ = {};
}
void HttpResponse::MakeResponse(Buffer& buf) {
    if (stat((srcDir_ + path_).data(), &mmFileStat_) < 0 || S_ISDIR(mmFileStat_.st_mode)) {
        code_ = 404;
    } else if (!(mmFileStat_.st_mode & S_IROTH)) {
        code_ = 403;
    } else if (code_ == -1) {
        code_ = 200;
    }
    ErrorHtml();
    AddStateLine(buf);
    AddHeader(buf);
    AddContent(buf);
}
void HttpResponse::UnmapFile() {
    if (mmFile_) {
        munmap(mmFile_, mmFileStat_.st_size);
        mmFile_ = nullptr;
    }
}
char* HttpResponse::File() {
    return mmFile_;
}
size_t HttpResponse::FileLen() const {
    return mmFileStat_.st_size;
}

int HttpResponse::Code() const {
    return code_;
}

void HttpResponse::AddStateLine(Buffer& buf) {
    std::string status;
    if (CODE_STATUS.count(code_)) {
        status = CODE_STATUS.find(code_)->second;

    } else {
        code_ = 400;
        status = CODE_STATUS.find(400)->second;
    }
    // buf.Append("HTTP/1.1" + std::to_string(code_) + " " + status + "\r\n");  // 有问题，待解决
    buf.Append("HTTP/1.1" + std::to_string(code_) + status + "\r\n");  // 有问题，待解决
}
void HttpResponse::AddHeader(Buffer& buf) {
    buf.Append("Connection: ");
    if (isKeepAlive_) {
        buf.Append("keep-alive\r\n");
        buf.Append("keep-alive: max=6, timeout=120\r\n");
    } else {
        buf.Append("close\r\n");
    }
    buf.Append("Content-type: " + GetFileType() + "\r\n");
}
void HttpResponse::AddContent(Buffer& buf) {
    int srcfd = open((srcDir_ + path_).data(), O_RDONLY);
    if (srcfd < 0) {
        ErrorContent(buf, "File not found");
        return;
    }
    LOG_DEBUG("file path %s", (srcDir_ = path_).data());

    int* mmRet = (int*)mmap(0, mmFileStat_.st_size, PROT_READ, MAP_PRIVATE, srcfd, 0);
    if (*mmRet == -1) {
        ErrorContent(buf, "File not found");
        return;
    }
    mmFile_ = (char*)mmRet;
    close(srcfd);
    buf.Append("Content-length: " + to_string(mmFileStat_.st_size) + "\r\n\r\n");
}
std::string HttpResponse::GetFileType() {
    std::string::size_type idx = path_.find_last_of('.');
    if (idx == std::string::npos) {
        return "text/plain";
    }
    std::string suffiex = path_.substr(idx);
    if (SUFFIX_TYPE.count(suffiex)) {
        return SUFFIX_TYPE.find(suffiex)->second;
    }
    return "text/plain";
}

void HttpResponse::ErrorHtml() {
    if (CODE_PATH.count(code_)) {
        path_ = CODE_PATH.find(code_)->second;
        stat((srcDir_ + path_).data(), &mmFileStat_);
    }
}
void HttpResponse::ErrorContent(Buffer& buf, std::string message) {
    std::string body_;
    std::string status;
    body_ += "<html><title>Error</title>";
    body_ += "<body_ bgcolor=\"ffffff\">";
    if (CODE_STATUS.count(code_) == 1) {
        status = CODE_STATUS.find(code_)->second;
    } else {
        status = "Bad Request";
    }
    body_ += to_string(code_) + " : " + status + "\n";
    body_ += "<p>" + message + "</p>";
    body_ += "<hr><em>TinyWebServer</em></body_></html>";

    buf.Append("Content-length: " + to_string(body_.size()) + "\r\n\r\n");
    buf.Append(body_);
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
