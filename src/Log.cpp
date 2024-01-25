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
}
void Log::init(int level, const char* path = "./log", const char* suffix = ".log",
               int maxQueueCapacity = 1024) {
}

Log* Log::Instance() {
}
void Log::FlushLogThread() {
}
void Log::write(int level, const char* format, ...) {
}
void Log::flush() {
}

int Log::GetLevel() {
}
void Log::SetLevel(int level) {
}
bool Log::IsOpen() {
    return isOpen;
}
void Log::AsyncWrite() {
}
void Log::AppendLogLevelTitle(int level) {
}
