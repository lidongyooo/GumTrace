//
// Created by lidongyooo on 2026/2/6.
//

#ifndef GUMTRACE_CALLBACKCONTEXT_H
#define GUMTRACE_CALLBACKCONTEXT_H
#include "Utils.h"

#define CALLBACK_CTX_SIZE 10240

struct CALLBACK_CTX {
    const char* module_name;
    uint64_t module_base;
    cs_insn instruction{};
    cs_detail instruction_detail{};
    csh handle = 0;
};

class CallbackContext {
public:
    static CallbackContext *get_instance();
    CALLBACK_CTX* list;
    int curr_index = 0;

    CALLBACK_CTX* pull(const cs_insn* _instruction, csh _handle, const char* module_name, uint64_t module_base);
private:
    CallbackContext();

    ~CallbackContext();

    CallbackContext(const CallbackContext &) = delete;

    CallbackContext &operator=(const CallbackContext &) = delete;
};

#endif //GUMTRACE_CALLBACKCONTEXT_H