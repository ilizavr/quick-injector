#pragma once
#include "memory/memory.hpp"

constexpr unsigned char shellcode_data[] =
{
	0x88, 0x00, 0x00, 0x10, 0x08, 0x01, 0x40, 0xf9, 0x00, 0x01, 0x3f, 0xd6, 0x00, 0x00, 0x00, 0x14
};

class c_remote_call
{
    c_memory* m_memory;
    int m_pid;
    unsigned char* m_old_memory;
	int m_memory_size;
    uintptr_t free_codecave = 0;

    uintptr_t load_shellcode(uintptr_t fnc_address);
	void restore_codecave(uintptr_t addr);
    void waitexec(uintptr_t code_cave);

    void ptrace_getregs(struct user_pt_regs *regs);
    void ptrace_setregs(struct user_pt_regs *regs);
    
public:
    struct shellcode_caller
    {
        unsigned char shell[sizeof(shellcode_data)];
        uintptr_t fnc_address;

        shellcode_caller(uintptr_t fnc_address) : fnc_address(fnc_address)
        {
            memcpy(shell, shellcode_data, sizeof(shellcode_data));
        }
    };

    c_remote_call(c_memory* memory) : m_memory(memory), m_pid(memory->get_pid()) {}

    void *mremap(void *old_address, size_t old_size, size_t new_size, int flags, long long new_address);
    void *mmap(void* addr, size_t size, int prot, int flags, int fd, off_t offset);
    void *memmove(void* dst,void* src, size_t size);
    long long mprotect(void* addr, size_t size, int prot);
    void *dlopen(const char* module, int flags);

    long long call(uintptr_t fnc_address, 
        long long x0 = 0,
        long long x1 = 0,
        long long x2 = 0,
        long long x3 = 0,
        long long x4 = 0,
        long long x5 = 0,
        long long x6 = 0,
        long long x7 = 0
    );

    void ptrace_attach();
    void ptrace_detach();
};