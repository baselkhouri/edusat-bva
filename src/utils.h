#include <iostream>
#include <chrono>

// A small RAII helper that records a start time in the constructor,
// and in the destructor it prints the message and the elapsed time.
class TimeBlockHelper {
public:
    // Constructor: store the message, record start time
    explicit TimeBlockHelper(const char* msg)
        : message(msg), start(std::chrono::steady_clock::now())
    {}

    // Destructor: measure end time, compute elapsed, and print
    ~TimeBlockHelper()
    {
        auto end = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration<double>(end - start).count();
        std::cout << message << " ~ " << elapsed << " seconds\n";
    }

private:
    const char* message;
    std::chrono::steady_clock::time_point start;
};

// Helper macros to create unique variable names
#define UNIQUE_VAR_NAME_INNER(line) timeBlockObj_##line
#define UNIQUE_VAR_NAME(line) UNIQUE_VAR_NAME_INNER(line)

// The user-facing macro that you place in a scope to measure time
// The "msg" argument is the message you want printed along with the time.
#define TIME_BLOCK(msg) TimeBlockHelper UNIQUE_VAR_NAME(__LINE__)(msg)
