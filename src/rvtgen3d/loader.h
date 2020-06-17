#ifndef RS_GLOADER_H
#define RS_GLOADER_H

#define RVT_APP_LOADER_FORMAT_PLY      1


void loader_load_all_vbodata();

void loader_create_vbodata(int vbodata_index, char* filename, int format_type);

void loader_create_vbodata_ply(int vbodata_index, char* filename);


#endif

