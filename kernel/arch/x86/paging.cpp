#include <paging.h>
#include <base.hpp>
extern int kernel_start;
extern int kernel_end;
paging_context_x86::paging_context_x86(): paging_context() {
    *pmm >> pagedir_addr;
    if(context_enabled) {
        //Copy System section (first 256 entries) over to the new context
        current_context->mmap(pagedir_addr, (void*)0x401000, protection::RW, false);
        pagedir *src=(pagedir *)0x400000;
        pagedir *dest=(pagedir *)0x401000;
        for(int i=0;i<256;i++)
            dest[i]=src[i];
        for(int i=256;i<1024;i++)
            dest[i].active=false;
        //remap self-map correctly
        dest[1].active=false;
        dest[2].active=false;
        dest[3].active=false;
        phys_t selfmap_meta;
        *pmm >> selfmap_meta;

        dest[1].pagetable = selfmap_meta >> 12;
        dest[1].writeable = true;
        dest[1].privileged = false;
        dest[1].no_write_cache = true;
        dest[1].no_read_cache = false;
        dest[1].size = false;
        dest[1].active = true;

        phys_t selfmap;
        *pmm >> selfmap;
        dest[2].pagetable = selfmap >> 12;
        dest[2].writeable = true;
        dest[2].privileged = false;
        dest[2].no_write_cache = true;
        dest[2].no_read_cache = false;
        dest[2].size = false;
        dest[2].active = true;

        current_context->map_pagetable(this);

        //Remap all parts of the system
        for(int i=0;i<256;i++) {
            if(!src[i].active)
                continue;
            if(i == 1 || i == 2 || i == 3)
                continue;
            pagetbl *pagetbl_start = (pagetbl *)(0x800000 + (i << 22));
            for(int j=0;j<1024;j++) {
                if(!pagetbl_start[j].active)
                    continue;
                mmap(pagetbl_start[j].page << 12, pagetbl_start+1024, protection::RW, false);

            }
        }
        return;
    }
    //Map the entire kernel
    pagedir *context=(pagedir *)pagedir_addr;
    for(int i=0;i<1024;i++)
        context[i].active=false;
    phys_t kernel_tbl;
    *pmm >> kernel_tbl;
    context[0].pagetable=kernel_tbl >> 12;
    context[0].writeable=true;
    context[0].privileged=false;
    context[0].no_write_cache=false;
    context[0].no_read_cache=false;
    context[0].size=false;
    context[0].active=true;

    pagetbl *kerneltbl = (pagetbl *)kernel_tbl;
    for(int i=0;i<1024;i++)
        kerneltbl[i].active=false;
    for(phys_t i=((phys_t)&kernel_start)>>12,p = (phys_t)&kernel_start; p < (phys_t)&kernel_end; p+=0x1000, i++) {
        kerneltbl[i].page=i;
        kerneltbl[i].writeable=true;
        kerneltbl[i].privileged=false;
        kerneltbl[i].no_write_cache=false;
        kerneltbl[i].no_read_cache=false;
        kerneltbl[i].pat=false;
        kerneltbl[i].no_flush_after_cr3=true;
        kerneltbl[i].active=true;
    }

    //self-mapping page directory
    phys_t pdir_selfmap_addr;
    *pmm >> pdir_selfmap_addr;
    context[1].pagetable = pdir_selfmap_addr >> 12;
    context[1].writeable=true;
    context[1].privileged=false;
    context[1].no_write_cache=true;
    context[1].no_read_cache=false;
    context[1].size=false;
    context[1].active=true;
    pagetbl *pdir_selfmap=(pagetbl *)pdir_selfmap_addr;
    for(int i=0;i<1024;i++) {
        pdir_selfmap[i].active=false;
        pdir_selfmap[i].lazy=false;
    }

    pdir_selfmap[0].page=pagedir_addr >> 12;
    pdir_selfmap[0].writeable=true;
    pdir_selfmap[0].privileged=false;
    pdir_selfmap[0].no_write_cache=true;
    pdir_selfmap[0].no_read_cache=false;
    pdir_selfmap[0].pat=false;
    pdir_selfmap[0].no_flush_after_cr3=false;
    pdir_selfmap[0].active=true;

    phys_t ptbl_selfmap_addr;
    *pmm >> ptbl_selfmap_addr;
    context[2].pagetable = ptbl_selfmap_addr>>12;
    context[2].writeable=true;
    context[2].privileged=false;
    context[2].no_write_cache=true;
    context[2].no_read_cache=false;
    context[2].size=false;
    context[2].active=true;
    pagetbl *ptbl_selfmap=(pagetbl *)ptbl_selfmap_addr;
    for(int i=0;i<1024;i++) {
        ptbl_selfmap[i].active=false;
        ptbl_selfmap[i].lazy=false;
    }


    //Create a self-map
    for(int i=0;i<1024;i++) {
        if(!context[i].active)
            continue;
        ptbl_selfmap[i].page = context[i].pagetable;
        ptbl_selfmap[i].writeable=true;
        ptbl_selfmap[i].privileged=false;
        ptbl_selfmap[i].no_write_cache=true;
        ptbl_selfmap[i].no_read_cache=false;
        ptbl_selfmap[i].pat=false;
        ptbl_selfmap[i].no_flush_after_cr3=false;
        ptbl_selfmap[i].active=true;
    }
}

