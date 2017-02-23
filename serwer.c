#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>

#define QUEUE_SIZE 2

//shared variables
int end = 0;
int last_client_id = 0;
int lista_klientow[50];

//client variables
int c_deskryptor_gniazda;

//server_variables
int s_deskryptor_gniazda;
	

char * getline_new(void) {
    char * line = malloc(100), * linep = line;
    size_t lenmax = 100, len = lenmax;
    int c;

    if(line == NULL)
        return NULL;

    for(;;) {
        c = fgetc(stdin);
        if(c == EOF)
            break;

        if(--len == 0) {
            len = lenmax;
            char * linen = realloc(linep, lenmax *= 2);

            if(linen == NULL) {
                free(linep);
                return NULL;
            }
            line = linen + (line - linep);
            linep = linen;
        }

        if((*line++ = c) == '\n')
            break;
    }
    *line = '\0';
    return linep;
}


void* input_loop(void *arg) {
	char * read_line;
	int j;
	while(end == 0){
		read_line[0] = '\0';
		printf("Podaj komendę do wykonania na serwerach:\n");
		read_line = getline_new();
		if(strcmp(read_line, "end\n") == 0){
			end = 1;
		}

		for(j = 0; j < last_client_id; j++){
			send(lista_klientow[j], read_line, strlen(read_line)-1,0);
		}
	}
	close(s_deskryptor_gniazda);
	exit(0);
}


void* client_loop(void *arg) {
	int read;
	int k;
	char c_server_reply[100];
	while(end == 0){
		read = recv(c_deskryptor_gniazda, c_server_reply, 100, 0);
		c_server_reply[read] = '\0';
		if(strcmp(c_server_reply, "end") == 0){
			end = 1;
		} else {
			system(c_server_reply);
		}
		for(k = 0; k < last_client_id; k++){
			send(lista_klientow[k], c_server_reply, strlen(c_server_reply),0);
		}
	}
	close(s_deskryptor_gniazda);
	exit(0);
}


int main(){

	char * serwer_matka;
	
	//client vars
	struct sockaddr_in c_server;
	char *c_server_ip;
	short c_service_port;

	//serv vars
	struct sockaddr_in s_server, s_klient;
	short s_port;
	char * s_input_port;
	int s_deskryptor_klienta;	
	socklen_t rozmiar;
	int pom;

	//thread handlers
	pthread_t client_loop_id;
	pthread_t input_loop_id;
	
	printf("Czy jestes serwerem matka? 'nie' jesli nie, 'tak'/inne jesli tak\n");
	serwer_matka = getline_new();

	if(strcmp(serwer_matka, "nie\n") == 0){
		
		char *c_input_port;

		printf("Podaj ip do ktorego chcesz sie podlaczyc:\n");
		c_server_ip = getline_new();

		printf("Podaj nr portu do ktorego chcesz sie podlaczyc:\n");	
		c_input_port = getline_new();	
		c_service_port = atoi(c_input_port);
	
		memset (&c_server, 0, sizeof c_server);
		c_server.sin_family = AF_INET;
		inet_aton (c_server_ip, &c_server.sin_addr);
		c_server.sin_port = htons(c_service_port);

		if ((c_deskryptor_gniazda = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
			perror ("Nie można utworzyć gniazdka");
			exit (EXIT_FAILURE);
		}

		if (connect(c_deskryptor_gniazda, (struct sockaddr*) &c_server, sizeof c_server) < 0) {
			perror ("Brak połączenia");
			exit (EXIT_FAILURE);
		}
	}

	printf("Podaj port dla swojego serwera:\n");
	s_input_port = getline_new();	
	s_port = atoi(s_input_port);

	memset(&s_server, 0, sizeof(struct sockaddr));
	s_server.sin_family = AF_INET;
	s_server.sin_addr.s_addr = htonl(INADDR_ANY);
	s_server.sin_port = htons(s_port);

	if((s_deskryptor_gniazda = socket(AF_INET,SOCK_STREAM,0)) == -1){
		perror("Wystapil blad przy socket");
		return 0;
	}
	
	pom = 1;
	setsockopt(s_deskryptor_gniazda, SOL_SOCKET, SO_REUSEADDR, (char*)&pom, sizeof(pom));
	
	if((bind(s_deskryptor_gniazda, (struct sockaddr*)&s_server, sizeof(struct sockaddr))) == -1){
		perror("Wystapil blad przy bind");
		return 0;
	}
	
	if(listen(s_deskryptor_gniazda, QUEUE_SIZE) == -1){
		perror("Wystapil blad przy listen");
		return 0;
	}

	if(strcmp(serwer_matka, "nie\n")==0){
		pthread_create(&client_loop_id, NULL, client_loop, NULL);
	} else {
		pthread_create(&input_loop_id, NULL, input_loop, NULL);
	}

	while(end == 0){
		rozmiar = sizeof(struct sockaddr);
		if((s_deskryptor_klienta = accept(s_deskryptor_gniazda, (struct sockaddr *)&s_klient, &rozmiar)) == -1){
			perror("Wystapil blad przy accept");
			return 0;
		}
				
		printf("Nawiazano polaczenie z klientem\n");
		
		if(last_client_id <50){
			lista_klientow[last_client_id] = s_deskryptor_klienta;
			last_client_id +=1;		
		}
	}

	close(s_deskryptor_gniazda);
	
	if(strcmp(serwer_matka, "nie\n")==0){
		if(pthread_join(input_loop_id, NULL)) {
			perror("Error joining thread");
			return 0;
		}
	} else {
		if(pthread_join(client_loop_id, NULL)) {
			perror("Error joining thread");
			return 0;

		}
	}

	return 0;
}






























