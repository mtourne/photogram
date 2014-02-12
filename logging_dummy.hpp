#ifndef LOGGING_DUMMY_HPP
#define LOGGING_DUMMY_HPP

// logging_dummy replaces logging.hpp for compilers that don't support

#include <iostream>
#include <stdio.h>
#include <string>

#define DEBUG 0
#define INFO 1
#define WARNING 2
#define ERROR 3
#define FATAL 4

#ifndef LOGLEVEL
#define LOGLEVEL DEBUG
#endif

#define _INITIALIZE_EASYLOGGINGPP

class Log {
public:
    bool operator&(std::ostream& stream) {
        stream << std::endl;
        return true;
    }
};

inline std::string log_level_to_str(const int log_level) {
    std::string str = "";

    switch(log_level) {
        case DEBUG:
            str = "[DEBUG]";
            break;
        case INFO:
            str = "[INFO]";
            break;
        case WARNING:
            str = "[WARNING]";
            break;
        case ERROR:
            str = "[ERROR]";
            break;
        case FATAL:
            str = "[FATAL]";
            break;
        default:
            break;
    }

    return str;
}

/* If the minimum log level requirement is satisfied then prints the log
message. This works because the & operator has lower precedence than the
<< operator, therefore the stream output is evaluated before the Log
class is initialised. */
#define LOG(log_level) log_level >= LOGLEVEL && \
                            Log() & std::cerr << \
                            log_level_to_str(log_level) <<  \
                            __FILE__ << ':' << __LINE__ << ": "
#endif
