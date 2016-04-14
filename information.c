#include "information.h"
#include <pthread.h>

// Variables globales que se partagent les threads

char g_serveur[255]; // Copie du serveur
char g_port[255]; // Copie du port

pthread_cond_t t_cond = PTHREAD_COND_INITIALIZER; // Création de la condition
pthread_mutex_t t_mutex = PTHREAD_MUTEX_INITIALIZER; // Création du mutex

// Déclaration des structures et des tableaux pour traiter les pages;
FilePage f;
char *analyseur[TAILLE_TAB];
char *downloadeur[TAILLE_TAB];

int indCstruct = 0; // Indice de la structure pour savoir où en est la complétion
int indCana = 0; // Indice du tableau pour savoir où en est la complétion de l'analyseur
int indCdown = 0; // Indice du tableau pour savoir où en est la complétion de dowloadeur

int indRstruct = 0; // Indice du tableau pour savoir où en est la récupération
int indRana = 0; // Indice du tableau pour savoir où en est la récupération de l'analyseur
int indRdown = 0; // Indice du tableau pour savoir où en est la récupération de downloadeur


bool finis = true; // Une variable qui indique si la page a terminé de se téléchargé

// TODO: N'oublis pas de gérer le chunked
void http_get(const char* serveur, const char* port, const char* chemin, const char* nom_fichier, const int nb_th_a, const int nb_th_d)
{
    // Déclaration des threads
    pthread_t pt_analyse[nb_th_a];
    pthread_t pt_download[nb_th_d];


    // On garde le serveur et le port en mémoire
    strcpy(g_serveur,serveur);
    strcpy(g_port,port);

    char url_fichier[255]; // Variable permettant de mettre l'url du fichier dans le tableau
    // On insère le premier lien à analyser dans l'analyseur
    // On alloue l'espace pour garder les chemins en mémoire et on copie
    f.repertoire[indCstruct] = malloc(sizeof(char)*strlen(chemin));
    strcpy(f.repertoire[indCstruct],chemin);
    strcpy(url_fichier,g_serveur); // On copie l'url de base
    strcat(url_fichier,"/"); // Ajout du slash
    strcat(url_fichier,chemin); // Puis du chemin vers le fichier
    f.url[indCstruct] = malloc(sizeof(char)*strlen(url_fichier));
    strcpy(f.url[indCstruct],url_fichier);
    f.t_analyze[indCstruct] = false;
    f.t_download[indCstruct] = false;

    // On s'occupe maintenant de l'analyseur
    analyseur[indCana] = malloc(sizeof(char)*strlen(chemin));
    strcpy(analyseur[indCana],chemin);

    // On incrémente
    indCana++;
    indCstruct++;


    //Création des threads
    int j = 0;
    while(j<nb_th_a){
        if(pthread_create(&pt_analyse[j],NULL,analyse_page,NULL) != 0)
        {
            perror("Erreur création du thread analyse");
            exit(EXIT_FAILURE);
        }
        ++j;
    }

    j = 0;
    while(j<nb_th_a){
        if(pthread_join(pt_analyse[j],NULL) != 0){
            perror("join analyse fail");
            exit(EXIT_FAILURE);
        }

        ++j;
    }
    j = 0;
    while(j<nb_th_d){
        if(pthread_create(&pt_download[j],NULL,download_page,NULL) != 0){
            perror("Erreur création du thread download");
            exit(EXIT_FAILURE);
        }
        ++j;
    }
    j = 0;
    while(j<nb_th_d){
        if(pthread_join(pt_download[j],NULL) != 0){
            perror("join download fail");
            exit(EXIT_FAILURE);
        }
        ++j;
    }

}

