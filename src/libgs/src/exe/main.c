#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#define MIPS_ASM_BIG_ENDIAN
#define MIPS_ASM_CONCISE
#include <mips.h>
#include <vector/vector.h>
#include "../lib/libgs.h"
#include "../lib/mos_io.h"
#include "../lib/gs_io.h"
#include "ops.h"
#include "progress.h"

#define NO_DETAILS "this option has no detailed information"
#define HELP_NOTE  "use `gs -h` or `gs --help` for usage instructions"

#define ARRAY_SIZE(A) (sizeof(A) / sizeof(*(A)))

enum option_error
{
  OPTION_SUCCESS,
  OPTION_ERROR,
  OPTION_ERROR_UNRECOGNIZED,
  OPTION_ERROR_UNEXPECTED,
  OPTION_ERROR_END,
  OPTION_ERROR_FOPEN,
};

struct option
{
  const char *names[3];
  enum option_error (*handler)(int argc, const char *argv[], int *argp);
  const char *brief;
  const char *details;
};

static enum option_error option_handler_version(int argc, const char *argv[],
                                                int *argp);
static enum option_error option_handler_help(int argc, const char *argv[],
                                             int *argp);
static enum option_error option_handler_write(int argc, const char *argv[],
                                              int *argp);
static enum option_error option_handler_unhook(int argc, const char *argv[],
                                               int *argp);

static const struct option option_list[] =
{
  {
    {"-v", "--version"},
    option_handler_version,
    "  -v, --version                 show version information",
    NO_DETAILS
  },
  {
    {"-h", "--help"},
    option_handler_help,
    "  -h, --help [option]           show general or option-specific help text",
    "use `-h [option]` or `--help [option]` to show detailed information "
    "about an option\n"
    "example usage (shows this text): `gs --help -h`"
  },
  {
    {"-w", "--write"},
    option_handler_write,
    "  -w, --write <format>\n"
    "  (address) [chunk size] <file> write data to ram",
    "format can be `text` for gameshark codes in text form, "
    "or `bin` for raw data\n"
    "when writing raw data, the address must be specified, "
    "and for text it must not\n"
    "address is specified in decimal, or hexadecimal if preceded by `0x`\n"
    "raw data is uploaded all at once by default, but chunked upload "
    "can be selected by specifying a chunk size (not applicable for text)\n"
    "file can be the path of the file which contains the data, or `-` "
    "to read data from stdin"
  },
  {
    {"-u", "--unhook"},
    option_handler_unhook,
    "  -u, --unhook [address]        unhook gameshark routines",
    "useful when gameshark functionality is no longer needed, "
    "and the presence of the gameshark causes the game to become unstable\n"
    "a routine is uploaded together with a hook that causes the gameshark "
    "to call said routine, which then disables gameshark traps and restores "
    "the game's original exception handler\n"
    "this prevents any code injected by the gameshark from running, "
    "which means that gameshark codes, menus, and sharklink communication "
    "will be disabled\n"
    "the address of the insurgent routine defaults to 0xa07e0000 if no "
    "address is specified"
  },
};


static int option_find_index(const char *search_name)
{
  for (int i = 0; i < ARRAY_SIZE(option_list); ++i) {
    for (int j = 0; j < ARRAY_SIZE(option_list[i].names); ++j) {
      const char *option_name = option_list[i].names[j];
      if (option_name && strcmp(search_name, option_name) == 0)
        return i;
    }
  }
  return -1;
}

static const char *option_get_next(int argc, const char *argv[], int *argp)
{
  if (++*argp >= argc)
    return NULL;
  return argv[*argp];
}

static const char *option_peek_next(int argc, const char *argv[], int *argp)
{
  int argp_n = *argp + 1;
  if (argp_n >= argc)
    return NULL;
  return argv[argp_n];
}

static enum option_error option_handler_version(int argc, const char *argv[],
                                                int *argp)
{
  printf("libgs-0.1\n"
         "build date: " __TIME__ ", " __DATE__ "\n"
         "written by: ppcasm, hcs64, glank\n"
         HELP_NOTE "\n");
  return OPTION_SUCCESS;
}

