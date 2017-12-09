#include "pmm.hpp"
#include <base.hpp>
enum class DEVICETYPE: uint8_t {
    PC,
    CONSOLE,
    EMBEDDED
};
enum class CPUARCH: uint8_t {
    X86,
    X86_64,
    ARM
};
enum class ENDIAN: uint8_t {
    BIG,
    LITTLE
};
enum class PERM: uint8_t {
    NONE,
    X,
    W,
    WX,
    R,
    RX,
    RW,
    RWX
};
struct MMAP {
    char magic[4];
    uint32_t version;
    uint32_t length;
}__attribute__((packed));
union MMAPTag {
    struct {
        uint8_t id;
        uint8_t size;
    }__attribute__((packed));
    struct {
        uint8_t id;
        uint8_t size;
        DEVICETYPE value;
    }__attribute__((packed)) DEVICETYPE;
    struct {
        uint8_t id;
        uint8_t size;
        char value[1];
    }__attribute__((packed)) DEVICENAME;
    struct {
        uint8_t id;
        uint8_t size;
        CPUARCH value;
    }__attribute__((packed)) CPUARCH;
    struct {
        uint8_t id;
        uint8_t size;
        ENDIAN endian;
    }__attribute__((packed)) ENDIAN;
    struct {
        uint8_t id;
        uint8_t size;
        uint8_t value;
    }__attribute__((packed)) BITSi;
    struct {
        uint8_t id;
        uint8_t size;
        phys_t start;
        phys_t end;
        PERM permission;
    }__attribute__((packed)) REGION;
};
extern uint8_t mmap;

PMM_MMAP::PMM_MMAP(): PMM(0x1000) {
    MMAP &header = *((MMAP*)&mmap);
    if((header.magic[0] != 'M')||(header.magic[1] != 'M')||(header.magic[2] != 'A')||(header.magic[3] != 'P'))
        for(;;); //This is not a mmap
#ifdef __BIG_ENDIAN__
    header.length = __builtin_bswap32(header.length);
#endif
    uint8_t *off=(uint8_t*)(&header+1);
    for(uint32_t i=0;i<header.length;i++) {
        MMAPTag &tag = *((MMAPTag*)off);
        off+=tag.size+2;
        switch(tag.id) {
            case 0:
            case 1:
                break; //Ignored for now
            case 2: //CPUARCH
                if(tag.CPUARCH.value !=
#ifdef __i386__
                        CPUARCH::X86
#else
#ifdef __x86_64__
                        CPUARCH::X86_64
#else
                        CPUARCH::ARM
#endif
#endif
                    )
                    for(;;);
                break;
            case 3:
                hostEndian = tag.ENDIAN.endian ==
#ifdef __BIG_ENDIAN__
                    ENDIAN::BIG
#else
                    ENDIAN::LITTLE
#endif
                    ;
                break;
            case 4:
                bits = tag.BITSi.value;
                break;
            case 5:
                if(tag.REGION.permission != PERM::RWX)
                    break;
                if(tag.REGION.start < lowest_page)
                    lowest_page = tag.REGION.start;
                if(tag.REGION.end > highest_page)
                    highest_page = tag.REGION.end;
                break;
            default:
                for(;;);
        }
    }
    fill();
}
PMM_MMAP::~PMM_MMAP() {}
auto PMM_MMAP::isFree(phys_t addr) -> bool {
    MMAP &header = *((MMAP*)&mmap);
    uint8_t *off=(uint8_t*)(&header+1);
    for(uint32_t i=0;i<header.length;i++) {
        MMAPTag &tag = *((MMAPTag*)off);
        off+=tag.size+2;
        if(tag.id != 5)
            continue;
        if((tag.REGION.start > addr) || (tag.REGION.end <= addr))
            continue;
        if(tag.REGION.permission == PERM::RWX)
            return PMM::isFree(addr);
    }
    return false;
}
