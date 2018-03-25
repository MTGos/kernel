#include "uart.hpp"
auto delay = [](uint32_t count) {
    for(uint32_t x=0;x<count;x++);
};
UART &UART::getInstance() {
    static UART instance;
    return instance;
}
uint32_t &UART::operator[](uint32_t off) {
    return *((uint32_t*)(0x3F200000+off));
}
uint32_t &UART::operator[](UART_MEM off) {
    return (*this)[(uint32_t)off];
}
UART::UART() {
    auto &self = *this;
    self[UART_MEM::CR] = 0;
    self[UART_MEM::GPPUD] = 0;
    delay(150);
    self[UART_MEM::GPPUDCLK0] = (3 << 14);
    delay(150);
    self[UART_MEM::GPPUDCLK0] = 0;
    self[UART_MEM::ICR] = 0x7FF; //Clear interrupts
}
