#include <string.h>
#include <stdio.h>

#define __PFILE__ (strrchr(__FILE__, '/')                               \
    ? strrchr(__FILE__, '/') + 1 : __FILE__)                            \

#define LOG(level, format, ...)                                         \
  if (level == "err") {                                                 \
    fprintf(stderr, "[\033[31m%s\033[0m] %s: " format "\n", level,      \
        __PFILE__, ##__VA_ARGS__);                                      \
  }                                                                     \
  else                                                                  \
  {                                                                     \
    printf("[%s] %s: " format "\n", level, __PFILE__,                   \
        ##__VA_ARGS__);                                                 \
  }                                                                     \

#define ERR_LEVEL    1
#define WARN_LEVEL   2
#define DEBUG_LEVEL  3

#if LOG_LEVEL >= ERR_LEVEL
#define ERR(format, ...)                                                \
  LOG("err", format, ##__VA_ARGS__)
#else
#define ERR(...)
#endif

#if LOG_LEVEL >= WARN_LEVEL
#define WARN(format, ...)                                               \
  LOG("warn", format, ##__VA_ARGS__)
#else
#define WARN(...)
#endif

#if LOG_LEVEL >= DEBUG_LEVEL
#define DEBUG(format, ...)                                              \
  LOG("deb", format, ##__VA_ARGS__)
#else
#define DEBUG(...)
#endif
