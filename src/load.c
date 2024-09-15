

internal void
init_loaded_code(loaded_code_t *code, void **func_table,
                 const char **func_names, u32 func_count,
                 const char *build_dir, const char *dll_name,
                 const char *ext)
{
    memset(code, 0, sizeof(*code));
    
    strcpy(code->build_dir, build_dir);
    strcpy(code->dll_name, dll_name);
    cstr_cat_many(code->temp_dll_name, dll_name, "_temp");
    strcpy(code->ext, ext);
    
    code->func_count = func_count;
    code->func_names = func_names;
    code->funcs = func_table;
}

internal void
free_loaded_code(loaded_code_t *code)
{
    if (!code->is_valid && code->dll) {
        unload_library(code->dll);
        code->dll = 0;
    }
    
    memset(code->funcs, 0, code->func_count*sizeof(*code->funcs));
}

internal void
load_code(loaded_code_t *code)
{
    char number[5];
    char dll_path[MAX_PATH], temp_dll_path[MAX_PATH];
    dll_path[0] = 0;
    
    cstr_cat_many(dll_path, code->build_dir,
                  "/", code->dll_name, ".", code->ext);
    LOG("DLL_PATH: %s", dll_path);
    
    for (u32 attempt = 0; attempt < 128; attempt++)
    {
        temp_dll_path[0] = 0;
        sprintf(number, "%d", code->temp_dll_num);
        
        cstr_cat_many(temp_dll_path, code->build_dir,
                      "/", code->temp_dll_name, number,
                      ".", code->ext);
        LOG("TEMP_DLL_PATH: %s", temp_dll_path);
        
        if (++code->temp_dll_num >= 1024)
            code->temp_dll_num = 0;
        
        if (!copy_file(dll_path, temp_dll_path))
            break;
    }
    
    code->dll = load_library(temp_dll_path);
    if (code->dll)
    {
        code->is_valid = 1;
        for (u32 func_idx = 0;
             func_idx < code->func_count;
             func_idx++)
        {
            void *func = load_func(code->dll, code->func_names[func_idx]);
            
            if (func) {
                LOG("LOADED %s", code->func_names[func_idx]);
                code->funcs[func_idx] = func;
            }
            else code->is_valid = 0;
        }
    }
    
    if (!code->is_valid && code->dll) {
        unload_library(code->dll);
        code->dll = 0;
    }
}