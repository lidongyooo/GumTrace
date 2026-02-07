//
// Created by lidongyooo on 2026/2/6.
//

#ifndef GUMTRACE_UTILS_H
#define GUMTRACE_UTILS_H
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <fstream>
#include <array>
#include <sstream>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/mman.h>
#include "platform.h"
#include <pthread.h>
#include <sys/stat.h>

#if PLATFORM_ANDROID
#include "../libs/FridaGum-Android-17.6.2.h"
#include <android/log.h>
#else
#include "../libs/FridaGum-IOS-17.6.2.h"
#endif

#define LOG_TAG "gumtrace"
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

#define PAGE_SIZE 4096

#define PAGE_ALIGN_DOWN(addr) \
((uintptr_t)(addr) & ~(SYSTEM_PAGE_SIZE - 1))

#define PAGE_ALIGN_UP(addr) \
(((uintptr_t)(addr) + SYSTEM_PAGE_SIZE - 1) & ~(SYSTEM_PAGE_SIZE - 1))

class Utils {
public:
    static std::vector<std::string> str_split(const std::string& s, char symbol);
    static bool is_lse(cs_insn *insn);
    static bool get_register_value(arm64_reg reg, _GumArm64CpuContext *ctx, __uint128_t &value);
    static void format_uint128_hex(__uint128_t value, int& counter, char* buff);
    static void auto_snprintf(int& counter, char* buff, const char* __restrict __format, ...);

    static inline void append_string(char* buff, int& counter, const char* str) {
        if (!str) return;
        while (*str) {
            buff[counter++] = *str++;
        }
    }
    
    static inline void append_char(char* buff, int& counter, char c) {
        buff[counter++] = c;
    }

    static void append_uint64_hex(char* buff, int& counter, uint64_t val);
    static void append_uint64_hex_fixed(char* buff, int& counter, uint64_t val);
};


#endif //GUMTRACE_UTILS_H