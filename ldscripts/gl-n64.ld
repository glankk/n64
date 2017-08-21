/* gl-n64.ld
 * -glank
 */

OUTPUT_FORMAT("elf32-bigmips", "elf32-bigmips", "elf32-littlemips")
OUTPUT_ARCH(mips)
ENTRY(_start)
SECTIONS
{
  . = start ;

  .text :
  {
    *(.start .start.*)
    *(.init)
    *(.fini)
    *(.text .text.*)
    *(.gnu.linkonce.t.*)
  }

  .eh_frame_hdr :
  {
    *(.eh_frame_hdr)
    *(.eh_frame_entry .eh_frame_entry.*)
  }
  .eh_frame :
  {
    KEEP(*(.eh_frame .eh_frame.*))
  }
  .gcc_except_table :
  {
    *(.gcc_except_table .gcc_except_table.*)
  }

  .rodata :
  {
    *(.rdata .rdata.*)
    *(.rodata .rodata.*)
    *(.gnu.linkonce.r.*)
  }

  .ctors :
  {
    __CTOR_LIST__ = . ;
    KEEP(*(.ctors))
    __CTOR_END__ = . ;
  }
  .dtors :
  {
    __DTOR_LIST__ = . ;
    KEEP(*(.dtors))
    __DTOR_END__ = . ;
  }

  .data :
  {
    __RESOURCE_LIST__ = . ;
    KEEP(*(.data.resource_table))
    __RESOURCE_END__ = . ;
    KEEP(*(.data.resource_data))
    *(.data .data.*)
    *(.gnu.linkonce.d.*)
  }
  .sdata ALIGN(16) :
  {
    _gp = . ;
    *(.sdata .sdata.*)
    *(.gnu.linkonce.s.*)
  }

  .lit8 :
  {
    *(.lit8)
  }
  .lit4 :
  {
    *(.lit4)
  }

  __bss_start = . ;
  .sbss (NOLOAD) :
  {
    *(.sbss .sbss.*)
    *(.gnu.linkonce.sb.*)
    *(.scommon)
  }
  .bss (NOLOAD) :
  {
    *(.bss .bss.*)
    *(.gnu.linkonce.b.*)
    *(COMMON)
  }
  __bss_end = . ;

  .stack ALIGN(8) (NOLOAD) :
  {
    *(.stack .stack.*)
  }

  end = ALIGN(8) ;

  /DISCARD/ :
  {
    *(.*)
  }
}
