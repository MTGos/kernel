#include <paging.h>
#include <base.hpp>
extern int kernel_start;
extern int kernel_end;



paging_context_x86::paging_context_x86(): paging_context() {
    *pmm >> pagedir_addr;
    if(context_enabled) {
        current_context->mmap(pagedir_addr, (void*)0x800000, protection::RW, false);
        pagemap *src=(pagemap *)0x400000;
        pagemap *dest=(pagedir*)0x600000;
        for(int i=0;i<256;i++)
            dest[i]=src[i];
        for(int i=256;i<512;i++)
            dest[i].active=false;
        dest[0].active=false;
        phys_t first_pagedir;
        *pmm >> first_pagedir;
        dest[0].pagetable = first_pagedir >> 12;
        dest[0].writeable=true;
        dest[0].unprivileged=false;
        dest[0].no_write_cache=true;
        dest[0].no_read_cache=false;
        dest[0].sbz=0;
        dest[0].active=true;

    }
}
