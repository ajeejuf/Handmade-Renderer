
internal void
create_app_memory(app_memory_t *app_mem, u64 p_size, u64 t_size) {
    u64 size = 2*sizeof(memory_arena_t)+p_size+t_size;
    u8 *mem = malloc(size);/*VirtualAlloc(NULL, size,
                                            MEM_RESERVE|MEM_COMMIT,
                                            PAGE_READWRITE);*/
    
    app_mem->perm_arena = (memory_arena_t *)mem;
    mem += sizeof(memory_arena_t);
    init_memory_arena(app_mem->perm_arena, (void *)mem, p_size);
    mem += p_size;
    
    app_mem->temp_arena = (memory_arena_t *)mem;
    mem += sizeof(memory_arena_t);
    init_memory_arena(app_mem->temp_arena, (void *)mem, t_size);
}

internal void
init_app_memory(app_memory_t *out_mem, app_memory_t *in_mem,
                u64 p_size, u64 t_size)
{
    u64 np_size = p_size-sizeof(memory_arena_t);
    u64 nt_size = t_size-sizeof(memory_arena_t);
    
    u8 *mem = arena_push(in_mem->perm_arena, p_size);
    out_mem->perm_arena = (memory_arena_t *)mem;
    mem += sizeof(memory_arena_t);
    init_memory_arena(out_mem->perm_arena, (void *)mem, np_size);
    
    mem = arena_push(in_mem->temp_arena, t_size);
    out_mem->temp_arena = (memory_arena_t *)mem;
    mem += sizeof(memory_arena_t);
    init_memory_arena(out_mem->temp_arena, (void *)mem, nt_size);
}