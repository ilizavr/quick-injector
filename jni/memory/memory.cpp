#include "memory.hpp"

#include <filesystem>
#include <fstream>
#include <algorithm>
#include <cctype>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <regex>
#include <dlfcn.h>
#include <fcntl.h>

#include "logger/log.h"
#include "memory/ProcessManager.h"



namespace fs = std::filesystem;

bool pvm(void *address, void *buffer, size_t size, bool iswrite, pid_t game_pid)
{
    struct iovec local[1];
    struct iovec remote[1];
    local[0].iov_base = buffer;
    local[0].iov_len = size;
    remote[0].iov_base = address;
    remote[0].iov_len = size;
    if (game_pid < 0)
    {
        return false;
    }
    ssize_t bytes = syscall(iswrite ? SYS_process_vm_writev : SYS_process_vm_readv, game_pid, local, 1, remote, 1, 0, iswrite);
    return bytes == size;
}

bool vm_readv(unsigned long address, void *buffer, size_t size, pid_t game_pid)
{
    return pvm(reinterpret_cast<void *>(address), buffer, size, false, game_pid);
}

bool vm_writev(unsigned long address, void *buffer, size_t size, pid_t game_pid)
{
    return pvm(reinterpret_cast<void *>(address), buffer, size, true, game_pid);
}

c_memory::c_memory(std::string package)
{
    pid = find_pid(package);
}

int c_memory::find_pid(std::string package)
{
    while(true){
        const fs::path proc{"/proc/"};

        for(auto dir : fs::directory_iterator{proc})
        {
            if(!dir.is_directory()) continue;

            std::string dir_name = dir.path().filename();

            if(!std::all_of(dir_name.begin(), dir_name.end(), ::isdigit))
                continue;

            int pid = std::stoi(dir_name);

            auto path = dir.path() / fs::path("cmdline");

            if(!fs::exists(path))
                continue;

            std::ifstream file{path.string(), std::ios::in};
            if(!file.is_open())
                continue;

            std::string cur_package;
            std::getline(file, cur_package, '\0');

            if(cur_package == package)
            {
                file.close();
                LOGI("found pid: %d", pid);
                return pid;
            }
        }
    }
}


void* sdlopen(const char* module, long long flag)
{
    void* module_handle = dlopen(module, flag);
    if(!module_handle)
        LOGE("dlopen failed: %s", module);

    return module_handle;
}
void* sdlsym(void* handle, const char* symbol)
{
    void* local_fn_address = dlsym(handle, symbol);
    if(!local_fn_address)
        LOGE("dlsym failed: %s", symbol);

    return local_fn_address;
}

void c_memory::memcpy(unsigned char* src, uintptr_t dest, int size)
{
    LOGI("write %d bytes to %llx", size, dest);

    vm_writev(dest, reinterpret_cast<void *>(src), size, pid);
}
void c_memory::memcpy_v2(unsigned char* src, uintptr_t dest, int size)
{
    LOGI("write %d bytes to %llx", size, dest);

    char memfile[24];
    sprintf(memfile,"/proc/%d/mem",pid);

    int fd = open(memfile, O_RDWR);

    pwrite(fd, src, size, dest);

    close(fd);
}
void c_memory::memcpy_back(uintptr_t src, unsigned char* dest, int size)
{
    LOGI("read %d bytes from %llx", size, src);

    vm_readv(src, dest, size, pid);
}

template <typename T> 
T read_proc_mem(uintptr_t addr)
{
    T ret;
    memcpy_back(addr,&ret,sizeof(T));
    return ret;
}

template <typename T> 
void write_proc_mem(uintptr_t addr, T val)
{
    memcpy(&val,addr,sizeof(T));
}

template <typename T> 
void write_proc_mem_v2(uintptr_t addr, T val)
{
    memcpy_v2(&val,addr,sizeof(T));
}

uintptr_t c_memory::find_code_cave(int size, int prot)
{
    ProcessManager proc(pid);
    return proc.FindCodeCave(size,prot);
}

uintptr_t c_memory::find_remote_symbol(std::string module, std::string symbol)
{
    c_memory local_memory{};
    uintptr_t local_module_address = local_memory.get_module_base(module);
    uintptr_t remote_module_address = get_module_base(module);

    void* module_handle = sdlopen(module.c_str(), 1);
    uintptr_t local_fn_address = (uintptr_t)sdlsym(module_handle, symbol.c_str());
    LOGI("function offset: %s:%llx", symbol.c_str(), local_fn_address - local_module_address);

    uintptr_t remote_fn_address = local_fn_address - local_module_address + remote_module_address;
    LOGI("found remote function: %s:%llx", symbol.c_str(), remote_fn_address);

    return remote_fn_address;
}
uintptr_t c_memory::create_remote_string(std::string str)
{
	uintptr_t addr = find_code_cave(str.size(),READ);
	if(!addr){
		LOGE("codecave not found");
	}
	memcpy_v2((unsigned char*)str.c_str(), addr, str.size());

    LOGI("create remote string %llx:%s",addr,str.c_str());

	return addr;
}
uintptr_t c_memory::find_remote_symbol(std::string symbol)
{
    c_memory local_memory{};
    uintptr_t local_fn_address = (uintptr_t)sdlsym(RTLD_NEXT, symbol.c_str());

    return find_remote_symbol(local_memory.get_path(local_fn_address), symbol);
}

std::vector<c_memory::maps_line> c_memory::find_in_maps(std::string regexp)
{
    std::vector<maps_line> maps_vector;

    std::string pid_str = std::to_string(pid);
    const fs::path path{"/proc/" + pid_str + "/maps"};

    if(!fs::exists(path))
    {
        LOGE("maps file not exists: %s", path.c_str());
    }

    std::ifstream file{path.string(), std::ios::in};
    if(!file.is_open())
    {
        LOGE("can't open maps file: %s", path.c_str());
    }

    std::smatch match;
    std::string current_line;
    while(std::getline(file, current_line))
    {
        if(!std::regex_search(current_line, match, std::regex(regexp)))
            continue;

        
        maps_line line;
        uintptr_t unk1;
        

        sscanf(current_line.c_str(), "%llx-%llx %s %x %02x:%02x %d %s",
        &line.start,
        &line.end, 
        line.prot,
        &unk1,
        &unk1,
        &unk1,
        &unk1,
        line.path
        );

        maps_vector.push_back(line);
    }
    return maps_vector;
}

std::string c_memory::get_path(uintptr_t function_address)
{
    std::vector<maps_line> maps_entries = find_in_maps(".*");
    
    for (maps_line entry : maps_entries) {
        if (function_address >= entry.start && function_address < entry.end) {
            LOGI("success found path: %s:%llx", entry.path, function_address);
            return entry.path;
        }
    }

    LOGE("cant find path for: %llx", function_address);
    return "";
}

uintptr_t c_memory::get_module_base(std::string module)
{   
    std::vector<maps_line> maps = find_in_maps(module);
    if(!maps.size())
        LOGE("not found module: %s", module.c_str());

    maps_line line = maps[0];

    LOGI("found module base: \n{start: %llx, end: %llx, prot: %s, path: %s}", 
        line.start, line.end, line.prot, line.path
    );

    return line.start;
}
int c_memory::get_pid()
{
    return pid;
}