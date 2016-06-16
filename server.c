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

#define MAX_PENDING 5
#define MAX_LINE 256

int main()
{

    
    if (argc != 2) {
       fprintf(stderr,"usage: %s <port>", argv[0]);
       exit(0);
    }

    int serverPort = atoi(argv[1]);
    struct sockaddr_in sin, clientaddr;
    char buf[MAX_LINE];
    int len, clilen;
    int s, new_s, pid;

    /* build address data structure */
    bzero((char *)&sin, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = INADDR_ANY;
    sin.sin_port = htons(serverPort);

    /* setup passive open */
    if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        perror("simplex-talk: socket");
        exit(1);
    }
    if ((bind(s, (struct sockaddr *)&sin, sizeof(sin))) < 0) {
        perror("simplex-talk: bind");
        exit(1);
    }
    listen(s, MAX_PENDING);
    clilen = sizeof(clientaddr);
    /* wait for connection, then receive and print text */
    while(1) {
        if ((new_s = accept(s, (struct sockaddr *)&clientaddr, &clilen)) < 0) {
            perror("simplex-talk: accept");
            exit(1);
        }
        socklen_t temp = sizeof(sin);
        if (getpeername(new_s, (struct sockaddr *)&sin, &temp) == 0) {
            char ipAddress[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &(sin.sin_addr), ipAddress, INET_ADDRSTRLEN);
            printf("IP Remoto: %s\n", ipAddress);
            printf ("Porta remota: %d\n", (int) ntohs(sin.sin_port));
        }

        pid = fork();
        //fazemos um fork para que cada cliente tenha seu próprio processo
        if (pid < 0) {
            printf("Error on fork!\n");
        }

        if (pid == 0) {
	    close(s);
            //client process
            while (len = recv(new_s, buf, sizeof(buf), 0))
                fputs(buf, stdout);
            close(new_s);
        } else {
            close(new_s);
        }
    }
}
