#ifndef VECTOR_H
#define VECTOR_H

#include <math.h>

typedef struct {
	float x, y;
} vector_t;

/* create new vector_t */
vector_t vector_new(float x, float y) {
	return (vector_t) {
		.x = x,
		.y = y
	};
}

/* returns a + b */
vector_t vector_add(const vector_t a, const vector_t b) {
	return (vector_t) {
		.x = a.x + b.x,
		.y = a.y + b.y
	};
}

/* returns a - b */
vector_t vector_sub(const vector_t a, const vector_t b) {
	return (vector_t) {
		.x = a.x - b.x,
		.y = a.y - b.y
	};
}

/* returns a * b */
vector_t vector_mult(const vector_t a, const vector_t b) {
	return (vector_t) {
		.x = a.x * b.x,
		.y = a.y * b.y
	};
}

/* returns a * v where v is a scalar value */
vector_t vector_smult(const vector_t a, const float v) {
	return (vector_t) {
		.x = a.x * v,
		.y = a.y * v
	};
}

/* returns a / b */
vector_t vector_div(const vector_t a, const vector_t b) {
	return (vector_t) {
		.x = a.x / b.x,
		.y = a.y / b.y
	};
}

/* returns a / b where v is a scalar value */
vector_t vector_sdiv(const vector_t a, const float v) {
	return (vector_t) {
		.x = a.x / v,
		.y = a.y / v
	};
}

/* returns the length of vector */
const float vector_len(const vector_t a) {
	return sqrt(a.x*a.x + a.y*a.y);
}

/* returns a normalized vector */
vector_t vector_normalize(const vector_t a) {
	const float len = vector_len(a);
	return vector_new(a.x / len, a.y / len);
}

/* rotates the vector in radians */
void vector_rotate(vector_t *a, const float theta) {
	a->x = a->x * cos(theta) - a->y * sin(theta);
	a->y = a->x * sin(theta) + a->y * cos(theta);
}

/* makes the vector fixed length */
void vector_setlen(vector_t *a, const float len) {
	const vector_t vec = vector_smult(vector_normalize(*a), len);
	a->x = vec.x;
	a->y = vec.y;
}

/* returns a vector with dir `a` and len `len` */
vector_t vector_tolen(const vector_t a, const float len) {
	return vector_smult(vector_normalize(a), len);
}

/* limits the length of `vector` to `max_len` */
void vector_limit(vector_t *a, const float max_len) {
	if (vector_len(*a) > max_len)
		vector_setlen(a, max_len);
}

#endif


