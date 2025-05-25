// Declarações de funções que estarão em um arquivo assembly (ex: isr_stubs.asm)
extern void idt_load(unsigned int idt_ptr_addr); // Carrega o ponteiro da IDT
extern void isr0();                              // Stub para IRQ0 (timer)

void print_terminal(const char *message);
void delay_simple(unsigned int ticks);

#define VGA_WIDTH 80
#define DELAY_TICKS 500000

#define PIT_CMD_PORT 0x43
#define PIT_CHANNEL0_PORT 0x40
// PIT_CHANNEL1_PORT 0x41
// PIT_CHANNEL2_PORT 0x42
#define PIT_BASE_FREQUENCY 1193182

// --- Estruturas e funções da IDT ---
struct idt_entry_struct
{
    unsigned short base_lo; // Os 16 bits inferiores do endereço do handler
    unsigned short sel;     // Seletor de segmento do Kernel (0x08)
    unsigned char always0;  // Deve ser sempre zero
    unsigned char flags;    // Flags (P, DPL, S, Type)
    unsigned short base_hi; // Os 16 bits superiores do endereço do handler
} __attribute__((packed));
typedef struct idt_entry_struct idt_entry_t;

struct idt_ptr_struct
{
    unsigned short limit;
    unsigned int base; // Endereço da primeira idt_entry_t
} __attribute__((packed));
typedef struct idt_ptr_struct idt_ptr_t;

idt_entry_t idt_entries[256];
idt_ptr_t idt_p;

// --- Constantes do PIC ---
#define PIC1_COMMAND 0x20
#define PIC1_DATA 0x21
#define PIC2_COMMAND 0xA0
#define PIC2_DATA 0xA1
#define PIC_EOI 0x20 // Comando End-Of-Interrupt

#define ICW1_ICW4 0x01      // ICW4 (not) needed
#define ICW1_SINGLE 0x02    // Single (cascade) mode
#define ICW1_INTERVAL4 0x04 // Call address interval 4 (8)
#define ICW1_LEVEL 0x08     // Level triggered (edge) mode
#define ICW1_INIT 0x10      // Initialization - required!

#define ICW4_8086 0x01       // 8086/88 (MCS-80/85) mode
#define ICW4_AUTO 0x02       // Auto (normal) EOI
#define ICW4_BUF_SLAVE 0x08  // Buffered mode/slave
#define ICW4_BUF_MASTER 0x0C // Buffered mode/master
#define ICW4_SFNM 0x10       // Special fully nested (not)

// Variável global para os ticks do timer
volatile unsigned int timer_ticks = 0;

static inline void outb(unsigned short port, unsigned char val)
{
    asm volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline unsigned char inb(unsigned short port)
{
    unsigned char ret;
    asm volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

void idt_set_gate(unsigned char num, unsigned long base, unsigned short sel, unsigned char flags)
{
    idt_entries[num].base_lo = (base & 0xFFFF);
    idt_entries[num].base_hi = (base >> 16) & 0xFFFF;
    idt_entries[num].sel = sel;
    idt_entries[num].always0 = 0;
    idt_entries[num].flags = flags /* | 0x60 */; // Adiciona DPL se precisar
}

void idt_install()
{
    idt_p.limit = (sizeof(idt_entry_t) * 256) - 1;
    idt_p.base = (unsigned int)&idt_entries;

    // Limpa a IDT (preenche com zeros) - opcional, mas boa pratica
    char *idt_ptr_char = (char *)&idt_entries;
    for (int i = 0; i < sizeof(idt_entry_t) * 256; i++)
    {
        idt_ptr_char[i] = 0;
    }

    // Configura o gate para a interrupção do timer (IRQ0 -> INT 32 após remapeamento do PIC)
    // 0x08 é o seletor de segmento de código do kernel
    // 0x8E indica um interrupt gate presente, DPL 0 (nível do kernel)
    idt_set_gate(32, (unsigned long)isr0, 0x08, 0x8E);
    // Adicione outras ISRs aqui (ex: idt_set_gate(33, (unsigned long)isr1, 0x08, 0x8E);)

    idt_load((unsigned int)&idt_p); // Chama a função assembly para carregar a IDT
}

void pic_send_eoi(unsigned char irq)
{
    if (irq >= 8)
        outb(PIC2_COMMAND, PIC_EOI);
    outb(PIC1_COMMAND, PIC_EOI);
}

void pic_remap(int offset1, int offset2)
{
    unsigned char a1, a2;

    a1 = inb(PIC1_DATA); // Salva máscaras
    a2 = inb(PIC2_DATA);

    outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4); // Inicia a sequência de inicialização (modo cascata)
    outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);

    outb(PIC1_DATA, offset1); // ICW2: Master PIC vector offset
    outb(PIC2_DATA, offset2); // ICW2: Slave PIC vector offset

    outb(PIC1_DATA, 4); // ICW3: Diz ao Master PIC que há um slave PIC em IRQ2 (0000 0100)
    outb(PIC2_DATA, 2); // ICW3: Diz ao Slave PIC seu ID de cascata (0000 0010)

    outb(PIC1_DATA, ICW4_8086);
    outb(PIC2_DATA, ICW4_8086);

    outb(PIC1_DATA, a1); // Restaura máscaras salvas
    outb(PIC2_DATA, a2);
}

