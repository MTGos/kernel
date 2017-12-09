#include <config.h>
#include <multiboot.h>
#ifndef ENABLE_FRAMEBUFFER
#include "../../../hw/pc/cgaterm/cgaterm.hpp"
#else
#include "../../../hw/pc/vesafb/vesafb.hpp"
#endif
#include "../../../hw/pc/8259/pic.hpp"
#include "../../../hw/pc/idt/idt.hpp"
#include "../../../hw/pc/pmm/pmm.hpp"

#include <base.hpp>
static multiboot_info_t *mb_info;
#ifndef ENABLE_FRAMEBUFFER
CGATerm term;
#else
VESAfb term(mb_info);
#endif
PMM_MB lpmm(mb_info);

#include<paging.hpp>
void main();
extern "C" void start(int eax, multiboot_info_t *ebx) {
    mb_info = ebx;
    context_enabled=false;
    main();
}
void drivers_init() {
    pmm=(PMM*)(&lpmm);
    setMainTTY(&term);
    --term;
    initIDT();
    PIC::initPIC(0x20, 0x28);
    asm volatile("sti");
}
