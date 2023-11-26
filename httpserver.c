#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/uio.h>
#include <fcntl.h>
#include "bind.h"

int parseRequest(char buf[], int caseNumber, char* passbyPointer){
    char* pointerParser;
    int jumpTo;
    switch (caseNumber) {
        case 5: jumpTo = 5; break;
        case 6: jumpTo = 6; break;
        case 10: jumpTo = 10; break;
    }

    if (jumpTo == 5){
        char* ref = strstr(buf, "/");
        int i;
        int counter = 0;
        for (i = 1; ref[i] != ' '; i++){
            counter++;
        }
        return counter;
    }
    else if (jumpTo == 6){//size of buf
        int counter = 0;
        for (int i = 1; buf[i] != ' '; i++){
            passbyPointer[counter] = buf[i];
            counter += 1;
        }
    }
    else if (jumpTo == 10){

        char* cl = strtok_r(&buf[16], "\r\n", &pointerParser);
        return atoi(cl);
    }
    return -1;
}
void parser(char buf[], char* parsemessage, int counter){
    int tracker = 0;
    for(int i = 4; tracker < counter; i++){
        parsemessage[tracker] = buf[i];
        tracker += 1;
    }
}

void sendresponse(int socketD, char* casecode, int caseNumber, int contentLength, char* req){//HTTP/1.1 201 Created\r\nContent-Length: 8\r\n\r\nCreated\n
//    int lengthofN = floor(log10(abs(contentLength)))+1;//length of a number
    char lL[50];
    sprintf(lL, "%d", contentLength);
    int lengthofN = strlen(lL);
    if ((strcmp(req, "GET") == 0) || (strcmp(req, "HEAD") == 0)){

        char sendMessage[32 + lengthofN + strlen(casecode) + 3];
        sprintf(sendMessage, "HTTP/1.1 %d %s\r\nContent-Length: %d\r\n\r\n", caseNumber, casecode, contentLength);
        write(socketD, sendMessage, strlen(sendMessage));
    }
    else{
        char sendMessage[32 + lengthofN + strlen(casecode) + 3 + contentLength];
        sprintf(sendMessage, "HTTP/1.1 %d %s\r\nContent-Length: %d\r\n\r\n%s\n", caseNumber, casecode, contentLength, casecode);
        write(socketD, sendMessage, strlen(sendMessage));
    }
}

int illChecker(char buf[], int iterator){
    if (iterator == 0){
        char str[20], str1[40], str2[20];
        int hi = sscanf(buf, "%s %s %s", str, str1, str2);
        if((strlen(str) > 8) || (strlen(str1) > 19)){
            return -1;
        }
        if (hi != 3){return -1;}
        return illChecker(strstr(buf, "\r\n"), iterator+1);
    }
    else if(buf[2] == '\r'){
        return 0;
    }
    else{
        int counter = 0;
        int colon = 0;
        int space = 0;
        int bins = 0;
        for (int i = 2; buf[i] != '\r'; i++){
            bins = buf[i];
            if (buf[i] == ':' & colon == 0){
                colon +=1;
                counter += 1;
            }
            if (buf[i] == ' '){
                space = counter +1;
                counter += 1;
            }
        }
        if (colon != 1 || space != 2){
            return -1;
        }
        return illChecker(strstr(&buf[2], "\r\n"), iterator);
    }
}

