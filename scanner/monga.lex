%{

#include "monga_tokens.h"
#include <stdio.h>

char * duplicate(const char *str);
int process_scapes(char *str);

#ifndef NDEBUG
#define PRINT_TOKEN(token) printf("<" # token">")
#define PRINT_SINGLE_CHAR_TOKEN() printf("%c", *yytext)
#define PRINT_TOKEN_WITH_TEXT(token) printf("<" # token">[%s]", yytext)
#else
#define PRINT_TOKEN(token) (void)0
#define PRINT_TOKEN_WITH_TEXT(token) (void)0
#endif

%}

%%

"char" {
  PRINT_TOKEN(TK_CHAR_T);
  return TK_CHAR_T;
}

"float" {
  PRINT_TOKEN(TK_FLOAT_T);
  return TK_FLOAT_T;
}

"int" {
  PRINT_TOKEN(TK_INT_T);
  return TK_INT_T;
}

"void" {
  PRINT_TOKEN(TK_VOID_T);
  return TK_VOID_T;
}

"if" {
  PRINT_TOKEN(TK_IF);
  return TK_IF;
}

"else" {
  PRINT_TOKEN(TK_ELSE);
  return TK_ELSE;
}

"while" {
  PRINT_TOKEN(TK_WHILE);
  return TK_WHILE;
}

"new" {
  PRINT_TOKEN(TK_NEW);
  return TK_NEW;
}

"return" {
  PRINT_TOKEN(TK_RETURN);
  return TK_RETURN;
}

"==" {
  PRINT_TOKEN(TK_EQ);
  return TK_EQ;
}

"&&" {
  PRINT_TOKEN(TK_AND);
  return TK_AND;
}

"||" {
  PRINT_TOKEN(TK_OR);
  return TK_OR;
}

"<" {
  PRINT_SINGLE_CHAR_TOKEN();
  return '<';
}

">" {
  PRINT_SINGLE_CHAR_TOKEN();
  return '>';
}

"<=" {
  PRINT_TOKEN(TK_LEQ);
  return TK_LEQ;
}

">=" {
  PRINT_TOKEN(TK_GEQ);
  return TK_GEQ;
}

(\+|\-|\*|\/|\{|\}|\(|\)|\[|\]|\;|\=|\!|\,) {
  PRINT_SINGLE_CHAR_TOKEN();
  return *yytext; 
}

[A-Za-z_][A-Za-z0-9_]* {
  PRINT_TOKEN_WITH_TEXT(TK_ID);
  yyval.string_val = duplicate(yytext);
  return TK_ID;
}

"/*"(.|\n)*"*/" {
  PRINT_TOKEN_WITH_TEXT(TK_COMMENT);
  yyval.string_val = duplicate(yytext);
  return TK_COMMENT;
}

\"(\\n|\\t|\\\"|[^\\\"])*\" {
  PRINT_TOKEN_WITH_TEXT(TK_STRING_LITERAL);
  yyval.string_val = duplicate(yytext);
  process_scapes(yyval.string_val);
  return TK_STRING_LITERAL;
}

[0-9]+ {
  PRINT_TOKEN_WITH_TEXT(TK_INT_LITERAL);
  yyval.int_val = atoi(yytext);
  return TK_INT_LITERAL;
}

0[xX][A-Fa-f0-9]+ {
  PRINT_TOKEN_WITH_TEXT(TK_INT_LITERAL);
  yyval.int_val = atoi(yytext);
  return TK_INT_LITERAL;
}

(\.[0-9]+|[0-9]+\.|[0-9]+\.[0-9]+)[Ff]? {
  PRINT_TOKEN_WITH_TEXT(TK_FLOAT_LITERAL);
  yyval.float_val = atof(yytext);
  return TK_FLOAT_LITERAL;
}

" "|\n|\r|\t {
  printf("%c", *yytext);
}

.  {
  PRINT_TOKEN_WITH_TEXT(?);
  yyval.char_val = *yytext;
  return TK_UNKNOWN;
}

%%

char * duplicate(const char *str)
{
  size_t alloc_size = sizeof(char) * (strlen(str) + 1);
  char * new_str = (char *)yyalloc(alloc_size);
  memcpy(new_str, str, alloc_size);
  return new_str;
}

int process_scapes(char *str)
{
  enum
  {
    READING_CHAR,
    READING_SCAPE
  } state = READING_CHAR;

  str = strchr(str, '\\');

  // No escape processing needed.
  if (str == NULL) { return 0; }

  char * output = str;

  for (; *str != '\0'; ++str)
  {
    switch (state)
    {
      READING_CHAR:

        if (*str != '\\') { *output++ = *str; }
        else { state = READING_SCAPE; }

        break;

      READING_SCAPE:
        switch (*str)
        {
          case 't':
            *output++ = '\t';
            break;
          case 'n':
            *output++ = '\n';
            break;
          case '\"':
            *output++ = '\"';
            break;
          default:
            return 1;
        }
        break;
    }
  }
  return 0;
}

int yywrap()
{
  return 1;
}