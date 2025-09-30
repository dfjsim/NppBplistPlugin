#include "Logger.h"
#include <Windows.h>
#include <ctime>

namespace bplist
{
  Logger& Logger::GetInstance()
  {
    static Logger instance;
    return instance;
  }

  Logger::~Logger()
  {
    if (logFile_.is_open())
    {
      logFile_.flush();
      logFile_.close();
    }
  }

  void Logger::Initialize(const std::wstring& logFilePath)
  {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (initialized_)
    {
      if (logFile_.is_open())
        logFile_.close();
    }

    logFilePath_ = logFilePath;
    logFile_.open(logFilePath_, std::ios::out | std::ios::app);
    
    if (logFile_.is_open())
    {
      initialized_ = true;
      logFile_ << "\n========================================\n";
      logFile_ << "NppBplistPlugin Logger Initialized\n";
      logFile_ << "Time: " << GetTimestamp() << "\n";
      logFile_ << "========================================\n";
      logFile_.flush();
    }
  }

  void Logger::SetLogLevel(LogLevel level)
  {
    std::lock_guard<std::mutex> lock(mutex_);
    currentLevel_ = level;
  }

  void Logger::Log(LogLevel level, const std::string& message)
  {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_ || !logFile_.is_open())
      return;

    if (level < currentLevel_)
      return;

    logFile_ << "[" << GetTimestamp() << "] "
             << "[" << GetLevelString(level) << "] "
             << message << "\n";
    logFile_.flush();
  }

  void Logger::Log(LogLevel level, const std::wstring& message)
  {
    Log(level, WStringToString(message));
  }

  void Logger::Flush()
  {
    std::lock_guard<std::mutex> lock(mutex_);
    if (logFile_.is_open())
      logFile_.flush();
  }

  std::string Logger::GetTimestamp()
  {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
    
    std::tm tm;
    localtime_s(&tm, &time);
    
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    oss << '.' << std::setfill('0') << std::setw(3) << ms.count();
    
    return oss.str();
  }

  std::string Logger::GetLevelString(LogLevel level)
  {
    switch (level)
    {
    case LogLevel::Debug:   return "DEBUG";
    case LogLevel::Info:    return "INFO ";
    case LogLevel::Warning: return "WARN ";
    case LogLevel::Error:   return "ERROR";
    default:                return "UNKNOWN";
    }
  }

  std::string Logger::WStringToString(const std::wstring& wstr)
  {
    if (wstr.empty())
      return std::string();

    int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), NULL, 0, NULL, NULL);
    std::string strTo(sizeNeeded, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), &strTo[0], sizeNeeded, NULL, NULL);
    return strTo;
  }

} // namespace bplist
