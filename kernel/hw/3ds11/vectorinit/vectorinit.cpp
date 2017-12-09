#include "vectorinit.hpp"
extern "C" {
extern uintptr_t branch_macro;
void data_abort();
void fast_irq();
void normal_irq();
void prefetch_abort();
void svc_call();
void undefined_op();
}
void initVectors() {
    uintptr_t *vectors = (uintptr_t *)0x1FFFFFA0;
    // branch_macro is a ldr pc, [pc,#-4], meaning it reads the following word as PC
    vectors[0] = branch_macro;
    vectors[1] = (uintptr_t)&normal_irq;
    vectors[2] = branch_macro;
    vectors[3] = (uintptr_t)&fast_irq;
    vectors[4] = branch_macro;
    vectors[5] = (uintptr_t)&svc_call;
    vectors[6] = branch_macro;
    vectors[7] = (uintptr_t)&undefined_op;
    vectors[8] = branch_macro;
    vectors[9] = (uintptr_t)&prefetch_abort;
    vectors[10] = branch_macro;
    vectors[11] = (uintptr_t)&data_abort;
}
IRQ_IO::IRQ_IO() {
    initVectors();
    *((volatile uint32_t*)0x17E00100)=1;
    *((volatile uint32_t*)0x17E00104)=0xF0;
    for(int i=0;i<32;i+=4) {
        *((volatile uint32_t*)(0x17E01100+i))=~0;
    }
    uint32_t intid;
    while(((intid=*((volatile uint32_t*)0x17E00118))&1023)!=1023) {
        *((volatile uint32_t*)0x17E00110)=intid;
    }
}
IRQ_IO::~IRQ_IO() {}

void *IRQ_IO::handleIRQ(void *data) {
    uint32_t interrupt=*((volatile uint32_t*)0x17E0010C);
    int intid = interrupt & 255;
    data = handlers[intid](data);
    *((volatile uint32_t*)0x17E00110) = interrupt;
    return data;
}

void IRQ_IO::mask(int number) {}
void IRQ_IO::unmask(int number) {}

static IRQ_IO irq;
__attribute__((constructor))
static void init_irq() {
    irqs = (IRQ*)&irq;
}
