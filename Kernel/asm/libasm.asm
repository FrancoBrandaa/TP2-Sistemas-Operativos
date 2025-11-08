GLOBAL cpuVendor
GLOBAL getKeyboardBuffer

GLOBAL getSecond
GLOBAL getMinute
GLOBAL getHour

GLOBAL getRegisterSnapshot

GLOBAL acquire_spinlock

GLOBAL release_spinlock

EXTERN register_snapshot
EXTERN register_snapshot_taken

section .text

acquire_spinlock:
	mov rax, 0
	mov al, 1
	xchg al, [rdi]
	cmp al, 0
	jne acquire_spinlock
	ret

release_spinlock:
	mov byte [rdi], 0
    ret


getKeyboardBuffer:
	push rbp
	mov rbp, rsp

	in al, 60h

	mov rsp, rbp
	pop rbp

	ret


getSecond:
	push rbp
	mov rbp, rsp

	mov al, 0
	out 70h, al
	in al, 71h

	mov rsp, rbp
	pop rbp

	ret


getMinute:
	push rbp
	mov rbp, rsp

	mov al, 2
	out 70h, al
	in al, 71h

	mov rsp, rbp
	pop rbp

	ret


getHour:
	push rbp
	mov rbp, rsp

	mov al, 4
	out 70h, al
	in al, 71h

	mov rsp, rbp
	pop rbp

	ret


