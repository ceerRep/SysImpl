.code32

#define	MULTIBOOT_HEADER_MAGIC  0x1BADB002
#define	MULTIBOOT_HEADER_FLAGS  0x00010003
#define MULTIBOOT_SIGNATURE	    0x2BADB002	/* Multiboot signature verification	*/
#define MULTIBOOT_BOOTINFO_MMAP	0x00000040	/* mmap_length mmap_addr valid		*/

.data
aSysHalt:
	.asciz "Kernel returned, system halted\n"

.text
jmp start

.align 4
mbootheader:  				# Beginning of multiboot header, embedded in ELF header
	.long   MULTIBOOT_HEADER_MAGIC
	.long   MULTIBOOT_HEADER_FLAGS
	.long   -(MULTIBOOT_HEADER_MAGIC + MULTIBOOT_HEADER_FLAGS)
	.long   mbootheader 	# Begin a.out kludge: header address
	.long   text			# load address
   	.long   bss			    # load end address
	.long   ebss			# bss end address
	.long   start			# Entry address: end of multiboot header, end of a.out kludge

.globl start

start:
	movl $BOOT_STACK, %esp
	push %ebx
	push %eax
	call crt_init
	call _init
    call enter_main
	push $aSysHalt
	call puts
	cli
halt:
	hlt
	jmp halt
