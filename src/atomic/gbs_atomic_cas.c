#include "check.h"
#include "stopwatch.h"
#include "util.h"
#include "util_memory.h"

int
main(int argc, char* argv[])
{
  gaspi_rank_t my_id, num_pes;
  size_t size;
  int i, j;
  int bo_ret = OPTIONS_OKAY;
  struct measurements_t measurements;
  double time;
  char *old, *new;

  options.type = ATOMIC;
  options.subtype = LAT;
  options.name = "gbs_atomic_cas";

  bo_ret = benchmark_options(argc, argv);

  init_comm_library(&my_id, &num_pes);

  switch(bo_ret)
  {
  case OPTIONS_BAD_USAGE:
    if (my_id == 0)
    {
      print_bad_usage();
      print_help_message();
    }
    return EXIT_FAILURE;
  case OPTIONS_HELP:
    if (my_id == 0)
    {
      print_help_message();
    }
    return EXIT_SUCCESS;
  }

  if(num_pes > 2)
  {
    fprintf(stderr, "Benchmark requires exactly two processes!\n");
    return EXIT_FAILURE;
  }

  old = malloc(options.iterations * sizeof(char));
  new = malloc(options.iterations * sizeof(char));
  measurements.time = malloc(options.iterations * sizeof(double));
  measurements.n = options.iterations;

  const gaspi_segment_id_t segment_id = 0;
  const gaspi_queue_id_t q_id = 0;
  gaspi_pointer_t ptr;
  size_t new_value = 1;
  size_t comparator = 0;
  size_t old_value;

  print_header(my_id);

  allocate_gaspi_memory_initialized(segment_id, sizeof(size_t));
  GASPI_CHECK(gaspi_segment_ptr(segment_id, &ptr));
  for(i = 0; i < options.iterations + options.skip; ++i)
  {
    if(my_id == 0)
    {
      if(i >= options.skip)
      {
        time = Wtime();
      }
      GASPI_CHECK(gaspi_atomic_compare_swap(
          segment_id, 0, 1, (gaspi_atomic_value_t)comparator,
          (gaspi_atomic_value_t)new_value, (gaspi_atomic_value_t*)&old_value,
          GASPI_BLOCK));
      if(i >= options.skip)
      {
        measurements.time[i - options.skip] = Wtime() - time;
      }
      comparator = new_value++;
    }
  }

  if(my_id == 0 && options.verify)
  {
    GASPI_CHECK(gaspi_read(segment_id, 0, 1, segment_id, 0, sizeof(size_t), 0,
                           GASPI_BLOCK));
    GASPI_CHECK(gaspi_wait(0, GASPI_BLOCK));
    size_t expected_counter_val = options.iterations + options.skip;
    size_t actual_counter_val = *((size_t*)ptr);

    if(actual_counter_val != expected_counter_val)
    {
      fprintf(stderr, "Error: expected result is %ld but actual result is %ld\n",
              expected_counter_val, actual_counter_val);
      return EXIT_FAILURE;
    }
  }

  GASPI_CHECK(gaspi_barrier(q_id, GASPI_BLOCK));

  print_atomic_lat(my_id, measurements);
  free_gaspi_memory(segment_id);
  free(measurements.time);
  finalize_comm_library();
  return EXIT_SUCCESS;
}
