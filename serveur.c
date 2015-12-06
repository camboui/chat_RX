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
#define MAX_PSEUDOS 30

/** Définition des variables globales pour la communication avec le client. */

typedef struct{

	int socket;
	char pseudo[MAX_PSEUDOS];
	int contact_with[MAX_CLIENTS];

}un_client;

void ajouter_client(int newsock);
void traiter_requete_client(int num_client,char *message);
void clients_connectes(char* liste);
int pseudo_existe(char* dest);
int client_libre();
void init_clients();


un_client les_clients[MAX_CLIENTS];
int socketInit;
fd_set set,setbis;

int main(int argc,char *argv[]) {

	int socktemp;
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
    
    int lg_client;
    FD_ZERO(&set); // initialisation de la liste
    int maxsock = getdtablesize(); 
	FD_SET(socketInit,&set); // ajout du socket du serveur dans la liste
	
	init_clients();	
	printf("\nServeur prêt : attente de clients...\n");	
	
    while (1)
    {
		bcopy ( (char*) &set, (char*) &setbis, sizeof(set)) ; // On réinitialise set à chaque fois
		
		select(maxsock, &setbis, 0, 0, 0);
		//Socket du serveur pret a etre lu: connexion d'un nouveau client
		if (FD_ISSET(socketInit, &setbis))
		{
		    socktemp = accept(socketInit,(struct sockaddr *)&client, &lg_client); 
			if (socktemp == -1) 
			{
				printf("Erreur lors de la connexion d'un client\n");
				return -1;
			}
			else
			{
				ajouter_client(socktemp); // s'il n'y a pas d'erreur on ajoute le client
			}
		}
		for(int i=0;i<MAX_CLIENTS;i++)
		{
			//On vérifie si les clients ont une requête
			if(les_clients[i].socket!=-1 && FD_ISSET( les_clients[i].socket, &setbis))
			{
				message_length = read(les_clients[i].socket,message, MAX_MESSAGES);
				message[message_length]='\0';
				traiter_requete_client(i,message);
			}
		}
 
    }
}


void ajouter_client(int newsock) // On initialise la structure client
{
	char temp[MAX_MESSAGES]="";
	char temp2[MAX_MESSAGES]="";
	char tempPseudo[MAX_PSEUDOS]="";
	int num_libre = client_libre();

	if(num_libre==-1)
	{
		// On notifie au client que le serveur est complet
		printf("Connexion impossible : Serveur complet \n");
		strcpy(temp,"#SERVEUR > Connexion impossible : Serveur complet\n");
		write(newsock,temp,strlen(temp));
		close(newsock);
	}
	else
	{
		les_clients[num_libre].socket=newsock; // Assigne la socket au client
		read(les_clients[num_libre].socket,tempPseudo, MAX_PSEUDOS); // récupère le pseudo du client
		
		if(pseudo_existe(tempPseudo)!=-1) // le pseudo existe
		{
			//On notifie au client que le pseudo existe
			printf("Un client tente d'utiliser un pseudo existant... KICKED\n");
			strcpy(temp,"#SERVEUR > Connexion impossible : Pseudo déjà utilisé\n");
			write(les_clients[num_libre].socket,temp,strlen(temp));
			close(les_clients[num_libre].socket);
		
			//On reset les valeurs nécessaires
			les_clients[num_libre].socket=-1;
			strcpy(les_clients[num_libre].pseudo,"");
		
		}
		else
		{
			strcpy(les_clients[num_libre].pseudo,tempPseudo);
			FD_SET(les_clients[num_libre].socket ,&set);	// Actualise les descripteurs de fichier

			strcpy(temp,"#SERVEUR > Connexion établie\n"); //on envoie un message de confirmation
			clients_connectes(temp2);
			strcat(temp,temp2);
			write(les_clients[num_libre].socket,temp,strlen(temp));

	
			// Le client n'est en contact avec personne au début
			for(int i=0;i<MAX_CLIENTS;i++)
			{
				les_clients[num_libre].contact_with[i]=0;
			}
	
			printf("%s est connecté au serveur\n",les_clients[num_libre].pseudo);

		}
	}
}


