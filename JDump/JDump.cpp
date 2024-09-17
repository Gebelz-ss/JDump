#include "JDump.hpp"

JDump::JDump() : vm(nullptr), env(nullptr), jvmti(nullptr), file(nullptr) {}

JDump::~JDump() {
    if (vm) {
        vm->DetachCurrentThread();
    }
}

JDump& JDump::getInstance() {
    static JDump instance;
    return instance;
}

void JDump::initialize() {
    AllocConsole();
    file.reset(freopen("CONOUT$", "w", stdout));
    std::cout << "JDump by Gebelz!\nPlease select dump dir: ";
    std::string dumpDir;
    std::getline(std::cin, dumpDir);
    setDumpDirectory(dumpDir);

    findJVMs();
    attachJVM();
    getJVMTIEnv();
    setJVMTICallbacks();
}

void JDump::shutdown() {
    if (vm) {
        vm->DetachCurrentThread();
    }
}

void JDump::setDumpDirectory(const std::string& dumpDirectory) {
    this->dumpDirectory = dumpDirectory;
}

std::string JDump::getDumpDirectory() const {
    return dumpDirectory;
}

void JDump::findJVMs() {
    jsize vmCount;
    JavaVM* tempVm;
    if (JNI_GetCreatedJavaVMs(&tempVm, 1, &vmCount) != JNI_OK || vmCount == 0) {
        logError("Couldn't find Java VM");
        return;
    }
    vm.reset(tempVm, [](JavaVM* vm) { vm->DestroyJavaVM(); });
    std::cout << "Successfully found Java VM\n";
}

void JDump::attachJVM() {
    JNIEnv* tempEnv;
    jint attachResult = vm->AttachCurrentThread(reinterpret_cast<void**>(&tempEnv), nullptr);
    if (attachResult != JNI_OK) {
        logError("Failed to attach to the JVM");
        return;
    }
    env.reset(tempEnv, [vm = vm](JNIEnv* env) { vm->DetachCurrentThread(); });
    std::cout << "Successfully attached to the JVM\n";
}

void JDump::getJVMTIEnv() {
    jvmtiEnv* tempJvmti;
    if (vm->GetEnv(reinterpret_cast<void**>(&tempJvmti), JVMTI_VERSION_1_2) != JNI_OK) {
        logError("Failed to get JVMTI environment");
        return;
    }
    jvmti.reset(tempJvmti, [](jvmtiEnv* jvmti) { jvmti->DisposeEnvironment(); });
    std::cout << "Successfully got JVMTI environment\n";
}

void JDump::setJVMTICallbacks() {
    jvmtiCapabilities caps;
    std::memset(&caps, 0, sizeof(caps));
    caps.can_generate_all_class_hook_events = 1;
    if (jvmti->AddCapabilities(&caps) != JVMTI_ERROR_NONE) {
        logError("Failed to add capabilities");
        return;
    }

    jvmtiEventCallbacks callbacks;
    std::memset(&callbacks, 0, sizeof(callbacks));
    callbacks.ClassFileLoadHook = [](jvmtiEnv *jvmti_env, JNIEnv *jni_env,
            jclass class_being_redefined, jobject loader, const char *name,
            jobject protection_domain, jint class_data_len,
            const unsigned char *class_data, jint *new_class_data_len,
            unsigned char **new_class_data) {
        std::string className = name;
        std::replace(className.begin(), className.end(), '/', '.');
        ClassDumper dumper(JDump::getInstance().getDumpDirectory());
        dumper.dumpClass(className, class_data, class_data_len);
    };

    if (jvmti->SetEventCallbacks(&callbacks, sizeof(callbacks)) != JVMTI_ERROR_NONE) {
        logError("Failed to set event callbacks");
        return;
    }

    if (jvmti->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_CLASS_FILE_LOAD_HOOK, nullptr) != JVMTI_ERROR_NONE) {
        logError("Failed to enable class file load hook");
        return;
    }
}

void JDump::logError(const std::string& message) {
    std::cerr << "Error: " << message << "\n";
    if (file) {
        fclose(file.get());
    }
    file.reset();
}

ClassDumper::ClassDumper(const std::string& dumpDirectory) : dumpDirectory(dumpDirectory) {}

void ClassDumper::dumpClass(const std::string& className, const unsigned char* classData, jint classDataLen) {
    std::string filePath = dumpDirectory + "\\" + className + ".class";
    std::string directoryPath = dumpDirectory + "\\" + className.substr(0, className.find_last_of('.'));
    createDirectory(directoryPath);

    std::ofstream outFile(filePath, std::ios::binary);
    if (outFile.is_open()) {
        outFile.write(reinterpret_cast<const char*>(classData), classDataLen);
        outFile.close();
        std::cout << "Dumped class: " << className << " to " << filePath << "\n";
    } else {
        std::cout << "Failed to open file for class: " << className << "\n";
    }
}

void ClassDumper::createDirectory(const std::string& path) {
    fs::path dirPath(path);
    if (!fs::exists(dirPath)) {
        if (!fs::create_directories(dirPath)) {
            std::cout << "Failed to create directory: " << path << "\n";
        }
    }
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        JDump::getInstance().initialize();
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:
        JDump::getInstance().shutdown();
        break;
    }
    return TRUE;
}
