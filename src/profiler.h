#ifndef MY_PROFILER_H
#define MY_PROFILER_H

#include <chrono>
#include <string>
#include <unordered_map>
#include <stdexcept>
#include <iostream>
#include <iomanip> // for formatting (setw, setprecision, left, etc.)

class Profiler {
public:
    static Profiler& getInstance() {
        static Profiler instance;
        return instance;
    }

    // Register a new operation with initial total time = 0
    void addOperation(const std::string& operationName) {
        if (totalTimes.find(operationName) == totalTimes.end()) {
            totalTimes[operationName] = 0.0;
        }
    }

    // Start timing the given operation
    void start(const std::string& operationName) {
        // Make sure operation was added first
        if (totalTimes.find(operationName) == totalTimes.end()) {
            throw std::runtime_error("Operation '" + operationName + "' not registered with Profiler.");
        }
        startTimes[operationName] = std::chrono::high_resolution_clock::now();
    }

    // Stop timing the given operation and accumulate total time
    void stop(const std::string& operationName) {
        auto it = startTimes.find(operationName);
        if (it == startTimes.end()) {
            throw std::runtime_error("Attempted to stop operation '" + operationName + "' that was never started.");
        }
        auto endTime = std::chrono::high_resolution_clock::now();
        double elapsed = std::chrono::duration<double>(endTime - it->second).count();
        totalTimes[operationName] += elapsed;

        // Erase the start time from the map (so repeated usage won't collide)
        startTimes.erase(it);
    }

    // Retrieve total elapsed time for a single operation
    double getTotalTime(const std::string& operationName) const {
        auto it = totalTimes.find(operationName);
        if (it != totalTimes.end()) {
            return it->second;
        }
        return 0.0;
    }

    // Print the time statistics for every operation,
    // plus the percentage relative to "preprocessing".
    // (No "percentage of total" is displayed.)
    void printAllStatistics() const {
        // Grab total time of "preprocessing" (if it exists)
        double preprocessingTime = 0.0;
        auto preprocIt = totalTimes.find("preprocessing");
        if (preprocIt != totalTimes.end()) {
            preprocessingTime = preprocIt->second;
        }

        // Prepare formatting
        std::ios_base::fmtflags oldFlags = std::cout.flags();
        auto oldPrecision = std::cout.precision();

        std::cout << std::fixed << std::setprecision(6);

        // Print header
        std::cout << "================ Profiler Statistics ================\n\n";
        // Column titles
        std::cout << std::left
                  << std::setw(25) << "Operation"
                  << std::setw(15) << "Time (s)"
                  << std::setw(20) << "% of preprocessing"
                  << "\n";

        // Print a separator line
        std::cout << std::string(25 + 15 + 20, '-') << "\n";

        // Print each operation's stats
        for (const auto& [opName, opTime] : totalTimes) {
            // Percentage of preprocessing
            double percentagePreproc = (preprocessingTime > 0.0)
                                          ? (opTime / preprocessingTime * 100.0)
                                          : 0.0;
            std::string preprocStr    = std::to_string(percentagePreproc) + "%";

            std::cout << std::left
                      << std::setw(25) << opName
                      << std::setw(15) << opTime
                      << std::setw(20) << preprocStr
                      << "\n";
        }
        std::cout << "\n=====================================================\n\n";

        // Restore previous formatting
        std::cout.flags(oldFlags);
        std::cout.precision(oldPrecision);
    }

private:
    // Private constructor for singleton
    Profiler() = default;
    ~Profiler() = default;
    Profiler(const Profiler&) = delete;
    Profiler& operator=(const Profiler&) = delete;

    // Map: operation name -> total accumulated time in seconds
    std::unordered_map<std::string, double> totalTimes;
    // Map: operation name -> last start time point
    std::unordered_map<std::string, std::chrono::high_resolution_clock::time_point> startTimes;
};

// Macros for convenience
#define ADD_PROFILER_OPERATION(opName) \
    Profiler::getInstance().addOperation(#opName)

#define PROFILER_START(opName) \
    Profiler::getInstance().start(#opName)

#define PROFILER_STOP(opName) \
    Profiler::getInstance().stop(#opName)

#endif // MY_PROFILER_H
