
#if _WIN32
// Windows #defines min/max. This stops it from doing so.
#define NOMINMAX
// Reduce header size.
#define WIN32_LEAN_AND_MEAN
#endif

#ifdef __linux__
#define X11
#endif

#ifdef __APPLE__
#define OSX
#endif

typedef uint64_t size_t;