static enum option_error option_handler_help(int argc, const char *argv[],
                                             int *argp)
{
  const char *next_option = option_get_next(argc, argv, argp);
  if (next_option) {
    int option_index = option_find_index(next_option);
    if (option_index == -1) {
      printf("no help text available for `%s`, unrecognized option\n",
             next_option);
      return OPTION_ERROR;
    }
    printf("%s\n%s\n", option_list[option_index].brief,
           option_list[option_index].details);
  }
  else {
    printf("invocation: `gs [options...]`\n"
           "options are applied sequentially from left to right\n"
           "operations are only affected by options that precede them\n"
           "list of options:\n");
    for (int i = 0; i < ARRAY_SIZE(option_list); ++i)
      printf("%s\n", option_list[i].brief);
  }
  return OPTION_SUCCESS;
}

static enum option_error option_handler_write(int argc, const char *argv[],
                                              int *argp)
{
  enum option_error e = OPTION_SUCCESS;
  enum
  {
    FORMAT_TEXT,
    FORMAT_BINARY,
  } format;
  FILE *file = NULL;
  int close_file;
  uint32_t address;
  uint32_t data_size = 0;
  uint32_t data_chunk_size = 0;
  struct vector code_vector;
  vector_init(&code_vector, sizeof(struct gs_code));
  struct vector data_vector;
  vector_init(&data_vector, 1);

  const char *format_name = option_get_next(argc, argv, argp);
  if (!format_name)
    return OPTION_ERROR_END;
  if (strcmp(format_name, "text") == 0)
    format = FORMAT_TEXT;
  else if (strcmp(format_name, "bin") == 0)
    format = FORMAT_BINARY;
  else
    return OPTION_ERROR_UNEXPECTED;

  if (format == FORMAT_BINARY) {
    const char *next_option = option_get_next(argc, argv, argp);
    if (!next_option)
      return OPTION_ERROR_END;
    if (sscanf(next_option, "0x%" SCNx32, &address) != 1)
      if (sscanf(next_option, "%" SCNu32, &address) != 1)
        return OPTION_ERROR_UNEXPECTED;
    next_option = option_peek_next(argc, argv, argp);
    if (next_option && (sscanf(next_option, "0x%" SCNx32, &data_chunk_size) == 1 ||
                        sscanf(next_option, "%" SCNu32, &data_chunk_size) == 1))
      ++*argp;
  }

  const char *file_name = option_get_next(argc, argv, argp);
  if (!file_name)
    return OPTION_ERROR_END;
  if (strcmp(file_name, "-") == 0) {
    file = stdin;
    close_file = 0;
  }
  else {
    file = fopen(file_name, "r");
    if (!file)
      return OPTION_ERROR_FOPEN;
    close_file = 1;
  }
  file = freopen(NULL, format == FORMAT_TEXT ? "r" : "rb", file);

  if (format == FORMAT_TEXT) {
    while (!feof(file)) {
      struct gs_code code;
      if (fscanf(file, " %8" SCNx32 " %4" SCNx16,
                 &code.address, &code.value) != 2)
        break;
      if ((code.address & 0xFF000000) == 0x80000000)
        data_size += 1;
      else if ((code.address & 0xFF000000) == 0x81000000)
        data_size += 2;
      else
        continue;
      vector_push_back(&code_vector, 1, &code);
    }
  }
  else if (format == FORMAT_BINARY) {
    while (!feof(file)) {
      const size_t chunk_max_size = 1024;
      char chunk[chunk_max_size];
      size_t chunk_size = fread(chunk, 1, chunk_max_size, file);
      vector_push_back(&data_vector, chunk_size, chunk);
      data_size += chunk_size;
      if (chunk_size != chunk_max_size)
        break;
    }
  }

  {
    printf("writing data to ram\n"
           "  file:         %s\n"
           "  format:       %s\n",
           file_name, format_name);
    if (format == FORMAT_BINARY)
      printf("  address:      0x%08" PRIx32 "\n", address);
    printf("  size:         %" PRIu32 "\n", data_size);
    if (data_chunk_size != 0)
      printf("  chunk size:   %u\n", data_chunk_size);

    enum gs_error gs_error_code;
    char gs_error_description[GS_ERROR_LENGTH];
    if (format == FORMAT_TEXT)
      write_codes(code_vector.begin, code_vector.size, data_size,
                  &gs_error_code, &gs_error_description);
    else if (format == FORMAT_BINARY)
      write_raw(address, data_vector.begin, data_size, data_chunk_size,
                &gs_error_code, &gs_error_description);
    if (gs_error_failed(gs_error_code)) {
      printf("%s\n", gs_error_description);
      e = OPTION_ERROR;
      goto exit;
    }
    printf("%" PRIu32 " bytes written successfully\n", data_size);
  }

exit:
  vector_destroy(&code_vector);
  vector_destroy(&data_vector);
  if (file) {
    file = freopen(NULL, "r", file);
    if (close_file)
      fclose(file);
  }
  return e;
}

