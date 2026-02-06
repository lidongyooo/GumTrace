//
// Created by lidongyooo on 2026/2/6.
//

#include "Utils.h"
#include "GumTrace.h"

std::vector<std::string> Utils::str_split(const std::string& s, char symbol) {
    std::vector<std::string> res;
    size_t pos = 0;
    while (pos < s.size()) {
        size_t comma = s.find(symbol, pos);
        if (comma == std::string::npos) {
            comma = s.size();
        }
        res.push_back(s.substr(pos, comma - pos));
        pos = comma + 1;
    }
    return res;
}

bool Utils::is_lse(cs_insn *insn) {
    if (insn == nullptr) return false; // 防御空指针

    switch (insn->id) {
        case ARM64_INS_LDAXR:
        case ARM64_INS_LDAXP:
        case ARM64_INS_LDAXRB:
        case ARM64_INS_LDAXRH:
        case ARM64_INS_LDXR:
        case ARM64_INS_LDXP:
        case ARM64_INS_LDXRB:
        case ARM64_INS_LDXRH:
        case ARM64_INS_STXR:
        case ARM64_INS_STXP:
        case ARM64_INS_STXRB:
        case ARM64_INS_STXRH:
        case ARM64_INS_STLXR:
        case ARM64_INS_STLXP:
        case ARM64_INS_STLXRB:
        case ARM64_INS_STLXRH:

            return true;

        default:
            return false;
    }
}

bool Utils::get_register_value(arm64_reg reg, _GumArm64CpuContext *ctx, __uint128_t &value) {
    if (reg >= ARM64_REG_W0 && reg <= ARM64_REG_W30) {
        int idx = reg - ARM64_REG_W0;
        value = ctx->x[idx] & 0xFFFFFFFF;
    } else if (reg >= ARM64_REG_X0 && reg <= ARM64_REG_X28) {
        int idx = reg - ARM64_REG_X0;
        value = ctx->x[idx];
    } else if (reg >= ARM64_REG_Q0 && reg <= ARM64_REG_Q31) {
        int idx = reg - ARM64_REG_Q0;
        value = *(__uint128_t *)(ctx->v[idx].q);
    } else if (reg >= ARM64_REG_D0 && reg <= ARM64_REG_D31) {
        int idx = reg - ARM64_REG_D0;
        value = ctx->v[idx].d;
    } else if (reg >= ARM64_REG_S0 && reg <= ARM64_REG_S31) {
        int idx = reg - ARM64_REG_S0;
        value = ctx->v[idx].s;
    } else if (reg >= ARM64_REG_H0 && reg <= ARM64_REG_H31) {
        int idx = reg - ARM64_REG_H0;
        value = ctx->v[idx].h;
    } else if (reg >= ARM64_REG_B0 && reg <= ARM64_REG_B31) {
        int idx = reg - ARM64_REG_B0;
        value = ctx->v[idx].b;
    } else {
        switch (reg) {
            case ARM64_REG_SP:
                value = ctx->sp;
                break;
            case ARM64_REG_FP:
                value = ctx->fp;
                break;      // ARM64_REG_X29
            case ARM64_REG_LR:
                value = ctx->lr;
                break;      // ARM64_REG_X30
            case ARM64_REG_NZCV:
                value = ctx->nzcv;
                break;
            default:
                return false;
        }
    }

    return true;
}

void Utils::format_uint128_hex(__uint128_t value, int& counter, char* buff) {
    uint64_t high = value >> 64;
    uint64_t low = value;
    if (high > 0) {
        auto_snprintf(counter, buff, "%llx%016llx", high, low);
    } else {
        auto_snprintf(counter, buff, "%llx", low);
    }
}

void Utils::auto_snprintf(int& counter, char* buff, const char* __restrict __format, ...) {
    if (!buff || !__format) {
        return;
    }

    va_list args;
    va_start(args, __format);
    int written = vsnprintf(buff + counter, BUFFER_SIZE, __format, args);
    va_end(args);
    counter += written;
}
