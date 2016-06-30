/** Lab04
Matheus Figueiredo 137036
João Guilherme Fidelis 136242

*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define MAX_LINE 256

void printName(char *username) {
    printf("[ %s ] ", username);

}


int main(int argc, char * argv[])
{
    FILE *fp;
    struct hostent *hp;
    struct sockaddr_in sin;
    char *host;
    char username[100] = {'\0'};
    char buf[MAX_LINE];
    int s;
    int len;
    int serverPort;
    if (argc==4) {
        host = argv[1];
        serverPort = atoi(argv[2]);
        strcpy(username, argv[3]);
        //printf("n:%d\n", strlen(username));
    }
    else {
        fprintf(stderr, "usage: ./client <hostname> <port> <username>\n");
    exit(1);
}

    /* translate host name into peerâ€™s IP address */
    hp = gethostbyname(host);
    if (!hp) {
        fprintf(stderr, "simplex-talk: unknown host: %s\n", host);
        exit(1);
    }

    /* build address data structure */
    bzero((char *)&sin, sizeof(sin));
    sin.sin_family = AF_INET;
    bcopy(hp->h_addr, (char *)&sin.sin_addr, hp->h_length);
    sin.sin_port = htons(serverPort);

    /* active open */
    if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        perror("simplex-talk: socket");
        exit(1);
    }
    if (connect(s, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
        perror("simplex-talk: connect");
        close(s);
        exit(1);
    }

    //Conectado!

    send(s, username, strlen(username)+1, 0);


    //Nome enviado ao servidor!
    printName(username);
    int pid= fork();
    if (pid == 0) {
        while(recv(s , buf , MAX_LINE , 0) > 0){
            printf("\r");
            fputs(buf, stdout);
            printName(username);
            bzero(buf, MAX_LINE);
        }
        //close(s);
        printf("chegou aqui\n");
    } else {
        while(fgets(buf, sizeof(buf), stdin)){
            buf[MAX_LINE-1] = '\0';
            len = strlen(buf) + 1;
            send(s, buf, len, 0);
            //printName(username);
            printf("%s\n", buf);
            printName(username);
            bzero(buf, MAX_LINE);
        }
        close(s);
    }
    return 0;

}