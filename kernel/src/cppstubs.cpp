#include <stddef.h>
#include <stdint.h>
/**
 * __cxa_pure_virtual is function that is pointed to in the vtable entry of a pure virtual function
 * we need to provide this symbol, however it should never get called.
 */
extern "C" void __cxa_pure_virtual() {
    // panic("Pure virtual function called.");
}
void *operator new(size_t s) { return (void *)1; }
void *operator new[](size_t s) { return (void *)1; }
void operator delete(void *p) {}
void operator delete[](void *p) {}
void operator delete(void *p, size_t s) {}
void operator delete[](void *p, size_t s) {}
void *operator new(size_t s, void *p) { return p; }
void *operator new[](size_t s, void *p) { return p; }
void operator delete(void *, void *p) {}
void operator delete[](void *, void *p) {}
__extension__ typedef int __guard __attribute__((mode(__DI__)));
extern "C" int __cxa_guard_acquire(__guard *g) {
    return !*(char*)g;
}
extern "C" int __cxa_guard_release(__guard *g) {
    *(char *)g = 1;
    return 0;
}
