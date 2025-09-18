#ifndef DVEC_H
#define DVEC_H

#include <stdlib.h>
#include <string.h>

#define dvec_t(type) \
	struct { \
		size_t size; \
		size_t capacity; \
		type *items; \
	}

#define DVEC_INIT { .size=0, .capacity=0, .items=NULL }
#define dv_init(v) ((v).size = 0, (v).capacity = 0, (v).items = NULL)

#define dv_destroy(v) \
	(free((v).items), (v).size = (v).capacity = 0)


#define _dv_type(v) typeof(*(v).items)
#define _dv_item_size(v) (sizeof(*(v).items))

#define _dv_resize(v, n) \
	(((n) > (v).capacity) \
		? (((v).capacity = (n), (v).items = realloc((v).items, _dv_item_size(v)*(n))) ? (v).capacity : ((v).capacity=0)) \
	 	: (((n) > (v).size) ? n : (v).capacity))
#define dv_resize(v, n) ((v).size = _dv_resize((v), (n)))

#define dv_shrink(v) (_dv_resize((v), (v).size))

#define dv_copy_to(v, i, ptr, n) \
	((_dv_resize((v), (i)+(n))) \
		? (memcpy((v).items+(i), (ptr), (n)*_dv_item_size(v)), (v).size=(v).capacity, 1) \
		: 0)

#define dv_concat_b_on_a(va, vb) \
	(dv_copy_to((va), (va).size, (vb).items, (vb).size))
#define dv_sconcat_b_on_a(va, vb) \
	(dv_copy_to((va), (va).size-1, (vb).items, (vb).size))

#define dv_push(v, val) \
	((_dv_resize((v), (v).size+1)) \
		? (((v).items[(v).size++] = (val)), 1) \
		: 0)

#define dv_pop(v) \
	((_dv_resize((v), (v).size+1)) \
		? ((v).items[--(v).size]) \
		: ((typeof(*(v).items)){}))
#define dv_peek(v) \
	(((v).size > 0) \
		? &(v).items[(v).size-1] \
		: NULL)

/* the `i`th element of `v` */
#define dv_a(v, i) ((v).items[i])

#define dv_n(v) ((v).size)
#define _dv_ptr(v) ((v).items)
#define dv_max(v) ((v).capacity)

#define dv_foreach(v, item_p) \
	for ((item_p) = (v).items; (size_t)((item_p)-(v).items) < (v).size; ++(item_p))

#define dv_assign(dst, src) (memcpy((dst), (src), sizeof(dvec_t(int))))


#endif /* DVEC_H */
