;No MBR os primeiros 446 bytes são usados para carregar o os ou seja 
;é o tamanho "maximo" que um bootloader pode ter, os proximos 64 bytes
;são para a tabela de partições do disco e os ultimos 2 bytes
;para assinatura que é usada para validar o mbr

;ax é um registrador de 16 bits dividido em 2 partes, ah e al sendo respectivamente mais significativa e menos significativa dos 16 bits
;e cada uma sendo de 8 bits é claro

;O endereço é o segmento X 16 (pois estamos no modo real de 16bits)
;0800h é 2048 em hexadecimal multiplicado por 16 temos um número de bits
;32768 e seu deslocamente é 0 então a kernel é carregada no endereço
;32768, bx é deslocamento e es é o segmento

[BITS 16]
[ORG 7C00H]
call disableSeg
call loadKernel
jmp 0:0x600


disableSeg:
    mov ax, 0      ; Define o segmento base como 0
    mov ds, ax     ; Data Segment
    mov es, ax     ; Extra Segment
    mov ss, ax     ; Stack Segment
    mov fs, ax     ; Additional Segment
    mov gs, ax     ; Additional Segment
    ret

loadKernel:
    mov ah, 2h ; função para ler setores
    mov al, 18 ; irá ler só 1 setor
    mov ch, 0 ; usa o primeiro cilindro
    mov cl, 2 ; o setor que será lido é o setor 2
    mov dh, 0 ; primeiro cabeçote
    mov bx, 0 ; endereço de load dos dados
    mov es, bx ; configura o es para o endereço dos dados
    mov bx, 0x600 ; configura para 0
    int 13h ; ler o disco
    jc error_occurred ; verifica um erro
    ret

veio db "Paa",0

teste3:
    mov si, veio
    mov ah, 0eh
    mov al, [si]
    int 10h
    ret

END:
    jmp $

error_occurred:
    call teste3

times 510 - ($-$$) db 0
dw 0xAA55