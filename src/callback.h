
#ifndef CALLBACK_H
#define CALLBACK_H

typedef enum {
    TYPE_VOID,
    TYPE_UINT32,
    TYPE_INT32,
    TYPE_FLOAT32,
    TYPE_STRING
} var_type_t;

typedef struct callback_info_t {
    char name[256];
    var_type_t return_type;
    void *func;
} callback_info_t;


internal callback_info_t
get_callback_info(const char *name, u32 len,
                  var_type_t r_type, 
                  void *func)
{
    callback_info_t info = {0};
    
    memcpy(info.name, name, len);
    
    info.return_type = r_type;
    
    info.func = func;
    
    return info;
}

void
call_func_callback(callback_info_t *info,
                   void *r_value,
                   void *args)
{
    if (info->func == NULL) return;
    
    switch(info->return_type)
    {
        case TYPE_VOID: {
            void (*func)(void *) = info->func;
            func(args);
        } break;
        
        case TYPE_UINT32: {
            u32 (*func)(void *) = info->func;
            *(u32 *)r_value = func(args);
        } break;
        
        case TYPE_INT32: {
            i32 (*func)(void *) = info->func;
            *(i32 *)r_value = func(args);
        } break;
        
        case TYPE_FLOAT32: {
            f32 (*func)(void *) = info->func;
            *(f32 *)r_value = func(args);
        } break;
        
        case TYPE_STRING: {
            const char *(*func)(void *) = info->func;
            const char *str = func(args);
            memcpy(r_value, (void *)str, strlen(str)+1);
        } break;
        
        default: {
            ASSERT_LOG(0, "Unsupported function return type");
        } break;
    }
}



#endif //CALLBACK_H
