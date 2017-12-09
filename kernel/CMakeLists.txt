ENABLE_LANGUAGE(C CXX ASM-ATT)
FUNCTION(LOAD_PROFILE ISA PLATFORM)
    # Obtain sources for the ISA
    FILE(GLOB ISA_SRCS arch/${ISA}/*.c arch/${ISA}/*.s arch/${ISA}/*.cpp)
    FILE(GLOB PLATFORM_SRCS arch/${ISA}/${PLATFORM}/*.c arch/${ISA}/${PLATFORM}/*.s arch/${ISA}/${PLATFORM}/*.cpp)

    # Load flags associated with ISA and Profile
    INCLUDE("arch/${ISA}/flags.cmake")
    INCLUDE("arch/${ISA}/${PLATFORM}/flags.cmake")

    # Now export our output variables
    SET(ISA_SRCS ${ISA_SRCS} PARENT_SCOPE)
    SET(PLATFORM_SRCS ${PLATFORM_SRCS} PARENT_SCOPE)
    SET(PLATFORM_LAYOUT "arch/${ISA}/${PLATFORM}/layout.ld" PARENT_SCOPE)

    # And specific flags
    SET(ISA_C_FLAGS ${ISA_C_FLAGS} PARENT_SCOPE)
    SET(ISA_CXX_FLAGS ${ISA_CXX_FLAGS} PARENT_SCOPE)
    SET(ISA_ASM_FLAGS ${ISA_ASM_FLAGS} PARENT_SCOPE)

    SET(PLATFORM_C_FLAGS ${PLATFORM_C_FLAGS} PARENT_SCOPE)
    SET(PLATFORM_CXX_FLAGS ${PLATFORM_CXX_FLAGS} PARENT_SCOPE)
    SET(PLATFORM_ASM_FLAGS ${PLATFORM_ASM_FLAGS} PARENT_SCOPE)

    SET(ISA_LINKER_FLAGS ${ISA_LINKER_FLAGS} PARENT_SCOPE)
    SET(PLATFORM_LINKER_FLAGS ${ISA_LINKER_FLAGS} PARENT_SCOPE)
ENDFUNCTION(LOAD_PROFILE)
FILE(GLOB_RECURSE GENERIC_SRCS src/*.c src/*.cpp ${DRIVER_SRCS})
LOAD_PROFILE(${ARCH} ${SYSTEM})
ADD_EXECUTABLE(kernel ${PLATFORM_SRCS} ${ISA_SRCS} ${GENERIC_SRCS})
SET(CMAKE_ASM-ATT_COMPILE_OBJECT 
    "<CMAKE_C_COMPILER> -g -x assembler-with-cpp -I../../ -I../../libk/include/ -I../../kernel/src/include/ ${ISA_ASM_FLAGS} ${PLATFORM_ASM_FLAGS} -c -o <OBJECT> <SOURCE>")
SET(CMAKE_C_FLAGS "-g --all-warnings -Werror -Wno-error=reorder -Wno-unused -Wno-unused-variable -fno-use-cxa-atexit -std=c11 -ffreestanding -ffunction-sections -fdata-sections -I../../ -I../../libk/include/ -I../../kernel/src/include/ ${ISA_C_FLAGS} ${PLATFORM_C_FLAGS}")
SET(CMAKE_CXX_FLAGS "-g --all-warnings -Werror -Wno-error=reorder -Wno-unused -Wno-unused-variable -fno-use-cxa-atexit -ffreestanding -fno-rtti -fno-exceptions -std=gnu++17 -ffunction-sections -fdata-sections -I../../ -I../../libk/include/ -I../../kernel/src/include/ ${ISA_CXX_FLAGS} ${PLATFORM_CXX_FLAGS}")
SET_TARGET_PROPERTIES(kernel PROPERTIES LINK_FLAGS 
    "-T ../../kernel/${PLATFORM_LAYOUT} -N ${ISA_LINKER_FLAGS} ${PLATFORM_LINKER_FLAGS} -nostdlib -nodefaultlibs -Wl,--gc-sections -ffreestanding")
TARGET_LINK_LIBRARIES(kernel gcc)
