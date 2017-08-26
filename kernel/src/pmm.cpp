#include <base.hpp>
#include <pmm.hpp>
extern "C" int kernel_start;
extern "C" int kernel_end;


uint32_t pmm_bitmap[32768];


auto PMM::isFree(phys_t addr) -> bool {
    if(addr == 0)
        return false;
    phys_t start = (phys_t)(&kernel_start);
    phys_t end = (phys_t)(&kernel_end);
    if((addr >= start) && (addr < end))
        return false;
    return true;
}
PMM::PMM(phys_t page_size): page_size(page_size), first_free(0), lowest_page(~0), highest_page(0) {}
void PMM::fill() {
    for(phys_t i=highest_page; i>=lowest_page && i; i-=page_size) {
        if(isFree(i))
            *this << i;
    }
}
PMM::~PMM() {}

auto PMM::operator<<(phys_t page) -> PMM & {
    (*this)(page,1);
    return *this;
}
auto PMM::operator>>(phys_t &page) -> PMM & {
    page = (*this, 1);
    return *this;
}

auto PMM::operator,(size_t no_pages) -> phys_t {
    if(first_free > highest_page)
        panic("No free physical memory is available.");
    if(no_pages == 1) {
        while(!(first_free in *this)) {
            first_free += page_size;
            if(first_free > highest_page)
                panic("No free physical memory is available.");
        }
        phys_t pageno = first_free / page_size;
        phys_t index = pageno >> 5;
        phys_t bit = pageno & 31;
        pmm_bitmap[index] &= ~(1<<bit);
        auto x = first_free;
        first_free += page_size;
        return x;
    }
    //Now we need to find a free page with n-1 free pages after it
    phys_t linear_start = first_free;
    phys_t length = (no_pages-1) * page_size;
    while(true) {
        bool found=true;
        while(!(linear_start in *this)) {
            linear_start += page_size;
            if((linear_start > highest_page)||(linear_start+length > highest_page))
                panic("No free physical memory is available.");
        }
        if(!((linear_start + length) in *this)) {
            linear_start += no_pages * page_size;
        }
        //We're on to something
        phys_t i;
        for(i=linear_start; i < linear_start + length; i+=page_size) {
            if(!(i in *this)) {
                found=false;
                break;
            }
        }
        if(!found) {
            linear_start=i+page_size;
            continue;
        }
        //Set the first_free variable correctly when the first page was a hit
        if(linear_start == first_free)
            first_free += length + page_size;

        phys_t pageno = linear_start / page_size;

        for(i=linear_start; i <= linear_start + length; i+= page_size) {
            phys_t index = pageno >> 5;
            phys_t bit = pageno & 31;
            pmm_bitmap[index] &= ~(1<<bit);
            pageno++;
        }
        return linear_start;
    }

}
auto PMM::operator()(phys_t pages, size_t no_pages) -> void {
    phys_t pageno = pages / page_size;
    if(pages < first_free)
        first_free = pages;
    for(size_t i=0; i<no_pages; i++, pages+=page_size) {
        phys_t index = pageno >> 5;
        phys_t bit = pageno & 31;
        pmm_bitmap[index] |= 1<<bit;
        pageno++;
    }
}
auto PMM::operator&&(phys_t page) -> bool {
    phys_t pageno = page / page_size;
    phys_t index = pageno >> 5;
    if(!pmm_bitmap[index])
        return false;
    phys_t bit = pageno & 31;
    if(!(pmm_bitmap[index] & (1<<bit)))
        return false;
    return true;
}
auto operator&&(phys_t a, PMM mm) -> bool {
    return mm && a;
}