paging_context_x86::~paging_context_x86() {
    //Deallocate user space
    pagedir *pd = (pagedir*)0x401000;
    for(int i=256;i<1024;i++) {
        if(!pd[i].active)
            continue;
        pagetbl *pt = (pagetbl*)(0xC00000+(i<<12));
        for(int j=0;j<1024;j++) {
            if(!pt[j].active)
                continue;
            void * addr = (void *)((i<<22)+(j<<12));
            munmap(addr);
        }
        munmap(pt);
        *pmm << (pd[i].pagetable << 12);
    }
    //Deallocate selfmap
    for(int i=1;i<3;i++) {
        if(!pd[i].active)
            continue;
        pagetbl *pt = (pagetbl*)(0xC00000+(i<<12));
        for(int j=0;j<1024;j++) {
            if(!pt[j].active)
                continue;
            void * addr = (void *)((i<<22)+(j<<12));
            munmap(addr);
        }
        munmap(pt);
        *pmm << (pd[i].pagetable << 12);
    }
    munmap(pd);
    *pmm << pagedir_addr;
}

void paging_context_x86::switch_context() {
    asm volatile("mov %0, %%cr3" :: "r"(pagedir_addr) : "memory");
    if(!context_enabled) {
        uint32_t cr0;
        asm volatile("mov %%cr0, %0" : "=r"(cr0));
        cr0 |= (1<<31);
        asm volatile("mov %0, %%cr0" :: "r"(cr0) : "memory");
    }
    paging_context::switch_context();
}

void paging_context_x86::map_pagetable(struct paging_context *pc_t) {
    paging_context_x86 *pc = (paging_context_x86*)pc_t;
    mmap(pc->pagedir_addr, (void*)0x401000, protection::RW, false);
    pagedir * pd = (pagedir*)0x401000;
    for(int i=0;i<1024;i++) {
        if(!pd[i].active)
            continue;
        mmap(pd[i].pagetable<<12, (void*)(0xC00000), protection::RW, false);
    }
}

