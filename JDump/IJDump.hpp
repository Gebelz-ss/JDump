#ifndef IJDUMP_HPP_
#define IJDUMP_HPP_

#include <string>

class IJDump {
public:
    virtual ~IJDump() = default;
    virtual void initialize() = 0;
    virtual void shutdown() = 0;
    virtual void setDumpDirectory(const std::string& dumpDirectory) = 0;
    virtual std::string getDumpDirectory() const = 0;
};

#endif /* IJDUMP_HPP_ */