void *analyse_page(void *arg)
{
    (void)arg; //Pour enlever warning
    int nb_bytes = 0; // Le nombre d'octets reçus pour le chunked
    char *chemin; // Le chemin du fichier contenu dans le tableau
     // Variable qui récupérera la ligne courante
    char *cp_ligne; // Copie de ligne afin de ne pas modifer la ligne d'origine
    char *ligne;
    // Variable concernant le cas chunked
    bool chunked = false; // Indiquera si la page est chunked ou pas
    int  chk_bytes;
    // On établit la connexion et on récupère l'en-tête du site \\
    // On ouvre la sockfd et on l'a créé et on l'a test
    int sockfd = CreeSocketClient(g_serveur, g_port) ; // La socket du thread
    if(sockfd == -1)
        perror("Erreur sur la sockfd");
    // On récupère la page à analyser
    chemin = accesTableauAnalyse();
    EnvoieMessage(sockfd,"GET /%s HTTP/1.1\r\nHost: %s\r\n\r\n",chemin,g_serveur);
    // On gère les codes HTTP
    traitementEnTete(sockfd,chemin,&chunked,&chk_bytes);

    while(1)  // Boucle infini
    {
        // Envoie de la requète au serveur

        // Traitement de la page \\
        // Gestion des chemins de fichiers
       // ligne = RecoieLigne(sockfd); initial ligne à la place de REcoie ligne


        // On fait une copie de la ligne pour ne pas l'altérer par la suite
        /*
         * Avant on faisait ligne = RecoieLigne(sockfd)
         * Avec ca probleme de segmentation quel que soit ce qu'on fait
         * Tous les recoiesLIgne(sockfd) de cette fonction ont remplacé ligne
         *
        */
        ligne = RecoieLigne(sockfd);
        nb_bytes = nb_bytes + strlen(ligne);
        cp_ligne = malloc(sizeof(char)*strlen(ligne));
        strcpy(cp_ligne,ligne);
        printf(" Ligne : %s\n",cp_ligne);
        printf("\n%d\n",nb_bytes);

        if (strstr("</html>",cp_ligne) != NULL)
            break;

            //break; //Pour sortir de la boucle

        // Image
        if(strstr(cp_ligne,"<img") != NULL)
            rempliTableauxDownload("src=",cp_ligne);
        // Script JS
        if(strstr(cp_ligne,"<script") != NULL)
            rempliTableauxDownload("src=",cp_ligne);
        // Lien CSS
        if(strstr(cp_ligne,"<link") != NULL){
            rempliTableauxDownload("href=",cp_ligne);
        }
        // Lien <a>
        if(strstr(cp_ligne,"<a") != NULL)
            rempliTableauxDownload("href=",cp_ligne);

    // Si on est dans le cas chunked
        if(chunked){
            // Quand on arrive à la taille indiqué par le chunked on stock la prochaine taille
            if(nb_bytes == chk_bytes){
                //ligne = RecoieLigne(sockfd);
                chk_bytes = strtol(ligne,NULL,16); // On parse la taille en hexa en octets
                nb_bytes = 0;
            }
        } // FIn if chunked
        free(ligne); // On libère la mémoire
    }// FIn while(1
    close(sockfd);
    pthread_exit(NULL);
}// Fin void analyse_page

void *download_page(void *arg)
{
    (void)arg; //Pour enlever warning
    int sockfd  = CreeSocketClient(g_serveur, g_port) ; // La socket du thread
    char *chemin; // Le chemin du fichier contenu dans le tableau
    char *ligne; // Variable qui récupérera la ligne courante
    char *nom_fichier; // Le nom du fichier reçu
    char *lastslash;
    int outfd; // Descripteur du nouveau fichier
    // On établit la connexion et on récupère l'en-tête du site \\
    // On ouvre la sockfd et on l'a créé et on l'a test
    if(sockfd == -1)
        perror("Erreur sur la sockfd");
    while(1){  // Boucle infini
        //Téléchargement des ressources

        chemin = accesTableauDownload();

        //Récupérer le fichier,son extension, etc
        lastslash = malloc(sizeof(char)*strlen(chemin));
        lastslash = strrchr(chemin,'/');

        nom_fichier = malloc(sizeof(char)*strlen(lastslash));
        strcpy(nom_fichier,lastslash);
        lastslash = '\0';
        // Crééation des répertoires

        // Création du fichier
        outfd = open(nom_fichier,O_WRONLY | O_CREAT,S_IRWXU);
        if(outfd == -1) {
            perror("Erreur lors de la création du fichier");
            exit(EXIT_FAILURE);
        }
        // Envoie de la requète au serveur
        EnvoieMessage(sockfd,"GET /%s HTTP/1.1\r\nHost: %s\r\n\r\n",chemin,g_serveur);

        // On va retirer l'en-tête
        // RecoieLigne enlève les caractère spéciaux , c'est pourquoi on va attendre une ligne vide
        // Celle qui sépare l'en-tête du reste du corps
        while( strcmp(ligne,"") != 0) {
            if(strstr("Content-Length: text/html",ligne) != NULL){
                rempliTableauxAnalyse(chemin);
                break;
            }
            free(ligne);
            ligne = RecoieLigne(sockfd);
        }

        // Réception du code source du fichier
        while( ligne != NULL) {
            ligne = RecoieLigne(sockfd);
            if(write(outfd,ligne,strlen(ligne)) != strlen(ligne))
                perror("\nErreur lors de l'écriture du ficher\n");
        }

            free(nom_fichier);
    } // FIn while(1)
    close(sockfd);
    pthread_exit(NULL);
}

