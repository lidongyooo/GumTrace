//
// Created by lidongyooo on 2026/2/6.
//

#ifndef GUMTRACE_FUNCPRINTER_H
#define GUMTRACE_FUNCPRINTER_H
#include "Utils.h"
#include "GumTrace.h"

typedef enum {
    STR_INDEX_ZERO = 0,
    STR_INDEX_ONE = 1,
    STR_INDEX_TWO = 2,
    STR_INDEX_THREE = 3,
    STR_INDEX_FOUR = 4,
    STR_INDEX_FIVE = 5,
    STR_INDEX_SIX = 6,
    STR_INDEX_SEVEN = 7
} StrEnum;

typedef enum {
    HEX_INDEX_ZERO = 0,
    HEX_INDEX_ONE = 1,
    HEX_INDEX_TWO = 2,
    HEX_INDEX_THREE = 3,
    HEX_INDEX_FOUR = 4,
    HEX_INDEX_FIVE = 5,
    HEX_INDEX_SIX = 6,
    HEX_INDEX_SEVEN = 7,
    HEX_INDEX_SPECIAL_32 = 32
} HexEnum;

typedef enum {
    PARAMS_NUMBER_ZERO = 0,
    PARAMS_NUMBER_ONE = 1,
    PARAMS_NUMBER_TWO = 2,
    PARAMS_NUMBER_THREE = 3,
    PARAMS_NUMBER_FOUR = 4,
    PARAMS_NUMBER_FIVE = 5,
    PARAMS_NUMBER_SIX = 6,
    PARAMS_NUMBER_SEVEN = 7
} ParamsNumberEnum;

using HexRegisterPair = std::array<HexEnum, 2>;
struct BeforeFuncConfig {
    int params_number;
    std::vector<int> string_indices;
    std::vector<std::array<int, 2>> hexdump_indices;
    std::function<std::stringstream(FUNC_CONTEXT*)> special_handler;

    BeforeFuncConfig(ParamsNumberEnum params = PARAMS_NUMBER_ZERO,
               std::initializer_list<StrEnum> str_idx = {},
               std::initializer_list<HexRegisterPair> hex_idx = {},
               std::function<std::stringstream(FUNC_CONTEXT*)> handler = nullptr)
        : params_number(static_cast<int>(params)),
          special_handler(handler) {

        for (auto idx : str_idx) {
            string_indices.push_back(static_cast<int>(idx));
        }

        for (const auto& pair : hex_idx) {
            hexdump_indices.push_back({
                static_cast<int>(pair[0]),
                static_cast<int>(pair[1])
            });
        }
    }
};

