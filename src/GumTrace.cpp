//
// Created by lidongyooo on 2026/2/6.
//

#include "GumTrace.h"
#include "Utils.h"

GumTrace *GumTrace::get_instance() {
    static GumTrace instance;
    return &instance;
}

GumTrace::GumTrace(){
    _stalker = gum_stalker_new();
    gum_stalker_set_trust_threshold(_stalker, 3);
    _transformer = gum_stalker_transformer_make_from_callback(transform_callback, nullptr, nullptr);

    callback_context_instance = CallbackContext::get_instance();
}

GumTrace::~GumTrace() {
    if (_stalker) g_object_unref(_stalker);
    if (_transformer) g_object_unref(_transformer);
}

void GumTrace::callout_callback(GumCpuContext *cpu_context, gpointer user_data) {
    auto self = get_instance();
    auto callback_ctx = (CALLBACK_CTX *)(user_data);
    char* buff = self->buffer;
    int buff_n = 0;

    std::stringstream write_reg_value_info;
    if (self->write_reg_list.num > 0) {
        for (int i = 0; i < self->write_reg_list.num; i++) {
            __uint128_t reg_value = 0;
            if (Utils::get_register_value(self->write_reg_list.regs[i], cpu_context, reg_value)) {
                if (i == 0) {
                    buff[buff_n++] = '-';
                    buff[buff_n++] = '>';
                    buff[buff_n++] = ' ';
                }

                const char *reg_name = cs_reg_name(callback_ctx->handle, self->write_reg_list.regs[i]);
                Utils::auto_snprintf(buff_n, buff, "%s=0x", reg_name);
                Utils::format_uint128_hex(reg_value, buff_n, buff);
                buff[buff_n++] = ' ';
            }
        }

        buff[buff_n++] = '\n';
        self->write_reg_list.num = 0;
    }

    Utils::auto_snprintf(buff_n, buff, "[%s] 0x%llx!0x%llx %s %s",
        callback_ctx->module_name.c_str(), cpu_context->pc, cpu_context->pc - callback_ctx->module_base,
        callback_ctx->instruction.mnemonic, callback_ctx->instruction.op_str);

    bool is_write = false;
    bool is_first_print = true;
    for (int i = 0; i < callback_ctx->instruction_detail.arm64.op_count; i++) {
        cs_arm64_op &op = callback_ctx->instruction_detail.arm64.operands[i];
        __uint128_t reg_value = 0;
        if ((op.access & CS_AC_READ) && (op.access & CS_AC_WRITE) && op.type == ARM64_OP_REG) {
            if (Utils::get_register_value(op.reg, cpu_context, reg_value)) {

                if (is_first_print) {
                    is_first_print = false;
                    buff[buff_n++] = ';';
                    buff[buff_n++] = ' ';
                }

                const char *reg_name = cs_reg_name(callback_ctx->handle, op.reg);
                Utils::auto_snprintf(buff_n, buff, "%s=0x", reg_name);
                Utils::format_uint128_hex(reg_value, buff_n, buff);
                buff[buff_n++] = ' ';
            }
            is_write = true;
            self->write_reg_list.regs[self->write_reg_list.num++] = op.reg;
        } else if (op.access & CS_AC_READ && op.type == ARM64_OP_REG) {
            if (Utils::get_register_value(op.reg, cpu_context, reg_value)) {

                if (is_first_print) {
                    is_first_print = false;
                    buff[buff_n++] = ';';
                    buff[buff_n++] = ' ';
                }

                const char *reg_name = cs_reg_name(callback_ctx->handle, op.reg);
                Utils::auto_snprintf(buff_n, buff, "%s=0x", reg_name);
                Utils::format_uint128_hex(reg_value, buff_n, buff);
                buff[buff_n++] = ' ';
            }
        } else if ((op.access & CS_AC_WRITE) && op.type == ARM64_OP_MEM) {
            __uint128_t base = 0;
            __uint128_t index = 0;
            bool flag = true;

            if (op.mem.base != ARM64_REG_INVALID) {
                flag = Utils::get_register_value(op.mem.base, cpu_context, base);
                const char *base_reg_name = cs_reg_name(callback_ctx->handle, op.mem.base);
                Utils::auto_snprintf(buff_n, buff, "%s=0x", base_reg_name);
                Utils::format_uint128_hex(base, buff_n, buff);
                buff[buff_n++] = ' ';
            }

            if (op.mem.index != ARM64_REG_INVALID) {
                flag = Utils::get_register_value(op.mem.index, cpu_context, index);
                const char *index_reg_name = cs_reg_name(callback_ctx->handle, op.mem.index);
                Utils::auto_snprintf(buff_n, buff, "%s=0x", index_reg_name);
                Utils::format_uint128_hex(index, buff_n, buff);
                buff[buff_n++] = ' ';
            }

            if (flag) {
                uintptr_t write_address = base + index + op.mem.disp;
                Utils::auto_snprintf(buff_n, buff, "mem_w=0x%llx ", write_address);
            }
        } else if ((op.access & CS_AC_READ) && op.type == ARM64_OP_MEM) {
            __uint128_t base = 0;
            __uint128_t index = 0;
            bool flag = true;

            if (is_first_print) {
                is_first_print = false;
                buff[buff_n++] = ';';
                buff[buff_n++] = ' ';
            }

            if (op.mem.base != ARM64_REG_INVALID) {
                flag = Utils::get_register_value(op.mem.base, cpu_context, base);
                const char *base_reg_name = cs_reg_name(callback_ctx->handle, op.mem.base);
                Utils::auto_snprintf(buff_n, buff, "%s=0x", base_reg_name);
                Utils::format_uint128_hex(base, buff_n, buff);
                buff[buff_n++] = ' ';
            }
            if (op.mem.index != ARM64_REG_INVALID) {
                flag = Utils::get_register_value(op.mem.index, cpu_context, index);
                const char *index_reg_name = cs_reg_name(callback_ctx->handle, op.mem.index);
                Utils::auto_snprintf(buff_n, buff, "%s=0x", index_reg_name);
                Utils::format_uint128_hex(index, buff_n, buff);
                buff[buff_n++] = ' ';
            }
            if (flag) {
                uintptr_t read_address = base + index + op.mem.disp;
                Utils::auto_snprintf(buff_n, buff, "mem_r=0x%llx ", read_address);
            }
        } else if (op.access & CS_AC_WRITE && op.type == ARM64_OP_REG) {
            if (Utils::get_register_value(op.reg, cpu_context, reg_value)) {

                if (is_first_print) {
                    is_first_print = false;
                    buff[buff_n++] = ';';
                    buff[buff_n++] = ' ';
                }

                const char *reg_name = cs_reg_name(callback_ctx->handle, op.reg);
                Utils::auto_snprintf(buff_n, buff, "%s=0x", reg_name);
                Utils::format_uint128_hex(reg_value, buff_n, buff);
                buff[buff_n++] = ' ';
            }

            is_write = true;
            self->write_reg_list.regs[self->write_reg_list.num++] = op.reg;
        }
    }

    if (is_write == false) {
        buff[buff_n++] = '\n';
    }

    self->trace_file.write(buff, buff_n);
    self->trace_file.flush();
}

