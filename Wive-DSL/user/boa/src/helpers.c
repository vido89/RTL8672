#include "helpers.h"
#include <string.h>

// Sizes table
const char *normalSizes[] =
{
	"B",
	"kB",
	"MB",
	"GB",
	"TB",
	"PB",
	"HB",
	"ZB"
};

// Make replacement by table, return NULL if no match
const char *replaceWords(const char *key, const replacement_t *table)
{
	// Make replacement by table
	for (; table->key != NULL; table++)
	{
		// Check if key matches
		if (strcmp(key, table->key)==0)
			return table->value;
	}
	return NULL;
}

// Normalize size
const char *normalizeSize(long long *size)
{
	const char **result = normalSizes;
	while ((*size)>(128*1024))
	{
		*size /= 1024;
		result++;
	}
	return *result;
}
