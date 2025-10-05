#ifndef ARENA__ARENA_H
#define ARENA__ARENA_H

#include <stddef.h>
#include <stdint.h>

typedef struct Arena {
	uint8_t *base;
	size_t size;
	size_t index;
} Arena;
typedef size_t arena_index;

#define arena_i(_a, _i, _T) ((_T *)&((_a)->base[(_i)]))

Arena *arena_make(size_t init_size);
void arena_free(Arena *arena);

arena_index arena_alloc(Arena *arena, size_t size);

#endif /* ARENA__ARENA_H */
