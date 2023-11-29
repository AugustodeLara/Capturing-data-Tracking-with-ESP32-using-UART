#ifndef UART_CONFIG_H
#define UART_CONFIG_H

#include <termios.h>

class UARTConfig {
public:
    UARTConfig(const char* portName, speed_t baudRate);
    ~UARTConfig();

    bool openPort();
    bool configurePort();

    int getSerialPort() const {
        return serialPort;
    }

private:
    const char* portName;
    speed_t baudRate;
    int serialPort;
    struct termios tty;

    bool setBaudRate();
    bool applyConfig();
};

#endif // UART_CONFIG_H

