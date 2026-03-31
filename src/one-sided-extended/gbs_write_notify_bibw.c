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
  char check_val;
  struct measurements_t measurements;

  options.type = ONESIDED;
  options.subtype = BW;
  options.name = "gbs_write_bibw";

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

  const gaspi_segment_id_t segment_id_send = 0;
  const gaspi_segment_id_t segment_id_recv = 1;
  const gaspi_queue_id_t q_id = 0;
  const gaspi_notification_t notification_val = 1;
  const gaspi_notification_id_t notification_id = 0;
  gaspi_notification_id_t first;

  gaspi_pointer_t ptr;

  print_header(my_id);

  int window_size = options.window_size;
  if(options.single_buffer)
  {
    allocate_gaspi_memory(segment_id_send,
                          options.max_message_size * sizeof(char),
                          my_id == 0 ? 'a' : 'b');
    allocate_gaspi_memory(segment_id_recv,
                          options.max_message_size * sizeof(char), 'y');
    GASPI_CHECK(gaspi_segment_ptr(segment_id_recv, &ptr));
    GASPI_CHECK(gaspi_barrier(GASPI_GROUP_ALL, GASPI_BLOCK));
  }
  for(size = options.min_message_size; size <= options.max_message_size;
      size *= 2)
  {
    if(!options.single_buffer)
    {
      allocate_gaspi_memory(segment_id_send, size * window_size * sizeof(char),
                            my_id == 0 ? 'a' : 'b');
      allocate_gaspi_memory(segment_id_recv, size * window_size * sizeof(char),
                            'y');
      GASPI_CHECK(gaspi_segment_ptr(segment_id_recv, &ptr));
      GASPI_CHECK(gaspi_barrier(GASPI_GROUP_ALL, GASPI_BLOCK));

    }
    if(my_id == 0)
    {
      for(i = 0; i < options.iterations + options.skip; ++i)
      {
        GASPI_CHECK(gaspi_barrier(q_id, GASPI_BLOCK));
        if(i >= options.skip)
        {
          time = Wtime();
        }
        for(j = 0; j < window_size; ++j)
        {
          GASPI_CHECK(gaspi_write_notify(
              segment_id_send, options.single_buffer ? 0 : j * size, 1,
              segment_id_recv, options.single_buffer ? 0 : j * size, size,
              notification_id + j, notification_val, q_id, GASPI_TEST));
        }
        GASPI_CHECK(gaspi_wait(q_id, GASPI_BLOCK));
        GASPI_CHECK(gaspi_notify_waitsome(segment_id_recv, notification_id,
                                          window_size, &first, GASPI_BLOCK));
        if(i >= options.skip)
        {
          measurements.time[i - options.skip] = Wtime() - time;
        }
      }
    }
    else if(my_id == 1)
    {
      for(i = 0; i < options.iterations + options.skip; ++i)
      {
        GASPI_CHECK(gaspi_barrier(q_id, GASPI_BLOCK));
        for(j = 0; j < window_size; ++j)
        {
          GASPI_CHECK(gaspi_write_notify(
              segment_id_send, options.single_buffer ? 0 : j * size, 0, 
              segment_id_recv, options.single_buffer ? 0 : j * size, size,
              notification_id + j, notification_val, q_id, GASPI_TEST));
        }
        GASPI_CHECK(gaspi_wait(q_id, GASPI_BLOCK));
        GASPI_CHECK(gaspi_notify_waitsome(segment_id_recv, notification_id,
                                          window_size, &first, GASPI_BLOCK));
      }
    }
    GASPI_CHECK(gaspi_barrier(GASPI_GROUP_ALL, GASPI_BLOCK));
    if(options.verify)
    {
      check_val = my_id == 0 ? 'b' : 'a';
      int limit = options.single_buffer ? size : size * window_size;
      for(i = 0; i < limit; ++i)
      {
        if(((char*)ptr)[i] != check_val)
        {
          fprintf(stderr, "Verification failed. Result is invalid!\n");
          return EXIT_FAILURE;
        }
      }
    }
    print_result(my_id, measurements, size * 2);
    if(!options.single_buffer)
    {
      free_gaspi_memory(segment_id_send);
      free_gaspi_memory(segment_id_recv);
    }
  }
  if(options.single_buffer)
  {
    free_gaspi_memory(segment_id_send);
    free_gaspi_memory(segment_id_recv);
  }
  free(measurements.time);
  finalize_comm_library();
  return EXIT_SUCCESS;
}
