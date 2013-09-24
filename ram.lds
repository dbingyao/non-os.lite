/* Entry Point */
ENTRY(_stext)
 
/* Specify the memory areas */
MEMORY
{
  /*RAM (xrw)       : ORIGIN = 0x00000000, LENGTH = 0x10000000 *//*256M*/
  RAM (xrw)       : ORIGIN = 0x00000000, LENGTH = 0x4000000 /*256M*/
}
 
/* define stack size and heap size here */
stack_size = 4096;
heap_size = 0x2000000; /*32M*/ 
 
/* define beginning and ending of stack */
__stack_start = ORIGIN(RAM)+LENGTH(RAM);
__stack_end = __stack_start - stack_size;
 
/* Define output sections */
SECTIONS
{
  /* The startup code goes first into RAM */
  .isr_vector :
  {
    . = ALIGN(4);
    KEEP(*(.isr_vector)) /* Startup code */
    . = ALIGN(4);
  } >RAM
 
  /* The program code and other data goes into RAM */
  .text :
  {
    . = ALIGN(4);
    *(.text)           /* .text sections (code) */
    *(.text*)          /* .text* sections (code) */
    *(.rodata)         /* .rodata sections (constants, strings, etc.) */
    *(.rodata*)        /* .rodata* sections (constants, strings, etc.) */
    *(.glue_7)         /* glue arm to thumb code */
    *(.glue_7t)        /* glue thumb to arm code */
    . = ALIGN(4);
    __text_end = .;        /* define a global symbols at end of code */
  } >RAM
 
   .ARM.extab   : { *(.ARM.extab* .gnu.linkonce.armextab.*) } >RAM
    .ARM : {
    __exidx_start = .;
      *(.ARM.exidx*)
      __exidx_end = .;
    } >RAM
 
  /* Initialized data sections goes into RAM, load LMA copy after code */
  .data :   
  {
    . = ALIGN(4);
    __data_start = .;  /* create a global symbol at data start */
    *(.data)           /* .data sections */
    *(.data*)          /* .data* sections */
 
    . = ALIGN(4);
    __data_end = .;        /* define a global symbol at data end */
  } >RAM
 
  /* Uninitialized data section */
  . = ALIGN(4);
  .bss :
  {
    /*  Used by the startup in order to initialize the .bss secion */
    _sbss = .;         /* define a global symbol at bss start */
    __bss_start = _sbss;
    *(.bss)
    *(.bss*)
    *(COMMON)
 
    . = ALIGN(4);
    _ebss = .;         /* define a global symbol at bss end */
    __bss_end = _ebss;
  } >RAM
 
    . = ALIGN(4);
    end = .;
    .heap :
    {
        __heap_start = .;
        . = . + heap_size;
    } > RAM
 
    . = ALIGN(4);
    . = __stack_end;
    .stack :
    {
        . = . + stack_size;
    } > RAM
 
    /* Remove information from the standard libraries */
    /DISCARD/ :
    {
        lib/libUtils.a ( * )
        libgcc.a ( * )
    }
 
    .ARM.attributes 0 : { *(.ARM.attributes) }
}
