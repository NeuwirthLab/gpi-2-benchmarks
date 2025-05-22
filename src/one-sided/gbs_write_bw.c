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
  double time;

  options.type = ONESIDED;
  options.subtype = BW;
  options.name = "gbs_write_bw";

  struct measurements_t measurements;

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

  measurements.time = malloc(options.iterations * sizeof(double));
  measurements.n = options.iterations;

  const gaspi_segment_id_t segment_id = 0;
  const gaspi_queue_id_t q_id = 0;
  gaspi_pointer_t ptr;

  print_header(my_id);

  void (*allocate_benchmark_memory)(const gaspi_segment_id_t, const size_t,
                                    const char) =
      options.pin_memory ? allocate_gaspi_memory : allocate_pinned_gaspi_memory;

  int window_size = options.window_size;
  if(options.single_buffer)
  {
    allocate_benchmark_memory(segment_id,
                              options.max_message_size * sizeof(char),
                              my_id == 0 ? 'a' : 'b');
    GASPI_CHECK(gaspi_segment_ptr(segment_id, &ptr));
  }

  for(size = options.min_message_size; size <= options.max_message_size;
      size *= 2)
  {
    if(!options.single_buffer)
    {
      allocate_benchmark_memory(segment_id, size * window_size * sizeof(char),
                                my_id == 0 ? 'a' : 'b');
      GASPI_CHECK(gaspi_segment_ptr(segment_id, &ptr));
    }
    if(my_id == 0)
    {
      for(i = 0; i < options.iterations + options.skip; ++i)
      {
        if(i >= options.skip)
        {
          time = Wtime();
        }
        for(j = 0; j < window_size; ++j)
        {
          GASPI_CHECK(gaspi_write(segment_id, options.single_buffer ? 0 : j * size, 1,
                                  segment_id,  options.single_buffer ? 0 : j * size, 
                                  size, q_id, GASPI_BLOCK));
        }
        GASPI_CHECK(gaspi_wait(q_id, GASPI_BLOCK));
        if(i >= options.skip)
        {
          measurements.time[i - options.skip] = Wtime() - time;
        }
      }
    }
    if(options.verify)
    {
      GASPI_CHECK(gaspi_barrier(GASPI_GROUP_ALL, GASPI_BLOCK));
    }
    if(my_id == 1 && options.verify)
    {
      for(i = 0; i < options.single_buffer ? size : size * window_size; ++i)
      {
        if(((char*)ptr)[i] != 'a')
        {
          fprintf(stderr, "Verification failed. Result is invalid!\n");
          return EXIT_FAILURE;
        }
      }
    }
    print_result(my_id, measurements, size);
    if(!options.single_buffer)
    {
      free_gaspi_memory(segment_id);
    }
  }
  if(options.single_buffer)
  {
    free_gaspi_memory(segment_id);
  }
  free(measurements.time);
  finalize_comm_library();
  return EXIT_SUCCESS;
}