#include "i2c.hpp"
struct I2CBus {
    volatile uint8_t data;
    volatile uint8_t ctl;
    uint16_t cntex;
    uint16_t scl;
} __attribute__((packed));
I2CBus *buses[] = {(I2CBus *)0x10161000, (I2CBus *)0x10144000, (I2CBus *)0x10148000};
I2C::I2C() {}
I2C::~I2C() {}
auto I2C::waitBusy() -> void {
    while (buses[busID]->ctl & 0x80)
        ;
}
auto I2C::getResult() -> bool {
    waitBusy();
    return buses[busID]->ctl & 0x10;
}
auto I2C::stop() -> void {
    buses[busID]->ctl = 0xC0;
    waitBusy();
    buses[busID]->ctl = 0xC5;
}
auto I2C::selectDev(int dev) -> bool {
    static int buss[] = {1, 1, 1, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 2, 1, 3};
    static int addr[] = {0x4a, 0x7a, 0x78, 0x4a, 0x78, 0x2C, 0x2E, 0x40, 0x44,
                         0xA6, 0xD0, 0xD2, 0xA4, 0x9A, 0xA0, 0xEE, 0x40, 0x54};
    currDev = dev;
    busID = buss[dev];
    waitBusy();
    buses[busID]->data = addr[dev];
    buses[busID]->ctl = 0xC2;
    return getResult();
}
auto I2C::selectReg(int reg) -> bool {
#ifdef PROTECT_MCU
    if ((currReg == 3) && (reg == 5)) return false;
#endif
    currReg = reg;
    waitBusy();
    buses[busID]->data = (uint8_t)reg;
    buses[busID]->ctl = 0xC0;
    return getResult();
}
auto I2C::write(uint8_t data) -> bool {
    for (int i = 0; i < 10; i++) {
        waitBusy();
        buses[busID]->data = data;
        buses[busID]->ctl = 0xC1;
        if (getResult()) { return true; }
        stop();
        selectDev(currDev);
        selectReg(currReg);
    }
    return false;
}
auto I2C::read() -> uint8_t {
    waitBusy();
    buses[busID]->ctl = 0xE1;
    waitBusy();
    uint8_t data = buses[busID]->data;
    buses[busID]->ctl |= 0x10;
    getResult();
    stop();
    return data;
}
