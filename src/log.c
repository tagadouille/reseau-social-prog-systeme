#include "log.h"
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdarg.h>
#include <time.h>

static void vlog(const char *file, const char *format, va_list args)
{
    int fd = open(file, O_WRONLY | O_CREAT | O_APPEND, 0644);

    if (fd == -1)
    {
        perror("Failed to open log file");
        return;
    }

    // Get current time
    time_t now = time(NULL);

    struct tm tm_now;
    // Convert to local time
    localtime_r(&now, &tm_now);

    // Write timestamp in HH:MM:SS format
    char s_now[9]; // HH:MM:SS
    strftime(s_now, sizeof s_now, "%H:%M:%S", &tm_now);

    dprintf(fd, "[%s] ", s_now);

    // Write the formatted log message
    vdprintf(fd, format, args);

    dprintf(fd, "\n");

    close(fd);
}

void log_client(const char *format, ...)
{
    va_list args;
    va_start(args, format);

    vlog("client.log", format, args);

    va_end(args);
}

void log_server(const char *format, ...)
{
    va_list args;
    va_start(args, format);

    vlog("server.log", format, args);

    va_end(args);
}