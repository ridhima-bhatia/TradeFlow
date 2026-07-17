#ifndef CONFIG_H
#define CONFIG_H

#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <iostream>

// Minimal key=value config file reader.
// Lines starting with '#' are comments. Missing file or missing keys fall
// back to sane defaults so the server still runs out of the box.
class Config
{
private:
    std::unordered_map<std::string, std::string> values;

public:
    explicit Config(const std::string& path)
    {
        std::ifstream file(path);
        if (!file.is_open())
        {
            std::cout << "No config file found at '" << path
                       << "', using defaults.\n";
            return;
        }

        std::string line;
        while (std::getline(file, line))
        {
            if (line.empty() || line[0] == '#')
                continue;

            auto eq = line.find('=');
            if (eq == std::string::npos)
                continue;

            std::string key = line.substr(0, eq);
            std::string value = line.substr(eq + 1);
            values[key] = value;
        }
    }

    int getInt(const std::string& key, int fallback) const
    {
        auto it = values.find(key);
        if (it == values.end()) return fallback;
        try { return std::stoi(it->second); }
        catch (...) { return fallback; }
    }

    std::string getString(const std::string& key, const std::string& fallback) const
    {
        auto it = values.find(key);
        return (it == values.end()) ? fallback : it->second;
    }
};

#endif
