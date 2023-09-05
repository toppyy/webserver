#include <unistd.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <respond.h>

#define MSGBUFF     50000
#define REQFNBUFF   50


typedef enum {
    UNKNOWN = 1,
    HTML,
    IMG_PNG
} ContentType;


ContentType find_content_type(char* fn) {
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


int addCRLF(char* str, int at) {
    str[at]     = 13;    // CR
    str[at + 1] = 10;    // LF
    return 2;
}

int parse_requested_file(char* buff, char* fn) {
    // Parse name of requested file from HTTP-request

    int idx = 5; // "GET /"
    int fn_idx = 0;
    fn[fn_idx] = 0;
    while (buff[idx] >= 33 & buff[idx] <= 126) {
        fn[fn_idx] = buff[idx];
        fn_idx++;
        idx++;
    }
    fn[fn_idx + 1] =  0;
    return fn_idx;
}

int set_header_content_type(char* msg, ContentType content_type) {

    int bytes_added = 14;
    memcpy(msg, "Content-Type: ", bytes_added);


    if (content_type == IMG_PNG) {
        memcpy(msg, "image/png", 9);
        return bytes_added + 9;
    }

    // (content_type == HTML | content_type == UNKNOWN) is default
    memcpy(msg, "text/html", 9);
    return bytes_added + 9;
}


int make_200(char* msg, ContentType content_type) {
    
    int bytes_added = 0;
    /* ----- Make headers ----- */

    // Status 200
    int  status_size                    = 15;
    memcpy(msg, "HTTP/1.0 200 OK", status_size);
    bytes_added                       += status_size;
    bytes_added                       += addCRLF(msg, bytes_added);

    // Content type
    bytes_added                       += set_header_content_type(msg + bytes_added, content_type);
    bytes_added                       += addCRLF(msg, bytes_added);

    // Mark end of headers
    bytes_added                       += addCRLF(msg, bytes_added);

    return bytes_added;
}


int make_404(char* msg) {
    
    char not_f_size = 27;
    
    memcpy(msg, "HTTP/1.0 404 File not found", not_f_size);

    return not_f_size + addCRLF(msg, not_f_size);
}


int add_data_from_file(char* msg, int fd) {
    // Read file 100 bytes at a time and add to message
    int bytes_read, bytes_read_total = 0;
    
    do {
        bytes_read          = read(fd, msg + bytes_read_total, 100);
        bytes_read_total    += bytes_read;   
    } while( bytes_read > 0 );

    return bytes_read_total;
}


void log_request_info(char* buff, int recvd, char* req_fn) {
    // Print method
    int idx = 0;
    while (buff[idx] != '/' & idx < recvd) {
        printf("%c", buff[idx]);
        idx++;
    }
    // Print requested file
    printf(": %s\n", req_fn);
}


void respond(int recvd, char* buff, int cfd) {

     // init response datastructures
    char msg[MSGBUFF];
    char req_fn[REQFNBUFF];
    memset(&req_fn, 0, REQFNBUFF);

    char root[7]        = "../out/";
    int  root_size      = sizeof(root);
    memcpy(req_fn, root, root_size); // Init requested file path with 'root'

    char std[9]         = "root.html";
    int  std_size       = sizeof(std);
    int  bytes_to_send  = 0;
    int  req_fn_size    = 0;
    int  fd             = -1;

    // If there's no bytes received, don't try to parse the msg - just 404
    if (recvd > 0) {

        // Parse requested file and log request
        req_fn_size = parse_requested_file(buff, req_fn + root_size);
        log_request_info(buff, recvd, req_fn);

        // If no specific file was requested, return file specified as 'std'
        if (req_fn_size == 0) {
            memcpy(req_fn + root_size, std, std_size);
        }
        // Try to open the file at path req_fn
        fd = open(req_fn, O_RDONLY);

    }
   
    if (fd == -1) {
        bytes_to_send += make_404(msg);        
    } else {
        bytes_to_send += make_200(msg, find_content_type(req_fn));
        bytes_to_send += add_data_from_file(msg + bytes_to_send, fd);
        close(fd);
    }

    // Send it 
    int bytes_sent = send(cfd, msg, bytes_to_send, 0);
    printf("Sent %d bytes\n", bytes_sent);

}