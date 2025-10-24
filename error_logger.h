
#ifndef ERROR_LOGGER_H
#define ERROR_LOGGER_H

#include <stdio.h>

void log_semantic_error(const char *message, int line);

int print_errors_to_file(const char *filename);

int get_semantic_error_count();

#endif