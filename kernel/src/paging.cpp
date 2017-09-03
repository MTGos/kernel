#include <paging.hpp>
paging_context *current_context(nullptr);
bool context_enabled=false;
paging_context::paging_context() {}
paging_context::~paging_context() {}

void paging_context::switch_context() {
    current_context = this;
    context_enabled = true;
}

void paging_context::map_pagetable(struct paging_context *) {}

void* paging_context::mmap(phys_t addr, void* dest, protection prot, bool lazy) {
    return nullptr;
}

void paging_context::munmap(void *addr) {}

bool paging_context::is_mapped(void *addr) {
    return false;
}

void paging_context::mprotect(void *addr, protection prot) {}

bool paging_context::has_no_exec() {
    return false;
}

void *paging_context::get_exception_address(cpu_state *cpu) {
    return nullptr;
}

bool paging_context::lazymap(void *addr, cpu_state *cpu) {
    return false;
}
