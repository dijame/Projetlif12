#include "information.h"
#include <pthread.h>

// Variables globales que se partagent les threads
int socket; // Socket qui permettra la connexion entre l'application et le serveur
pthread_cond_t t_cond = PTHREAD_COND_INITIALIZER; // Création de la condition
pthread_mutex_t t_mutex = PTHREAD_MUTEX_INITIALIZER; // Création du mutex

// Variable convercant le cas chunked
bool chunked = false; // Indiquera si la page est chunked ou pas
int  chk_bytes;

// Déclaration des structures et des tableaux pour traiter les pages;
FilePage f;
char *analyseur[TAILLE_TAB];
char *downloadeur[TAILLE_TAB];

bool finis = true; // Une variable qui indique si la page a terminé de se téléchargé
char url[255];

void http_get(const char* serveur, const char* port, const char* chemin, const char* nom_fichier)
{
    // Déclaration des threads
    pthread_t pt_analyse;
    pthread_t pt_telecharger;

    // On garde l'url en mémoire
    strcpy(url,serveur);
    strcat(url,"/");
    strcat(url,chemin);

    // On établit la connexion et on récupère l'en-tête du site \\
    // On ouvre la socket et on l'a créé et on l'a test
    if(socket = CreeSocketClient(serveur, port) == -1)
        perror("Erreur sur la socket");

    // Envoie de la requète au serveur
    EnvoieMessage(socket,"GET /%s HTTP/1.1\nHost: %s\n\n",chemin,serveur);

    //Regarde le code HTTP, ex : 200,404,...
    char* header = RecoieLigne(socket);
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
    if(code == 302)
    {
        while(strncmp(tmp,"Location:",9) != 0)
        {
            tmp = RecoieLigne(socket);
        }
        // On récupère l'adresse sans le http://
        if(strncmp(tmp,"http://",7) == 0)
        {
            char* url = tmp + 17;
            // On récupère le serveur
            tmp = strtok(url,"/");
            char* new_serveur = tmp;
            // On s'occupe du chemin
            tmp = strtok(NULL,"");
            char* new_chemin  = tmp;

            // On relance la fonction de base
            http_get(new_serveur,port,new_chemin,nom_fichier);

        }
        else
        {
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

        close(socket);

        // Enfin on sort de la fontion
        exit(0);
    }

    // RecoieLigne enlève les caractère spéciaux , c'est pourquoi on va attendre une ligne vide
    // Celle qui sépare l'en-tête du reste du corps
    while( strcmp(tmp,"") != 0)
    {
        if(strcmp(tmp,"Transfer-Encoding: chunked"))
            chunked = true;
        tmp = RecoieLigne(socket);
    }
    if(chunked)
    {
        tmp = RecoieLigne(socket);
        chk_bytes = strtol(tmp,NULL,16); // on parse l'hexa en octets
    }
    // Fin du traitement de l'en-tête \\

    //Création des threads
    if(pthread_create(&pt_analyse,NULL,analyse_page,NULL) != 0)
    {
        perror("Erreur création du thread analyse");
        exit(EXIT_FAILURE);
    }
    if(pthread_create(&pt_download,NULL,download_page,NULL) != 0)
    {
        perror("Erreur création du thread download");
        exit(EXIT_FAILURE);
    }

    if(pthread_join(pt_analyse,NULL) != 0)
    {
        perror("join analyse fail");
        exit(EXIT_FAILURE);
    }
    if(pthread_join(pt_download,NULL) != 0)
    {
        perror("join download fail");
        exit(EXIT_FAILURE);
    }




    // On ferme la socket
    close(socket);

}

void analyse_page()
{

    int i = 0; // Indice du tableau pour savoir où on en est
    int nb_bytes = 0; // Le nombre d'octets reçus pour le chunked
    char *ligne; // Variable qui récupérera la ligne courante
    char *cp_ligne; // Copie de ligne afin de ne pas modifer la ligne d'origine
    strcpy(url_fichier,url); // On copie l'url de base

    while(1)  // Boucle infini
    {
        // On vérouille le mutex pour que ce thread soit prioritaire sur la ressource
        pthread_mutex_lock(&t_mutex);

        // Traitement de la page \\
        // Gestion des chemins de fichiers \\

        ligne = RecoieLigne(socket);

        // On fait une copie de la ligne pour ne pas l'altérer
        cp_ligne = malloc(sizeof(char)*strlen(ligne));
        strcpy(cp_ligne,ligne);

        // Image
        if((chemin = strstr("<img",cp_ligne) != NULL)
    {
        if((chemin = strstr("src=",chemin) != NULL)
        {
            if(lastguimet = strrchr(chemin,'"') != NULL)
                {
                    lastguimet = '\0'; // On supprime tous ce qui est après la guillemet
                    chemin = chemin + 5; // On supprime le src="
                    // On alloue l'espace pour garder les chemins en mémoire et on copie
                    f.repertoire[i] = malloc(sizeof(char)*strlen(chemin));
                    strcpy(f.repertoire[i],chemin);
                    strcat(url_fichier,chemin);
                    f.url[i] = malloc(sizeof(char)*strlen(url_fichier));
                    strcpy(f.url[i],url_fichier);
                    f.t_analyze[i] = false;
                    f.t_download[i] = true;

                    // On s'occupe maintenant de l'analyseur
                    analyseur[i] = malloc(sizeof(char)*strlen(chemin));
                    strcpy(analyseur[i],chemin);

                    // On réinitialise les variables
                    strcpy(cp_ligne,ligne); // On rcopie la ligne de base
                    strcpy(url_fichier,url);// On remet l'url de base
                }
            }
        }
        // Script JS
        if((chemin = strstr("<script",cp_ligne) != NULL)
    {
        if((chemin = strstr("src=",chemin) != NULL)
        {
            if(lastguimet = strrchr(chemin,'"') != NULL)
                {
                    lastguimet = '\0'; // On suppirme tous ce qui est après la guillemet
                    chemin = chemin + 5; // On supprime le src="
                    // On alloue l'espace pour garder les chemins en mémoire et on copie
                    f.repertoire[i] = malloc(sizeof(char)*strlen(chemin));
                    strcpy(f.repertoire[i],chemin);
                    strcat(url_fichier,chemin);
                    f.url[i] = malloc(sizeof(char)*strlen(url_fichier));
                    strcpy(f.url[i],url_fichier);
                    f.t_analyze[i] = false;
                    f.t_download[i] = true;

                    // On s'occupe maintenant de l'analyseur
                    analyseur[i] = malloc(sizeof(char)*strlen(chemin));
                    strcpy(analyseur[i],chemin);

                    // On réinitialise les variables
                    strcpy(cp_ligne,ligne); // On rcopie la ligne de base
                    strcpy(url_fichier,url);// On remet l'url de base
                }
            }
        }
        // Lien CSS
        if((chemin = strstr("<link",cp_ligne) != NULL)
    {

    }
    /*Si le fichier est une page du site alors on le thread de téléchargement doit
      nous renvoyé la page à analyser
    */

    // Si on est dans le cas chunked
    if(chunked)
    {
        // Quand on arrive à la taille indiqué par le chunked on stock la prochaine taille
        if(nb_bytes == chk_bytes)
            {
                ligne = RecoieLigne(socket);
                chk_bytes = strtol(ligne,NULL,16); // On parse la taille en hexa en octets
                nb_bytes = 0;
            }
        }

        i++;

        if(i<TAILLE_TAB || (strcmp(ligne,"0") == 0))
    {
        // Quand la page a finis de se télécharger on lui indique par finis = false
        if(strcmp(ligne,"0") == 0) finis = false;
            // Lorsque que la condition est vérifié on envoie le signal
            pthread_cond_signal(&t_cond);
            // Enfin on laisse l'accès à la reschemin
            pthread_mutex_unlock(&t_mutex);
        }
    }

    pthread_exit(NULL);
}

void download_page()
{
    //Téléchargement des ressources
    while(1)  // Boucle infini
    {

        /* Récupérer le fichier,son extension, etc
           nom_fichier = strrchr(f.repertoire[i],'/');
        */

    }
    pthread_exit(NULL);
}

void rempliTableaux(char *type)
{
    char *chemin; // Chemin de fichier
    char url_fichier[255]; // Variable permettant de mettre l'url du fichier dans le tableau
    char *lastguimet; // Afin de supprimer le dernier guillemet des chemins

    if((chemin = strstr("href=",chemin) != NULL)
{
    if(lastguimet = strrchr(chemin,'"') != NULL)
        {
            lastguimet = '\0'; // On suppirme tous ce qui est après la guillemet
            chemin = chemin + 5; // On supprime le src="
            // On alloue l'espace pour garder les chemins en mémoire et on copie
            f.repertoire[i] = malloc(sizeof(char)*strlen(chemin));
            strcpy(f.repertoire[i],chemin);
            strcat(url_fichier,chemin);
            f.url[i] = malloc(sizeof(char)*strlen(url_fichier));
            strcpy(f.url[i],url_fichier);
            f.t_analyze[i] = false;
            f.t_download[i] = true;

            // On s'occupe maintenant de l'analyseur
            analyseur[i] = malloc(sizeof(char)*strlen(chemin));
            strcpy(analyseur[i],chemin);

            // On réinitialise les variables
            strcpy(cp_ligne,ligne); // On rcopie la ligne de base
            strcpy(url_fichier,url);// On remet l'url de base
        }
    }

}


