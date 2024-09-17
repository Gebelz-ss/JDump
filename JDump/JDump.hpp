#ifndef JDUMP_HPP_
#define JDUMP_HPP_

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

namespace fs = std::filesystem;

class JDump {
public:
    static JDump& getInstance();
    void initialize();
    void shutdown();
    void setDumpDirectory(const std::string& dumpDirectory);
    std::string getDumpDirectory() const;

private:
    JDump();
    ~JDump();

    void findJVMs();
    void attachJVM();
    void getJVMTIEnv();
    void setJVMTICallbacks();
    void logError(const std::string& message);

    std::shared_ptr<JavaVM> vm;
    std::shared_ptr<JNIEnv> env;
    std::shared_ptr<jvmtiEnv> jvmti;
    std::shared_ptr<FILE> file;
    std::string dumpDirectory;
};

class ClassDumper {
public:
    ClassDumper(const std::string& dumpDirectory);
    void dumpClass(const std::string& className, const unsigned char* classData, jint classDataLen);

private:
    std::string dumpDirectory;
    void createDirectory(const std::string& path);
};

#endif /* JDUMP_HPP_ */
