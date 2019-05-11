if test -n "$ULTRA_BASE_EM_FILE" ; then
  source_em "${srcdir}/emultempl/${ULTRA_BASE_EM_FILE}.em"
fi

fragment <<EOF

static int force_dynamic;

static void
ultra_after_open (void)
{
  gldelf32ebmipultra_after_open ();

  if (force_dynamic
      && link_info.input_bfds
      && bfd_get_flavour (link_info.output_bfd) == bfd_target_elf_flavour
      && !_bfd_elf_link_create_dynamic_sections (link_info.input_bfds,
                                                 &link_info))
    einfo (_("%X%P: cannot create dynamic sections %E\n"));

  if (!force_dynamic
      && !bfd_link_pic (&link_info)
      && bfd_get_flavour (link_info.output_bfd) == bfd_target_elf_flavour
      && elf_hash_table (&link_info)->dynamic_sections_created)
    einfo (_("%X%P: dynamic sections created in non-dynamic link\n"));
}

EOF

PARSE_AND_LIST_PROLOGUE=$PARSE_AND_LIST_PROLOGUE'
enum {
  OPTION_FORCE_DYNAMIC = 501
};
'

PARSE_AND_LIST_LONGOPTS=$PARSE_AND_LIST_LONGOPTS'
  {"force-dynamic", no_argument, NULL, OPTION_FORCE_DYNAMIC},
'

PARSE_AND_LIST_OPTIONS=$PARSE_AND_LIST_OPTIONS'
  fprintf (file, _("\
  --force-dynamic             Always create dynamic sections\n"));
'

PARSE_AND_LIST_ARGS_CASES=$PARSE_AND_LIST_ARGS_CASES'
    case OPTION_FORCE_DYNAMIC:
      force_dynamic = 1;
      break;
'

for override in after_open; do
  var="LDEMUL_`echo ${override} | tr 'abcdefghijklmnopqrstuvwxyz' 'ABCDEFGHIJKLMNOPQRSTUVWXYZ'`"
  eval value=\$${var}
  if test "${value}" = "gld${EMULATION_NAME}_${override}"; then
    fragment <<EOF
#define ${value} ultra_${override}
EOF
  else
    eval $var=ultra_${override}
  fi
done
