# 操作系统实现

### ceerRep 2022/02/16

## 简介
该内核使用 C++ 编写，使用 libcxxrt 库提供了 RTTI 和异常支持。  
内核为单线程，所以不需要使用锁。  
用户程序可以操作的对象模仿 POSIX 在用户态体现为 fd，在内核态则为不同对象。  
对象继承自虚基类 `Object`，储存在 `shared_ptr<Object>` 数组中。  
fd 即为对应 `Object` 在当前进程结构数组中的下标。  

## 目录结构
- common_inc  
    内核和用户态共享的头文件  
- compile-rt-builtins   
    从 llvm 的 compile-rt 中拿的部分 builtin 函数实现  
    - 比如软浮点等  
- crt  
    内核态 c 运行时所需的部分文件  
- inc  
    内核的头文件目录  
- src  
  内核源文件目录  
    - elf  
    解析 elf 格式  
    - libcxxrt  
    C++ 运行时，提供了异常支持  
    - objects  
    对象  
    - syscall  
    系统调用实现  
    
- user  
用户态代码目录  
    - inc  
    用户态头文件目录  
    - lib  
    用户态库目录  
- ldscript.ld  
  指定了链接器怎么链接内核，并且暴露一些符号  
  在 `inc/ld_syms.h` 中有部分引用  

