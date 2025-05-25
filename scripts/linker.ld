OUTPUT_FORMAT("elf32-i386")
ENTRY(start_protected_mode)

SECTIONS
{
  /* Antes de tudo, zera o valor da localização em memória */
  . = 0x600;           /* Offset dentro do setor 1 (512 bytes * 1 = 0x200),
                          mas como estamos no acostamento de 16 bit,
                          definimos 0x600 (o bootloader carrega para 0:0x600). */

  .text : {            /* Coloca todo o código (.text) aqui */
      *(.text)         /* Instruções do kernel.asm e do kernel.c */
  }

  .rodata : {          /* Dados somente‐leitura (strings) */
      *(.rodata)
  }

  .data : {            /* Dados iniciais (variáveis globais inicializadas) */
      *(.data)
  }

  .bss : {             /* Dados não-inicializados (zerados) */
      __bss_start = .;
      *(.bss)
      __bss_end = .;
  }
}
