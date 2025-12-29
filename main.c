/**
 * @file main.c
 * @brief Azan Alarm Application for Linux
 * @details Fetches prayer times for Tehran, schedules Azan playback for Dhuhr and Maghrib using cron, and plays MP3 using mpg123.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>

/**
 * @brief Callback for libcurl to write data.
 */
size_t write_callback(void *ptr, size_t size, size_t nmemb, void *data) {
    size_t realsize = size * nmemb;
    char **response = (char **)data;
    *response = realloc(*response, strlen(*response) + realsize + 1);
    if (*response == NULL) {
        return 0;
    }
    strncat(*response, ptr, realsize);
    return realsize;
}

/**
 * @brief Fetches prayer times from the API.
 * @return JSON string or NULL on failure.
 */
char* fetch_prayer_times() {
    CURL *curl;
    CURLcode res;
    char *response = calloc(1, 1);

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, "https://prayer.aviny.com/api/prayertimes/1");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        if (res != CURLE_OK) {
            free(response);
            return NULL;
        }
    }
    return response;
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

