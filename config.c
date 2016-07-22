/*
 *  Config. See Readme.md
 */

#include "config.h"
#include <string.h>
#include "jsmn/jsmn.h"

static int 
jsoneq (const char *json, jsmntok_t *tok, const char *s) {
    if (tok->type == JSMN_STRING && (int) strlen(s) == tok->end - tok->start &&
            strncmp(json + tok->start, s, tok->end - tok->start) == 0) {
        return 0;
    }
    return -1;
}

void
parse_config (stress_config* config, char* config_filename) {
    FILE* file = fopen(config_filename, "r");
    long length;
    if(file) {
        fseek (file, 0, SEEK_END);
        length = ftell (file);
        fseek (file, 0, SEEK_SET);
        char * config_text_buf = malloc (length);
        if (config_text_buf) {
            fread (config_text_buf, 1, length, file);
        }
        fclose (file);

        jsmn_parser p;
        jsmntok_t t[128]; /* We expect no more than 128 tokens */
        jsmn_init(&p);
        int r = jsmn_parse(&p, config_text_buf, strlen(config_text_buf), t, sizeof(t)/sizeof(t[0]));
        
        if (r < 0) {
            printf("Failed to parse JSON: %d\n", r);
            exit;
        }

        /* Assume the top-level element is an object */
        if (r < 1 || t[0].type != JSMN_OBJECT) {
            printf("Object expected\n");
            exit;
        }

        /* Loop over all keys of the root object */
        char util_buf[50];
        char* true_string = "true";
        int i;
        for (i = 1; i < r; i+=2) {
            if (jsoneq(config_text_buf, &t[i], "hostname") == 0) {
                strncpy(config->hostname, config_text_buf + t[i+1].start, t[i+1].end-t[i+1].start);
                config->hostname[t[i+1].end-t[i+1].start] = '\0';
            }
            else if (jsoneq(config_text_buf, &t[i], "vpn") == 0) {
                strncpy(config->vpn_name, config_text_buf + t[i+1].start, t[i+1].end-t[i+1].start);
                config->vpn_name[t[i+1].end-t[i+1].start] = '\0';
            }
            else if (jsoneq(config_text_buf, &t[i], "username") == 0) {
                strncpy(config->username, config_text_buf + t[i+1].start, t[i+1].end-t[i+1].start);
                config->username[t[i+1].end-t[i+1].start] = '\0';
            }
            else if (jsoneq(config_text_buf, &t[i], "password") == 0) {
                strncpy(config->password, config_text_buf + t[i+1].start, t[i+1].end-t[i+1].start);
                config->password[t[i+1].end-t[i+1].start] = '\0';
            }
            else if (jsoneq(config_text_buf, &t[i], "topic") == 0) {
                strncpy(config->topic, config_text_buf + t[i+1].start, t[i+1].end-t[i+1].start);
                config->topic[t[i+1].end-t[i+1].start] = '\0';
            }
            else if (jsoneq(config_text_buf, &t[i], "warmups") == 0) {
                strncpy(util_buf, config_text_buf + t[i+1].start, t[i+1].end-t[i+1].start);
                util_buf[t[i+1].end-t[i+1].start] = '\0';
                config->warmup_count =  atoi(util_buf);
            }
            else if (jsoneq(config_text_buf, &t[i], "runs") == 0) {
                strncpy(util_buf, config_text_buf + t[i+1].start, t[i+1].end-t[i+1].start);
                util_buf[t[i+1].end-t[i+1].start] = '\0';
                config->run_count =  atoi(util_buf);
            }
            else if (jsoneq(config_text_buf, &t[i], "threshold") == 0) {
                strncpy(util_buf, config_text_buf + t[i+1].start, t[i+1].end-t[i+1].start);
                util_buf[t[i+1].end-t[i+1].start] = '\0';
                config->threshold =  atoi(util_buf);
            }
            else if (jsoneq(config_text_buf, &t[i], "payload") == 0) {
                strncpy(util_buf, config_text_buf + t[i+1].start, t[i+1].end-t[i+1].start);
                util_buf[t[i+1].end-t[i+1].start] = '\0';
                config->payload_size_bytes = atoi(util_buf);
            }
            // else if (jsoneq(config_text_buf, &t[i], "blocking") == 0) {
            //     strncpy(util_buf, config_text_buf + t[i+1].start, t[i+1].end-t[i+1].start);
            //     util_buf[t[i+1].end-t[i+1].start] = '\0';
            //     printf("blocking: %s \n", util_buf);
            //     config->is_blocking = (strncmp(true_string, util_buf, 5) == 0) ? 1 : 0;
            // }
            else if (jsoneq(config_text_buf, &t[i], "sendDirect") == 0) {
                strncpy(util_buf, config_text_buf + t[i+1].start, t[i+1].end-t[i+1].start);
                util_buf[t[i+1].end-t[i+1].start] = '\0';
                printf("is_send_direct: %s \n", util_buf);
                config->is_send_direct = (strncmp(true_string, util_buf, 5) == 0) ? 1 : 0;
            }
            else {
                printf("Unexpected key: %.*s\n", t[i].end-t[i].start, config_text_buf + t[i].start);
            }
        }
    }
}

void
print_analysis(char* role, char* action, struct timeval* action_times, int times_count, int threshold) {

    
    printf("\n--- %s ---\n Times between %s:\n", role, action);
    int i;
    double time_taken_sum = 0;

    long gte0_lt3_count = 0;
    long gte3_lt6_count = 0;
    long gte6_lt9_count = 0;
    long gte9_lt20_count = 0;
    long gte20_count = 0;

    long over_threshold_count = 0;

    for(i = 1; i < times_count; i++) { 
        long prev_micros = action_times[i - 1].tv_sec * 1000000 + action_times[i - 1].tv_usec;
        long this_micros = action_times[i].tv_sec * 1000000 + action_times[i].tv_usec;
        time_taken_sum += this_micros - prev_micros;

        long time_diff = this_micros - prev_micros;

        if(time_diff < 3) {
            gte0_lt3_count++;
        }
        else if(time_diff >= 3 && time_diff < 6) {
            gte3_lt6_count++;
        }
        else if(time_diff >= 6 && time_diff < 9) {
            gte6_lt9_count++;
        }
        else if(time_diff >= 9 && time_diff < 20) {
            gte9_lt20_count++;
        }
        else if(time_diff >= 20) {
            gte20_count++;
        }

        if (time_diff > threshold) {
            over_threshold_count++;
            printf("Time b/t msg found over %ld us: %ld us\n", threshold, time_diff);
        }
    }

    printf("\n\n Stats: 0-3us: %ld \t 3-6us: %ld \t 6-9us: %ld \t 9-20us: %ld \t 20us+: %ld \t %ldus+: %ld\n", gte0_lt3_count, gte3_lt6_count, gte6_lt9_count, gte9_lt20_count, gte20_count, threshold, over_threshold_count);
}
