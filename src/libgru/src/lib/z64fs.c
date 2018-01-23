#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <vector/vector.h>
#include "gru.h"
#include "z64fs.h"
#include "blob.h"
#include "n64rom.h"
#include "util.h"

struct z64_file
{
  size_t  vrom_start;
  size_t  vrom_end;
  size_t  vrom_size;
  size_t  vrom_padding_size;
  size_t  vrom_order;
  size_t  prom_start;
  size_t  prom_end;
  size_t  prom_size;
  void   *prom_data;
  size_t  prom_padding_size;
  void   *prom_padding;
  size_t  prom_order;
};

struct z64_ftab_entry
{
  uint32_t vrom_start;
  uint32_t vrom_end;
  uint32_t prom_start;
  uint32_t prom_end;
};


/**
  data manipulation helpers
**/

static size_t data_pre_cat(void *dst, size_t size, void *src, size_t src_size)
{
  if (src) {
    size_t data_size = (size < src_size ? size : src_size);
    memcpy((char*)dst - data_size,
           (char*)src + src_size - data_size, data_size);
    return size - data_size;
  }
  else {
    memset((char*)dst - size, 0, size);
    return 0;
  }
}

static size_t data_post_cat(void *dst, size_t size, void *src, size_t src_size)
{
  if (src) {
    size_t data_size = (size < src_size ? size : src_size);
    memcpy(dst, src, data_size);
    return size - data_size;
  }
  else {
    memset((char*)dst, 0, size);
    return 0;
  }
}

static enum gru_error z64fs_padding_grow_forwards(struct gru_z64fs *z64fs,
                                                  size_t index,
                                                  size_t vrom_size,
                                                  size_t prom_size,
                                                  gru_bool_t insert_data)
{
  if (index >= z64fs->files.size)
    return GRU_ERROR_RANGE;
  struct z64_file *file = vector_at(&z64fs->files, index);
  size_t prom_padding_size = file->prom_padding_size + prom_size;
  void *prom_padding = realloc(file->prom_padding, prom_padding_size);
  if (prom_padding_size > 0 && !prom_padding)
    return GRU_ERROR_MEMORY;
  size_t pn = prom_size;
  if (insert_data) {
    size_t data_index = index;
    while (pn > 0) {
      if (gru_z64fs_pnext(z64fs, data_index, &data_index))
        break;
      struct z64_file *data_file = vector_at(&z64fs->files, data_index);
      pn = data_post_cat((char*)prom_padding + prom_padding_size - pn,
                         pn, data_file->prom_data,
                         data_file->prom_size);
      pn = data_post_cat((char*)prom_padding + prom_padding_size - pn,
                         pn, data_file->prom_padding,
                         data_file->prom_padding_size);
    }
  }
  data_post_cat((char*)prom_padding + prom_padding_size - pn, pn, NULL, 0);
  file->vrom_padding_size += vrom_size;
  file->prom_padding_size = prom_padding_size;
  file->prom_padding = prom_padding;
  return GRU_SUCCESS;
}

static enum gru_error z64fs_padding_grow_backwards(struct gru_z64fs *z64fs,
                                                   size_t index,
                                                   size_t vrom_size,
                                                   size_t prom_size,
                                                   gru_bool_t insert_data)
{
  if (index >= z64fs->files.size)
    return GRU_ERROR_RANGE;
  struct z64_file *file = vector_at(&z64fs->files, index);
  size_t prom_padding_size = file->prom_padding_size + prom_size;
  void *prom_padding = malloc(prom_padding_size);
  if (prom_padding_size > 0 && !prom_padding)
    return GRU_ERROR_MEMORY;
  size_t pn = prom_padding_size;
  if (insert_data) {
    size_t data_index = index;
    struct z64_file *data_file = file;
    while (pn > 0) {
      pn = data_pre_cat((char*)prom_padding + pn, pn, data_file->prom_padding,
                        data_file->prom_padding_size);
      pn = data_pre_cat((char*)prom_padding + pn, pn, data_file->prom_data,
                        data_file->prom_size);
      if (gru_z64fs_pprev(z64fs, data_index, &data_index))
        break;
      data_file = vector_at(&z64fs->files, data_index);
    }
  }
  else
    pn = data_pre_cat((char*)prom_padding + pn, pn,
                     file->prom_padding, file->prom_padding_size);
  data_pre_cat((char*)prom_padding + pn, pn, NULL, 0);
  if (file->prom_padding)
    free(file->prom_padding);
  file->vrom_padding_size += vrom_size;
  file->prom_padding_size = prom_padding_size;
  file->prom_padding = prom_padding;
  return GRU_SUCCESS;
}

static enum gru_error z64fs_padding_shrink_forwards(struct gru_z64fs *z64fs,
                                                    size_t index,
                                                    size_t vrom_size,
                                                    size_t prom_size)
{
  if (index >= z64fs->files.size)
    return GRU_ERROR_RANGE;
  size_t vrom_index = index;
  size_t vn = vrom_size;
  while (vn > 0) {
    struct z64_file *vrom_file = vector_at(&z64fs->files, vrom_index);
    size_t size = (vrom_file->vrom_padding_size < vn ?
                   vrom_file->vrom_padding_size : vn);
    vrom_file->vrom_padding_size -= size;
    vn -= size;
    if (gru_z64fs_vnext(z64fs, vrom_index, &vrom_index))
      break;
  }
  size_t prom_index = index;
  size_t pn = prom_size;
  while (pn > 0) {
    struct z64_file *prom_file = vector_at(&z64fs->files, prom_index);
    size_t size = (prom_file->prom_padding_size < pn ?
                   prom_file->prom_padding_size : pn);
    size_t prom_padding_size = prom_file->prom_padding_size - size;
    memmove(prom_file->prom_padding,
            (char*)prom_file->prom_padding + size,
            prom_padding_size);
    void *prom_padding = realloc(prom_file->prom_padding, prom_padding_size);
    if (prom_padding_size > 0 && !prom_padding)
      return GRU_ERROR_MEMORY;
    prom_file->prom_padding_size = prom_padding_size;
    prom_file->prom_padding = prom_padding;
    pn -= size;
    if (gru_z64fs_pnext(z64fs, prom_index, &prom_index))
      break;
  }
  return GRU_SUCCESS;
}

static enum gru_error z64fs_padding_shrink_backwards(struct gru_z64fs *z64fs,
                                                     size_t index,
                                                     size_t vrom_size,
                                                     size_t prom_size)
{
  if (index >= z64fs->files.size)
    return GRU_ERROR_RANGE;
  size_t vrom_index = index;
  size_t vn = vrom_size;
  while (vn > 0) {
    struct z64_file *vrom_file = vector_at(&z64fs->files, vrom_index);
    size_t size = (vrom_file->vrom_padding_size < vn ?
                   vrom_file->vrom_padding_size : vn);
    vrom_file->vrom_padding_size -= size;
    vn -= size;
    if (gru_z64fs_vprev(z64fs, vrom_index, &vrom_index))
      break;
  }
  size_t prom_index = index;
  size_t pn = prom_size;
  while (pn > 0) {
    struct z64_file *prom_file = vector_at(&z64fs->files, prom_index);
    size_t size = (prom_file->prom_padding_size < pn ?
                   prom_file->prom_padding_size : pn);
    size_t prom_padding_size = prom_file->prom_padding_size - size;
    void *prom_padding = realloc(prom_file->prom_padding, prom_padding_size);
    if (prom_padding_size > 0 && !prom_padding)
      return GRU_ERROR_MEMORY;
    prom_file->prom_padding_size = prom_padding_size;
    prom_file->prom_padding = prom_padding;
    pn -= size;
    if (gru_z64fs_pprev(z64fs, prom_index, &prom_index))
      break;
  }
  return GRU_SUCCESS;
}

