#include "socklib.h"
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
	char identifiant[TAILLE_TAB]; // Tableau bi-dimensionnel, contenant l'identifiant de la page & l'url a telechargé
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
 */
//void http_get(const char * serveur, const char * port, const char * chemin, const char * nom_fichier);
void http_get(const char * serveur, const char * port, const char * chemin, const char * nom_fichier);

/**
 * Fonction qui s'occupe d'analyser les pages afin de récupérer les liens,images,css,etc
 * @param *PageAnalyse
 * @param *PageDowload
 */
void analyse_page(PageAnalyze *analyse, PageDownload *download);

/**
 * Fonction qui s'occupe de télécharger les pages,images,css,etc
 * @param *PageAnalyse
 * @param *PageDowload
 */
void download_page(PageAnalyze *analyse, PageDownload *download);
