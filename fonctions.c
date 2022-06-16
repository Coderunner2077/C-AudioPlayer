#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fmod/fmod.h>
#include <fmod/fmod_common.h>
#include <dirent.h>
#ifndef WIN32
    #include <sys/types.h>
#endif // WIN32

#include "constantes.h"

int chargerMusique(FMOD_SYSTEM **system, FMOD_SOUND** musique, long *numeroMorceau, char emplacementMorceau[])
{
    char emplacementFichier[150] = {"bibliotheque\\"};
    char *positionEntree = NULL;
    long compteur = 0;
    DIR *rep = NULL;
    rep = opendir("bibliotheque");
    if(rep == NULL)
    {
        fprintf(stderr, "Erreur d'ouverture du fichier");
        exit(EXIT_FAILURE);
    }
    struct dirent* fichierLu = NULL;
    seekdir(rep, 2);
    fichierLu = readdir(rep);
    strcat(emplacementFichier, fichierLu->d_name);
    FILE* fichier = NULL;
    fichier = fopen(emplacementFichier, "r");
    if(fichier == NULL)
    {
        fprintf(stderr, "Impossible d'ouvrir le playlist");
        exit(EXIT_FAILURE);
    }

    while(compteur < *numeroMorceau)
    {
        fgets(emplacementMorceau, 150, fichier);
        if(strstr(emplacementMorceau, "#EXT") == NULL)
            compteur++;
    }
    positionEntree = strchr(emplacementMorceau, '\n');
    *positionEntree = '\0';
    if(*numeroMorceau != 1)
    FMOD_Sound_Release(*musique);
    FMOD_RESULT resultat;
    resultat = FMOD_System_CreateSound(*system, emplacementMorceau, FMOD_2D | FMOD_CREATESTREAM | FMOD_LOOP_NORMAL, 0, &(*musique));
    fclose(fichier);
    closedir(rep);
    if(resultat == FMOD_OK)
        return 1;
    else
        return 0;

}
int determinerMorceau(long *numeroMorceau, int *replay, int *commande, long *lastSong, char *sonActuel)
{
    char emplacementFichier[150] = {"bibliotheque\\"}, empMorceau[150] = {""};
    long compteur = 0, verif = 0, numLigneMorceau = 0;
    int play = 1;
    char *virgule = NULL, *positionEntree = NULL;
    *lastSong = verif;
    DIR *rep = NULL;
    rep = opendir("bibliotheque");
    if(rep == NULL)
    {
        fprintf(stderr, "Erreur d'ouverture du fichier");
        exit(EXIT_FAILURE);
    }
    struct dirent* fichierLu = NULL;
    seekdir(rep, 2);
    fichierLu = readdir(rep);
    strcat(emplacementFichier, fichierLu->d_name);
    FILE* fichier = NULL;
    fichier = fopen(emplacementFichier, "r");
    if(fichier == NULL)
    {
        fprintf(stderr, "Impossible d'ouvrir le playlist");
        exit(EXIT_FAILURE);
    }
     while(fgets(empMorceau, 150, fichier) != NULL)
    {
        if(strstr(empMorceau, ".mp3\n") != 0)
            *lastSong += 1;
    }
    verif = 0;
    if(*commande == FORWARD && *numeroMorceau < *lastSong && *replay <= 0)
        *numeroMorceau = *numeroMorceau + 1;
    else if(*commande == NEXT && *numeroMorceau < *lastSong && *replay <= 0)
        *numeroMorceau = *numeroMorceau + 1;
    else if(*commande == NEXT && *numeroMorceau < *lastSong && *replay <= 0)
        *numeroMorceau = *numeroMorceau + 1;
    else if(*commande == FORWARD && *numeroMorceau == *lastSong && *replay < 0)
        *numeroMorceau = 1;
    else if(*commande == NEXT && *numeroMorceau == *lastSong && *replay < 0)
        *numeroMorceau = 1;
    else if(*commande == FORWARD && *numeroMorceau == *lastSong && *replay == 0)
        *numeroMorceau = *lastSong;
    else if(*commande == NEXT && *numeroMorceau == *lastSong && *replay == 0)
    {
        play = 0;
        *numeroMorceau = 1;
    }
    else if(*commande == BACKWARD && *numeroMorceau > 1)
        *numeroMorceau = *numeroMorceau - 1;

    numLigneMorceau = *numeroMorceau * 2;
    rewind(fichier);
     while(compteur <= numLigneMorceau)
    {
        fgets(empMorceau, 150, fichier);
        compteur++;
        if(compteur == numLigneMorceau)
        {
            sprintf(sonActuel, "%s", strchr(empMorceau, ','));
            virgule = strchr(sonActuel, ',');
            *virgule = ' ';
            positionEntree = strchr(sonActuel, '\n');
            *positionEntree = '\0';
            verif = compteur;
        }
    }

    fclose(fichier);
    closedir(rep);
    return play;
}

void configurerAffichage(int dispo[][NB_BLOCS_LARGEUR], int progression[][NB_BLOCS_BIS_LARGEUR])
{
    int i = 0, j = 0;

    dispo[0][0] = TITLE;


    dispo[2][0] = DUREE;
    for(i=2; i<3; i++)
    {
        for(j=1; j<NB_BLOCS_LARGEUR - 1; j++)
        {
            dispo[i][j] = BARRE;
        }
    }
    for(i=0; i<NB_BLOCS_BIS_HAUTEUR; i++)
    {
        for(j=0; j<NB_BLOCS_BIS_LARGEUR; j++)
        {
            progression[i][j] = TOPLAY;
        }
    }

    dispo[2][6] = TEMPS_RESTANT;
    dispo[3][0] = BACKWARD;
    dispo[3][1] = STOP;
    dispo[3][2] = PAUSE;
    dispo[3][3] = FORWARD;
    dispo[3][4] = REPLAY_OFF;
    dispo[3][5] = VOLUME;
    dispo[3][6] = BARRE_VOLUME;
}

int determinerProgression(unsigned int positionMorceau, unsigned int length)
{
    int played = 0;
    double position = 0, longueur = 0;
    position = positionMorceau / 1000;
    longueur = length / 1000;
    played = (position / longueur) * NB_BLOCS_BIS_LARGEUR;
    return played;
}

void synchroniserDurees(unsigned int *m, unsigned int *s, unsigned int *length, unsigned int *positionMorceau, unsigned int *minutes, unsigned int *secondes)
{
    *s = *positionMorceau / 1000;
    *m = *s / 60;
    *s %= 60;
    *secondes = (*length - *positionMorceau) / 1000;
    *minutes = *secondes / 60;
    *secondes %= 60;

}

int determinerIndicateur(unsigned int positionMorceau, unsigned int length)
{
    int indicateur = 0;
    double position = 0, longueur = 0;
    position = positionMorceau / 1000;
    longueur = length / 1000;
    indicateur = (position / longueur) * (5 * TAILLE_BLOC - 19);

    return indicateur;
}
