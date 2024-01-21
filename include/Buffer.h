// Copyright (c) RealCoolEngineer. 2024. All rights reserved.
// Author: XJArsen
// Date: 2024-01-21

#ifndef INCLUDE_BUFFER_H_
#define INCLUDE_BUFFER_H_
#include <string>
class Buffer {
   private:
    std::string buf;

   public:
    Buffer() = default;
    ~Buffer() = default;
    void append(const char*, int);
    ssize_t size();
    const char* c_str();
    void clear();
    void getline();
};

#endif
