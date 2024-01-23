#ifndef _SLB_STRING_H
#define _SLB_STRING_H
#include <stddef.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

typedef struct {
    char *content;
    size_t capacity;
    size_t size;
    size_t cursor_pos;
} Slb_string;


Slb_string* slb_read_entire_file(char *filepath);
void        slb_string_close(Slb_string* string);
void        slb_string_reset_cursor(Slb_string* string);
char*       slb_string_get_next(Slb_string *string, char delim);
void        slb_cstr_trim(char *str);

/* TODO: proper implementation
Slb_string slb_string_create_from_cstr(char *cstr);
void slb_string_append(Slb_string *a, Slb_string *b);
void slb_string_append_cstr(Slb_string *a, char *b);
*/

#ifdef SLB_STRING_IMPLEMENTATION

/*
 *  Reads entire file into a Slb_string struct.
 *  Returns NULL on error.
 *  Returns Slb_string* otherwise.
 *  Slb_string must be freed with slb_string_close().
 */
Slb_string* slb_read_entire_file(char *filepath) {

    // getting file size
    Slb_string *result = malloc(sizeof(Slb_string));
    FILE *fd = fopen(filepath, "r"); if (fd == NULL) return NULL;
    if (fseek(fd, 0, SEEK_END) < 0) return NULL;
    int size = ftell(fd); if (size < 0) return NULL;
    if (fseek(fd, 0, SEEK_SET) < 0) return NULL;
    
    // allocating necessary memory
    result->content = malloc(size + 1);
    result->capacity = size;
    result->size = size;
    result->cursor_pos = 0;

    // copying file content to allocated memory
    fread(result->content, result->size, 1, fd);
    fclose(fd);

    return result;
}

/*
 *  Resets cursor position to 0.
 */
void slb_string_reset_cursor(Slb_string* string) {
    if (string != NULL) string->cursor_pos = 0;
}

/*
 *  Returns next string in Slb_string struct.
 *  Delimiter is used to determine where string ends.
 *  Returns NULL if no string is found.
 *  Returned string must be freed with free().
 */
char* slb_string_get_next(Slb_string *string, char delim) {
    size_t start = string->cursor_pos;
    while (string->content[string->cursor_pos] != delim) {
        if (string->cursor_pos >= string->size) return NULL;
        string->cursor_pos++;
    }
    size_t end = string->cursor_pos;
    
    char *result = malloc(end - start + 1);
    strncpy(result, string->content + start, end - start);
    result[end - start] = '\0';
    string->cursor_pos++;
    return result;
}

/*
 *  Frees memory allocated by slb_read_entire_file().
 */
void slb_string_close(Slb_string* string) {
    free(string->content);
    free(string);
}

void slb_cstr_trim(char *str) {
    int i = 0;
    int j = strlen(str) - 1;
    while (str[i] == ' ') i++;
    while (str[j] == ' ') j--;
    str[j + 1] = '\0';
    strcpy(str, str + i);
}

/* TODO: proper implementation
Slb_string slb_string_create_from_cstr(char *cstr)
{
    size_t size = 0;
    for (size = 0; cstr[size] != '\0'; ++size){}

    Slb_string result = {0};
    result.content = malloc(size);
    result.capacity = size;
    result.size = size;
    result.content = cstr;
    return result;
}

void slb_string_append_cstr(Slb_string *a, char *b)
{
    Slb_string s = slb_string_create_from_cstr(b);
    slb_string_append(a, &s);
}

void slb_string_append(Slb_string *a, Slb_string *b)
{
    assert(0 && "Not implemented");
}
*/


#endif //SLB_STRING_IMPLEMENTATION


#endif //_SLB_STRING_H