static enum gru_error z64fs_padding_resize(struct gru_z64fs *z64fs,
                                           size_t index,
                                           size_t vrom_padding_size,
                                           size_t prom_padding_size,
                                           gru_bool_t insert_data)
{
  if (index >= z64fs->files.size)
    return GRU_ERROR_RANGE;
  struct z64_file *file = vector_at(&z64fs->files, index);
  enum gru_error e = GRU_SUCCESS;
  if (vrom_padding_size > file->vrom_padding_size) {
    size_t vrom_delta = vrom_padding_size - file->vrom_padding_size;
    e = z64fs_padding_grow_forwards(z64fs, index, vrom_delta, 0, insert_data);
  }
  else if (vrom_padding_size < file->vrom_padding_size) {
    size_t vrom_delta = vrom_padding_size - file->vrom_padding_size;
    e = z64fs_padding_shrink_backwards(z64fs, index, vrom_delta, 0);
  }
  if (e)
    return e;
  if (prom_padding_size > file->prom_padding_size) {
    size_t prom_delta = prom_padding_size - file->prom_padding_size;
    e = z64fs_padding_grow_forwards(z64fs, index, 0, prom_delta, insert_data);
  }
  else if (prom_padding_size < file->prom_padding_size) {
    size_t prom_delta = file->prom_padding_size - prom_padding_size;
    e = z64fs_padding_shrink_backwards(z64fs, index, 0, prom_delta);
  }
  return e;
}

static enum gru_error z64fs_padding_redistribute(struct gru_z64fs *z64fs,
                                                 size_t index,
                                                 size_t vrom_padding_size,
                                                 size_t prom_padding_size,
                                                 gru_bool_t insert_data)
{
  if (index >= z64fs->files.size)
    return GRU_ERROR_RANGE;
  struct z64_file *file = vector_at(&z64fs->files, index);
  enum gru_error e = GRU_SUCCESS;
  size_t vrom_next_index;
  if (!gru_z64fs_vnext(z64fs, index, &vrom_next_index)) {
    if (vrom_padding_size > file->vrom_padding_size) {
      size_t vrom_delta = vrom_padding_size - file->vrom_padding_size;
      e = z64fs_padding_shrink_forwards(z64fs, vrom_next_index,
                                        vrom_delta, 0);
    }
    else if (vrom_padding_size < file->vrom_padding_size) {
      size_t vrom_delta = file->vrom_padding_size - vrom_padding_size;
      e = z64fs_padding_grow_backwards(z64fs, vrom_next_index,
                                       vrom_delta, 0, insert_data);
    }
  }
  if (e)
    return e;
  size_t prom_next_index;
  if (!gru_z64fs_pnext(z64fs, index, &prom_next_index)) {
    if (prom_padding_size > file->prom_padding_size) {
      size_t prom_delta = prom_padding_size - file->prom_padding_size;
      e = z64fs_padding_shrink_forwards(z64fs, prom_next_index,
                                        0, prom_delta);
    }
    else if (prom_padding_size < file->prom_padding_size) {
      size_t prom_delta = file->prom_padding_size - prom_padding_size;
      e = z64fs_padding_grow_backwards(z64fs, prom_next_index,
                                       0, prom_delta, insert_data);
    }
  }
  if (e)
    return e;
  return z64fs_padding_resize(z64fs, index,
                              vrom_padding_size, prom_padding_size,
                              GRU_FALSE);
}

static enum gru_error z64fs_file_eat(struct gru_z64fs *z64fs,
                                     size_t index,
                                     gru_bool_t eat_vrom,
                                     gru_bool_t eat_prom)
{
  if (index >= z64fs->files.size)
    return GRU_ERROR_RANGE;
  struct z64_file *file = vector_at(&z64fs->files, index);
  enum gru_error e = GRU_SUCCESS;
  if (eat_vrom) {
    size_t vrom_padding_index;
    if (!gru_z64fs_vprev(z64fs, index, &vrom_padding_index))
      e = z64fs_padding_grow_forwards(z64fs, vrom_padding_index,
                                      file->vrom_size +
                                      file->vrom_padding_size,
                                      0,
                                      GRU_TRUE);
    else if (!gru_z64fs_vnext(z64fs, index, &vrom_padding_index))
      e = z64fs_padding_grow_backwards(z64fs, vrom_padding_index,
                                       file->vrom_size +
                                       file->vrom_padding_size,
                                       0,
                                       GRU_TRUE);
    if (e)
      return e;
  }
  if (eat_prom) {
    size_t prom_padding_index;
    if (!gru_z64fs_pprev(z64fs, index, &prom_padding_index))
      e = z64fs_padding_grow_forwards(z64fs, prom_padding_index,
                                      0,
                                      file->prom_size +
                                      file->prom_padding_size,
                                      GRU_TRUE);
    else if (!gru_z64fs_pnext(z64fs, index, &prom_padding_index))
      e = z64fs_padding_grow_backwards(z64fs, prom_padding_index,
                                       0,
                                       file->prom_size +
                                       file->prom_padding_size,
                                       GRU_TRUE);
  }
  return e;
}


/**
  file helpers
**/

static gru_bool_t file_is_compressed(struct z64_file *file)
{
  return file->prom_end != 0x00000000;
}

static gru_bool_t file_is_null(struct z64_file *file)
{
  return file->prom_start == 0xFFFFFFFF && file->prom_end == 0xFFFFFFFF;
}


/**
  file table helpers
**/

static gru_bool_t ftab_validate(struct gru_blob *blob, size_t ftab_start,
                                size_t *ftab_length_out, size_t *ftab_index_out)
{
  if (ftab_start > blob->size)
    return GRU_FALSE;
  struct z64_ftab_entry *ftab = blob_at(blob, ftab_start);
  size_t ftab_length = (blob->size - ftab_start) /
                       sizeof(*ftab);
  if (ftab_length < 16)
    return GRU_FALSE;
  ftab_length -= 16;
  size_t ftab_index = ftab_length;
  size_t vrom_min = ~(size_t)0;
  size_t prom_min = ~(size_t)0;
  for (size_t i = 0; i < ftab_length; ++i) {
    struct z64_ftab_entry *entry = &ftab[i];
    uint32_t vrom_start = gru_util_betoh32(entry->vrom_start);
    uint32_t vrom_end = gru_util_betoh32(entry->vrom_end);
    uint32_t prom_start = gru_util_betoh32(entry->prom_start);
    uint32_t prom_end = gru_util_betoh32(entry->prom_end);
    gru_bool_t null = (prom_start == 0xFFFFFFFF && prom_end == 0xFFFFFFFF);
    if (prom_end == 0x00000000)
      prom_end = prom_start + (vrom_end - vrom_start);
    if (vrom_start > vrom_end || prom_start > prom_end ||
        (vrom_start & 0xF) || (vrom_end & 0xF) ||
        (!null && (prom_start > blob->size || prom_end > blob->size ||
                   (prom_start & 0xF) || (prom_end & 0xF))))
      return GRU_FALSE;
    for (size_t j = 0; j < i; ++j) {
      struct z64_ftab_entry *c_entry = &ftab[j];
      uint32_t c_vrom_start = gru_util_betoh32(c_entry->vrom_start);
      uint32_t c_vrom_end = gru_util_betoh32(c_entry->vrom_end);
      uint32_t c_prom_start = gru_util_betoh32(c_entry->prom_start);
      uint32_t c_prom_end = gru_util_betoh32(c_entry->prom_end);
      if (c_prom_end == 0x00000000)
        c_prom_end = c_prom_start + (c_vrom_end - c_vrom_start);
      if ((vrom_start >= c_vrom_start && vrom_start < c_vrom_end) ||
          (vrom_end > c_vrom_start && vrom_end <= c_vrom_end) ||
          (prom_start >= c_prom_start && prom_start < c_prom_end) ||
          (prom_end > c_prom_start && prom_end <= c_prom_end))
        return GRU_FALSE;
    }
    if (prom_start == ftab_start &&
        prom_end - prom_start == vrom_end - vrom_start)
    {
      size_t ftab_size = prom_end - prom_start;
      ftab_length = ftab_size / sizeof(*ftab);
      if (ftab_length <= i + 16 ||
          ftab_length * sizeof(*ftab) != ftab_size)
        return GRU_FALSE;
      ftab_length -= 16;
      ftab_index = i;
      for (size_t i = 0; i < sizeof(*ftab) * 16; ++i)
        if (*((char*)&ftab[ftab_length] + i) != 0)
          return GRU_FALSE;
    }
    if (vrom_start < vrom_min)
      vrom_min = vrom_start;
    if (prom_start < prom_min)
      prom_min = prom_start;
  }
  if (ftab_index >= ftab_length)
    return GRU_FALSE;
  if (vrom_min != 0 || prom_min != 0)
    return GRU_FALSE;
  *ftab_length_out = ftab_length;
  *ftab_index_out = ftab_index;
  return GRU_TRUE;
}

