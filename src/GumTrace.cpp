//
// Created by lidongyooo on 2026/2/6.
//

#include "GumTrace.h"
#include "Utils.h"
#include "FuncPrinter.h"

GumTrace *GumTrace::get_instance() {
    static GumTrace instance;
    return &instance;
}

GumTrace::GumTrace() {
    _stalker = gum_stalker_new();
    gum_stalker_set_trust_threshold(_stalker, 3);
    _transformer = gum_stalker_transformer_make_from_callback(transform_callback, nullptr, nullptr);

    callback_context_instance = CallbackContext::get_instance();
}

GumTrace::~GumTrace() {
    if (_stalker) g_object_unref(_stalker);
    if (_transformer) g_object_unref(_transformer);
}

#if PLATFORM_ANDROID

JNIEnv *GumTrace::get_run_time_env() {
    if (java_vm == nullptr) {
        return nullptr;
    }

    if (jni_env == nullptr) {
        java_vm->GetEnv((void**)&jni_env, JNI_VERSION_1_6);
    }

    if (jni_env != nullptr && jni_env_init == false) {
        jni_env_init = true;

        auto jni_func_table = (uint64_t)jni_env->functions;
        int index = 0;
        for (const auto &func_name: jni_func_names) {
            auto func_addr_ptr = (void **)(jni_func_table + index * sizeof(void *));
            auto func_addr = (uint64_t)(*func_addr_ptr);
            jni_func_maps[func_addr] = func_name;
            index++;
        }
    }
    return jni_env;
}

#endif



