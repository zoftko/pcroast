#ifndef PCROAST_LOGGING_H
#define PCROAST_LOGGING_H

#ifndef LOG_PRINT_FUNC
#define LOG_PRINT_FUNC printf
#endif

#define NEWLINE "\n"
#define LOG_FORMAT "%-7s: "
#define LOG_PRINT(message, ...) LOG_PRINT_FUNC(LOG_FORMAT message NEWLINE, __VA_ARGS__)

#if DEBUG_BUILD
#define LOG_DEBUG(message, args...) LOG_PRINT(message, "DEBUG", ##args)
#else
#define LOG_DEBUG(message, args...)
#endif

#define LOG_ERROR(message, args...) LOG_PRINT(message, "ERROR", ##args)
#define LOG_WARNING(message, args...) LOG_PRINT(message, "WARNING", ##args)
#define LOG_INFO(message, args...) LOG_PRINT(message, "INFO", ##args)

#endif  // PCROAST_LOGGING_H