static gru_bool_t ftab_locate(struct gru_blob *blob, size_t *ftab_start_out,
                              size_t *ftab_length_out, size_t *ftab_index_out)
{
  const char search_string[] = "zelda@";
  const size_t search_size = sizeof(search_string) - 1;
  const size_t search_offset = 0x30;
  for (size_t i = 0; i + search_offset < blob->size; ++i)
    if (memcmp(blob_at(blob, i), search_string, search_size) == 0)
      if (ftab_validate(blob, i + search_offset,
                        ftab_length_out, ftab_index_out))
      {
        *ftab_start_out = i + search_offset;
        return GRU_TRUE;
      }
  return GRU_FALSE;
}

static enum gru_error z64fs_update(struct gru_z64fs *z64fs)
{
  struct z64_file *ftab = vector_at(&z64fs->files, z64fs->ftab_index);
  struct z64_ftab_entry *ftab_data;
  size_t ftab_size = sizeof(*ftab_data) * (z64fs->files.size + 16);
  if (ftab->prom_size != ftab_size) {
    enum gru_error e = GRU_SUCCESS;
    gru_bool_t vvolatile = z64fs->vrom_volatile && z64fs->ftab_vvolatile;
    gru_bool_t pvolatile = z64fs->prom_volatile && z64fs->ftab_pvolatile;
    if (ftab_size > ftab->prom_size) {
      size_t delta = ftab_size - ftab->prom_size;
      e = z64fs_padding_shrink_forwards(z64fs, z64fs->ftab_index,
                                        vvolatile ? 0 : delta,
                                        pvolatile ? 0 : delta);
    }
    else if (ftab_size < ftab->prom_size) {
      size_t delta = ftab->prom_size - ftab_size;
      e = z64fs_padding_grow_backwards(z64fs, z64fs->ftab_index,
                                       vvolatile ? 0 : delta,
                                       pvolatile ? 0 : delta,
                                       GRU_TRUE);
    }
    if (e)
      return e;
    void *prom_data = realloc(ftab->prom_data, ftab_size);
    if (ftab_size > 0 && !prom_data)
      return GRU_ERROR_MEMORY;
    ftab->vrom_size = ftab_size;
    ftab->prom_size = ftab_size;
    ftab->prom_data = prom_data;
  }
  ftab_data = ftab->prom_data;
  size_t vc = 0;
  size_t pc = 0;
  for (size_t i = 0; i < z64fs->files.size; ++i) {
    size_t vrom_index;
    size_t prom_index;
    gru_z64fs_vindex(z64fs, i, &vrom_index);
    gru_z64fs_pindex(z64fs, i, &prom_index);
    struct z64_file *vrom_file = vector_at(&z64fs->files, vrom_index);
    struct z64_file *prom_file = vector_at(&z64fs->files, prom_index);
    struct z64_ftab_entry *vrom_entry = &ftab_data[vrom_index];
    struct z64_ftab_entry *prom_entry = &ftab_data[prom_index];
    vrom_file->vrom_start = vc;
    vc += vrom_file->vrom_size;
    vrom_file->vrom_end = vc;
    vc += vrom_file->vrom_padding_size;
    if (!file_is_null(prom_file)) {
      prom_file->prom_start = pc;
      pc += prom_file->prom_size;
      if (prom_file->prom_end != 0x00000000)
        prom_file->prom_end = pc;
      pc += prom_file->prom_padding_size;
    }
    vrom_entry->vrom_start = gru_util_htobe32(vrom_file->vrom_start);
    vrom_entry->vrom_end = gru_util_htobe32(vrom_file->vrom_end);
    prom_entry->prom_start = gru_util_htobe32(prom_file->prom_start);
    prom_entry->prom_end = gru_util_htobe32(prom_file->prom_end);
  }
  memset(&ftab_data[z64fs->files.size], 0, sizeof(*ftab_data) * 16);
  z64fs->vrom_size = vc;
  z64fs->prom_size = pc;
  return GRU_SUCCESS;
}


/**
  library functions
**/

enum gru_error gru_z64fs_init(struct gru_z64fs *z64fs)
{
  vector_init(&z64fs->files, sizeof(struct z64_file));
  z64fs->ftab_index = 0;
  z64fs->ftab_vvolatile = GRU_TRUE;
  z64fs->ftab_pvolatile = GRU_TRUE;
  z64fs->vrom_volatile = GRU_FALSE;
  z64fs->prom_volatile = GRU_FALSE;
  struct z64_file *ftab = vector_push_back(&z64fs->files, 1, NULL);
  if (!ftab) {
    vector_destroy(&z64fs->files);
    return GRU_ERROR_MEMORY;
  }
  size_t ftab_size = sizeof(struct z64_ftab_entry) * (z64fs->files.size + 16);
  void *ftab_data = malloc(ftab_size);
  if (ftab_size > 0 && !ftab_data) {
    vector_destroy(&z64fs->files);
    return GRU_ERROR_MEMORY;
  }
  ftab->vrom_start = 0x00000000;
  ftab->vrom_end = ftab->vrom_start + ftab_size;
  ftab->vrom_size = ftab_size;
  ftab->vrom_padding_size = 0;
  ftab->vrom_order = 0;
  ftab->prom_start = 0x00000000;
  ftab->prom_end = 0x00000000;
  ftab->prom_size = ftab_size;
  ftab->prom_data = ftab_data;
  ftab->prom_padding_size = 0;
  ftab->prom_padding = NULL;
  ftab->prom_order = 0;
  enum gru_error e = z64fs_update(z64fs);
  if (e) {
    vector_destroy(&z64fs->files);
    return e;
  }
  return GRU_SUCCESS;
}

void gru_z64fs_destroy(struct gru_z64fs *z64fs)
{
  for (size_t i = 0; i < z64fs->files.size; ++i) {
    struct z64_file *file = vector_at(&z64fs->files, i);
    if (file->prom_data)
      free(file->prom_data);
    if (file->prom_padding)
      free(file->prom_padding);
  }
  vector_destroy(&z64fs->files);
}

