.code16

.global code16_source_start
.global code16_source_int9
.global code16_source_int10
.global code16_source_int13
.global code16_source_int16
.global real_mode_buffer
.global code16_source_end

code16_source_start:

code16_source_int9:
int $0x9
hlt
iret

code16_source_int10:
int $0x10
hlt
iret

code16_source_int13:
int $0x13
hlt
iret

code16_source_int16:
int $0x16
hlt
iret

real_mode_buffer:
.fill 1024, 1, 0

code16_source_end:
