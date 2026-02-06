//
// Created by lidongyooo on 2026/2/5.
//


#include "GumTrace.h"
#include "Utils.h"

extern "C" __attribute__((visibility("default")))
void init(const char *module_names, char *trace_file_path, int thread_id) {

    gum_init();

    GumTrace *instance = GumTrace::get_instance();
    auto module_names_vector = Utils::str_split(module_names, ',');
    for (const auto &module_name: module_names_vector) {
        auto &module_map = instance->modules[module_name];
        auto *gum_module = gum_process_find_module_by_name(module_name.c_str());
        auto *gum_module_range = gum_module_get_range(gum_module);
        module_map ["base"] = gum_module_range->base_address;
        module_map ["size"] = gum_module_range->size;
    }

    instance->trace_file_path = trace_file_path;
    instance->trace_thread_id = thread_id;

    instance->trace_file = std::ofstream(instance->trace_file_path, std::ios::out | std::ios::trunc);


// gum_module_enumerate_symbols(gum_module, module_symbol_cb, nullptr);
// gum_module_enumerate_dependencies(gum_module, module_dependency_cb, nullptr);
    // instance->follow();
}

extern "C" __attribute__((visibility("default")))
void run() {
    GumTrace *instance = GumTrace::get_instance();
    instance->follow();
}

extern "C" __attribute__((visibility("default")))
void unrun() {
    GumTrace *instance = GumTrace::get_instance();
    instance->unfollow();
}

int main() {
    printf("xxx %p", main);
}
