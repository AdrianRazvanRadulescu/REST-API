#include <stdio.h>      /* printf, sprintf */
#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"

#include "parson.h"
#define HOST "34.118.48.238"
#define PORT 8080

#define MAX_BUF 100
#define MAX_COOKIES 30



char** get_cookie(char *response_cpy, int *cookie_count) {
    *cookie_count = 0;
    
    char **cookie;
    cookie = malloc(MAX_COOKIES * sizeof(char *));
    for (int i = 0; i < MAX_COOKIES; i++) {
        cookie[i] = malloc(MAX_BUF * sizeof(char));
    }
    
    char *response = malloc(sizeof(char) * 1000);
    strcpy(response, response_cpy);

    char *substr;
    substr = strstr(response, "Set-Cookie:");

    const char s[2] = "\n";
    char *cookie_line;

    cookie_line = strtok(substr, s);


    const char space[2] = " ";
    char *token;
    int i = 0;
    token = strtok(cookie_line, space);

    while(token != NULL) {
        if (i != 0) {
            strcpy(cookie[*cookie_count], token);
            (*cookie_count)++;
        }
        
        token = strtok(NULL, space);
        i++;
    }

    for(i = 0; i < (*cookie_count); i++) {
        cookie[i][strlen(cookie[i])-1] = '\0';
    }

    return cookie;
}

