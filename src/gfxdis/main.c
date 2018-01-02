#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <vector/vector.h>
#include "gfxdis.h"
#include <n64.h>

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
    fprintf(stderr, "error: out of memory\n");
  return result;
}

int main(int argc, char *argv[])
{
  if (argc < 2) {
    fprintf(stderr, "gfxdis-0.1: f3dex2 display list disassembler\n"
                    "written by: glank\n"
                    "build date: " __TIME__ ", " __DATE__ "\n"
                    "options (order matters):\n"
                    "  -i           do not stop at unrecognized or "
                    "malformed instructions\n"
                    "  -n <max>     disassemble at most <max> instructions\n"
                    "  -a <offset>  start disassembling at <offset> in file\n"
                    "  -d <data>    disassemble hex code data "
                    "from command line\n"
                    "  -f <file>    disassemble <file>\n");
    return 0;
  }
  long offset = 0;
  long max = 0;
  _Bool dis_invd = 0;
  for (int i = 1; i < argc; ++i) {
    if (strcmp(argv[i], "-i") == 0)
      dis_invd = 1;
    else if (strcmp(argv[i], "-n") == 0) {
      if (i + 1 >= argc) {
        fprintf(stderr, "error: no max value specified\n");
        return -1;
      }
      char *endptr;
      max = strtol(argv[++i], &endptr, 0);
      if (*endptr != 0) {
        fprintf(stderr, "error: invalid max value\n");
        return -1;
      }
    }
    else if (strcmp(argv[i], "-a") == 0) {
      if (i + 1 >= argc) {
        fprintf(stderr, "error: no offset specified\n");
        return -1;
      }
      char *endptr;
      offset = strtol(argv[++i], &endptr, 0);
      if (*endptr != 0) {
        fprintf(stderr, "error: invalid offset\n");
        return -1;
      }
    }
    else if (strcmp(argv[i], "-d") == 0) {
      if (i + 1 >= argc) {
        fprintf(stderr, "error: no input data\n");
        return -1;
      }
      struct vector d_vect;
      vector_init(&d_vect, sizeof(char));
      while (i + 1 < argc && argv[i + 1][0] != '-') {
        char *p = argv[++i];
        int part = strlen(p) % 2;
        char v = 0;
        while (*p) {
          char c = *p++;
          if (c >= '0' && c <= '9')
            c -= '0';
          else if (c >= 'A' && c <= 'F')
            c -= 'A' - 10;
          else if (c >= 'a' && c <= 'f')
            c -= 'a' - 10;
          else {
            fprintf(stderr, "error: invalid input data\n");
            return -1;
          }
          v |= (c << (4 * (1 - part)));
          if (++part == 2) {
            if (!vector_push_back(&d_vect, 1, &v)) {
              fprintf(stderr, "error: out of memory\n");
              return -1;
            }
            part = 0;
            v = 0;
          }
        }
      }
      if (d_vect.size == 0) {
        fprintf(stderr, "error: no input data\n");
        return -1;
      }
      long fmax = d_vect.size / sizeof(Gfx);
      if (fmax == 0) {
        vector_destroy(&d_vect);
        if (!dis_invd)
          return 1;
      }
      else {
        int result = do_dis(d_vect.begin, fmax < max || max <= 0 ? fmax : max,
                            dis_invd);
        vector_destroy(&d_vect);
        if (result)
          return result;
      }
    }
    else if (strcmp(argv[i], "-f") == 0) {
      if (i + 1 >= argc) {
        fprintf(stderr, "error: no input file specified\n");
        return -1;
      }
      FILE *f = fopen(argv[++i], "rb");
      if (!f) {
        fprintf(stderr, "error: failed to open `%s`\n", argv[i]);
        return -1;
      }
      long size;
      if (fseek(f, 0, SEEK_END) || (size = ftell(f)) == -1 ||
          fseek(f, 0, SEEK_SET))
      {
        fclose(f);
        fprintf(stderr, "error: file input failed\n");
        return -1;
      }
      if (offset < 0 || offset > size) {
        fclose(f);
        fprintf(stderr, "error: invalid offset\n");
        return -1;
      }
      long fmax = (size - offset) / sizeof(Gfx);
      if (fmax == 0) {
        fclose(f);
        if (!dis_invd)
          return 1;
      }
      else {
        char *buf = malloc(size);
        if (!buf) {
          fclose(f);
          fprintf(stderr, "error: out of memory\n");
          return -1;
        }
        size_t n = fread(buf, 1, size, f);
        fclose(f);
        if (n != size) {
          fprintf(stderr, "error: file input failed\n");
          return -1;
        }
        int result = do_dis(&buf[offset], fmax < max || max <= 0 ? fmax : max,
                            dis_invd);
        free(buf);
        if (result)
          return result;
      }
    }
    else {
      fprintf(stderr, "error: unrecognized option: `%s`\n", argv[i]);
      return -1;
    }
  }
  return 0;
}

#include <vector/vector.c>