enum gru_error gru_z64fs_load(struct gru_z64fs *z64fs, struct gru_blob *blob,
                              size_t *ftab_start_in)
{
  vector_clear(&z64fs->files);
  size_t ftab_start;
  size_t ftab_length;
  if (ftab_start_in) {
    ftab_start = *ftab_start_in;
    if (!ftab_validate(blob, ftab_start, &ftab_length, &z64fs->ftab_index))
      return GRU_ERROR_DATA;
  }
  else if (!ftab_locate(blob, &ftab_start, &ftab_length, &z64fs->ftab_index))
    return GRU_ERROR_DATA;
  for (size_t i = 0; i < z64fs->files.size; ++i) {
    struct z64_file *file = vector_at(&z64fs->files, i);
    if (file->prom_data)
      free(file->prom_data);
    if (file->prom_padding)
      free(file->prom_padding);
  }
  struct z64_ftab_entry *ftab = blob_at(blob, ftab_start);
  for (size_t i = 0; i < ftab_length; ++i) {
    struct z64_file file;
    file.vrom_start = gru_util_betoh32(ftab[i].vrom_start);
    file.vrom_end = gru_util_betoh32(ftab[i].vrom_end);
    file.vrom_size = file.vrom_end - file.vrom_start;
    file.vrom_padding_size = 0;
    file.prom_start = gru_util_betoh32(ftab[i].prom_start);
    file.prom_end = gru_util_betoh32(ftab[i].prom_end);
    file.prom_size = (file.prom_end != 0x00000000 ?
                      file.prom_end - file.prom_start :
                      file.vrom_end - file.vrom_start);
    file.prom_data = malloc(file.prom_size);
    if (file.prom_size > 0 && !file.prom_data)
      return GRU_ERROR_MEMORY;
    memcpy(file.prom_data, blob_at(blob, file.prom_start), file.prom_size);
    file.prom_padding_size = 0;
    file.prom_padding = NULL;
    file.prom_order = ~(size_t)0;
    if (!vector_push_back(&z64fs->files, 1, &file)) {
      if (file.prom_data)
        free(file.prom_data);
      return GRU_ERROR_MEMORY;
    }
  }
  struct z64_file *vrom_prev_file = NULL;
  struct z64_file *prom_prev_file = NULL;
  size_t vc = 0;
  size_t pc = 0;
  for (size_t i = 0; i < z64fs->files.size; ++i) {
    size_t vrom_index = i;
    size_t vrom_dist = ~(size_t)0;
    size_t prom_index = i;
    size_t prom_dist = ~(size_t)0;
    for (size_t j = 0; j < z64fs->files.size; ++j) {
      struct z64_file *file = vector_at(&z64fs->files, j);
      if (file->vrom_start >= vc && file->vrom_start - vc < vrom_dist) {
        vrom_index = j;
        vrom_dist = file->vrom_start - vc;
      }
      if (file->prom_order == ~(size_t)0 &&
          file->prom_start >= pc && file->prom_start - pc < prom_dist)
      {
        prom_index = j;
        prom_dist = file->prom_start - pc;
      }
    }
    struct z64_file *vrom_file = vector_at(&z64fs->files, vrom_index);
    struct z64_file *prom_file = vector_at(&z64fs->files, prom_index);
    vrom_file->vrom_order = i;
    prom_file->prom_order = i;
    if (vrom_prev_file)
      vrom_prev_file->vrom_padding_size = vrom_dist;
    vrom_prev_file = vrom_file;
    vc = vrom_file->vrom_start + vrom_file->vrom_size;
    if (!file_is_null(prom_file)) {
      if (prom_prev_file && prom_dist > 0) {
        prom_prev_file->prom_padding_size = prom_dist;
        prom_prev_file->prom_padding = malloc(prom_dist);
        if (!prom_prev_file->prom_padding)
          return GRU_ERROR_MEMORY;
        memcpy(prom_prev_file->prom_padding, blob_at(blob, pc), prom_dist);
      }
      prom_prev_file = prom_file;
      pc = prom_file->prom_start + prom_file->prom_size;
    }
  }
  if (prom_prev_file) {
    size_t prom_dist = blob->size - pc;
    if (prom_dist > 0) {
      prom_prev_file->prom_padding_size = prom_dist;
      prom_prev_file->prom_padding = malloc(prom_dist);
      if (!prom_prev_file->prom_padding)
        return GRU_ERROR_MEMORY;
      memcpy(prom_prev_file->prom_padding, blob_at(blob, pc), prom_dist);
    }
  }
  return z64fs_update(z64fs);
}

enum gru_error gru_z64fs_assemble_blob(struct gru_z64fs *z64fs,
                                       struct gru_blob *blob)
{
  blob->size = z64fs->prom_size;
  blob->data = malloc(blob->size);
  if (blob->size > 0 && !blob->data)
    return GRU_ERROR_MEMORY;
  for (size_t i = 0; i < z64fs->files.size; ++i) {
    size_t prom_index;
    gru_z64fs_pindex(z64fs, i, &prom_index);
    struct z64_file *prom_file = vector_at(&z64fs->files, prom_index);
    if (file_is_null(prom_file))
      continue;
    memcpy(blob_at(blob, prom_file->prom_start),
           prom_file->prom_data, prom_file->prom_size);
    memcpy(blob_at(blob, prom_file->prom_start + prom_file->prom_size),
           prom_file->prom_padding, prom_file->prom_padding_size);
  }
  return GRU_SUCCESS;
}

enum gru_error gru_z64fs_assemble_rom(struct gru_z64fs *z64fs,
                                      struct gru_n64rom *n64rom)
{
  return gru_z64fs_assemble_blob(z64fs, &n64rom->blob);
}

size_t gru_z64fs_length(struct gru_z64fs *z64fs)
{
  return z64fs->files.size;
}

size_t gru_z64fs_ftab(struct gru_z64fs *z64fs)
{
  return z64fs->ftab_index;
}

gru_bool_t gru_z64fs_ftab_vvolatile(struct gru_z64fs *z64fs)
{
  return z64fs->ftab_vvolatile;
}

void gru_z64fs_set_ftab_vvolatile(struct gru_z64fs *z64fs,
                                  gru_bool_t ftab_vvolatile)
{
  z64fs->ftab_vvolatile = ftab_vvolatile;
}

gru_bool_t gru_z64fs_ftab_pvolatile(struct gru_z64fs *z64fs)
{
  return z64fs->ftab_pvolatile;
}

void gru_z64fs_set_ftab_pvolatile(struct gru_z64fs *z64fs,
                                  gru_bool_t ftab_pvolatile)
{
  z64fs->ftab_pvolatile = ftab_pvolatile;
}

size_t gru_z64fs_vrom_first(struct gru_z64fs *z64fs)
{
  size_t vrom_index;
  gru_z64fs_vindex(z64fs, 0, &vrom_index);
  return vrom_index;
}

size_t gru_z64fs_vrom_last(struct gru_z64fs *z64fs)
{
  size_t vrom_index;
  return gru_z64fs_vindex(z64fs, z64fs->files.size - 1, &vrom_index);
  return vrom_index;
}

size_t gru_z64fs_prom_first(struct gru_z64fs *z64fs)
{
  for (size_t i = 0; i < z64fs->files.size; ++i) {
    size_t prom_index;
    gru_z64fs_pindex(z64fs, i, &prom_index);
    struct z64_file *prom_file = vector_at(&z64fs->files, prom_index);
    if (!file_is_null(prom_file))
      return prom_index;
  }
  return 0;
}

size_t gru_z64fs_prom_last(struct gru_z64fs *z64fs)
{
  for (size_t i = z64fs->files.size - 1; ; --i) {
    size_t prom_index;
    gru_z64fs_pindex(z64fs, i, &prom_index);
    struct z64_file *prom_file = vector_at(&z64fs->files, prom_index);
    if (!file_is_null(prom_file))
      return prom_index;
    if (i == 0)
      break;
  }
  return z64fs->files.size - 1;
}

size_t gru_z64fs_vrom_size(struct gru_z64fs *z64fs)
{
  return z64fs->vrom_size;
}

size_t gru_z64fs_vrom_tail(struct gru_z64fs *z64fs)
{
  size_t vrom_tail = z64fs->vrom_size;
  gru_z64fs_vend(z64fs, gru_z64fs_vrom_last(z64fs), &vrom_tail);
  return vrom_tail;
}

size_t gru_z64fs_prom_size(struct gru_z64fs *z64fs)
{
  return z64fs->prom_size;
}

enum gru_error gru_z64fs_set_prom_size(struct gru_z64fs *z64fs,
                                       size_t prom_size)
{
  if (prom_size & 0xF)
    return GRU_ERROR_PARAM;
  size_t prom_last_index = gru_z64fs_prom_last(z64fs);
  struct z64_file *prom_last_file = vector_at(&z64fs->files, prom_last_index);
  size_t prom_end = prom_last_file->prom_start + prom_last_file->prom_size;
  if (prom_size < prom_end)
    return GRU_ERROR_PARAM;
  size_t prom_padding_size = prom_size - prom_end;
  void *prom_padding = realloc(prom_last_file->prom_padding, prom_padding_size);
  if (prom_padding_size > 0 && !prom_padding)
    return GRU_ERROR_MEMORY;
  for (size_t i = prom_last_file->prom_padding_size; i < prom_padding_size; ++i)
    ((char*)prom_padding)[i] = ((prom_end + i) & 0xFF);
  prom_last_file->prom_padding_size = prom_padding_size;
  prom_last_file->prom_padding = prom_padding;
  return z64fs_update(z64fs);
}

size_t gru_z64fs_prom_tail(struct gru_z64fs *z64fs)
{
  size_t prom_tail = z64fs->prom_size;
  gru_z64fs_ptail(z64fs, gru_z64fs_prom_last(z64fs), &prom_tail);
  return prom_tail;
}

