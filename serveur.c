#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <unistd.h>



/** Définition des paramètres par défaut. */
#define MAX_CLIENTS 3

#define MAX_MESSAGES 5000

/** Définition des variables globales pour la communication avec le client. */
int socketInit, socketClient[3];
fd_set set,setbis;

int main(int argc,char *argv[]) {

	char message[MAX_MESSAGES];
	int message_length;
  
    // Initialisation de la socket
    socketInit = socket (AF_INET,SOCK_STREAM ,IPPROTO_TCP );
    if(socketInit==-1)
	{
		printf("\n ERREUR lors de la création de la socket côté serveur");
		return -1;
	}
        
    struct	sockaddr_in  serveur,client;
    unsigned short port = atoi(argv[1]);
        
    //initialisation de la structure sockaddr_in
    serveur.sin_family = AF_INET;
    serveur.sin_port = port;
    serveur.sin_addr.s_addr = INADDR_ANY;
    
    // liaison de la socket avec l'adresse IP et port du serveur
	if (bind(socketInit, (struct sockaddr*) &serveur, sizeof(serveur)) == -1){
		printf("ERREUR lors du bind côté serveur\n");
		return -1;
	}


	// mise en ecoute du serveur
	if (listen(socketInit, MAX_CLIENTS) == -1){
		printf("ERREUR lors du listen côté serveur\n");
		return -1;
	}
    
    int lg_client = sizeof(client);
    int maxsock = getdtablesize(); 
	FD_SET(socketInit,&set); // ajout du socket du serveur dans la liste
		
		
    while (1)
    {
    
		bcopy ( (char*) &set, (char*) &setbis, sizeof(set)) ;
		
		select(maxsock, &setbis, 0, 0, 0);
		//Socket du serveur pret a etre lu: connexion d'un nouveau client
		if (FD_ISSET(socketInit, &setbis))
		{
		    socketClient[0] = accept(socketInit,(struct sockaddr *)&client, &lg_client);
			if (socketClient[0] == -1)
			{
					printf("Erreur lors de la connexion d'un client\n");
					return -1;
			}
			else
				FD_SET(socketClient[0] ,&set);
		}


		if(FD_ISSET( socketClient[0], &setbis))
		{
			message_length = read(socketClient[0],message, MAX_MESSAGES);
			message[message_length]='\0';
			puts(message);
		}



    

 
    }
    close(socketClient[0]);
}