void traiter_requete_client(int num_client,char *message)
{
	char reponse[MAX_MESSAGES]="";

	if(strcmp(message,"quit")==0)
	{
		printf("Le client %s a quitté le serveur\n",les_clients[num_client].pseudo);
		sprintf(reponse, "#SERVEUR > Le client %s s'est déconnecté.\n", les_clients[num_client].pseudo);
		
		//On notifie à tous les clients que leur correspondant s'est deconnecté et on réinitialise les infos
		for(int i=0;i<MAX_CLIENTS;i++) 
		{
			if(i!=num_client &&  les_clients[i].socket!=-1 && (les_clients[num_client].contact_with[i]==1 || les_clients[i].contact_with[num_client]==1))
			{
			
				les_clients[num_client].contact_with[i]=0;
				les_clients[i].contact_with[num_client]=0;
				write(les_clients[i].socket,reponse,strlen(reponse));
			}
			
		}
		close(les_clients[num_client].socket);
		// réinitialise les infos sur le client qui est parti
		FD_CLR(les_clients[num_client].socket, &set);
		les_clients[num_client].socket=-1;
		strcpy(les_clients[num_client].pseudo,"");

	}
	else if(strcmp(message,"list")==0) // On affiche la liste si elle est demandée
	{
		printf("Client %s : demande de liste\n",les_clients[num_client].pseudo);
		clients_connectes(reponse);
		write(les_clients[num_client].socket,reponse,strlen(reponse));
	}
	else // Si ce n'est pas une commande 
	{
		char dest[MAX_PSEUDOS]="";
		int i=0;
		int num_dest;
		//On récupère le pseudo du destinataire
		while(message[i]!='\0' && i<MAX_PSEUDOS-1 && message[i]!=' ')	
		{
			dest[i]=message[i];
			i++;
		}
		dest[i+1]='\0';
		
		num_dest=pseudo_existe(dest); // recupère le numéro du destinataire à partir du pseudo
		if(num_dest==-1) // Si le pseudo n'existe pas on le notifie au client
		{
			printf("Client %s ---> ... ---> %s NON EXISTANT\n",les_clients[num_client].pseudo,dest);
			sprintf(reponse, "#SERVEUR > Le client/Commande %s n'existe pas.\n", dest);
			write(les_clients[num_client].socket,reponse,strlen(reponse));
		}
		else //sinon
		{
			//On actualise les tables de relation client
			les_clients[num_client].contact_with[num_dest]=1;
			les_clients[num_dest].contact_with[num_client]=1;
			
			// On récupère la partie message
			int k=0;
			char private_message[MAX_MESSAGES]=""; 
			while(message[i]!='\0' && k<MAX_MESSAGES)	
			{
				private_message[k]=message[i];
				i++;
				k++;
			}
			
			//On envoie l'ensemble des informations récupérées au destinataire
			sprintf(reponse, "Message de %s > %s \n", les_clients[num_client].pseudo,private_message);
			write(les_clients[num_dest].socket,reponse,strlen(reponse));
			printf("\nClient %s ---> \"%s\" ---> %s \n",les_clients[num_client].pseudo,private_message,dest);
		}	
	}

}

int pseudo_existe(char* dest)
{
	for(int i=0;i<MAX_CLIENTS;i++) // On parcours tous les clients
	{
		if(les_clients[i].socket!=-1 && strcmp(dest,les_clients[i].pseudo)==0)
		{
			return i;
		}
	}
	return -1;
}

int client_libre()
{
	for(int i=0;i<MAX_CLIENTS;i++)  // parcours de tous les clients
	{
		if(les_clients[i].socket==-1) // si le client n'a pas de socket positive, il est libre
		{
			return i;
		}
	}
	return -1;
}

void init_clients()
{
	for(int i=0;i<MAX_CLIENTS;i++)  // parcours de tous les clients et les initialise
	{
		strcpy(les_clients[i].pseudo,""); 
		les_clients[i].socket=-1;
	}
}



void clients_connectes(char *liste)
{
	int cpt=0;
	for(int i=0;i<MAX_CLIENTS;i++) // parcours de tous les clients
	{
		if(les_clients[i].socket!=-1) // Si le client est existe dans le tableau
		{
		cpt++;
		}
	}
	
	sprintf(liste,"#SERVEUR > %i utilisateur(s) connecté(s) : ",cpt);
	for(int i=0;i<MAX_CLIENTS;i++) // parcours de tous les clients
	{
		if(les_clients[i].socket!=-1) // Si le client est existe dans le tableau
		{
			strcat(liste,les_clients[i].pseudo);
			if(i!=MAX_CLIENTS-1 )
				strcat(liste," / ");
		}
	}
	strcat(liste,"\n");
}





