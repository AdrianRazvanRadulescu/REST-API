#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <stdio.h>
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"


char *compute_delete_request(char *host, char *url, char **cookies, 
                                    int cookies_count, char *token) {
    char *message = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));

    // Step 1: write the method name, URL, request params (if any) and protocol type
    sprintf(line, "DELETE %s HTTP/1.1", url);

    compute_message(message, line);

    // Step 2: add the host
    memset(line, 0, LINELEN);
    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    // Step 3 (optional): add headers and/or cookies, according to the protocol format
    char *cookie_line = calloc(LINELEN, sizeof(char));
    
    if (token != NULL) {
        memset(line, 0, LINELEN);
        sprintf(line, "Authorization: Bearer %s", token);
        compute_message(message, line); 
    }

    if (cookies != NULL) {
        sprintf(cookie_line, "Cookie: ");
        for (int i = 0; i < cookies_count; i++) {
            memset(line, 0, LINELEN);
            if (i == cookies_count - 1) {
                sprintf(line, "%s", cookies[i]);
            } else {
                sprintf(line, "%s; ", cookies[i]);
            }
            strcat(cookie_line, line);
        }
        compute_message(message, cookie_line);
    }


    // Step 4: add final new line
    compute_message(message, "");
    free(line);
    free(cookie_line);
    return message;
}


char *compute_post_json(char *host, char *url, char* content_type, char *body_data,
                             char **cookies, int cookies_count, char *token)
{
    char *message = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));

    // Step 1: write the method name, URL and protocol type
    sprintf(line, "POST %s HTTP/1.1", url);
    compute_message(message, line);
    
    // Step 2: add the host
    memset(line, 0, LINELEN);
    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    /* Step 3: add necessary headers (Content-Type and Content-Length are mandatory)
            in order to write Content-Length you must first compute the message size
    */
    if (body_data != NULL) {
        memset(line, 0, LINELEN);
        sprintf(line, "Content-Type: %s", content_type);
        compute_message(message, line);
        
        memset(line, 0, LINELEN);
        sprintf(line, "Content-Length: %ld", strlen(body_data));
        compute_message(message, line);
    }


    // Step 4 (optional): add cookies
    char *cookie_line = calloc(LINELEN, sizeof(char));
    if (cookies != NULL) {
        sprintf(cookie_line, "Cookie: ");
        for (int i = 0; i < cookies_count; i++) {
            memset(line, 0, LINELEN);
            if (i == cookies_count - 1) {
                sprintf(line, "%s", cookies[i]);
            } else {
                sprintf(line, "%s; ", cookies[i]);
            }
            strcat(cookie_line, line);
        }   
        compute_message(message, cookie_line);    
    }

    if (token != NULL) {
        memset(line, 0, LINELEN);
        sprintf(line, "Authorization: Bearer %s", token);
        compute_message(message, line); 
    }

    // Step 5: add new line at end of header
    compute_message(message, "");

    // Step 6: add the actual payload data
    compute_message(message, body_data);

    free(line);
    free(body_data);
    free(cookie_line);
    return message;
}

char *compute_get_request(char *host, char *url, char *query_params,
                            char **cookies, int cookies_count, char *token)
{
    char *message = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));

    // Step 1: write the method name, URL, request params (if any) and protocol type
    if (query_params != NULL) {
        sprintf(line, "GET %s?%s HTTP/1.1", url, query_params);
    } else {
        sprintf(line, "GET %s HTTP/1.1", url);
    }

    compute_message(message, line);

    // Step 2: add the host
    memset(line, 0, LINELEN);
    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    // Step 3 (optional): add headers and/or cookies, according to the protocol format
    char *cookie_line = calloc(LINELEN, sizeof(char));
    
    if (token != NULL) {
        memset(line, 0, LINELEN);
        sprintf(line, "Authorization: Bearer %s", token);
        compute_message(message, line); 
    }

    if (cookies != NULL) {
        sprintf(cookie_line, "Cookie: ");
        for (int i = 0; i < cookies_count; i++) {
            memset(line, 0, LINELEN);
            if (i == cookies_count - 1) {
                sprintf(line, "%s", cookies[i]);
            } else {
                sprintf(line, "%s; ", cookies[i]);
            }
            strcat(cookie_line, line);
        }
        compute_message(message, cookie_line);
    }


    // Step 4: add final new line
    compute_message(message, "");
    free(line);
    free(cookie_line);
    return message;
}
