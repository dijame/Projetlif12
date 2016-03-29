#include "socklib.h"
#define TAILLE_TAB 255
/* 	Structure reposant sur des tableaux dynamique.
	FilePage est une structure contenant les fichiers de chacune des pages à traiter ou à télécharger
*/
typedef struct _FilePage
{
    char *repertoire[TAILLE_TAB];
    char *url[TAILLE_TAB];
    bool t_download[TAILLE_TAB]; // Si Oui, on a déjà traité avec le thread de telechargement
    bool t_analyze[TAILLE_TAB]; // Si Oui, on a déjà traité avec le thread d'analyse

} FilePage;

// Fonctions d'initialisations ///////TESTS///
void initialisationFilePage(FilePage f);


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
 * Fonction qui se connecte au serveur, telecharge la page http://serveur:port/chemin et sauvegarde dans nom_fichier
 * @param serveur
 * @param port
 * @param chemin
 * @param nom_fichier
 * @param nb_th_a
 * @param nb_th_d
 */
//void http_get(const char * serveur, const char * port, const char * chemin, const char * nom_fichier);
void http_get(const char * serveur, const char * port, const char * chemin, const char * nom_fichier, const int nb_th_a, const int nb_th_d);

/**
 * Fonction qui s'occupe d'analyser les pages afin de récupérer les liens,images,css,etc
 * @param *f
 */
void *analyse_page(void *arg);

/**
 * Fonction qui s'occupe de télécharger les pages,images,css,etc
 * @param *f
 */
void *download_page(void *arg);

/**
 * Fonction qui rempli la structure ansi que le tableau analyseur
 * @param *type
 * @param *cp_ligne
*/
void rempliTableauxAnalyse(char *type,char *cp_ligne);
