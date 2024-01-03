 /* servTcpConc.c - Exemplu de server TCP concurent
    Asteapta un nume de la clienti; intoarce clientului sirul
    "Hello nume".
    */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* portul folosit */
#define PORT 2024
#define READ 0
#define WRITE 1

/* codul de eroare returnat de anumite apeluri */
extern int errno;

int main ()
{
    struct sockaddr_in server;	// structura folosita de server
    struct sockaddr_in from;
    char msg[100];		//mesajul primit de la client
    char msgrasp[100]=" ";        //mesaj de raspuns pentru client
    int sd;			//descriptorul de socket
	char *users[] = {"Terry", "Jom"};
	int boolean = 0;
	int clients = 0;
	int pipefd[4][2];
	pid_t pid;
	char condition[4];
	int listener = 0;

    /* crearea unui socket */
    if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    {
    	perror ("[server]Eroare la socket().\n");
    	return errno;
    }

	for (int i = 0; i < 4; i++) {
        if (pipe(pipefd[i]) == -1) {
            perror("Pipe creation failed");
            exit(EXIT_FAILURE);
        }
    }

    /* pregatirea structurilor de date */
    bzero (&server, sizeof (server));
    bzero (&from, sizeof (from));

    /* umplem structura folosita de server */
    /* stabilirea familiei de socket-uri */
    server.sin_family = AF_INET;
    /* acceptam orice adresa */
    server.sin_addr.s_addr = htonl (INADDR_ANY);
    /* utilizam un port utilizator */
    server.sin_port = htons (PORT);

    /* atasam socketul */
    if (bind (sd, (struct sockaddr *) &server, sizeof (struct sockaddr)) == -1)
    {
    	perror ("[server]Eroare la bind().\n");
    	return errno;
    }

    /* punem serverul sa asculte daca vin clienti sa se conecteze */
    if (listen (sd, 1) == -1)
    {
    	perror ("[server]Eroare la listen().\n");
    	return errno;
    }

    /* servim in mod concurent clientii... */
    while (1)
    {
    	int client;
    	int length = sizeof (from);

		printf("S-au conectat pana acum %d clienti\n", clients);

    	printf ("[server]Asteptam la portul %d...\n",PORT);
    	fflush (stdout);

    	/* acceptam un client (stare blocanta pina la realizarea conexiunii) */
    	client = accept (sd, (struct sockaddr *) &from, &length);

    	/* eroare la acceptarea conexiunii de la un client */
    	if (client < 0)
    	{
    		perror ("[server]Eroare la accept().\n");
    		continue;
    	}

    	int pid;
    	if ((pid = fork()) == -1) {
    		close(client);
    		continue;
    	} else if (pid > 0) {
			clients++;
			listener++;
    		// parinte
    		close(client);
			if (clients >= 4){
				for(int i = 0; i<4; i++){
					close(pipefd[i][READ]);

					write(pipefd[i][WRITE], "Stop", 4);

        			close(pipefd[i][WRITE]);
				}
			}
    		while(waitpid(-1,NULL,WNOHANG));
    		continue;
    	} else if (pid == 0) {
    		// copil
    		close(sd);

    		/* s-a realizat conexiunea, se astepta mesajul */
    		bzero (msg, 100);
    		printf ("[server]Asteptam mesajul...\n");
    		fflush (stdout);

    		/* citirea mesajului */
    		if (read (client, msg, 100) <= 0)
    		{
    			perror ("[server]Eroare la read() de la client.\n");
    			close (client);	/* inchidem conexiunea cu clientul */
    			continue;		/* continuam sa ascultam */
    		}

			msg[5] = '\0';
    		printf ("[server]Mesajul a fost receptionat...%s\n", msg);

    		for (int i = 0; i < 2; i++) {

        		if (strcmp(users[i], msg) == 0) {
            		boolean = 1;
					break;
        		}

				msg[3] = '\0';
    		}

			if (boolean == 1)
			{
				close(pipefd[listener][WRITE]);
				/*pregatim mesajul de raspuns */
				bzero(msgrasp,100);
				strcat(msgrasp,"Hello ");
				strcat(msgrasp,msg);

				printf("[server]Trimitem mesajul inapoi...%s\n",msgrasp);


				/* returnam mesajul clientului */
				if (write (client, msgrasp, 100) <= 0)
				{
					perror ("[server]Eroare la write() catre client.\n");
					continue;		/* continuam sa ascultam */
				}
				else
					printf ("[server]Mesajul a fost trasmis cu succes.\n");
				
				while(strcmp("Stop", condition) != 0){
					read(pipefd[listener][READ], condition, sizeof(condition));
					condition[4] = '\0';
					sleep(10);
					printf("Acesta este conditia: %s", condition);
					printf("Sunt in while.\n");
				}
				/* am terminat cu acest client, inchidem conexiunea */
				
				if (strcmp("Stop", condition) == 0){
					printf ("S-a atins numarul maxim de clienti.\n");
					
					if (write (client, "Stop", 5) <= 0)
					{
						perror ("[server]Eroare la write() catre client.\n");
						continue;		/* continuam sa ascultam */
					}
					else
						printf ("[server]Conditie trimisa.\n");


					close (client);
					exit(0);
				}
			}
			else{

				bzero(msgrasp,100);
				strcat(msgrasp,"Login Denied");

				printf("[server]Trimitem mesajul inapoi...%s\n",msgrasp);


				/* returnam mesajul clientului */
				if (write (client, msgrasp, 100) <= 0)
				{
					perror ("[server]Eroare la write() catre client.\n");
					continue;		/* continuam sa ascultam */
				}
				else
					printf ("[server]Mesajul a fost trasmis cu succes.\n");
				
				/* am terminat cu acest client, inchidem conexiunea */
				close (client);
				exit(0);
			}
    	}

    }				/* while */
}				/* main */