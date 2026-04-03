#ifndef LOG_H
#define LOG_H

/**
 * @brief Logs a message to the server log.
 * @param format The format string (printf-style).
 * @param ... Additional arguments to format the message.
 */
void log_server(const char *format, ...);

/**
 * @brief Logs a message to the client log.
 * @param format The format string (printf-style).
 * @param ... Additional arguments to format the message.
 */
void log_client(const char *format, ...);

#endif