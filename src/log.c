#ifndef LOG_DEF
#define LOG_DEF

/*
 File:          log.c
 Description:   Implementation of file logging.
 Created:       May 5, 2017
 Author:        Matt Mumau
 */

/* System includes */ 
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

/* Application includes */
#include "config_defaults.h"
#include "main.h"
#include "config.h"
#include "string_utils.h"

/* Header */
#include "log.h"

static FILE *logfile = NULL;
bool is_active;

void log_init()
{
    char *filename = (char *) config_get(CONF_LOG_FULLPATH);
    if (!filename)
        APP_ERROR("Log file name is null.", 1);

    logfile = fopen(filename, "w");
    if (!logfile)
        APP_ERROR("Could not open log file.", 1);

    char hdr_line[LOG_LINE_LEN];
    snprintf(hdr_line, sizeof(hdr_line), "Peabot Server: %s", APP_VERSION);

    log_h(hdr_line);
}

void log_write(char *line)
{
    if (!logfile || !line)
        return;

    while (is_active) {} // block while being read from

    is_active = true;

    char line_cpy[LOG_LINE_LEN];
    str_clearcopy(line_cpy, line, sizeof(line_cpy));

    fprintf(logfile, "%s\n", line_cpy);
    fflush (logfile);

    is_active = false;
}

void log_close()
{
    if (!logfile)
        return;
    fclose(logfile);
}

void log_h(char *val)
{
    char lb[128];
    str_fill(lb, '-', sizeof(lb));
    
    log_write(lb);
    log_write(val);
    log_write(lb);
}

void log_br()
{
    char line_break[81];
    for (int i = 0; i < 80; i++)
        line_break[i] = '-';
    line_break[80] = '\0';

    log_write(line_break);
}

void log_event(const char *msg)
{
    time_t current_time = time(NULL);
    struct tm *ltime = localtime(&current_time);

    char timestamp[LOG_TIMESTAMP_MAXLEN];
    strftime(timestamp, LOG_TIMESTAMP_MAXLEN, "[%m-%d-%y %H:%M:%S]", ltime);

    char buffer[LOG_LINE_MAXLEN];
    snprintf(buffer, sizeof(buffer), "%s %s", timestamp, msg);

    log_write(buffer);
}

void log_error(const char *msg, int error_code)
{
    char log_msg[LOG_LINE_LEN];
    snprintf(log_msg, sizeof(log_msg), "[ERROR] %s [e:%d]", msg, error_code);

    log_event(log_msg);
}

int log_getlines(int begin, LogLine *lines, size_t lines_len) 
{
    if (logfile == NULL)
        return 0;

    printf("you see me rollin...\n");

    while (is_active) {} // block while being written to

    printf("they hatin'...\n");

    is_active = true;

    fseek(logfile, 0, SEEK_SET);

    printf("patrollin'...");

    LogLine log_line;
    char *last_str;
    int count = 0;

    printf("These are the [SKIPPED] lines...\n");

    for (int skip = 0; skip < begin; skip++)
    {
        last_str = fgets(log_line, sizeof(log_line), logfile);
        printf("last_str: %s\n", last_str);
        printf("log_line: %s\n", log_line);
        if (last_str == NULL)
            return 0;
    }

    printf("These are the [ACTUAL] lines...\n");
    
    for ( ; count < lines_len; count++) 
    {
        memset(log_line, '\0', sizeof(log_line));
        last_str = fgets(log_line, sizeof(log_line), logfile);
        if (last_str == NULL)
            break;

        str_removenl(log_line);
        str_clearcopy(lines[count], log_line, sizeof(log_lines));
    }

    fseek(logfile, 0, SEEK_END);
    is_active = false;

    return count + 1;
}

#endif