void *paging_context_x86::mmap(phys_t addr, void *dest, protection prot, bool lazy) {
    if((!addr)&&(!lazy)) {
        *pmm >> addr;
    }
    if(!dest) {
        if(!lazy) {
            for(phys_t i=16; i < 256; i++) {
                if((**this)[i].active) {
                    for(phys_t j=0; j<1024; j++) {
                        if((*this)[i][j].active)
                            continue;
                        dest = (void*)((i<<22)|(j<<12));
                    }
                    continue;
                }
                dest = (void*)(i<<22);
            }
        } else {
            for(phys_t i=256; i; i++) {
                if((**this)[i].active) {
                    for(phys_t j=0; j<1024; j++) {
                        if((*this)[i][j].active)
                            continue;
                        if((*this)[i][j].lazy)
                            continue;
                        dest = (void*)((i<<22)|(j<<12));
                    }
                    continue;
                }
                dest = (void*)(i<<22);
            }
        }
    }
    if(!dest) {
        panic("Not enough virtual memory is available for mmap()");
    }
    phys_t i=((phys_t)dest)>>22;
    phys_t j=((phys_t)dest)>>12;
    j&=0x3FF;
    if(!(**this)[i].active) {
        phys_t ptbl_addr;
        *pmm >> ptbl_addr;
        mmap(ptbl_addr, (*this)[i], protection::RW, false);
        pagetbl *pent = (*this)[i];
        for(int i=0;i<1024;i++)
            pent[i].active=false;
        pagedir *ptbl = &((**this)[i]);
        ptbl->pagetable = ptbl_addr >> 12;
        ptbl->writeable = true;
        ptbl->privileged = (i < 256) ? false: true;
        ptbl->no_write_cache = (i < 16) ? true : false;
        ptbl->no_read_cache = false;
        ptbl->size = false;
        ptbl->active=true;
    }
    pagetbl *pent = (*this)[i] + j;
    uint8_t p = (uint8_t)prot;
    pent->page = addr >> 12;
    pent->writeable = (p & 2)? true : false;
    pent->privileged = (p & 4)? true : false;
    pent->no_write_cache = (i < 16) ? true : false;
    pent->no_read_cache = false;
    pent->pat = false;
    pent->no_flush_after_cr3 = false;
    pent->lazy = lazy;
    pent->active = !lazy;
    if(current_context == this) {
        asm volatile("invlpg %0" :: "m"(*(char*)dest) : "memory");
    }
    return dest;
}

void paging_context_x86::munmap(void *addr) {
    phys_t i=((phys_t)addr)>>22;
    phys_t j=((phys_t)addr)>>12;
    j &= 0x3FF;
    if(!(**this)[i].active)
        return;
    if(!(*this)[i][j].active)
        (*this)[i][j].lazy = false;
    *pmm << ((*this)[i][j].page << 12);
    (*this)[i][j].active=false;
}

bool paging_context_x86::is_mapped(void *addr) {
    phys_t i=((phys_t)addr)>>22;
    phys_t j=((phys_t)addr)>>12;
    j &= 0x3FF;
    if(!(**this)[i].active)
        return false;
    return (*this)[i][j].active;
}

void paging_context_x86::mprotect(void *addr, protection prot) {
    phys_t i=((phys_t)addr)>>22;
    phys_t j=((phys_t)addr)>>12;
    j &= 0x3FF;
    if(!(**this)[i].active)
        return;
    if((!(*this)[i][j].active) && (!(*this)[i][j].lazy))
        return;
    pagetbl *pent=  &((*this)[i][j]);
    uint8_t p = (uint8_t)prot;
    pent->writeable = (p & 2) ? true : false;
    pent->privileged = (p & 4) ? true : false;
    if(current_context == this)
        asm volatile("invlpg %0" :: "m"(*(char*)addr) : "memory");
}

bool paging_context_x86::has_no_exec() {
    return false;
}

void *paging_context_x86::get_exception_address(cpu_state *cpu) {
    uint32_t cr2;
    asm volatile("mov %%cr2, %0" : "=r"(cr2));
    return (void*)cr2;
}

bool paging_context_x86::lazymap(void *addr, cpu_state *cpu) {
    phys_t i=((phys_t)addr)>>22;
    phys_t j=((phys_t)addr)>>12;
    j &= 0x3FF;
    if(!(**this)[i].active)
        return false;
    if((*this)[i][j].active)
        return false;
    if(!(*this)[i][j].lazy)
        return false;
    phys_t page;
    *pmm >> page;
    pagetbl *pent = &((*this)[i][j]);
    pent->page = page >> 12;
    pent->lazy = false;
    pent->active = true;
    return true;
}

pagedir * paging_context_x86::operator*() {
    if(current_context == this)
        return (pagedir*)0x400000;
    return (pagedir*)0x401000;
}

pagetbl * paging_context_x86::operator[](phys_t i) {
    if(current_context == this)
        return (pagetbl *)(0x800000+(i<<12));
    return (pagetbl *)(0xC00000+(i<<12));
}