void rempliTableauxAnalyse(char *chemin){

    // On vérouille le mutex pour que ce thread soit prioritaire sur la ressource
    pthread_mutex_lock(&t_mutex);

    analyseur[indCana] = malloc(sizeof(char)*strlen(chemin));
    strcpy(downloadeur[indCana],chemin);

    // Enfin on laisse l'accès à la ressource
    pthread_mutex_unlock(&t_mutex);

}

void rempliTableauxDownload(char *type,char *cp_ligne)
{
    char *chemin; // Chemin de fichier
    char url_fichier[255]; // Variable permettant de mettre l'url du fichier dans le tableau
    char *lastguimet; // Afin de supprimer le dernier guillemet des chemins

    // On vérouille le mutex pour que ce thread soit prioritaire sur la ressource
    pthread_mutex_lock(&t_mutex);

    chemin = strstr(cp_ligne,type);
    lastguimet = strrchr(chemin,'"');

    if(chemin != NULL){
        if(lastguimet != NULL)
        {
            lastguimet = '\0'; // On supprime tous ce qui est après la guillemet
            chemin = chemin + strlen(type) + 1; // On supprime le src=",etc
            // On alloue l'espace pour garder les chemins en mémoire et on copie
            f.repertoire[indCstruct] = malloc(sizeof(char)*strlen(chemin));
            strcpy(f.repertoire[indCstruct],chemin);
            strcpy(url_fichier,g_serveur); // On copie l'url de base
            strcat(url_fichier,"/"); // Ajout du slash
            strcat(url_fichier,chemin); // Puis du chemin vers le fichier
            f.url[indCstruct] = malloc(sizeof(char)*strlen(url_fichier));
            strcpy(f.url[indCstruct],url_fichier);
            f.t_analyze[indCstruct] = false;
            f.t_download[indCstruct] = false;

            // On s'occupe maintenant de l'analyseur
            downloadeur[indCdown] = malloc(sizeof(char)*strlen(chemin));
            strcpy(downloadeur[indCdown],chemin);
            printf("\nTest\n");

            // On incrémente
            indCdown++;
            indCstruct++;
        }
    }

    // Enfin on laisse l'accès à la ressource
    pthread_mutex_unlock(&t_mutex);

}

char* accesTableauDownload(){
    //char *donnee;
    // Cas à traiter
    pthread_mutex_lock(&t_mutex);

    char *donnee = (char*)malloc(sizeof(char)*strlen(downloadeur[indRdown]) );
    strcpy(donnee,downloadeur[indRdown]);
    f.t_download[indRstruct] = true;

    // On incrémente
    indRdown++;
    indRstruct++;

    pthread_mutex_unlock(&t_mutex);

    return donnee;

}

char* accesTableauAnalyse(){
    char *donnee;
    // Cas à traiter
    pthread_mutex_lock(&t_mutex);

    donnee = malloc(sizeof(char)*strlen(analyseur[indRana]));
    strcpy(donnee,analyseur[indRana]);
    f.t_analyze[indRstruct] = true;

    // On incrémente
    indRana++;
    indRstruct++;

    pthread_mutex_unlock(&t_mutex);
    return donnee;

}

void creerRepertoire(char* chemin){
    char* repertoire = strtok(chemin,"/");
    struct stat st = {0};
    while(repertoire != NULL) {
        if(stat(repertoire,&st) == -1){
            if(mkdir(repertoire,0700) == -1){
                perror("Erreur création du dossier");
                break;
            }

        }
        repertoire = strtok(chemin,"/");
    }
}

