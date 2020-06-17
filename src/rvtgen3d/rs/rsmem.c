#include "rsmem.h"

#include "rsdebug.h"

#include "rsmx.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>


rs_mem_reg_t rs_mem_reg;

void rs_mem_init() {

    DEBUG10f("rs_mem_init() start\n");

    memset(&rs_mem_reg, 0, sizeof(rs_mem_reg_t) );

    RS_MUTEX_INIT( &rs_mem_reg.mutex );

    // 0: 0-255
    rs_mem_pool_init(0, 256, 2048, 512);

    // 1: 256-4095
    rs_mem_pool_init(1, 4096, 1024, 1024);

    // 2: 4096-16383
    rs_mem_pool_init(2, 16384, 256, 1024);

    // 3: 16384-65535
    rs_mem_pool_init(3, 65536, 64, 1024);

    // 4: 65536-262143
    rs_mem_pool_init(4, 262144, 16, 1024);

    // 5: 262144 - 1 MB
    rs_mem_pool_init(5, (1*1024*1024), 16, 1024);

    // 6: 1 MB - 4 MB
    rs_mem_pool_init(6, (4*1024*1024), 4, 1024);

    // 7: 4 MB - 8 MB
    rs_mem_pool_init(7, (8*1024*1024), 4, 1024);

    // 8: 0-64 bytes
    rs_mem_pool_init(8, 64, 8192, 1024);


};

void rs_mem_term() {

    for (int i = 0; i < RS_MEM_MAX_POOLS; i++) {
        if (rs_mem_reg.pools[i].status) {
            rs_mem_pool_term( i );
        };
    };


};


void rs_mem_pool_init( int pool_index, int chunk_len, int chunks_per_chunkset_count, int max_chunksets_count ) {

    rs_mem_pool_t *pool = &rs_mem_reg.pools[pool_index];

    memset(pool, 0, sizeof(rs_mem_pool_t));

    pool->status = 1;

    pool->chunk_len = chunk_len;
    pool->chunks_per_chunkset_count = chunks_per_chunkset_count;
    pool->max_chunksets_count = max_chunksets_count;
    pool->max_chunks_count = max_chunksets_count * chunks_per_chunkset_count;

    pool->chunksets = malloc( sizeof(void*) * max_chunksets_count );
    rs_app_assert_memory_adv(pool->chunksets, "rsmem", __LINE__, sizeof(void*) * max_chunksets_count );

    pool->chunk_statuses = malloc( sizeof(uint32_t) * pool->max_chunks_count );
    rs_app_assert_memory_adv(pool->chunk_statuses, "rsmem", __LINE__, sizeof(uint32_t) * pool->max_chunks_count );

    pool->chunk_index_numpool = malloc( sizeof(uint32_t) * pool->max_chunks_count );
    rs_app_assert_memory_adv(pool->chunk_index_numpool, "rsmem", __LINE__, sizeof(uint32_t) * pool->max_chunks_count );

    memset(pool->chunksets, 0, sizeof(void*) * max_chunksets_count );
    memset(pool->chunk_statuses, 0, sizeof(uint32_t) * pool->max_chunks_count );

    for (int i = 0; i < pool->max_chunks_count; i++) {
        pool->chunk_index_numpool[i] = i;
    };

};

void rs_mem_pool_term( int pool_index ) {

    rs_mem_pool_t *pool = &rs_mem_reg.pools[pool_index];

    for (int i = 0; i < pool->max_chunksets_count; i++) {
        if (pool->chunksets[i]) {
            free(pool->chunksets[i]);
        };
    };

    free(pool->chunksets);
    free(pool->chunk_statuses);

    pool->status = 0;

};

