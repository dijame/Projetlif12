#pragma once
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
//#include <error.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h> 

/* Une partie nÃ©cessaire pour utiliser les sockets sous linux et windows */
#if defined (WIN32)
#include <winsock2.h>
#elif defined (linux)
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

//#define MSG_NOSIGNAL -1 //TODO: celle-là devrait être définie dans <sys/socket.h> ???

#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define TAILLE_TAB 255
/* 	Structure reposant sur les listes chainés. 
	DescPage est une structure contenant la description de chacune des pages à traiter ou déjà traité
*/
typedef struct _DescPage{
	char repertoire[TAILLE_TAB];
	char url[TAILLE_TAB];
	bool t_download; // Si Oui, on a déjà traité avec le thread de telechargement
	bool t_analyze; // SI Oui, on a déjà traité avec le thread d'analyse
	struct _DescPage *suivant; //POur passer à la page suivante
	/* ou faire char description[255][255]
	
	*/
}DescPage;

typedef struct _PageDownload{
	char identifiant[TAILLE_TAB]; // Contient l'identifiant de la page & l'url a telechargé
	struct _PageDownload *suivant;	
	// Pour remplir cette structure, il faut ajouter chaque descPage ou t_download est à false
	
}PageDownload;

typedef struct _PageAnalyze{
	char identifiant[TAILLE_TAB]; // Tableau bi dimensionnel, contenant l'identifiant des oages tekecgargé mais pas traités
	struct _PageAnalyze *suivant;	
	// Pour remplir cette structure, il faut ajouter chaque descPage où t_download à true && t_analyze à false
}PageAnalyze;
typedef struct _ListeEnsemblePage{
	DescPage *debut; // Premiere element de la liste de page
	int nb_pages; //Indique le nombre de page à traité ou déjà traité
}ListeEnsemblePage;

typedef struct _ListeDownload{
	PageDownload *debut;
	int nb_pages;
}ListeDownload;

typedef struct _ListeAnalyze{
	PageAnalyze *debut;
	int nb_pages;
}ListeAnalyze;
// Fonctions d'initialisations
void initialisationDescPage(ListeEnsemblePage l_page);
void initialisationPageDownload(ListeDownload l_download);
void initialisationPageAnalyze(ListeAnalyze l_analyze);

/*
 * Fonctions qui suppriment les PageDOwnload & PAgesAnalyze déjà traités
 * Utiliser le pointeur de liste, faire pointer la tete vers l'element suivant
 * NE pas oublier de faire un free de l'element supprimé
 */
void free_pageDownload(ListeDownload ld);
void free_pageAnalyze(LIsteAnalyze la);

/** Fonction qui remplit la structure de description page
 *  Créér socket connexion, faire comme 1er TP, jusqu'a recoie et sauvedonnee
 *  SOIT !Créer un truc simliaire à REcoieETSAuvedonne sauf qui stocke chaque url (via href) dans descPAge
 *  (ON peut utilser une bibliotheque pour parser)
 *  SOIT , lance un thread de type Telechargement qui parcours l'ensemble des lien dispo page et remplit
 *  LIsteEnsemblePAge + ListeDownload + ListeAnalyze (AU sein d'une boucle on parcours le code source une seule fois)
 *  T
 * /!\ Avec la structure, bien gerer l'initialisation & le premier element
*/
void recup_description_page(const char * url);


/**
 * @brief Cree une socket d'attente pour le serve																																																																																																																																																																																																																																																																																																																																																																																																																																																			ur sur le port port
 * @param port : le port utilisÃ©
 * @return la socket d'attente ou -1 en cas d'erreur
 *
 * en cas d'erreur, un message explicatif est affiché sur la sortie d'erreur
 * standart
 */
int CreeSocketServeur(const char* port);
int CreeSocketClient(const char *serveur, const char* port);

///retourne la socket acceptée
int AcceptConnexion(int s);

/**
 * Lire les données sur une socket et les écrire automatiquement dans un descripteur de fichier
 * Cette fonction stoppe la lecture lorsque la soket est fermée.
 * @param sock : la socket d'où proviennent les données, cela fonctionne aussi
 *               si sock est un file descriptor quelconque
 * @param fd : le descripteur de fichier sur lesquel enregistrer les données
 * @return : si cela a fonctionné le nombre d'octets lus, sinon -1
 */
int RecoieEtSauveDonnees(int fd, int sock);

/**
 * Lire les données sur une socket (uniquement j'utilise recv) jusqu'à arrivé à un retour chariot '\n' la donnée est stokée dans un tableau dont la taille est adapté pour cela).
 * @param sock : la socket de lecture
 * @return la chaine lue à libérer par free
 */
char *RecoieLigne(int sock);

/**
 * envoie le message formaté sur la socket s (comme un printf)
 * @param s : la socket sur laquel ecrire le message
 * @param format : le format du message (comme pour printf)
 * @param ...: les parametres optionnels
 * @return le nombre d'octet écrit ou -1 s'il y a eu un problème
 */
int EnvoieMessage(int s, char* format, ...);


/**
 * Regarde s'il y a qqchose à lire sur une socket
 * @param s : la socket
 * @return * -1 s'il y a une erreur
 *         * -2 s'il n'y a rien à lire
 *         * 0 si la socket est fermée
 *         * 1 s'il y a qqchose à lire
 */
int TestLecture(int s);
/**
 * Fonction qui se connecte au serveur, telecharge la page http://serveur:port/chemin et sauvegarde dans nom_fichier
 * @param serveur
 * @param port
 * @param chemin
 * @param nom_fichier
 */
//void http_get(const char * serveur, const char * port, const char * chemin, const char * nom_fichier);
void http_get(const char * serveur, const char * port, const char * chemin, const char * nom_fichier);