// Manipulador de interrupção do Timer (chamado pelo stub assembly isr0)
void timer_handler()
{
    timer_ticks++;

    // Teste visual: A cada 100 ticks (aproximadamente 1 segundo se o timer for 100Hz (ele é))
    // muda um caractere no canto superior direito.
    if (timer_ticks % 100 == 0)
    {
        char *video_memory = (char *)0xB8000;
        // Posição: linha 0, coluna 79 (última coluna)
        // O caractere alterna entre '.' e '*'
        if ((timer_ticks / 100) % 2 == 0)
        {
            video_memory[(VGA_WIDTH - 1) * 2] = '.';
        }
        else
        {
            video_memory[(VGA_WIDTH - 1) * 2] = '*';
        }
        video_memory[(VGA_WIDTH - 1) * 2 + 1] = 0x0F; // Atributo: Branco sobre Preto
    }

    pic_send_eoi(0); // Envia EOI para IRQ0
}

void timer_phase(unsigned int frequency)
{ // Renomeado de init_pit
    unsigned int divisor = PIT_BASE_FREQUENCY / frequency;
    outb(PIT_CMD_PORT, 0x34); // Canal 0, LSB/MSB, modo 2 (rate generator), binário
    outb(PIT_CHANNEL0_PORT, (unsigned char)(divisor & 0xFF));
    outb(PIT_CHANNEL0_PORT, (unsigned char)((divisor >> 8) & 0xFF));
}

void timer_install()
{
    timer_phase(100); // Configura o PIT para 100Hz
    // Habilita IRQ0 no PIC (desmascara)
    // A máscara inicial geralmente é 0xFF (tudo mascarado)
    // Para desmascarar IRQ0, precisamos setar o bit 0 para 0.
    // Ex: inb(PIC1_DATA) & ~0x01
    outb(PIC1_DATA, inb(PIC1_DATA) & 0xFE); // Desmascara IRQ0 (bit 0)
}

void kernel_main()
{
    idt_install();         // Configura a IDT
    pic_remap(0x20, 0x28); // Remapeia PIC: IRQs 0-7 para INT 32-39, IRQs 8-15 para INT 40-47
    timer_install();       // Configura o PIT e desmascara IRQ0

    // Habilita interrupções! (Esta é uma instrução assembly)
    // Sem isso, o timer_handler nunca será chamado.
    asm volatile("sti");

    const char *message = "Kernel C: IDT, PIC, Timer Configurados.\nInterrupcoes Habilitadas.\n";
    print_terminal(message); // Esta função tem um loop infinito

    while (1)
    {
        // O kernel pode fazer outras coisas aqui,
        // e o timer_handler será chamado em segundo plano.
    }
}

void print_terminal(const char *message)
{
    char *video_base = (char *)0xB8000;
    char attribute = 0x14;

    while (1)
    {
        char *current_video_ptr = video_base;
        int current_col_on_screen = 0;
        int msg_char_index = 0;

        // --- Escreve a mensagem ---
        while (message[msg_char_index] != '\0')
        {
            if (message[msg_char_index] == '\n')
            {
                int bytes_to_next_line_start = (VGA_WIDTH - current_col_on_screen) * 2;
                current_video_ptr += bytes_to_next_line_start;
                current_col_on_screen = 0;
            }
            else
            {
                current_video_ptr[0] = message[msg_char_index];
                current_video_ptr[1] = attribute;
                current_video_ptr += 2;
                current_col_on_screen++;
                if (current_col_on_screen >= VGA_WIDTH)
                {
                    current_col_on_screen = 0;
                    // Não avança para nova linha automaticamente aqui, \n deve tratar
                }
            }
            msg_char_index++;
        }
    }
}
