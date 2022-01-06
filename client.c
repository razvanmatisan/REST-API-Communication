#include <stdio.h>     
#include <stdlib.h>    
#include <unistd.h>    
#include <string.h>    
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"
#include "parson.h"

#define NMAX 10000

int main(int argc, char **argv) {
    char command[NMAX], path[NMAX];
    char *sent_message, *received_message;
    
    char *body[1];
    body[0] = malloc(sizeof(NMAX * sizeof(char)));
    
    char *content_type = "application/json";
    char *host = "34.118.48.238";
    int port = 8080;

    char *cookie_safe = calloc(BUFLEN, sizeof(char));
    char *token_safe = calloc(BUFLEN, sizeof(char));

    int flag = 1;

    while (flag) {
        printf("Write your command here: ");

        memset(path, 0, sizeof(path));
        memset(command, 0, sizeof(command));
        fgets(command, NMAX, stdin);

        int sockfd = open_connection(host, port, AF_INET, SOCK_STREAM, 0);

        if (!strcmp(command, "register\n")) {
            char username[NMAX];
            char password[NMAX];

            JSON_Value *value_init_object = json_value_init_object();
            JSON_Object *json_object = json_value_get_object(value_init_object);

            printf("username=");
            fgets(username, BUFLEN, stdin);
            json_object_set_string(json_object, "username", username);

            printf("password=");
            fgets(password, BUFLEN, stdin);
            json_object_set_string(json_object, "password", password);

            body[0] = json_serialize_to_string_pretty(value_init_object);

            strcpy(path, "/api/v1/tema/auth/register");

            sent_message = compute_post_request(host, path, content_type, body, 1, NULL, 0, NULL);
            send_to_server(sockfd, sent_message);
            received_message = receive_from_server(sockfd);

            JSON_Value *val = json_parse_string(strstr(received_message, "{"));
            JSON_Object *obj = json_value_get_object(val);

            const char* error = json_object_dotget_string(obj, "error");
            if (!error) {
                printf("The registration was successful!\n\n");
            } else {
                printf("Error: %s\n\n", error);
            }

            json_value_free(val);
            json_free_serialized_string(body[0]);
            json_value_free(value_init_object);
        
        } else if (!strcmp(command, "login\n")) {
            char username[NMAX];
            char password[NMAX];

            JSON_Value *value_init_object = json_value_init_object();
            JSON_Object *json_object = json_value_get_object(value_init_object);

            printf("username=");
            fgets(username, BUFLEN, stdin);
            json_object_set_string(json_object, "username", username);

            printf("password=");
            fgets(password, BUFLEN, stdin);
            json_object_set_string(json_object, "password", password);

            body[0] = json_serialize_to_string_pretty(value_init_object);

            strcpy(path, "/api/v1/tema/auth/login");

            sent_message = compute_post_request(host, path, content_type, body, 1, NULL, 0, NULL);
            send_to_server(sockfd, sent_message);
            received_message = receive_from_server(sockfd);

            char *begin, *end;
            if (strstr(received_message, "connect")) {
                begin = strstr(received_message, "connect");
                end = strstr(begin, "/;");
                memcpy(cookie_safe, begin, end - begin + 1);
            }

            char *text = strstr(received_message, "{");
            JSON_Value *val = json_parse_string(text);
            JSON_Object *obj = json_value_get_object(val);

            const char* error = json_object_dotget_string(obj, "error");
            if (!error) {
                printf("The login was successful!\n\n");
            } else {
                printf("Error: %s\n\n", error);
            }

            json_value_free(val);
            json_free_serialized_string(body[0]);
            json_value_free(value_init_object);
        } else if (!strcmp(command, "enter_library\n")) {
            strcpy(path, "/api/v1/tema/library/access");

            sent_message = compute_get_request(host, path, NULL, &cookie_safe, 1, NULL);
            send_to_server(sockfd, sent_message);
            received_message = receive_from_server(sockfd);

            char *text = strstr(received_message, "{");
            JSON_Value *val = json_parse_string(text);
            JSON_Object *obj = json_value_get_object(val);

            const char* error = json_object_dotget_string(obj, "error");
            if (!error) {
                const char* received_token = json_object_dotget_string(obj, "token");
                strcpy(token_safe, received_token);
                printf("Good job! You entered in the library successfully!\n\n");
            } else {
                printf("Error: %s\n\n", error);
            }
            
            json_value_free(val);
        } else if (!strcmp(command, "get_books\n")) {
            strcpy(path, "/api/v1/tema/library/books");

            sent_message = compute_get_request(host, path, NULL, NULL, 1, &token_safe);
            send_to_server(sockfd, sent_message);
            received_message = receive_from_server(sockfd);

            char *books = strstr(received_message, "[");
            char *text = strstr(received_message, "{");

            if (books) {
                JSON_Value *val = json_parse_string(books);
                JSON_Array *arr = json_value_get_array(val);
                int size_array = json_array_get_count(arr);

                if (!size_array) {
                    printf("Your library has no books!\n\n");
                    json_value_free(val);
                    continue;
                }

                for (int i = 0; i < size_array; i++) {
                    JSON_Object *obj = json_array_get_object(arr, i);
                    const int id = json_object_dotget_number(obj, "id");
                    const char* title = json_object_dotget_string(obj, "title");

                    printf("Id: %d\n", id);
                    printf("Title: %s\n\n", title);
                }

                json_value_free(val);
                continue;
                
            }
            
            JSON_Object *obj = json_value_get_object(json_parse_string(text));

            printf("Error: %s\n\n", json_object_dotget_string(obj, "error"));

        } else if (!strcmp(command, "get_book\n")) {
            char id[BUFLEN];
            printf("id=");
            fgets(id, BUFLEN, stdin);
            
            if (!strlen(id)) {
                printf("No id was inserted!\n\n");
                continue;
            }
            
            id[strlen(id) - 1] = '\0';

            strcpy(path, "/api/v1/tema/library/books/");
            strcat(path, id);

            sent_message = compute_get_request(host, path, NULL, NULL, 1, &token_safe);
            send_to_server(sockfd, sent_message);
            received_message = receive_from_server(sockfd);

            char *text = strstr(received_message, "{");
            JSON_Value *val = json_parse_string(text);
            JSON_Object *obj = json_value_get_object(val);

            const char* error = json_object_dotget_string(obj, "error");

            if (!error) {
                const char* title = json_object_dotget_string(obj, "title");
                const char* author = json_object_dotget_string(obj, "author");
                int page_count = json_object_dotget_number(obj, "page_count");
                const char* publisher  = json_object_dotget_string(obj, "publisher");
                const char* genre = json_object_dotget_string(obj, "genre");

                printf("Id: %s\nTitle: %s\nAuthor: %s\nPublisher: %s\nGenre: %s\nPage count: %d\n\n", id, title, author, publisher, genre, page_count);
            } else {
                printf("Error: %s\n\n", error);
            }

            json_value_free(val);
        
        } else if (!strcmp(command, "add_book\n")) {
            strcpy(path, "/api/v1/tema/library/books");

            JSON_Value *value_init_object = json_value_init_object();
            JSON_Object *object = json_value_get_object(value_init_object);

            char title[BUFLEN], author[BUFLEN], genre[BUFLEN], page_count[BUFLEN], publisher[BUFLEN];
            
            printf("title=");
            fgets(title, BUFLEN, stdin);
            title[strlen(title) - 1] = '\0';
            json_object_set_string(object, "title", title);

            printf("author=");
            fgets(author, BUFLEN, stdin);
            author[strlen(author) - 1] = '\0';
            json_object_set_string(object, "author", author);

            printf("genre=");
            fgets(genre, BUFLEN, stdin);
            genre[strlen(genre) - 1] = '\0';
            json_object_set_string(object, "genre", genre);

            printf("page_count=");
            fgets(page_count, BUFLEN, stdin);
            json_object_set_number(object, "page_count", atoi(page_count));

            printf("publisher=");
            fgets(publisher, BUFLEN, stdin);
            publisher[strlen(publisher) - 1] = '\0';
            json_object_set_string(object, "publisher", publisher);

            if (strlen(title) > 0 && strlen(author) > 0 && strlen(genre) > 0 && strlen(publisher) > 0 && strlen(page_count) > 0 && atoi(page_count) > 0) {
                body[0] = json_serialize_to_string_pretty(value_init_object);

                sent_message = compute_post_request(host, path, content_type, body, 1, NULL, 0, &token_safe);
                send_to_server(sockfd, sent_message);
                received_message = receive_from_server(sockfd);
                
                char *text = strstr(received_message, "{");
                JSON_Value *val = json_parse_string(text);
                JSON_Object *obj = json_value_get_object(val);

                const char* error = json_object_dotget_string(obj, "error");
                if (!error) {
                    printf("The book was successfully added!\n\n");
                } else {
                    printf("Error: %s\n\n", error);
                }

                json_free_serialized_string(body[0]);
                json_value_free(val);
            } else {
                printf("Incorrect format of book!\n\n");
            }
            
        } else if (!strcmp(command, "delete_book\n")) {
            char id[BUFLEN];
            printf("id=");
            fgets(id, BUFLEN, stdin);
            
            if (!strlen(id)) {
                printf("No id was inserted!\n\n");
                continue;
            }
            
            id[strlen(id) - 1] = '\0';

            strcpy(path, "/api/v1/tema/library/books/");
            strcat(path, id);

            sent_message = compute_delete_request(host, path, NULL, NULL, 1, &token_safe);
            send_to_server(sockfd, sent_message);
            received_message = receive_from_server(sockfd);


            char *text = strstr(received_message, "{");
            JSON_Value *val = json_parse_string(text);
            JSON_Object *obj = json_value_get_object(val);

            const char* error = json_object_dotget_string(obj, "error");
            if (!error) {
                printf("The book has been successfully deleted!\n\n");
            } else {
                printf("Error: %s\n\n", error);
            }
            
            json_value_free(val);
        } else if (!strcmp(command, "logout\n")) {
            strcpy(path, "/api/v1/tema/auth/logout");

            sent_message = compute_get_request(host, path, NULL, &cookie_safe, 1, NULL);
            send_to_server(sockfd, sent_message);
            received_message = receive_from_server(sockfd);

            char *text = strstr(received_message, "{");
            JSON_Value *val = json_parse_string(text);
            JSON_Object *obj = json_value_get_object(val);
            const char* error = json_object_dotget_string(obj, "error");

            if (!error) {
                printf("The logout was successfully!\n\n");
                memset(cookie_safe, 0, NMAX);
                memset(token_safe, 0, NMAX);
            } else {
                printf("Error: %s\n\n", error);
            }
            
            json_value_free(val);
            
        } else if (!strcmp(command, "exit\n")) {
            flag = 0;
        } else {
            printf("Invalid command. Please try again.\n\n");
        }

        close_connection(sockfd);
    }

    return 0;
}
