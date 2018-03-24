#include <paging.h>
#include <base.hpp>
extern int kernel_start;
extern int kernel_end;

pagemap_tbl *curr_pagemaptbl = (pagemap_tbl *)(0x8000000000);
pagemap_tbl *ref_pagemaptbl = (pagemap_tbl *)(0x8000001000);
pagemap *curr_pagemap = (pagemap *)(0x8000200000);
pagemap *ref_pagemap = (pagemap *)(0x8000400000);
pagedir *curr_pagedir = (pagedir *)(0x8040000000);
pagedir *ref_pagedir = (pagedir *)(0x8080000000);
pagetbl *curr_pagetbl = (pagetbl *)(0x10000000000);
pagetbl *ref_pagetbl = (pagetbl *)(0x18000000000);
inline uint64_t unwrap(phys_t addr) {
    return addr & 0xFFFFFFFFFFFF;
}
inline uint64_t pagemaptbl_index(phys_t addr) {
    return unwrap(addr) >> 39;
}
inline uint64_t pagemap_index(phys_t addr) {
    return unwrap(addr) >> 30;
}
inline uint64_t pagedir_index(phys_t addr) {
    return unwrap(addr) >> 21;
}
inline uint64_t pagetbl_index(phys_t addr) {
    return unwrap(addr) >> 12;
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
inline void* make_virt(phys_t addr) {
    return (void*)wrap(addr);
}


pagemap_tbl kernel[256];

paging_context_x86::paging_context_x86(): paging_context(), _512G_full(false), _1G_full(false), _2M_full(false) {
    *pmm >> pagemaptbl_addr;
    phys_t pagedir_addr = pagemaptbl_addr;
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
                        mmap(ref_pagetbl[l].page<<12, curr_pagetbl+l, protection::RW, false);
                    }
                }
            }
        }
        //TODO: find an architecture with so many layers of paging that i run out of letters
        return;
    }
    //Map the entire kernel
    pagemap_tbl *pagemaptbl = (pagemap_tbl *)pagemaptbl_addr;
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
    phys_t nullpagetbl;
    *pmm >> nullpagetbl;
    kpagedir[0].pagetbl = nullpagetbl >> 12;
    kpagedir[0].writeable=true;
    kpagedir[0].unprivileged=false;
    kpagedir[0].no_write_cache=false;
    kpagedir[0].no_read_cache=false;
    kpagedir[0].sbz=0;
    kpagedir[0]._2M_page=false;
    kpagedir[0].global=true;
    kpagedir[0].executable=false;
    kpagedir[0].active=true;

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
    for(int i=0;i<512;i++) {
        kpagetbl[i].active=false;
        kpagetbl[i].lazy=false;
    }
    for(phys_t i=(((phys_t)&kernel_start)>>12)-512,p=(phys_t)&kernel_start;p<(phys_t)&kernel_end;p+=0x1000) {
        kpagetbl[i].page=i+512;
        kpagetbl[i].writeable=true;
        kpagetbl[i].unprivileged=false;
        kpagetbl[i].no_write_cache=false;
        kpagetbl[i].no_read_cache=false;
        kpagetbl[i].sbz=false;
        kpagetbl[i].pat=false;
        kpagetbl[i].global=true;
        kpagetbl[i].executable=false;
        kpagetbl[i].active=true;
        i++;
    }

    //build the selfmap
    phys_t  pagemap_self_1;
    *pmm >> pagemap_self_1;
    pagemaptbl[1].pagemap=pagemap_self_1 >> 12;
    pagemaptbl[1].writeable=true;
    pagemaptbl[1].unprivileged=false;
    pagemaptbl[1].no_write_cache=true;
    pagemaptbl[1].no_read_cache=true;
    pagemaptbl[1].sbz=0;
    pagemaptbl[1].executable=false;
    pagemaptbl[1].active=true;

    pagemap *selfmap_1 = (pagemap*)pagemap_self_1;
    for(int i=0;i<512;i++)
        selfmap_1[i].active=false;

    phys_t pagedir_self_1;
    *pmm >> pagedir_self_1;

    selfmap_1[0].pagedir=pagedir_self_1 >> 12;
    selfmap_1[0].writeable=true;
    selfmap_1[0].unprivileged=false;
    selfmap_1[0].no_write_cache=true;
    selfmap_1[0].no_read_cache=true;
    selfmap_1[0]._1G_page=false;
    selfmap_1[0].sbz=0;
    selfmap_1[0].executable=false;
    selfmap_1[0].active=true;

    pagedir *selfdir_1 = (pagedir*)pagedir_self_1;
    for(int i=0;i<512;i++)
        selfdir_1[i].active=false;

    //init l2 selftbl

    phys_t pagetbl_self_1;
    *pmm >> pagetbl_self_1;
    selfdir_1[0].pagetbl=pagetbl_self_1 >> 12;
    selfdir_1[0].writeable=true;
    selfdir_1[0].unprivileged=false;
    selfdir_1[0].no_write_cache=true;
    selfdir_1[0].no_read_cache=true;
    selfdir_1[0]._2M_page=false;
    selfdir_1[0].sbz=0;
    selfdir_1[0].executable=false;
    selfdir_1[0].active=true;

    //map selfmap of pagemaptbl

    pagetbl *selftbl_1 = (pagetbl*)pagetbl_self_1;
    for(int i=0;i<512;i++) {
        selftbl_1[i].active=false;
        selftbl_1[i].lazy=false;
    }

    selftbl_1[0].page=pagedir_addr >> 12;
    selftbl_1[0].writeable=true;
    selftbl_1[0].unprivileged=false;
    selftbl_1[0].no_write_cache=true;
    selftbl_1[0].no_read_cache=true;
    selftbl_1[0].pat=false;
    selftbl_1[0].global=false;
    selftbl_1[0].sbz=0;
    selftbl_1[0].executable=false;
    selftbl_1[0].active=true;

    phys_t pagetbl_self_2;
    *pmm >> pagetbl_self_2;
    selfdir_1[1].pagetbl=pagetbl_self_2 >> 12;
    selfdir_1[1].writeable=true;
    selfdir_1[1].unprivileged=false;
    selfdir_1[1].no_write_cache=true;
    selfdir_1[1].no_read_cache=true;
    selfdir_1[1]._2M_page=false;
    selfdir_1[1].global=false;
    selfdir_1[1].sbz=0;
    selfdir_1[1].executable=false;
    selfdir_1[1].active=true;

    //map selfmap of pagemap

    pagetbl * selftbl_2 = (pagetbl*)pagetbl_self_2;
    for(int i=0;i<512;i++) {
        if(!pagemaptbl[i].active) {
            selftbl_2[i].active=false;
            selftbl_2[i].lazy=false;
        } else {
            selftbl_2[i].page=pagemaptbl[i].pagemap;
            selftbl_2[i].writeable=true;
            selftbl_2[i].unprivileged=false;
            selftbl_2[i].no_write_cache=true;
            selftbl_2[i].no_read_cache=true;
            selftbl_2[i].pat=false;
            selftbl_2[i].global=false;
            selftbl_2[i].sbz=0;
            selftbl_2[i].executable=false;
            selftbl_2[i].active=true;
        }
    }

    phys_t pagedir_self_2;
    *pmm >> pagedir_self_2;
    selfmap_1[1].pagedir=pagedir_self_2 >> 12;
    selfmap_1[1].writeable=true;
    selfmap_1[1].unprivileged=false;
    selfmap_1[1].no_write_cache=true;
    selfmap_1[1].no_read_cache=true;
    selfmap_1[1]._1G_page=false;
    selfmap_1[1].global=false;
    selfmap_1[1].sbz=0;
    selfmap_1[1].executable=false;
    selfmap_1[1].active=true;

    //map selfmap of pagedir

    pagedir * selfdir_2 = (pagedir*)pagedir_self_2;
    for(int i=0;i<512;i++) {
        if(!pagemaptbl[i].active) {
            selfdir_2[i].active=false;
        } else {
            phys_t pagetbl_3;
            *pmm >> pagetbl_3;
            selfdir_2[i].pagetbl = pagetbl_3 >> 12;
            selfdir_2[i].writeable=true;
            selfdir_2[i].unprivileged=false;
            selfdir_2[i].no_write_cache=true;
            selfdir_2[i].no_read_cache=true;
            selfdir_2[i]._2M_page=false;
            selfdir_2[i].global=false;
            selfdir_2[i].sbz=0;
            selfdir_2[i].executable=false;
            selfdir_2[i].active=true;

            pagemap * pm = (pagemap*)(pagemaptbl[i].pagemap << 12);
            pagetbl * pt = (pagetbl*)pagetbl_3;
            for(int j=0;j<512;j++) {
                if(!pm[j].active) {
                    pt[j].active=false;
                    pt[j].lazy=false;
                } else {
                    pt[j].page=pm[j].pagedir;
                    pt[j].writeable=true;
                    pt[j].unprivileged=false;
                    pt[j].no_write_cache=true;
                    pt[j].no_read_cache=true;
                    pt[j].pat=false;
                    pt[j].global=false;
                    pt[j].sbz=0;
                    pt[j].executable=false;
                    pt[j].active=true;
                }
            }
        }
    }

    phys_t selfmap_self_2;
    *pmm >> selfmap_self_2;
    pagemaptbl[2].pagemap = selfmap_self_2 >> 12;
    pagemaptbl[2].writeable=true;
    pagemaptbl[2].unprivileged=false;
    pagemaptbl[2].no_write_cache=false;
    pagemaptbl[2].no_read_cache=false;
    pagemaptbl[2].sbz=0;
    pagemaptbl[2].executable=false;
    pagemaptbl[2].active=true;
    pagemap * selfmap_2 = (pagemap*)selfmap_self_2;
    for(int i=0;i<512;i++) {
        if(i==2) {
            selfmap_2[i].active=false;
            continue;
        }
        if(!pagemaptbl[i].active) {
            selfmap_2[i].active=false;
        } else {
            phys_t pagedir_3;
            *pmm >> pagedir_3;
            selfmap_2[i].pagedir = pagedir_3 >> 12;
            selfmap_2[i].unprivileged=false;
            selfmap_2[i].no_write_cache=true;
            selfmap_2[i].no_read_cache=true;
            selfmap_2[i]._1G_page=false;
            selfmap_2[i].global = false;
            selfmap_2[i].sbz=false;
            selfmap_2[i].executable=false;
            selfmap_2[i].active=true;

            pagemap *spm = (pagemap*)(pagemaptbl[i].pagemap << 12);
            pagedir *dpd = (pagedir*)pagedir_3;
            for(int j=0;j<512;j++) {
                if(!spm[j].active) {
                    dpd[j].active=false;
                } else {
                    phys_t pagetbl_3;
                    *pmm >> pagetbl_3;
                    dpd[j].pagetbl = pagetbl_3 >> 12;
                    dpd[j].unprivileged = false;
                    dpd[j].no_write_cache=true;
                    dpd[j].no_read_cache=true;
                    dpd[j]._2M_page=false;
                    dpd[j].global = false;
                    dpd[j].sbz=false;
                    dpd[j].executable=false;
                    dpd[j].active=true;

                    pagedir *spd = (pagedir*)(spm[j].pagedir << 12);
                    pagetbl *dpt = (pagetbl*)pagetbl_3;
                    for(int k=0;k<512;k++) {
                        if(!spd[k].active) {
                            dpt[k].active=false;
                            dpt[k].lazy=false;
                        } else {
                            dpt[k].page=spd[k].pagetbl;
                            dpt[k].writeable=true;
                            dpt[k].unprivileged=false;
                            dpt[k].no_write_cache=true;
                            dpt[k].no_read_cache=true;
                            dpt[k].pat=false;
                            dpt[k].global=false;
                            dpt[k].sbz=0;
                            dpt[k].executable=false;
                            dpt[k].active=true;
                        }
                    }
                }
            }
        }
    }
    //needs more nesting


    //Fix selfmap
    bool has_changed=true;
    while(has_changed) {
        has_changed = false;
        for(int i=0;i<512;i++) {
            if(!pagemaptbl[i].active)
                continue;
            if(!selftbl_2[i].active) {
                //add mapping
                selftbl_2[i].page = pagemaptbl[i].pagemap;
                selftbl_2[i].writeable = true;
                selftbl_2[i].unprivileged=false;
                selftbl_2[i].no_write_cache=true;
                selftbl_2[i].no_read_cache=true;
                selftbl_2[i].pat=false;
                selftbl_2[i].global=false;
                selftbl_2[i].sbz=0;
                selftbl_2[i].executable=false;
                selftbl_2[i].active=true;
                has_changed=true;
            }
            pagemap *spagemap = (pagemap*)(pagemaptbl[i].pagemap << 12);
            for(int j=0;j<512;j++) {
                if(!spagemap[j].active)
                    continue;
                if(!selfdir_2[i].active) {
                    phys_t ptbl;
                    *pmm >> ptbl;
                    selfdir_2[i].pagetbl = ptbl >> 12;
                    selfdir_2[i].unprivileged=false;
                    selfdir_2[i].no_write_cache=true;
                    selfdir_2[i].no_read_cache=true;
                    selfdir_2[i]._2M_page=false;
                    selfdir_2[i].global=false;
                    selfdir_2[i].sbz=0;
                    selfdir_2[i].executable=false;
                    selfdir_2[i].active=true;
                    pagetbl *selftbl = (pagetbl*)(selfdir_2[i].pagetbl<<12);
                    for(int k=0;k<512;k++)
                        selftbl[k].active = false;
                    has_changed=true;
                }
                pagetbl *selftbl = (pagetbl*)(selfdir_2[i].pagetbl<<12);
                if(!selftbl[j].active) {
                    selftbl[j].page = spagemap[j].pagedir;
                    selftbl[j].unprivileged=false;
                    selftbl[j].no_write_cache=true;
                    selftbl[j].no_read_cache=true;
                    selftbl[j].pat=false;
                    selftbl[j].global=false;
                    selftbl[j].sbz=0;
                    selftbl[j].executable=false;
                    selftbl[j].active=true;
                    has_changed=true;
                }
                pagedir *spagedir=(pagedir*)(spagemap[j].pagedir<<12);
                for(int k=0;k<512;k++) {
                    if(!spagedir[k].active)
                        continue;
                    if(!selfmap_2[i].active) {
                        phys_t pdir;
                        *pmm >> pdir;
                        selfmap_2[i].pagedir = pdir >> 12;
                        selfmap_2[i].unprivileged=false;
                        selfmap_2[i].no_write_cache=true;
                        selfmap_2[i].no_read_cache=true;
                        selfmap_2[i]._1G_page=false;
                        selfmap_2[i].global=false;
                        selfmap_2[i].sbz=0;
                        selfmap_2[i].executable=false;
                        selfmap_2[i].active=true;
                        pagedir *selfdir = (pagedir*)(selfmap_2[i].pagedir<<12);
                        for(int l=0;l<512;l++)
                            selfdir[l].active = false;
                        has_changed=true;
                    }
                    pagedir *selfdir = (pagedir*)(selfmap_2[i].pagedir<<12);
                    if(!selfdir[j].active) {
                        phys_t ptbl;
                        *pmm >> ptbl;
                        selfdir[j].pagetbl = ptbl >> 12;
                        selfdir[j].unprivileged=false;
                        selfdir[j].no_write_cache=true;
                        selfdir[j].no_read_cache=true;
                        selfdir[j]._2M_page=false;
                        selfdir[j].global=false;
                        selfdir[j].sbz=0;
                        selfdir[j].executable=false;
                        selfdir[j].active=true;
                        pagetbl *selftbl = (pagetbl*)(selfdir[j].pagetbl<<12);
                        for(int l=0;l<512;l++)
                            selftbl[l].active = false;
                        has_changed=true;
                    }
                    pagetbl *selftbl = (pagetbl*)(selfdir[j].pagetbl<<12);
                    if(!selftbl[k].active) {
                        selftbl[k].page = spagedir[k].pagetbl;
                        selftbl[k].unprivileged=false;
                        selftbl[k].no_write_cache=true;
                        selftbl[k].no_read_cache=true;
                        selftbl[k].pat=false;
                        selftbl[k].global=false;
                        selftbl[k].sbz=0;
                        selftbl[k].executable=false;
                        selftbl[k].active=true;
                        has_changed=true;
                    }
                }
            }
        }
    }
}

