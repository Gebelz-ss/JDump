#ifndef JDUMP_HPP_
#define JDUMP_HPP_

#include "IJDump.hpp"
#include <jvmti.h>
#include <iostream>
#include <windows.h>
#include <fstream>
#include <algorithm>
#include <string>
#include <memory>
#include <filesystem>
#include <stdexcept>
#include <cstring>
#include <sstream>

namespace fs = std::filesystem;

class JDump : public IJDump {
public:
    static JDump& getInstance();
    void initialize() override;
    void shutdown() override;
    void setDumpDirectory(const std::string& dumpDirectory) override;
    std::string getDumpDirectory() const override;

private:
    JDump();
    ~JDump();

    void findJVMs();
    void attachJVM();
    void getJVMTIEnv();
    void setJVMTICallbacks();
    void logError(const std::string& message);
    std::string getUserInputDirectory();

    std::shared_ptr<JavaVM> vm;
    std::shared_ptr<JNIEnv> env;
    std::shared_ptr<jvmtiEnv> jvmti;
    std::shared_ptr<FILE> file;
    std::string dumpDirectory;
    CRITICAL_SECTION logCriticalSection;
};

#endif /* JDUMP_HPP_ */