## 启动过程  
1. `ldscript.ld` 中定义了内核入口点为 `start` 符号，同时在 `start.S` 中开头的 multiboot 结构中也有该项定义。引导器会将内核加载到 `ldscript.ld` 中指定的位置并在初始化相关结构后跳转到入口点。  
2. `start.S` 头部定义了 `multiboot` 头，具体规范看这里 [Multiboot Specification version 0.6.96](https://www.gnu.org/software/grub/manual/multiboot/multiboot.html)  
    - 规范中要求该头部应当在前 8k 中，因此我们在 `ldscript.ld` 中指定 `start.S.o` 的代码段放在最前面  
    - 将 esp 赋值为在 `ldscript.ld` 中要求链接器专门留出的一段空洞，用作启动时的内核栈  
    - 将 `ebx` (Multiboot 信息结构的 32 位物理地址) `eax` (Magic Number 0x2BADB002) 入栈
    - 调用 `crt_init`，该函数  
        1. 在内核栈边界写入一个 protector，可以用 gdb watch 该地址检测内核栈溢出。  
        2. 备份 `multiboot_info` 和 `cmdline`, 防止被破坏   
        3. 设置启动时内核堆，同样指向链接时留出的空间。  
        4. 初始化 `stdout` 和 `stderr`，此处将其设置为 `EarlyStageOutputDevice`，一个向 `0xB8000` 显存直接写入的简单输出程序  
    - 调用 `_init`，该函数为编译器提供的 C 运行时初始化程序  
    - 调用 `call_main`，该函数调用一些全局变量初始化函数，并且捕获未捕获的异常并打印错误信息  
    - 内核退出后打印信息并死循环  
3. `main.cpp` 控制权移交给 `main`  
    1. 初始化 gdt  
        1. 由于 multiboot 引导器已经帮我们关闭了 A20 Gate 并进入了保护模式，此部分我们不用处理  
        2. 初始化 tss (task state segment), 一种硬件任务切换机制, 见 [Task State Segment (OSDEV)](https://wiki.osdev.org/Task_State_Segment). 我们用其实现状态的保存，以及中断时栈的切换  
        3. 分配 gdt, 分别为 空描述符 (0x0), 内核代码段 (0x8), 内核数据段 (0x10), 用户代码段 (0x18), 用户数据段 (0x20), 初始 tss (0x28), 用户 tss (0x30), 系统调用 tss (0x38), v8086 tss (0x40), 并装载  
        4. 长跳转 (虽然并不用)  
        5. 装载初始 tss  
    2. 初始化中断  
        1. 初始化中断描述符表 (idt)，将各个默认中断服务例程 (ISR) 填入 idt 中  
        2. 设置 General Protect 以及 0x80 的中断服务例程，后者为系统调用所用中断  
        3. 每个 ISR 开头都要实例化 SegmentRegsSetter，因为虽然我们的中断处理程序也使用 C/C++ 编写，编译器会自动帮我们保存通用寄存器，但段寄存器仍要我们自己保存并设置为正确的值  
        PS. 间接寻址会使用 ds，所以我们先保存在栈上    
    3. 初始化可编程中断控制器 (PIC)  
        历史上，外部中断由两个 8259 芯片通知 cpu。为保持兼容性，现代 IBM 兼容机仍然保留了其兼容接口。但由于历史原因，其中断有部分与保护模式处理器中断号重叠，我们需要将其重定位到阳间位置并暂时屏蔽所有中断请求 (IRQ)    
    4. 检查并关闭 apic  
        现代一些的中断控制器，我们不用它，为以防万一把它关掉  
    5. 启用时钟中断  
        告诉一个 1.193182MHz 的可编程间隔计时器 (PIT) 每隔 1193 个周期发起一次 IRQ0, 这样我们就有了一个 1kHz 的时钟  
        每隔一段时间将当前进程换出，如果当前在内核态不要直接发起切换，等退出内核态时再切换（保证单线程）  
        如果在用户态就调用 syscall 通知内核将自己换出 
    6. (可选) 初始化串口  
    7. 初始化虚拟 8086 模式，该内核中读取磁盘以及向屏幕输入输出都通过调用 BIOS 中断完成。  
        你们可以重写 `Disk.cpp:PhysicalDisk` 类等来在保护模式中完成此类工作  
    7. 由 `multiboot_info` 中的信息初始化文件系统  
    8. 用解析内核命令行得到的参数调用 init  
4. `enter_init`  
    1. 手动创建第一个进程，设置其 `stdin` `stdout` `stderr`  
    2. 调用 `execv` 读取 init 并载入
    3. 调用 `Process::run` 将其设置为当前进程  
    4. 调用 `Process::leaveKernelMode` ~~恢复其状态~~ 将其状态载入用户 tss  
        相应的，`Process::enterKernelMode` 就是保存状态到 `Process` 类  
    5. `ljmp` 用户 tss，发起任务切换  
5. 调度  
    基本在 `resche.cpp` 里，简单易懂，在 `pit.cpp` `syscall_handler.cpp` 还有 `yield_sche` 处理函数中被调用  
6. 杂项和提示
    - 实现虚拟内存的话，可以在 `Process::leaveKernelMode` `Process::enterKernelMode` 重写页表，或者多份页表直接赋值为 `usermode_state` 中的 `cr3`  
        `exec` 的时候由 `segment` 信息初始化页表  
        `fork` 系统调用也要做相应调整，由于不分页也不分段的话地址空间不能重复，所以我这里是备份栈顶 512 字节并且子进程在 `exec` 或者 `exit` 前父进程会一直睡着，防止污染栈空间。  
        - 因为子进程通常都会 `exec`  
        - 有分页，独立的地址空间以后父进程就没必要睡了，栈也没必要备份了  
            - 指直接拷贝一份就行 (包括原版没备份的代码和数据也要拷贝)  
            - 当然有闲心也可以实现 copy-on-write  
        - 如果你们的分页以 `segment` 或者 `section` 为单位并且用 `Object` 储存它们， `BeforeAttachedHook` 和 `onRemovedByOwner` 可能对你有帮助  
            - `BeforeAttachedHook` 能在 `fork` 的时候给子进程一份真正的副本而不是 `shared_ptr` 引用，并且此时你也可以改改页表  
            - `onRemovedByOwner` 能让你在段被解挂的时候有机会改改页表  
    - `BlockInputDeviceMixin` 如果有更好的写法请告诉我，我不知道我写了个什么玩意  
    - 关于运行平台:  
        - qemu:  
            一般是好文明，有阳间的调试功能，但它经常漏 feature，而且异常没啥提示  
        - bochs:  
            比 qemu 慢，支持的特性更多，阴间调试，阳间异常提示  
        - virtualbox 等虚拟机:  
            当你想挑战能否在实机上跑时先试试  
        - x86 开发板，带 JTAG 那种:  
            不知道阳不阳间  
        - 实机:  
            记得 CSM 启动  