void* rs_mem_pool_get_chunk( int pool_index ) {



    rs_mem_pool_t *pool = &rs_mem_reg.pools[pool_index];

    if (pool->allocated_chunks_count >= pool->max_chunks_count) {
        rs_critical_alert_and_halt_sprintf("Memory pool %d: max_chunks_count", pool_index);
    };

    RS_MUTEX_LOCK(&rs_mem_reg.mutex);

    int i = pool->chunk_index_numpool[ pool->chunk_index_numpool_pos++ ];

    if (pool->chunk_statuses[i]) {
        rs_critical_alert_and_halt_sprintf("pool status is already 1: pool index %d, chunk index %d", pool_index, i);
    };


    pool->allocated_chunks_count++;
    pool->max_allocated_chunks_count = rs_max_i(pool->max_allocated_chunks_count, pool->allocated_chunks_count);

    pool->chunk_statuses[i] = 1;


    int chunkset_i = i / pool->chunks_per_chunkset_count;
    int chunkset_chunk_i = i - (chunkset_i*pool->chunks_per_chunkset_count);


    if (!pool->chunksets[chunkset_i]) {
        uint32_t bytes = (pool->chunk_len + RS_MEM_CHUNK_HDR_LEN) * pool->chunks_per_chunkset_count;

        pool->chunksets[chunkset_i] = malloc( bytes );

        if (!pool->chunksets[chunkset_i]) {
            rs_show_message_sprintf("Cannot allocate %d bytes for pool index #%d\n", bytes, pool_index);

            rs_mem_print_statistics();
            exit(-1);

        };

        DEBUG20f("Allocating %d kB chunkset, pool index %d\n", bytes/1024, pool_index);

        rs_mem_reg.max_total_bytes_allocated += bytes;
        rs_mem_reg.total_allocations++;

    };

    rs_mem_chunk_t *chunk_hdr = (rs_mem_chunk_t*) (   ( (unsigned char*) pool->chunksets[chunkset_i]) + chunkset_chunk_i*(pool->chunk_len + RS_MEM_CHUNK_HDR_LEN)  );

    chunk_hdr->pool_index = pool_index;
    chunk_hdr->chunk_index = i;

    RS_MUTEX_UNLOCK(&rs_mem_reg.mutex);


    return ( ((unsigned char*)chunk_hdr) + RS_MEM_CHUNK_HDR_LEN );


};

void rs_mem_pool_release_chunk( int pool_index, int chunk_index ) {


    rs_mem_pool_t *pool = &rs_mem_reg.pools[pool_index];

    RS_MUTEX_LOCK(&rs_mem_reg.mutex);

        if (pool->chunk_statuses[ chunk_index ] != 1) {
            rs_critical_alert_and_halt_sprintf("DOUBLE RELEASE: pool %d, chunk %d, pool->chunk_index_numpool_pos %d status %d\n", pool_index, chunk_index, pool->chunk_index_numpool_pos, pool->chunk_statuses[ chunk_index ]);
        };

        pool->chunk_index_numpool[--pool->chunk_index_numpool_pos] = chunk_index;

        pool->allocated_chunks_count--;
        pool->chunk_statuses[ chunk_index ] = 0;

    RS_MUTEX_UNLOCK(&rs_mem_reg.mutex);

};



unsigned char* rs_mem_alloc(uint32_t bytes, int pool_index) {
    return rs_mem_alloc_adv(bytes, "", 0, pool_index);
};

unsigned char* rs_mem_alloc_adv(uint32_t bytes, const char *filename, int line, int pool_index) {


    RS_UNUSED_PARAM(filename);
    RS_UNUSED_PARAM(line);


    if (pool_index == RS_MEM_POOL_AUTO) {
        if (bytes <= 64) {
            pool_index = 8;
        }
        else if (bytes <= 256) {
            pool_index = 0;
        }
        else if (bytes <= 4096) {
            pool_index = 1;
        }
        else if (bytes <= 16384) {
            pool_index = 2;
        }
        else if (bytes <= 65536) {
            pool_index = 3;
        }
        else if (bytes <= 262144) {
            pool_index = 4;
        }
        else if (bytes <= 1*1024*1024) {
            pool_index = 5;
        }
        else if (bytes <= 4*1024*1024 ) {
            pool_index = 6;
        }
        else if (bytes <= 8*1024*1024 ) {
            pool_index = 7;
        }
    };


    if (pool_index != RS_MEM_POOL_AUTO) {
        if ( rs_mem_reg.pools[pool_index].chunk_len >= bytes ) {
            return rs_mem_pool_get_chunk( pool_index );
        }
        else {
            rs_show_message_sprintf("WARNING\npool #%d chunk_len = %d, but we are allocating %d\n", pool_index, rs_mem_reg.pools[pool_index].chunk_len, bytes);
        };
    };

    // Dynamic allocation:

    pool_index = RS_MEM_POOL_AUTO;

    RS_MUTEX_LOCK(&rs_mem_reg.mutex);

        rs_mem_reg.counter++;

        rs_mem_reg.total_allocations++;
        rs_mem_reg.max_total_allocations = rs_max_i(rs_mem_reg.max_total_allocations, rs_mem_reg.total_allocations);


        unsigned char *p = malloc( bytes + RS_MEM_CHUNK_HDR_LEN );

        if (!p) {

            rs_show_message_sprintf("Cannot allocate %d bytes\nrsmem:%d", bytes + RS_MEM_CHUNK_HDR_LEN, __LINE__);

            rs_mem_print_statistics();

            exit(-1);

        };

        rs_mem_chunk_t *p_hdr = (rs_mem_chunk_t*) p;

        p_hdr->pool_index = RS_MEM_POOL_AUTO;


    RS_MUTEX_UNLOCK(&rs_mem_reg.mutex);

    return (p + RS_MEM_CHUNK_HDR_LEN);
};

