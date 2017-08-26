#pragma once
#include <stdint.h>
#include <stddef.h>
typedef uintptr_t phys_t; ///< Type used for physical addresses
/**
 * A single entry in the PMM list
 */
/**
 * Physical memory manager. It stores the free memory in a linked list
 */
class PMM {
    protected:
        phys_t first_free;
        virtual auto isFree(phys_t addr) -> bool; ///< Returns true if the provided page is free to use
        auto fill() -> void;
        phys_t page_size; ///< Contains the size of a single memory page, in bytes
        phys_t lowest_page;
        phys_t highest_page;
    public:
        PMM(phys_t page_size);
        virtual ~PMM();
        virtual auto operator<<(phys_t page) -> PMM &; ///< Frees a page. O(1)
        virtual auto operator>>(phys_t &page) -> PMM &; ///< Allocates a page. O(1)
        virtual auto operator,(size_t no_pages) -> phys_t; ///< Allocates multiple pages. O(n²)
        virtual auto operator()(phys_t pages,size_t no_pages) -> void; ///< Deallocates multiple pages. O(n)
        virtual auto operator&&(phys_t page) -> bool; //Returns true if this page is free. O(n).
};
/**
 * This definition is for having a python-like syntax - like `page in pmm`
 */
#define in and

auto operator&&(phys_t a, PMM mm) -> bool; ///< Returns true when page is free. Used for syntax `page in pmm`
