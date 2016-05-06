/* gl-n64-debug.ld
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

  end = ALIGN(8) ;

  .debug 0 :
  {
    *(.debug)
  }
  .line 0 :
  {
    *(.line)
  }
  .debug_srcinfo 0 :
  {
    *(.debug_srcinfo)
  }
  .debug_sfnames 0 :
  {
    *(.debug_sfnames)
  }
  .debug_aranges 0 :
  {
    *(.debug_aranges)
  }
  .debug_pubnames 0 :
  {
    *(.debug_pubnames)
  }
  .debug_info 0 :
  {
    *(.debug_info)
  }
  .debug_abbrev 0 :
  {
    *(.debug_abbrev)
  }
  .debug_line 0 :
  {
    *(.debug_line)
  }
  .debug_frame 0 :
  {
    *(.debug_frame)
  }
  .debug_str 0 :
  {
    *(.debug_str)
  }
  .debug_loc 0 :
  {
    *(.debug_loc)
  }
  .debug_macinfo 0 :
  {
    *(.debug_macinfo)
  }
  .debug_ranges 0 :
  {
    *(.debug_ranges)
  }
  .debug_weaknames 0 :
  {
    *(.debug_weaknames)
  }
  .debug_funcnames 0 :
  {
    *(.debug_funcnames)
  }
  .debug_typenames 0 :
  {
    *(.debug_typenames)
  }
  .debug_varnames 0 :
  {
    *(.debug_varnames)
  }

  /DISCARD/ :
  {
    *(.*)
  }
}
