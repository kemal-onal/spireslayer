#include <string.h>
#include <stdlib.h>
#include "parser.h"

// Reserved Keywords
static const char *RESERVED[] = {
    "Ironclad", "gains", "gold", "max", "hp", "card", "relic", "potion",
    "buys", "for", "removes", "upgraded", "upgrades", "enters", "room",
    "learns", "is", "effective", "against", "fights", "heals", "takes",
    "damage", "discards", "sells", "marks", "as", "exhaust",
    "Total", "Floor", "Where", "Deck", "size", "Relics", "Potions",
    "What", "Defeated", "Health", "Exhausts", "Exit",
    NULL
};

// Room Types
static const char *ROOMS[] = {
    "Monster", "Elite", "Rest", "Shop", "Treasure", "Event", "Boss",
    NULL
};

/* Checks If A Char Is Alphabetic
 * Checks if the given character is between A-Z or a-z.
 */
static int is_alpha(char c){
    if((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')){
        return 1;
    }
    return 0;
}

/* Checks If A Char Is Digit
 * Checks if the given character is between 0-9.
 */
static int is_digit(char c){
    if(c >= '0' && c <= '9'){
        return 1;
    }
    return 0;
}

/* Checks If A Word Only Contains Letters
 * Iterates through the word and checks if every character is alphabetic.
 */
static int is_only_letters(const char* word, int len){
    for(int i = 0; i < len; i++){
        if(!is_alpha(word[i])){
            return 0;
        }
    }
    return 1;
}

/* Checks If A Word Is Reserved
 * Iterates through the RESERVED array to see if the word matches any reserved keyword.
 */
static int is_reserved_keyword(const char *word, int length){
    for(int i = 0; RESERVED[i] != NULL; i++){
        if(strlen(RESERVED[i]) == (size_t)length && strncmp(word, RESERVED[i], (size_t)length) == 0){
            return 1;
        }
    }
    return 0;
}

/* Standardize The Line By Deleting Newlines
 * Deletes '\n' or '\r' at the end of the line if they are present.
 */
void chomp_newline(char *line) {
    size_t len = strlen(line);

    if (len > 0 && line[len - 1] == '\n') {
        line[--len] = '\0';
    }
    if (len > 0 && line[len - 1] == '\r') {
        line[len - 1] = '\0';
    }
}

/* Trims The Line
 * Basically deletes every whitespace present at the beginning and the end of the string.
 */
char* trim_line(char *line){
    if(!line){
        return NULL;
    }
    while(*line == ' '){
        line++;
    }
    if(*line == '\0'){
        return line;
    }
    char *end = line + strlen(line) - 1;
    while(end > line && *end == ' '){
        *end = '\0';
        end--;
    }
    return line;
}

/* Skips Spaces Until Any Other Char Is Found
 * Advances the cursor pointer past any spaces.
 */
void skip_spaces(Parser *p){
    while(*(p->cursor) == ' '){
        p->cursor++;
    }
}

/* Initializes The Parser Struct
 * Points the cursor to the input line to start parsing.
 */
void init_parser(Parser *p, char *line){
    p->cursor = line;
}

/* Parses A Specified Word
 * Compares every single char of the cursor with the specified word. 
 * Checks for trailing non-whitespace or non-null chars at the end.
 */
int parse_keyword(Parser *p, const char *keyword){
    skip_spaces(p);
    size_t length = strlen(keyword);
    if(strncmp(p->cursor, keyword, length) == 0){
        char next_char = p->cursor[length];
        if(next_char == ' ' || next_char == '\0'){
            p->cursor += length;
            return 1;
        }
    }
    return 0;
}

/* Parses A Positive Integer
 * Checks for leading zero which is invalid. Also checks for trailing 
 * non-whitespace or non-null chars at the end.
 */
int parse_pos_int(Parser *p, int *value){
    skip_spaces(p);
    if(*(p->cursor) == '0' || !is_digit(*(p->cursor))){
        return 0;
    }
    char *pEnd;
    long val = strtol(p->cursor, &pEnd, 10);
    if(val <= 0){
        return 0;
    }
    if(*pEnd != ' ' && *pEnd != '\0'){
        return 0;
    }
    *value = (int) val;
    p->cursor = pEnd;
    return 1;
}

/* Parses An Unknown Name
 * Single whitespaces are allowed but checks for multiple whitespaces or null chars and terminates. 
 * Therefore leads parser to invalid by remaining an extra word in that case.
 */
int parse_name(Parser *p, char *name){
    skip_spaces(p);
    int word_count = 0;
    name[0] = '\0';
    while(*(p->cursor) != '\0'){
        char *word = p->cursor;
        int length = 0;
        while(word[length] != ' ' && word[length] != '\0'){
            length++;
        }
        if(length == 0){
            break;
        }
        if(!is_only_letters(word, length)){
            break;
        }
        if(is_reserved_keyword(word, length)){
            break;
        }
        if(word_count > 0){
            strcat(name, " ");
        }
        strncat(name, word, length);
        word_count++;
        p->cursor += length;
        if(*(p->cursor) == ' '){
            if(*(p->cursor + 1) == ' '){
                break;
            }
            p->cursor++;
        }
    }
    return (word_count > 0) ? 1 : 0;
}

/* Parses Room Name
 * Checks constant roomtype array as it should contain the name.
 */
int parse_room(Parser *p, char *name){
    skip_spaces(p);
    name[0] = '\0';
    int length = 0;
    while(*(p->cursor + length) != '\0' && *(p->cursor + length) != ' '){
        length++;
    }
    if(length == 0){
        return 0;
    }
    for(int i = 0; ROOMS[i] != NULL; i++){
        if(strlen(ROOMS[i]) == (size_t)length && strncmp(ROOMS[i], p->cursor, (size_t)length) == 0){
            strncpy(name, p->cursor, (size_t)length);
            name[length] = '\0';
            p->cursor += length;
            return 1;
        }
    }
    return 0;
}