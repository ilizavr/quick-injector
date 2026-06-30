#ifndef LOGGER_H
#define LOGGER_H

#include <android/log.h>

#define LOGD(...) {printf("[info]");printf(__VA_ARGS__);printf("\n");}
#define LOGE(...) {printf("[error]");printf(__VA_ARGS__);exit(-1);}
#define LOGI(...) {printf("[info]");printf(__VA_ARGS__);printf("\n");}
#define LOGW(...) {printf("[warning]");printf(__VA_ARGS__);printf("\n");}

#endif
