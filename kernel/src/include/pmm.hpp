#pragma once
#include <stdint.h>
#include <stddef.h>
class PMM;
#include <base.hpp>
typedef uintptr_t phys_t; ///< Type used for physical addresses
/**
 * A single entry in the PMM list
 */

#if BITS == 32
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
        virtual auto operator>>(phys_t &page) -> PMM &; ///< Allocates a page. Probably O(log n)
        virtual auto operator,(size_t no_pages) -> phys_t; ///< Allocates multiple pages. O(n)
        virtual auto operator()(phys_t pages,size_t no_pages) -> void; ///< Deallocates multiple pages. O(n)
        virtual auto operator&&(phys_t page) -> bool; //Returns true if this page is free. O(1).
        virtual auto setUsed(phys_t page) -> void; //Marks a page as used. O(1).
};

#else

struct pmm_entry {
    phys_t next;
    phys_t last;
};

class PMM {
    protected:
        phys_t head;
        virtual auto isFree(phys_t address) -> bool;
        auto fill() -> void;
        phys_t page_size, lowest_page, highest_page;
        void push(phys_t);
        phys_t pop();
    public:
        PMM(phys_t page_size);
        virtual ~PMM();
        virtual auto operator<<(phys_t page) -> PMM &;
        virtual auto operator>>(phys_t &page) -> PMM &;
        virtual auto operator,(size_t no_pages) -> phys_t;
        virtual auto operator()(phys_t pages, size_t no_pages) -> void;
        virtual auto operator&&(phys_t page) -> bool;
        virtual auto setUsed(phys_t page) -> void;
};

#endif
/**
 * This definition is for having a python-like syntax - like `page in pmm`
 */
#define in and

auto operator&&(phys_t a, PMM mm) -> bool; ///< Returns true when page is free. Used for syntax `page in pmm`
