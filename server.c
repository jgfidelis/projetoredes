/** Projeto de redes
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
#include <pthread.h> //for threading , link with lpthread

void *connection_handler(void *);
#define MAX_PENDING 5
#define MAX_LINE 256

//char userDatabase[100][100];
pthread_mutex_t lock;
int numberOfUsers;
int numberOfGroups;
typedef struct _usuario {
    char username[100];
    int online;
    int socket;
    int hasOfflineMessages;

} Usuario;

typedef struct _group {
    char groupName[100];
    int usersids[50];
    int usersInGroup;
} Group;

Usuario usuarios[100];
Group grupos[100];
void sendToUser(char userDestination[], char mensagem[], char userSource[]);
void logout(int sourceid);

void printUserOnline(int sourceid);
void createGroup(char group[], int sourceid);

void joinGroup(char group[], int sourceid);
void sendToGroup(char groupName[], char mensagem[], char userSource[], int sourceid);

void sendInvalidCommand(int sourceid);

void commandCall(char *command, char restOfString[], char userSource[], int sourceid){
    printf("comparing\n");
    if (strcmp(command, "SEND") == 0){
        //send to userDestination
        //recebe <user> <message>
        //get username destination
        int i;
        char userDestination[100] = {'\0'};
        for (i=0; restOfString[i] != ' '; i++){
            userDestination[i] = restOfString[i];
        }
        userDestination[i] = '\0';
        int len = strlen(userDestination);
        char mensagem[2000];

        for (i = 0; i + len + 1 < 2000 ; i++)
            mensagem[i] = restOfString[i+len+1];
        mensagem[1999] = '\0';

        printf("User: %s\n", userDestination);
        printf("Tamanho %d\n", len);
        printf("Mesage: %s\n", mensagem);

        //envia mensagem a user
        printf("ENVIANDO!\n");
        sendToUser(userDestination, mensagem, userSource);

    }
    else if (strcmp(command, "CREATEG") == 0){
        //create group
        //recebe<group_name>

        //restOfString é o group name
        createGroup(restOfString, sourceid);
    }
    else if (strcmp(command, "JOING") == 0){
        //join group
        //recebe <group_name>
        //restOfString é o group name
        //faz usuário que enviou se juntar ao grupo group name
        joinGroup(restOfString, sourceid);
    }
    else if (strcmp(command, "SENDG") == 0){
        //send message to group
        //recebe <group_name> <msg>
        int i;
        char groupName[100] = {'\0'};
        for (i=0; restOfString[i] != ' '; i++){
            groupName[i] = restOfString[i];
        }
        groupName[i] = '\0';
        int len = strlen(groupName);
        char mensagem[2000];

        for (i = 0; i + len + 1 < 2000 ; i++)
            mensagem[i] = restOfString[i+len+1];
        mensagem[1999] = '\0';

        //envia ao grupo a mensagem
        sendToGroup(groupName, mensagem, userSource, sourceid);

    }
    else if (strcmp(command, "WHO") == 0){
        //imprime lista de usuarios online
        printf("imprimidno\n");
        printUserOnline(sourceid);
    }
    else if (strcmp(command, "EXIT") == 0){
        //faz logout
        logout(sourceid);
    }
    else {
        sendInvalidCommand(sourceid);
    }
}

int main(int argc, char* argv[])
{


    if (argc != 2) {
       fprintf(stderr,"usage: %s <port>", argv[0]);
       exit(0);
    }

     if (pthread_mutex_init(&lock, NULL) != 0)
    {
        printf("\n mutex init failed\n");
        return 1;
    }

    int serverPort = atoi(argv[1]);
    struct sockaddr_in sin, clientaddr;
    char buf[MAX_LINE];
    int len, clilen;
    int s, new_s, pid;
    int *new_sock, new_socket;

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

    while( (new_socket = accept(s, (struct sockaddr *)&clientaddr, (socklen_t*)&clilen)) )
    {
        puts("Connection accepted");

        pthread_t sniffer_thread;
        new_sock = malloc(1);
        *new_sock = new_socket;

        if( pthread_create( &sniffer_thread , NULL ,  connection_handler , (void*) new_sock) < 0)
        {
            perror("could not create thread");
            return 1;
        }

        //Now join the thread , so that we dont terminate before the thread
        //pthread_join( sniffer_thread , NULL);
        puts("Handler assigned");
    }

    if (new_socket<0)
    {
        perror("accept failed");
        return 1;
    }

    pthread_mutex_destroy(&lock);


    return 0;
}

void logout(int sourceid) {
    usuarios[sourceid].online = 0;
    
    int sock = usuarios[sourceid].socket;
    char buffer[1024];
    
    snprintf(buffer, sizeof(buffer), "Você foi desconectado\n");

    send(sock, buffer, strlen(buffer)+1, 0);
}

void sendInvalidCommand(int sourceid) {
    int sock = usuarios[sourceid].socket;
    char buffer[1024];
    
    snprintf(buffer, sizeof(buffer), "Comando inválido\n");

    send(sock, buffer, strlen(buffer)+1, 0);
}

void printUserOnline(int sourceid) {
    int i;
    int sock = usuarios[sourceid].socket;
    char buffer[1024];
    snprintf(buffer, sizeof(buffer), "| usuario | status |\n");
    send(sock, buffer, strlen(buffer)+1, 0);
    bzero(buffer, 1024);
    for (i = 0; i < numberOfUsers; i++){
        if (usuarios[i].online == 1){
            snprintf(buffer, sizeof(buffer), "| %s | online |\n", usuarios[i].username);
            send(sock, buffer, strlen(buffer)+1, 0);
            bzero(buffer, 1024);
        } else {
            snprintf(buffer, sizeof(buffer), "| %s | offline |\n", usuarios[i].username);
            send(sock, buffer, strlen(buffer)+1, 0);
            bzero(buffer, 1024);
        }
    }
}

int registraUsuario(char usernameToRegister[], int sock) {
    int i;
    char buffer[1024];
    for (i = 0; i < numberOfUsers; i++){
            printf("i:%d s:%s\n", i, usuarios[i].username);
        if (strcmp(usuarios[i].username, usernameToRegister) == 0) {
            //ja esta no banco de dados
            //printf("USUARIO JA EXISTIA!\n");
            //ver se ja tava logado!
            if (usuarios[i].online == 1){
                //usuario ja esta logado!
                snprintf(buffer, sizeof(buffer), "Usuário já está logado\n");

                send(sock, buffer, strlen(buffer)+1, 0);
            } else {
                usuarios[i].online = 1;
                usuarios[i].socket = sock;
            }
            return i;
        }
    }

    //aqui ainda nao existia, criar novo
    printf("criando novo usuario %s\n", usernameToRegister);
    strcpy(usuarios[numberOfUsers].username, usernameToRegister);
    usuarios[numberOfUsers].online = 1;
    usuarios[numberOfUsers].socket = sock;
    usuarios[numberOfUsers].hasOfflineMessages = 0;
    numberOfUsers++;
    return numberOfUsers-1;

}
//retorna posicao de user no vetor ou -1 se nao existir
int getUserNumber(char user[]) {
     int i;
    for (i = 0; i < numberOfUsers; i++){
        if (strcmp(usuarios[i].username, user) == 0) {
            return i;
        }
    }

    return -1;
}

//retorna posicao de user no vetor ou -1 se nao existir
int getGroupNumber(char grupo[]) {
     int i;
    for (i = 0; i < numberOfGroups; i++){
        if (strcmp(grupos[i].groupName, grupo) == 0) {
            return i;
        }
    }

    return -1;
}

void sendToUser(char userDestination[], char mensagem[], char userSource[]) {

    char buffer[1024];

    FILE *fp;
    
    int num = getUserNumber(userDestination);
    if (num < 0) {
        //envia erro pro userSource, userDestination nao existe
        num = getUserNumber(userSource);

        int sock = usuarios[num].socket;
    
        snprintf(buffer, sizeof(buffer), "Usuario inexistente\n");
        send(sock, buffer, strlen(buffer)+1, 0);

        return;
    }
    
    printf("Usuario achado em %d\n", num);
    snprintf(buffer, sizeof(buffer), "[%s>] %s", userSource, mensagem);
    
    if (usuarios[num].online == 1) {
        //envia ja
        int sockUser = usuarios[num].socket;

        buffer[1023] = '\0';

        int len = strlen(buffer) + 1;

        printf("enviando para %s\n", userDestination);

        send(sockUser, buffer, len, 0);
    } else {
        //TODO colocar para enviar depois
        printf("Colocando msgm no arquivo\n");
        fp = fopen(userDestination, "a+");
        fputs(buffer, fp);
        fclose(fp);

        usuarios[num].hasOfflineMessages += 1;

        num = getUserNumber(userSource);

        int sock = usuarios[num].socket;
    
        snprintf(buffer, sizeof(buffer), "Usuario offline, sua mensagem sera estregue proxima vez que este usuario entrar\n");
        send(sock, buffer, strlen(buffer)+1, 0);
    }
}

void createGroup(char group[], int sourceid) {
    //procurar no vetor e ver se grupo ja esta criado
    int i;
    for (i = 0; i < numberOfGroups; i++){
        if (strcmp(group, grupos[i].groupName) == 0) {
            //grupo ja existe
            int sock = usuarios[sourceid].socket;
            char buffer[1024];
    
            snprintf(buffer, sizeof(buffer), "Grupo ja existente\n");

            send(sock, buffer, strlen(buffer)+1, 0);
            return;
        }
    }

    //chegou aqui eh para criar
    strcpy(grupos[numberOfGroups].groupName, group);
    grupos[numberOfGroups].usersids[0] = sourceid;
    grupos[numberOfGroups].usersInGroup=1;
    //avisar que deu certo?
    numberOfGroups++;

}

void joinGroup(char group[], int sourceid) {
    //procurar no vetor e ver se grupo ja esta criado
    int i;
    char buffer[1024];

    int sock = usuarios[sourceid].socket;

    for (i = 0; i < numberOfGroups; i++){
        if (strcmp(group, grupos[i].groupName) == 0) {
            int num =grupos[i].usersInGroup;
            //ver se usuario ja não esta no grupo
            int j;
            for (j = 0; j < num; j++){
                if (grupos[i].usersids[j] == sourceid){
                    //usuario ja esta no grupo
                    snprintf(buffer, sizeof(buffer), "Você já faz parte deste grupo\n");

                    send(sock, buffer, strlen(buffer)+1, 0);
                    return;
                }
            }
            grupos[i].usersids[num] = sourceid;
            grupos[i].usersInGroup++;
            //avisar que deu certo
            snprintf(buffer, sizeof(buffer), "Você foi adicionado ao grupo com sucesso\n");

            send(sock, buffer, strlen(buffer)+1, 0);
            return;
        }
    }
}

void sendToGroup(char groupName[], char mensagem[], char userSource[], int sourceid) {
    char buffer[2000];
    snprintf(buffer, sizeof(buffer), "(Em: %s) [%s>] %s", groupName, userSource, mensagem);

    int sock = usuarios[sourceid].socket;
    int num = getGroupNumber(groupName);
    if (num < 0) {
        //avisar que deu ruim
        snprintf(buffer, sizeof(buffer), "Grupo não existe\n");

        send(sock, buffer, strlen(buffer)+1, 0);
        return;
    }

    int i;
    //mandar mensagem para todos
    for (i = 0; i < grupos[num].usersInGroup; i++) {
        int id = grupos[num].usersids[i];
        if (id != sourceid) {
            if (usuarios[id].online == 1) {
                //enviar
                buffer[1999] = '\0';
                int len = strlen(buffer) + 1;
                send(usuarios[id].socket, buffer, len, 0);
            } else {
                //TODO usuario esta offline!
                //salvar em aquivo
            }
        }
    }

}

void sendOfflineMessages (int id) {
    char buffer[1024];
    int sock = usuarios[id].socket;
    FILE *fp;

    fp = fopen(usuarios[id].username, "r");

    printf("Usuario tem msgns offline, mandando...\n");

    int numOfMessages = usuarios[id].hasOfflineMessages;

    while (numOfMessages) {
        fgets(buffer, 1024, fp);

        send(sock, buffer, strlen(buffer)+1, 0);

        bzero(buffer, 1024);

        numOfMessages--;
        sleep(1);
    }

    usuarios[id].hasOfflineMessages = 0;

    printf("Feito, reescrevendo arquivo...\n");
    freopen(usuarios[id].username, "w", fp);
    fclose(fp);
}

void *connection_handler(void *socket_desc)
{
    //Get the socket descriptor
    int sock = *(int*)socket_desc;
    int read_size;
    char *message , client_message[2000];
    char username[100];
    int id;
    //Pegar o username dessa thread
    if (recv(sock , client_message , 2000 , 0) >  0){
        strcpy(username, client_message);
        bzero(client_message, 2000);
        //printf("User: %s logged in\n", username);

        //peguei o lock
        //strcpy(userDatabase[numberOfUsers], username);
        //numberOfUsers++;
        pthread_mutex_lock(&lock);
        id = registraUsuario(username, sock);
        if (usuarios[id].hasOfflineMessages) {
            sendOfflineMessages(id);
        }
        pthread_mutex_unlock(&lock);
        printf("usuario %s registrado\n", client_message);

        //liberei o lock

    }
    //Receive a message from client
    while( (read_size = recv(sock , client_message , 2000 , 0)) > 0 )
    {
        printf("%s\n", client_message);
        int tamanhoMensagem = strlen(client_message);
        int i;
        char command[10] = {'\0'};
        char mensagem[2000];
        i = 0;
        /*while(1) {
            if (client_message[i] != '\0' && client_message[i] != ' ' && client_message[i] != '\n'){
                printf("i:%d e c:%c\n", i, client_message[i]);
                command[i] = client_message[i];
                i++;
            } else {
                break;
            }
        }*/

        for(i=0; client_message[i] != ' ' && client_message[i] != '\0' && client_message[i] != '\n'; i++) {
            //printf("i:%d e c:%c\n", i, client_message[i]);
            command[i] = client_message[i];
        }
        command[i] = '\0';
        int lenCommand = strlen(command);

        for (i = 0; i + lenCommand + 1 < 2000 ; i++)
            mensagem[i] = client_message[i+lenCommand+1];
        mensagem[1999] = '\0';

        bzero(client_message, 2000);
        printf("command: %s\n", command);
        printf ("tamanhos : %d\n", lenCommand);
        printf("Mensagem: %s\n", mensagem);
        pthread_mutex_lock(&lock);
        commandCall(command, mensagem, username, id);
        pthread_mutex_unlock(&lock);
        //Send the message back to client
        //write(sock , client_message , strlen(client_message));
    }

    if(read_size == 0)
    {
        logout(id);
        puts("Client disconnected");
        fflush(stdout);
    }
    else if(read_size == -1)
    {
        perror("recv failed");
    }

    //Free the socket pointer
    free(socket_desc);

    return 0;
}
