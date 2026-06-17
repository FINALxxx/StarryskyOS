#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "log.h"

static LogLevel g_log_level = LOG_INFO;

// ANSI颜色代码
#define ANSI_COLOR_RESET   "\033[0m"
#define ANSI_COLOR_RED     "\033[31m"
#define ANSI_COLOR_GREEN   "\033[32m"
#define ANSI_COLOR_YELLOW  "\033[33m"
#define ANSI_COLOR_BLUE    "\033[34m"
#define ANSI_COLOR_MAGENTA "\033[35m"
#define ANSI_COLOR_CYAN    "\033[36m"

static const char* log_level_strings[] = {
    "DEBUG",
    "INFO",
    "WARN",
    "ERROR",
    "FATAL"
};

static const char* log_level_colors[] = {
    ANSI_COLOR_BLUE,
    ANSI_COLOR_GREEN,
    ANSI_COLOR_YELLOW,
    ANSI_COLOR_RED,
    ANSI_COLOR_MAGENTA
};

void log_init(LogLevel level, const char* filename) {
    g_log_level = level;
}

void log_close(void) {
}

void log_print(LogLevel level, const char *file, int line, const char *format, ...) {
    if (level < g_log_level) {
        return;
    }
    va_list args;
    va_start(args, format);
    printf("%s[%s](%s:%d):\t%s", log_level_colors[level], log_level_strings[level], file, line, ANSI_COLOR_RESET);
    vprintf(format, args);
    printf("\r\n");
    va_end(args);
    if (level == LOG_FATAL) {
        while(1);
    }
}
