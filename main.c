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
    system("mpg123 -f 3500 /home/s4eidhp/azan_alarm/azan.mp3");
}

/**
 * @brief Prints help information.
 */
void print_help() {
    printf("Azan Alarm Application for Linux\n");
    printf("Usage:\n");
    printf("  ./main                 Fetch prayer times and schedule Azan for Dhuhr and Maghrib\n");
    printf("  ./main show            Display all prayer times\n");
    printf("  ./main notify <prayer> Play Azan for the specified prayer (used by cron)\n");
    printf("  ./main -h, --help      Show this help message\n");
}

/**
 * @brief Main entry point.
 */
int main(int argc, char *argv[]) {
    if (argc > 1) {
        if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
            print_help();
        } else if (strcmp(argv[1], "notify") == 0 && argc > 2) {
            play_azan(argv[2]);
        } else if (strcmp(argv[1], "show") == 0) {
            char *json = fetch_prayer_times();
            if (!json) {
                printf("Failed to fetch prayer times.\n");
                return 1;
            }
            char *fajr = extract_time(json, "Imsaak");
            char *sunrise = extract_time(json, "Sunrise");
            char *dhuhr = extract_time(json, "Noon");
            char *asr = extract_time(json, "Sunset");
            char *maghreb = extract_time(json, "Maghreb");
            char *isha = extract_time(json, "Midnight");
            printf("Prayer Times for Tehran:\n");
            printf("Fajr: %s\n", fajr ? fajr : "N/A");
            printf("Sunrise: %s\n", sunrise ? sunrise : "N/A");
            printf("Dhuhr: %s\n", dhuhr ? dhuhr : "N/A");
            printf("Asr: %s\n", asr ? asr : "N/A");
            printf("Maghreb: %s\n", maghreb ? maghreb : "N/A");
            printf("Isha: %s\n", isha ? isha : "N/A");
            free(json);
            free(fajr);
            free(sunrise);
            free(dhuhr);
            free(asr);
            free(maghreb);
            free(isha);
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
        // Schedule cron jobs
        char exe_path[PATH_MAX];
        ssize_t len = readlink("/proc/self/exe", exe_path, sizeof(exe_path) - 1);
        if (len != -1) {
            exe_path[len] = '\0';
            // Remove old Azan entries
            system("crontab -l 2>/dev/null | grep -v 'azan_notify' > /tmp/cron || true");
            FILE *fp = fopen("/tmp/cron", "a");
            if (fp) {
                int hour, min, sec;
                sscanf(dhuhr, "%d:%d:%d", &hour, &min, &sec);
                fprintf(fp, "%d %d * * * %s azan_notify dhuhr\n", min, hour, exe_path);
                sscanf(maghreb, "%d:%d:%d", &hour, &min, &sec);
                fprintf(fp, "%d %d * * * %s azan_notify maghreb\n", min, hour, exe_path);
                fclose(fp);
                system("crontab /tmp/cron");
                system("rm /tmp/cron");
            }
        }
        printf("Scheduled Azan times for today.\n");
        free(json);
        free(dhuhr);
        free(maghreb);
    }
    return 0;
}
