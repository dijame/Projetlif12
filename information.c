#include "information.h"
#include <pthread.h>

void http_get(const char* serveur, const char* port, const char* chemin, const char* nom_fichier)
{
    //Déclaration des threads
    pthread_t pt_analyse;
    pthread_t pt_telecharger;

    //Déclaration des structures pour traiter les pages;
    PageAnalyze analyse;
    PageDownload download;

    //Création des threads
    /*if(pthread_create(&pt_analyse,NULL,analyse_page,) != 0)
        perror("Erreur création du thread analyse");
      if(pthread_create(&pt_download,NULL,download_page,) != 0)
        perror("Erreur création du thread download");
    */

    // Créer un fichier s'il n'existe pas et donne les droits de lecture, écriture
	// et exécution au propriétare du fichier
    int outfd = open(nom_fichier,O_WRONLY | O_CREAT,S_IRWXU);
    if(outfd == - 1) perror("Erreur à la création du fichier !"); // Si la création du fichier échoue
    else {

        // On ouvre la socket et on l'a créé
        int sockfd = CreeSocketClient(serveur, port);

        // Test de la socket
        if(sockfd == -1) perror("Erreur sur la socket");

        // Envoie de la requète au serveur
        EnvoieMessage(sockfd,"GET /%s HTTP/1.1\nHost: %s\n\n",chemin,serveur);

        //Regarde le code HTTP, ex : 200,404,...
        char* header = RecoieLigne(sockfd);
        char* tmp = malloc(sizeof(char)*(strlen(header)+4));

        strcpy(tmp,header); // On sauvegarde la première ligne de l'en-tête
        header = strtok(header," "); // On découpe la chaine en délimitant pas des espaces
        header = strtok(NULL," "); // On récupère la deuxième partie soit le code
        free(tmp); // On libère tmp

        // On cast le code en int
        int code = atoi(header);
        // On réchupère le message erreur
        char* messerr = strtok(NULL,"");

        // On gère les erreurs ou réussites, ici on ne tiens pas compte précisement de tous les cas possibles
        if(code >= 200 && code < 400)
            printf("\n%d , %s\n",code,messerr);
        else if(code >= 400 && code < 600)
            printf("\nErreur %d , %s\n",code,messerr);

        // En cas de redirection on recupère l'url
        if(code == 302) {
            while(strncmp(tmp,"Location:",9) != 0) {
                tmp = RecoieLigne(sockfd);
            }
            // On récupère l'adresse sans le http://
            if(strncmp(tmp,"http://",7) == 0){
                char* url = tmp + 17;
                // On récupère le serveur
                tmp = strtok(url,"/");
                char* new_serveur = tmp;
                // On s'occupe du chemin
                tmp = strtok(NULL,"");
                char* new_chemin  = tmp;

                // On relance la fonction de base
                http_get(new_serveur,port,new_chemin,nom_fichier);

            } else {
                // On récupère le chemin sans la page (ex:"test.php" on le supprime)
                char* lastslash  = strrchr(chemin,'/');
                if(lastslash != NULL) *lastslash = '\0';
                printf("\%s\n",chemin);
                tmp = tmp + 11; // On enlève le "./"
                char* new_chemin  = malloc(sizeof(char)*(strlen(tmp)+strlen(chemin)));
                strcpy(new_chemin,chemin);
                strcat(new_chemin,tmp);

                printf("\%s\n",new_chemin);

                // On relance la fonction de base
                http_get(serveur,port,new_chemin,nom_fichier);
                free(new_chemin);
            }

            close(sockfd);
            close(outfd);
            // Enfin on sort de la fontion
            exit(0);
        }


        // RecoieLigne enlève les caractère spéciaux , c'est pourquoi on va attendre une ligne vide
        // Celle qui sépare l'en-tête du reste du corps
        while( strcmp(tmp,"") != 0) {
            if(write(outfd,tmp,strlen(tmp)) != strlen(tmp))
                perror("\nErreur lors de l'écriture de l'en-tête\n");
            tmp = RecoieLigne(sockfd);
        }

        if(write(outfd,tmp,strlen(tmp)) != strlen(tmp))
                perror("\nErreur lors de l'écriture de l'en-tête\n");

        // Réception du code source de la page
        RecoieEtSauveDonnees(outfd,sockfd);

        // On ferme la socket
        close(sockfd);
    }
    close(outfd);
}
