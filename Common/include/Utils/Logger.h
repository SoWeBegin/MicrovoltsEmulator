#ifndef CUSTOM_LOGGER_H
#define CUSTOM_LOGGER_H

#include <iostream>
#include <fstream>
#include <iomanip>
#include <chrono>
#include <ctime>
#include <string>
#include <sstream>
#include <format>
#include <memory>

namespace Utils
{
    enum class LogType
    {
        Info, Error, Warning, Normal
    };

    enum LogDestination
    {
        Console, File
    };

    class Logger {
    private:
        bool m_loggingEnabled;
        bool logToFile;
        std::ostream& outputStream;
        std::ofstream fileStream;

        // Private constructor
        Logger(bool enableLogging = true, bool logToConsole = true, const std::string& filename = "log.txt")
            : m_loggingEnabled(enableLogging), logToFile(!logToConsole)
            , outputStream(logToConsole ? std::cout : fileStream)
        {
            if (logToFile)
            {
               /* fileStream.open(filename, std::ios_base::app);
                if (!fileStream.is_open())
                {
                    std::cerr << "Error opening log file " << filename << '\n';
                }*/
            }
        }

        // Disable copy constructor and assignment operator
        Logger(const Logger&) = delete;
        Logger& operator=(const Logger&) = delete;

        // Destructor
        ~Logger() {
            if (logToFile)
            {
                //fileStream.close();
            }
        }

        inline static Logger* instance;

    public:
        static Logger& getInstance(bool enableLogging = true, bool logToConsole = true, const std::string& filename = "log.txt")
        {
            if (!instance)
            {
                instance = new Logger(enableLogging, true, filename);
            }
            return *instance;
        }

        void enableLogging()
        {
            m_loggingEnabled = true;
        }

        void disableLogging()
        {
            m_loggingEnabled = false;
        }

        template<LogDestination LogDest = LogDestination::Console>
        void log(const std::string& message, LogType type = LogType::Normal, const std::string& functionName = "")
        {
            if (m_loggingEnabled)
            {
                std::string logMessage = message;
                if (!functionName.empty())
                {
                    logMessage = "[" + functionName + "] " + logMessage;
                }
                logImpl<LogDest>(logMessage, type);
            }
        }

    private:
        template<LogDestination LogDest>
        void logImpl(const std::string& message, LogType type)
        {
            if constexpr (LogDest == LogDestination::Console)
            {
                switch (type) {
                case LogType::Info:
                    std::cout << "[Info] " << message << "\n\n";
                    break;
                case LogType::Error:
                    std::cout << "[Error] " << message << "\n\n";
                    break;
                case LogType::Normal:
                    std::cout << message << "\n\n";
                    break;
                case LogType::Warning:
                    std::cout << "[Warning] " << message << "\n\n";
                    break;
                }
            }
            else
            {
                //fileStream << getCurrentDateTime() << " [" << logTypeToString(type) << "] " << message << "\n";
            }
        }

        std::string getCurrentDateTime() const
        {
            auto now = std::chrono::system_clock::now();
            std::time_t now_c = std::chrono::system_clock::to_time_t(now);

            std::tm tm_time{};
#ifdef WIN32
            localtime_s(&tm_time, &now_c);
#else
            localtime_r(&now_c, &tm_time);
#endif
            std::stringstream ss;
            ss << std::put_time(&tm_time, "%Y-%m-%d %X");
            return ss.str();
        }

        std::string logTypeToString(LogType type)
        {
            switch (type) {
            case LogType::Info: return "Info";
            case LogType::Error: return "Error";
            case LogType::Normal: return "Normal";
            case LogType::Warning: return "Warning";
            }
            return "Unknown";
        }
    };
}

#endif
