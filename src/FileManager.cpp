#include <filesystem>
#include <ctime>
#include <iomanip>
#include <sstream>

#include "../header/FileManager.h"

void FileManager::trade_log(const std::string& symbol, const std::string& log_type, const std::string& content)
{
    std::lock_guard<std::mutex> lock(file_mutex);

    //cria uma pasta de logs, caso ela não exista
    std::filesystem::create_directories("logs");

    //gera arquivo baseado na data e na moeda
    auto timing = std::time(nullptr);
    auto local_time = *std::localtime(&timing);

    std::ostringstream filename_stream;
    filename_stream << "logs/" << symbol << "_" << std::put_time(&local_time, "%d-%m-%Y") << ".log";

    std::ofstream file(filename_stream.str(), std::ios::app);
    if(file.is_open())
    {
        file << "[" << std::put_time(&local_time, "%H:%M:%S") << "]" "[" << log_type << "]" << content << "\n";
        file.close();
    }
};