void traitementEnTete(int sockfd, char *chemin, bool *chunked, int *chk_bytes){

//Regarde le code HTTP, ex : 200,404,...
    char* header = RecoieLigne(sockfd);
    int s_header = strlen(header);
    char* tmp = malloc(sizeof(char)*(s_header+4));

    strcpy(tmp,header); // On sauvegarde la première ligne de l'en-tête
    free(header); // On libère la première ligne de code
    header = strtok(tmp," "); // On découpe la chaine en délimitant pas des espaces
    header = strtok(NULL," "); // On récupère la deuxième partie soit le code

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
    if(code == 302){
        while(strncmp(tmp,"Location:",9) != 0)
        {
            free(tmp); // On libère la ligne précédente
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

            pthread_mutex_lock(&t_mutex);
            char url_fichier[255]; // Variable permettant de mettre l'url du fichier dans le tableau
            // On alloue l'espace pour garder les chemins en mémoire et on copie
            f.repertoire[indCstruct] = malloc(sizeof(char)*strlen(new_chemin));
            strcpy(f.repertoire[indCstruct],new_chemin);
            strcpy(url_fichier,new_serveur); // On copie l'url de base
            strcat(url_fichier,"/"); // Ajout du slash
            strcat(url_fichier,new_chemin); // Puis du chemin vers le fichier
            f.url[indCstruct] = malloc(sizeof(char)*strlen(url_fichier));
            strcpy(f.url[indCstruct],url_fichier);
            f.t_analyze[indCstruct] = false;
            f.t_download[indCstruct] = false;

            // On s'occupe maintenant de l'analyseur
            analyseur[indCana] = malloc(sizeof(char)*strlen(new_chemin));
            strcpy(analyseur[indCana],new_chemin);

            // On incrémente
            indCana++;
            indCstruct++;
            pthread_mutex_unlock(&t_mutex);

        } else {
            // On récupère le chemin sans la page (ex:"./test.php" on le supprime)
            char* lastslash  = strrchr(chemin,'/');
            if(lastslash != NULL) *lastslash = '\0';
            printf("\%s\n",chemin);
            tmp = tmp + 11; // On enlève le "./"
            char* new_chemin  = malloc(sizeof(char)*(strlen(tmp)+strlen(chemin)));//Utilise pthread create :)
            strcpy(new_chemin,chemin);
            strcat(new_chemin,tmp);

            printf("\%s\n",new_chemin);

            pthread_mutex_lock(&t_mutex);
            char url_fichier[255]; // Variable permettant de mettre l'url du fichier dans le tableau
            // On alloue l'espace pour garder les chemins en mémoire et on copie
            f.repertoire[indCstruct] = malloc(sizeof(char)*strlen(new_chemin));
            strcpy(f.repertoire[indCstruct],new_chemin);
            strcpy(url_fichier,g_serveur); // On copie l'url de base
            strcat(url_fichier,"/"); // Ajout du slash
            strcat(url_fichier,new_chemin); // Puis du chemin vers le fichier
            f.url[indCstruct] = malloc(sizeof(char)*strlen(url_fichier));
            strcpy(f.url[indCstruct],url_fichier);
            f.t_analyze[indCstruct] = false;
            f.t_download[indCstruct] = false;

            // On s'occupe maintenant de l'analyseur
            analyseur[indCana] = malloc(sizeof(char)*strlen(new_chemin));
            strcpy(analyseur[indCana],new_chemin);

            // On incrémente
            indCana++;
            indCstruct++;
            pthread_mutex_unlock(&t_mutex);

            free(new_chemin);
        }
    }
    printf("\nTest en tete :\n");
    // RecoieLigne enlève les caractère spéciaux , c'est pourquoi on va attendre une ligne vide
    // Celle qui sépare l'en-tête du reste du corps
    while( strcmp(tmp,"") != 0)
    {
        if(strcmp(tmp,"Transfer-Encoding: chunked"))
            *chunked = true;
        free(tmp);
        tmp = RecoieLigne(sockfd);
    }
    if(*chunked)
    {
        tmp = RecoieLigne(sockfd);
        *chk_bytes = strtol(tmp,NULL,16); // on parse l'hexa en octets
        free(tmp);
    }
    // Fin du traitement de l'en-tête \\


}


