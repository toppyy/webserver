#pragma once

typedef enum  {
    UNKNOWN = 1,
    HTML,
    JAVASCRIPT,
    IMG_JPEG,
    IMG_GIF,
    IMG_PNG
} content_type;


typedef struct request {
    content_type ct;
    char* filename;
    int filename_size;
}   request;

request parse_request(char *buff, int recvd);

content_type find_content_type(char* fn);
