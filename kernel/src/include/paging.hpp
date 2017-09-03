#pragma once
#include <stdint.h>
#include <pmm.hpp>
#include <regs.h>

enum class protection: uint8_t {
    NONE=0,
    X=1,
    W=2,
    WX=3,
    R=4,
    RX=5,
    RW=6,
    RWX=7
};

struct paging_context {
    paging_context();
    virtual ~paging_context();
    virtual void switch_context();
    virtual void map_pagetable(struct paging_context *);
    virtual void* mmap(phys_t addr, void* dest, protection prot=protection::RW, bool lazy=true);
    virtual void munmap(void* addr);
    virtual bool is_mapped(void* addr);
    virtual void mprotect(void* addr, protection prot);
    virtual bool has_no_exec();
    virtual void *get_exception_address(cpu_state *cpu);
    virtual bool lazymap(void* addr, cpu_state *cpu);
};

extern paging_context *current_context;
extern bool context_enabled;