int main(int argc, char* argv[]) {
    if(argc != 2){
        return -1;
    }
    // char originalBuf [2049];
    int file_dPut;
    struct stat bufferStat;

    char* port_number = argv[argc-1];
    int socket_descriptor = create_listen_socket(atoi(port_number));
    while(1){
        char originalBuf [2049];
        int client_socket = accept(socket_descriptor, NULL, NULL);
        char* ending;
        int bytes_read=0; int totalbytes = 0;
        while ((bytes_read = (recv(client_socket, &originalBuf[totalbytes], 2048 - totalbytes, 0)))>0){
            totalbytes += bytes_read;
            ending = strstr(originalBuf, "\r\n\r\n");
            if (ending != NULL){
                break;
            }
        }
        
        int ill = illChecker(originalBuf, 0);
        if (ill == -1){
            sendresponse(client_socket, "Bad Request", 400, 12, "NULL");
            close(client_socket);
            continue;
        }
        char* buf = strdup(originalBuf);

        int filesnameLength = parseRequest(buf, 5, NULL);
        if (filesnameLength > 18){
            sendresponse(client_socket, "Bad Request", 400, 12, "NULL");
            close(client_socket);
            free(buf);
            continue;
        }
        char fileName[filesnameLength+1];//CHECK FOR URI < 19 CHARACTERS LONG!!!!
        
        char method[5];
        sscanf(buf, "%s %*c%s", method, fileName);

        

        if (ending == NULL){
            printf("error");
        }
        if(strstr(buf, "HTTP/1.1") == NULL){
            sendresponse(client_socket, "Bad Request", 400, 12, "NULL");
            free(buf);
            close(client_socket);
            continue;
        }
        
        int fileSize;

        if (access(fileName, F_OK) == -1 & strncmp(method, "PUT", 3) != 0){
            sendresponse(client_socket, "Not Found", 404, 10, "NULL");
            free(buf);
            close(client_socket);
            continue;
        }
        else if (access(fileName, R_OK) == -1 & strncmp(method, "PUT", 3) != 0){
            sendresponse(client_socket, "Forbidden", 403, 10, "NULL");
            free(buf);
            close(client_socket);
            continue;
        }
        else{
            stat(fileName, &bufferStat);
            if (bufferStat.st_mode & S_IFDIR){
                sendresponse(client_socket, "Forbidden", 403, 10, "NULL");
                free(buf);
                close(client_socket);
                continue;
            }
            fileSize = bufferStat.st_size;
        }
        if (strncmp(method, "PUT", 3) == 0 & strlen(method) == 3){//hello there
            int contentLength = parseRequest(strstr(buf, "Content-Length: "), 10, NULL);
            char message_body[contentLength+1];
            char*result = strstr(originalBuf, "\r\n\r\n");
            int position = (result - originalBuf) + 3;
            //int substring = strlen(originalBuf) - position;
//--------------------------------------------------------------------------------------------------parsing message body
            int messageTotalbytes = 0;

            if ((totalbytes-1) > position){
                parser(&ending[4], message_body, messageTotalbytes = (totalbytes-position-1));
            }
            while((totalbytes-1) != (position + contentLength)){
                bytes_read = recv(client_socket, &message_body[messageTotalbytes], contentLength - messageTotalbytes, 0);
                totalbytes += bytes_read;
                messageTotalbytes += bytes_read;
            }
            //printf("%s", message_body);
            int messageNumber;
            if (access(fileName, F_OK) == -1){
                messageNumber = 201;
            }
            else{
                messageNumber = 200;
            }
            file_dPut = open(fileName, O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
            if (file_dPut < 0){
                sendresponse(client_socket, "Forbidden", 403, 10, "NULL");
                close(file_dPut);
                close(client_socket);
                free(buf);
                continue;
            }
            write(file_dPut, message_body, messageTotalbytes);
            close(file_dPut);
            sendresponse(client_socket, "Created", messageNumber, 8, "PUT");

        }


        else if (strncmp(method, "GET", 3) == 0 & strlen(method) == 3){
            sendresponse(client_socket, "OK", 200, fileSize, "GET");
            int getBytes_read = 0;
            int sendbytes_read = 0;

            char sendbuf[fileSize];
            int fdGET = open(fileName, O_RDONLY);
            if (fdGET < 0){
                sendresponse(client_socket, "Forbidden", 403, 10, "NULL");
                close(fdGET);
                close(client_socket);
                free(buf);
                continue;
            }
            while (sendbytes_read != fileSize){
                getBytes_read = read(fdGET, &sendbuf[sendbytes_read], 4096);
                write(client_socket, &sendbuf[sendbytes_read], getBytes_read);
                sendbytes_read += getBytes_read;
            }

            close(fdGET);
        }
        else if (strncmp(method, "HEAD", 4) == 0 & strlen(method) == 4){
            sendresponse(client_socket, "OK", 200, fileSize, "HEAD");
        }
        else{
            sendresponse(client_socket, "Not Implemented", 501, 16, "NULL");
            close(client_socket);
            free(buf);
            continue;
        }
        close(client_socket);
        free(buf);
    }

    return 0;
}
