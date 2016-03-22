#include "socklib.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char** argv)
{

    if(argc == 3)
    {
        // On déclare les variables
        char url[255];
        char* serveur = NULL;
        char* chemin = "";
        char* port = NULL;
        char* tmp;

        // On récupère l'adresse de la page
        strcpy(url,argv[1]);
        // On vérifie qu'il n'y ai pas "http://" dans l'url
        if(strncmp(url,"http://",7) == 0)
            tmp = url + 7;
        else
            tmp = url;
        // On la découpe
        // On récupère le serveur
        serveur = strtok(tmp,"/");
        // On s'occupe du chemin
        chemin = strtok(NULL,"");
        // Si le chemin n'est pas renseigné
        if(chemin == NULL) chemin = "";
        // On sépare le port du serveur
        serveur = strtok(serveur,":");
        port = strtok(NULL,":");

        if(port == NULL)
        {
            port = "80";
        }

        printf("//Récupération de la page\\\n");

        http_get(serveur,port,chemin,argv[2]);
    }
    else
        perror("Veuillez entrer le bon nombre d'arguments, l'url puis le nom du fichier ! ");

    return 0;
}
