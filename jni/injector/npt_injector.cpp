#include <sys/mman.h>
#include <dlfcn.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

#include "memory/memory.hpp"
#include "logger/log.h"
#include "injector/npt_injector.hpp"


unsigned char dl_shell_[] = {
	//0xff, 0x43, 0x00, 0xd1, 0xfe, 0x07, 0x00, 0xf9, 0x40, 0x01, 0x00, 0x10, 0x21, 0x00, 0x80, 0xd2, 0xc8, 0x00, 0x00, 0x10, 0x08, 0x01, 0x40, 0xf9, 0x00, 0x01, 0x3f, 0xd6, 0xfe, 0x07, 0x40, 0xf9, 0xff, 0x43, 0x00, 0x91, 0x00, 0x00, 0x00, 0x14,
};//переписать
//1. востановление ориг 
//2. запуск ориг
//3. инжект
//4. рет

struct shellcode_dl
{
	unsigned char shell[sizeof(dl_shell_)];
	long long gldrawelements;
	long long dlopen;
	char lib[0];
};

void c_npt_injector::no_ptrace_call(uintptr_t fncaddr){
	//redirect dldrawelements to fncaddr
	//hook
}

uintptr_t c_npt_injector::load_shellcode_dl(const char * libname)
{
	memory_size = sizeof(shellcode_dl) + strlen(libname) + 1;
	uintptr_t addr = memory->find_code_cave(memory_size,EXECUTE_READ);
	
	if(!addr){
		LOGE("codecave not found");
	}
	LOGI("loading shellcode to codecave %llx",addr);
	
	shellcode_dl * ret = (shellcode_dl*)malloc(memory_size);
	old_memory = new unsigned char[memory_size];
	
	memcpy(ret->shell, dl_shell_, sizeof(dl_shell_));
	strcpy(ret->lib, libname);
	ret->dlopen = memory->find_remote_symbol("dlopen");
	ret->gldrawelements = memory->find_remote_symbol("/system/lib64/libGLESv2.so","glDrawElements");
	
	memory->memcpy_back(addr, old_memory, memory_size);
	memory->memcpy_v2((unsigned char*)ret, addr, memory_size);

	free(ret);
	
	return addr;
}

void c_npt_injector::_inject_library(const char * proc,const char * activity, const char * libname, const char * inject_mode)
{
	LOGE("SOON");

	LOGI("starting process %s",proc);
	char cmd[255];
	sprintf(cmd, "am start %s/%s",proc, activity);
	system(cmd);

	LOGI("waiting process %s",proc);
	memory = new c_memory(proc);
	
	LOGI("injecting %s mode %s",libname,inject_mode);

	uintptr_t code_cave = 0;

	if(!strcmp(inject_mode,"npdl")) code_cave = load_shellcode_dl(libname);

	no_ptrace_call(code_cave);

	LOGI("injected");
}

void c_npt_injector::inject_library(const char * proc, const char * activity, const char * libname, const char * inject_mode)
{
	c_npt_injector injector;

	injector._inject_library(proc,activity,libname,inject_mode);
}
