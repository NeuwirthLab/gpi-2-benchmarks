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
  struct measurements_t measurements;

  options.type = PASSIVE;
  options.subtype = BW;
  options.name = "gbs_passive_bw";

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

  gaspi_size_t max_transfer_size;
  GASPI_CHECK(gaspi_passive_transfer_size_max(&max_transfer_size));

  if(options.max_message_size > max_transfer_size)
  {
    if(my_id == 0)
    {
      fprintf(stderr, "Message size was truncated from %ld to %ld!\n",
              options.max_message_size, max_transfer_size);
    }
    options.max_message_size = max_transfer_size;
  }

  measurements.time = malloc(options.iterations * sizeof(double));
  measurements.n = options.iterations;

  const gaspi_segment_id_t segment_id_a = 0;
  const gaspi_segment_id_t segment_id_b = 1;
  gaspi_rank_t remote_id = my_id == 0 ? 1 : 0;
  gaspi_pointer_t ptr;

  print_header(my_id);

  int window_size = options.window_size;
  if(options.single_buffer)
  {
    allocate_gaspi_memory(segment_id_a, options.max_message_size * sizeof(char),
                          my_id == 0 ? 'a' : 'b');
    allocate_gaspi_memory(segment_id_b, sizeof(char), 'a');
    GASPI_CHECK(gaspi_segment_ptr(segment_id_a, &ptr));
  }
  for(size = options.min_message_size; size <= options.max_message_size;
      size *= 2)
  {
    if(!options.single_buffer)
    {
      allocate_gaspi_memory(segment_id_a, size * window_size * sizeof(char),
                            my_id == 0 ? 'a' : 'b');
      allocate_gaspi_memory(segment_id_b, sizeof(char), 'a');
      GASPI_CHECK(gaspi_segment_ptr(segment_id_a, &ptr));
    }
    for(i = 0; i < options.iterations + options.skip; ++i)
    {
      if(i >= options.skip)
      {
        time = Wtime();
      }
      if(my_id == 0)
      {
        for(j = 0; j < window_size; ++j)
        {
          GASPI_CHECK(
              gaspi_passive_send(segment_id_a, options.single_buffer ? 0 : j * size,
                                 1, size, GASPI_BLOCK));
        }
        GASPI_CHECK(gaspi_passive_receive(segment_id_b, 0, &remote_id,
                                          sizeof(char), GASPI_BLOCK));
      }
      else
      {
        for(j = 0; j < window_size; ++j)
        {
          GASPI_CHECK(gaspi_passive_receive(segment_id_a, options.single_buffer ? 0 : j * size,
                                            &remote_id, size, GASPI_BLOCK));
        }
        GASPI_CHECK(gaspi_passive_send(segment_id_b, 0, 0, sizeof(char),
                                        GASPI_BLOCK));
      }
      if(i >= options.skip)
      {
        measurements.time[i - options.skip] = Wtime() - time;
      }
    }
    if(my_id == 1 && options.verify)
    {
      int limit = options.single_buffer ? size : size * window_size;
      for(i = 0; i < limit; ++i)
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
      free_gaspi_memory(segment_id_a);
      free_gaspi_memory(segment_id_b);
    }
  }
  if(options.single_buffer)
  {
    free_gaspi_memory(segment_id_a);
    free_gaspi_memory(segment_id_b);
  }
  free(measurements.time);
  finalize_comm_library();
  return EXIT_SUCCESS;
}