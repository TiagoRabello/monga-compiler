#ifndef UTIL_H
#define UTIL_H

typedef struct _string_list_node string_list_node;
struct _string_list_node
{
  const char       *string;
  string_list_node *next;
};

#endif