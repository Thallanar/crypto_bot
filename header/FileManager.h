#include <string>
#include <fstream>
#include <mutex>

#pragma once

class FileManager 
{
private:
    std::mutex file_mutex;

    std::string symbol;

public:
    FileManager(std::string symbol_ref) : symbol(symbol_ref) {};
    
    void trade_log(const std::string& symbol, const std::string& log_type, const std::string& content);
};