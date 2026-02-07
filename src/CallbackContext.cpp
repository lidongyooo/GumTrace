//
// Created by lidongyooo on 2026/2/6.
//

#include "CallbackContext.h"

#include <utility>


CallbackContext *CallbackContext::get_instance() {
    static CallbackContext instance;
    return &instance;
}


CallbackContext::CallbackContext() {
    list = (CALLBACK_CTX*)calloc(CALLBACK_CTX_SIZE, sizeof(CALLBACK_CTX));
}

CallbackContext::~CallbackContext() {
    free(list);
}

CALLBACK_CTX* CallbackContext::pull(const cs_insn* _instruction, csh _handle, const char* module_name, uint64_t module_base) {
    if (curr_index >= CALLBACK_CTX_SIZE) {
        curr_index = 0;
    }

    CALLBACK_CTX *ctx = &list[curr_index++];
    ctx->handle = _handle;
    ctx->module_name = module_name;
    ctx->module_base = module_base;
    memcpy(&ctx->instruction, _instruction, sizeof(cs_insn));
    if (_instruction->detail) {
        memcpy(&ctx->instruction_detail, _instruction->detail, sizeof(cs_detail));
    }
    
    return ctx;
}



