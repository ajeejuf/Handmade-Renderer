
#ifndef LOAD_H
#define LOAD_H

typedef struct loaded_code_t {
    void *dll;
    
    char build_dir[MAX_PATH];
    char dll_name[MAX_PATH];
    char temp_dll_name[MAX_PATH];
    char ext[6]; // TODO(ajeej): make sure this holds any ext that will be loaded
    char slash[2];
    u32 temp_dll_num;
    
    u32 func_count;
    const char **func_names;
    void **funcs;
    
    u32 is_valid;
} loaded_code_t;

#endif //LOAD_H
