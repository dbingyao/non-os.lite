/* Entry Point */
ENTRY(_stext)

/* Specify the memory areas */
MEMORY
{
  /*RAM (xrw)       : ORIGIN = 0x00000000, LENGTH = 0x10000000 *//*256M*/
  RAM (xrw)       : ORIGIN = 0x00000000, LENGTH = 0x4000000 /*256M*/
}

/* define stack size and heap size here */
heap_size = 0x2000000; /*32M*/

/* 4G mem space needs 4096 entry and each entry is 4bytes.*/
pg_tb_size = 0x4000;

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
    .stack :
    {
	__stack_end = .;
	__stack_start = ORIGIN(RAM)+LENGTH(RAM)-0x4000;
	__stack_size = __stack_start - __stack_end;
	. = . + __stack_size;
    } > RAM

    . = ALIGN(4);
	.pg_tb :
	{
		__pg_tb_start = .;
		. = . + pg_tb_size;
		__pg_tb_end = .;
	} > RAM

    /* Remove information from the standard libraries */
    /DISCARD/ :
    {
        libgcc.a ( * )
    }

    .ARM.attributes 0 : { *(.ARM.attributes) }
}