static const std::unordered_map<std::string, BeforeFuncConfig> func_configs = {
    // 字符串操作
    {"strstr", {PARAMS_NUMBER_TWO, {STR_INDEX_ZERO, STR_INDEX_ONE}, {}}},
    {"strlen", {PARAMS_NUMBER_ONE, {STR_INDEX_ZERO}, {}}},
    {"__strlen_chk", {PARAMS_NUMBER_ONE, {STR_INDEX_ZERO}, {}}},
    {"strcpy", {PARAMS_NUMBER_TWO, {STR_INDEX_ZERO, STR_INDEX_ONE}, {}}},
    {"__strcpy_chk", {PARAMS_NUMBER_TWO, {STR_INDEX_ZERO, STR_INDEX_ONE}, {}}},
    {"strncpy", {PARAMS_NUMBER_TWO, {STR_INDEX_ZERO, STR_INDEX_ONE}, {}}},
    {"__strncpy_chk", {PARAMS_NUMBER_TWO, {STR_INDEX_ZERO, STR_INDEX_ONE}, {}}},
    {"strcat", {PARAMS_NUMBER_TWO, {STR_INDEX_ZERO, STR_INDEX_ONE}, {}}},
    {"__strcat_chk", {PARAMS_NUMBER_TWO, {STR_INDEX_ZERO, STR_INDEX_ONE}, {}}},
    {"strncat", {PARAMS_NUMBER_TWO, {STR_INDEX_ZERO, STR_INDEX_ONE}, {}}},
    {"__strncat_chk", {PARAMS_NUMBER_TWO, {STR_INDEX_ZERO, STR_INDEX_ONE}, {}}},
    {"strdup", {PARAMS_NUMBER_ONE, {STR_INDEX_ZERO}, {}}},
    {"__strdup_chk", {PARAMS_NUMBER_ONE, {STR_INDEX_ZERO}, {}}},
    {"strndup", {PARAMS_NUMBER_ONE, {STR_INDEX_ZERO}, {}}},
    {"__strndup_chk", {PARAMS_NUMBER_ONE, {STR_INDEX_ZERO}, {}}},
    {"strchr", {PARAMS_NUMBER_ONE, {STR_INDEX_ZERO}, {}}},
    {"strrchr", {PARAMS_NUMBER_ONE, {STR_INDEX_ZERO}, {}}},
    {"strspn", {PARAMS_NUMBER_ONE, {STR_INDEX_ZERO}, {}}},
    {"strcspn", {PARAMS_NUMBER_ONE, {STR_INDEX_ZERO}, {}}},
    {"strcasestr", {PARAMS_NUMBER_ONE, {STR_INDEX_ZERO}, {}}},
    {"strlcpy", {PARAMS_NUMBER_THREE, {STR_INDEX_ZERO, STR_INDEX_ONE}, {}}},
    {"__strlcpy_chk", {PARAMS_NUMBER_THREE, {STR_INDEX_ZERO, STR_INDEX_ONE}, {}}},
    {"strlcat", {PARAMS_NUMBER_THREE, {STR_INDEX_ZERO, STR_INDEX_ONE}, {}}},
    {"__strlcat_chk", {PARAMS_NUMBER_THREE, {STR_INDEX_ZERO, STR_INDEX_ONE}, {}}},

    // 内存操作
    {"memcpy", {PARAMS_NUMBER_THREE, {}, {{HEX_INDEX_ONE, HEX_INDEX_TWO}}}},
    {"__memcpy_chk", {PARAMS_NUMBER_THREE, {}, {{HEX_INDEX_ONE, HEX_INDEX_TWO}}}},
    {"memmove", {PARAMS_NUMBER_THREE, {}, {{HEX_INDEX_ONE, HEX_INDEX_TWO}}}},
    {"__memmove_chk", {PARAMS_NUMBER_THREE, {}, {{HEX_INDEX_ONE, HEX_INDEX_TWO}}}},
    {"memset", {PARAMS_NUMBER_THREE, {}, {}}},
    {"__memset_chk", {PARAMS_NUMBER_THREE, {}, {}}},
    {"memmem", {PARAMS_NUMBER_FOUR, {}, {{HEX_INDEX_ZERO, HEX_INDEX_ONE}, {HEX_INDEX_TWO, HEX_INDEX_THREE}}}},
    {"memcmp", {PARAMS_NUMBER_THREE, {}, {{HEX_INDEX_ZERO, HEX_INDEX_TWO}, {HEX_INDEX_ONE, HEX_INDEX_TWO}}}},
    {"memchr", {PARAMS_NUMBER_THREE, {STR_INDEX_ZERO}, {}}},

    // 文件操作
    {"fopen", {PARAMS_NUMBER_TWO, {STR_INDEX_ZERO, STR_INDEX_ONE}, {}}},
    {"fopen64", {PARAMS_NUMBER_TWO, {STR_INDEX_ZERO, STR_INDEX_ONE}, {}}},
    {"read", {PARAMS_NUMBER_THREE, {}, {{HEX_INDEX_ONE, HEX_INDEX_TWO}}}},
    {"pread64", {PARAMS_NUMBER_THREE, {}, {{HEX_INDEX_ONE, HEX_INDEX_TWO}}}},
    {"open", {PARAMS_NUMBER_TWO, {STR_INDEX_ZERO}, {}}},
    {"openat", {PARAMS_NUMBER_FOUR, {STR_INDEX_ONE}, {}}},
    {"close", {PARAMS_NUMBER_ONE, {}, {}}},
    {"write", {PARAMS_NUMBER_THREE, {}, {{HEX_INDEX_ONE, HEX_INDEX_TWO}}}},
    {"pwrite64", {PARAMS_NUMBER_THREE, {}, {{HEX_INDEX_ONE, HEX_INDEX_TWO}}}},
    {"mknodat", {PARAMS_NUMBER_FOUR, {STR_INDEX_ONE}, {}}},
    {"mkdirat", {PARAMS_NUMBER_THREE, {STR_INDEX_ONE}, {}}},
    {"newfstatat", {PARAMS_NUMBER_THREE, {STR_INDEX_ONE}, {}}},
    {"fstat", {PARAMS_NUMBER_TWO, {}, {}}},
    {"stat", {PARAMS_NUMBER_TWO, {STR_INDEX_ZERO}, {}}},
    {"readlink", {PARAMS_NUMBER_THREE, {STR_INDEX_ZERO, STR_INDEX_ONE}, {}}},
    {"readlinkat", {PARAMS_NUMBER_FOUR, {STR_INDEX_ONE, STR_INDEX_TWO}, {}}},
    {"opendir", {PARAMS_NUMBER_ONE, {STR_INDEX_ZERO}, {}}},
    {"access", {PARAMS_NUMBER_TWO, {STR_INDEX_ZERO}, {}}},
    {"popen", {PARAMS_NUMBER_TWO, {STR_INDEX_ZERO, STR_INDEX_ONE}, {}}},
    {"pclose", {PARAMS_NUMBER_ONE, {}, {}}},

    // 格式化输出
    {"sprintf", {PARAMS_NUMBER_TWO, {STR_INDEX_ZERO, STR_INDEX_ONE}, {}}},
    {"__sprintf_chk", {PARAMS_NUMBER_TWO, {STR_INDEX_ZERO, STR_INDEX_ONE}, {}}},
    {"snprintf", {PARAMS_NUMBER_THREE, {STR_INDEX_ZERO}, {}}},
    {"__snprintf_chk", {PARAMS_NUMBER_THREE, {STR_INDEX_ZERO}, {}}},
    {"vsprintf", {PARAMS_NUMBER_TWO, {STR_INDEX_ZERO}, {}}},
    {"__vsprintf_chk", {PARAMS_NUMBER_TWO, {STR_INDEX_ZERO}, {}}},
    {"vsnprintf", {PARAMS_NUMBER_THREE, {STR_INDEX_ZERO}, {}}},
    {"__vsnprintf_chk", {PARAMS_NUMBER_THREE, {STR_INDEX_ZERO}, {}}},
    {"fgets", {PARAMS_NUMBER_THREE, {STR_INDEX_ZERO}, {}}},
    {"__fgets_chk", {PARAMS_NUMBER_THREE, {STR_INDEX_ZERO}, {}}},
    {"sscanf", {PARAMS_NUMBER_TWO, {STR_INDEX_ZERO, STR_INDEX_ONE}, {}}},

    // 内存分配
    {"calloc", {PARAMS_NUMBER_TWO, {}, {}}},
    {"malloc", {PARAMS_NUMBER_ONE, {}, {}}},
    {"realloc", {PARAMS_NUMBER_TWO, {}, {{HEX_INDEX_ZERO, HEX_INDEX_SPECIAL_32}}}},
    {"free", {PARAMS_NUMBER_ONE, {}, {{HEX_INDEX_ZERO, HEX_INDEX_SPECIAL_32}}}},
    {"aligned_alloc", {PARAMS_NUMBER_TWO, {}, {}}},

    // 内存映射
    {"mmap", {PARAMS_NUMBER_SIX, {}, {}}},
    {"mmap64", {PARAMS_NUMBER_SIX, {}, {}}},
    {"mprotect", {PARAMS_NUMBER_THREE, {}, {}}},

    // 动态链接
    {"dlopen", {PARAMS_NUMBER_TWO, {STR_INDEX_ZERO}, {}}},
    {"dlsym", {PARAMS_NUMBER_TWO, {STR_INDEX_ONE}, {}}},
    {"dlclose", {PARAMS_NUMBER_ONE, {}, {}}},

    // 系统相关
    {"sysconf", {PARAMS_NUMBER_ONE, {}, {}}},
    {"__system_property_get", {PARAMS_NUMBER_TWO, {STR_INDEX_ZERO, STR_INDEX_ONE}, {}}},
    {"gettimeofday", {PARAMS_NUMBER_TWO, {}, {{HEX_INDEX_ZERO, HEX_INDEX_SPECIAL_32}}}},
    {"srand48", {PARAMS_NUMBER_ONE, {}, {}}},
    {"arc4random_buf", {PARAMS_NUMBER_TWO, {}, {{HEX_INDEX_ZERO, HEX_INDEX_ONE}}}}
};

class FuncPrinter {
public:
    static void before(FUNC_CONTEXT* func_context);
    static void after(FUNC_CONTEXT* func_context, GumCpuContext* curr_cpu_context);
    static void params_join(FUNC_CONTEXT* func_context, uint count);
    static void read_string(int& buff_n, char *buff, char* str, size_t max_len = 1024);
    static void hexdump(int& buff_n, char *buff, uint64_t address, size_t count);
};

#endif //GUMTRACE_FUNCPRINTER_H