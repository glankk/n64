#ifndef CODEGEN_H
#define CODEGEN_H

void *generate_setup(uint32_t payload_entrypoint, uint32_t patch_addr);
void *generate_2x_receive();
void *generate_bulk_receive();

#endif
