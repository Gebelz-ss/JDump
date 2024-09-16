/*
 * JDump.hpp
 *
 *  Created on: 16 сент. 2024 г.
 *      Author: Home
 */

#ifndef JDUMP_HPP_
#define JDUMP_HPP_
#include <jvmti.h>
#include <jni.h>
#include <iostream>
#include <string>
#include <windows.h>
class JDump {
public:
	JDump();
	virtual ~JDump();
	void init(HMODULE h);
	JavaVM *vm;
	JNIEnv *env;
	FILE *file;
};

#endif
