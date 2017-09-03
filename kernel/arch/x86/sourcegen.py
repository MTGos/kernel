print("Writing interrupt stubs")
int_handler.write(".macro intr_stub nr\n.global intr_stub_\\nr\n.align 16\nintr_stub_\\nr:\n    pushl $0\n    pushl $\\nr\n    jmp intr_common_handler\n.endm\n.macro intr_stub_error_code nr\n.global intr_stub_\\nr\n.align 16\nintr_stub_\\nr:\n    pushl $\\nr\n    jmp intr_common_handler\n.endm\n")
for i in range(256):
    if i in [8,10,11,12,13,14,17]:
        int_handler.write("intr_stub_error_code "+str(i)+"\n")
    else:
        int_handler.write("intr_stub "+str(i)+"\n")

int_handler.write(".extern handleINT\n")
int_handler.write("intr_common_handler:\n")
int_handler.write("    cli\n")
print("Generating register save/restore")
all_regs_push=[]
all_regs_pop=[]
data_section=""
for reg in ["ebp","edi","esi","edx","ecx","ebx","eax"]:
    all_regs_push.append("push %"+reg)
    all_regs_pop.append("pop %"+reg)
if config["ENABLE_FPU"] and not config["ENABLE_SSE"]:
    data_section+="fsave_reg:\n    .space 108\n"
    all_regs_push.append("fsave fsave_reg")
    all_regs_pop.append("frstor fsave_reg")
    all_regs_push.append("pushl $fsave_reg")
if config["ENABLE_SSE"]:
    data_section+=".align 16\nfxsave_reg:\n    .space 512"
    all_regs_push.append("fxsave fxsave_reg")
    all_regs_pop.append("fxrstor (%eax)")
    all_regs_pop.append("pop %eax")
    all_regs_push.append("pushl $fxsave_reg")

print("Writing interrupt handler")
for ins in all_regs_push:
    int_handler.write("    "+ins+"\n")
int_handler.write("    push %esp\n    call handleINT\n    mov %eax, %esp\n")
for ins in reversed(all_regs_pop):
    int_handler.write("    "+ins+"\n")
int_handler.write("    add $8, %esp\n    iret\n")

print("Writing panic handler")
int_handler.write(".global panic\n.extern panic2\npanic:\n")
for ins in all_regs_push:
    int_handler.write("    "+ins+"\n")
int_handler.write("    push %esp\n    jmp panic2\n")

int_handler.write(".section .data\n"+data_section)

print("Generating cpu_state struct")
reg_struct.write("#pragma once\n")
reg_struct.write("#include <stdint.h>\n")
if config["ENABLE_FPU"] and not config["ENABLE_SSE"]:
    reg_struct.write("struct fpu_state {\n")
    for reg in ["cwd","swd","twd","fip","fcs","foo","fos"]:
        reg_struct.write("    uint32_t "+reg+";\n")
    for reg in ("st"+str(i) for i in range(8)):
        reg_struct.write("    __float80 "+reg+";\n")
    reg_struct.write("}__attribute__((packed,aligned(16)));\n")
if config["ENABLE_SSE"]:
    reg_struct.write("struct fpu_state {\n")
    for reg in ["cwd","swd","twd","fop"]:
        reg_struct.write("    uint16_t "+reg+";\n")
    for reg in ["fip","fcs","foo","fos","mxcsr","mxcsr_mask"]:
        reg_struct.write("    uint32_t "+reg+";\n")
    for reg in range(8):
        reg_struct.write("    __float80 st"+str(reg)+";\n")
        reg_struct.write("    char buf"+str(reg)+"[6];\n")
    for reg in range(16):
        reg_struct.write("    __float128 xmm"+str(reg)+";\n")
    reg_struct.write("}__attribute__((packed,aligned(16)));\n")
reg_struct.write("struct cpu_state {\n")
if config["ENABLE_FPU"]:
    reg_struct.write("    fpu_state *fpu;\n")
for reg in ["eax","ebx","ecx","edx","esi","edi","ebp","intr","error","eip","cs","eflags","esp","ss"]:
    reg_struct.write("    uint32_t "+reg+";\n")
reg_struct.write("}__attribute__((packed));")