unsigned char* rs_mem_realloc( void *pointer_void, uint32_t bytes) {

    unsigned char *pointer = (unsigned char*) pointer_void;

    unsigned char *p = pointer - RS_MEM_CHUNK_HDR_LEN;

    rs_mem_chunk_t *p_hdr = (rs_mem_chunk_t*) p;



    if (p_hdr->pool_index != RS_MEM_POOL_AUTO) {

        uint32_t chunk_len = rs_mem_reg.pools[p_hdr->pool_index].chunk_len;

        if ( chunk_len >= bytes ) {
            // Current chunk is big enough for new size.
            return pointer_void;
        };

        unsigned char *p_new = rs_mem_alloc(bytes, RS_MEM_POOL_AUTO);
        memcpy(p_new, pointer_void, chunk_len); // bytes>chunk_len so it's guaranteed that we have enough memory

        rs_mem_free(pointer_void);

        return p_new;

    };


    // dynamic

    RS_MUTEX_LOCK(&rs_mem_reg.mutex);

        rs_mem_reg.total_reallocations++;
        rs_mem_reg.max_total_allocations = rs_max_i(rs_mem_reg.max_total_allocations, rs_mem_reg.total_allocations);


    RS_MUTEX_UNLOCK(&rs_mem_reg.mutex);

    p = realloc( p, bytes + RS_MEM_CHUNK_HDR_LEN );
    p_hdr = (rs_mem_chunk_t*) p;

    p_hdr->pool_index = RS_MEM_POOL_AUTO;

    return (p + RS_MEM_CHUNK_HDR_LEN);

};

void rs_mem_free(void *pointer_void) {


    unsigned char *pointer = (unsigned char*) pointer_void;

    unsigned char *p = (pointer - RS_MEM_CHUNK_HDR_LEN);

    rs_mem_chunk_t *p_hdr = (rs_mem_chunk_t*) p;

    if (p_hdr->pool_index != RS_MEM_POOL_AUTO) {

        rs_mem_pool_release_chunk( p_hdr->pool_index, p_hdr->chunk_index );

        return;
    };

    // dynamic

    RS_MUTEX_LOCK(&rs_mem_reg.mutex);

        rs_mem_reg.total_allocations--;
        free(p);

    RS_MUTEX_UNLOCK(&rs_mem_reg.mutex);
};


void rs_mem_print_statistics() {

    char s[8192];
    s[0] = 0;

    char t[1024];

    uint64_t total = 0;

    for (int i = 0; i < 13; i++) {
        sprintf(t, "%d) (current %d) (chunk_len %d kb) x (max allocations %d) = %d kb\n", i, rs_mem_reg.pools[i].allocated_chunks_count,
            rs_mem_reg.pools[i].chunk_len/1024, rs_mem_reg.pools[i].max_allocated_chunks_count,
            rs_mem_reg.pools[i].chunk_len * rs_mem_reg.pools[i].max_allocated_chunks_count / 1024 );
        strcat(s, t);
        total += rs_mem_reg.pools[i].chunk_len * rs_mem_reg.pools[i].max_allocated_chunks_count / 1024;
    };


    sprintf(t, "\n\nTOTAL: %d MB\n\nDynamic: total_allocations: %d, max total allocations %d\nmax total allocated bytes %d MB\n\ntotal reallocations %d\n",
        (uint32_t)(total/1024),
        (uint32_t)rs_mem_reg.total_allocations, (uint32_t)rs_mem_reg.max_total_allocations,
        (uint32_t)(rs_mem_reg.max_total_bytes_allocated/1024/1024),
        (uint32_t)rs_mem_reg.total_reallocations);


    strcat(s, t);

    DEBUG10f("%s", s);


};