gru_bool_t gru_z64fs_vvolatile(struct gru_z64fs *z64fs)
{
  return z64fs->vrom_volatile;
}

void gru_z64fs_set_vvolatile(struct gru_z64fs *z64fs, gru_bool_t vrom_volatile)
{
  z64fs->vrom_volatile = vrom_volatile;
}

gru_bool_t gru_z64fs_pvolatile(struct gru_z64fs *z64fs)
{
  return z64fs->prom_volatile;
}

void gru_z64fs_set_pvolatile(struct gru_z64fs *z64fs, gru_bool_t prom_volatile)
{
  z64fs->prom_volatile = prom_volatile;
}

enum gru_error gru_z64fs_vfind(struct gru_z64fs *z64fs,
                               size_t vrom_start, size_t vrom_end,
                               size_t *index_out)
{
  for (size_t i = 0; i < z64fs->files.size; ++i) {
    struct z64_file *file = vector_at(&z64fs->files, i);
    if (file->vrom_start == vrom_start && file->vrom_end == vrom_end) {
      *index_out = i;
      return GRU_SUCCESS;
    }
  }
  return GRU_ERROR_PARAM;
}

enum gru_error gru_z64fs_pfind(struct gru_z64fs *z64fs,
                               size_t prom_start, size_t prom_end,
                               size_t *index_out)
{
  for (size_t i = 0; i < z64fs->files.size; ++i) {
    struct z64_file *file = vector_at(&z64fs->files, i);
    if (file_is_null(file))
      continue;
    if (file->prom_start == prom_start &&
        file->prom_start + file->prom_size == prom_end)
    {
      *index_out = i;
      return GRU_SUCCESS;
    }
  }
  return GRU_ERROR_PARAM;
}

enum gru_error gru_z64fs_vat(struct gru_z64fs *z64fs,
                             size_t vrom_address, size_t *index_out)
{
  for (size_t i = 0; i < z64fs->files.size; ++i) {
    struct z64_file *file = vector_at(&z64fs->files, i);
    if (file->vrom_start <= vrom_address && file->vrom_end > vrom_address) {
      *index_out = i;
      return GRU_SUCCESS;
    }
  }
  return GRU_ERROR_PARAM;
}

enum gru_error gru_z64fs_pat(struct gru_z64fs *z64fs,
                             size_t prom_address, size_t *index_out)
{
  for (size_t i = 0; i < z64fs->files.size; ++i) {
    struct z64_file *file = vector_at(&z64fs->files, i);
    if (file_is_null(file))
      continue;
    if (file->prom_start <= prom_address &&
        file->prom_start + file->prom_size > prom_address)
    {
      *index_out = i;
      return GRU_SUCCESS;
    }
  }
  return GRU_ERROR_PARAM;
}

enum gru_error gru_z64fs_vindex(struct gru_z64fs *z64fs,
                                size_t vrom_order, size_t *index_out)
{
  *index_out = vrom_order;
  if (vrom_order >= z64fs->files.size)
    return GRU_ERROR_RANGE;
  for (size_t i = 0; i < z64fs->files.size; ++i) {
    struct z64_file *file = vector_at(&z64fs->files, i);
    if (file->vrom_order == vrom_order) {
      *index_out = i;
      break;
    }
  }
  return GRU_SUCCESS;
}

enum gru_error gru_z64fs_pindex(struct gru_z64fs *z64fs,
                                size_t prom_order, size_t *index_out)
{
  *index_out = prom_order;
  if (prom_order >= z64fs->files.size)
    return GRU_ERROR_RANGE;
  for (size_t i = 0; i < z64fs->files.size; ++i) {
    struct z64_file *file = vector_at(&z64fs->files, i);
    if (file->prom_order == prom_order) {
      *index_out = i;
      break;
    }
  }
  return GRU_SUCCESS;
}

enum gru_error gru_z64fs_vprev(struct gru_z64fs *z64fs,
                               size_t index, size_t *index_out)
{
  if (index >= z64fs->files.size)
    return GRU_ERROR_RANGE;
  struct z64_file *file = vector_at(&z64fs->files, index);
  size_t vrom_order = file->vrom_order;
  if (vrom_order == 0)
    return GRU_ERROR_RANGE;
  gru_z64fs_vindex(z64fs, vrom_order - 1, index_out);
  return GRU_SUCCESS;
}

enum gru_error gru_z64fs_vnext(struct gru_z64fs *z64fs,
                               size_t index, size_t *index_out)
{
  if (index >= z64fs->files.size)
    return GRU_ERROR_RANGE;
  struct z64_file *file = vector_at(&z64fs->files, index);
  size_t vrom_order = file->vrom_order;
  for (size_t i = vrom_order + 1; i < z64fs->files.size; ++i) {
    size_t i_index;
    gru_z64fs_vindex(z64fs, i, &i_index);
    *index_out = i_index;
    return GRU_SUCCESS;
  }
  return GRU_ERROR_RANGE;
}

enum gru_error gru_z64fs_pprev(struct gru_z64fs *z64fs,
                               size_t index, size_t *index_out)
{
  if (index >= z64fs->files.size)
    return GRU_ERROR_RANGE;
  struct z64_file *file = vector_at(&z64fs->files, index);
  size_t prom_order = file->prom_order;
  if (prom_order == 0)
    return GRU_ERROR_RANGE;
  for (size_t i = prom_order - 1; ; --i) {
    size_t i_index;
    gru_z64fs_pindex(z64fs, i, &i_index);
    struct z64_file *i_file = vector_at(&z64fs->files, i_index);
    if (!file_is_null(i_file)) {
      *index_out = i_index;
      return GRU_SUCCESS;
    }
    if (i == 0)
      break;
  }
  return GRU_ERROR_RANGE;
}

enum gru_error gru_z64fs_pnext(struct gru_z64fs *z64fs,
                               size_t index, size_t *index_out)
{
  if (index >= z64fs->files.size)
    return GRU_ERROR_RANGE;
  struct z64_file *file = vector_at(&z64fs->files, index);
  size_t prom_order = file->prom_order;
  for (size_t i = prom_order + 1; i < z64fs->files.size; ++i) {
    size_t i_index;
    gru_z64fs_pindex(z64fs, i, &i_index);
    struct z64_file *i_file = vector_at(&z64fs->files, i_index);
    if (!file_is_null(i_file)) {
      *index_out = i_index;
      return GRU_SUCCESS;
    }
  }
  return GRU_ERROR_RANGE;
}

