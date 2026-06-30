#include <stdint.h>
#include "memory/memory.hpp"
#include "remote_call/remote_call.hpp"

class c_injector
{
private:
	c_memory* memory;
	c_remote_call* remote_call;
	void _inject_library(const char * proc,const char * activity, const char * libname, const char * inject_mode);

public:
	static void inject_library(const char * proc, const char * activity, const char * libname, const char * inject_mode);
};