#ifndef GRU_Z64FS_H
#define GRU_Z64FS_H
#include <stddef.h>
#include <stdint.h>
#include <vector/vector.h>
#include "gru.h"
#include "blob.h"
#include "n64rom.h"

struct gru_z64fs
{
  struct vector files;
  size_t        ftab_index;
  gru_bool_t    ftab_vvolatile;
  gru_bool_t    ftab_pvolatile;
  size_t        vrom_size;
  size_t        prom_size;
  gru_bool_t    vrom_volatile;
  gru_bool_t    prom_volatile;
};

enum gru_error gru_z64fs_init(struct gru_z64fs *z64fs);
void gru_z64fs_destroy(struct gru_z64fs *z64fs);
enum gru_error gru_z64fs_load(struct gru_z64fs *z64fs, struct gru_blob *blob,
                              size_t *ftab_start_in);
enum gru_error gru_z64fs_assemble_blob(struct gru_z64fs *z64fs,
                                       struct gru_blob *blob);
enum gru_error gru_z64fs_assemble_rom(struct gru_z64fs *z64fs,
                                      struct gru_n64rom *n64rom);
size_t gru_z64fs_length(struct gru_z64fs *z64fs);
size_t gru_z64fs_ftab(struct gru_z64fs *z64fs);
gru_bool_t gru_z64fs_ftab_vvolatile(struct gru_z64fs *z64fs);
void gru_z64fs_set_ftab_vvolatile(struct gru_z64fs *z64fs,
                                  gru_bool_t ftab_vvolatile);
gru_bool_t gru_z64fs_ftab_pvolatile(struct gru_z64fs *z64fs);
void gru_z64fs_set_ftab_pvolatile(struct gru_z64fs *z64fs,
                                  gru_bool_t ftab_pvolatile);
size_t gru_z64fs_vrom_first(struct gru_z64fs *z64fs);
size_t gru_z64fs_vrom_last(struct gru_z64fs *z64fs);
size_t gru_z64fs_prom_first(struct gru_z64fs *z64fs);
size_t gru_z64fs_prom_last(struct gru_z64fs *z64fs);
size_t gru_z64fs_vrom_size(struct gru_z64fs *z64fs);
size_t gru_z64fs_vrom_tail(struct gru_z64fs *z64fs);
size_t gru_z64fs_prom_size(struct gru_z64fs *z64fs);
enum gru_error gru_z64fs_set_prom_size(struct gru_z64fs *z64fs,
                                       size_t prom_size);
size_t gru_z64fs_prom_tail(struct gru_z64fs *z64fs);
gru_bool_t gru_z64fs_vvolatile(struct gru_z64fs *z64fs);
void gru_z64fs_set_vvolatile(struct gru_z64fs *z64fs, gru_bool_t vrom_volatile);
gru_bool_t gru_z64fs_pvolatile(struct gru_z64fs *z64fs);
void gru_z64fs_set_pvolatile(struct gru_z64fs *z64fs, gru_bool_t prom_volatile);
enum gru_error gru_z64fs_vfind(struct gru_z64fs *z64fs,
                               size_t vrom_start, size_t vrom_end,
                               size_t *index_out);
enum gru_error gru_z64fs_pfind(struct gru_z64fs *z64fs,
                               size_t prom_start, size_t prom_end,
                               size_t *index_out);
enum gru_error gru_z64fs_vat(struct gru_z64fs *z64fs,
                             size_t vrom_address, size_t *index_out);
enum gru_error gru_z64fs_pat(struct gru_z64fs *z64fs,
                             size_t prom_address, size_t *index_out);
enum gru_error gru_z64fs_vindex(struct gru_z64fs *z64fs,
                                size_t vrom_order, size_t *index_out);
enum gru_error gru_z64fs_pindex(struct gru_z64fs *z64fs,
                                size_t prom_order, size_t *index_out);
enum gru_error gru_z64fs_vprev(struct gru_z64fs *z64fs,
                               size_t index, size_t *index_out);
enum gru_error gru_z64fs_vnext(struct gru_z64fs *z64fs,
                               size_t index, size_t *index_out);
enum gru_error gru_z64fs_pprev(struct gru_z64fs *z64fs,
                               size_t index, size_t *index_out);
