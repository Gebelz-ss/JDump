#ifndef ICLASSDUMPER_HPP_
#define ICLASSDUMPER_HPP_

#include <string>
#include <jni.h>
class IClassDumper {
public:
	virtual ~IClassDumper() = default;
	virtual void dumpClass(const std::string &className,
			const unsigned char *classData, jint classDataLen) = 0;
};

#endif /* ICLASSDUMPER_HPP_ */
