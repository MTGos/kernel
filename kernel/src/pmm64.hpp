#include <pmm.hpp>
#include <paging.hpp>
extern "C" int kernel_start;
extern "C" int kernel_end;

static inline void * map(phys_t addr) {
    if(__builtin_expect(context_enabled, true))
        return current_context->mmap(addr, (void*)1, protection::RW, false);
    return (void*)addr;
}
static inline void unmap(void* addr) {
/*    if(__builtin_expect(context_enabled, true))
        current_context->munmap(addr);*/
}

auto PMM::isFree(phys_t addr) -> bool {
    if(!addr)
        return false;
    auto start = (phys_t)(&kernel_start);
    auto end = (phys_t)(&kernel_end);
    if((addr >= start) && (addr < end))
        return false;
    return true;
}

void PMM::push(phys_t p) {
    phys_t h=head;
    auto ent=(pmm_entry*)map(p);
    ent->next=head;
    ent->last=0;
    head=p;
    unmap(ent);
    ent=(pmm_entry*)map(h);
    ent->last=p;
    unmap(ent);
}
phys_t PMM::pop() {
    phys_t p = head;
    if(!p)
        panic("oom");
    auto ent=(pmm_entry*)map(p);
    head=ent->next;
    unmap(ent);
    ent=(pmm_entry*)map(head);
    ent->last=0;
    unmap(ent);
    return p;
}

PMM::PMM(phys_t page_size): page_size(page_size), head(0), lowest_page(~0), highest_page(0) {
    pmm=this;
}
void PMM::fill() {
    for(phys_t i=highest_page; i>=lowest_page && i; i-=page_size) {
        if(isFree(i))
            *this << i;
    }
}

PMM::~PMM() {}

auto PMM::operator<<(phys_t page) -> PMM & {
    push(page);
    return *this;
}
auto PMM::operator>>(phys_t &page) -> PMM& {
    page=pop();
    return *this;
}
#define self (*this)
auto PMM::operator,(size_t no_pages) -> phys_t {
    if(no_pages == 1)
        return pop();
    for(phys_t i = lowest_page; i < highest_page; i+=page_size) {
        if(not (i in self))
            continue;
        if(not ((i+no_pages*page_size) in self)) {
            i+=(no_pages-1)*page_size;
            continue;
        }
        //we found something
        for(phys_t j = i+page_size; j<i+no_pages*page_size; j+=page_size) {
            if(not (j in self)) {
                i+=(no_pages-1)*page_size;
                goto outlbl;
            }
        }
        //if we are here, we found it
        for(phys_t j = i; j<=i+no_pages*page_size; j+=page_size) {
            setUsed(j);
        }
        return i;
outlbl:
        continue;
    }
}

auto PMM::operator()(phys_t pages, size_t no_pages) -> void {
    for(phys_t i=pages; i<=pages+no_pages*page_size; i+=page_size) {
        if(!isFree(i))
            continue;
        push(i);
    }
}

auto PMM::operator&&(phys_t page) -> bool {
    phys_t curr = head;
    while(curr) {
        if(curr == page)
            return true;
        auto ent = (pmm_entry *)map(curr);
        curr = ent->next;
        unmap(ent);
    }
    return false;
}

auto PMM::setUsed(phys_t page) -> void {
    if(head == page) {
        pop();
        return;
    }
    if(not (page in self))
        return;
    phys_t next, last;
    auto ent = (pmm_entry*)map(page);
    next=ent->next;
    last=ent->last;
    unmap(ent);
    if(next) {
        ent=(pmm_entry*)map(next);
        ent->last=last;
        unmap(ent);
    }
    if(last) {
        ent=(pmm_entry*)map(last);
        ent->next=next;
    }
}

auto operator&&(phys_t a, PMM mm) -> bool{
    return mm in a;
}
