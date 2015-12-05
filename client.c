#include <sys/socket.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>
#include <string.h> 
#include <unistd.h>

#define TAILLEMAXMSG 5000
 
 

int sock, sockfd;
int rv;
char cmd[TAILLEMAXMSG];
 
fd_set set,setbis;
 
int main(int argc,char *argv[]) {

int taille_msg;
	int port=atoi(argv[2]);
 	struct hostent *hote;
  	struct	sockaddr_in serveur;
  
  //Create socket, tcp usage
  sock = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
  if(sock ==-1)
    {
      printf("Erreur lors de la creation de la socket");
      return -1;
    }
    
    hote=gethostbyname(argv[1]);
	// nom de serveur introuvable
	if(hote==NULL){
		printf("Le serveur distant est introuvable.\n");
		return -1;
	}
  
	// Mise a jour de la structure serveur utilisee pour la connection
	serveur.sin_family = AF_INET;
	memcpy (&(serveur.sin_addr) , hote->h_addr , hote->h_length );
	serveur.sin_port = port;

	//Connexion au serveur
	if (connect(sock, (struct sockaddr*) &serveur, sizeof(serveur)) == -1)
	{
	  printf("Echec de la connexion\n");
	  return -1;
	}
	
	
	FD_ZERO(&set); // initialisation de la liste
	int maxsock = getdtablesize(); // plus grand numéro de descripteur a observer
	FD_SET(0, &set); // ajout de l'entree standard
	FD_SET(sock, &set); // ajout de la socket
	
  
  	while(strcmp(cmd,"quit")!=0)
    {
		bcopy ( (char*) &set, (char*) &setbis, sizeof(set)) ;
		
		// Ecoute sur l'entree standard et la socket
		select(maxsock, &setbis, 0, 0, 0);

		// Cas ou l'utilisateur entre une commande
		if(FD_ISSET(0, &setbis)){
		
		  gets(cmd);
		  write(sock,cmd,strlen(cmd));
      	}
      	if(FD_ISSET(sock, &setbis)){
		
		  /*//recoit un message serveur
		  	taille_msg=read(sock,cmd,TAILLEMAXMSG);
			cmd[taille_msg]='\0';
			printf("%s",cmd);*/
      	}
    }
  

      return 0;
    }

