
#ifdef __cplusplus
extern "C" {
#endif

extern void* memcpy(void*, const void*, size_t);
extern void* memmove(void*, const void*, size_t);
extern int memcmp(const void*, const void*, size_t);
void* memset(void*, int, size_t);

#ifdef __cplusplus
}  // extern "C"
#endif
