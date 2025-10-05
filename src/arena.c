#include "arena/arena.h"

#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>

Arena *arena_make(size_t init_size)
{
	Arena *ret = calloc(1, sizeof(Arena));
	ret->base = malloc(ret->size = init_size);
	ret->index = 0;

	return ret;
}

void arena_free(Arena *arena)
{
	free(arena->base);
	free(arena);
}

#define ARENA_REALLOC_FACTOR 2
inline arena_index arena_alloc(Arena *arena, size_t size)
{
	if (arena->index + size > arena->size) {
		uint8_t *new_base = realloc(arena->base, arena->size *= ARENA_REALLOC_FACTOR);
		if (new_base == NULL)
			return SIZE_MAX;
		arena->base = new_base;
	}
	
	size_t ret_index = arena->index;
	arena->index += size;

	return ret_index;
}
