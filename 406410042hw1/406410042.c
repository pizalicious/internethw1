#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>

char webpage[] =
"HTTP/1.1 200 OK\r\n"
"Content-Type: text/html; charset=UTF-8\r\n\r\n"
"<!DOCTYPE html>\r\n"
"<html><head><title>ShellWaveX</title>\r\n"
"<style>body { background-color: #BEBEBE }</style></head>\r\n"
"<body><center><h1>YEEEEEEEEEEEEEEEE!</h1><br>\r\n"
"<form action=\"uploadFile\" method=\"post\" enctype=\"multipart/form-data\">\r\n"
"<input type=\"file\" name=\"file\" accept=\"image/gif,image/jpeg,image/png\"</br>\r\n>"
"<input type=\"submit\" name=\"submit\" value=\"submit\">\r\n"
"</form>\r\n"
"</center></body></html>\r\n"
"<img src=\"doctest.jpg\"></center></body></html>\r\n";

int main(int argc, char *argv[]){
    struct sockaddr_in server_addr, client_addr;
    socklen_t sin_len = sizeof(client_addr);
    int fd_server, fd_client;
    char buf[1048576];
    int fdimg;
    int on = 1;

    fd_server = socket(AF_INET, SOCK_STREAM, 0);
    if(fd_server < 0){
        perror("socket");
        exit(1);
    }
    setsockopt(fd_server, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(int));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(8080);
    
    if(bind(fd_server, (struct sockaddr *) &server_addr, sizeof(server_addr)) == -1){
        perror("bind");
        close(fd_server);
        exit(1);
    }

    if(listen(fd_server, 10) == -1){
        perror("listen");
        close(fd_server);
        exit(1);
    }

    while(1){
        fd_client = accept(fd_server, (struct sockaddr *) &client_addr, &sin_len);

        if(fd_client == -1){
            perror("Connection failed....\n");
            continue;
        }

        printf("Got client connection......\n");

        if(!fork()){
            /*child*/
            close(fd_server);
            memset(buf, 0, 1048576);
            int reallen = read(fd_client, buf, 1048575);
            //printf("%s\n", buf);
            char imgname[1000], tmp[1048576], *ptr;
            int i, len;
            if(!strncmp(buf, "POST /uploadFile", 16)){
                ptr = strstr(buf, "Content-Length");
                ptr = ptr + 16;
                for(i = 0; *ptr!='\0'; i++)
                    tmp[i] = *ptr++;
                tmp[i]='\0';
                printf("tmp = %s\n", tmp);
                len = atoi(tmp);
                if(len>reallen)
                    while((reallen = read(fd_client, tmp, 1048575))!=0)
                        strcat(buf,tmp);
                ptr = strstr(buf, "filename");
                if( ptr!=NULL ){
                    FILE *fd;

                    strcpy(tmp, ptr);
                    ptr = strtok(tmp, "\n");
                    ptr = ptr + 10;
                    for(i = 2; *ptr!='\"' ; i++)
                        imgname[i] = *ptr++;
                    imgname[0]='.';
                    imgname[1]='/';
                    imgname[i]='\0';
                    printf("imgname = %s\n", imgname);
                    
                    fd = fopen(imgname, "w");

                    ptr = strstr(buf, "Content-Type: image");
                    ptr = strtok(ptr, "\n");
                    ptr = strtok(NULL, "\n");
                    printf("ptr = %s\n", ptr);
                    for(i = 2; i < len; i++)
                        fprintf(fd, "%c", ptr[i]);
                    printf("\n");
                }
                write(fd_client, webpage, sizeof(webpage) - 1);
            }
            else if(!strncmp(buf, "GET /favicon.ico", 16)){
                fdimg = open("favicon.ico", O_RDONLY);
                sendfile(fd_client, fdimg, NULL, 4000);
                close(fdimg);
            }
            else if(!strncmp(buf,"GET /doctest.jpg", 16 )){
                fdimg = open("doctest.jpg", O_RDONLY);
                sendfile(fd_client, fdimg, NULL, 400000);
                close(fdimg);
            }
            else
                write(fd_client, webpage, sizeof(webpage) - 1);
            
            close(fd_client);
            printf("closing...\n\n\n");
            exit(0);

        }
        /*parent*/
        close(fd_client);
    }
    return 0;
}