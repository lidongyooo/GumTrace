//
// Created by lidongyooo on 2026/2/6.
//

#include "FuncPrinter.h"

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


    for (int idx : config.string_indices) {
        if (idx >= 0) {
            Utils::auto_snprintf(func_context->info_n, func_context->info, "\nargs%d: ", idx);
            read_string(func_context->info_n, func_context->info, (char*)func_context->cpu_context.x[idx]);
        }
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

void FuncPrinter::after(FUNC_CONTEXT *func_context, GumCpuContext *curr_cpu_context) {
    Utils::append_string(func_context->info, func_context->info_n, "ret: ");
    Utils::append_uint64_hex(func_context->info, func_context->info_n, curr_cpu_context->x[0]);
    func_context->info[func_context->info_n++] = '\n';
}
