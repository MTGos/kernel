#pragma once
#include <stdint.h>
#include <pmm.hpp>
#include <regs.h>
#include <paging.hpp>
struct pagedir {
    bool active:1;
    bool writeable:1;
    bool privileged:1;
    bool no_write_cache:1;
    bool no_read_cache:1;
    bool accessed:1;
    bool ignored:1;
    bool size:1;
    bool ignored2:1;
    uint8_t ignored3:3;
    phys_t pagetable:20;
}__attribute__((packed));
static_assert(sizeof(pagedir)==4);

struct pagetbl {
    bool active:1;
    bool writeable:1;
    bool privileged:1;
    bool no_write_cache:1;
    bool no_read_cache:1;
    bool accessed:1;
    bool written:1;
    bool pat:1;
    bool no_flush_after_cr3:1;
    uint8_t ignored:2;
    bool lazy:1;
    phys_t page:20;
}__attribute__((packed));
static_assert(sizeof(pagetbl)==4);

struct paging_context_x86: public paging_context {
    phys_t pagedir_addr;
    paging_context_x86();
    virtual ~paging_context_x86();
    virtual void switch_context();
    virtual void map_pagetable(struct paging_context *);
    virtual void *mmap(phys_t addr, void *dest, protection prot=protection::RW, bool lazy=true);
    virtual void munmap(void *addr);
    virtual bool is_mapped(void *addr);
    virtual void mprotect(void *addr, protection prot);
    virtual bool has_no_exec();
    virtual void *get_exception_address(cpu_state *cpu);
    virtual bool lazymap(void *addr, cpu_state *cpu);
    pagedir * operator*();
    pagetbl * operator[](phys_t);
};
