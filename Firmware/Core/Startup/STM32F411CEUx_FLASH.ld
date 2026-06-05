/**
 * @file    STM32F411CEUx_FLASH.ld
 * @brief   Linker script untuk STM32F411CE (Black Pill)
 * @author  Tim CuffnCode
 *
 * @details Memory layout:
 *          - Flash: 0x08000000, 512KB
 *          - SRAM:  0x20000000, 128KB
 */

/* ======================== Memory Regions ======================== */
MEMORY
{
    FLASH  (rx)  : ORIGIN = 0x08000000, LENGTH = 512K
    SRAM   (rwx) : ORIGIN = 0x20000000, LENGTH = 128K
}

/* ======================== Stack & Heap ======================== */
_estack       = ORIGIN(SRAM) + LENGTH(SRAM);  /* Top of SRAM */
_Min_Heap_Size = 0x200;                       /* 512 bytes */
_Min_Stack_Size = 0x400;                      /* 1KB */

/* ======================== Section Definitions ======================== */
SECTIONS
{
    /* === ISR Vector Table === */
    .isr_vector :
    {
        . = ALIGN(4);
        KEEP(*(.isr_vector))
        . = ALIGN(4);
    } > FLASH

    /* === Program Code === */
    .text :
    {
        . = ALIGN(4);
        *(.text)
        *(.text*)
        *(.glue_7)
        *(.glue_7t)
        *(.eh_frame)

        KEEP(*(.init))
        KEEP(*(.fini))

        . = ALIGN(4);
        _etext = .;
        __etext = .;
    } > FLASH

    /* === Read-Only Data === */
    .rodata :
    {
        . = ALIGN(4);
        *(.rodata)
        *(.rodata*)
        . = ALIGN(4);
    } > FLASH

    /* === ARM Constructor/Destructor === */
    .ARM.extab   :
    {
        . = ALIGN(4);
        *(.ARM.extab*)
        . = ALIGN(4);
    } > FLASH

    .ARM :
    {
        . = ALIGN(4);
        __exidx_start = .;
        *(.ARM.exidx*)
        __exidx_end = .;
        . = ALIGN(4);
    } > FLASH

    .preinit_array :
    {
        . = ALIGN(4);
        PROVIDE_HIDDEN(__preinit_array_start = .);
        KEEP(*(.preinit_array*))
        PROVIDE_HIDDEN(__preinit_array_end = .);
        . = ALIGN(4);
    } > FLASH

    .init_array :
    {
        . = ALIGN(4);
        PROVIDE_HIDDEN(__init_array_start = .);
        KEEP(*(SORT(.init_array.*)))
        KEEP(*(.init_array*))
        PROVIDE_HIDDEN(__init_array_end = .);
        . = ALIGN(4);
    } > FLASH

    .fini_array :
    {
        . = ALIGN(4);
        PROVIDE_HIDDEN(__fini_array_start = .);
        KEEP(*(SORT(.fini_array.*)))
        KEEP(*(.fini_array*))
        PROVIDE_HIDDEN(__fini_array_end = .);
        . = ALIGN(4);
    } > FLASH

    /* === Initialized Data (.data) === */
    __data_start__ = .;
    . = ALIGN(4);
    _sidata = .;

    .data : AT(__data_start__)
    {
        . = ALIGN(4);
        __data_start__ = .;
        _sdata = .;
        *(.data)
        *(.data*)
        . = ALIGN(4);
        __data_end__ = .;
        _edata = .;
    } > SRAM

    /* === Uninitialized Data (.bss) === */
    .bss :
    {
        . = ALIGN(4);
        __bss_start__ = .;
        _sbss = .;
        *(.bss)
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        __bss_end__ = .;
        _ebss = .;
    } > SRAM

    /* === Heap === */
    ._user_heap_stack :
    {
        . = ALIGN(8);
        PROVIDE(end = .);
        PROVIDE(_end = .);
        . = . + _Min_Heap_Size;
        . = . + _Min_Stack_Size;
        . = ALIGN(8);
    } > SRAM

    /* === Discard === */
    /DISCARD/ :
    {
        libc.a(*)     /* Discard unused libc */
        libm.a(*)     /* Discard unused libm */
        libgcc.a(*)   /* Discard unused libgcc */
    }
}
