//
// Created by lidongyooo on 2026/2/6.
//

#include "FuncPrinter.h"

const std::vector<std::string> call_jni_methods = {"CallStaticObjectMethod",
                                                 "CallStaticObjectMethodV",
                                                 "CallStaticObjectMethodA",
                                                 "CallStaticBooleanMethod",
                                                 "CallStaticBooleanMethodV",
                                                 "CallStaticBooleanMethodA",
                                                 "CallStaticByteMethod", "CallStaticByteMethodV",
                                                 "CallStaticByteMethodA",
                                                 "CallStaticCharMethod", "CallStaticCharMethodV",
                                                 "CallStaticCharMethodA",
                                                 "CallStaticShortMethod", "CallStaticShortMethodV",
                                                 "CallStaticShortMethodA",
                                                 "CallStaticIntMethod", "CallStaticIntMethodV",
                                                 "CallStaticIntMethodA",
                                                 "CallStaticLongMethod", "CallStaticLongMethodV",
                                                 "CallStaticLongMethodA",
                                                 "CallStaticFloatMethod", "CallStaticFloatMethodV",
                                                 "CallStaticFloatMethodA",
                                                 "CallStaticDoubleMethod",
                                                 "CallStaticDoubleMethodV",
                                                 "CallStaticDoubleMethodA",
                                                 "CallStaticVoidMethod", "CallStaticVoidMethodV",
                                                 "CallStaticVoidMethodA",
                                                 "CallObjectMethod", "CallObjectMethodV",
                                                 "CallObjectMethodA", "CallBooleanMethod",
                                                 "CallBooleanMethodV",
                                                 "CallBooleanMethodA", "CallByteMethod",
                                                 "CallByteMethodV", "CallByteMethodA",
                                                 "CallCharMethod", "CallCharMethodV",
                                                 "CallCharMethodA", "CallShortMethod",
                                                 "CallShortMethodV", "CallShortMethodA",
                                                 "CallIntMethod", "CallIntMethodV",
                                                 "CallIntMethodA", "CallLongMethod",
                                                 "CallLongMethodV", "CallLongMethodA",
                                                 "CallFloatMethod", "CallFloatMethodV",
                                                 "CallFloatMethodA", "CallDoubleMethod",
                                                 "CallDoubleMethodV", "CallDoubleMethodA",
                                                 "CallVoidMethod", "CallVoidMethodV",
                                                 "CallVoidMethodA", "CallNonvirtualObjectMethod",
                                                 "CallNonvirtualObjectMethodV",
                                                 "CallNonvirtualObjectMethodA",
                                                 "CallNonvirtualBooleanMethod",
                                                 "CallNonvirtualBooleanMethodV",
                                                 "CallNonvirtualBooleanMethodA",
                                                 "CallNonvirtualByteMethod",
                                                 "CallNonvirtualByteMethodV",
                                                 "CallNonvirtualByteMethodA",
                                                 "CallNonvirtualCharMethod",
                                                 "CallNonvirtualCharMethodV",
                                                 "CallNonvirtualCharMethodA",
                                                 "CallNonvirtualShortMethod",
                                                 "CallNonvirtualShortMethodV",
                                                 "CallNonvirtualShortMethodA",
                                                 "CallNonvirtualIntMethod",
                                                 "CallNonvirtualIntMethodV",
                                                 "CallNonvirtualIntMethodA",
                                                 "CallNonvirtualLongMethod",
                                                 "CallNonvirtualLongMethodV",
                                                 "CallNonvirtualLongMethodA",
                                                 "CallNonvirtualFloatMethod",
                                                 "CallNonvirtualFloatMethodV",
                                                 "CallNonvirtualFloatMethodA",
                                                 "CallNonvirtualDoubleMethod",
                                                 "CallNonvirtualDoubleMethodV",
                                                 "CallNonvirtualDoubleMethodA",
                                                 "CallNonvirtualVoidMethod",
                                                 "CallNonvirtualVoidMethodV",
                                                 "CallNonvirtualVoidMethodA",};

void FuncPrinter::params_join(FUNC_CONTEXT *func_context, uint count) {
    func_context->info[func_context->info_n++] = '(';
    for (int i = 0; i < count; i++) {
        Utils::auto_snprintf(func_context->info_n, func_context->info, "0x%llx", func_context->cpu_context.x[i]);
        if (i != count - 1) {
            func_context->info[func_context->info_n++] = ',';
            func_context->info[func_context->info_n++] = ' ';
        }
    }
    func_context->info[func_context->info_n++] = ')';
}

