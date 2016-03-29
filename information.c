#include "information.h"
#include <pthread.h>

// Variables globales que se partagent les threads

int sockfd; // Socket qui permettra la connexion entre l'application et le serveur

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
char *url; // Copie du serveur

// TODO: N'oublis pas de gérer le chunked
void http_get(const char* serveur, const char* port, const char* chemin, const char* nom_fichier, const int nb_th_a, const int nb_th_d)
{
    // Déclaration des threads
    pthread_t pt_analyse[nb_th_a];
    pthread_t pt_download[nb_th_d];


    // On garde le serveur en mémoire
    strcpy(url,serveur);

    // On établit la connexion et on récupère l'en-tête du site \\
    // On ouvre la sockfd et on l'a créé et on l'a test
    if(sockfd = CreesockfdClient(serveur, port) == -1)
        perror("Erreur sur la sockfd");

    // Envoie de la requète au serveur
    EnvoieMessage(sockfd,"GET /%s HTTP/1.1\r\nHost: %s\r\n\r\n",chemin,serveur);

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
    else if(code >= 400 && code < 600){
        printf("\nErreur %d , %s\n",code,messerr);
        exit(EXIT_FAILURE);
    }

    // En cas de redirection on recupère l'url
    if(code == 302)
    {
        while(strncmp(tmp,"Location:",9) != 0)
        {
            tmp = RecoieLigne(sockfd);
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

        } else {
            // On récupère le chemin sans la page (ex:"./test.php" on le supprime)
            char* lastslash  = strrchr(chemin,'/');
            if(lastslash != NULL) *lastslash = '\0';
            printf("\%s\n",chemin);
            tmp = tmp + 11; // On enlève le "./"
            char* new_chemin  = malloc(sizeof(char)*(strlen(tmp)+strlen(chemin)));Utilise pthread create :)
            strcpy(new_chemin,chemin);
            strcat(new_chemin,tmp);

            printf("\%s\n",new_chemin);

            // On relance la fonction de base
            http_get(serveur,port,new_chemin,nom_fichier);
            free(new_chemin);
        }

        close(sockfd);

        // Enfin on sort de la fontion
        exit(0);
    }

    // RecoieLigne enlève les caractère spéciaux , c'est pourquoi on va attendre une ligne vide
    // Celle qui sépare l'en-tête du reste du corps
    while( strcmp(tmp,"") != 0)
    {
        if(strcmp(tmp,"Transfer-Encoding: chunked"))
            chunked = true;
        tmp = RecoieLigne(sockfd);
    }
    if(chunked)
    {
        tmp = RecoieLigne(sockfd);
        chk_bytes = strtol(tmp,NULL,16); // on parse l'hexa en octets
    }
    // Fin du traitement de l'en-tête \\

    //Création des threads
    for(int j = 0; j<nb_th_a;j++){
        if(pthread_create(&pt_analyse[j],NULL,analyse_page,NULL) != 0)
        {
            perror("Erreur création du thread analyse");
            exit(EXIT_FAILURE);
        }
    }

    for(int j = 0; j<nb_th_a;j++){
        if(pthread_join(pt_analyse[j],NULL) != 0){
            perror("join analyse fail");
            exit(EXIT_FAILURE);
        }
    }
    for(int j = 0; j<nb_th_d;j++){
        if(pthread_create(&pt_download[j],NULL,download_page,NULL) != 0){
            perror("Erreur création du thread download");
            exit(EXIT_FAILURE);
        }
    }
    for(int j = 0; j<nb_th_d;j++){
        if(pthread_join(pt_download[j],NULL) != 0){
            perror("join download fail");
            exit(EXIT_FAILURE);
        }
    }


    // On ferme la sockfd
    close(sockfd);

}

