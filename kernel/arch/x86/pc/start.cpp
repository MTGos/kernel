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
#include <paging.h>

#include <base.hpp>
static multiboot_info_t *mb_info;
#ifndef ENABLE_FRAMEBUFFER
CGATerm term;
#else
VESAfb term(mb_info);
#endif
PMM_MB lpmm(mb_info);

paging_context_x86 main_context;
void main();
extern "C" void start(int eax, multiboot_info_t *ebx) {
    mb_info = ebx;
    main();
}
void drivers_init() {
    pmm=(PMM*)(&lpmm);
    setMainTTY(&term);
    --term;
    initIDT();
    main_context.switch_context();
    main_context.mmap((phys_t)mb_info, mb_info, protection::RW, false);
    phys_t start_fb = (phys_t)(mb_info->framebuffer_addr);
    phys_t end_fb = start_fb + mb_info->framebuffer_height * mb_info->framebuffer_pitch;
    for(phys_t i=start_fb; i<end_fb; i+=0x1000) {
        main_context.mmap(i, (void*)i, protection::RW, false);
    }
    PIC::initPIC(0x20, 0x28);
    asm volatile("sti");
}
