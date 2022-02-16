#include <objects/Process.hpp>
#include <syscall/syscall.hpp>
#include <syscall/syscall_handler.hpp>

#include <objects/File.hpp>
#include <objects/FileSystem.hpp>
#include <objects/SpecialPathManager.hpp>

#include <errno.h>

uint64_t open_handler(uint32_t filename)
{
    char *pfilename = (char *)filename;
    auto now_proc = Process::getProcess(Process::getCurrentProcess());

    auto device_manager = SpecialPathManager::getInstance();

    shared_ptr<Object> obj;

    if (obj = device_manager->get(pfilename); !obj)
    {
        auto file = Fat16FileSystem::openFile(nullptr, pfilename);

        if (!file)
            return -ENOENT;
        
        obj = file.cast<Object>();
    }

    try
    {
        return now_proc->addObject(obj);
    }
    catch (ObjectLimitExceededException)
    {
        return ENOSPC;
    }
}

static void __attribute__((constructor, used)) register_open_handler()
{
    register_syscall(SYSCALL_OPEN, new SyscallWrapper(&open_handler));
}
