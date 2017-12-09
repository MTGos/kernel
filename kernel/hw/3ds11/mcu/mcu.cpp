#include "mcu.hpp"
namespace MCU {
I2C access;
uint8_t HIDStatus;
static auto read(int reg) -> unsigned char {
    access.selectDev(3);
    access.selectReg(reg);
    return access.read();
}
static auto write(int reg, uint8_t value) -> void {
    access.selectDev(3);
    access.selectReg(reg);
    access.write(value);
}
auto getVersionLo() -> unsigned char { return read(0); }
auto getVersionHi() -> unsigned char { return read(1); }
auto getLCDFlickerTop() -> unsigned char { return read(3); }
auto setLCDFlickerTop(unsigned char val) -> void { write(3, val); }
auto getLCDFlickerBottom() -> unsigned char { return read(4); }
auto setLCDFlickerBottom(unsigned char val) -> void { write(4, val); }
auto get3DSlider() -> unsigned char { return read(8); }
auto getVolume() -> unsigned char { return read(9); }
auto getBatteryPercent() -> unsigned char { return read(11); }
auto getSystemVoltage() -> unsigned char { return read(13); }
auto isCharging() -> bool { return read(15) & 0x10; }
auto isOpen() -> bool { return read(15) & 0x2; }
auto updateHIDStatus() -> void { HIDStatus = read(16); }
auto powerButtonPressed() -> bool { return HIDStatus & 0x1; }
auto powerButtonLongPressed() -> bool { return HIDStatus & 0x2; }
auto homeButtonPressed() -> bool { return HIDStatus & 0x4; }
auto homeButtonReleased() -> bool { return HIDStatus & 0x8; }
auto wifiEnabled() -> bool { return HIDStatus & 0x10; }
auto closed() -> bool { return HIDStatus & 0x20; }
auto opened() -> bool { return HIDStatus & 0x40; }
auto poweroff() -> void { write(32, 1); }
auto reboot() -> void { write(32, 4); }
auto enableTopLCD() -> void { write(34, 0b100010); }
auto disableTopLCD() -> void { write(34, 0b010010); }
auto enableBottomLCD() -> void { write(34, 0b1010); }
auto disableBottomLCD() -> void { write(34, 0b0110); }
auto setWifiLED(char val) -> void { write(0x2A, (unsigned char)val); }
auto setCameraLED(char val) -> void { write(0x2B, (unsigned char)val); }
auto set3DLED(char val) -> void { write(0x2C, (unsigned char)val); }
auto getRTC(RTC_date *date) -> void {
    auto bcdToVal = [](unsigned char c) -> unsigned char { return (c & 0xF) + (10 * (c >> 4)); };
    date->seconds = bcdToVal(read(0x30));
    date->minutes = bcdToVal(read(0x31));
    date->hours = bcdToVal(read(0x32));
    date->days = bcdToVal(read(0x34));
    date->months = bcdToVal(read(0x35));
    date->years = bcdToVal(read(0x36));
}
auto setRTC(RTC_date *date) -> void {
    auto valToBCD = [](unsigned char c) -> unsigned char { return (c % 10) + ((c / 10) << 4); };
    write(0x30, valToBCD(date->seconds));
    write(0x31, valToBCD(date->minutes));
    write(0x32, valToBCD(date->hours));
    write(0x34, valToBCD(date->days));
    write(0x35, valToBCD(date->months));
    write(0x36, valToBCD(date->years));
}
}
