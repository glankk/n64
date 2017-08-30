/* gl-n64.ld
 * -glank
 */

OUTPUT_FORMAT("elf32-bigmips", "elf32-bigmips", "elf32-littlemips")
OUTPUT_ARCH(mips)
ENTRY(_start)
SECTIONS
{
  PROVIDE(start = 0x80000400) ;
  . = start ;

  .text :
  {
    PROVIDE(_start = .) ;
    KEEP(*(SORT_NONE(.text.startup)))
    KEEP(*(SORT(.text.startup.*)))
    KEEP(*crt0.o(.text SORT(.text.*)))
    *(.text .text.*)
    *(.gnu.linkonce.t.*)
  }
  .init :
  {
    KEEP(*(SORT_NONE(.init)))
  }
  .fini :
  {
    KEEP(*(SORT_NONE(.fini)))
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
    PROVIDE(__CTOR_LIST__ = .) ;
    KEEP(*(.ctor_i))
    KEEP(*crtbegin.o(.ctors))
    KEEP(*crtbegin?.o(.ctors))
    KEEP(*(EXCLUDE_FILE(*crtend.o *crtend?.o) .ctors))
    KEEP(*(SORT(.ctors.*)))
    KEEP(*(.ctors))
    KEEP(*(.ctor_n))
  }
  .dtors :
  {
    PROVIDE(__DTOR_LIST__ = .) ;
    KEEP(*(.dtor_i))
    KEEP(*crtbegin.o(.dtors))
    KEEP(*crtbegin?.o(.dtors))
    KEEP(*(EXCLUDE_FILE(*crtend.o *crtend?.o) .dtors))
    KEEP(*(SORT(.dtors.*)))
    KEEP(*(.dtors))
    KEEP(*(.dtor_n))
  }

  .tm_clone_table :
  {
    *(.tm_clone_table)
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

  HIDDEN (_gp = ALIGN(16) + 0x7FF0);
  .sdata :
  {
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

  .debug_info 0 :
  {
    *(.debug_info .gnu.linkonce.wi.*)
  }
  .debug_abbrev 0 :
  {
    *(.debug_abbrev)
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
  .debug_line 0 :
  {
    *(.debug_line .debug_line.* .debug_line_end)
  }
  .debug_aranges 0 :
  {
    *(.debug_aranges)
  }
  .debug_ranges 0 :
  {
    *(.debug_ranges)
  }
  .debug_pubtypes 0 :
  {
    *(.debug_pubtypes)
  }
  .debug_pubnames 0 :
  {
    *(.debug_pubnames)
  }
  .debug_macinfo 0 :
  {
    *(.debug_macinfo)
  }
  .debug_macro 0 :
  {
    *(.debug_macro)
  }
  .debug_addr 0 :
  {
    *(.debug_addr)
  }

  /DISCARD/ :
  {
    *(.*)
  }
}
