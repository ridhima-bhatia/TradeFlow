#ifndef TRADELOGGER_H
#define TRADELOGGER_H

#include <fstream>
#include <mutex>
#include <string>
#include <chrono>
#include <ctime>
#include <sstream>
#include <iomanip>

// Thread-safe append-only logger. Multiple worker threads may log at the
// same time, so writes are guarded by a mutex to prevent interleaved/
// corrupted lines in the file.
class TradeLogger
{
private:
    std::ofstream file;
    std::mutex logMutex;

    static std::string timestamp()
    {
        auto now = std::chrono::system_clock::now();
        std::time_t t = std::chrono::system_clock::to_time_t(now);
        std::ostringstream oss;
        oss << std::put_time(std::localtime(&t), "%Y-%m-%d %H:%M:%S");
        return oss.str();
    }

public:
    explicit TradeLogger(const std::string& path)
        : file(path, std::ios::app)
    {
        if (!file.is_open())
            throw std::runtime_error("Could not open trade log file: " + path);
    }

    void log(const std::string& entry)
    {
        std::lock_guard<std::mutex> lock(logMutex);
        file << "[" << timestamp() << "] " << entry << std::endl;
        file.flush(); // flush immediately so a crash doesn't lose recent trades
    }
};

#endif
