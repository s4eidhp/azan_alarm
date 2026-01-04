/**
 * @file main.c
 * @brief Azan Alarm Application for Linux
 * @details Fetches prayer times for Tehran, schedules Azan playback for Dhuhr and Maghrib using cron, and plays MP3 using mpg123.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <limits.h>

/**
 * @brief Fetches prayer times from the API using curl command.
 * @return JSON string or NULL on failure.
 */
char* fetch_prayer_times() {
    system("curl -s https://prayer.aviny.com/api/prayertimes/1 > /tmp/prayer.json");
    FILE *fp = fopen("/tmp/prayer.json", "r");
    if (!fp) return NULL;
    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    char *json = malloc(size + 1);
    fread(json, 1, size, fp);
    json[size] = '\0';
    fclose(fp);
    return json;
}

/**
 * @brief Extracts a time value from the JSON response.
 * @param json The JSON string.
 * @param key The key (e.g., "Noon").
 * @return The time string or NULL.
 */
char* extract_time(const char *json, const char *key) {
    char search[50];
    sprintf(search, "\"%s\":\"", key);
    char *start = strstr(json, search);
    if (!start) return NULL;
    start += strlen(search);
    char *end = strchr(start, '"');
    if (!end) return NULL;
    size_t len = end - start;
    char *time_str = malloc(len + 1);
    strncpy(time_str, start, len);
    time_str[len] = '\0';
    return time_str;
}

/**
 * @brief Plays the Azan music.
 * @param prayer The prayer name.
 */
void play_azan(const char *prayer) {
    system("mpg123 azan.mp3");
}

/**
 * @brief Schedules a cron job.
 * @param time_str The time in HH:MM:SS.
 * @param arg The prayer name.
 */
void schedule_task(const char *time_str, const char *arg) {
    int hour, min, sec;
    sscanf(time_str, "%d:%d:%d", &hour, &min, &sec);

    char exe_path[PATH_MAX];
    ssize_t len = readlink("/proc/self/exe", exe_path, sizeof(exe_path) - 1);
    if (len == -1) return;
    exe_path[len] = '\0';

    char cron_line[256];
    sprintf(cron_line, "%d %d * * * %s notify %s", min, hour, exe_path, arg);

    // Add to crontab
    system("crontab -l > /tmp/cron 2>/dev/null || true");
    FILE *fp = fopen("/tmp/cron", "a");
    if (fp) {
        fprintf(fp, "%s\n", cron_line);
        fclose(fp);
        system("crontab /tmp/cron");
        system("rm /tmp/cron");
    }
}

/**
 * @brief Main entry point.
 */
int main(int argc, char *argv[]) {
    if (argc > 1) {
        if (strcmp(argv[1], "notify") == 0 && argc > 2) {
            play_azan(argv[2]);
        }
    } else {
        char *json = fetch_prayer_times();
        if (!json) {
            printf("Failed to fetch prayer times.\n");
            return 1;
        }
        char *dhuhr = extract_time(json, "Noon");
        char *maghreb = extract_time(json, "Maghreb");
        if (!dhuhr || !maghreb) {
            printf("Failed to parse times.\n");
            free(json);
            free(dhuhr);
            free(maghreb);
            return 1;
        }
        schedule_task(dhuhr, "dhuhr");
        schedule_task(maghreb, "maghrib");
        printf("Scheduled Azan times for today.\n");
        free(json);
        free(dhuhr);
        free(maghreb);
    }
    return 0;
}

