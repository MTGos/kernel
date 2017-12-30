#include <paging.h>
#include <base.hpp>
extern int kernel_start;
extern int kernel_end;

const pagemap_tbl *curr_pagemaptbl = pagemap_tbl *(0x8000000000);
const pagemap_tbl *ref_pagemaptbl = pagemap_tbl *(0x8000001000);
const pagemap *curr_pagemap = pagemap *(0x8000200000);
const pagemap *ref_pagemap = pagemap *(0x8000400000);
const pagedir *curr_pagedir = pagedir *(0x8040000000);
const pagedir *ref_pagedir = pagedir *(0x8080000000);
const pagetbl *curr_pagetbl = pagetbl *(0xC000000000);
const pagetbl *ref_pagetbl = pagetbl *(0x10000000000);

inline uint64_t pagemaptbl_index(phys_t addr) {
    return addr >> 39;
}
inline uint64_t pagemap_index(phys_t addr) {
    return addr >> 30;
}
inline uint64_t pagedir_index(phys_t addr) {
    return addr >> 21;
}
inline uint64_t pagetbl_index(phys_t addr) {
    return addr >> 12;
}

inline phys_t wrap(uint64_t x) {
    if(x >= 0x800000000000ull)
        return x | 0xFFFF000000000000ull;
    else
        return x;
}

inline phys_t make_phys(uint64_t i, uint64_t j, uint64_t k, uint64_t l) {
    return (i << 39) | (j << 30) | (k << 21) | (l << 12);
}

inline void* make_virt(uint64_t i, uint64_t j, uint64_t k, uint64_t l) {
    return (void*)wrap(make_phys(i,j,k,l));
}

pagemap_tbl kernel[256];

paging_context_x86::paging_context_x86(): paging_context() {
    phys_t pagedir_addr;
    *pmm >> pagedir_addr;
    if(context_enabled) {
        current_context->mmap(pagedir_addr, ref_pagemaptbl, protection::RW, false);
        for(int i = 0;i < 256; i++)
            ref_pagemaptbl[i] = curr_pagemaptbl[i];
        for(int i = 256; i < 512; i++)
            ref_pagemaptbl[i].active = false;
        ref_pagemaptbl[1].active = false;
        ref_pagemaptbl[2].active = false;
        ref_pagemaptbl[3].active = false;

        phys_t pagemaptbl_map;
        *pmm >> pagemaptbl_map;
        ref_pagemaptbl[1].pagemap = pagemaptbl_map >> 12;
        ref_pagemaptbl[1].writeable=true;
        ref_pagemaptbl[1].unprivileged=false;
        ref_pagemaptbl[1].no_write_cache=true;
        ref_pagemaptbl[1].no_read_cache=false;
        ref_pagemaptbl[1].sbz = 0;
        ref_pagemaptbl[1].executable = false;
        ref_pagemaptbl[1].active = true;

        phys_t selfmap;
        *pmm >> selfmap;
        ref_pagemaptbl[2].pagemap = selfmap >> 12;
        ref_pagemaptbl[2].writeable=true;
        ref_pagemaptbl[2].unprivileged=false;
        ref_pagemaptbl[2].no_write_cache=true;
        ref_pagemaptbl[2].no_read_cache=false;
        ref_pagemaptbl[2].sbz = 0;
        ref_pagemaptbl[2].executable = false;
        ref_pagemaptbl[2].active = true;

        current_context->map_pagetable(this);

        //remap everything
        for(uint64_t i = 0; i < 256; i++) {
            if(!ref_pagemaptbl[i].active)
                continue;
            if(i > 0 && i < 4)
                continue;
            uint64_t pagemap_start = pagemap_index(make_phys(i,0,0,0));
            for(uint64_t j = pagemap_start; j<pagemap_start+512;j++) {
                if(!ref_pagemap[j].active)
                    continue;
                uint64_t pagedir_start = pagedir_index(make_phys(i,j,0,0));
                for(uint64_t k = pagedir_start; k<pagedir_start+512;k++) {
                    if(!ref_pagedir[k].active)
                        continue;
                    uint64_t pagetbl_start = pagetbl_index(make_phys(i,j,k,0));
                    for(uint64_t l = pagetbl_start; l < pagetbl_start+512; l++) {
                        if(!ref_pagetbl[l].active)
                            continue;
                        mmap(make_phys(ref_pagetbl[l].page<<12, curr_pagetbl+l, protection::RW, false);
                    }
                }
            }
        }
        //TODO: find an architecture with so many layers of paging that i run out of letters
        return;
    }
    //Map the entire kernel
    pagemap_tbl *pagemaptbl = (pagedir *)pagedir_addr;
    for(int i=0;i<512;i++)
        pagemaptbl[i].active=false;
    phys_t kernelpagemap;
    *pmm >> kernelpagemap;
    pagemaptbl[0].pagemap = kernelpagemap >> 12;
    pagemaptbl[0].writeable=true;
    pagemaptbl[0].unprivileged=false;
    pagemaptbl[0].no_write_cache=false;
    pagemaptbl[0].no_read_cache=false;
    pagemaptbl[0].sbz = 0;
    pagemaptbl[0].executable = false;
    pagemaptbl[0].active = true;

    pagemap *kpagemap = (pagemap*)kernelpagemap;
    for(int i=0;i<512;i++)
        kpagemap[i].active=false;
    phys_t kernelpagedir;
    *pmm >> kernelpagedir;
    kpagemap[0].pagedir = kernelpagedir >> 12;
    kpagemap[0].writeable=true;
    kpagemap[0].unprivileged=false;
    kpagemap[0].no_write_cache=false;
    kpagemap[0].no_read_cache=false;
    kpagemap[0].sbz = 0;
    kpagemap[0]._1G_page=false;
    kpagemap[0].global=true;
    kpagemap[0].executable=false;
    kpagemap[0].active=true;

    pagedir *kpagedir = (pagedir*)kernelpagedir;
    for(int i=0;i<512;i++)
        kpagedir[i].active=false;
    phys_t kernelpagetbl;
    *pmm >> kernelpagetbl;
    kpagedir[1].pagetbl = kernelpagetbl >> 12;
    kpagedir[1].writeable=true;
    kpagedir[1].unprivileged=false;
    kpagedir[1].no_write_cache=false;
    kpagedir[1].no_read_cache=false;
    kpagedir[1].sbz=0;
    kpagedir[1]._2M_page=false;
    kpagedir[1].global=true;
    kpagedir[1].executable=false;
    kpagedir[1].active=true;

    pagetbl *kpagetbl = (pagetbl*)kernelpagetbl;
    for(int i=0;i<512;i++)
        kpagetbl[i].active=false;
    for(phys_t i=(((phys_t)&kernel_start)>>12)-512,p=(phys_t)&kernel_start;p<(phys_t)&kernel_end;p+=0x1000;i++) {
        kpagetbl[i].page=i;
        kpagetbl[i].writeable=true;
        kpagetbl[i].unprivileged=false;
        kpagetbl[i].no_write_cache=false;
        kpagetbl[i].no_read_cache=false;
        kpagetbl[i].sbz=false;
        kpagetbl[i].pat=false;
        kpagetbl[i].global=true;
        kpagetbl[i].executable=true;
        kpagetbl[i].active=true;
    }

}
