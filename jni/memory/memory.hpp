#pragma once
#include <stdio.h>
#include <vector>
#include <string>
#include <unistd.h>

#define READ 0
#define READ_WRITE 1
#define EXECUTE_READ 2
#define EXECUTE_READ_WRITE 3

class c_memory
{
private:
    int pid;

public:
    struct maps_line
    {
        uintptr_t start;
        uintptr_t end;
        char prot[5];
        char path[256];
    };

    c_memory(std::string package);
    c_memory(int pid) : pid(pid) {}
    c_memory() : pid(getpid()) {}

    template <typename T> T read_proc_mem(uintptr_t addr);
    template <typename T> void write_proc_mem(uintptr_t addr, T val);
    template <typename T> void write_proc_mem_v2(uintptr_t addr, T val);
    void memcpy(unsigned char* src, uintptr_t dest, int size);
    void memcpy_v2(unsigned char* src, uintptr_t dest, int size);
    void memcpy_back(uintptr_t src, unsigned char* dest, int size);
    uintptr_t find_code_cave(int size, int prot);
    std::vector<maps_line> find_in_maps(std::string regexp);
    uintptr_t get_module_base(std::string regexp);
    uintptr_t find_remote_symbol(std::string module_regexp, std::string symbol);
    uintptr_t find_remote_symbol(std::string symbol);
    uintptr_t create_remote_string(std::string string);
    std::string get_path(uintptr_t function_address);
    int get_pid();

    static int find_pid(std::string package);
};