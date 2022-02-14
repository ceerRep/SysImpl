#include <resche.hpp>

#include <kernel_defines.h>
#include <protected_mode.hpp>
#include <stdio.h>

#include <objects/Process.hpp>

#include <syscall/syscall.hpp>

static volatile int resche_counter = PROCESS_TICKS;

void resche()
{
    for (int i = 1; i <= Process::MAX_PROCESS_NUM; i++)
    {
        Process *proc = Process::getProcess((Process::getCurrentProcess() + i) % Process::MAX_PROCESS_NUM);

        if (proc && proc->getProcessState() == Process::PROCESS_STATE_RUNNABLE)
        {
            Process::setCurrentProcess(proc->getPid());

            resche_counter = PROCESS_TICKS;
            return;
        }
    }

    // no more user process
    // return to kernel

    // set INIT_TSS busy
    gdts[SEG_INIT_TSS].read_write = 1;
    // clear user tss busy
    gdts[SEG_USER_TSS].read_write = 0;
    tss_array[SEG_USER_TSS]->prev_tss = 0;

    tss_array[SEG_SYSCALL_TSS]->prev_tss = SEGMENT_SELECTOR(SEG_INIT_TSS, 0, 0);
}

void resche_tick(interrupt_frame *frame)
{
    if (resche_counter > 0)
        resche_counter--;

    bool kernel_mode = 1;

    if (frame->eflags & (1 << 17)) // VM, already entered kernel mode
    {
    }
    else if (gdts[frame->cs >> 3].DPL != 0)
    {
        kernel_mode = 0;
    }

    if (resche_counter == 0 && !kernel_mode)
    {
        // call resche
        asm volatile("int $0x80"
                     :
                     : "a"(SYSCALL_SCHED_YIELD)
                     : "memory", "cc");
    }
}

void resche_leave_kernel_mode_hook()
{
    if (resche_counter == 0)
        resche();
}
