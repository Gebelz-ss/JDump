#ifndef CLASSDUMPER_HPP_
#define CLASSDUMPER_HPP_

#include "IClassDumper.hpp"
#include <string>
#include <filesystem>

namespace fs = std::filesystem;

class ClassDumper : public IClassDumper {
public:
    ClassDumper(const std::string& dumpDirectory);
    void dumpClass(const std::string& className, const unsigned char* classData, jint classDataLen) override;

private:
    std::string dumpDirectory;
    void createDirectory(const std::string& path);
};

#endif /* CLASSDUMPER_HPP_ */
