#include "Log.h"
#include <cassert>
#include <iostream>
Log::Log() {
    fp_ = nullptr;
    deq_ = nullptr;
    writeThread_ = nullptr;
    lineCount_ = 0;
    toDay_ = 0;
    isAsync_ = false;
}

Log::~Log() {
    while (!deq_->empty()) {
        deq_->flush();
    }
    deq_->Close();
    writeThread_->join();
    if (fp_) {
        lock_guard<mutex> locker(mtx_);
        flush();
        fclose(fp_);
    }
}

Log* Log::Instance() {
    static Log log;
    return &log;
}

void Log::init(int _level, const char* _path, const char* _suffix, int maxQueCapacity) {
    isOpen_ = true;
    level_ = _level;
    path_ = _path;
    suffix_ = _suffix;
    if (maxQueCapacity) {  // 异步方式
        isAsync_ = true;
        if (!deq_) {  // 为空则创建一个
            unique_ptr<BlockQueue<std::string>> newQue(new BlockQueue<std::string>);
            // 因为unique_ptr不支持普通的拷贝或赋值操作,所以采用move
            // 将动态申请的内存权给deque，newDeque被释放
            deq_ = move(newQue);  // 左值变右值,掏空newDeque

            unique_ptr<thread> newThread(new thread(FlushLogThread));
            writeThread_ = move(newThread);
        }
    } else {
        isAsync_ = false;
    }

    lineCount_ = 0;

    time_t timer = time(nullptr);
    struct tm* sysTime = localtime(&timer);
    struct tm t = *sysTime;
    path_ = _path;
    suffix_ = _suffix;
    char fileName[LOG_NAME_LEN] = {0};
    snprintf(fileName, LOG_NAME_LEN - 1, "%s/%04d_%02d_%02d%s", path_, t.tm_year + 1900,
             t.tm_mon + 1, t.tm_mday, suffix_);
    toDay_ = t.tm_mday;

    {
        lock_guard<mutex> locker(mtx_);
        buf_.RetrieveAll();
        if (fp_) {  // 重新打开
            flush();
            fclose(fp_);
        }
        fp_ = fopen(fileName, "a");  // 打开文件读取并附加写入
        if (fp_ == nullptr) {
            mkdir(path_, 0777);
            fp_ = fopen(fileName, "a");  // 生成目录文件（最大权限）
        }
        // assert(fp_ != nullptr);
    }
}

void Log::FlushLogThread() {
    Log::Instance()->AsyncWrite();
}

void Log::write(int _level, const char* _format, ...) {
    struct timeval now;
    gettimeofday(&now, nullptr);
    time_t tSec = now.tv_sec;
    struct tm* sysTime = localtime(&tSec);
    struct tm t = *sysTime;
    va_list vaList;

    // 不是今天 或 行数超了
    if (toDay_ != t.tm_mday || (lineCount_ && (lineCount_ % MAX_LINES == 0))) {
        unique_lock<mutex> locker(mtx_);
        locker.unlock();

        char newFile[LOG_NAME_LEN];
        char tail[36] = {0};
        snprintf(tail, 36, "%04d_%02d_%02d", t.tm_year + 1900, t.tm_mon + 1, t.tm_mday);

        // 时间不匹配，则替换为最新的日志文件名
        if (toDay_ != t.tm_mday) {
            snprintf(newFile, LOG_NAME_LEN - 72, "%s/%s%s", path_, tail, suffix_);
            toDay_ = t.tm_mday;
            lineCount_ = 0;
        } else {
            snprintf(newFile, LOG_NAME_LEN - 72, "%s/%s-%d%s", path_, tail,
                     (lineCount_ / MAX_LINES), suffix_);
        }

        locker.lock();
        flush();
        fclose(fp_);
        fp_ = fopen(newFile, "a");
        // assert(fp_ != nullptr);
    }

    // 在buffer内生成一条对应的日志信息
    {
        unique_lock<mutex> locker(mtx_);
        lineCount_++;
        int n =
            snprintf(buf_.BeginWrite(), 128, "%d-%02d-%02d %02d:%02d:%02d.%06ld ", t.tm_year + 1900,
                     t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec, now.tv_usec);

        buf_.HasWritten(n);
        AppendLogLevelTitle(_level);

        va_start(vaList, _format);
        int m = vsnprintf(buf_.BeginWrite(), buf_.WritableBytes(), _format, vaList);
        va_end(vaList);

        buf_.HasWritten(m);
        buf_.Append("\n\0", 2);

        if (isAsync_ && deq_ && !deq_->full()) {
            deq_->push_back(buf_.RetrieveAllToStr());
        } else {
            fputs(buf_.Peek(), fp_);
        }
        buf_.RetrieveAll();
    }
}

void Log::flush() {
    if (isAsync_) {
        deq_->flush();
    }
    fflush(fp_);
}

int Log::GetLevel() {
    lock_guard<mutex> locker(mtx_);
    return level_;
}

void Log::SetLevel(int _level) {
    lock_guard<mutex> locker(mtx_);
    level_ = _level;
}

bool Log::IsOpen() {
    return isOpen_;
}

void Log::AsyncWrite() {
    string str = "";
    while (deq_->pop(str)) {
        lock_guard<mutex> locker(mtx_);
        fputs(str.c_str(), fp_);
    }
}

void Log::AppendLogLevelTitle(int _level) {
    const int len = 9;
    switch (_level) {
        case 0:
            buf_.Append("[debug]: ");
            break;
        case 1:
            buf_.Append("[info] : ");
            break;
        case 2:
            buf_.Append("[warn] : ");
            break;
        case 3:
            buf_.Append("[error]: ");
            break;
        default:
            buf_.Append("[info] : ");
            break;
    }
}
