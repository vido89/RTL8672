#ifndef HELPERS_H_
#define HELPERS_H_

/* Special functions */
typedef struct replacement_t
{
	const char *key;
	const char *value;
} replacement_t;

extern const char *replaceWords(const char *key, const replacement_t *table);
extern const char *normalizeSize(long long *size);

#endif /* _HELPERS_H */

