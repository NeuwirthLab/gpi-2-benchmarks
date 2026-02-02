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

  options.type = ONESIDED;
  options.subtype = BW;
  options.name = "gbs_notification_ping_pong";

  bo_ret = benchmark_options(argc, argv);

  switch(bo_ret)
  {
  case OPTIONS_BAD_USAGE:
    print_bad_usage();
    return EXIT_FAILURE;
  case OPTIONS_HELP:
    print_help_message();
    return EXIT_SUCCESS;
  }

  init_comm_library(&my_id, &num_pes);
  if(num_pes > 2)
  {
    fprintf(stderr, "Benchmark requires exactly two processes!\n");
    return EXIT_FAILURE;
  }

  measurements.time = malloc(options.iterations * sizeof(double));
  measurements.n = options.iterations;

  const gaspi_segment_id_t segment_id_dst = 0;
  const gaspi_segment_id_t segment_id_src = 1;
  const gaspi_queue_id_t q_id = 0;
  gaspi_notification_t notification_val = 1;
  gaspi_notification_id_t notification_id = 0;
  gaspi_notification_id_t first_notification_id = 0;
  gaspi_pointer_t ptr_dst;
  gaspi_pointer_t ptr_src;

  print_header(my_id);
  void (*allocate_benchmark_memory)(const gaspi_segment_id_t, const size_t,
                                    const char) =
      options.pin_memory ? allocate_gaspi_memory : allocate_pinned_gaspi_memory;
  
  int window_size = options.window_size;
  if(options.single_buffer)
  {
    allocate_benchmark_memory(segment_id_dst,
                              options.max_message_size * sizeof(char),
                              my_id == 0 ? 'a' : 'b');
    GASPI_CHECK(gaspi_segment_ptr(segment_id_dst, &ptr_dst));

    allocate_benchmark_memory(segment_id_src,
                              options.max_message_size * sizeof(char),
                              my_id == 0 ? 'a' : 'b');
    GASPI_CHECK(gaspi_segment_ptr(segment_id_src, &ptr_src));
  }

  for(size = options.min_message_size; size <= options.max_message_size;
      size *= 2)
  {
    if(!options.single_buffer)
    {
      allocate_benchmark_memory(segment_id_dst, size * window_size * sizeof(char),
                                my_id == 0 ? 'a' : 'b');
      GASPI_CHECK(gaspi_segment_ptr(segment_id_dst, &ptr_dst));

      allocate_benchmark_memory(segment_id_src, size * window_size * sizeof(char),
                                my_id == 0 ? 'a' : 'b');
      GASPI_CHECK(gaspi_segment_ptr(segment_id_src, &ptr_src));
    }
    
    for(notification_id = 0; notification_id < options.iterations + options.skip; ++notification_id)
    {
      
      if(my_id == 0)
      {
        if(notification_id >= options.skip)
        {
          time = Wtime();
        }
        for(j = 0; j < window_size; ++j)
        { 
          GASPI_CHECK(gaspi_write_notify(segment_id_src, options.single_buffer ? 0 : j * size, 1, segment_id_dst, 
                                         options.single_buffer ? 0 : j * size, size, notification_id * window_size + j, 
                                         notification_val, q_id, GASPI_BLOCK));
        
          GASPI_CHECK(gaspi_notify_waitsome(segment_id_dst, notification_id * window_size + j, 1, &first_notification_id,
                                            GASPI_BLOCK));
        }
        GASPI_CHECK(gaspi_wait(q_id, GASPI_BLOCK));
        if(notification_id >= options.skip)
        {
          measurements.time[notification_id - options.skip] = Wtime() - time;
        }
        if(options.verify)
        {
          GASPI_CHECK(gaspi_barrier(GASPI_GROUP_ALL, GASPI_BLOCK));

          int limit = options.single_buffer ? size : size * window_size;
          for(i = 0; i < limit; ++i)
          {
            if(((char*)ptr_dst)[i] != 'b')
            {
              fprintf(stderr, "Verification for Prozess 0 failed. Result is invalid! %i\n", i);
              return EXIT_FAILURE;
            }
          }
        }
      }
      else //id == 1
      {
        for(j = 0; j < window_size; ++j)
        { 
          GASPI_CHECK(gaspi_notify_waitsome(segment_id_dst, notification_id * window_size + j, 1, &first_notification_id,
                                            GASPI_BLOCK));
        

          GASPI_CHECK(gaspi_write_notify(segment_id_src, options.single_buffer ? 0 : j * size, 0, segment_id_dst, 
                                         options.single_buffer ? 0 : j * size, size, notification_id * window_size + j, 
                                         notification_val, q_id, GASPI_BLOCK));
        }
        if(options.verify)
        {
          GASPI_CHECK(gaspi_barrier(GASPI_GROUP_ALL, GASPI_BLOCK));

          int limit = options.single_buffer ? size : size * window_size;
          for(i = 0; i < limit; ++i)
          {
            if(((char*)ptr_dst)[i] != 'a')
            {
              fprintf(stderr, "Verification for Prozess 1 failed. Result is invalid! %i\n", i);
              return EXIT_FAILURE;
            }
          }
        }
        
      }

      GASPI_CHECK(gaspi_wait(q_id, GASPI_BLOCK));
      
    }
    print_result(my_id, measurements, size);
    if(!options.single_buffer)
    {
      free_gaspi_memory(segment_id_src);
      free_gaspi_memory(segment_id_dst);
    }
  }
  //print_notify_lat(my_id, measurements);
  if(options.single_buffer)
  {
    free_gaspi_memory(segment_id_src);
    free_gaspi_memory(segment_id_dst);
  }
  free(measurements.time);
  finalize_comm_library();
  return EXIT_SUCCESS;
}
