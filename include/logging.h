#ifndef PCROAST_LOGGING_H
#define PCROAST_LOGGING_H

#ifndef LOG_PRINT_FUNC
#define LOG_PRINT_FUNC printf
#endif

#define NEWLINE "\n"
#define LOG_FORMAT "%-7s %s:%s:%d: "
#define LOG_ARGS(TAG) TAG, __FILE_NAME__, __FUNCTION__, __LINE__
#define LOG_PRINT(message, ...) LOG_PRINT_FUNC(LOG_FORMAT message NEWLINE, __VA_ARGS__)

#if DEBUG_BUILD
#define LOG_DEBUG(message, args...) LOG_PRINT(message, LOG_ARGS("DEBUG"), ##args)
#else
#define LOG_DEBUG(message, args...)
#endif

#define LOG_ERROR(message, args...) LOG_PRINT(message, LOG_ARGS("ERROR"), ##args)
#define LOG_WARNING(message, args...) LOG_PRINT(message, LOG_ARGS("WARNING"), ##args)
#define LOG_INFO(message, args...) LOG_PRINT(message, LOG_ARGS("INFO"), ##args)

#endif  // PCROAST_LOGGING_H
