#include <cadmium/core/logger/logger.hpp>
#include <fstream>
#include <iostream>
#include <chrono>
#include <ctime>
#include <sstream>

class RAFTLogger : public cadmium::Logger {
private:
    std::ofstream logFile;

    std::string generateTimestampedFilename() {
        auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        std::tm* now_tm = std::localtime(&now); 

        std::stringstream ss;
        ss << "simulation_log_"
           << (now_tm->tm_year + 1900) << "-"  // Year
           << (now_tm->tm_mon + 1) << "-"     // Month
           << now_tm->tm_mday << "_"
           << now_tm->tm_hour << "-"          // Hour
           << now_tm->tm_min << "-"           // Minute
           << now_tm->tm_sec << ".txt";       // Second

        return ss.str();
    }

public:

    RAFTLogger() {
        std::string filename = generateTimestampedFilename();  
        logFile.open("logs/" + filename, std::ios::out | std::ios::trunc);  
        if (!logFile) {
            std::cerr << "Error opening log file!" << std::endl;
        }
    }


    ~RAFTLogger() {
        if (logFile.is_open()) {
            logFile.close();
        }
    }

    void start() override {
        logFile << "RAFT Simulation Started." << std::endl;
    }

    void stop() override {
        logFile << "RAFT Simulation Ended." << std::endl;
    }

    void logTime(double time) override {
        logFile << "Simulation Time: " << time << std::endl;
    }

    void logOutput(double time, long modelId, const std::string& modelName, 
                   const std::string& portName, const std::string& output) override {
        logFile << "[" << time << "] "
                << " | Output size: " << output.size()  // Assuming output is a log entry
                << " | Ouput: " << output 
                << std::endl;
    }

    void logState(double time, long modelId, const std::string& modelName, 
                  const std::string& state) override {
        logFile << "[" << time << "] "
                << modelName
                << " | Current State: " << state 
                << std::endl;
    }
};
