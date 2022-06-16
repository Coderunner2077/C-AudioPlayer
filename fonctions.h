#ifndef FONCTIONS_H_INCLUDED
#define FONCTIONS_H_INCLUDED
void configurerAffichage(int dispo[][NB_BLOCS_LARGEUR], progression[][NB_BLOCS_BIS_LARGEUR]);
int determinerProgression(unsigned int positionMorceau, unsigned int length);
int determinerIndicateur(unsigned int positionMorceau, unsigned int length);
void synchroniserDurees(unsigned int *m, unsigned int *s, unsigned int *length, unsigned int *positionMorceau, unsigned int *minutes, unsigned int *secondes);
int chargerMusique(FMOD_System **system, FMOD_SOUND** musique, long *numeroMorceau, char emplacementMorceau[]);
int determinerMorceau(long *numeroMorceau, int *replay, int *commande, long *lastSong, char *sonActuel);


#endif // FONCTIONS_H_INCLUDED