void FuncPrinter::read_string(int& buff_n, char *buff, char* str, size_t max_len) {
    if ((uint64_t)str <= 0x1000) {
        return;
    }

    size_t i = 0;
    while (i < max_len && str[i]) {
        buff[buff_n++] = str[i++];
    }
}


void FuncPrinter::hexdump(int& buff_n, char *buff, uint64_t address, size_t count) {
    Utils::auto_snprintf(buff_n, buff, "\nhexdump at address 0x%llx with length 0x%llx:\n", address, count);

    if (address < 0x10000) {
        return;
    }

    auto bytePtr = (char*)(address);
    if (count == 0) {
        int i = 0;
        while (bytePtr[i++]);
        count = i - 1;
    }

    size_t offset = 0;
    size_t total_lines = (count + 15) / 16;  // 向上取整
    size_t current_line = 0;

    while (offset < count) {
        current_line++;

        Utils::auto_snprintf(buff_n, buff, "%llx: ", address + offset);

        char ascii[20];
        int ascii_n = 0;
        ascii[ascii_n++] = '|';
        for (size_t i = 0; i < 16; ++i) {
            if (offset + i < count) {
                char byte = bytePtr[offset + i];

                Utils::auto_snprintf(buff_n, buff, "%02x ", byte);
                ascii[ascii_n++] = std::isprint(byte) ? byte : '.';
            } else {
                buff[buff_n++] = ' ';
                buff[buff_n++] = ' ';
                buff[buff_n++] = ' ';
                ascii[ascii_n++] = ' ';
            }
        }
        ascii[ascii_n++] = '|';

        Utils::auto_snprintf(buff_n, buff, "%s", ascii);

        if (current_line < total_lines) {
            buff[buff_n++] = '\n';
        }

        offset += 16;
    }
}

void FuncPrinter::syscall(FUNC_CONTEXT *func_context) {
    func_context->info[func_context->info_n++] = '\n';

    auto *self = GumTrace::get_instance();
    func_context->name = self->svc_func_maps[func_context->cpu_context.x[0]].c_str();
    before(func_context);
}


void FuncPrinter::before(FUNC_CONTEXT *func_context) {
    Utils::auto_snprintf(func_context->info_n, func_context->info, "call func: %s", func_context->name);

    auto it = func_configs.find(func_context->name);
    if (it == func_configs.end()) {
        params_join(func_context, 0);
        func_context->info[func_context->info_n++] = '\n';
        return;
    }

    const auto& config = it->second;
    params_join(func_context, config.params_number);

    if (config.special_handler) {
        return config.special_handler(func_context);
    }

    for (int idx : config.string_indices) {
        Utils::auto_snprintf(func_context->info_n, func_context->info, "\nargs%d: ", idx);
        read_string(func_context->info_n, func_context->info, (char*)func_context->cpu_context.x[idx]);
    }

    for (const auto& reg_pair : config.hexdump_indices) {
        if (reg_pair[0] >= 0 && reg_pair[1] >= 0) {
            hexdump(func_context->info_n, func_context->info, func_context->cpu_context.x[reg_pair[0]], reg_pair[1] == HEX_INDEX_SPECIAL_32 ? 0 : func_context->cpu_context.x[reg_pair[1]]);
        }
    }


    // if (config.special_handler) {
    //     config.special_handler(func_context);
    //     return;
    // }

    func_context->info[func_context->info_n++] = '\n';
}

#if PLATFORM_ANDROID

void FuncPrinter::jni_before(FUNC_CONTEXT *func_context) {
    // nothing todo
}

