.code16

.global code16_source_start
.global code16_source_int10
.global code16_source_int13
.global real_mode_buffer
.global code16_source_end

code16_source_start:

code16_source_int10:
int $0x10
hlt
iret

code16_source_int13:
int $0x13
hlt
iret

real_mode_buffer:
.fill 1024, 1, 0

code16_source_end:
