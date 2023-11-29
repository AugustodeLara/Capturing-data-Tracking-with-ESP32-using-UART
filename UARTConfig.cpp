#include "UARTConfig.h"
#include <iostream>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>

UARTConfig::UARTConfig(const char* port, speed_t rate)
    : portName(port), baudRate(rate), serialPort(-1) {
    memset(&tty, 0, sizeof tty);
}

UARTConfig::~UARTConfig() {
    if (serialPort != -1) {
        close(serialPort);
    }
}

bool UARTConfig::openPort() {
    serialPort = open(portName, O_RDWR);

    if (serialPort == -1) {
        std::cerr << "Erro ao abrir a porta serial." << std::endl;
        return false;
    }

    return true;
}

bool UARTConfig::setBaudRate() {
    if (cfsetispeed(&tty, baudRate) != 0 || cfsetospeed(&tty, baudRate) != 0) {
        std::cerr << "Erro ao configurar a velocidade da porta serial." << std::endl;
        return false;
    }

    return true;
}

bool UARTConfig::configurePort() {
    if (tcgetattr(serialPort, &tty) != 0) {
        std::cerr << "Erro ao obter os atributos da porta serial." << std::endl;
        return false;
    }

    // Configuração da porta serial...
    tty.c_cflag &= ~PARENB;
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;
    tty.c_cflag &= ~CRTSCTS;

    tty.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

    tty.c_iflag &= ~(IXON | IXOFF | IXANY);
    tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL);

    tty.c_oflag &= ~OPOST;

    tty.c_cc[VMIN] = 1;
    tty.c_cc[VTIME] = 0;

    if (!setBaudRate() || !applyConfig()) {
        return false;
    }

    return true;
}

bool UARTConfig::applyConfig() {
    if (tcsetattr(serialPort, TCSANOW, &tty) != 0) {
        std::cerr << "Erro ao aplicar os atributos da porta serial." << std::endl;
        return false;
    }

    return true;
}

