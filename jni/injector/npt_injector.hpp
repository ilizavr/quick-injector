#include <stdint.h>
#include "memory/memory.hpp"

class c_npt_injector{
private:
	c_memory* memory;
	unsigned char* old_memory;
	int memory_size;

	uintptr_t load_shellcode_dl(const char * libname);
	void no_ptrace_call(uintptr_t fncaddr);

	void _inject_library(const char * proc,const char * activity, const char * libname, const char * inject_mode);


public:
	static void inject_library(const char * proc, const char * activity, const char * libname, const char * inject_mode);
};