enum gru_error gru_z64fs_insert(struct gru_z64fs *z64fs,
                                struct gru_blob *blob,
                                gru_bool_t compress)
{
  /* initialize file */
  struct z64_file file;
  file.vrom_padding_size = 0;
  file.vrom_order = z64fs->files.size;
  file.prom_padding_size = 0;
  file.prom_padding = NULL;
  file.prom_order = z64fs->files.size;
  /* setup file data */
  struct gru_blob null_blob;
  null_blob.size = 0;
  null_blob.data = NULL;
  if (!blob)
    blob = &null_blob;
  size_t file_size = blob->size;
  void *file_data;
  if (compress) {
    file_data = blob->data;
    enum gru_error e = gru_util_yaz0_encode(&file_data, &file_size);
    if (e)
      return e;
    file.prom_end = 0x00000001;
  }
  else {
    file_data = malloc(file_size);
    if (file_size > 0 && !file_data)
      return GRU_ERROR_MEMORY;
    memcpy(file_data, blob->data, blob->size);
    file.prom_end = 0x00000000;
  }
  /* find unused space to place file in */
  size_t vrom_order_padding_size = ~(size_t)0;
  size_t prom_order_padding_size = ~(size_t)0;
  for (size_t i = z64fs->files.size; i > 0; --i) {
    if (!z64fs->vrom_volatile) {
      size_t vrom_index;
      gru_z64fs_vindex(z64fs, i - 1, &vrom_index);
      struct z64_file *vrom_file = vector_at(&z64fs->files, vrom_index);
      if (vrom_file->vrom_padding_size >= blob->size &&
          vrom_file->vrom_padding_size <= vrom_order_padding_size)
      {
        file.vrom_order = i;
        vrom_order_padding_size = vrom_file->vrom_padding_size;
      }
    }
    if (!z64fs->prom_volatile) {
      size_t prom_index;
      gru_z64fs_pindex(z64fs, i - 1, &prom_index);
      struct z64_file *prom_file = vector_at(&z64fs->files, prom_index);
      if (prom_file->prom_padding_size >= file_size &&
          prom_file->prom_padding_size <= prom_order_padding_size)
      {
        file.prom_order = i;
        prom_order_padding_size = prom_file->prom_padding_size;
      }
    }
  }
  /* insert file */
  size_t index = z64fs->files.size;
  struct z64_file *p_file = vector_insert(&z64fs->files, index, 1, &file);
  if (!p_file) {
    if (file_data)
      free(file_data);
    return GRU_ERROR_MEMORY;
  }
  /* increase order of subsequent files */
  for (size_t i = 0; i < z64fs->files.size; ++i) {
    struct z64_file *i_file = vector_at(&z64fs->files, i);
    if (i_file == p_file)
      continue;
    if (i_file->vrom_order >= p_file->vrom_order)
      ++i_file->vrom_order;
    if (i_file->prom_order >= p_file->prom_order)
      ++i_file->prom_order;
  }
  /* steal padding from preceding file */
  if (!z64fs->vrom_volatile) {
    size_t vrom_prev_index;
    if (!gru_z64fs_vprev(z64fs, index, &vrom_prev_index)) {
      struct z64_file *vrom_prev_file = vector_at(&z64fs->files,
                                                  vrom_prev_index);
      p_file->vrom_padding_size = vrom_prev_file->vrom_padding_size;
      vrom_prev_file->vrom_padding_size = 0;
    }
  }
  if (!z64fs->prom_volatile) {
    size_t prom_prev_index;
    if (!gru_z64fs_pprev(z64fs, index, &prom_prev_index)) {
      struct z64_file *prom_prev_file = vector_at(&z64fs->files,
                                                  prom_prev_index);
      p_file->prom_padding = prom_prev_file->prom_padding;
      p_file->prom_padding_size = prom_prev_file->prom_padding_size;
      prom_prev_file->prom_padding = NULL;
      prom_prev_file->prom_padding_size = 0;
    }
  }
  /* pad file to 16 byte boundary */
  size_t vrom_size = ((blob->size + 0xF) & ~0xF);
  size_t prom_size = ((file_size + 0xF) & ~0xF);
  void *prom_data = realloc(file_data, prom_size);
  if (prom_size > 0 && !prom_data) {
    if (file_data)
      free(file_data);
    return GRU_ERROR_MEMORY;
  }
  memset((char*)prom_data + file_size, 0, prom_size - file_size);
  p_file->vrom_size = vrom_size;
  p_file->prom_size = prom_size;
  p_file->prom_data = prom_data;
  /* shrink padding to accommodate the new file */
  enum gru_error e = z64fs_padding_shrink_forwards(z64fs, index,
                                                   z64fs->vrom_volatile ?
                                                   0 : vrom_size,
                                                   z64fs->prom_volatile ?
                                                   0 : prom_size);
  if (e)
    return e;
  return z64fs_update(z64fs);
}

enum gru_error gru_z64fs_replace(struct gru_z64fs *z64fs, size_t index,
                                 struct gru_blob *blob,
                                 gru_bool_t compress)
{
  if (index >= z64fs->files.size)
    return GRU_ERROR_RANGE;
  if (index == z64fs->ftab_index)
    return GRU_ERROR_PARAM;
  struct z64_file *file = vector_at(&z64fs->files, index);
  /* grow padding to eat old file data */
  enum gru_error e = z64fs_padding_grow_backwards(z64fs, index,
                                                  z64fs->vrom_volatile ?
                                                  0 : file->vrom_size,
                                                  z64fs->prom_volatile ?
                                                  0 : file->prom_size,
                                                  GRU_TRUE);
  if (e)
    return e;
  /* setup file data */
  struct gru_blob null_blob;
  null_blob.size = 0;
  null_blob.data = NULL;
  if (!blob)
    blob = &null_blob;
  size_t file_size = blob->size;
  void *file_data;
  if (compress) {
    file_data = blob->data;
    enum gru_error e = gru_util_yaz0_encode(&file_data, &file_size);
    if (e)
      return e;
    file->prom_end = 0x00000001;
  }
  else {
    file_data = malloc(file_size);
    if (file_size > 0 && !file_data)
      return GRU_ERROR_MEMORY;
    memcpy(file_data, blob->data, blob->size);
    file->prom_end = 0x00000000;
  }
  /* pad file to 16 byte boundary */
  size_t vrom_size = ((blob->size + 0xF) & ~0xF);
  size_t prom_size = ((file_size + 0xF) & ~0xF);
  void *prom_data = realloc(file_data, prom_size);
  if (prom_size > 0 && !prom_data) {
    if (file_data)
      free(file_data);
    return GRU_ERROR_MEMORY;
  }
  memset((char*)prom_data + file_size, 0, prom_size - file_size);
  if (file->prom_data)
    free(file->prom_data);
  file->vrom_size = vrom_size;
  file->prom_size = prom_size;
  file->prom_data = prom_data;
  /* shrink padding to accommodate the new data */
  e = z64fs_padding_shrink_forwards(z64fs, index,
                                    z64fs->vrom_volatile ?
                                    0 : vrom_size,
                                    z64fs->prom_volatile ?
                                    0 : prom_size);
  if (e)
    return e;
  return z64fs_update(z64fs);
}

enum gru_error gru_z64fs_remove(struct gru_z64fs *z64fs, size_t index)
{
  if (index >= z64fs->files.size)
    return GRU_ERROR_RANGE;
  if (index == z64fs->ftab_index)
    return GRU_ERROR_PARAM;
  struct z64_file *file = vector_at(&z64fs->files, index);
  /* grow preceding file padding to eat file space,
     or grow succeeding file padding backwards if there's no preceding file */
  enum gru_error e = z64fs_file_eat(z64fs, index,
                                    !z64fs->vrom_volatile,
                                    !z64fs->prom_volatile);
  if (e)
    return e;
  /* decrease order of subsequent files */
  if (z64fs->ftab_index > index)
    --z64fs->ftab_index;
  for (size_t i = 0; i < z64fs->files.size; ++i) {
    struct z64_file *i_file = vector_at(&z64fs->files, i);
    if (i_file == file)
      continue;
    if (i_file->vrom_order >= file->vrom_order)
      --i_file->vrom_order;
    if (i_file->prom_order >= file->prom_order)
      --i_file->prom_order;
  }
  /* remove file */
  if (file->prom_data)
    free(file->prom_data);
  if (file->prom_padding)
    free(file->prom_padding);
  vector_erase(&z64fs->files, index, 1);
  return z64fs_update(z64fs);
}

enum gru_error gru_z64fs_nullify(struct gru_z64fs *z64fs, size_t index)
{
  if (index >= z64fs->files.size)
    return GRU_ERROR_RANGE;
  if (index == z64fs->ftab_index)
    return GRU_ERROR_PARAM;
  struct z64_file *file = vector_at(&z64fs->files, index);
  /* grow preceding file padding to eat file space,
     or grow succeeding file padding backwards if there's no preceding file */
  enum gru_error e = z64fs_file_eat(z64fs, index,
                                    !z64fs->vrom_volatile,
                                    !z64fs->prom_volatile);
  if (e)
    return e;
  /* nullify file */
  if (file->prom_data)
    free(file->prom_data);
  if (file->prom_padding)
    free(file->prom_padding);
  file->prom_start = 0xFFFFFFFF;
  file->prom_end = 0xFFFFFFFF;
  file->prom_size = 0;
  file->prom_data = NULL;
  file->prom_padding_size = 0;
  file->prom_padding = NULL;
  return z64fs_update(z64fs);
}

