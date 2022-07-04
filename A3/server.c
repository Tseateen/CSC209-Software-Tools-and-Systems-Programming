#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <getopt.h>
#include <errno.h>

// The maximum length of an HTTP message line
#define MAX_LINE 256
// The maximum length of an HTTP response message
#define MAX_LENGTH 16*1024
// The size of a chunk of HTTP response to read from the pipe
#define CHUNK_SIZE 1024


void printError(char *);
void printServerError();
void printResponse(char *str);

int debug = 0;


int main(int argc, char **argv) {
    char msg[MAX_LENGTH];

    FILE *fp = stdin; // default is to read from stdin

    // Parse command line options.
    int opt;
    while((opt = getopt(argc, argv, "v")) != -1) {
        switch(opt) {
            case 'v':
                debug = 1;
                break;
            default:
                fprintf(stderr, "Usage: %s [-v] [filename]\n", argv[0]);
                exit(1);
        }
    }
    if(optind < argc) {
        if((fp = fopen(argv[optind], "r")) == NULL) {
            perror("fopen");
            exit(1);
        }
    }


    // TODO Complete program
    char str[MAX_LINE];
    while(fgets(str, MAX_LINE, fp)) {
        if (strlen(str) < 3) continue;
        char *c = strtok(str, " ");
        if (strcmp(c, "GET") != 0) continue;
        char p_q[2][MAX_LINE + 1]; // path and query
        c = strtok(NULL," ");
        if (strcmp(c, "/die H") == 0) continue;
        c = strtok(c, "?");
        
        if (c) {
            strcpy(p_q[0], c);
        } else {
            strcpy(p_q[0], "\0");
        }
        c = strtok(NULL,str);
        if (c) {
            strcpy(p_q[1], c);
        } else {
            strcpy(p_q[1], "\0");
        }
        int pp[2];
        if (pipe(pp) < 0) {perror("pipe"); exit(1);}

        int p = fork();

        if (p < 0) {perror("fork"); exit(1); }
        else if(p == 0) { 
            
            if (close(pp[0]) < 0) {
                perror("close");
                exit(1);
            } 
                
            if (setenv("QUERY_STRING", p_q[1], 1) < 0) {
                perror("setenv");
                exit(1);
            }
            if (dup2(pp[1], 1) == -1) {
                perror("dup2");
                exit(1);
            }
            char path[MAX_LINE+ 1];
            strcpy(path, p_q[0]);
            char file_name[MAX_LINE] = "\0";
            char *toke = strtok(path, "/");
            
            while (toke) {
                strcpy(file_name, toke);
                toke = strtok(NULL, path);
            }
            char path_new[MAX_LENGTH] = ".\0";
            strcat(path_new, p_q[0]);
            if (execl(path_new, file_name, NULL) == -1) {
                perror("execl");
                if (write(pp[1], p_q[0], strlen(p_q[0])) == -1) {
                    perror("write");
                    exit(1);
                }
                exit(errno);    
            }
            
            if (close(pp[1]) < 0) {
                perror("close");
                exit(1);
            }
            exit(0);
        } else {
            if(close(pp[1]) < 0) {
                perror("close");
                continue;
            } 
            //char info[CHUNK_SIZE];
            if (read(pp[0], msg, MAX_LENGTH) == -1) {
                perror("read");
                continue;
            }

            int status;
            if (wait(&status) == -1) {
                perror("wait");
                continue;
            }
            if (WIFSIGNALED(status) != 0) {
                printServerError();
                if (close(pp[0]) < 0) {
                    perror("close");
                    continue;
                }
                continue;
            }
            switch (WEXITSTATUS(status))
            {
            case 0:
                printResponse(msg);
                break;
            
            case 1:
                printServerError();
                break;
            
            case 2:
                printError(msg);
                break;

            default:
                printServerError();
                break;
            }
            if (close(pp[0]) < 0) {
                perror("close");
                continue;
            }
        }
    }

    if(fp != stdin) {
        if(fclose(fp) == EOF) {
            perror("fclose");
            exit(1);
        }
    }
}


/* Print an http error page  
 * Arguments:
 *    - str is the path to the resource. It does not include the question mark
 * or the query string.
 */
void printError(char *str) {
    printf("HTTP/1.1 404 Not Found\r\n\r\n");

    printf("<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\">\n");
    printf("<html><head>\n");
    printf("<title>404 Not Found</title>\n");
    printf("</head><body>\n");
    printf("<h1>Not Found</h1>\n");
    printf("The requested resource %s was not found on this server.\n", str);
    printf("<hr>\n</body></html>\n");
}


/* Prints an HTTP 500 error page 
 */
void printServerError() {
    printf("HTTP/1.1 500 Internal Server Error\r\n\r\n");

    printf("<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\">\n");
    printf("<html><head>\n");
    printf("<title>500 Internal Server Error</title>\n");
    printf("</head><body>\n");
    printf("<h1>Internal Server Error</h1>\n");
    printf("The server encountered an internal error or\n");
    printf("misconfiguration and was unable to complete your request.<p>\n");
    printf("</body></html>\n");
}


/* Prints a successful response message
 * Arguments:
 *    - str is the output of the CGI program
 */
void printResponse(char *str) {
    printf("HTTP/1.1 200 OK\r\n\r\n");
    printf("%s", str);
}
