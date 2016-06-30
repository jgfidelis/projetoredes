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
#include <pthread.h>
#define MAX_LINE 256

pthread_mutex_t lock;
char username[100];
void printName(char *username) {
    printf("[ %s ] ", username);

}

void *reader(void *socket_desc) {
    char recv_msg[MAX_LINE];
    //printf("lendo\n");
    int sock = *(int*)socket_desc;
    int len;

    while((len = recv(sock , recv_msg , MAX_LINE , 0)) > 0){
            pthread_mutex_lock(&lock);
            //printf("\r");
            //fputs(recv_msg, stdout);
            printf("\r%s", recv_msg);
            //printf("\n oi \n oi \n");
            bzero(recv_msg, MAX_LINE);
            //printf("\nAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA ");
            printf("\n[%s] ", username);
            //printf("HELLO\n");//funciona!
            pthread_mutex_unlock(&lock);
        }
        //printf("oi\n");
        //printName(username);
        close(sock);



}

int main(int argc, char * argv[])
{
    FILE *fp;
    struct hostent *hp;
    struct sockaddr_in sin;
    char *host;
    char buf[MAX_LINE];
    int s;
    int len;
    int serverPort;
    char recv_msg[MAX_LINE];
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
    if (pthread_mutex_init(&lock, NULL) != 0)
    {
        printf("\n mutex init failed\n");
        return 1;
    }
    //Conectado!

    send(s, username, strlen(username)+1, 0);


    //Nome enviado ao servidor!
    printName(username);
    pthread_t sniffer_thread;
    int *new_sock = malloc(1);
    *new_sock = s;

    if( pthread_create( &sniffer_thread , NULL ,  reader , (void*) new_sock) < 0)
    {
        perror("could not create thread");
        return 1;
    }
    /*while(1){

        if(fgets(buf, sizeof(buf), stdin) != NULL) {
            buf[MAX_LINE-1] = '\0';
            len = strlen(buf) + 1;
            send(s, buf, len, 0);
        }

        if((len = recv(s, recv_msg, sizeof(recv_msg), 0)) > 0){
            fputs(recv_msg, stdout);
            printf("[%s] ", username);
        }

    }*/
        /*
    int pid= fork();
    if (pid == 0) {
        while((len = recv(s , recv_msg , MAX_LINE , 0)) > 0){
            pthread_mutex_lock(&lock);
            printf("\r");
            fputs(recv_msg, stdout);
            bzero(recv_msg, MAX_LINE);
            printf("[%s] ", username);
            pthread_mutex_unlock(&lock);
        }
        //printf("oi\n");
        //printName(username);
        close(s);
    } else {
        while(fgets(buf, sizeof(buf), stdin)){
            pthread_mutex_lock(&lock);
            buf[MAX_LINE-1] = '\0';
            len = strlen(buf) + 1;
            send(s, buf, len, 0);
            //printName(username);
            //printf("%s\n", buf);
            printName(username);
            bzero(buf, MAX_LINE);
            pthread_mutexunlock(&lock);
        }
        close(s);
    }*/

    while(fgets(buf, sizeof(buf), stdin)){
            pthread_mutex_lock(&lock);
            buf[MAX_LINE-1] = '\0';
            len = strlen(buf) + 1;
            send(s, buf, len, 0);
            //printName(username);
            //printf("%s\n", buf);
            printName(username);
            bzero(buf, MAX_LINE);
            pthread_mutex_unlock(&lock);
        }
    close(s);
    pthread_mutex_destroy(&lock);
    return 0;

}