void FuncPrinter::jni_after(FUNC_CONTEXT *func_context, GumCpuContext *curr_cpu_context) {
    auto instance = GumTrace::get_instance();
    Utils::auto_snprintf(func_context->info_n, func_context->info, "call jni func: %s", func_context->name);

    auto it = after_jni_func_configs.find(func_context->name);
    if (it == after_jni_func_configs.end()) {
        params_join(func_context, 0);
    } else {
        const auto& config = it->second;
        params_join(func_context, config.params_number);

        for (int reg_index: config.jni_string_indices) {
            auto jstr = (jstring)(func_context->cpu_context.x[reg_index]);
            const char *cstr = instance->jni_env->GetStringUTFChars(jstr, nullptr);
            Utils::auto_snprintf(func_context->info_n, func_context->info, "\nargs%d: ", reg_index);
            read_string(func_context->info_n, func_context->info, (char*)cstr);
            instance->jni_env->ReleaseStringUTFChars(jstr, cstr);
        }

        for (int reg_index: config.curr_jni_string_indices) {
            auto jstr = (jstring)(curr_cpu_context->x[reg_index]);
            const char *cstr = instance->jni_env->GetStringUTFChars(jstr, nullptr);
            Utils::auto_snprintf(func_context->info_n, func_context->info, "\nargs%d: ", reg_index);
            read_string(func_context->info_n, func_context->info, (char*)cstr);
            instance->jni_env->ReleaseStringUTFChars(jstr, cstr);
        }

        for (int reg_index : config.string_indices) {
            Utils::auto_snprintf(func_context->info_n, func_context->info, "\nargs%d: ", reg_index);
            read_string(func_context->info_n, func_context->info, (char*)func_context->cpu_context.x[reg_index]);
        }

        for (int reg_index : config.curr_string_indices) {
            Utils::auto_snprintf(func_context->info_n, func_context->info, "\nargs%d: ", reg_index);
            read_string(func_context->info_n, func_context->info, (char*)curr_cpu_context->x[reg_index]);
        }

        for (std::array<int, 2> reg_pair: config.hexdump_indices) {
            hexdump(func_context->info_n, func_context->info, func_context->cpu_context.x[reg_pair[0]], reg_pair[1] == HEX_INDEX_SPECIAL_32 ? 0 : func_context->cpu_context.x[reg_pair[1]]);
        }

        for (std::array<int, 2> reg_pair: config.curr_hexdump_indices) {
            hexdump(func_context->info_n, func_context->info, curr_cpu_context->x[reg_pair[0]], reg_pair[1] == HEX_INDEX_SPECIAL_32 ? 0 : curr_cpu_context->x[reg_pair[1]]);
        }
    }

    if (strcmp(func_context->name, "FindClass") == 0 || strcmp(func_context->name, "DefineClass") == 0) {
        char jclass_name[1024] = {0};
        int jclass_name_n = 0;
        read_string(jclass_name_n, jclass_name, (char*)func_context->cpu_context.x[1]);
        if (jclass_name_n > 0) {
            instance->jni_classes[curr_cpu_context->x[0]] = jclass_name;
        }
    } else if (strcmp(func_context->name, "GetMethodID") == 0 || strcmp(func_context->name, "GetStaticMethodID") == 0) {
        char jmethod_name[4096] = {0};
        int jmethod_name_n = 0;

        read_string(jmethod_name_n, jmethod_name, (char*)func_context->cpu_context.x[2]);
        if (jmethod_name_n > 0) {
            instance->jni_methods[curr_cpu_context->x[0]] = jmethod_name;
        }

        if (instance->jni_classes.count(func_context->cpu_context.x[1]) > 0) {
            auto jclass_name = instance->jni_classes[func_context->cpu_context.x[1]];

            Utils::auto_snprintf(func_context->info_n, func_context->info, "\njclass: %s", jclass_name.c_str());
            instance->jni_methods_classes[curr_cpu_context->x[0]] = jclass_name;
        }
    }

    for (const auto &method_name: call_jni_methods) {
        if (strcmp(method_name.c_str(), func_context->name) == 0) {

            if (instance->jni_classes.count(func_context->cpu_context.x[1]) > 0) {
                 Utils::auto_snprintf(func_context->info_n, func_context->info, "\njclass: %s", instance->jni_classes[func_context->cpu_context.x[1]].c_str());
            }

            if (instance->jni_methods.count(func_context->cpu_context.x[2]) > 0) {
                Utils::auto_snprintf(func_context->info_n, func_context->info, "\njmethod: %s", instance->jni_methods[func_context->cpu_context.x[2]].c_str());

                if (instance->jni_methods_classes.count(func_context->cpu_context.x[2]) > 0) {
                     Utils::auto_snprintf(func_context->info_n, func_context->info, "\njclass: %s", instance->jni_methods_classes[func_context->cpu_context.x[2]].c_str());
                }
            }
            break;
        }
    }

    Utils::append_string(func_context->info, func_context->info_n, "\nret: 0x");
    Utils::append_uint64_hex(func_context->info, func_context->info_n, curr_cpu_context->x[0]);
    func_context->info[func_context->info_n++] = '\n';
}

#endif

void FuncPrinter::after(FUNC_CONTEXT *func_context, GumCpuContext *curr_cpu_context) {
    Utils::append_string(func_context->info, func_context->info_n, "ret: 0x");
    Utils::append_uint64_hex(func_context->info, func_context->info_n, curr_cpu_context->x[0]);
    func_context->info[func_context->info_n++] = '\n';
}
