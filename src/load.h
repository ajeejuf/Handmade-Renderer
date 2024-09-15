
#ifndef LOAD_H
#define LOAD_H

typedef dll_t;

typedef struct loaded_code_t {
    dll_t dll;
    
    char build_dir[MAX_PATH];
    char dll_name[MAX_PATH];
    char temp_dll_name[MAX_PATH];
    char ext[6]; // TODO(ajeej): make sure this holds any ext that will be loaded
    u32 temp_dll_num;
    
    u32 func_count;
    const char **func_names;
    void **funcs;
    
    u32 is_valid;
} loaded_code_t;

#endif //LOAD_H
