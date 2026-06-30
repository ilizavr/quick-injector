#include <sys/mman.h>
#include <dlfcn.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

#include "memory/memory.hpp"
#include "injector/injector.hpp"
#include "logger/log.h"
#include "remote_remap.h"


void c_injector::_inject_library(const char * proc,const char * activity, const char * libname, const char * inject_mode)
{
	LOGI("starting process %s",proc);
	char cmd[255];
	sprintf(cmd, "am start %s/%s",proc, activity);
	system(cmd);

	printf("\n\n");
	LOGI("waiting process %s",proc);
	memory = new c_memory(proc);
	remote_call = new c_remote_call(memory);
	remote_call->ptrace_attach();
	
	printf("\n\n");
	LOGI("injecting %s mode %s",libname, inject_mode);

	if(!strcmp(inject_mode,"dl")||!strcmp(inject_mode,"rm")){
		remote_call->dlopen(libname,1);
		LOGI("injected");
	}
	if(!strcmp(inject_mode,"rm")){
		printf("\n\n");
		LOGI("remaping library %s",libname);

		RemapTools::RemapLibrary(memory->get_pid(),remote_call,libname);
	}

	remote_call->ptrace_detach();
	
}

void c_injector::inject_library(const char * proc, const char * activity, const char * libname, const char * inject_mode)
{
	c_injector injector;
	injector._inject_library(proc,activity,libname,inject_mode);
}
