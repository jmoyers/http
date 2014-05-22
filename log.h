#include <string.h>
#include <stdio.h>

#define __PFILE__ (strrchr(__FILE__, '/')                               \
    ? strrchr(__FILE__, '/') + 1 : __FILE__)                            \

#ifdef DEBUG
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

#define ERR(format, ...)                                                \
  LOG("err", format, ##__VA_ARGS__)                                     \

#define DEB(format, ...)                                                \
  LOG("deb", format, ##__VA_ARGS__)                                     \

#else
#define LOG(...)
#define ERR(...)
#define DEB(...)
#endif
