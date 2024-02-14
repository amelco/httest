#include <stdio.h>

#define SLB_ARGS_IMPLEMENTATION 
#define SLB_STRING_IMPLEMENTATION
#include "slb_args.h"
#include "slb_string.h"

///// HTTP_REQUEST_IMPLEMENTATION - BEGIN /////
#include <stdlib.h>
#include <curl/curl.h>
typedef struct {
  char *memory;
  size_t size;
} Response;

static size_t write(void *contents, size_t size, size_t nmemb, void *userp) {
  size_t realsize = size * nmemb;
  Response *mem = (Response *)userp;

  mem->memory = realloc(mem->memory, mem->size + realsize + 1);
  if(mem->memory == NULL) {
    printf("not enough memory (realloc returned NULL)\n");
    return 0;
  }

  memcpy(&(mem->memory[mem->size]), contents, realsize);
  mem->size += realsize;
  mem->memory[mem->size] = 0;

  return realsize;
}

char* http_get(char* url, int *code) {
    Response response;
    response.memory = malloc(1);
    response.size = 0;

    CURL *curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&response);
        CURLcode res;
        res = curl_easy_perform(curl);
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, code);
        if (res != CURLE_OK) {
            fprintf(stderr, "ERROR: curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        }
        curl_easy_cleanup(curl);
    }
    return response.memory;
}

char* http_post(char* url, char* body, int *code) {
    if (body == NULL) strcpy(body, "{}");

    Response response;
    response.memory = malloc(1);
    response.size = 0;

    struct curl_slist *headers = NULL;
    CURL *curl = curl_easy_init();
    if (curl) {
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&response);
        CURLcode res;
        res = curl_easy_perform(curl);
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, code);
        if (res != CURLE_OK) {
            fprintf(stderr, "ERROR: curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        }
        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
    }
    return response.memory;
}
///// HTTP_REQUEST_IMPLEMENTATION - END /////

///// HTTP_FILE_PARSING_IMPLEMENTATION - BEGIN /////
#include <assert.h>
#define MAX_VAR_QTY 256

typedef enum {
    HTTP_METHOD_GET,
    HTTP_METHOD_POST,
} Method;

typedef struct {
    Method method;
    char url[256];
    char body[1024];
} Request;

typedef enum {
    TYPE_VARIABLE = '@',
} Types;

int find_var_index(char* var_name, char var_names[MAX_VAR_QTY][256]) {
    for (int i = 0; i < MAX_VAR_QTY; ++i) {
        if (strcmp(var_name, var_names[i]) == 0) {
            return i;
        }
    }
    return -1;
}

char* fill_variables(char *rota, char var_names[256][256], char var_values[256][256]) {
    int pos_ini = -1;
    int pos_end = -1;
    static char parsed_rota[256] = {'\0'};
    char var_name[256] = {'\0'};
    int strptr = 0;
    int j = 0;
    for (int i = 0; i < rota[i] != '\0'; ++i) {
        parsed_rota[j] = rota[i];
        if (rota[i] == '{') {
            pos_ini = i;
            continue;
        }
        if (rota[i] == '}') {
            assert(pos_ini != -1 && "Syntax error: '}' without '{'");
            var_name[strptr] = '\0';
            parsed_rota[j] = '\0';
            pos_end = i;

            int index = find_var_index(var_name, var_names);
            assert(index != -1 && "Variable not found");
            strcat(parsed_rota, var_values[index]);
            j = strlen(parsed_rota);

            pos_ini = -1;
            strptr = 0;
            continue;
        }
        if (pos_ini != -1) {
            var_name[strptr] = rota[i];
            strptr++;
            continue;
        }
        j++;
    }
    return parsed_rota;
}

void parse_requests(Slb_string *file, Request requests[256]) {
    char var_names[MAX_VAR_QTY][256]  = {0};
    char var_values[MAX_VAR_QTY][256] = {0};
    int var_count = 0;
    int total_requests = 0;

    slb_string_reset_cursor(file);
    for (;;) {
        char *line = slb_string_get_next(file, '\n');
        if (line == NULL) break;

        slb_cstr_trim(line);
        if (line[0] == TYPE_VARIABLE) {
            strcpy(var_names[var_count], strtok(line + 1, "="));
            strcpy(var_values[var_count], strtok(NULL, "\n"));
            var_count++;
        }
        else if (line[0] == 'G' && line[1] == 'E' && line[2] == 'T') {
            strtok(line, " ");
            char* rota = strtok(NULL, "\n");
            char* url = fill_variables(rota, var_names, var_values);
            strcpy(requests[total_requests].url, url);
            requests[total_requests].method = HTTP_METHOD_GET;
            total_requests++;
        }
        else if (line[0] == 'P' && line[1] == 'O' && line[2] == 'S' && line[3] == 'T') {
            strtok(line, " ");
            char* rota = strtok(NULL, "\n");
            char* url = fill_variables(rota, var_names, var_values);
            strcpy(requests[total_requests].url, url);

            char* hasBody = slb_string_peek_next(file, '\n');
            if (hasBody == NULL) break;
            slb_cstr_trim(hasBody);
            if (hasBody[0] == '{') {
                char* body = slb_string_get_next(file, '\n');
                strcpy(requests[total_requests].body, body);
                // TODO: parse multiline json body
            }

            requests[total_requests].method = HTTP_METHOD_POST;
            total_requests++;
        }
        free(line);
    }
}
///// HTTP_FILE_PARSING_IMPLEMENTATION - END /////

#define bool int
#define true 1
#define false 0
#define PRINT_CSTR_RED(x) printf("\033[0;31m%s\033[0m", x)
#define PRINT_INT_RED(x) printf("\033[0;31m%d\033[0m", x)

void usage(char* program) {
    printf("Usage: %s <http_file> [-v]\n", program);
    printf("  -v: verbose [OPTIONAL]\n");
    exit(1);
}

int main(int argc, char **argv) {
    // manage command line arguments
    bool verbose = false;
    Slb_args args = slb_args_init(argc, argv);
    char* program = slb_args_next(&args);
    char* http_file = slb_args_next(&args);
    char* arg = slb_args_next(&args);
    if (http_file == NULL) usage(program);
    if (arg != NULL) {
        if(strcmp(arg, "-v") == 0) verbose = true;
        else usage(program);
    }

    // parse http file
    Slb_string *file = slb_read_entire_file(http_file);
    if (file == NULL) {
        PRINT_CSTR_RED("ERROR: ");
        printf("Could not open file '%s'\n", http_file);
        exit(1);
    }
    Request requests[256] = {0};
    parse_requests(file, requests);
    slb_string_close(file);

    // make a http request for each request in requests
    for (int i = 0; requests[i].url[0] != '\0'; ++i) {
        int status_code = -1;
        if (requests[i].method == HTTP_METHOD_GET) {
            char *response = http_get(requests[i].url, &status_code);
            printf("GET %s - ", requests[i].url);
            if (status_code != 200) PRINT_INT_RED(status_code);
            else printf("%d", status_code);
            printf("\n");
            if (verbose) {
                printf("Response:\n%s\n\n", response);
            }
            free(response);
            continue;
        }
        if (requests[i].method == HTTP_METHOD_POST) {
            char *response = http_post(requests[i].url, requests[i].body, &status_code);
            printf("POST %s - ", requests[i].url);
            if (status_code != 200) PRINT_INT_RED(status_code);
            else printf("%d", status_code);
            printf("\n");
            if (verbose) {
                printf("Body:\n%s\n", requests[i].body);
                printf("Response:\n%s\n\n", response);
            }
            free(response);
            continue;
        }
    }

    return 0;
}
