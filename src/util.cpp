#include "util.h"

void errif(bool condition, const char *errmsg) {
    if (condition) {
        LOG_ERROR(errmsg);
        exit(EXIT_FAILURE);
    }
}