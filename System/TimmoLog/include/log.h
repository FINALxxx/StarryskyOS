#ifndef __LOG_H__
#define __LOG_H__

#include <stdarg.h>

typedef enum {
  LOG_DEBUG,   // 调试信息，最详细
  LOG_INFO,    // 普通信息
  LOG_WARN,    // 警告信息
  LOG_ERROR,   // 错误信息
  LOG_FATAL    // 致命错误，最严重
} LogLevel;

void log_init(LogLevel level, const char* filename);
void log_print(LogLevel level, const char *file, int line, const char *format, ...);

#define log_debug(format, ...)   log_print(LOG_DEBUG, __FILE__, __LINE__, format, ##__VA_ARGS__)
#define log_info(format, ...)    log_print(LOG_INFO, __FILE__, __LINE__, format, ##__VA_ARGS__)
#define log_warn(format, ...)     log_print(LOG_WARN, __FILE__, __LINE__, format, ##__VA_ARGS__)
#define log_error(format, ...)    log_print(LOG_ERROR, __FILE__, __LINE__, format, ##__VA_ARGS__)
#define log_fatal(format, ...)    log_print(LOG_FATAL, __FILE__, __LINE__, format, ##__VA_ARGS__)


#endif
