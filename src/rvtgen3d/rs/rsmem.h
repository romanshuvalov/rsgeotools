#ifndef RSMEM_H_INCLUDED
#define RSMEM_H_INCLUDED

#include <inttypes.h>

#include "rsthread.h"


typedef struct rs_mem_chunk_t {

    int32_t pool_index;
    int32_t chunk_index;


} rs_mem_chunk_t;

#define RS_MEM_CHUNK_HDR_LEN (sizeof(rs_mem_chunk_t))

typedef struct rs_mem_pool_t {

    uint32_t status;
    uint32_t chunk_len;
    int32_t chunks_per_chunkset_count;

    int32_t max_chunksets_count;

    int32_t max_chunks_count;

    int32_t allocated_chunks_count;
    int32_t max_allocated_chunks_count;

    uint32_t *chunk_index_numpool;
    uint32_t chunk_index_numpool_pos;

    uint32_t *chunk_statuses;
    unsigned char **chunksets;

} rs_mem_pool_t;

void rs_mem_pool_init( int pool_index, int chunk_len, int chunks_per_chunkset_count, int max_chunks_count );
void rs_mem_pool_term( int pool_index );
void* rs_mem_pool_get_chunk( int pool_index );
void rs_mem_pool_release_chunk( int pool_index, int chunk_index );

#define RS_MEM_MAX_POOLS    24

#define RS_MEM_RESERVED_POOLS   9

typedef struct rs_mem_reg_t {

    rs_mem_pool_t pools[RS_MEM_MAX_POOLS];

    int counter;

    RS_MUTEX_T mutex;

    int total_allocations;
    int max_total_allocations;

    uint64_t max_total_bytes_allocated;

    int total_reallocations;

} rs_mem_reg_t;


#define RS_MEM_POOL_AUTO    (-1)

extern rs_mem_reg_t rs_mem_reg;

void rs_mem_init();
void rs_mem_term();

void rs_mem_print_statistics();

unsigned char* rs_mem_alloc(uint32_t bytes, int pool_index);
unsigned char* rs_mem_alloc_adv(uint32_t bytes, const char *filename, int line, int index);
unsigned char* rs_mem_realloc( void *pointer, uint32_t bytes);
void rs_mem_free(void *pointer);


#endif // RSMEM_H_INCLUDED