void GumTrace::callout_callback(GumCpuContext *cpu_context, gpointer user_data) {
    auto self = get_instance();
    auto callback_ctx = (CALLBACK_CTX *) (user_data);
    char *buff = self->buffer;
    int &buff_n = self->buffer_offset;

    if (buff_n > BUFFER_SIZE - 1024) {
        self->trace_file.write(buff, buff_n);
        buff_n = 0;
    }

    std::stringstream write_reg_value_info;
    if (self->write_reg_list.num > 0) {
        for (int i = 0; i < self->write_reg_list.num; i++) {
            __uint128_t reg_value = 0;
            if (Utils::get_register_value(self->write_reg_list.regs[i], cpu_context, reg_value)) {
                if (i == 0) {
                    Utils::append_string(buff, buff_n, "-> ");
                }

                const char *reg_name = cs_reg_name(callback_ctx->handle, self->write_reg_list.regs[i]);
                Utils::append_string(buff, buff_n, reg_name);
                Utils::append_string(buff, buff_n, "=0x");
                Utils::format_uint128_hex(reg_value, buff_n, buff);
                Utils::append_char(buff, buff_n, ' ');
            }
        }

        Utils::append_char(buff, buff_n, '\n');
        self->write_reg_list.num = 0;
    }

    if (self->last_func_context.call) {
        if (buff_n > 0) {
            self->trace_file.write(buff, buff_n);
            buff_n = 0;
        }

        self->last_func_context.call = false;
#        if PLATFORM_ANDROID

        if (self->last_func_context.is_jni) {
            self->last_func_context.is_jni = false;
            FuncPrinter::jni_after(&self->last_func_context, cpu_context);
        } else {
            FuncPrinter::after(&self->last_func_context, cpu_context);
        }

#        else

            FuncPrinter::after(&self->last_func_context, cpu_context);

#endif

        self->trace_file.write(self->last_func_context.info, self->last_func_context.info_n);
    }

    Utils::append_char(buff, buff_n, '[');
    Utils::append_string(buff, buff_n, callback_ctx->module_name);
    Utils::append_string(buff, buff_n, "] 0x");
    Utils::append_uint64_hex(buff, buff_n, cpu_context->pc);
    Utils::append_string(buff, buff_n, "!0x");
    Utils::append_uint64_hex(buff, buff_n, cpu_context->pc - callback_ctx->module_base);
    Utils::append_char(buff, buff_n, ' ');
    Utils::append_string(buff, buff_n, callback_ctx->instruction.mnemonic);
    Utils::append_char(buff, buff_n, ' ');
    Utils::append_string(buff, buff_n, callback_ctx->instruction.op_str);

    bool is_write = false;
    bool is_first_print = true;
    for (int i = 0; i < callback_ctx->instruction_detail.arm64.op_count; i++) {
        cs_arm64_op &op = callback_ctx->instruction_detail.arm64.operands[i];
        __uint128_t reg_value = 0;
        if ((op.access & CS_AC_READ) && (op.access & CS_AC_WRITE) && op.type == ARM64_OP_REG) {
            if (Utils::get_register_value(op.reg, cpu_context, reg_value)) {
                if (is_first_print) {
                    is_first_print = false;
                    Utils::append_string(buff, buff_n, "; ");
                }

                const char *reg_name = cs_reg_name(callback_ctx->handle, op.reg);
                Utils::append_string(buff, buff_n, reg_name);
                Utils::append_string(buff, buff_n, "=0x");
                Utils::format_uint128_hex(reg_value, buff_n, buff);
                Utils::append_char(buff, buff_n, ' ');
            }
            is_write = true;
            self->write_reg_list.regs[self->write_reg_list.num++] = op.reg;
        } else if (op.access & CS_AC_READ && op.type == ARM64_OP_REG) {
            if (Utils::get_register_value(op.reg, cpu_context, reg_value)) {
                if (is_first_print) {
                    is_first_print = false;
                    Utils::append_string(buff, buff_n, "; ");
                }

                const char *reg_name = cs_reg_name(callback_ctx->handle, op.reg);
                Utils::append_string(buff, buff_n, reg_name);
                Utils::append_string(buff, buff_n, "=0x");
                Utils::format_uint128_hex(reg_value, buff_n, buff);
                Utils::append_char(buff, buff_n, ' ');
            }
        } else if ((op.access & CS_AC_WRITE) && op.type == ARM64_OP_MEM) {
            __uint128_t base = 0;
            __uint128_t index = 0;
            bool flag = true;

            if (op.mem.base != ARM64_REG_INVALID) {
                flag = Utils::get_register_value(op.mem.base, cpu_context, base);
                const char *base_reg_name = cs_reg_name(callback_ctx->handle, op.mem.base);
                Utils::append_string(buff, buff_n, base_reg_name);
                Utils::append_string(buff, buff_n, "=0x");
                Utils::format_uint128_hex(base, buff_n, buff);
                Utils::append_char(buff, buff_n, ' ');
            }

            if (op.mem.index != ARM64_REG_INVALID) {
                flag = Utils::get_register_value(op.mem.index, cpu_context, index);
                const char *index_reg_name = cs_reg_name(callback_ctx->handle, op.mem.index);
                Utils::append_string(buff, buff_n, index_reg_name);
                Utils::append_string(buff, buff_n, "=0x");
                Utils::format_uint128_hex(index, buff_n, buff);
                Utils::append_char(buff, buff_n, ' ');
            }

            if (flag) {
                uintptr_t write_address = base + index + op.mem.disp;
                Utils::append_string(buff, buff_n, "mem_w=0x");
                Utils::append_uint64_hex(buff, buff_n, write_address);
                Utils::append_char(buff, buff_n, ' ');
            }
        } else if ((op.access & CS_AC_READ) && op.type == ARM64_OP_MEM) {
            __uint128_t base = 0;
            __uint128_t index = 0;
            bool flag = true;

            if (is_first_print) {
                is_first_print = false;
                Utils::append_string(buff, buff_n, "; ");
            }

            if (op.mem.base != ARM64_REG_INVALID) {
                flag = Utils::get_register_value(op.mem.base, cpu_context, base);
                const char *base_reg_name = cs_reg_name(callback_ctx->handle, op.mem.base);
                Utils::append_string(buff, buff_n, base_reg_name);
                Utils::append_string(buff, buff_n, "=0x");
                Utils::format_uint128_hex(base, buff_n, buff);
                Utils::append_char(buff, buff_n, ' ');
            }
            if (op.mem.index != ARM64_REG_INVALID) {
                flag = Utils::get_register_value(op.mem.index, cpu_context, index);
                const char *index_reg_name = cs_reg_name(callback_ctx->handle, op.mem.index);
                Utils::append_string(buff, buff_n, index_reg_name);
                Utils::append_string(buff, buff_n, "=0x");
                Utils::format_uint128_hex(index, buff_n, buff);
                Utils::append_char(buff, buff_n, ' ');
            }
            if (flag) {
                uintptr_t read_address = base + index + op.mem.disp;
                Utils::append_string(buff, buff_n, "mem_r=0x");
                Utils::append_uint64_hex(buff, buff_n, read_address);
                Utils::append_char(buff, buff_n, ' ');
            }
        } else if (op.access & CS_AC_WRITE && op.type == ARM64_OP_REG) {
            if (Utils::get_register_value(op.reg, cpu_context, reg_value)) {
                if (is_first_print) {
                    is_first_print = false;
                    Utils::append_string(buff, buff_n, "; ");
                }

                const char *reg_name = cs_reg_name(callback_ctx->handle, op.reg);
                Utils::append_string(buff, buff_n, reg_name);
                Utils::append_string(buff, buff_n, "=0x");
                Utils::format_uint128_hex(reg_value, buff_n, buff);
                Utils::append_char(buff, buff_n, ' ');
            }

            is_write = true;
            self->write_reg_list.regs[self->write_reg_list.num++] = op.reg;
        }
    }

    if (is_write == false) {
        Utils::append_char(buff, buff_n, '\n');
    }

    if (callback_ctx->instruction.id == ARM64_INS_SVC) {
        std::string svc_name = self->svc_func_maps[cpu_context->x[8]];
        self->last_func_context.info_n = 0;
        self->last_func_context.name = svc_name.c_str();
        memcpy(&self->last_func_context.cpu_context, cpu_context, sizeof(GumCpuContext));
        self->last_func_context.call = true;

        FuncPrinter::before(&self->last_func_context);
    } else {
        __uint128_t jump_addr = 0;
        if (callback_ctx->instruction.id == ARM64_INS_BL &&
            callback_ctx->instruction_detail.arm64.operands[0].type == ARM64_OP_IMM) {
            jump_addr = callback_ctx->instruction_detail.arm64.operands[0].imm;
        } else if (callback_ctx->instruction.id == ARM64_INS_BLR &&
                   callback_ctx->instruction_detail.arm64.operands[0].type == ARM64_OP_REG) {
            Utils::get_register_value(callback_ctx->instruction_detail.arm64.operands[0].reg, cpu_context, jump_addr);
        } else if (callback_ctx->instruction.id == ARM64_INS_BR &&
                   callback_ctx->instruction_detail.arm64.operands[0].type == ARM64_OP_REG) {
            Utils::get_register_value(callback_ctx->instruction_detail.arm64.operands[0].reg, cpu_context, jump_addr);
        } else if (callback_ctx->instruction.id == ARM64_INS_B &&
                   callback_ctx->instruction_detail.arm64.operands[0].type == ARM64_OP_IMM) {
            jump_addr = callback_ctx->instruction_detail.arm64.operands[0].imm;
        }

        if (jump_addr > 0) {
            if (self->func_maps.count(jump_addr) > 0) {
                self->last_func_context.info_n = 0;
                self->last_func_context.address = jump_addr;
                self->last_func_context.name = self->func_maps[jump_addr].c_str();
                memcpy(&self->last_func_context.cpu_context, cpu_context, sizeof(GumCpuContext));
                self->last_func_context.call = true;

                FuncPrinter::before(&self->last_func_context);
            }
#            if PLATFORM_ANDROID
            else if (self->get_run_time_env() != nullptr && self->jni_func_maps.count(jump_addr) > 0) {
                self->last_func_context.info_n = 0;
                self->last_func_context.address = jump_addr;
                self->last_func_context.name = self->jni_func_maps[jump_addr].c_str();
                memcpy(&self->last_func_context.cpu_context, cpu_context, sizeof(GumCpuContext));
                self->last_func_context.call = true;
                self->last_func_context.is_jni = true;

                FuncPrinter::jni_before(&self->last_func_context);
            }
#endif


        }
    }


    self->trace_flash++;
    if (self->trace_flash > 100000) {
        if (buff_n > 0) {
            self->trace_file.write(buff, buff_n);
            buff_n = 0;
        }

        self->trace_file.flush();
        self->trace_flash = 0;
    }
}

