#!/bin/bash

echo "Dispositivos disponíveis:"
lsblk -d -o NAME,SIZE,MODEL,TRAN | grep usb

devices=($(lsblk -d -o NAME,TRAN | grep usb | awk '{print $1}'))
count=${#devices[@]}

if [[ $count -eq 0 ]]; then
    echo "Nenhum dispositivo USB encontrado. Saindo..."
    exit 1
elif [[ $count -eq 1 ]]; then
    selected_device="/dev/${devices[0]}"
    echo "Apenas um dispositivo encontrado: $selected_device"
else
    echo "Mais de um dispositivo encontrado:"
    for i in "${!devices[@]}"; do
        dev="/dev/${devices[$i]}"
        size=$(lsblk -dno SIZE "$dev")
        model=$(lsblk -dno MODEL "$dev")
        echo "$((i + 1)). $dev - $size - $model"
    done

    read -rp "Escolha o dispositivo (1-${count}): " choice
    if [[ $choice -lt 1 || $choice -gt $count ]]; then
        echo "Escolha inválida. Saindo..."
        exit 1
    fi
    selected_device="/dev/${devices[$((choice - 1))]}"
fi

read -rp "Você selecionou $selected_device. Deseja continuar? (s/n): " confirm
if [[ $confirm != "s" && $confirm != "S" ]]; then
    echo "Operação cancelada pelo usuário."
    exit 1
fi

echo "Escrevendo bootloader em $selected_device..."
dd if=../binary/bootloader.bin of="$selected_device" bs=512

echo "Escrevendo kernel em $selected_device..."
dd if=../binary/kernel.bin of="$selected_device" bs=512 seek=1

echo "Processo concluído com sucesso!"
