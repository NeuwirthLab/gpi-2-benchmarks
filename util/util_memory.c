#include "util_memory.h"

#include "check.h"

void
allocate_gaspi_memory(const gaspi_segment_id_t id, const size_t size,
                      const char c)
{
  GASPI_CHECK(gaspi_segment_create(id, size, GASPI_GROUP_ALL, GASPI_BLOCK,
                                   GASPI_MEM_UNINITIALIZED));
  void* ptr;
  GASPI_CHECK(gaspi_segment_ptr(id, &ptr));
  memset(ptr, c, size);
  GASPI_CHECK(gaspi_barrier(GASPI_GROUP_ALL, GASPI_BLOCK));
}

void
allocate_pinned_gaspi_memory(const gaspi_segment_id_t id, const size_t size,
                             const char c)
{
  void* ptr = malloc(size);
  memset(ptr, c, size);
  mlock(ptr, size);
  GASPI_CHECK(gaspi_segment_use(id, ptr, size, GASPI_GROUP_ALL, GASPI_BLOCK,
                                GASPI_MEM_INITIALIZED));
}

// allocate zeroed memory segments
void
allocate_gaspi_memory_initialized(const gaspi_segment_id_t id,
                                  const size_t size)
{
  GASPI_CHECK(gaspi_segment_create(id, size, GASPI_GROUP_ALL, GASPI_BLOCK,
                                   GASPI_MEM_INITIALIZED));
}

void
free_gaspi_memory(const gaspi_segment_id_t id)
{
  GASPI_CHECK(gaspi_barrier(GASPI_GROUP_ALL, GASPI_BLOCK));
  GASPI_CHECK(gaspi_segment_delete(id));
}

void
allocate_memory(void** buffer, const size_t size)
{
  int r;
  size_t alignment = sysconf(_SC_PAGESIZE);
  r = posix_memalign(buffer, alignment, size);
  if(r < 0)
  {
    fprintf(stderr, "posix_memalign failed with %i", r);
    exit(EXIT_FAILURE);
  }
}

void
free_memory(void* buffer)
{
  free(buffer);
}
