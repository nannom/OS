[org 0]
[bits 16]

jmp 0x07c0:_start

_start:
    mov ah, 0x0e
    mov al, 'B'
    int 0x10

	mov ax, 0x1000
	mov es, ax
	mov bx, 0x0000

	mov ah, 0x02
	mov al, 2
	mov ch, 0
	mov dh, 0
	mov cl, 2
	mov dl, 0x80
	int 0x13
	jc error

	lgdt[gdtr]
	mov eax, cr0
	or eax, 1
	mov cr0, eax
	
	jmp $+2
	nop
	nop

	mov bx, DataSegment
	mov ds, bx
	mov es, bx
	mov fs, bx
	mov gs, bx
	mov ss, bx
	jmp dword CodeSegment:0x10000

error:
    mov ah, 0x0e
    mov al, 'E'
    int 0x10
    jmp $

gdtr:
dw gdt_end - gdt - 1
dd gdt

gdt:
	dd 0
	dd 0
	CodeSegment equ 0x08
	dd 0x0000FFFF, 0x00CF9A00 ; 코드 세그
	DataSegment equ 0x10
	dd 0x0000FFFF, 0x00CF9200 ; 데이터 세그
	VideoSegment equ 0x18
	dd 0x8000FFFF, 0x0040920B ; 비디오 세그

gdt_end:

times 510-($-$$) db 0
dw 0xAA55