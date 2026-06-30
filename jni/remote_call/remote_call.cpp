#include "remote_call.hpp"

#include <asm/ptrace.h>
#include <sys/ptrace.h>
#include <elf.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <dlfcn.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

#include "logger/log.h"

void *c_remote_call::mmap(void* addr, size_t size, int prot, int flags, int fd, off_t offset){
    LOGI("calling remote mmap");
    return (void*)call(m_memory->find_remote_symbol("mmap64"),(long long)addr,size,prot,flags,fd,offset);
}
void *c_remote_call::mremap(void *old_address, size_t old_size, size_t new_size, int flags, long long new_address){
    LOGI("calling remote mremap");
    return (void*)call(m_memory->find_remote_symbol("mremap"),(long long)old_address,old_size,new_size,flags,(long long)new_address);
}
long long c_remote_call::mprotect(void *addr, size_t size, int prot){
    LOGI("calling remote mprotect");
    return call(m_memory->find_remote_symbol("mprotect"),(long long)addr,size,prot);
}
void *c_remote_call::memmove(void* dst,void* src, size_t size){
    LOGI("calling remote memmove");
    return (void*)call(m_memory->find_remote_symbol("memmove"),(long long)dst,(long long)src,size);
}
void *c_remote_call::dlopen(const char* module, int flags){
    LOGI("calling remote dlopen");
    return (void*)call(m_memory->find_remote_symbol("dlopen"),m_memory->create_remote_string(module),flags);
}
void c_remote_call::waitexec(uintptr_t code_cave){
    LOGI("waiting execution");
    /*ptrace_detach();
    sleep(1);
    ptrace_attach();*/

    user_pt_regs regs;
    while(true){
        ptrace_detach();
        ptrace_attach();
        ptrace_getregs(&regs);
        if(regs.pc == (code_cave + sizeof(shellcode_data)-4)) {
            ptrace_detach();
            ptrace_attach();
            break;
        }
    }
}
long long c_remote_call::call(uintptr_t fnc_address, 
    long long x0,
    long long x1,
    long long x2,
    long long x3,
    long long x4,
    long long x5,
    long long x6,
    long long x7)
{
    printf("\n");
	LOGI("calling function %llx",fnc_address);
	
	uintptr_t code_cave = load_shellcode(fnc_address);

	user_pt_regs regs,old_regs;

    ptrace_getregs(&regs);
    ptrace_getregs(&old_regs);

	regs.pc = code_cave;
	regs.regs[0] = x0;
	regs.regs[1] = x1;
	regs.regs[2] = x2;
	regs.regs[3] = x3;
	regs.regs[4] = x4;
	regs.regs[5] = x5;
	regs.regs[6] = x6;
	regs.regs[7] = x7;
    LOGI("setting registers pc: %llx args: %llx %llx %llx %llx %llx %llx %llx %llx",regs.pc,x0,x1,x2,x3,x4,x5,x6,x7);
    ptrace_setregs(&regs);

	waitexec(code_cave);

	ptrace_getregs(&regs);
    LOGI("checking pc %llx",regs.pc);
	LOGI("return %llx",regs.regs[0]);

	//LOGI("restoring registers");
    ptrace_setregs(&old_regs);

	restore_codecave(code_cave);

    printf("\n");
	return regs.regs[0];
}

uintptr_t c_remote_call::load_shellcode(uintptr_t fnc_address)
{
	//LOGI("loading shellcode");
	
	m_memory_size = sizeof(shellcode_caller);

    uintptr_t addr = free_codecave;
    free_codecave = 0;
    if(!addr) addr = m_memory->find_code_cave(m_memory_size,EXECUTE_READ);
	
	if(!addr)
		LOGE("codecave not found");

	LOGI("loading shellcode to codecave %llx",addr);
	
	shellcode_caller * ret = new shellcode_caller(fnc_address);
	m_old_memory = new unsigned char[m_memory_size];

	m_memory->memcpy_back(addr, m_old_memory, m_memory_size);
	m_memory->memcpy_v2((unsigned char*)ret, addr, m_memory_size);

	free(ret);

	//LOGI("shellcode inited");
	
	return addr;
}

void c_remote_call::restore_codecave(uintptr_t addr)
{
	//LOGI("restoring codecave");
    free_codecave=addr;	
	m_memory->memcpy_v2(m_old_memory, addr, m_memory_size);
}

void c_remote_call::ptrace_attach()
{
    int status = 0;
    if (ptrace(PTRACE_ATTACH, m_pid, NULL, NULL) == -1) 
        LOGE("cannot attach to %d", m_pid);

    //LOGI("ptrace_attach success");
    waitpid(m_pid, &status, WUNTRACED);
}
void c_remote_call::ptrace_detach()
{
    if (ptrace(PTRACE_DETACH, m_pid, NULL, NULL) == -1) 
        LOGE("failed detach from %d", m_pid);

    //LOGI("ptrace_detach success");
}
void c_remote_call::ptrace_getregs(struct user_pt_regs *regs)
{
    int regset = NT_PRSTATUS;
    struct iovec ioVec;

    ioVec.iov_base = regs;
    ioVec.iov_len = sizeof(*regs);
    if (ptrace(PTRACE_GETREGSET, m_pid, reinterpret_cast<void*>(regset), &ioVec) == -1)
        LOGE("ptrace_getregs failed");

    //LOGI("ptrace getregs success"); 
}
void c_remote_call::ptrace_setregs(struct user_pt_regs *regs)
{
    int regset = NT_PRSTATUS;
    struct iovec ioVec;

    ioVec.iov_base = regs;
    ioVec.iov_len = sizeof(*regs);
    if (ptrace(PTRACE_SETREGSET, m_pid, reinterpret_cast<void*>(regset), &ioVec) == -1)
        LOGE("ptrace_setregs failed");

    //LOGI("ptrace setregs success"); 
}