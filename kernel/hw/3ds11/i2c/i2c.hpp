#pragma once
#include <config.h>
#include <stdint.h>
/**
 * I2C class for the 3DS
 */
class I2C {
    private:
    int currDev;
    int currReg;
    int busID;

    public:
    I2C();
    ~I2C();
    auto waitBusy() -> void;          ///< Waits for the currently selected device to finish
    auto getResult() -> bool;         ///< Returns true when the device sent a ACK
    auto stop() -> void;              ///< Stops the current transfer
    auto selectDev(int dev) -> bool;  ///< Selects current I2C device
    auto selectReg(int reg) -> bool;  ///< Selects current register
    auto write(uint8_t data) -> bool; ///< Sends 8 bits of data
    auto read() -> uint8_t;           ///< Receives 8 bits of data
};