void *analyse_page(void *arg)
{
    (void)arg; //Pour enlever warning
    int sockfd; // La socket du thread
    int i = 0; // Indice du tableau pour savoir où on en est
    int nb_bytes = 0; // Le nombre d'octets reçus pour le chunked
    char *chemin; // Le chemin du fichier contenu dans le tableau
    char *ligne; // Variable qui récupérera la ligne courante
    char *cp_ligne; // Copie de ligne afin de ne pas modifer la ligne d'origine

    // On établit la connexion et on récupère l'en-tête du site \\
    // On ouvre la sockfd et on l'a créé et on l'a test
    if(sockfd = CreesockfdClient(serveur, port) == -1)
        perror("Erreur sur la sockfd");

    while(1)  // Boucle infini
    {
        // Envoie de la requète au serveur
        EnvoieMessage(sockfd,"GET /%s HTTP/1.1\r\nHost: %s\r\n\r\n",chemin,url);

        // Traitement de la page \\
        // Gestion des chemins de fichiers \\

        ligne = RecoieLigne(sockfd);

        // On fait une copie de la ligne pour ne pas l'altérer par la suite
        cp_ligne = malloc(sizeof(char)*strlen(ligne));
        strcpy(cp_ligne,ligne);

        // Image
        if(cp_ligne = strstr("<img",cp_ligne) != NULL)
            rempliTableaux("src=",cp_ligne);
        // Script JS
        if(cp_ligne = strstr("<script",cp_ligne) != NULL)
            rempliTableaux("src=",cp_ligne);
        // Lien CSS
        if(cp_ligne = strstr("<link",cp_ligne) != NULL)
            rempliTableaux("href=",cp_ligne);
        // Lien <a>
        if((cp_ligne = strstr("<a",cp_ligne) != NULL)
            rempliTableaux("href=",cp_ligne);


    // Si on est dans le cas chunked
    if(chunked){
        // Quand on arrive à la taille indiqué par le chunked on stock la prochaine taille
        if(nb_bytes == chk_bytes){
            ligne = RecoieLigne(sockfd);
            chk_bytes = strtol(ligne,NULL,16); // On parse la taille en hexa en octets
            nb_bytes = 0;
        }
    }

        i++;

        if(i<TAILLE_TAB || (strcmp(ligne,"0") == 0))
    {
        // Quand la page a finis de se télécharger on lui indique par finis = false
        if(strcmp(ligne,"0") == 0) finis = false;


        }
    }

    pthread_exit(NULL);
}

void *download_page(void *arg)
{
    (void)arg; //Pour enlever warning
    int sockfd; // La socket du thread
    char *chemin; // Le chemin du fichier contenu dans le tableau
    char *ligne; // Variable qui récupérera la ligne courante
    char *nom_fichier; // Le nom du fichier reçu
    int outfd; // Descripteur du nouveau fichier

    // On établit la connexion et on récupère l'en-tête du site \\
    // On ouvre la sockfd et on l'a créé et on l'a test
    if(sockfd = CreesockfdClient(serveur, port) == -1)
        perror("Erreur sur la sockfd");

    while(1){  // Boucle infini
        //Téléchargement des ressources
        //Récupérer le fichier,son extension, etc
        nom_fichier = strrchr(f.repertoire[i],'/');

        // Création du fichier
        outfd = open(nom_fichier,O_WRONLY | O_CREAT,S_IRWXU);
        if(outfd == -1) {
            perror("Erreur lors de la création du fichier");
            exit(EXIT_FAILURE);
        }
        // Envoie de la requète au serveur
        EnvoieMessage(sockfd,"GET /%s HTTP/1.1\r\nHost: %s\r\n\r\n",chemin,url);

        // On va retirer l'en-tête
        // RecoieLigne enlève les caractère spéciaux , c'est pourquoi on va attendre une ligne vide
        // Celle qui sépare l'en-tête du reste du corps
        while( strcmp(tmp,"") != 0) {
            tmp = RecoieLigne(sockfd);
        }

        // Réception du code source du fichier
        while( tmp != NULL) {
            tmp = RecoieLigne(sockfd);
        }

    }
    pthread_exit(NULL);
}

void rempliTableauxAnalyse(char *type,char *cp_ligne)
{
    char *chemin; // Chemin de fichier
    char url_fichier[255]; // Variable permettant de mettre l'url du fichier dans le tableau
    char *lastguimet; // Afin de supprimer le dernier guillemet des chemins

    // On vérouille le mutex pour que ce thread soit prioritaire sur la ressource
    pthread_mutex_lock(&t_mutex);

    if(chemin = strstr(type,cp_ligne) != NULL){
        if(lastguimet = strrchr(chemin,'"') != NULL)
        {
            lastguimet = '\0'; // On suppirme tous ce qui est après la guillemet
            chemin = chemin + strlen(type) + 1; // On supprime le src=",etc
            // On alloue l'espace pour garder les chemins en mémoire et on copie
            f.repertoire[i] = malloc(sizeof(char)*strlen(chemin));
            strcpy(f.repertoire[i],chemin);
            strcpy(url_fichier,url); // On copie l'url de base
            strcat(url_fichier,"/"); // Ajout du slash
            strcat(url_fichier,chemin); // Puis du chemin vers le fichier
            f.url[i] = malloc(sizeof(char)*strlen(url_fichier));
            strcpy(f.url[i],url_fichier);
            f.t_analyze[i] = false;
            f.t_download[i] = true;

            // On s'occupe maintenant de l'analyseur
            analyseur[i] = malloc(sizeof(char)*strlen(chemin));
            strcpy(analyseur[i],chemin);

            // On réinitialise les variables
            strcpy(cp_ligne,ligne); // On recopie la ligne de base
            strcpy(url_fichier,url);// On remet l'url de base
        }
    }

    // Enfin on laisse l'accès à la reschemin
    pthread_mutex_unlock(&t_mutex);

}

char* accesTableauDownload(FilePage f, int i){
    char *donnee;
    // Cas à traiter
    pthread_mutex_lock(&t_mutex);

    donnee = malloc(sizeof(char)*strlen(f.repertoire[i]));
    f.t_download[i] = true;

    pthread_mutex_unlock(&t_mutex);
    return *donnee;

}

char* accesTableauAnalyse(FilePage f, int i){
    char *donnee;
    // Cas à traiter
    pthread_mutex_lock(&t_mutex);

    donnee = malloc(sizeof(char)*strlen(f.repertoire[i]));
    f.t_analyze[i] = true;
    pthread_mutex_unlock(&t_mutex);
    return *donnee;

}


