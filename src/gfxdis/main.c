#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <vector/vector.h>
#include "gfxdis.h"
#include <n64.h>

#if defined(F3D_GBI)
# if defined(F3D_BETA)
static const char *prog_name = "gfxdis.f3db";
# else
static const char *prog_name = "gfxdis.f3d";
# endif
#elif defined(F3DEX_GBI)
# if defined(F3D_BETA)
static const char *prog_name = "gfxdis.f3dexb";
# else
static const char *prog_name = "gfxdis.f3dex";
# endif
#elif defined(F3DEX_GBI_2)
static const char *prog_name = "gfxdis.f3dex2";
#endif

static int usage(void)
{
  fprintf(stderr,
          "gfxdis-0.2: display list disassembler\n"
          "written by: glank\n"
          "build date: " __TIME__ ", " __DATE__ "\n"
          "usage:\n"
          "  %s [-i] [-n <max>] [-a <offset>] <file>\n"
          "  %s -d <data>...\n"
          "options:\n"
          "  -i            do not stop at unrecognized or malformed "
          "instructions\n"
          "  -n <max>      disassemble at most <max> instructions\n"
          "  -a <offset>   start disassembling at <offset> in file\n"
          "  <file>        file to disassemble, '-' for stdin\n"
          "  -d            disassemble hexadecimal byte codes from the "
          "command line\n",
          prog_name, prog_name);
  return -1;
}

static int parse_number(const char *str, int base, int *num)
{
  int sign = 1;
  if (base < 0) {
    if (str[0] == '+')
      ++str;
    else if (str[0] == '-') {
      sign = -1;
      ++str;
    }
    base = -base;
  }
  if (base <= 1) {
    if (strncmp(str, "0x", 2) == 0 || strncmp(str, "0X", 2) == 0) {
      base = 16;
      str += 2;
    }
    else if (strncmp(str, "0b", 2) == 0 || strncmp(str, "0B", 2) == 0) {
      base = 2;
      str += 2;
    }
    else if (str[0] == '0' && str[1] != 0) {
      base = 8;
      ++str;
    }
    else
      base = 10;
  }
  int v = 0;
  do {
    int c = *str++;
    if (c >= '0' && c <= '9')
      c = c - '0';
    else if (c >= 'a' && c <= 'f')
      c = c - 'a' + 10;
    else if (c >= 'A' && c <= 'F')
      c = c - 'A' + 10;
    else
      return -1;
    if (c >= base)
      return -1;
    v = v * base + c;
  } while (*str);
  v = v * sign;
  *num = v;
  return 0;
}

static int do_dis(void *buf, int max, _Bool dis_invd)
{
  struct vector insn_vect;
  int result = gfx_dis(&insn_vect, buf, max, dis_invd);
  printf("{\n");
  for (int i = 0; i < insn_vect.size; ++i) {
    char s[1024];
    gfx_insn_str(vector_at(&insn_vect, i), s);
    printf("  %s,\n", s);
  }
  vector_destroy(&insn_vect);
  printf("}\n");
  if (result == -1)
    fprintf(stderr, "%s: out of memory\n", prog_name);
  return result;
}

static int from_file(int argc, char *argv[])
{
  int result = -1;

  const char *opt_i = NULL;
  const char *opt_n = NULL;
  const char *opt_a = NULL;
  const char *opt_file = NULL;
  for (int i = 1; i < argc; ++i) {
    const char **p_opt = &opt_file;
    if (strcmp(argv[i], "-i") == 0)
      p_opt = &opt_i;
    else if (strcmp(argv[i], "-n") == 0) {
      ++i;
      p_opt = &opt_n;
    }
    else if (strcmp(argv[i], "-a") == 0) {
      ++i;
      p_opt = &opt_a;
    }
    else {
      p_opt = &opt_file;
      if (*p_opt)
        return usage();
    }
    if (i >= argc)
      return usage();
    else
      *p_opt = argv[i];
  }

  int max = -1;
  int offset = 0;
  if ((opt_n && parse_number(opt_n, 1, &max)) ||
      (opt_a && parse_number(opt_a, 1, &offset)) ||
      !opt_file)
  {
    return usage();
  }

  struct vector gfx_v;
  vector_init(&gfx_v, sizeof(Gfx));

  FILE *f = NULL;
  if (strcmp(opt_file, "-") == 0)
    f = freopen(NULL, "rb", stdin);
  else
    f = fopen(opt_file, "rb");
  if (!f) {
    perror(prog_name);
    goto exit;
  }

  if (opt_a && fseek(f, offset, SEEK_SET)) {
    perror(prog_name);
    goto exit;
  }

  while (max < 0 || gfx_v.size < max) {
    Gfx gfx;
    if (fread(&gfx, sizeof(Gfx), 1, f) != 1) {
      if (ferror(f)) {
        perror(prog_name);
        goto exit;
      }
      else
        break;
    }
    if (!vector_push_back(&gfx_v, 1, &gfx)) {
      fprintf(stderr, "%s: out of memory\n", prog_name);
      goto exit;
    }
  }
  int n_gfx = gfx_v.size * gfx_v.element_size / sizeof(Gfx);
  result = do_dis(gfx_v.begin, n_gfx, opt_i);

exit:
  vector_destroy(&gfx_v);
  if (f && strcmp(opt_file, "-") != 0)
    fclose(f);
  return result;
}

static int from_line(int argc, char *argv[])
{
  int result = -1;

  struct vector gfx_v;
  vector_init(&gfx_v, sizeof(unsigned char));

  for (int i = 2; i < argc; ++i) {
    char *p = argv[i];
    while (*p) {
      unsigned char byte = 0;
      for (int j = 0; j < 2 && *p; ++j) {
        int c = *p++;
        if (c >= '0' && c <= '9')
          c = c - '0';
        else if (c >= 'a' && c <= 'f')
          c = c - 'a' + 10;
        else if (c >= 'A' && c <= 'F')
          c = c - 'A' + 10;
        else {
          fprintf(stderr, "%s: invalid input data\n", prog_name);
          goto exit;
        }
        byte = byte * 16 + c;
      }
      if (!vector_push_back(&gfx_v, 1, &byte)) {
        fprintf(stderr, "%s: out of memory\n", prog_name);
        goto exit;
      }
    }
  }
  int n_gfx = gfx_v.size * gfx_v.element_size / sizeof(Gfx);
  result = do_dis(gfx_v.begin, n_gfx, 1);

exit:
  vector_destroy(&gfx_v);
  return result;
}

int main(int argc, char *argv[])
{
  if (argc >= 2 && strcmp(argv[1], "-d") == 0)
    return from_line(argc, argv);
  else
    return from_file(argc, argv);
}

#include <vector/vector.c>
