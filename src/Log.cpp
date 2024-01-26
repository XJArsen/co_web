#include "Log.h"
Log::Log() {
    fp = nullptr;
    deq = nullptr;
    writeThread = nullptr;
    lineCount = 0;
    toDay = 0;
    isAsync = false;
}

Log::~Log() {
    while (!deq->empty()) {
        deq->flush();
    }
    deq->Close();
    writeThread->join();
    if (fp) {
        lock_guard<mutex> locker(mtx);
        flush();
        fclose(fp);
    }
}

Log* Log::Instance() {
    static Log log;
    return &log;
}

void Log::init(int _level, const char* _path = "./log", const char* _suffix = ".log",
               int maxQueueCapacity = 1024) {
    isOpen = true;
    level = _level;
    path = _path;
    suffix = _suffix;
    if (maxQueueCapacity == 0) {  // 同步方式
        isAsync = false;
    } else {  // 异步方式
        isAsync = true;
        if (!deq) {
            unique_ptr<BlockQueue<std::string>> newQue(new BlockQueue<std::string>);
            deq = move(newQue);
            unique_ptr<thread> newThread(new thread(FlushLogThread));
            writeThread = move(newThread);
        }
    }

    lineCount = 0;
    time_t timer = time(nullptr);
    struct tm* systime = localtime(&timer);
    char fileName[LOG_NAME_LEN] = {0};
    snprintf(fileName, LOG_NAME_LEN - 1, "%s/%04d_%02d_%02d%s", path, systime->tm_year + 1900,
             systime->tm_mon + 1, systime->tm_mday, suffix);
    toDay = systime->tm_mday;

    {
        lock_guard<mutex> locker(mtx);
        buf.RetrieveAll();
        if (fp) {
            flush();
            fclose(fp);
        }
        fp = fopen(fileName, "a");
        if (fp == nullptr) {
            mkdir(fileName, S_IRWXU);
            fp = fopen(fileName, "a");
        }
        // assert(fp != nullptr);
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
    if (toDay != t.tm_mday || (lineCount && (lineCount % MAX_LINES == 0))) {
        unique_lock<mutex> locker(mtx);
        locker.unlock();

        char newFile[LOG_NAME_LEN];
        char tail[36] = {0};
        snprintf(tail, 36, "%04d_%02d_%02d", t.tm_year + 1900, t.tm_mon + 1, t.tm_mday);

        // 时间不匹配，则替换为最新的日志文件名
        if (toDay != t.tm_mday) {
            snprintf(newFile, LOG_NAME_LEN - 72, "%s/%s%s", path, tail, suffix);
            toDay = t.tm_mday;
            lineCount = 0;
        } else {
            snprintf(newFile, LOG_NAME_LEN - 72, "%s/%s-%d%s", path, tail, (lineCount / MAX_LINES),
                     suffix);
        }

        locker.lock();
        flush();
        fclose(fp);
        fp = fopen(newFile, "a");
        // assert(fp != nullptr);
    }

    // 在buffer内生成一条对应的日志信息
    {
        unique_lock<mutex> locker(mtx);
        lineCount++;
        int n =
            snprintf(buf.BeginWrite(), 128, "%d-%02d-%02d %02d:%02d:%02d.%06ld ", t.tm_year + 1900,
                     t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec, now.tv_usec);

        buf.HasWritten(n);
        AppendLogLevelTitle(_level);

        va_start(vaList, _format);
        int m = vsnprintf(buf.BeginWrite(), buf.WritableBytes(), _format, vaList);
        va_end(vaList);

        buf.HasWritten(m);
        buf.Append("\n\0", 2);

        if (isAsync && deq && !deq->full()) {
            deq->push_back(buf.RetrieveAllToStr());
        } else {
            fputs(buf.Peek(), fp);
        }
        buf.RetrieveAll();
    }
}

void Log::flush() {
    if (isAsync) {
        deq->flush();
    }
    fflush(fp);
}

int Log::GetLevel() {
    lock_guard<mutex> locker(mtx);
    return level;
}

void Log::SetLevel(int _level) {
    lock_guard<mutex> locker(mtx);
    level = _level;
}

bool Log::IsOpen() {
    return isOpen;
}

void Log::AsyncWrite() {
    string str = "";
    while (deq->pop(str)) {
        lock_guard<mutex> locker(mtx);
        fputs(str.c_str(), fp);
    }
}

void Log::AppendLogLevelTitle(int _level) {
    const int len = 9;
    switch (_level) {
        case 0:
            buf.Append("[debug]: ");
            break;
        case 1:
            buf.Append("[info] : ");
            break;
        case 2:
            buf.Append("[warn] : ");
            break;
        case 3:
            buf.Append("[error]: ");
            break;
        default:
            buf.Append("[info] : ");
            break;
    }
}
