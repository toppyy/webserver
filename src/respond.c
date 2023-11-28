#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <math.h>
#include <string.h>
#include <stdio.h>

#include <respond.h>
#include <parse.h>

#define HEADERSIZE  100 // Bytes allocated for header
#define REQFNBUFF   500 // Bytes allocated for filename

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

int set_header_content_length(char* msg, int content_length) {
    char *msg_start = msg;
    
    memcpy(msg, "Content-Length: ", 16);
    msg += 16;

    int number_len = (int)((ceil(log10(content_length)))*sizeof(char));

    char snum[number_len];
    sprintf(snum, "%d", content_length);
    memcpy(msg, snum, number_len);
    msg += number_len;

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


void respond(int recvd, request req, int cfd, char* root) {

    // String containing the filename requested (appended to root)
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
            req.ct = find_content_type(def);
        }
        // Try to open the file at path res_fn
        fd = open(res_fn, O_RDONLY);

    }

    char* msg;
    char* msg_end;
    
    if (fd == -1) {
        // Requested file (nor index.html) not found
        msg = malloc(HEADERSIZE);
        memset(msg, 0, HEADERSIZE);
        msg_end = msg;

        msg_end += set_status_404(msg_end);
        msg_end += set_header_content_type(msg_end, HTML);
        msg_end += addCRLF(msg_end);
        msg_end += add_data(msg_end, RESPONSE404, RESPONSE404LEN );
    } else {
        // Requested file found. Count size, allocate memory for and send it
        struct stat sb;
        fstat(fd, &sb);

        msg = malloc(sb.st_size + HEADERSIZE); // Size of file + bytes for header
        msg_end = msg;
        
        msg_end += set_status_200(msg_end);
        msg_end += set_header_content_type(msg_end, req.ct);
        msg_end += set_header_content_length(msg_end, sb.st_size);
        msg_end += addCRLF(msg_end);
        msg_end += add_data_from_file(msg_end, fd);
        close(fd);
    }


    // Send it 
    int bytes_sent = send(cfd, msg, msg_end - msg, 0);
    printf("Sent %ld bytes\n", msg_end - msg);
    
    free(msg);

}