enum gru_error gru_z64fs_pnext(struct gru_z64fs *z64fs,
                               size_t index, size_t *index_out);
enum gru_error gru_z64fs_insert(struct gru_z64fs *z64fs,
                                struct gru_blob *blob,
                                gru_bool_t compress);
enum gru_error gru_z64fs_replace(struct gru_z64fs *z64fs, size_t index,
                                 struct gru_blob *blob,
                                 gru_bool_t compress);
enum gru_error gru_z64fs_remove(struct gru_z64fs *z64fs, size_t index);
enum gru_error gru_z64fs_nullify(struct gru_z64fs *z64fs, size_t index);
enum gru_error gru_z64fs_get(struct gru_z64fs *z64fs, size_t index,
                             gru_bool_t decompress, struct gru_blob *blob);
enum gru_error gru_z64fs_vstart(struct gru_z64fs *z64fs, size_t index,
                                size_t *vrom_start_out);
enum gru_error gru_z64fs_vend(struct gru_z64fs *z64fs, size_t index,
                              size_t *vrom_end_out);
enum gru_error gru_z64fs_vsize(struct gru_z64fs *z64fs, size_t index,
                               size_t *vrom_size_out);
enum gru_error gru_z64fs_set_vsize(struct gru_z64fs *z64fs, size_t index,
                                   size_t vrom_size);
enum gru_error gru_z64fs_vpadding(struct gru_z64fs *z64fs,
                                  size_t index,
                                  size_t *vrom_padding_size_out);
enum gru_error gru_z64fs_set_vpadding(struct gru_z64fs *z64fs,
                                      size_t index,
                                      size_t vrom_padding_size);
enum gru_error gru_z64fs_vspace(struct gru_z64fs *z64fs,
                                size_t index,
                                size_t *vrom_space_out);
enum gru_error gru_z64fs_set_vspace(struct gru_z64fs *z64fs,
                                    size_t index, size_t vrom_space);
enum gru_error gru_z64fs_vorder(struct gru_z64fs *z64fs,
                                size_t index,
                                size_t *vrom_order_out);
enum gru_error gru_z64fs_set_vorder(struct gru_z64fs *z64fs,
                                    size_t index,
                                    size_t vrom_order);
enum gru_error gru_z64fs_pstart(struct gru_z64fs *z64fs, size_t index,
                                size_t *prom_start_out);
enum gru_error gru_z64fs_pend(struct gru_z64fs *z64fs, size_t index,
                              size_t *prom_end_out);
enum gru_error gru_z64fs_ptail(struct gru_z64fs *z64fs, size_t index,
                               size_t *prom_tail_out);
enum gru_error gru_z64fs_psize(struct gru_z64fs *z64fs, size_t index,
                               size_t *prom_size_out);
enum gru_error gru_z64fs_ppadding(struct gru_z64fs *z64fs,
                                  size_t index,
                                  size_t *prom_padding_size_out);
enum gru_error gru_z64fs_get_ppadding(struct gru_z64fs *z64fs,
                                      size_t index,
                                      struct gru_blob *blob);
enum gru_error gru_z64fs_set_ppadding(struct gru_z64fs *z64fs,
                                      size_t index,
                                      struct gru_blob *blob);
enum gru_error gru_z64fs_pspace(struct gru_z64fs *z64fs,
                                size_t index,
                                size_t *prom_space_out);
enum gru_error gru_z64fs_set_pspace(struct gru_z64fs *z64fs,
                                    size_t index, size_t prom_space);
enum gru_error gru_z64fs_porder(struct gru_z64fs *z64fs,
                                size_t index,
                                size_t *prom_order_out);
enum gru_error gru_z64fs_set_porder(struct gru_z64fs *z64fs,
                                    size_t index,
                                    size_t prom_order);
enum gru_error gru_z64fs_set_index(struct gru_z64fs *z64fs, size_t index,
                                   size_t new_index);
enum gru_error gru_z64fs_compressed(struct gru_z64fs *z64fs,
                                    size_t index,
                                    gru_bool_t *compressed_out);
enum gru_error gru_z64fs_set_compressed(struct gru_z64fs *z64fs,
                                        size_t index,
                                        gru_bool_t compressed);
enum gru_error gru_z64fs_null(struct gru_z64fs *z64fs,
                              size_t index,
                              gru_bool_t *null_out);

#endif
