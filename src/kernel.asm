extern kernel_main
[BITS 16]

CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start

start_protected_mode:
    cli    
    mov ax, 0 
    mov ds, ax         
    lgdt [gdt_descriptor]
    mov eax, cr0     
    or al, 1
    mov cr0, eax
    jmp CODE_SEG:setup_segments

gdt_start:
    dd 0x0           ; Null descriptor
    dd 0x0           

; Descriptor de código
gdt_code:
    dw 0xFFFF        ; Limit
    dw 0x0000        ; Base (parte baixa)
    db 0x00          ; Base (parte média)
    db 10011010b     ; Access
    db 11001111b     ; Granularity
    db 0x00          ; Base (parte alta)

; Descriptor de dados
gdt_data:
    dw 0xFFFF
    dw 0x0000
    db 0x00  
    db 10010010b     
    db 11001111b     
    db 0x00        

gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

[bits 32]
global setup_segments
setup_segments:
    mov ax, DATA_SEG 
    mov ds, ax       
    mov es, ax       
    mov fs, ax       
    mov ss, ax       
    mov gs, ax       
    mov ebp, 0x9C00  
    mov esp, ebp     

    ; Ativa A20
    in al, 0x92      
    or al, 2         
    out 0x92, al
    call kernel_main
hang:
    jmp hang