enum gru_error gru_z64fs_get(struct gru_z64fs *z64fs, size_t index,
                             gru_bool_t decompress, struct gru_blob *blob)
{
  if (index >= z64fs->files.size)
    return GRU_ERROR_RANGE;
  struct z64_file *file = vector_at(&z64fs->files, index);
  blob->size = file->prom_size;
  if (decompress && file_is_compressed(file)) {
    blob->data = file->prom_data;
    enum gru_error e = gru_util_yaz0_decode(&blob->data, &blob->size);
    if (e)
      return e;
  }
  else {
    blob->data = malloc(file->prom_size);
    if (file->prom_size > 0 && !blob->data)
      return GRU_ERROR_MEMORY;
    memcpy(blob->data, file->prom_data, file->prom_size);
  }
  return GRU_SUCCESS;
}

enum gru_error gru_z64fs_vstart(struct gru_z64fs *z64fs, size_t index,
                                size_t *vrom_start_out)
{
  if (index >= z64fs->files.size)
    return GRU_ERROR_RANGE;
  struct z64_file *file = vector_at(&z64fs->files, index);
  *vrom_start_out = file->vrom_start;
  return GRU_SUCCESS;
}

enum gru_error gru_z64fs_vend(struct gru_z64fs *z64fs, size_t index,
                              size_t *vrom_end_out)
{
  if (index >= z64fs->files.size)
    return GRU_ERROR_RANGE;
  struct z64_file *file = vector_at(&z64fs->files, index);
  *vrom_end_out = file->vrom_end;
  return GRU_SUCCESS;
}

enum gru_error gru_z64fs_vsize(struct gru_z64fs *z64fs, size_t index,
                               size_t *vrom_size_out)
{
  if (index >= z64fs->files.size)
    return GRU_ERROR_RANGE;
  struct z64_file *file = vector_at(&z64fs->files, index);
  *vrom_size_out = file->vrom_size;
  return GRU_SUCCESS;
}

enum gru_error gru_z64fs_set_vsize(struct gru_z64fs *z64fs, size_t index,
                                   size_t vrom_size)
{
  if (index >= z64fs->files.size)
    return GRU_ERROR_RANGE;
  if (vrom_size & 0xF)
    return GRU_ERROR_PARAM;
  struct z64_file *file = vector_at(&z64fs->files, index);
  file->vrom_size = vrom_size;
  return z64fs_update(z64fs);
}

enum gru_error gru_z64fs_vpadding(struct gru_z64fs *z64fs,
                                  size_t index,
                                  size_t *vrom_padding_size_out)
{
  if (index >= z64fs->files.size)
    return GRU_ERROR_RANGE;
  struct z64_file *file = vector_at(&z64fs->files, index);
  *vrom_padding_size_out = file->vrom_padding_size;
  return GRU_SUCCESS;
}

enum gru_error gru_z64fs_set_vpadding(struct gru_z64fs *z64fs,
                                      size_t index,
                                      size_t vrom_padding_size)
{
  if (index >= z64fs->files.size)
    return GRU_ERROR_RANGE;
  if (vrom_padding_size & 0xF)
    return GRU_ERROR_PARAM;
  struct z64_file *file = vector_at(&z64fs->files, index);
  enum gru_error e;
  if (z64fs->vrom_volatile)
    e = z64fs_padding_resize(z64fs, index,
                             vrom_padding_size, file->prom_padding_size,
                             GRU_FALSE);
  else
    e = z64fs_padding_redistribute(z64fs, index,
                                   vrom_padding_size, file->prom_padding_size,
                                   GRU_TRUE);
  if (e)
    return e;
  return z64fs_update(z64fs);
}

enum gru_error gru_z64fs_vspace(struct gru_z64fs *z64fs,
                                size_t index,
                                size_t *vrom_space_out)
{
  if (index >= z64fs->files.size)
    return GRU_ERROR_RANGE;
  struct z64_file *file = vector_at(&z64fs->files, index);
  *vrom_space_out = file->vrom_size + file->vrom_padding_size;
  return GRU_SUCCESS;
}

enum gru_error gru_z64fs_set_vspace(struct gru_z64fs *z64fs,
                                    size_t index, size_t vrom_space)
{
  if (index >= z64fs->files.size)
    return GRU_ERROR_RANGE;
  if (vrom_space & 0xF)
    return GRU_ERROR_PARAM;
  struct z64_file *file = vector_at(&z64fs->files, index);
  if (vrom_space < file->vrom_size)
    return GRU_ERROR_PARAM;
  size_t vrom_padding_size = vrom_space - file->vrom_size;
  enum gru_error e;
  if (z64fs->vrom_volatile)
    e = z64fs_padding_resize(z64fs, index,
                             vrom_padding_size, file->prom_padding_size,
                             GRU_FALSE);
  else
    e = z64fs_padding_redistribute(z64fs, index,
                                   vrom_padding_size, file->prom_padding_size,
                                   GRU_TRUE);
  if (e)
    return e;
  return z64fs_update(z64fs);
}

enum gru_error gru_z64fs_vorder(struct gru_z64fs *z64fs,
                                size_t index,
                                size_t *vrom_order_out)
{
  if (index >= z64fs->files.size)
    return GRU_ERROR_RANGE;
  struct z64_file *file = vector_at(&z64fs->files, index);
  *vrom_order_out = file->vrom_order;
  return GRU_SUCCESS;
}

enum gru_error gru_z64fs_set_vorder(struct gru_z64fs *z64fs,
                                    size_t index,
                                    size_t vrom_order)
{
  if (index >= z64fs->files.size || vrom_order >= z64fs->files.size)
    return GRU_ERROR_RANGE;
  struct z64_file *file = vector_at(&z64fs->files, index);
  if (file->vrom_order == vrom_order)
    return GRU_SUCCESS;
  /* grow preceding file padding to eat file space,
     or grow succeeding file padding backwards if there's no preceding file */
  enum gru_error e = z64fs_file_eat(z64fs, index,
                                    !z64fs->vrom_volatile,
                                    GRU_FALSE);
  if (e)
    return e;
  /* decrease order of old succeeding files,
     increase order of new succeeding files */
  for (size_t i = 0; i < z64fs->files.size; ++i) {
    struct z64_file *i_file = vector_at(&z64fs->files, i);
    if (i_file == file)
      continue;
    if (i_file->vrom_order >= file->vrom_order)
      --i_file->vrom_order;
    if (i_file->vrom_order >= vrom_order)
      ++i_file->vrom_order;
  }
  file->vrom_order = vrom_order;
  /* steal padding from new preceding file and
     shrink it to accommodate the new data */
  if (!z64fs->vrom_volatile) {
    size_t vrom_prev_index;
    if (!gru_z64fs_vprev(z64fs, file->vrom_order, &vrom_prev_index)) {
      struct z64_file *vrom_prev_file = vector_at(&z64fs->files,
                                                  vrom_prev_index);
      file->vrom_padding_size = vrom_prev_file->vrom_padding_size;
      vrom_prev_file->vrom_padding_size = 0;
    }
    else
      file->vrom_padding_size = 0;
    enum gru_error e = z64fs_padding_shrink_forwards(z64fs, index,
                                                     file->vrom_size, 0);
    if (e)
      return e;
  }
  return z64fs_update(z64fs);
}

enum gru_error gru_z64fs_pstart(struct gru_z64fs *z64fs, size_t index,
                                size_t *prom_start_out)
{
  if (index >= z64fs->files.size)
    return GRU_ERROR_RANGE;
  struct z64_file *file = vector_at(&z64fs->files, index);
  *prom_start_out = file->prom_start;
  return GRU_SUCCESS;
}

enum gru_error gru_z64fs_pend(struct gru_z64fs *z64fs, size_t index,
                              size_t *prom_end_out)
{
  if (index >= z64fs->files.size)
    return GRU_ERROR_RANGE;
  struct z64_file *file = vector_at(&z64fs->files, index);
  *prom_end_out = file->prom_end;
  return GRU_SUCCESS;
}

enum gru_error gru_z64fs_ptail(struct gru_z64fs *z64fs, size_t index,
                               size_t *prom_tail_out)
{
  if (index >= z64fs->files.size)
    return GRU_ERROR_RANGE;
  struct z64_file *file = vector_at(&z64fs->files, index);
  if (file_is_null(file))
    return GRU_ERROR_PARAM;
  *prom_tail_out = file->prom_start + file->prom_size;
  return GRU_SUCCESS;
}

