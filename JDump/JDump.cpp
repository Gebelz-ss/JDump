#include "JDump.hpp"

#include <jni.h>
#include <jvmti.h>
#include <iostream>
#include <string>
#include <windows.h>
#include <fstream>
#include <vector>
#include <algorithm>

JavaVM *vm;
JNIEnv *env = nullptr;
FILE *file = nullptr;
jvmtiEnv *jvmti = nullptr;

void init(HMODULE h) {
    AllocConsole();
    freopen_s(&file, "CONOUT$", "w", stdout);

    jsize vmCount;
    if (JNI_GetCreatedJavaVMs(&vm, 1, &vmCount) != JNI_OK || vmCount == 0) {
        std::cout << "Couldn't find java vm\n";
        return;
    }
    std::cout << "Successfully found Java VM\n";

    // Attach to the JVM
    jint attachResult = vm->AttachCurrentThread((void**)&env, nullptr);
    if (attachResult != JNI_OK) {
        std::cout << "Failed to attach to the JVM\n";
        return;
    }
    std::cout << "Successfully attached to the JVM\n";

    // Get JVMTI environment
    if (vm->GetEnv((void**)&jvmti, JVMTI_VERSION_1_2) != JNI_OK) {
        std::cout << "Failed to get JVMTI environment\n";
        return;
    }
    std::cout << "Successfully got JVMTI environment\n";

    // Enable class file load hook
    jvmtiCapabilities caps;
    memset(&caps, 0, sizeof(caps));
    caps.can_generate_all_class_hook_events = 1;
    if (jvmti->AddCapabilities(&caps) != JVMTI_ERROR_NONE) {
        std::cout << "Failed to add capabilities\n";
        return;
    }

    jvmtiEventCallbacks callbacks;
    memset(&callbacks, 0, sizeof(callbacks));
    callbacks.ClassFileLoadHook = [](jvmtiEnv *jvmti_env, JNIEnv* jni_env, jclass class_being_redefined, jobject loader, const char* name, jobject protection_domain, jint class_data_len, const unsigned char* class_data, jint* new_class_data_len, unsigned char** new_class_data) {
        std::string className = name;
        std::replace(className.begin(), className.end(), '/', '.');

        // Create the full path for the class file
        std::string filePath = "C:\\Dump\\" + className + ".class";

        // Ensure the directory exists
        std::string directoryPath = "C:\\Dump\\";
        if (!CreateDirectoryA(directoryPath.c_str(), NULL) && ERROR_ALREADY_EXISTS != GetLastError()) {
            std::cout << "Failed to create directory: " << directoryPath << "\n";
            return;
        }

        std::ofstream outFile(filePath, std::ios::binary);
        if (outFile.is_open()) {
            outFile.write((const char*)class_data, class_data_len);
            outFile.close();
            std::cout << "Dumped class: " << className << " to " << filePath << "\n";
        } else {
            std::cout << "Failed to open file for class: " << className << "\n";
        }
    };

    if (jvmti->SetEventCallbacks(&callbacks, sizeof(callbacks)) != JVMTI_ERROR_NONE) {
        std::cout << "Failed to set event callbacks\n";
        return;
    }

    if (jvmti->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_CLASS_FILE_LOAD_HOOK, nullptr) != JVMTI_ERROR_NONE) {
        std::cout << "Failed to enable class file load hook\n";
        return;
    }
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        init(hModule);
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}
