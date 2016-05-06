#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>
#include <fcntl.h>

int main(int argc, char *argv[])
{
  if (argc < 2) {
    printf("luapatch-0.1\n"
           "build date: " __TIME__ ", " __DATE__ "\n"
           "written by: glank\n"
           "invocation: luapatch <outfile> "
           "[{-text <infile>}|{-bin <address> <infile>}]...\n"
           "`-text` specifies a file containing a list of gameshark codes\n"
           "`-bin` specifies a raw data file "
           "(address must be provided, in hexadecimal)\n"
           "use `-` as filename for stdin / stdout\n");
    return -1;
  }
  FILE *fo;
  int close_fo;
  if (strcmp(argv[1], "-") == 0) {
    fo = stdout;
    close_fo = 0;
  }
  else {
    fo = fopen(argv[1], "w");
    close_fo = 1;
  }
  if (!fo) {
    printf("failed to open output file: `%s`\n", argv[1]);
    return -2;
  }
  int e = 0;
  fprintf(fo, "return\n{\n");
  for (int i = 2; i < argc; ++i) {
    FILE *fi;
    int close_fi;
    if (strcmp(argv[i], "-text") == 0) {
      if (i + 1 >= argc) {
        printf("unexpected end of option string\n");
        e = -3;
        goto exit;
      }
      ++i;
      if (strcmp(argv[i], "-") == 0) {
        fi = stdin;
        close_fi = 0;
        _setmode(_fileno(fi), _O_TEXT);
      }
      else {
        fi = fopen(argv[i], "r");
        close_fi = 1;
      }
      if (!fi) {
        printf("failed to open input file: `%s`\n", argv[i]);
        e = -4;
        goto exit;
      }
      while (!feof(fi)) {
        uint32_t address;
        uint16_t value;
        if (fscanf(fi, " %8" SCNx32 " %4" SCNx16, &address, &value) != 2)
          break;
        if ((address & 0xFF000000) == 0x80000000)
          fprintf(fo, "  {0x%08" PRIx32 ", {0x%02" PRIx8
                      "}},\n",
                  address & 0x00FFFFFF,
                  (value >> 0) & 0xFF);
        else if ((address & 0xFF000000) == 0x81000000)
          fprintf(fo, "  {0x%08" PRIx32 ", {0x%02" PRIx8
                      ", 0x%02" PRIx8 "}},\n",
                  address & 0x00FFFFFF,
                  (value >> 8) & 0xFF, (value >> 0) & 0xFF);
        else
          continue;
      }
    }
    else if (strcmp(argv[i], "-bin") == 0) {
      if (i + 2 >= argc) {
        printf("unexpected end of option string\n");
        e = -3;
        goto exit;
      }
      ++i;
      uint32_t address;
      if (sscanf(argv[i], "%" SCNx32, &address) != 1) {
        printf("unexpected option: `%s`\n", argv[i]);
        e = -5;
        goto exit;
      }
      ++i;
      if (strcmp(argv[i], "-") == 0) {
        fi = stdin;
        close_fi = 0;
        _setmode(_fileno(fi), _O_BINARY);
      }
      else {
        fi = fopen(argv[i], "rb");
        close_fi = 1;
      }
      if (!fi) {
        printf("failed to open input file: `%s`\n", argv[i]);
        e = -4;
        goto exit;
      }
      fprintf(fo, "  {0x%08" PRIx32 ",\n    {", address & 0x00FFFFFF);
      for (int n = 0; ; n = (n + 1) % 16) {
        int c = fgetc(fi);
        if (feof(fi))
          break;
        if (n == 0)
          fprintf(fo, "\n     ");
        fprintf(fo, " 0x%02" PRIx8 ",", c);
      }
      fprintf(fo, "\n    }\n  },\n");
    }
    else {
      printf("unexpected option: `%s`\n", argv[i]);
      e = -5;
      goto exit;
    }
    if (close_fi)
      fclose(fi);
  }
  fprintf(fo, "}\n");
exit:
  if (close_fo) {
    fclose(fo);
    if (e)
      remove(argv[1]);
  }
  return e;
}
