#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include <mutex>
#include <chrono>
#include <iomanip>
#include <utility>

namespace bplist
{
  enum class LogLevel
  {
    Debug,
    Info,
    Warning,
    Error
  };

  class Logger
  {
  public:
    static Logger& GetInstance();

    void Initialize(const std::wstring& logFilePath);
    void SetLogLevel(LogLevel level);
    void Log(LogLevel level, const std::string& message);
    void Log(LogLevel level, const std::wstring& message);
    
    // Convenience methods
    void Debug(const std::string& message) { Log(LogLevel::Debug, message); }
    void Info(const std::string& message) { Log(LogLevel::Info, message); }
    void Warning(const std::string& message) { Log(LogLevel::Warning, message); }
    void Error(const std::string& message) { Log(LogLevel::Error, message); }
    
    void Debug(const std::wstring& message) { Log(LogLevel::Debug, message); }
    void Info(const std::wstring& message) { Log(LogLevel::Info, message); }
    void Warning(const std::wstring& message) { Log(LogLevel::Warning, message); }
    void Error(const std::wstring& message) { Log(LogLevel::Error, message); }

    void Flush();

  private:
    Logger() : currentLevel_(LogLevel::Debug), initialized_(false) {}
    ~Logger();

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    std::string GetTimestamp();
    std::string GetLevelString(LogLevel level);
    std::string WStringToString(const std::wstring& wstr);

    std::wstring logFilePath_;
    std::ofstream logFile_;
    std::mutex mutex_;
    LogLevel currentLevel_;
    bool initialized_;
  };

  // Utility helper to concatenate arbitrary values into a single string
  template<typename... Args>
  std::string MakeLogString(Args&&... args)
  {
    std::ostringstream oss;
    (oss << ... << std::forward<Args>(args));
    return oss.str();
  }

  // Helper macros for easier logging with context
  // Only active in Debug builds
#ifdef _DEBUG
  #define LOG_DEBUG(...) bplist::Logger::GetInstance().Debug(bplist::MakeLogString(__FUNCTION__, ": ", __VA_ARGS__))
  #define LOG_INFO(...) bplist::Logger::GetInstance().Info(bplist::MakeLogString(__FUNCTION__, ": ", __VA_ARGS__))
  #define LOG_WARNING(...) bplist::Logger::GetInstance().Warning(bplist::MakeLogString(__FUNCTION__, ": ", __VA_ARGS__))
  #define LOG_ERROR(...) bplist::Logger::GetInstance().Error(bplist::MakeLogString(__FUNCTION__, ": ", __VA_ARGS__))
#else
  // No-op in Release builds
  #define LOG_DEBUG(...) ((void)0)
  #define LOG_INFO(...) ((void)0)
  #define LOG_WARNING(...) ((void)0)
  #define LOG_ERROR(...) ((void)0)
#endif

} // namespace bplist
