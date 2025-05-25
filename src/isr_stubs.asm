global idt_load
idt_load:
    mov eax, [esp+4]  ; Pega o ponteiro para idt_ptr_t
    lidt [eax]        ; Carrega o registro IDTR
    ret

global isr0
extern timer_handler ; Declara a função C

isr0:
    pusha            ; Salva todos os registradores de propósito geral
    
    call timer_handler ; Chama o manipulador C

    popa             ; Restaura os registradores
    iret             ; Retorno da interrupção