paging_context_x86::~paging_context_x86() {
    //deallocate userspace
    phys_t cur_addr = 0xFFFF000000000000ull;
    while(cur_addr >= 0xFFFF000000000000ull) {
        if(!curr_pagemaptbl[pagemaptbl_index(cur_addr)].active) {
            cur_addr += 0x8000000000ull; //512GB
            continue;
        }
        if(!curr_pagemap[pagemap_index(cur_addr)].active) {
            cur_addr += 0x40000000u; //1GB
            continue;
        }
        if(!curr_pagedir[pagedir_index(cur_addr)].active) {
            cur_addr += 0x200000; //2MB
            continue;
        }
        if(!curr_pagetbl[pagetbl_index(cur_addr)].active) {
            cur_addr += 0x1000; //4KB
        }
        munmap(make_virt(cur_addr));
        cur_addr+=0x1000;
    }

    //deallocate selfmap
    phys_t addr = 0x100000000000;
    while(addr >= 0xC000000000) {
        if(!curr_pagemap[pagemap_index(addr)].active) {
            addr += 0x40000000u; //1GB
        }
        if(!curr_pagedir[pagedir_index(addr)].active) {
            addr += 0x200000; //2MB
        }
    }
}

void paging_context_x86::switch_context() {
    pagemap_tbl *p=(pagemap_tbl*)pagemaptbl_addr;
    if(context_enabled)
        p=curr_pagemaptbl;
    for(int i=0;i<256;i++) {
        kernel[i]=p[i];
    }
    asm volatile("mov %0, %%cr3" :: "r"(pagemaptbl_addr) : "memory");
    if(context_enabled) {
        for(int i=0;i<256;i++) {
            if((i>=1) && (i<=3)) {
                continue;
            } else {
                p[i]=kernel[i];
            }
        }
    }
    paging_context::switch_context();
}

