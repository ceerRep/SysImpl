.code16

.global code16_source_start
.global code16_source_int10
.global vbe_real_mode_buffer
.global code16_source_end

code16_source_start:

code16_source_int10:
int $0x10
hlt
iret

vbe_real_mode_buffer:
.fill 1024, 0

code16_source_end:
