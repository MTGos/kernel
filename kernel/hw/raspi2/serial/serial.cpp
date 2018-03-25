#include "serial.hpp"
Serial::Serial(): TTY(80,25), uart(UART::getInstance()) {
    uart[UART_MEM::IBRD] = 1;
    uart[UART_MEM::FBRD] = 40;
    uart[UART_MEM::LCRH] = 0b1110000;
    uart[UART_MEM::IMSC] = 0b11111110010;
    uart[UART_MEM::CR]   = 0b1100000001;
}
auto Serial::serial_putc(char c) -> void {
    while(uart[UART_MEM::FR] & 0x20);
    uart[UART_MEM::DR] = (uint8_t)c;
}
auto Serial::serial_putc(int c) -> void {
    //UTF-8 encoder
    if(c <= 0x7F)
        return this->serial_putc((char)c);
    if(c <= 0x7FF) {
        this->serial_putc((char)(0b11000000 | (c >> 6)));
        this->serial_putc((char)(0b10000000 | (c & 0x3F)));
        return;
    }
    if(c <= 0xFFFF) {
        this->serial_putc((char)(0b11100000 | (c >> 12)));
        this->serial_putc((char)(0b10000000 | ((c >> 6)&0x3F)));
        this->serial_putc((char)(0b10000000 | (c&0x3F)));
        return;
    }
    this->serial_putc((char)(0b11110000 | (c >> 18)));
    this->serial_putc((char)(0b10000000 | ((c >> 12)&0x3F)));
    this->serial_putc((char)(0b10000000 | ((c >> 6)&0x3F)));
    this->serial_putc((char)(0b10000000 | (c&0x3F)));
}
auto Serial::serial_puti(int i) -> void {
    char buf[65];
    char *ptr=buf+63;
    ptr[1]=0;
    const char *chars = "0123456789";
    do {
        *(ptr--) = chars[i%10];
        i/=10;
    } while (i && (ptr != buf));
    ptr++;
    while(*ptr)
        this->serial_putc(*(ptr++));
}
auto Serial::plotChar(int x, int y, int c) -> void {
    //Send position
    serial_putc(0x1B);
    serial_putc('[');
    serial_puti(y+1);
    serial_putc(';');
    serial_puti(x+1);
    serial_putc('H');
    //Send color
    int r = rgbColor >> 16;
    int g = (rgbColor >> 8) & 0xFF;
    int b = rgbColor & 0xFF;
    serial_putc(0x1B);
    serial_putc('[');
    serial_puti(38);
    serial_putc(';');
    serial_puti(2);
    serial_putc(';');
    serial_puti(r);
    serial_putc(';');
    serial_puti(g);
    serial_putc(';');
    serial_puti(b);
    serial_putc('m');
    //Actually write the character
    serial_putc(c);
}