void paging_context_x86::map_pagetable(paging_context *pc_t) {
    paging_context_x86 *pc = (paging_context_x86*)pc_t;
    mmap(pc->pagemaptbl_addr, ref_pagemaptbl, protection::RW, false);
    for(uint64_t i=0;i<512;i++) {
        if(!ref_pagemaptbl[i].active)
            continue;
        mmap(ref_pagemaptbl[i].pagemap << 12, ref_pagemap+(i<<9), protection::RW, false);
        for(uint64_t j=0;j<512;j++) {
            if(!ref_pagemap[j+(i<<9)].active)
                continue;
            mmap(ref_pagemap[j+(i<<9)].pagedir << 12, ref_pagedir+(i<<18)+(j<<9), protection::RW, false);
            for(uint64_t k=0;k<512;k++) {
                if(!ref_pagedir[k+(j<<9)+(i<<18)].active)
                    continue;
                mmap(ref_pagedir[k+(j<<9)+(i<<18)].pagetbl << 12, ref_pagetbl+(i<<27)+(j<<18)+(k<<9), protection::RW, false);
            }
        }
    }
}

void *paging_context_x86::mmap(phys_t phys, void* addr, protection prot, bool lazy) {
    auto pagemaptbls = (current_context == this)?curr_pagemaptbl:ref_pagemaptbl;
    auto pagemaps = (current_context == this)?curr_pagemap:ref_pagemap;
    auto pagedirs = (current_context == this)?curr_pagedir:ref_pagedir;
    auto pagetbls = (current_context == this)?curr_pagetbl:ref_pagetbl;
    if((!phys)&&(!lazy)) {
        *pmm >> phys;
    }
    if(!addr) {
        phys_t start=lazy?0xFFFF800000000000ull:0x14000000000ull;
        phys_t end=lazy?~0ull:0x800000000000ull;
        phys_t caddr = start;
        bool found=false;
        if(!_512G_full) {
            for(auto i=pagemaptbl_index(start);i!=pagemaptbl_index(end);i++) {
                if(!pagemaptbls[i].active) {
                    addr = make_virt(i,0,0,0);
                    found=true;
                    break;
                }
            }
        }
        if(!_1G_full && !found) {
            _512G_full=true;
            for(auto i=pagemap_index(start);i!=pagemap_index(end);i++) {
                if(!pagemaps[i].active) {
                    addr = make_virt(0,i,0,0);
                    found=true;
                    break;
                }
            }
        }
        if(!_2M_full && !found) {
            _1G_full = true;
            for(auto i=pagedir_index(start);i!=pagedir_index(end);i++) {
                if(!pagedirs[i].active) {
                    addr = make_virt(0,0,i,0);
                    found=true;
                    break;
                }
            }
        }
        if(!found) {
            _2M_full=true;
            for(auto i=pagetbl_index(start);i!=pagetbl_index(end);i++) {
                if(!pagetbls[i].active) {
                    addr = make_virt(0,0,0,i);
                    found=true;
                    break;
                }
            }
        }
        if(!found) {
            panic("Not enough virtual address space available (how did you do that?)");
        }
    }

    phys_t v = unwrap((phys_t)addr);
    phys_t l = v>>12;
    phys_t k = v>>21;
    phys_t j = v>>30;
    phys_t i = v>>39;

    if(!pagemaptbls[i].active) {
        phys_t pm_addr;
        *pmm >> pm_addr;
        mmap(pm_addr, curr_pagemap+(i<<9), protection::RW, false);
        if(current_context != this)
            current_context->mmap(pm_addr, pagemaps+(i<<9), protection::RW, false);
        for(int x=0;x<512;x++)
            pagemaps[x+(i<<9)].active=false;
        pagemap_tbl &pm = pagemaptbls[i];
        pm.pagemap = pm_addr>>12;
        pm.writeable = true;
        pm.unprivileged = i>=256;
        pm.no_write_cache=i<4;
        pm.no_read_cache=false;
        pm.sbz=0;
        pm.executable=false;
        pm.active=true;
    }
    if(!pagemaps[j].active) {
        phys_t pd_addr;
        *pmm >> pd_addr;
        mmap(pd_addr, curr_pagedir+(j<<9), protection::RW, false);
        if(current_context != this)
            current_context->mmap(pd_addr, pagedirs+(j<<9), protection::RW, false);
        for(int x=0;x<512;x++)
            pagedirs[x+(j<<9)].active=false;
        pagemap &pd = pagemaps[j];
        pd.pagedir = pd_addr>>12;
        pd.writeable=true;
        pd.unprivileged = i>=256;
        pd.no_write_cache=i<4;
        pd.no_read_cache=false;
        pd.sbz=0;
        pd.global=false;
        pd._1G_page=false;
        pd.executable=false;
        pd.active=true;
    }
    if(!pagedirs[k].active) {
        phys_t pt_addr;
        *pmm >> pt_addr;
        mmap(pt_addr, curr_pagetbl+(k<<9), protection::RW, false);
        if(current_context != this)
            current_context->mmap(pt_addr, pagetbls+(k<<9), protection::RW, false);
        for(int x=0;x<512;x++)
            pagetbls[x+(k<<9)].active=false;
        pagedir &pt = pagedirs[k];
        pt.pagetbl = pt_addr >> 12;
        pt.writeable=true;
        pt.unprivileged = i>=256;
        pt.no_write_cache=i<4;
        pt.no_read_cache=false;
        pt.sbz=0;
        pt.global=false;
        pt._2M_page=false;
        pt.executable=false;
        pt.active=true;
    }
    uint8_t p=(uint8_t)prot;
    pagetbl &page = pagetbls[l];
    page.page = phys >> 12;
    page.writeable = p & 2;
    page.unprivileged = p&4;
    page.no_write_cache=i<4;
    page.no_read_cache=false;
    page.sbz=0;
    page.pat=false;
    page.global=false;
    page.lazy=lazy;
    page.executable=false; //!(p&1);
    page.active=!lazy;

    if(current_context == this) {
        asm volatile("invlpg %0" :: "m"(*(char*)addr) : "memory");
    }
    return addr;
}

