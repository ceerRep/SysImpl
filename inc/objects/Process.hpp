#ifndef _process_hpp

#define _process_hpp

#include "Object.hpp"
#include "Segment.hpp"

#include <kernel_defines.h>
#include <protected_mode.hpp>
#include <shared_ptr.hpp>
#include <stdexcept.h>

struct ProcessLimitExceededException : public std::exception
{
    virtual const char *what() const throw() override
    {
        return "too many processes";
    }
};

struct ObjectLimitExceededException : public std::exception
{
    virtual const char *what() const throw() override
    {
        return "too many objects";
    }
};

struct InvalidObjectException : public std::exception
{
    virtual const char *what() const throw() override
    {
        return "invalid objects";
    }
};

struct InvalidSectionException : public std::exception
{
    virtual const char *what() const throw() override
    {
        return "invalid section";
    }
};

struct Section
{
    uintptr_t base;
    uintptr_t limit;
    const void *buffer;
};

class Process : public virtual Object
{
public:
    enum
    {
        MAX_PROCESS_NUM = 32,
        PROCESS_MAX_SEGMENTS = 16,
        PROCESS_MAX_OBJECTS = 128,
        PROCESS_STACK_BACKUP_SIZE = 128,
        PROCESS_STACK_SIZE = 8192,
        PROCESS_STACK_ALIGNMENT = 4096,

        PROCESS_SCHE_NORMAL = 0,
        PROCESS_SCHE_IDLE = 1,

        PROCESS_NORMAL_OBJECT_BEGIN = 3, // skip 0 1 2
        PROCESS_SEGMENT_BEGIN = PROCESS_MAX_OBJECTS - PROCESS_MAX_SEGMENTS,

        PROCESS_USER_IMAGE_SPACE = 640_k
    };

    enum
    {
        PROCESS_STATE_RUNNABLE,
        PROCESS_STATE_SLEEPING
    };

private:
    // declare user_stack_pos as pointer to array PROCESS_STACK_SIZE of char
    inline static char (*user_stack_pos)[PROCESS_STACK_SIZE];

    // declare kernel_stack_pos as pointer to array PROCESS_STACK_SIZE of char
    inline static char (*(kernel_stack_pos))[PROCESS_STACK_SIZE];

    inline static int current_process;
    inline static shared_ptr<Process> process_list[Process::MAX_PROCESS_NUM] = {};

    int pid;
    int ppid;
    int process_state;
    int process_sche_type;

    // See fork()
    uintptr_t stack_top_backup;
    shared_ptr<void> stack_backup;

    tss_entry_struct_t usermode_state;
    shared_ptr<Object> objects[Process::PROCESS_MAX_OBJECTS];

public:
    static void setKernelStack(void *addr) { kernel_stack_pos = decltype(kernel_stack_pos)(addr); }
    static void setUserStack(void *addr) { user_stack_pos = decltype(user_stack_pos)(addr); }
    static int getCurrentProcess() { return current_process; }
    static void setCurrentProcess(int pid) { current_process = pid; }
    static Process *getProcess(int pid) { return process_list[pid]; }
    static void deleteProcess(int pid, int code);

    // should called after enter and before leave kernel mode
    static void enterKernelMode();
    static void leaveKernelMode();

    Process(Object *parent);

    // remove all objects
    ~Process();

    int getPid() const
    {
        return pid;
    }

    int getPpid() const
    {
        return ppid;
    }

    int getProcessState() const
    {
        return process_state;
    }

    void setProcessState(int state)
    {
        process_state = state;
    }

    int getProcessScheType() const
    {
        return process_sche_type;
    }

    void setProcessScheType(int type)
    {
        process_sche_type = type;
    }

    tss_entry_struct_t *getUsermodeState()
    {
        return &usermode_state;
    }

    int addObject(shared_ptr<Object> obj);
    int setObject(int pos, shared_ptr<Object> new_obj);
    shared_ptr<Object> getObject(int pos);
    void removeObject(int pos);

    // Warning: without paging, the two process shares memory, so we backup the memory near its top to avoid stack corruption
    // Parent process will keep sleeping until exec() called
    shared_ptr<Process> fork();

    // terminated by NULL
    // (object_keep_high << 32) || object_keep_low is a bitmap indicates should keep which objects, 1 for keep
    void exec(char **args, Section *sections, int section_num, void *entry_point, uint32_t object_keep_low, uint32_t object_keep_high);

    void restoreStack();

    void run();

    // Called when parent process dies
    virtual void onRemovedByOwner(Object *owner) override;
};

#endif
