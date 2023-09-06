#include <unistd.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>

#include <respond.h>
#include <parse.h>

#define MSGBUFF     50000
#define REQFNBUFF   50

#define RESPONSE404 "<html><body>404 - Nothing here, sorry</body></html>"
#define RESPONSE404LEN 51


int addCRLF(char* str) {
    str[0] = 13;    // CR
    str[1] = 10;    // LF
    return 2;
}


int set_header_content_type(char* msg, content_type ct) {

    char *msg_start = msg;
    
    memcpy(msg, "Content-Type: ", 14);
    msg += 14;

    printf("ct: %d\n", ct);

    switch(ct) {

        case HTML:
            memcpy(msg, "text/html", 9);
            msg += 9;
            break;

        case IMG_PNG:
            memcpy(msg, "image/png", 9);
            msg += 9;
            break;

        case IMG_GIF:
            memcpy(msg, "image/gif", 9);
            msg += 9;
            break;

        case IMG_JPEG:
            memcpy(msg, "image/jpeg", 10);
            msg += 10;
            break;

        case JAVASCRIPT:
            memcpy(msg, "text/javascript", 15);
            msg += 15;
            break;

        default:
            memcpy(msg, "text/plain", 10);
            msg += 10;
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


void log_request_info(char* buff, int recvd, char* res_fn) {
    // Print method
    int idx = 0;
    while (buff[idx] != '/' & idx < recvd) {
        printf("%c", buff[idx]);
        idx++;
    }
    // Print requested file
    printf(": %s\n", res_fn);
}


void respond(int recvd, request req, int cfd, char* root) {

     // init response datastructures
    char msg[MSGBUFF];
    char* msg_end = msg;
    char res_fn[REQFNBUFF];


    memset(&res_fn, 0, REQFNBUFF);

    int  root_size      = strlen(root);
    memcpy(res_fn, root, root_size);        // Init requested file path with 'root'

    char def[11]        = "index.html";
    int  def_size       = sizeof(def);
    int  bytes_to_send      = 0;
    int  res_fn_size        = 0;
    int  fd                 = -1;

    // If there's no bytes received, don't try to parse the msg - just 404
    if (recvd > 0) {

        // Add name of requested file (or def) to path
        if (req.filename_size > 0) {
            memcpy(res_fn + root_size, req.filename, req.filename_size);
        } else {
            // If no specific file was requested, return file specified as 'def'
            memcpy(res_fn + root_size, def, def_size);
        }
        // Try to open the file at path res_fn
        fd = open(res_fn, O_RDONLY);

    }
   
    if (fd == -1) {
        msg_end += set_status_404(msg_end);
        msg_end += set_header_content_type(msg_end, HTML);
        msg_end += addCRLF(msg_end);
        msg_end += add_data(msg_end, RESPONSE404, RESPONSE404LEN );
    } else {
        msg_end += set_status_200(msg_end);
        msg_end += set_header_content_type(msg_end, req.ct);
        msg_end += addCRLF(msg_end);
        msg_end += add_data_from_file(msg_end, fd);
        close(fd);
    }

    // Send it 
    int bytes_sent = send(cfd, msg, msg_end - msg, 0);
    printf("Sent %ld bytes\n", msg_end - msg);

}