#pragma once

typedef enum  {
    UNKNOWN = 1,
    HTML,
    IMG_PNG
} content_type;


typedef struct request {
    content_type ct;
    char* filename;
    int filename_size;
}   request;

request parse_request(char *buff, int recvd);