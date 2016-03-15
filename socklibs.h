#pragma once
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
//#include <error.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h> 

/* Une partie nécessaire pour utiliser les sockets sous linux et windows */
#if defined (WIN32)
#include <winsock2.h>
#elif defined (linux)
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

//#define MSG_NOSIGNAL -1 //TODO: celle-l� devrait �tre d�finie dans <sys/socket.h> ???

#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define TAILLE_TAB 255
/* 	Structure reposant sur les listes chain�s. 
	DescPage est une structure contenant la description de chacune des pages � traiter ou d�j� trait�
*/
typedef struct _DescPage{
	char repertoire[TAILLE_TAB];
	char url[TAILLE_TAB];
	bool t_download; // Si Oui, on a d�j� trait� avec le thread de telechargement
	bool t_analyze; // SI Oui, on a d�j� trait� avec le thread d'analyse
	struct _DescPage *suivant; //POur passer � la page suivante
	/* ou faire char description[255][255]
	
	*/
}DescPage;

typedef struct _PageDownload{
	char identifiant[TAILLE_TAB]; // Contient l'identifiant de la page & l'url a telecharg�
	struct _PageDownload *suivant;	
	// Pour remplir cette structure, il faut ajouter chaque descPage ou t_download est � false
	
}PageDownload;

typedef struct _PageAnalyze{
	char identifiant[TAILLE_TAB]; // Tableau bi dimensionnel, contenant l'identifiant des oages tekecgarg� mais pas trait�s
	struct _PageAnalyze *suivant;	
	// Pour remplir cette structure, il faut ajouter chaque descPage o� t_download � true && t_analyze � false
}PageAnalyze;
typedef struct _ListeEnsemblePage{
	DescPage *debut; // Premiere element de la liste de page
	int nb_pages; //Indique le nombre de page � trait� ou d�j� trait�
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
 * Fonctions qui suppriment les PageDOwnload & PAgesAnalyze d�j� trait�s
 * Utiliser le pointeur de liste, faire pointer la tete vers l'element suivant
 * NE pas oublier de faire un free de l'element supprim�
 */
void free_pageDownload(ListeDownload ld);
void free_pageAnalyze(LIsteAnalyze la);

/** Fonction qui remplit la structure de description page
 *  Cr��r socket connexion, faire comme 1er TP, jusqu'a recoie et sauvedonnee
 *  SOIT !Cr�er un truc simliaire � REcoieETSAuvedonne sauf qui stocke chaque url (via href) dans descPAge
 *  (ON peut utilser une bibliotheque pour parser)
 *  SOIT , lance un thread de type Telechargement qui parcours l'ensemble des lien dispo page et remplit
 *  LIsteEnsemblePAge + ListeDownload + ListeAnalyze (AU sein d'une boucle on parcours le code source une seule fois)
 *  T
 * /!\ Avec la structure, bien gerer l'initialisation & le premier element
*/
void recup_description_page(const char * url);


/**
 * @brief Cree une socket d'attente pour le serve																																																																																																																																																																																																																																																																																																																																																																																																																																																			ur sur le port port
 * @param port : le port utilisé
 * @return la socket d'attente ou -1 en cas d'erreur
 *
 * en cas d'erreur, un message explicatif est affich� sur la sortie d'erreur
 * standart
 */
int CreeSocketServeur(const char* port);
int CreeSocketClient(const char *serveur, const char* port);

///retourne la socket accept�e
int AcceptConnexion(int s);

/**
 * Lire les donn�es sur une socket et les �crire automatiquement dans un descripteur de fichier
 * Cette fonction stoppe la lecture lorsque la soket est ferm�e.
 * @param sock : la socket d'o� proviennent les donn�es, cela fonctionne aussi
 *               si sock est un file descriptor quelconque
 * @param fd : le descripteur de fichier sur lesquel enregistrer les donn�es
 * @return : si cela a fonctionn� le nombre d'octets lus, sinon -1
 */
int RecoieEtSauveDonnees(int fd, int sock);

/**
 * Lire les donn�es sur une socket (uniquement j'utilise recv) jusqu'� arriv� � un retour chariot '\n' la donn�e est stok�e dans un tableau dont la taille est adapt� pour cela).
 * @param sock : la socket de lecture
 * @return la chaine lue � lib�rer par free
 */
char *RecoieLigne(int sock);

/**
 * envoie le message format� sur la socket s (comme un printf)
 * @param s : la socket sur laquel ecrire le message
 * @param format : le format du message (comme pour printf)
 * @param ...: les parametres optionnels
 * @return le nombre d'octet �crit ou -1 s'il y a eu un probl�me
 */
int EnvoieMessage(int s, char* format, ...);


/**
 * Regarde s'il y a qqchose � lire sur une socket
 * @param s : la socket
 * @return * -1 s'il y a une erreur
 *         * -2 s'il n'y a rien � lire
 *         * 0 si la socket est ferm�e
 *         * 1 s'il y a qqchose � lire
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
