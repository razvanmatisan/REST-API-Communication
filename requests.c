#include <stdlib.h>     
#include <stdio.h>
#include <unistd.h>     
#include <string.h>     
#include <sys/socket.h> 
#include <netinet/in.h>
#include <netdb.h>      
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"

// preluat din laboratorul 10: Protocolul HTTP

char *compute_get_request(const char *host, char *url, char *query_params,
                            char **cookies, int cookies_count, char** token)
{
    char *message = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));

    if (query_params != NULL) {
        sprintf(line, "GET %s?%s HTTP/1.1", url, query_params);
    } else {
        sprintf(line, "GET %s HTTP/1.1", url);
    }

    compute_message(message, line);
    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    if (token != NULL) {
        memset(line, 0, LINELEN);
        sprintf(line, "Authorization: Bearer %s",token[0]);
        compute_message(message, line);
    }

    if(cookies != NULL){
        int i;
        for(i = 0; i < cookies_count; i++){
            sprintf(line, "Cookie: %s", cookies[i]);
            compute_message(message, line);
        }
    }

    compute_message(message, "");
    return message;
}

char *compute_post_request(const char *host, char *url, char* content_type, char **body_data,
                            int body_data_fields_count, char **cookies, int cookies_count, char** token)
{
    char *message = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));
    char *body_data_buffer = calloc(LINELEN, sizeof(char));

    memset(line, 0, LINELEN);
    sprintf(line, "POST %s HTTP/1.1", url);
    compute_message(message, line);

    if (token != NULL) {
        memset(line, 0, LINELEN);
        sprintf(line, "Authorization: Bearer %s",token[0]);
        compute_message(message, line);
    }
    
    memset(line, 0, LINELEN);
    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    int msg_size = 0;    
    if (body_data) {
        size_t i;
        for (i = 0; i < body_data_fields_count - 1; ++i) {
            strcat(body_data_buffer, body_data[i]);
            strcat(body_data_buffer, "&");

            msg_size += strlen(body_data[i]) + 1;
        }

        strcat(body_data_buffer, body_data[i]);
        msg_size += strlen(body_data[i]);
    }

    memset(line, 0, LINELEN);
    sprintf(line, "Content-Length: %d", msg_size);
    compute_message(message, line);

    memset(line, 0, LINELEN);
    sprintf(line, "Content-Type: %s", content_type);
    compute_message(message, line);

    if (cookies != NULL) {
        for(int i = 0; i < cookies_count; i++) {
            sprintf(line, "Cookies %s", cookies[i]);
            compute_message(message, line);
        }      
    }

    compute_message(message, "");

    compute_message(message, body_data_buffer);

    free(line);
    return message;
}

char *compute_delete_request(const char *host, char *url, char *query_params, char **cookies, int cookies_count, char **token) {
    char *message = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));

    if (query_params != NULL) {
        sprintf(line, "DELETE %s?%s HTTP/1.1", url, query_params);
    } else {
        sprintf(line, "DELETE %s HTTP/1.1", url);
    }

    compute_message(message, line);
    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    if (token != NULL) {
        memset(line, 0, LINELEN);
        sprintf(line, "Authorization: Bearer %s",token[0]);
        compute_message(message, line);
    }

    if(cookies != NULL){
        int i;
        for(i = 0; i < cookies_count; i++){
            sprintf(line, "Cookie: %s", cookies[i]);
            compute_message(message, line);
        }
    }

    compute_message(message, "");
    return message;
}