#include <stdio.h>
#include <stdint.h>
#include <io_port.h>

#define CPUID_FEAT_EDX_APIC (1 << 9)
#define IA32_APIC_BASE_MSR 0x1B
#define IA32_APIC_BASE_MSR_BSP 0x100 // Processor is a BSP
#define IA32_APIC_BASE_MSR_ENABLE 0x800

void check_disable_apic()
{
    uint32_t eax, edx;
    cpuid(1, &eax, &edx);
    if (edx & CPUID_FEAT_EDX_APIC)
    {
        wrmsr(IA32_APIC_BASE_MSR, rdmsr(IA32_APIC_BASE_MSR) & ~IA32_APIC_BASE_MSR_ENABLE);
        printf("local apic found, disabled\n");
    }
}
