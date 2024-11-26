[BITS 16]
[ORG 0]
;mov ax, 0x4F02
;mov bx, 0x0115
;int 0x10
jmp osMain
    
tesle db "Laa",0
backWidth db 0
backHeight db 0
pagination db 0
welcome db "Seja Bem Vindo ao MiniDriver Os",0

osMain:
    call configSegment
    call configStack
    call setTextVideoMode
    jmp showString

showString:
    mov dh, 3
    mov dl, 2
    call moveCursor
    mov si, welcome
    call printString
    jmp END

configSegment:
    mov ax, es
    mov ds, ax
    ret
configStack:
    mov ax, 7D00h
    mov ss, ax
    mov sp, 03FEh
    ret
setTextVideoMode:
    mov ah, 00h
    mov al, 03h
    int 10h
    mov BYTE[backHeight], 20
    mov BYTE[backWidth], 80
    ret

printString:
    mov ah, 09h
    mov bh, [pagination]
    mov bl, 40
    mov cx, 1
    mov al, [si]
    print:
        int 10h
        inc si
        call moveCursor
        mov ah, 09h
        mov al, [si]
        cmp al, 0
        jne print
    ret

moveCursor:
    mov ah, 02h
    mov bh, [pagination]
    inc dl
    int 10h
    ret

END:
    jmp $