static enum option_error option_handler_unhook(int argc, const char *argv[],
                                               int *argp)
{
  uint32_t address = 0xA07E0000;
  const char *next_option = option_peek_next(argc, argv, argp);
  if (next_option && (sscanf(next_option, "0x%" SCNx32, &address) == 1 ||
                      sscanf(next_option, "%" SCNu32, &address) == 1))
    ++*argp;
  printf("unhooking gameshark routines\n");
  printf("  insurgent address:  0x%08" PRIx32 "\n", address);
  enum gs_error gs_error_code;
  char gs_error_description[GS_ERROR_LENGTH];
  printf("uploading insurgent\n");
  uint32_t code[] =
  {
    /* disable gs watch breakpoints */
    MTC0(R0, MIPS_CP0_WATCHLO),
    /* restore original exception handler */
    LA(K0, 0xA0000120),
      LW(K1, 0x00, K0),
      SW(K1, 0x60, K0),
      ADDIU(K0, K0, 4),
      LA(K1, 0xA0000180),
      SLTU(K1, K0, K1),
      BNEZ(K1, -7 * 4),
      NOP,
    /* pass control to original exception handler */
    JR(K0),
    NOP,
  };
  write_raw(address, &code, sizeof(code), 0,
            &gs_error_code, &gs_error_description);
  if (gs_error_failed(gs_error_code)) {
    printf("%s\n", gs_error_description);
    return OPTION_ERROR;
  }
  printf("uploading hook\n");
  uint32_t hook[] =
  {
    /* jump to insurgent */
    LA(K0, address),
    JR(K0),
    NOP,
  };
  write_raw(0xA07C5C00, &hook, sizeof(hook), 0,
            &gs_error_code, &gs_error_description);
  if (gs_error_failed(gs_error_code)) {
    printf("%s\n", gs_error_description);
    return OPTION_ERROR;
  }
  printf("done\n");
  return OPTION_SUCCESS;
}

static enum option_error parse_options(int argc, const char *argv[], int *argp)
{
  if (argc == 1)
    return option_list[0].handler(argc, argv, argp);
  while (++*argp < argc) {
    int option_index = option_find_index(argv[*argp]);
    if (option_index == -1)
      return OPTION_ERROR_UNRECOGNIZED;
    enum option_error e = option_list[option_index].handler(argc, argv, argp);
    if (e != OPTION_SUCCESS)
      return e;
  }
  return OPTION_SUCCESS;
}

int main(int argc, const char *argv[])
{
  int argp = 0;
  enum option_error e = parse_options(argc, argv, &argp);
  if (e == OPTION_ERROR_UNRECOGNIZED)
    printf("unrecognized option: `%s`\n"
           HELP_NOTE "\n",
           argv[argp]);
  else if (e == OPTION_ERROR_UNEXPECTED)
    printf("unexpected option string: `%s`\n"
           HELP_NOTE "\n",
           argv[argp]);
  else if (e == OPTION_ERROR_END)
    printf("unexpected end of option string\n"
           HELP_NOTE "\n");
  else if (e == OPTION_ERROR_FOPEN)
    printf("failed to open file: `%s`\n", argv[argp]);
  return e;
}