void paging_context_x86::munmap(void *addr) {
   auto pagetbls = (current_context == this)?curr_pagetbl:ref_pagetbl;
   if(__builtin_expect(!is_mapped(addr),0))
        return;
    phys_t l=unwrap((phys_t)addr)>>12;
    pagetbls[l].active=false;
    pagetbls[l].lazy=false;
    asm volatile("invlpg %0" :: "m"(*(char*)addr) : "memory");
}

bool paging_context_x86::is_mapped(void* addr) {
    phys_t v = unwrap((phys_t)addr);
    phys_t l = v>>12;
    phys_t k = v>>21;
    phys_t j = v>>30;
    phys_t i = v>>39;
    auto pagemaptbls = (current_context == this)?curr_pagemaptbl:ref_pagemaptbl;
    auto pagemaps = (current_context == this)?curr_pagemap:ref_pagemap;
    auto pagedirs = (current_context == this)?curr_pagedir:ref_pagedir;
    auto pagetbls = (current_context == this)?curr_pagetbl:ref_pagetbl;

    if(!pagemaptbls[i].active)
        return false;
    if(!pagemaps[j].active)
        return false;
    if(!pagedirs[k].active)
        return false;
    return pagetbls[l].active;
}

void paging_context_x86::mprotect(void* addr, protection prot) {
    auto pagetbls = (current_context == this)?curr_pagetbl:ref_pagetbl;
    if(__builtin_expect(!is_mapped(addr),0))
        return;
    phys_t l=unwrap((phys_t)addr)>>12;
    uint8_t p=(uint8_t)prot;
    pagetbl &page=pagetbls[l];
    page.writeable=p&2;
    page.unprivileged=p&4;
    page.executable=false;//!(p&1);
    asm volatile("invlpg %0" :: "m"(*(char*)addr) : "memory");
}

bool paging_context_x86::has_no_exec() {return true;}
void *paging_context_x86::get_exception_address(cpu_state *cpu) {
    uint64_t cr2;
    asm volatile("mov %%cr2, %0" : "=r"(cr2));
    return (void*)cr2;
}
bool paging_context_x86::lazymap(void *addr, cpu_state *cpu) {
    phys_t v = unwrap((phys_t)addr);
    phys_t l = v>>12;
    phys_t k = v>>21;
    phys_t j = v>>30;
    phys_t i = v>>39;
    if(!curr_pagemaptbl[i].active)
        return false;
    if(!curr_pagemap[j].active)
        return false;
    if(!curr_pagedir[k].active)
        return false;
    if(!curr_pagetbl[l].lazy)
        return false;
    phys_t phys;
    *pmm >> phys;
    curr_pagetbl[l].page = phys>>12;
    curr_pagetbl[l].active=true;
    asm volatile("invlpg %0" :: "m"(*(char*)addr) :"memory");
    return true;
}
