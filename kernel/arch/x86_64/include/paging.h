#pragma once
#include <stdint.h>
#include <pmm.hpp>
#include <paging.hpp>
typedef phys_t virt_t;
struct pagemap_tbl {
    bool active:1;
    bool writeable:1;
    bool unprivileged:1;
    bool no_write_cache:1;
    bool no_read_cache:1;
    bool accessed:1;
    uint8_t sbz:3;
    uint8_t ignored:3;
    phys_t pagemap:40;
    uint16_t ignored2:11;
    bool executable:1;

}__attribute__((packed));
static_assert(sizeof(pagemap_tbl)==8);

struct pagemap {
    bool active:1;
    bool writeable:1;
    bool unprivileged:1;
    bool no_write_cache:1;
    bool no_read_cache:1;
    bool accessed:1;
    bool sbz:1;
    bool _1G_page:1;
    bool global:1;
    uint8_t ignored:3;
    phys_t pagedir:40;
    uint16_t ignored2:11;
    bool executable:1;
}__attribute__((packed));
static_assert(sizeof(pagemap)==8);

struct pagedir {
    bool active:1;
    bool writeable:1;
    bool unprivileged:1;
    bool no_write_cache:1;
    bool no_read_cache:1;
    bool accessed:1;
    bool sbz:1;
    bool _2M_page:1;
    bool global:1;
    uint8_t ignored:3;
    phys_t pagetbl:40;
    uint16_t ignored2:11;
    bool executable:1;
}__attribute__((packed));
static_assert(sizeof(pagedir)==8);

struct pagetbl {
    bool active:1;
    bool writeable:1;
    bool unprivileged:1;
    bool no_write_cache:1;
    bool no_read_cache:1;
    bool accessed:1;
    bool sbz:1;
    bool pat:1;
    bool global:1;
    uint8_t ignored:2;
    bool lazy:1;
    phys_t page:40;
    uint16_t ignored2:11;
    bool executable:1;
}__attribute__((packed));


struct paging_context_x86: public paging_context {
    phys_t pagemaptbl_addr;
    bool _512G_full, _1G_full, _2M_full;
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