void GumTrace::transform_callback(GumStalkerIterator *iterator, GumStalkerOutput *output, gpointer user_data) {
    const auto self = get_instance();

    cs_insn *p_insn;
    auto *it = iterator;
    while (gum_stalker_iterator_next(it, (const cs_insn **) &p_insn)) {
        const std::string *module_name_ptr = self->in_range_module(p_insn->address);
        if (module_name_ptr == nullptr) {
            gum_stalker_iterator_keep(it);
            continue;
        }

        if (Utils::is_lse(p_insn) == false) {
            auto module = self->get_module_by_name(*module_name_ptr);

            auto callback_ctx = self->callback_context_instance->pull(p_insn, gum_stalker_iterator_get_capstone(it),
                                                                      module_name_ptr->c_str(), module["base"]);

            gum_stalker_iterator_put_callout(it, callout_callback, callback_ctx, nullptr);
        }

        gum_stalker_iterator_keep(it);
    }
}

const std::string *GumTrace::in_range_module(size_t address) {
    if (last_module_cache.name != nullptr && address >= last_module_cache.base && address < last_module_cache.end) {
        return last_module_cache.name;
    }

    for (const auto &pair: modules) {
        const auto &module_map = pair.second;
        size_t base = module_map.at("base");
        size_t size = module_map.at("size");
        size_t end = base + size;
        if (address >= base && address < end) {
            last_module_cache.name = &pair.first;
            last_module_cache.base = base;
            last_module_cache.end = end;
            return &pair.first;
        }
    }
    return nullptr;
}

std::map<std::string, std::size_t> GumTrace::get_module_by_name(const std::string &module_name) {
    return modules[module_name];
}

void GumTrace::follow() {
    trace_thread_id > 0
        ? gum_stalker_follow(_stalker, trace_thread_id, _transformer, nullptr)
        : gum_stalker_follow_me(_stalker, _transformer, nullptr);
}


void GumTrace::unfollow() {
    if (trace_file.is_open()) {
        trace_file.write(buffer, buffer_offset);
        trace_file.flush();
        trace_file.close();
    }

    trace_thread_id > 0 ? gum_stalker_unfollow(_stalker, trace_thread_id) : gum_stalker_unfollow_me(_stalker);
}
