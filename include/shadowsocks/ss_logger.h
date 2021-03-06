#ifndef __SHADOWSOCKS_LOGGER_INCLUDED__
#define __SHADOWSOCKS_LOGGER_INCLUDED__


#include "shadowsocks/ss_types.h"


/* runtime get index of tuple */
template <size_t I>
struct VisitImpl {
    template <typename Type, typename Func>
    static void visit(Type &tuple, size_t idx, Func func) {
        if (idx == I - 1) {
            func(std::get<I - 1>(tuple));
        } else {
            VisitImpl<I - 1>::visit(tuple, idx, func);
        }
    }
};

template <>
struct VisitImpl<0> {
    template <typename Type, typename F>
    static void visit(Type &tuple, size_t idx, F fun) {
        assert(false);
    }
};

template <typename Func, typename ...Types>
void visit(const std::tuple<Types...> &tuple, size_t idx, Func func) {
    VisitImpl<sizeof...(Types)>::visit(tuple, idx, func);
}

class TupleElementPrinter {
    public:
        explicit TupleElementPrinter(std::ostream &out);
        void setFlag(std::ios::fmtflags flags);

        template<typename tuple>
        void operator()(tuple value) {
            _output << value;
            if (_defaultFlags) {
                _output.setf(_defaultFlags);
                _defaultFlags = static_cast<std::ios_base::fmtflags>(0);
            }
        }

    private:
        std::ostream &_output;
        std::ios::fmtflags _defaultFlags;
};


class SsLogger {
    public:
        enum class LoggerLevel : uint8_t {
            LL_VERBOSE      = 0x00,
            LL_DEBUG        = 0x10,
            LL_INFO         = 0x20,
            LL_WARNING      = 0x40,
            LL_ERROR        = 0x80,
            LL_EMERGENCY    = 0xff
        };

        using Format = const char *;
        using LoggerName = const char *;
        using SsLoggerPtr = std::shared_ptr<SsLogger>;

    public:
        explicit SsLogger(std::ostream &out);
        virtual ~SsLogger();

        void setLevel(LoggerLevel level);
        void setName(LoggerName name);
        static void addLogger(LoggerName name, SsLoggerPtr logger);
        static bool removeLogger(LoggerName name);

        template <typename ...Args>
        static void verbose(Format fmt, Args ...args);

        template <typename ...Args>
        static void debug(Format fmt, Args ...args);

        template <typename ...Args>
        static void info(Format fmt, Args ...args);

        template <typename ...Args>
        static void warning(Format fmt, Args ...args);

        template <typename ...Args>
        static void error(Format fmt, Args ...args);

        template <typename ...Args>
        static void emergency(Format fmt, Args ...args);

        template <typename ...Args>
        static std::string log(SsLogger::LoggerLevel level,
                               Format fmt, Args ...args);

        template <typename ...Args>
        static std::string format(Format fmt, Args ...args);

    private:
        static std::string currentDate(Format fmt);
        static void log(LoggerLevel level, std::string message);

    private:
        LoggerName _name = nullptr;
        std::ostream &_output;
        std::string _dateFormat = "%A %b %d %H:%M:%S %Y \t->\t ";
        LoggerLevel _level = LoggerLevel::LL_INFO;
        static std::map<SsLogger::LoggerName, SsLogger::SsLoggerPtr> _loggers;

    friend std::ostream &operator<<(std::ostream &out, SsLogger *logger);
};


/* utility methods declare */
std::ostream &operator<<(std::ostream &out, const SsLogger::LoggerLevel &level);


// all the things that happened
template<typename ...Args>
void SsLogger::verbose(SsLogger::Format fmt, Args... args) {
    log(LoggerLevel::LL_VERBOSE, format(fmt, args...));
}

// detailed debug information
template<typename ...Args>
void SsLogger::debug(SsLogger::Format fmt, Args... args) {
    log(LoggerLevel::LL_DEBUG, format(fmt, args...));
}

// interesting events.
template<typename ...Args>
void SsLogger::info(SsLogger::Format fmt, Args... args) {
    log(LoggerLevel::LL_INFO, format(fmt, args...));
}

// exceptional occurrences that are not errors.
template<typename ...Args>
void SsLogger::warning(SsLogger::Format fmt, Args... args) {
    log(LoggerLevel::LL_WARNING, format(fmt, args...));
}

// runtime errors that do not require immediate action but should typically
// be logged and monitored.
template<typename ...Args>
void SsLogger::error(SsLogger::Format fmt, Args... args) {
    log(LoggerLevel::LL_ERROR, format(fmt, args...));
}

// system is unusable, will be exit
template<typename ...Args>
void SsLogger::emergency(SsLogger::Format fmt, Args... args) {
    log(LoggerLevel::LL_EMERGENCY, format(fmt, args...));
    std::exit(OPERATOR_FAILURE);
}

// custom log message and return message
template<typename ...Args>
std::string SsLogger::log(SsLogger::LoggerLevel level,
                          SsLogger::Format fmt, Args... args) {
    std::string message = format(fmt, args...);

    log(level, message);
    if (level == SsLogger::LoggerLevel::LL_EMERGENCY) {
        std::exit(OPERATOR_FAILURE);
    }

    return message;
}

// format parameters and output it
template<typename ...Args>
std::string SsLogger::format(Format fmt, Args... args) {
    std::stringstream ss;
    TupleElementPrinter printer(ss);

    auto tuple = std::make_tuple(args...);
    size_t tupleSize = std::tuple_size<std::tuple<Args...>>::value;

    size_t index = 0;
    auto output = [&] (size_t &i) {
        if (index < tupleSize) {
            visit(tuple, index++, printer);
            while (isalnum(fmt[++i])) {
                ;
            }
            --i;
        } else {
            printer('%');
        }
    };

    for (size_t i = 0; i < strlen(fmt); ++i) {
        if (fmt[i] != '%') {
            ss << fmt[i];
        } else {
            /**
             * @todo hex oct and dec
             */
            switch (fmt[i + 1]) {
                case '%': ss << fmt[i++]; break;  // escape %
                case 'x': {
                    ss << "0x";
                    printer.setFlag(std::ios::hex);
                }
                default: output(++i);
            }
        }
    }

    return ss.str();
}


#define VVV(FMT, ARGS...)           SsLogger::verbose(FMT, ##ARGS)
#define DBG(FMT, ARGS...)           SsLogger::debug(FMT, ##ARGS)
#define INF(FMT, ARGS...)           SsLogger::info(FMT, ##ARGS)
#define WARN(FMT, ARGS...)          SsLogger::warning(FMT, ##ARGS)
#define ERR(FMT, ARGS...)           SsLogger::error(FMT, ##ARGS)
#define EXT(FMT, ARGS...)           SsLogger::emergency(FMT, ##ARGS)


#endif // __SHADOWSOCKS_LOGGER_INCLUDED__
