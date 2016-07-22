#ifndef _CONFIG_h
#define _CONFIG_h

#include "os.h"
#include "jsmn/jsmn.h"
#include <sys/time.h>


struct stress_config {
   char  hostname[100];
   char  vpn_name[100];
   char  username[100];
   char  password[100];
   char  topic[100];
   int warmup_count;
   int run_count;
   int threshold;
   int payload_size_bytes;
   int is_blocking;
   int is_send_direct;
};

typedef struct stress_config stress_config;

void
parse_config (stress_config* config, char* config_filename);

void
print_analysis(char* role, char* action, struct timeval* action_times, int times_count, int threshold);

#endif
