#include "JDump.hpp"
#include "IClassDumper.hpp"
#include "ClassDumper.hpp"

JDump::JDump() :
		vm(nullptr), env(nullptr), jvmti(nullptr), file(nullptr) {
	InitializeCriticalSection(&logCriticalSection);
}

JDump::~JDump() {
	if (vm) {
		vm->DetachCurrentThread();
	}
	DeleteCriticalSection(&logCriticalSection);
}

JDump& JDump::getInstance() {
	static JDump instance;
	return instance;
}

void JDump::initialize() {
	AllocConsole();
	file.reset(freopen("CONOUT$", "w", stdout));
	FILE *inputFile = freopen("CONIN$", "r", stdin);
	if (!inputFile) {
		logError("Failed to open console input");
		return;
	}

	std::cout << "JDump by Gebelz!\nPlease select dump dir: ";
	std::string dumpDir = getUserInputDirectory();
	setDumpDirectory(dumpDir);

	findJVMs();
	attachJVM();
	getJVMTIEnv();
	setJVMTICallbacks();
}

void JDump::shutdown() {
	if (vm) {
		std::cout << "Exiting!\n";
		vm->DetachCurrentThread();
	}
}

void JDump::setDumpDirectory(const std::string &dumpDirectory) {
	this->dumpDirectory = dumpDirectory;
}

std::string JDump::getDumpDirectory() const {
	return dumpDirectory;
}

void JDump::findJVMs() {
	jsize vmCount;
	JavaVM *tempVm;
	if (JNI_GetCreatedJavaVMs(&tempVm, 1, &vmCount) != JNI_OK || vmCount == 0) {
		logError("Couldn't find Java VM");
		return;
	}
	vm.reset(tempVm, [](JavaVM *vm) {
		vm->DestroyJavaVM();
	});
	std::cout << "Successfully found Java VM\n";
}

void JDump::attachJVM() {
	JNIEnv *tempEnv;
	jint attachResult = vm->AttachCurrentThread(
			reinterpret_cast<void**>(&tempEnv), nullptr);
	if (attachResult != JNI_OK) {
		logError("Failed to attach to the JVM");
		return;
	}
	env.reset(tempEnv, [vm = vm](JNIEnv *env) {
		vm->DetachCurrentThread();
	});
	std::cout << "Successfully attached to the JVM\n";
}

void JDump::getJVMTIEnv() {
	jvmtiEnv *tempJvmti;
	if (vm->GetEnv(reinterpret_cast<void**>(&tempJvmti),
			JVMTI_VERSION_1_2) != JNI_OK) {
		logError("Failed to get JVMTI environment");
		return;
	}
	jvmti.reset(tempJvmti, [](jvmtiEnv *jvmti) {
		jvmti->DisposeEnvironment();
	});
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
		std::replace(className.begin(), className.end(), '.', '/');
		ClassDumper dumper(JDump::getInstance().getDumpDirectory());
		try {
			dumper.dumpClass(className, class_data, class_data_len);
		} catch (const std::exception &e) {
			std::cerr << "Error dumping class " << className << ": " << e.what()
					<< "\n";
		}
	};

	if (jvmti->SetEventCallbacks(&callbacks, sizeof(callbacks))
			!= JVMTI_ERROR_NONE) {
		logError("Failed to set event callbacks");
		return;
	}

	if (jvmti->SetEventNotificationMode(JVMTI_ENABLE,
			JVMTI_EVENT_CLASS_FILE_LOAD_HOOK, nullptr) != JVMTI_ERROR_NONE) {
		logError("Failed to enable class file load hook");
		return;
	}
}

void JDump::logError(const std::string &message) {
	EnterCriticalSection(&logCriticalSection);
	std::cerr << "Error: " << message << "\n";
	if (file) {
		fclose(file.get());
	}
	file.reset();
	LeaveCriticalSection(&logCriticalSection);
}

std::string JDump::getUserInputDirectory() {
	std::string dumpDir;
	std::getline(std::cin, dumpDir);
	return dumpDir;
}
