[org 0x7c00]
[bits 16]

jmp short start

start:
	xor ax,ax
	mov ds, ax
	mov es, ax
	mov ss, ax
	mov sp, 0x7000

    mov ax, 0x1000
	mov es, ax
	mov bx, 0x0000

	mov ah, 0x02
	mov al, 41
	mov ch, 0
	mov dh, 0
	mov cl, 2
	mov dl, 0x80
	int 0x13
	
	mov ax, 0x2401
	int 0x15
	cli
	lgdt[gdt_descriptor]
	mov eax, cr0
	or eax, 1
	mov cr0, eax

	jmp $+2
	nop
	nop

	jmp 0x08:protected_mode

[bits 32]
protected_mode:
	mov ax, 0x10

	mov ds,ax
	mov es,ax
	mov fs,ax
	mov gs,ax
	mov ss,ax

	jmp 0x08:0x10000

gdt_start:
	gdt_null:
		dd 0
		dd 0
	gdt_code:
		dw 0xFFFF
		dw 0
		db 0
		db 0x9a
		db 0xcf
		db 0
	gdt_data:
		dw 0xffff
		dw 0
		db 0
		db 0x92
		db 0xcf
		db 0
gdt_end:

gdt_descriptor:
	dw gdt_end - gdt_start - 1
	dd gdt_start

times 510-($-$$) db 0
dw 0xAA55
