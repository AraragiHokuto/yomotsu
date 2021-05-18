#ifndef RENZAN_STRING_H_
#define RENZAN_STRING_H_




#define KSTRLEN_MAX_SIZE_UNKNOWN 1048576

size_t  kstrlen(size_t max_scan, const char *str);
boolean kstrequ(size_t max_scan, const char *a, const char *b);
size_t  kstrcpy(size_t max_copy, char *dst, const char *src);

/* DJBX33A hash */
size_t kstr_hash(const char *src, size_t strlen);

void kmemcpy(void *dst, const void *src, size_t size);
void kmemmov(void *dst, const void *src, size_t size);
void kmemset(void *dst, byte c, size_t size);

#endif /* RENZAN_STRING_H_ */
