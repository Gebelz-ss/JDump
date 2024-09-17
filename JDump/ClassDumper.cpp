#include "ClassDumper.hpp"
#include <fstream>
#include <sstream>
#include <iostream>

ClassDumper::ClassDumper(const std::string& dumpDirectory) : dumpDirectory(dumpDirectory) {}

void ClassDumper::dumpClass(const std::string& className, const unsigned char* classData, jint classDataLen) {
    std::string filePath;
    if (className.find('/') != std::string::npos) {
        std::string directoryPath = dumpDirectory + "/" + className.substr(0, className.find_last_of('/'));
        createDirectory(directoryPath);
        filePath = dumpDirectory + "/" + className + ".class";
    } else {
        filePath = dumpDirectory + "/" + className + ".class";
    }

    std::ofstream outFile(filePath, std::ios::binary);
    if (outFile.is_open()) {
        outFile.write(reinterpret_cast<const char*>(classData), static_cast<std::streamsize>(classDataLen));
        outFile.close();
        std::cout << "Dumped class: " << className << " to " << filePath << "\n";
    } else {
        std::stringstream ss;
        ss << "Failed to open file for class: " << className;
        throw std::runtime_error(ss.str());
    }
}

void ClassDumper::createDirectory(const std::string& path) {
    fs::path dirPath(path);
    if (!fs::exists(dirPath)) {
        if (!fs::create_directories(dirPath)) {
            std::stringstream ss;
            ss << "Failed to create directory: " << path;
            throw std::runtime_error(ss.str());
        }
    }
}
