﻿#pragma once
#include "IniFile.h"
#include <string>
#include <fstream>
#include <sstream>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <stdexcept>
#include <memory>

// L = Line
// F = File
// K = Key

#define LOG Log::Instance->logstream(Severity::DEFAULT)
#define INFOL(line) Log::Instance->stream(Severity::INFO, line)
#define WARNINGL(line) Log::Instance->stream(Severity::WARNING, line)
#define ERRORL(line) Log::Instance->stream(Severity::ERROR, line)
#define INFOF(section, filename, line) Log::Instance->stream(Severity::INFO, section, filename, line)
#define WARNINGF(section, filename, line) Log::Instance->stream(Severity::WARNING, section, filename, line)
#define ERRORF(section, filename, line) Log::Instance->stream(Severity::ERROR, section, filename, line)
#define INFOK(section, key) Log::Instance->stream(Severity::INFO, section, key)
#define WARNINGK(section, key) Log::Instance->stream(Severity::WARNING, section, key)
#define ERRORK(section, key) Log::Instance->stream(Severity::ERROR, section, key)

// 日志级别
enum class Severity {
    DEFAULT,
    INFO,
    WARNING,
    ERROR
};

// 日志类
class LogStream;
class Log {
public:
    static Log* Instance; 
	static std::vector<LogStream> Logs;
	static bool CanOutput;

    Log(const std::string& logFileName);
    ~Log();

	LogStream logstream(Severity severity);
    LogStream& stream(Severity severity, int line = -2);
    LogStream& stream(Severity severity, const Section& section, const std::string& key);
	LogStream& stream(Severity severity, const std::string& section, const std::string& filename, const int& line);

	void output();
    void stop();

private:
    friend class LogStream;

    void processLogQueue();
    void writeLog(const std::string& message);
    std::string getSeverityLabel(Severity severity);
    std::string getPlainSeverityLabel(Severity severity);

    std::ofstream logFile;
    std::queue<std::string> logQueue;
    std::mutex queueMutex;
    std::condition_variable condition;
    std::thread writerThread;
    bool running;

};

// 流式日志操作
class LogStream {
public:
	LogStream(Log* logger, Severity severity, int line);
	LogStream(Log* logger, Severity severity, std::string filename, std::string section, std::string key, std::string value, int line);
	~LogStream(); // 析构时提交日志

	int getline() const { return line; }

	template <typename T>
	LogStream& operator<<(const T& value) {
		std::ostringstream oss;
		oss << value;
		buffer += oss.str();
		return *this;
	}
	LogStream& operator<<(const Value& value) {
		buffer += value.value;
		return *this;
	}
private:
	Log* logger;
	Severity severity;
	int line;
	std::string filename{};
	std::string section{};
	std::string key{};
	std::string value{};
    std::string buffer;
};