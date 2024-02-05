#include "HttpRequest.h"
#include <iostream>
#include "Log.h"
#include "SqlPool.h"
HttpRequest::HttpRequest() {
    Init();
}
HttpRequest::~HttpRequest() = default;

void HttpRequest::Init() {
    method = path = version = body = "";
    state = REQUEST_LINE;
    header.clear();
    post.clear();
}
bool HttpRequest::parse(Buffer& buf) {
    const char END[] = "\r\n";
    if (buf.ReadableBytes() <= 0) {
        return false;
    }
    while (buf.ReadableBytes() && state != FINISH) {
        const char* lineEnd = std::search(buf.Peek(), buf.BeginWriteConst(), END, END + 2);
        std::string line(buf.Peek(), lineEnd);
        std::cout << line << "\n";
        switch (state) {
            case REQUEST_LINE:
                if (!ParseRequestLine(line)) {
                    return false;
                }
                ParsePath();
                break;
            case HEADERS:
                ParseHeader(line);
                if (buf.ReadableBytes() <= 2) {
                    state = FINISH;
                }
                break;
            case BODY:
                ParseBody(line);
                break;
            default:
                break;
        }
        if (lineEnd == buf.BeginWrite()) {
            break;
        }
        buf.RetrieveUntil(lineEnd + 2);
    }
    LOG_DEBUG("[%s] [%s] [%s]", method.c_str(), path.c_str(), version.c_str())
    return true;
}

std::string HttpRequest::Path() const {
    return path;
}
std::string& HttpRequest::Path() {
    return path;
}
std::string HttpRequest::Method() const {
    return method;
}
std::string HttpRequest::Version() const {
    return version;
}
std::string HttpRequest::GetPost(const std::string& key) const {
    if (post.count(key) == 1) {
        return post.find(key)->second;
    }
    return "";
}
std::string HttpRequest::GetPost(const char* key) const {
    if (post.count(key) == 1) {
        return post.find(key)->second;
    }
    return "";
}

bool HttpRequest::IsKeepAlive() const {
    if (header.count("Connection") == 1) {
        return header.find("Connection")->second == "keep-alive" && version == "1.1";
    }
    return false;
}

bool HttpRequest::ParseRequestLine(const std::string& line) {
    regex patten("^([^ ]*) ([^ ]*) HTTP/([^ ]*)$");
    std::smatch sub_match;
    if (regex_match(line, sub_match, patten)) {
        method = sub_match[1];
        path = sub_match[2];
        version = sub_match[3];
        state = HEADERS;
        return true;
    }
    LOG_ERROR("RequestLine Error");
    return false;
}
void HttpRequest::ParseHeader(const std::string& line) {
    regex patten("^([^:]*): ?(.*)$");
    smatch sub_match;
    if (regex_match(line, sub_match, patten)) {
        header[sub_match[1]] = sub_match[2];
        std::cout << sub_match[1] << " " << sub_match[2] << "\n";
    } else {
        state = BODY;
    }
}
void HttpRequest::ParseBody(const std::string& line) {
    body = line;
    ParsePost();
    state = FINISH;
    LOG_DEBUG("BODY;%s, len:%d", line.c_str(), line.size());
}

void HttpRequest::ParsePath() {
    if (path == "/") {
        path = "/index.html";
    } else {
        if (DEFAULT_HTML.find(path) != DEFAULT_HTML.end()) {
            path += ".html";
        }
    }
}
void HttpRequest::ParsePost() {
    if (method != "POST" || header["Content-Type"] != "application/x-www-form-urlencoded") return;
    ParseFromUrlencoded();
    if (!DEFAULT_HTML_TAG.count(path)) {
        return;
    }

    int tag = DEFAULT_HTML_TAG[path];
    LOG_DEBUG("Tag:%d", tag);
    if (tag == 0 || tag == 1) {
        bool isLogin = (tag == 1);
        if (UserVerify(post["username"], post["password"], isLogin)) {
            path = "/welcome.html";
        } else {
            path = "/error.html";
        }
    }
}
void HttpRequest::ParseFromUrlencoded() {
    if (body == "") return;

    string key, val;

    int num = 0, len = body.size();
    for (int i = 0, j = 0; i < len; i++) {
        char ch = body[i];
        switch (ch) {
            case '=':
                key = body.substr(j, i - j);
                j = i + 1;
                break;
            case '+':
                body[i] = ' ';
                break;
            case '%':
                num = ConverHex(body[i + 1]) * 16 + ConverHex(body[i + 2]);
                body[i + 2] = num % 10 + '0';
                body[i + 1] = num / 10 + '0';
                i += 2;
                break;
            case '&':
                val = body.substr(j, i - j);
                j = i + 1;
                post[key] = val;
                LOG_DEBUG("%s = %s", key.c_str(), val.c_str());
                break;
            default:
                break;
        }
        if (i == len - 1) {
            if (!post.count(key) && j < i) {
                val = body.substr(j, i - j);
                post[key] = val;
            }
        }
    }
}

bool HttpRequest::UserVerify(const std::string& name, const std::string& pwd, bool isLogin) {
    if (name == "" || pwd == "") return false;
    LOG_INFO("Verify name:%s  pwd:%s", name.c_str(), pwd.c_str());
    MYSQL* sql;
    SqlRAII(&sql, SqlPool::Instance());

    bool fg = (!isLogin);
    unsigned int j = 0;
    char order[256] = {};
    MYSQL_FIELD* fields = nullptr;
    MYSQL_RES* res = nullptr;

    snprintf(order, 256, "SELECT username, password FROM user WHERE username='%s' LIMIT 1",
             name.c_str());
    LOG_DEBUG("%s", order);
    if (mysql_query(sql, order)) {
        mysql_free_result(res);
        return false;
    }
    res = mysql_store_result(sql);
    j = mysql_num_fields(res);
    fields = mysql_fetch_fields(res);

    while (MYSQL_ROW row = mysql_fetch_row(res)) {
        LOG_DEBUG("MYSQL ROW: %s %s", row[0], row[1]);
        string password(row[1]);
        /* 注册行为 且 用户名未被使用*/
        if (isLogin) {
            if (pwd == password) {
                fg = true;
            } else {
                fg = false;
                LOG_INFO("pwd error!");
            }
        } else {
            fg = false;
            LOG_INFO("user used!");
        }
    }
    mysql_free_result(res);

    /* 注册行为 且 用户名未被使用*/
    if (!isLogin && fg == true) {
        LOG_DEBUG("regirster!");
        bzero(order, 256);
        snprintf(order, 256, "INSERT INTO user(username, password) VALUES('%s','%s')", name.c_str(),
                 pwd.c_str());
        LOG_DEBUG("%s", order);
        if (mysql_query(sql, order)) {
            LOG_DEBUG("Insert error!");
            fg = false;
        }
        fg = true;
    }
    // SqlConnPool::Instance()->FreeConn(sql);
    LOG_DEBUG("UserVerify success!!");
    return fg;
}

int HttpRequest::ConverHex(char ch) {
    if (islower(ch)) return ch - 'a' + 10;
    if (isupper(ch)) return ch - 'A' + 10;
    if (isdigit(ch)) return ch - '0';
    LOG_DEBUG("number error");
    return -1;
}

const std::unordered_set<std::string> HttpRequest::DEFAULT_HTML{
    "/index", "/register", "/login", "/welcome", "/video", "/picture",
};

std::unordered_map<std::string, int> HttpRequest::DEFAULT_HTML_TAG{
    {"/register.html", 0},
    {"/login.html", 1},
};
