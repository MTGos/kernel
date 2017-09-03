#include "pmm.hpp"
PMM_MB::PMM_MB(multiboot_info_t *mb_info): PMM(0x1000), mb_info(mb_info) {
    auto mmap = (multiboot_memory_map_t*)((uintptr_t)(mb_info->mmap_addr));
    uint32_t count = mb_info->mmap_length / sizeof(mmap[0]);
    for(uint32_t i=0;i<count;i++) {
        if(mmap[i].type != MULTIBOOT_MEMORY_AVAILABLE)
            continue;
        if((phys_t)mmap[i].addr < lowest_page)
            lowest_page = (phys_t)mmap[i].addr;
        if((phys_t)(mmap[i].addr + mmap[i].len) > highest_page)
            highest_page = (phys_t)(mmap[i].addr + mmap[i].len);
    }
    fill();
}

PMM_MB::~PMM_MB() {}

auto PMM_MB::isFree(phys_t addr) -> bool {
    auto mmap = (multiboot_memory_map_t*)((uintptr_t)(mb_info->mmap_addr));
    bool free=false;
    uint32_t count = mb_info->mmap_length / sizeof(mmap[0]);
    for(uint32_t i=0;i<count;i++) {
        if((mmap[i].addr > addr) || (mmap[i].addr + mmap[i].len < addr))
            continue;
        if(mmap[i].type != MULTIBOOT_MEMORY_AVAILABLE)
            return false;
        free=true;
    }
    if(!free)
        return false;
    if((addr >= (phys_t)((uintptr_t)mb_info)) && (addr < (phys_t)((uintptr_t)mb_info)+0x1000))
        return false;
    return PMM::isFree(addr);
}
