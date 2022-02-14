#ifndef _resche_hpp

#define _resche_hpp

struct interrupt_frame;

void resche();
void resche_leave_kernel_mode_hook();
void resche_tick(interrupt_frame *frame);

#endif
