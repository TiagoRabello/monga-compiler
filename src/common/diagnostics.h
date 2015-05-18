#ifndef DIAGNOSTICS_H
#define DIAGNOSTICS_H

#include <stdio.h>

#define RED_LABEL    "\033[0;31m"
#define YELLOW_LABEL "\033[0;33m"
#define GREEN_LABEL  "\033[0;32m"
#define CYAN_LABEL   "\033[0;36m"
#define NORMAL_LABEL "\033[0m"

#define ERROR_LABEL   RED_LABEL"Error: "NORMAL_LABEL
#define WARNING_LABEL YELLOW_LABEL"Warning: "NORMAL_LABEL
#define INFO_LABEL    CYAN_LABEL"Note: "NORMAL_LABEL

#define PRINTF_HEADER(fmt,...) printf("\n---------------------------------------------------------------\n"     \
                                      fmt                                                                       \
                                      "\n---------------------------------------------------------------\n\n", ##__VA_ARGS__);

#endif