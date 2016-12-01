#include <string.h>
#include <stdarg.h>
#include <jansson.h>
#include "texture.h"

struct param
{
  const char   *name;
  const char  **p_str;
};

static const char    *g_default_error_message[] =
{
  "success",
  "invalid parameter",
  "memory allocation failed",
  "file input error",
  "file output error",
  "data malformed",
};
static enum grc_error g_error_code;
static char           g_error_message[2048];

static struct grc_res_type g_res_types[] =
{
  {"texture", make_texture},
};

enum grc_error grc_set_error(enum grc_error error_code,
                             const char *format, ...)
{
  g_error_code = error_code;
  if (format) {
    va_list args;
    va_start(args, format);
    vsnprintf(g_error_message, sizeof(g_error_message), format, args);
    va_end(args);
  }
  else
    strcpy(g_error_message, g_default_error_message[error_code]);
  return g_error_code;
}

static enum grc_error parse_args(int argc, char *argv[],
                                 struct param params[], size_t no_params)
{
  for (int i = 1; i < argc; ++i) {
    _Bool match_found = 0;
    const char **def = NULL;
    for (size_t j = 0; j < no_params; ++j) {
      if (!params[j].name)
        def = params[j].p_str;
      else if (strcmp(argv[i], params[j].name) == 0) {
        match_found = 1;
        if (*params[j].p_str)
          return grc_set_error(GRC_ERROR_PARAM,
                               "multiple definition of option `%s`\n",
                               params[j].name);
        if (i + 1 >= argc)
          return grc_set_error(GRC_ERROR_PARAM,
                               "unexpected end of option string");
        *params[j].p_str = argv[++i];
        break;
      }
    }
    if (!match_found) {
      if (def && !*def)
        *def = argv[i];
      else
        return grc_set_error(GRC_ERROR_PARAM,
                             "unrecognized option `%s`\n", argv[i]);
    }
  }
  return grc_set_error(GRC_SUCCESS, NULL);
}

static enum grc_error parse_descriptor(const char *descriptor_file,
                                       const char *descriptor_name,
                                       json_t **j_root,
                                       json_t **j_descriptor,
                                       const char **resource_type,
                                       const char **resource_name)
{
  *j_root = json_load_file(descriptor_file, 0, NULL);
  if (!*j_root)
    return grc_set_error(GRC_ERROR_FILEIN, "failed to load `%s`\n",
                         descriptor_file);
  if (!json_is_object(*j_root))
    goto parse_error;
  *j_descriptor = json_object_get(*j_root, descriptor_name);
  if (*j_descriptor) {
    if (!json_is_object(*j_descriptor))
      goto parse_error;
    json_t *j_type = json_object_get(*j_descriptor, "type");
    if (j_type) {
      if (!json_is_string(j_type))
        goto parse_error;
      *resource_type = json_string_value(j_type);
    }
    json_t *j_name = json_object_get(*j_descriptor, "name");
    if (j_name) {
      if (!json_is_string(j_name))
        goto parse_error;
      *resource_name = json_string_value(j_name);
    }
  }
  return grc_set_error(GRC_SUCCESS, NULL);
parse_error:
  return grc_set_error(GRC_ERROR_DATA, "failed to parse `%s`\n",
                       descriptor_file);
}

int main(int argc, char *argv[])
{
  /* resources */
  json_t *j_root = NULL;
  /* version message */
  if (argc <= 1) {
    puts("grc-0.1\n"
         "build date: " __TIME__ ", " __DATE__ "\n"
         "written by: glank\n"
         "invocation: `grc <input-file> [-o <output-file>] "
         "[-d <descriptor-file>] [-n <descriptor-name>]`");
    grc_set_error(GRC_SUCCESS, NULL);
    goto exit;
  }
  const char *input_file      = NULL;
  const char *output_file     = NULL;
  const char *descriptor_file = NULL;
  const char *descriptor_name = NULL;
  {
    /* parse arguments */
    struct param params[] =
    {
      {NULL, &input_file},
      {"-o", &output_file},
      {"-d", &descriptor_file},
      {"-n", &descriptor_name},
    };
    parse_args(argc, argv, params, sizeof(params) / sizeof(*params));
    if (g_error_code)
      goto exit;
    if (!input_file) {
      grc_set_error(GRC_ERROR_PARAM, "no input file specified");
      goto exit;
    }
    if (!output_file)
      output_file = "resource.o";
  }
  /* set default descriptor values */
  const char *input_base = input_file;
  {
    const char *f = strrchr(input_base, '/');
    const char *b = strrchr(input_base, '\\');
    if (f && f > input_base)
      input_base = f + 1;
    if (b && b > input_base)
      input_base = b + 1;
  }
  if (!descriptor_name)
    descriptor_name = input_base;
  const char *resource_name = input_base;
  const char *resource_type = g_res_types[0].name;
  /* parse descriptor file */
  json_t *j_descriptor = NULL;
  if (descriptor_file) {
    parse_descriptor(descriptor_file, descriptor_name, &j_root, &j_descriptor,
                     &resource_type, &resource_name);
    if (g_error_code)
      goto exit;
  }
  /* make resource */
  _Bool match_found = 0;
  for (size_t i = 0; i < sizeof(g_res_types) / sizeof(*g_res_types); ++i)
    if (strcmp(g_res_types[i].name, resource_type) == 0) {
      match_found = 1;
      g_res_types[i].proc(input_file, output_file, resource_name,
                          j_descriptor);
      break;
    }
  if (!match_found)
    grc_set_error(GRC_ERROR_PARAM, "unrecognized resource type `%s`",
                  resource_type);
exit:
  if (g_error_code)
    printf("error: %s\n", g_error_message);
  if (j_root)
    json_decref(j_root);
  return g_error_code;
}