void GumTrace::transform_callback(GumStalkerIterator * iterator, GumStalkerOutput * output, gpointer user_data) {
    const auto self = get_instance();

    cs_insn *p_insn;
    auto *it = iterator;
    while (gum_stalker_iterator_next(it, (const cs_insn **) &p_insn)) {
        std::string module_name = self->in_range_module(p_insn->address);
        if (module_name.empty()) {
            gum_stalker_iterator_keep(it);
            continue;
        }

        if (Utils::is_lse(p_insn) == false) {
            auto module = self->get_module_by_name(module_name);

            auto callback_ctx = self->callback_context_instance->pull(p_insn, gum_stalker_iterator_get_capstone(it), module_name, module["base"]);

            gum_stalker_iterator_put_callout(it, callout_callback, callback_ctx, nullptr);
        }

        gum_stalker_iterator_keep(it);
    }
}

std::string GumTrace::in_range_module(size_t address) {
    for (const auto &pair: modules) {
        const auto &module_map = pair.second;
        size_t base = module_map.at("base");
        size_t size = module_map.at("size");
        size_t end = base + size;
        if (address >= base && address < end) {
            return pair.first;
        }
    }
    return "";
}

std::map<std::string, std::size_t> GumTrace::get_module_by_name(const std::string &module_name) {
    return modules[module_name];
}

void GumTrace::follow() {
    trace_thread_id > 0 ? gum_stalker_follow(_stalker, trace_thread_id, _transformer, nullptr) : gum_stalker_follow_me(_stalker, _transformer, nullptr);
}


void GumTrace::unfollow() {
    if (trace_file.is_open()) {
        trace_file.flush();
        trace_file.close();
    }

    trace_thread_id > 0 ? gum_stalker_unfollow(_stalker, trace_thread_id) : gum_stalker_unfollow_me(_stalker);
}