enum gru_error gru_z64fs_psize(struct gru_z64fs *z64fs, size_t index,
                               size_t *prom_size_out)
{
  if (index >= z64fs->files.size)
    return GRU_ERROR_RANGE;
  struct z64_file *file = vector_at(&z64fs->files, index);
  *prom_size_out = file->prom_size;
  return GRU_SUCCESS;
}

enum gru_error gru_z64fs_ppadding(struct gru_z64fs *z64fs,
                                  size_t index,
                                  size_t *prom_padding_size_out)
{
  if (index >= z64fs->files.size)
    return GRU_ERROR_RANGE;
  struct z64_file *file = vector_at(&z64fs->files, index);
  *prom_padding_size_out = file->prom_padding_size;
  return GRU_SUCCESS;
}

enum gru_error gru_z64fs_get_ppadding(struct gru_z64fs *z64fs,
                                      size_t index,
                                      struct gru_blob *blob)
{
  if (index >= z64fs->files.size)
    return GRU_ERROR_RANGE;
  struct z64_file *file = vector_at(&z64fs->files, index);
  blob->size = file->prom_padding_size;
  blob->data = malloc(blob->size);
  if (blob->size > 0 && !blob->data)
    return GRU_ERROR_MEMORY;
  memcpy(blob->data, file->prom_padding, file->prom_padding_size);
  return GRU_SUCCESS;
}

enum gru_error gru_z64fs_set_ppadding(struct gru_z64fs *z64fs,
                                      size_t index,
                                      struct gru_blob *blob)
{
  if (index >= z64fs->files.size)
    return GRU_ERROR_RANGE;
  struct z64_file *file = vector_at(&z64fs->files, index);
  if (file_is_null(file))
    return GRU_ERROR_PARAM;
  size_t prom_padding_size = (blob ? blob->size : 0);
  if (prom_padding_size & 0xF)
    return GRU_ERROR_PARAM;
  enum gru_error e;
  if (z64fs->prom_volatile)
    e = z64fs_padding_resize(z64fs, index,
                             file->vrom_padding_size, prom_padding_size,
                             GRU_FALSE);
  else
    e = z64fs_padding_redistribute(z64fs, index,
                                   file->vrom_padding_size, prom_padding_size,
                                   GRU_TRUE);
  if (e)
    return e;
  if (blob)
    memcpy(file->prom_padding, blob->data, blob->size);
  return z64fs_update(z64fs);
}

enum gru_error gru_z64fs_pspace(struct gru_z64fs *z64fs,
                                size_t index,
                                size_t *prom_space_out)
{
  if (index >= z64fs->files.size)
    return GRU_ERROR_RANGE;
  struct z64_file *file = vector_at(&z64fs->files, index);
  *prom_space_out = file->prom_size + file->prom_padding_size;
  return GRU_SUCCESS;
}

enum gru_error gru_z64fs_set_pspace(struct gru_z64fs *z64fs,
                                    size_t index, size_t prom_space)
{
  if (index >= z64fs->files.size)
    return GRU_ERROR_RANGE;
  struct z64_file *file = vector_at(&z64fs->files, index);
  if (file_is_null(file))
    return GRU_ERROR_PARAM;
  if (prom_space < file->prom_size || (prom_space & 0xF))
    return GRU_ERROR_PARAM;
  size_t prom_padding_size = prom_space - file->prom_size;
  enum gru_error e;
  if (z64fs->prom_volatile)
    e = z64fs_padding_resize(z64fs, index,
                             file->vrom_padding_size, prom_padding_size,
                             GRU_FALSE);
  else
    e = z64fs_padding_redistribute(z64fs, index,
                                   file->vrom_padding_size, prom_padding_size,
                                   GRU_TRUE);
  if (e)
    return e;
  return z64fs_update(z64fs);
}

enum gru_error gru_z64fs_porder(struct gru_z64fs *z64fs,
                                size_t index,
                                size_t *prom_order_out)
{
  if (index >= z64fs->files.size)
    return GRU_ERROR_RANGE;
  struct z64_file *file = vector_at(&z64fs->files, index);
  *prom_order_out = file->prom_order;
  return GRU_SUCCESS;
}

enum gru_error gru_z64fs_set_porder(struct gru_z64fs *z64fs,
                                    size_t index,
                                    size_t prom_order)
{
  if (index >= z64fs->files.size || prom_order >= z64fs->files.size)
    return GRU_ERROR_RANGE;
  struct z64_file *file = vector_at(&z64fs->files, index);
  if (file->prom_order == prom_order)
    return GRU_SUCCESS;
  /* grow preceding file padding to eat file space,
     or grow succeeding file padding backwards if there's no preceding file */
  enum gru_error e = z64fs_file_eat(z64fs, index,
                                    GRU_FALSE,
                                    !z64fs->prom_volatile);
  if (e)
    return e;
  /* decrease order of old succeeding files,
     increase order of new succeeding files */
  for (size_t i = 0; i < z64fs->files.size; ++i) {
    struct z64_file *i_file = vector_at(&z64fs->files, i);
    if (i_file == file)
      continue;
    if (i_file->prom_order >= file->prom_order)
      --i_file->prom_order;
    if (i_file->prom_order >= prom_order)
      ++i_file->prom_order;
  }
  file->prom_order = prom_order;
  /* steal padding from new preceding file and
     shrink it to accommodate the new data */
  if (!z64fs->prom_volatile) {
    if (file->prom_padding)
      free(file->prom_padding);
    size_t prom_prev_index;
    if (!gru_z64fs_pprev(z64fs, file->prom_order, &prom_prev_index)) {
      struct z64_file *prom_prev_file = vector_at(&z64fs->files,
                                                  prom_prev_index);
      file->prom_padding_size = prom_prev_file->prom_padding_size;
      file->prom_padding = prom_prev_file->prom_padding;
      prom_prev_file->prom_padding_size = 0;
      prom_prev_file->prom_padding = NULL;
    }
    else {
      file->prom_padding_size = 0;
      file->prom_padding = NULL;
    }
    enum gru_error e = z64fs_padding_shrink_forwards(z64fs, index,
                                                     0, file->prom_size);
    if (e)
      return e;
  }
  return z64fs_update(z64fs);
}

enum gru_error gru_z64fs_set_index(struct gru_z64fs *z64fs, size_t index,
                                   size_t new_index)
{
  if (index >= z64fs->files.size || new_index >= z64fs->files.size)
    return GRU_ERROR_RANGE;
  if (index == new_index)
    return GRU_SUCCESS;
  struct z64_file *p_file = vector_at(&z64fs->files, index);
  struct z64_file file = *p_file;
  vector_erase(&z64fs->files, index, 1);
  if (!vector_insert(&z64fs->files, new_index, 1, &file)) {
    if (file.prom_data)
      free(file.prom_data);
    if (file.prom_padding)
      free(file.prom_padding);
    return GRU_ERROR_MEMORY;
  }
  if (z64fs->ftab_index == index)
    z64fs->ftab_index = new_index;
  return z64fs_update(z64fs);
}

enum gru_error gru_z64fs_compressed(struct gru_z64fs *z64fs,
                                    size_t index,
                                    gru_bool_t *compressed_out)
{
  if (index >= z64fs->files.size)
    return GRU_ERROR_RANGE;
  struct z64_file *file = vector_at(&z64fs->files, index);
  *compressed_out = file_is_compressed(file);
  return GRU_SUCCESS;
}

enum gru_error gru_z64fs_set_compressed(struct gru_z64fs *z64fs,
                                        size_t index,
                                        gru_bool_t compressed)
{
  if (index >= z64fs->files.size)
    return GRU_ERROR_RANGE;
  if (index == z64fs->ftab_index)
    return GRU_ERROR_PARAM;
  struct z64_file *file = vector_at(&z64fs->files, index);
  file->prom_end = (compressed ? 0x00000001 : 0x00000000);
  return z64fs_update(z64fs);
}

enum gru_error gru_z64fs_null(struct gru_z64fs *z64fs,
                              size_t index,
                              gru_bool_t *null_out)
{
  if (index >= z64fs->files.size)
    return GRU_ERROR_RANGE;
  struct z64_file *file = vector_at(&z64fs->files, index);
  *null_out = file_is_null(file);
  return GRU_SUCCESS;
}
