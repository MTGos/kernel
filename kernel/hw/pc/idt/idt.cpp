#include "idt.hpp"
extern "C" void intr_stub_0();
IDT_entry entries[256];
void setIDTEntry(int i, void *entry) {
    uintptr_t p = (uintptr_t)entry;
    entries[i].offset0 = (uint16_t)p;
    p >>= 16;
#ifndef __x86_64__
    entries[i].selector = 0x8;
#else
    entries[i].selector = 0x28;
#endif
    entries[i].zero = 0;
    entries[i].type = 0b1110;
    entries[i].storageSeg = false;
    entries[i].dpl = 3;
    entries[i].used = true;
    entries[i].offset1 = (uint16_t)p;
    p >>= 16;
#ifdef __x86_64__
    entries[i].offset2 = (uint32_t)p;
    entries[i].zero2 = 0;
#endif
}
void initIDT() {
    char *start_vectors = (char *)&intr_stub_0;
    for (int i = 0; i < 256; i++) setIDTEntry(i, start_vectors + 16 * i);
    struct {
        uint16_t size;
        IDT_entry *off;
    } __attribute__((packed)) idtr;
    idtr.size = sizeof(entries);
    idtr.off = (IDT_entry *)(&entries);
    asm volatile("lidt %0" : : "m"(idtr));
}
