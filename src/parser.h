#ifndef PARSER_H
#define PARSER_H

/* The Parser Struct
 * Uses a simple cursor pointer to traverse the input string in-place.
 */
typedef struct {
    char *cursor;
} Parser;

// Parser Function Declarations
void chomp_newline(char *line);
char* trim_line(char *line);
void skip_spaces(Parser *p);
void init_parser(Parser *p, char *line);
int parse_keyword(Parser *p, const char *keyword);
int parse_pos_int(Parser *p, int *value);
int parse_name(Parser *p, char *name);
int parse_room(Parser *p, char *name);

#endif