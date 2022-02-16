#ifndef _8086_H

#define _8086_H

#ifdef __cplusplus
extern "C"
{
#endif

    extern char code16_source_start[0];
    extern char code16_source_int9[0]; // IRQ1 Keyboard
    extern char code16_source_int10[0];
    extern char code16_source_int13[0];
    extern char code16_source_int16[0];
    extern char real_mode_buffer[0];

#ifdef __cplusplus
}
#endif

#endif
