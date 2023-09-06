#include <unistd.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <respond.h>

#define MSGBUFF     50000
#define REQFNBUFF   50

#define RESPONSE404 "<html><body>404 - Nothing here, sorry</body></html>"
#define RESPONSE404LEN 51


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


int addCRLF(char* str) {
    str[0] = 13;    // CR
    str[1] = 10;    // LF
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

    char *msg_start = msg;
    
    memcpy(msg, "Content-Type: ", 14);
    msg += 14;

    if (content_type == IMG_PNG) {
        memcpy(msg, "image/png", 9);
        msg += 9;
    } else {
        // Resort to default: (content_type == HTML | content_type == UNKNOWN)
        memcpy(msg, "text/html", 9);
        msg += 9;
    }

    msg += addCRLF(msg);
    return msg - msg_start;
}


int set_status_200(char* msg) {
    // Status 200
    int bytes_added = 15;
    memcpy(
        msg,
        "HTTP/1.0 200 OK",
        bytes_added
    );
    
    return bytes_added + addCRLF(msg + bytes_added);
}



int set_status_404(char* msg) {
    
    char bytes_added = 27;
    memcpy(msg, "HTTP/1.0 404 File not found", bytes_added);
    return bytes_added + addCRLF(msg + bytes_added);
}


int add_data_from_file(char* msg, int fd) {
    // Read file 100 bytes at a time and add to message
    int bytes_read, bytes_added = 0;
    
    do {
        bytes_read      = read(fd, msg + bytes_added, 100);
        bytes_added     += bytes_read;
    } while( bytes_read > 0 );

    return bytes_added;
}

int add_data(char* msg, char* data, int size) {
    memcpy(msg, data, size);
    return size;
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


void respond(int recvd, char* buff, int cfd, char* root) {

     // init response datastructures
    char msg[MSGBUFF];
    char* msg_end = msg;
    char req_fn[REQFNBUFF];


    memset(&req_fn, 0, REQFNBUFF);

    int  root_size      = strlen(root);
    memcpy(req_fn, root, root_size);        // Init requested file path with 'root'

    char std[11]         = "index.html";
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
        msg_end += set_status_404(msg_end);
        msg_end += set_header_content_type(msg_end, HTML);
        msg_end += addCRLF(msg_end);
        msg_end += add_data(msg_end, RESPONSE404, RESPONSE404LEN );
    } else {
        msg_end += set_status_200(msg_end);
        msg_end += set_header_content_type(msg_end, find_content_type(req_fn));
        msg_end += addCRLF(msg_end);
        msg_end += add_data_from_file(msg_end, fd);
        close(fd);
    }

    // Send it 
    int bytes_sent = send(cfd, msg, msg_end - msg, 0);
    printf("Sent %ld bytes\n", msg_end - msg);

}