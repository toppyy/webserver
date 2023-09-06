#include <string.h>
#include<stdio.h>
#include<parse.h>

content_type find_content_type(char* fn) {
    int i       = sizeof(fn) - 1;
    char file_extension[100];

    while (fn[i--] !=  '.' & i >= 0) {};

    if (i < 0) {
        return UNKNOWN;
    }

    int idx = 0;
    while (fn[i] != 0) {
        file_extension[idx++] = fn[i];
        i++;
    }

    if (strcmp(file_extension, "html") == 0) {
        return HTML;
    }

    if (strcmp(file_extension, "png") == 0) {
        return IMG_PNG;
    }

}


int parse_requested_file(char* buff, char* fn) {
    // Parse name of requested file from HTTP-request

    int idx = 5; // "GET /" TODO !!
    int fn_idx = 0;
    fn[fn_idx] = 0;
    while (buff[idx] >= 33 & buff[idx] <= 126) {
        fn[fn_idx] = buff[idx];
        fn_idx++;
        idx++;
    }
    fn[fn_idx] =  0;
    return fn_idx;
}


request parse_request(char *buff, int recvd) {
    struct request req;

    if (recvd == 0) {
        req.filename_size = 0;
        return req;
    }
    
    req.filename_size = parse_requested_file(buff, req.filename);
    req.ct = find_content_type(req.filename);

    // Log the request
    printf("Requested file: %s\n", req.filename);
    return req;
}