int main(int argc, char *argv[])
{
    char *message;
    char *response;
    int sockfd;

    sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
    	error("Failed to connect to the server\n");
    }

    char *basic = NULL;
    char *command = malloc(sizeof(char) * MAX_BUF);
    char *username = malloc(sizeof(char) * MAX_BUF);
    char *password = malloc(sizeof(char) * MAX_BUF);
    char *author = malloc(sizeof(char) * MAX_BUF);
    char *title = malloc(sizeof(char) * MAX_BUF);
    char *genre = malloc(sizeof(char) * MAX_BUF);
    char *publisher = malloc(sizeof(char) * MAX_BUF);
    char *page_count_string = malloc(sizeof(char) * MAX_BUF);

    char *token = malloc(sizeof(char) * 1000);
    token = NULL;

    int login_cookie_count;
    char **login_cookie;
    login_cookie = malloc(MAX_COOKIES * sizeof(char *));
    for (int i = 0; i < MAX_COOKIES; i++) {
        login_cookie[i] = malloc(MAX_BUF * sizeof(char));
    }

    // Ex 1: Inregistrare
    while(1) {
        memset(command, 0, MAX_BUF);
        fgets(command, MAX_BUF - 1, stdin);

        if (strcmp(command, "register\n") == 0) {

            sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
            if (sockfd < 0) {
                error("Failed to connect to the server\n");
            }

            // Introducere username
            printf("username=");
            memset(username, 0, MAX_BUF);
            fgets(username, MAX_BUF - 1, stdin);
            username[strlen(username)-1] = 0;

            // Introducere parola
            printf("password=");
            memset(password, 0, MAX_BUF);
            fgets(password, MAX_BUF - 1, stdin);
            password[strlen(password)-1] = 0;

            JSON_Value *root_value = json_value_init_object();
            JSON_Object *root_object = json_value_get_object(root_value);
            char *serialized_string = NULL;
            json_object_set_string(root_object, "username", username);
            json_object_set_string(root_object, "password", password);
            serialized_string = json_serialize_to_string_pretty(root_value);
            
            message = compute_post_json(HOST, "/api/v1/tema/auth/register", 
                                            "application/json", serialized_string, NULL, 0, NULL);

            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);
            
            char *basic = basic_extract_json_response(response);

            if(basic == NULL) {
                printf("User registered succesfully.\n");
            }
            else {
                root_value = json_parse_string(basic);
                root_object = json_value_get_object(root_value);
                printf("\n%s\n", json_object_get_string(root_object, "error"));
            }

            json_value_free(root_value);
            close_connection(sockfd);
        }


        // COmanda de login
        if (strcmp(command, "login\n") == 0) {
            sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
            if (sockfd < 0) {
                error("Failed to connect to the server\n");
            }

            // Introducere username
            printf("username=");
            memset(username, 0, MAX_BUF);
            fgets(username, MAX_BUF - 1, stdin);
            username[strlen(username)-1] = 0;

            // Introducere parola
            printf("password=");
            memset(password, 0, MAX_BUF);
            fgets(password, MAX_BUF - 1, stdin);
            password[strlen(password)-1] = 0;

            JSON_Value *root_value = json_value_init_object();
            JSON_Object *root_object = json_value_get_object(root_value);
            char *serialized_string = NULL;
            json_object_set_string(root_object, "username", username);
            json_object_set_string(root_object, "password", password);
            serialized_string = json_serialize_to_string_pretty(root_value);
            
            message = compute_post_json(HOST, "/api/v1/tema/auth/login", 
                                            "application/json", serialized_string, NULL, 0, NULL);
            
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);

            // Iau cookie-urile de logare pe care le voi folosi la urmatoarele probleme.
            login_cookie = get_cookie(response, &login_cookie_count);
            char *basic = basic_extract_json_response(response);

            if(basic == NULL) {
                printf("User logged in succesfully!\n");
            }
            else {
                root_value = json_parse_string(basic);
                root_object = json_value_get_object(root_value);
                printf("\n%s\n", json_object_get_string(root_object, "error"));
            }

            json_value_free(root_value);
            close_connection(sockfd);
        }

        // Comanda enter_library
        if (strcmp(command, "enter_library\n") == 0) {
            sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
            if (sockfd < 0) {
                error("Failed to connect to the server\n");
            }
        
            message = compute_get_request(HOST, "/api/v1/tema/library/access", NULL, 
                                                    login_cookie, login_cookie_count, NULL);
            
            send_to_server(sockfd, message);
            
            response = receive_from_server(sockfd);

            char *basic = basic_extract_json_response(response);
            JSON_Value *root_value = json_value_init_object();
            JSON_Object *root_object = json_value_get_object(root_value);

            root_value = json_parse_string(basic);
            root_object = json_value_get_object(root_value);
            
            if (json_object_get_string(root_object, "token") != NULL) {
                printf("You entered the library.\n");
                
                token = strdup(json_object_get_string(root_object, "token"));
                printf("%s\n", token);
            }
            
            if (json_object_get_string(root_object, "error") != NULL) {
                printf("%s\n", json_object_get_string(root_object, "error"));
            }

            json_value_free(root_value);
            close_connection(sockfd);
        }

        if (strcmp(command, "get_books\n") == 0) {
            sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
            if (sockfd < 0) {
                error("Failed to connect to the server\n");
            }

            message = compute_get_request(HOST, "/api/v1/tema/library/books", NULL, 
                                                    login_cookie, login_cookie_count, token);
            
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);

            basic = basic_extract_json_response(response);
            JSON_Value *root_value = json_value_init_object();
            JSON_Object *root_object = json_value_get_object(root_value);

            root_value = json_parse_string(basic);
            root_object = json_value_get_object(root_value);            
            
            if (json_object_get_string(root_object, "error") != NULL) {
                printf("%s\n", json_object_get_string(root_object, "error"));
            }
            else if (basic != NULL) {
                printf("%s\n", basic);
            }

            json_value_free(root_value);
            close_connection(sockfd);
        }

        if(strcmp(command, "get_book\n") == 0) {
            sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
            if (sockfd < 0) {
                error("Failed to connect to the server\n");
            }

            printf("id=");
            int book_id;
            scanf("%d", &book_id);
            
            char *get_book = calloc(BUFLEN, sizeof(char));
            sprintf(get_book, "/api/v1/tema/library/books/%d", book_id);

            message = compute_get_request(HOST, get_book, NULL,
                                                    login_cookie, login_cookie_count, token);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);

            basic = basic_extract_json_response(response);
            
            JSON_Value *root_value = json_value_init_object();
            JSON_Object *root_object = json_value_get_object(root_value);

            root_value = json_parse_string(basic);
            root_object = json_value_get_object(root_value);            
            
            if (json_object_get_string(root_object, "error") != NULL) {
                printf("%s\n", json_object_get_string(root_object, "error"));
            }
            else if (basic != NULL) {
                printf("%s\n", basic);
            }

            json_value_free(root_value);
            close_connection(sockfd);

        }

        if (strcmp(command, "add_book\n") == 0) {
            sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
            if (sockfd < 0) {
                error("Failed to connect to the server\n");
            }

            printf("title=");
            memset(title, 0, MAX_BUF);
            fgets(title, MAX_BUF - 1, stdin);
            title[strlen(title)-1] = 0;

            printf("author=");
            memset(author, 0, MAX_BUF);
            fgets(author, MAX_BUF - 1, stdin);
            author[strlen(author)-1] = 0;

            printf("genre=");
            memset(genre, 0, MAX_BUF);
            fgets(genre, MAX_BUF - 1, stdin);
            genre[strlen(genre)-1] = 0;

            printf("page_count=");
            memset(page_count_string, 0, MAX_BUF);
            fgets(page_count_string, MAX_BUF - 1, stdin);
            page_count_string[strlen(page_count_string)-1] = 0;

            printf("publisher=");
            memset(publisher, 0, MAX_BUF);
            fgets(publisher, MAX_BUF - 1, stdin);
            publisher[strlen(publisher)-1] = 0;

            JSON_Value *root_value = json_value_init_object();
            JSON_Object *root_object = json_value_get_object(root_value);
            char *serialized_string = NULL;
            json_object_set_string(root_object, "title", title);
            json_object_set_string(root_object, "author", author);
            json_object_set_string(root_object, "genre", genre);
            json_object_set_string(root_object, "page_count", page_count_string);
            json_object_set_string(root_object, "publisher", publisher);
            
            serialized_string = json_serialize_to_string_pretty(root_value);

            message = compute_post_json(HOST, "/api/v1/tema/library/books", "application/json",
                                     serialized_string, login_cookie, login_cookie_count, token);

            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);

            char *basic = basic_extract_json_response(response);

            root_value = json_parse_string(basic);
            root_object = json_value_get_object(root_value);  

            if (json_object_get_string(root_object, "error") != NULL) {
                printf("%s\n", json_object_get_string(root_object, "error"));
            }

            else {
                printf("Book added succesfully.\n");
            }

            json_value_free(root_value);
            close_connection(sockfd);     
        }

        if (strcmp(command, "delete_book\n") == 0) {
            sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
            if (sockfd < 0) {
                error("Failed to connect to the server\n");
            }

            printf("id=");
            int book_id;
            scanf("%d", &book_id);
            
            char *get_book = calloc(BUFLEN, sizeof(char));
            sprintf(get_book, "/api/v1/tema/library/books/%d", book_id);

            message = compute_delete_request(HOST, get_book, login_cookie,
                                                 login_cookie_count, token);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);
            
            basic = basic_extract_json_response(response);
            
            JSON_Value *root_value = json_value_init_object();
            JSON_Object *root_object = json_value_get_object(root_value);

            root_value = json_parse_string(basic);
            root_object = json_value_get_object(root_value);            
            
            if (json_object_get_string(root_object, "error") != NULL) {
                printf("%s\n", json_object_get_string(root_object, "error"));
            }
            else {
                printf("Book deleted succesfully.\n");
            }

            json_value_free(root_value);
            close_connection(sockfd);
        }

        if(strcmp(command, "logout\n") == 0) {
            sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
            if (sockfd < 0) {
                error("Failed to connect to the server\n");
            }

            message = compute_get_request(HOST, "/api/v1/tema/auth/logout", NULL, 
                                            login_cookie, login_cookie_count, NULL);
            

            token = NULL;
            login_cookie = NULL;

            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);

            char *basic = basic_extract_json_response(response);

            JSON_Value *root_value = json_value_init_object();
            JSON_Object *root_object = json_value_get_object(root_value);

            if(basic == NULL) {
                printf("Logout succesful.\n");
            }
            else {
                root_value = json_parse_string(basic);
                root_object = json_value_get_object(root_value);
                printf("%s\n", json_object_get_string(root_object, "error"));
            }

            json_value_free(root_value);
            close_connection(sockfd);
        }

        if(strcmp(command, "exit\n") == 0) {
            printf("Exiting...\n");
            break;
        }
    }

    close(sockfd);
    return 0;
}
