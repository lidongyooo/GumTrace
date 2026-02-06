//
// Created by lidongyooo on 2026/2/6.
//

#ifndef GUMTRACE_GUMTRACE_H
#define GUMTRACE_GUMTRACE_H

#include "Utils.h"
#include "CallbackContext.h"

struct REG_LIST {
    int num = 0;
    arm64_reg regs[31] = {};
};

#define BUFFER_SIZE (1024 * 1024 * 1)

class GumTrace {
public:
    static GumTrace *get_instance();
    std::map<std::string, std::map<std::string, std::size_t>> modules;
    char* trace_file_path;
    std::ofstream trace_file;
    int trace_thread_id;

    GumStalker* _stalker;
    GumStalkerTransformer* _transformer;

    CallbackContext* callback_context_instance;

    static void transform_callback(GumStalkerIterator *iterator, GumStalkerOutput *output, gpointer user_data);
    std::string in_range_module(size_t address);
    std::map<std::string, std::size_t> get_module_by_name(const std::string &module_name);
    void follow();
    void unfollow();

    static void callout_callback(GumCpuContext *cpu_context, gpointer user_data);

    char buffer[BUFFER_SIZE] = {};
    REG_LIST write_reg_list;
private:
    GumTrace();

    ~GumTrace();

    GumTrace(const GumTrace &) = delete;

    GumTrace &operator=(const GumTrace &) = delete;
};


#endif //GUMTRACE_GUMTRACE_H