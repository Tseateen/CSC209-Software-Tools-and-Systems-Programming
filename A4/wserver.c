#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>    /* Internet domain header */
#include <errno.h>
#include <arpa/inet.h>

#include "wrapsock.h"
#include "ws_helpers.h"

#define MAXCLIENTS 10

int handleClient(struct clientstate *cs, char *line);

// You may want to use this function for initial testing
//void write_page(int fd);

int main(int argc, char **argv) {

    if(argc != 2) {
        fprintf(stderr, "Usage: wserver <port>\n");
        exit(1);
    }
    unsigned short port = (unsigned short)atoi(argv[1]);
    int listenfd;
    struct clientstate client[MAXCLIENTS];


    // Set up the socket to which the clients will connect
    listenfd = setupServerSocket(port);
    struct timeval *tv = malloc(sizeof(struct timeval));
    tv->tv_sec = 300;
    initClients(client, MAXCLIENTS);

    // TODO: complete this function
    int max_fd = listenfd;
    fd_set all_fds;
    FD_ZERO(&all_fds);
    FD_SET(listenfd, &all_fds);

    struct sockaddr_in addr;
    socklen_t len;
    int result;


    while (1) {
        // select updates the fd_set it receives, so we always use a copy and retain the original.
        fd_set rset = all_fds;
        int sel = Select(max_fd + 1, &rset, NULL, NULL, tv);
        if (sel == -1) {
            perror("server: select");
            exit(1);
        } else if(sel == 0) {
            perror("server: select timeout");
            exit(1);
        }
        if (FD_ISSET(listenfd, &rset)) {
            int mark = 0;
            len = sizeof(addr);
            int client_fd = Accept(listenfd, (struct sockaddr *)&addr, &len);
            if (client_fd < 0) {
                perror("server: accept error");
                exit(1);
            }
            FD_SET(client_fd, &all_fds);
            if (client_fd > max_fd) {
                max_fd = client_fd;
            }
            printf("connection from %s\n", inet_ntoa(addr.sin_addr));
            for (int check = 0; check < MAXCLIENTS; check++) {
                if (client[check].sock == client_fd) {
                    mark = 1;
                    break;
                }
            }
            if (mark == 0) {
                for (int i = 0; i < MAXCLIENTS; i++) {
                    if (client[i].sock == -1) {
                        client[i].sock = client_fd;
                        break;
                    }
                }
            }
        }
        for(int i = 0; i <= max_fd; i++) {
            if (FD_ISSET(i, &rset)) {
                for (int index = 0; index < MAXCLIENTS; index++) {   
                    if (client[index].sock == i) {
                        char buf[MAXLINE];
                        memset(buf, 0, MAXLINE);
                        int rnum = read(client[index].sock, buf, MAXLINE - 1);
                        if (rnum <=0) {
                            if(rnum < 0) perror("read");
                            else printf("Disconnected\n");
                            FD_CLR(client[index].sock, &all_fds);
                            close(client[index].sock);
                            resetClient(&client[index]);
                            continue;;
                        }
                        buf[rnum] = '\0';
                        printf("Received %d bytes: %s", rnum, buf);
                        result = handleClient(&(client[index]), buf);
                        if (result == -1) {
                            FD_CLR(client[index].sock, &all_fds);
                            close(client[index].sock);
                            resetClient(&client[index]);
                        } else if (result == 1) {
                            
                            int pr = processRequest(&client[index]);
                            
                            if(pr < 0) {
                                if (pr == -1) printServerError(client[index].sock);
                                FD_CLR(client[index].sock, &all_fds);
                                close(client[index].sock);
                                resetClient(&client[index]);
                            }
                            else {
                                if (pr > max_fd) max_fd = pr;
                                FD_SET(pr, &all_fds);
                            }
                        }
                        
                        break;
                    }
                }
            }
        }
        
        for(int i = 0; i <= max_fd; i++) {
            if (FD_ISSET(i, &rset)) {
                for (int index = 0; index < MAXCLIENTS; index++) {
                    if (client[index].fd[0] == i) {
                        int n;
                        if ((n = read(client[index].fd[0], client[index].optr, MAXLINE)) < 0) {
                            perror("read");
                            FD_CLR(client[index].sock, &all_fds);
                            close(client[index].sock);
                            resetClient(&client[index]);
                            break;
                        } else if (n == 0){
                            int status = 0;
                            waitpid(client[index].fd[1], &status,0);
                            if (WEXITSTATUS(status) == 0) printOK(client[index].sock, client[index].output, strlen(client[index].output)); 
                            else if (WEXITSTATUS(status)== 100) printNotFound(client[index].sock);
                            else printServerError(client[index].sock);
                            FD_CLR(client[index].fd[0], &all_fds);
                            close(client[index].fd[0]);
                            FD_CLR(client[index].sock, &all_fds);
                            close(client[index].sock);
                            resetClient(&(client[index]));
                        } else {
                            client[index].optr += n;
                        }

                        break;
                    }
                    
                }
            }
        }
    }
        
    return 0;
}

/* Update the client state cs with the request input in line.
 * Intializes cs->request if this is the first read call from the socket.
 * Note that line must be null-terminated string.
 *
 * Return 0 if the get request message is not complete and we need to wait for
 *     more data
 * Return -1 if there is an error and the socket should be closed
 *     - Request is not a GET request
 *     - The first line of the GET request is poorly formatted (getPath, getQuery)
 * 
 * Return 1 if the get request message is complete and ready for processing
 *     cs->request will hold the complete request
 *     cs->path will hold the executable path for the CGI program
 *     cs->query will hold the query string
 *     cs->output will be allocated to hold the output of the CGI program
 *     cs->optr will point to the beginning of cs->output
 */
int handleClient(struct clientstate *cs, char *line) {
    if (cs -> request == NULL){ 
        cs->request = malloc(MAXLINE*sizeof(char));
        strcpy(cs->request, line);
    } else strcat(cs->request, line);
    char *mark;
    mark = strstr(cs->request, "\r\n\r\n"); 
    if (mark==NULL) return 0;
    
    if (strncmp(cs->request, "GET", 3) != 0) {
        fprintf(stderr, "Invalid request type: %s\n", cs->request);
        return -1;
    }
    cs -> path = getPath(cs->request);
    cs -> query_string = getQuery(cs->request);
    if (!cs->path) {
        cs->path=malloc(MAXLINE);
        strcpy(cs->path,"\0");
    }
    if (!cs->query_string) {
        cs->query_string=malloc(MAXLINE);
        strcpy(cs->query_string,"\0");
    }
    
    // If the resource is favicon.ico we will ignore the request
    if(strcmp(cs->path, "favicon.ico") == 0){
        // A suggestion for debugging output
        fprintf(stderr, "Client: sock = %d\n", cs->sock);
        fprintf(stderr, "        path = %s (ignoring)\n", cs->path);
		printNotFound(cs->sock);
        return -1;
    }
    if (cs->output==NULL) {
        cs -> output = malloc(MAXPAGE);
        cs -> optr = cs->output;
    }
    

    // A suggestion for printing some information about each client. 
    // You are welcome to modify or remove these print statements
    fprintf(stderr, "Client: sock = %d\n", cs->sock);
    fprintf(stderr, "        path = %s\n", cs->path);
    fprintf(stderr, "        query_string = %s\n", cs->query_string);

    return 1;
}

