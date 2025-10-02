MEMORY {
  sram  : ORIGIN = 0x0f000000, LENGTH = 8K
  mrom  : ORIGIN = 0x20000000, LENGTH = 4K
  flash : ORIGIN = 0x30000000, LENGTH = 16M
  psram : ORIGIN = 0x80000000, LENGTH = 4M
  sdram : ORIGIN = 0xa0000000, LENGTH = 64M
}

SECTIONS {
  .data.extra : {
    /* section information for finsh shell */
    __fsymtab_start = .;
    KEEP(*(FSymTab))
    __fsymtab_end = .;
    . = ALIGN(8);
    __vsymtab_start = .;
    KEEP(*(VSymTab))
    __vsymtab_end = .;
    . = ALIGN(8);

    /* section information for initial. */
    . = ALIGN(8);
    __rt_init_start = .;
    KEEP(*(SORT(.rti_fn*)))
    __rt_init_end = .;
    . = ALIGN(8);

    __rt_utest_tc_tab_start = .;
    KEEP(*(UtestTcTab))
    __rt_utest_tc_tab_end = .;

    . = ALIGN(8);
    __am_apps_data_start = .;
    *(__am_apps.data*)
    *(__am_apps.sdata*)
    __am_apps_data_end = .;
    . = ALIGN(8);
  } > sdram AT> flash
  _data_extra_start_VMA = ADDR(.data.extra);
  _data_extra_end_VMA = _data_extra_start_VMA + SIZEOF(.data.extra);
  _data_extra_start_LMA = LOADADDR(.data.extra);
  _data_extra_end_LMA = _data_extra_start_LMA + SIZEOF(.data.extra);
}
INSERT AFTER .data;

SECTIONS {
  .bss.extra (NOLOAD) : {
    . = ALIGN(8);
    __am_apps_bss_start = .;
    *(__am_apps.bss*)
    *(__am_apps.sbss*)
    *(__am_apps.scommon*)
    __am_apps_bss_end = .;
    . = ALIGN(8);
  } > sdram
  _bss_extra_start = ADDR(.bss.extra);
  _bss_extra_end = _bss_extra_start + SIZEOF(.bss.extra);
}
INSERT AFTER .bss;