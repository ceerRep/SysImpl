#include <objects/Process.hpp>
#include <syscall/syscall.hpp>
#include <syscall/syscall_handler.hpp>

#include <objects/Directory.hpp>
#include <objects/File.hpp>
#include <objects/FileSystem.hpp>

#include <common_def.h>
#include <errno.h>
#include <string.h>

uint64_t readdir_handler(uint32_t fd, uint32_t info_array_addr, uint32_t n_addr)
{
    int *pn = (int *)n_addr;
    file_info_t *info_array = (file_info_t *)info_array_addr;

    auto now_proc = Process::getProcess(Process::getCurrentProcess());
    auto file_obj = now_proc->getObject(fd);

    if (!file_obj)
        return -ENOENT;

    auto file = file_obj.cast<File>();

    if (!file)
        return -ENOENT;

    auto dir = Directory::fromFile(file);

    if (!dir)
        return -ENOENT;

    int i;
    for (i = 0; i < *pn && i < dir->size(); i++)
    {
        auto info = (*dir)[i];
        strlcpy(info_array[i].filename, info.filename, 16);
        info_array[i].size = info.size;
        info_array[i].directory = info.directory;
    }

    *pn = dir->size();

    return i;
}

static void __attribute__((constructor, used)) register_readdir_handler()
{
    register_syscall(SYSCALL_READDIR, new SyscallWrapper(&readdir_handler));
}
