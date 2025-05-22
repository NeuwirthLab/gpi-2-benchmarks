#include "check.h"
#include "stopwatch.h"
#include "util.h"
#include "util_memory.h"

int
main(int argc, char* argv[])
{
  gaspi_rank_t my_id, num_pes;
  int i;
  int bo_ret = OPTIONS_OKAY;
  double time;
  struct measurements_t measurements;

  options.type = NOTIFY;
  options.subtype = PINGPONG;
  options.name = "gbs_notification_ping_pong";

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
  gaspi_notification_t notification_val = 1;
  gaspi_notification_id_t notification_id = 0;

  print_header(my_id);

  allocate_gaspi_memory(segment_id, sizeof(char), 'a');
  for(i = 0; i < options.iterations + options.skip; ++i)
  {
    if(my_id == 0)
    {
      if(i >= options.skip)
      {
        time = Wtime();
      }
      GASPI_CHECK(
          gaspi_notify(segment_id, 1, notification_id, notification_val, q_id, GASPI_BLOCK));
      GASPI_CHECK(gaspi_notify_waitsome(segment_id, notification_id, 1, &notification_id,
                                        GASPI_BLOCK));
      if(i >= options.skip)
      {
        measurements.time[i - options.skip] = Wtime() - time;
      }
    }
    else
    {
      GASPI_CHECK(gaspi_notify_waitsome(segment_id, notification_id, 1, &notification_id,
                                        GASPI_BLOCK));
      GASPI_CHECK(gaspi_notify(segment_id, 0, notification_id, notification_val, q_id, GASPI_BLOCK));
    }
    GASPI_CHECK(gaspi_wait(q_id, GASPI_BLOCK));
  }
  print_notify_lat(my_id, measurements);
  free_gaspi_memory(segment_id);
  free(measurements.time);
  finalize_comm_library();
  return EXIT_SUCCESS;
}
