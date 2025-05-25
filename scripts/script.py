import os
import subprocess
import shutil

BOOT_ASM = "../src/bootloader.asm"
KERNEL_ASM = "../src/kernel.asm"
ISR_STUBS_ASM = "../src/isr_stubs.asm"
BOOT_BIN = "../binary/boot.bin"
KERNEL_BIN = "../binary/kernel.bin"
IMAGE_FILE = "../binary/imagem.img"
IMAGE_SIZE = 1474560
QEMU_EXEC = "qemu-system-x86_64"

def run(command):
    result = subprocess.run(command, shell=True)
    if result.returncode != 0:
        print(f"Erro ao executar: {command}")
        exit(1)

def compile_sources():
    print("🛠️ Compilando bootloader...")
    run(f"nasm -f bin -o {BOOT_BIN} {BOOT_ASM}")

    print("🛠️ Montando kernel (asm + C)...")
    run(f"nasm -f elf32 -o ../binary/kernel.o {KERNEL_ASM}")
    print("🛠️ Montando stubs de interrupção...")
    run(f"nasm -f elf32 -o ../binary/isr_stubs.o {ISR_STUBS_ASM}")
    run(f"gcc -m32 -ffreestanding -c ../src/kernel.c -o ../binary/kernel_c.o")
    run(f"ld -m elf_i386 -T linker.ld -o ../binary/kernel.elf ../binary/kernel.o ../binary/kernel_c.o ../binary/isr_stubs.o") # Adicionado isr_stubs.o
    run(f"objcopy -O binary ../binary/kernel.elf {KERNEL_BIN}")



def create_image():
    print("💾 Criando imagem vazia de 1.44MB...")
    with open(IMAGE_FILE, "wb") as f:
        f.write(b"\x00" * IMAGE_SIZE)

def write_to_image():
    print("📥 Gravando bootloader no setor 0...")
    with open(IMAGE_FILE, "r+b") as img, open(BOOT_BIN, "rb") as boot:
        img.seek(0)
        img.write(boot.read())

    print("📥 Gravando kernel no setor 1 (LBA 1)...")
    with open(IMAGE_FILE, "r+b") as img, open(KERNEL_BIN, "rb") as kernel:
        img.seek(512 * 1)
        img.write(kernel.read())

def run_qemu():
    print("🚀 Iniciando QEMU...")
    run(f"{QEMU_EXEC} -drive format=raw,file={IMAGE_FILE}")

if __name__ == "__main__":
    compile_sources()
    create_image()
    write_to_image()
    run_qemu()
