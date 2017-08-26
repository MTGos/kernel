#include "../../../hw/3ds11/picafb/picafb.hpp"
#include "../../../hw/3ds11/vectorinit/vectorinit.hpp"
#include <base.hpp>
#include <config.h>
#include "../../../mmaps/3ds11.mh"
#include "../../../hw/pmm/pmm.hpp"

PICAfb term;
PMM_MMAP lpmm;

void main();
extern "C" void start() { main(); }
void drivers_init() {
    pmm=(PMM*)(&lpmm);
    setMainTTY(&term);
    --term;
    *((volatile uint32_t*)0x17E00600)=67108;
    *((volatile uint32_t*)0x17E00608)=3;
    asm volatile("CPSIE aif");
}
