#include <objects/BeforeAttachedHook.hpp>
#include <objects/Process.hpp>

#include <assert.h>
#include <common_def.h>
#include <ld_syms.h>
#include <protected_mode.hpp>
#include <string.h>

#include <syscall/syscall_handler.hpp>

extern tss_entry_struct_t **tss_array;

Process::Process(Object *parent)
{
    pid = -1;
    ppid = -1;

    if (parent)
    {
        if (auto p = dynamic_cast<Process *>(parent))
        {
            ppid = p->pid;
        }
    }

    for (int i = 0; i < Process::MAX_PROCESS_NUM; i++)
    {
        if (Process::process_list[i] == nullptr)
        {
            Process::process_list[i] = create_shared(this);
            pid = i;
            break;
        }
    }

    if (pid == -1)
    {
        throw ProcessLimitExceededException();
    }
}

Process::~Process()
{
    for (int i = 0; i < Process::PROCESS_MAX_OBJECTS; i++)
        removeObject(i);

    if (ppid >= 0)
    {
        if (process_list[ppid])
        {
            process_list[ppid]->restoreStack();
        }
    }
}

static int entered = 0;

void Process::enterKernelMode()
{
    if (process_list[current_process])
        process_list[current_process]->usermode_state = *tss_array[SEG_USER_TSS];
}

void Process::leaveKernelMode()
{
    if (process_list[current_process])
        *tss_array[SEG_USER_TSS] = process_list[current_process]->usermode_state;
}

int Process::addObject(shared_ptr<Object> obj)
{
    if (!obj)
        throw InvalidObjectException();

    for (int i = (dynamic_cast<Segment *>(obj.get())
                      ? PROCESS_ADD_SEGMENT_BEGIN
                      : PROCESS_ADD_OBJECT_INDEX_BEGIN);
         i < Process::PROCESS_MAX_OBJECTS;
         i++)
    {
        if (!objects[i])
        {
            if (auto hook = dynamic_cast<BeforeAttachedHook *>(obj.get()))
                objects[i] = hook->beforeAttachedToObjectHook(this, obj);
            else
                objects[i] = obj;
            return i;
        }
    }

    throw ObjectLimitExceededException();
}

void Process::removeObject(int pos)
{
    if (objects[pos])
    {
        objects[pos]->onRemovedByOwner(this);
        objects[pos] = nullptr;
    }
}

shared_ptr<Object> Process::setObject(int pos, shared_ptr<Object> obj)
{
    auto ret = objects[pos];
    removeObject(pos);

    if (!obj)
        throw InvalidObjectException();

    if (auto hook = dynamic_cast<BeforeAttachedHook *>(obj.get()))
        objects[pos] = hook->beforeAttachedToObjectHook(this, obj);
    else
        objects[pos] = obj;

    return ret;
}

shared_ptr<Object> Process::getObject(int pos)
{
    return objects[pos];
}

shared_ptr<Process> Process::fork()
{
    assert("Double fork in the same process, it shouldn't happen" && !stack_backup);

    Process *child = new Process(this);
    auto ret = process_list[child->getPid()];

    stack_backup = create_shared<void>(new char[PROCESS_STACK_BACKUP_SIZE]);

    stack_top_backup = usermode_state.esp;
    memcpy(stack_backup.get(), (void *)usermode_state.esp, PROCESS_STACK_BACKUP_SIZE);

    for (int i = 0; i < Process::PROCESS_MAX_OBJECTS; i++)
    {
        if (objects[i])
        {
            child->setObject(i, objects[i]);
        }
    }

    child->usermode_state = usermode_state;
    child->setProcessState(PROCESS_STATE_RUNNABLE);

    // return values for child
    child->usermode_state.*syscall_ret1 = 0;
    child->usermode_state.*syscall_ret2 = 0;

    // sleep
    setProcessState(PROCESS_STATE_SLEEPING);

    return ret;
}

void Process::exec(char **args, Section *sections, int section_num, void *entry_point, uint32_t object_keep_low, uint32_t object_keep_high)
{
    uint64_t bitmap = ((uint64_t)object_keep_high << 32) + object_keep_low;
    for (int i = 0; i < PROCESS_MAX_OBJECTS; i++)
    {
        if (!(bitmap & (1 << i)))
            removeObject(i);
    }

    // Clear
    memset(&usermode_state, 0, sizeof(usermode_state));

    // Initialize
    usermode_state.cs = SEGMENT_SELECTOR(SEG_USER_CODE, 0, 3);
    usermode_state.ss0 = SEGMENT_SELECTOR(SEG_KERNEL_DATA, 0, 0);
    usermode_state.ds = usermode_state.es = usermode_state.fs = usermode_state.gs = usermode_state.ss = SEGMENT_SELECTOR(SEG_USER_DATA, 0, 3);
    usermode_state.eip = (uintptr_t)entry_point;
    usermode_state.eflags |= 1 << 9; // enable interrupt
    usermode_state.iomap_base = sizeof(usermode_state);

    // Stack
    usermode_state.esp = (uintptr_t)(&(user_stack_pos[pid][0]) + PROCESS_STACK_SIZE);
    usermode_state.esp0 = (uintptr_t)(&(kernel_stack_pos[pid][0]) + PROCESS_STACK_SIZE);

    usermode_state.esp -= sizeof(process_runtime_info_t);

    process_runtime_info_t *info = (process_runtime_info_t *)usermode_state.esp;
    memset(info->args, 0xFF, sizeof(info->args));
    int total_length = 0;

    for (int i = 0; i < PROCESS_MAX_ARGUMENTS && args[i]; i++)
    {
        int length = strlen(args[i]);

        if (total_length + length + 1 <= sizeof(info->buffer))
        {
            strcpy(info->buffer + total_length, args[i]);
            info->args[i] = total_length;
            total_length += length + 1;
        }
        else
            break;
    }

    // write to ebx

    usermode_state.ebx = usermode_state.esp;

    for (int i = 0; i < section_num; i++)
    {

        if (sections[i].base < (uintptr_t)USER_SPACE_START)
        {
            throw InvalidSectionException();
        }

        auto segment = create_shared<Object>(new Segment(sections[i].base, sections[i].limit));
        memcpy((void *)sections[i].base, sections[i].buffer, sections[i].limit);

        addObject(segment);
    }

    if (ppid >= 0)
    {
        if (process_list[ppid])
        {
            process_list[ppid]->restoreStack();
        }
    }
}

void Process::restoreStack()
{
    if (stack_backup)
    {
        memcpy((void *)stack_top_backup, stack_backup, PROCESS_STACK_BACKUP_SIZE);
        stack_top_backup = 0;
        stack_backup = nullptr;

        // restore process state
        setProcessState(PROCESS_STATE_RUNNABLE);
    }
}

void Process::resume()
{
    setProcessState(PROCESS_STATE_RUNNABLE);
    *tss_array[SEG_USER_TSS] = usermode_state;
    current_process = pid;
}

void Process::onRemovedByOwner(Object *owner)
{
    // TODO
}
