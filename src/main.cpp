//
// Created by lidongyooo on 2026/2/5.
//


#include "GumTrace.h"
#include "Utils.h"

gboolean module_symbols_cb(const GumSymbolDetails * details, gpointer user_data) {
    auto *instance = GumTrace::get_instance();
    instance->func_maps[details->address] = details->name;
    if (details->is_global) {
        size_t global_addr = gum_module_find_global_export_by_name(details->name);
        if (global_addr > 0) {
            instance->func_maps[global_addr] = details->name;
        }
    }

    return true;
}

gboolean module_dependency_cb (const GumDependencyDetails * details, gpointer user_data) {
    auto gum_module = gum_process_find_module_by_name(details->name);
    gum_module_enumerate_symbols(gum_module, module_symbols_cb, nullptr);
    return true;
}

gboolean module_enumerate (GumModule * module, gpointer user_data) {
    auto instance = GumTrace::get_instance();
    const char *module_name = gum_module_get_name(module);
    if (instance->modules.count(module_name) <= 0) {
        gum_stalker_exclude(instance->_stalker, gum_module_get_range(module));
    }

    return true;
}

extern "C" __attribute__((visibility("default")))
void init(const char *module_names, char *trace_file_path, int thread_id) {

    gum_init();

    GumTrace *instance = GumTrace::get_instance();
    auto module_names_vector = Utils::str_split(module_names, ',');
    for (const auto &module_name: module_names_vector) {
        auto &module_map = instance->modules[module_name];
        auto *gum_module = gum_process_find_module_by_name(module_name.c_str());
        gum_module_enumerate_symbols(gum_module, module_symbols_cb, nullptr);
        gum_module_enumerate_dependencies(gum_module, module_dependency_cb, nullptr);;
        auto *gum_module_range = gum_module_get_range(gum_module);
        module_map ["base"] = gum_module_range->base_address;
        module_map ["size"] = gum_module_range->size;
    }

    gum_process_enumerate_modules(module_enumerate, nullptr);

    memcpy(instance->trace_file_path, trace_file_path, strlen(trace_file_path));
    instance->trace_thread_id = thread_id;
    instance->trace_file = std::ofstream(instance->trace_file_path, std::ios::out | std::ios::trunc);
}

void* thread_function(void* arg) {
    GumTrace *instance = GumTrace::get_instance();
    size_t last_size = 0;

    while (true) {
        if (instance->trace_file.is_open()) {
            struct stat stat_buf;
            int ret = stat(instance->trace_file_path, &stat_buf);

            if (ret == 0) {
                off_t growth = stat_buf.st_size - last_size;
                off_t growth_mb = growth / (1024 * 1024);
                off_t size_gb = stat_buf.st_size / (1024 * 1024 * 1024);

                LOGE("每20秒新增：%ldMB 当前文件大小：%ldGB",
                     growth_mb, size_gb);

                last_size = stat_buf.st_size;
            } else {
                LOGE("stat 失败，错误码：%d，错误信息：%s",
                     errno, strerror(errno));
                LOGE("文件路径：%s", instance->trace_file_path);
            }

            instance->trace_file.flush();
        } else {
            LOGE("trace_file 未打开");
            break;
        }

        sleep(20);
    }

    return nullptr;
}


extern "C" __attribute__((visibility("default")))
void run() {

    pthread_t thread1;
    pthread_create(&thread1, NULL, thread_function, nullptr);

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
