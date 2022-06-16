#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>
#include <fmod/fmod.h>
#include <fmod/fmod_common.h>
#include <dirent.h>
#ifndef WIN32
    #include <sys/types.h>
#endif // WIN32

#include "constantes.h"


int main(int argc, char **argv)
{
    SDL_Surface *ecran = NULL, *titre = NULL, *play = NULL, *pause = NULL, *stop = NULL, *forward = NULL, *backward = NULL,
    *played = NULL, *toplay = NULL, *replay = NULL, *replay_OFF = NULL, *duree = NULL, *tempsRestant = NULL, *verif = NULL,
    *indicateur = NULL, *sVolume = NULL, *sMuet = NULL, *barreVolume[TAILLE_BARRE_VOLUME] = {NULL},
    *indVolume = NULL, *replay_ONE = NULL;
    SDL_Event event;
    SDL_Rect position, positionIndicateur = {TAILLE_BLOC, 2*TAILLE_BLOC + TAILLE_BLOC / 3 + 4}, pos;
    int continuer =1, i, j, repet = 0, clicGaucheEnCours = 0, tempsActuel = 0, tempsPrecedent = 0, k, progressionMorceau = 0,
     progressionIndicateur = 0, muet = 0, volumeActuel = 0, commande = 0, playNext = 0, numParams = 0;
    int dispo[NB_BLOCS_HAUTEUR][NB_BLOCS_LARGEUR] = {0}, progression[NB_BLOCS_BIS_HAUTEUR][NB_BLOCS_BIS_LARGEUR] = {0};
    char sonActuel[100] = {""}, emplacementMorceau[150] = {""}, charDuree[6] = {""}, charTempsRestant [6] = {""},
     charVerif[10] = {""}, charTest[15] = {""};
    float volume = 1.0, teste = 0, flVolume = 0;
    float spectre[512] = {0};
    long lastSong = 0, numeroMorceau = 1;
    unsigned int length = 0, minutes = 0, secondes = 0, m = 0, s = 0, positionMorceau = 0, posMorceau = 0;
    FMOD_BOOL isPlaying, mute;

    FMOD_SYSTEM *system;
    FMOD_System_Create(&system);
    if(FMOD_System_Init(system, 36, FMOD_INIT_NORMAL, NULL) != FMOD_OK)
    {
        fprintf(stderr, "Erreur d'initialisation du FMOD");
        exit(EXIT_FAILURE);
    }

    FMOD_SOUND *musique;

    playNext = determinerMorceau(&numeroMorceau, &repet, &commande, &lastSong, sonActuel);
    if(!chargerMusique(&system, &musique, &numeroMorceau, emplacementMorceau))
    {
        fprintf(stderr, "Erreur de lecture du morceau : %s", emplacementMorceau);
        exit(EXIT_FAILURE);
    }

    FMOD_CHANNELGROUP *canaux;
    FMOD_CHANNEL *canal;
    FMOD_DSP *dsp;
    FMOD_System_GetMasterChannelGroup(system, &canaux);
    FMOD_ChannelGroup_GetChannel(canaux, 9, &canal);
    if(playNext)
        FMOD_System_PlaySound(system, musique, 0, 0, &canal);
    FMOD_Channel_SetLoopCount(canal, repet);
    /*
    FMOD_ChannelGroup_GetDSP(canal, FMOD_CHANNELCONTROL_DSP_HEAD, &dsp);
    FMOD_System_CreateDSPByType(system, FMOD_DSP_TYPE_FFT, &dsp);
    FMOD_ChannelGroup_AddDSP(canal, FMOD_CHANNELCONTROL_DSP_HEAD, dsp);
    FMOD_DSP_GetNumParameters(dsp, &numParams);
    FMOD_RESULT result;
    result = FMOD_DSP_GetParameterFloat(dsp, 0, spectre, 0, 0);
    if(result != FMOD_OK)
    {
        fprintf(stderr, "Erreur DSP : %d", result);
        exit(EXIT_FAILURE);
    }
    */

    SDL_Init(SDL_INIT_VIDEO);
    SDL_WM_SetIcon(IMG_Load("icon.png"), NULL);
    ecran = SDL_SetVideoMode(LARGEUR_FENETRE, HAUTEUR_FENETRE, 32, SDL_HWSURFACE);
    SDL_WM_SetCaption("Lecteur Audio", NULL);
    play = IMG_Load("play.png");
    pause = IMG_Load("pause.png");
    stop = IMG_Load("stop.png");
    backward = IMG_Load("backward.png");
    forward = IMG_Load("forward.png");
    replay = IMG_Load("replay.png");
    replay_OFF = IMG_Load("replay_OFF.png");
    played = IMG_Load("barre_progression.png");
    toplay = IMG_Load("barre_regression.png");
    indicateur = IMG_Load("indicateur2.png");
    sVolume = IMG_Load("volume.png");
    sMuet = IMG_Load("muet.png");
    indVolume = IMG_Load("ind_volume.png");
    replay_ONE = IMG_Load("replay_ONE.png");
    for(i=0; i< TAILLE_BARRE_VOLUME; i++)
        barreVolume[i] = SDL_CreateRGBSurface(SDL_HWSURFACE, 1, 5, 32, 0, 0, 0, 0);

    TTF_Init();
    TTF_Font *police = NULL, *policeTitre = NULL;
    police = TTF_OpenFont("arial.ttf", 35);
    policeTitre = TTF_OpenFont("arial.ttf", 15);
    SDL_Color couleurNoire = {0, 0, 0};

    configurerAffichage(dispo, progression);

    FMOD_Sound_GetLength(musique, &length, FMOD_TIMEUNIT_MS);
    synchroniserDurees(&m, &s, &length, &positionMorceau, &minutes, &secondes);
    sprintf(charDuree, "%d:0%d", m, s);
    duree = TTF_RenderText_Solid(police, charDuree, couleurNoire);
    sprintf(charTempsRestant, "%d:%d", minutes, secondes);
    tempsRestant = TTF_RenderText_Solid(police, charTempsRestant, couleurNoire);

    while(continuer)
    {
        SDL_PollEvent(&event);
        FMOD_Channel_SetVolume(canal, volume);
        switch(event.type)
        {
            case SDL_QUIT:
                continuer = 0;
                FMOD_Channel_Stop(canal);
                break;
            case SDL_MOUSEBUTTONDOWN:
                if(event.button.button == SDL_BUTTON_LEFT)
                {
                    if(dispo[event.button.y / TAILLE_BLOC][event.button.x / TAILLE_BLOC] == PLAY ||
                       dispo[event.button.y / TAILLE_BLOC][event.button.x / TAILLE_BLOC] == PAUSE)
                    {
                        FMOD_ChannelGroup_IsPlaying(canal, &isPlaying);
                        if(isPlaying)
                        {
                            FMOD_BOOL etat;
                            FMOD_Channel_GetPaused(canal, &etat);
                            if(etat == 1)
                            {
                                FMOD_Channel_SetPaused(canal, 0);
                                dispo[3][2] = PAUSE;
                            }
                            else
                            {
                                FMOD_Channel_SetPaused(canal, 1);
                                dispo[3][2] = PLAY;
                            }
                        }
                        else if(!isPlaying)
                        {
                            FMOD_System_PlaySound(system, musique, 0, 0, &canal);
                            FMOD_Sound_GetLength(musique, &length, FMOD_TIMEUNIT_MS);
                            synchroniserDurees(&m, &s, &length, &positionMorceau, &minutes, &secondes);
                            dispo[3][2] = PAUSE;
                        }
                    }
                    else if(dispo[event.button.y / TAILLE_BLOC][event.button.x / TAILLE_BLOC] == STOP)
                    {
                        FMOD_Channel_Stop(canal);
                        dispo[3][2] = PLAY;
                        synchroniserDurees(&m, &s, &length, &positionMorceau, &minutes, &secondes);
                    }
                    else if(dispo[event.button.y / TAILLE_BLOC][event.button.x / TAILLE_BLOC] == FORWARD)
                        {
                            commande = FORWARD;
                            playNext = determinerMorceau(&numeroMorceau, &repet, &commande, &lastSong, sonActuel);
                            sprintf(charVerif, "%ld", lastSong);
                            verif = TTF_RenderText_Solid(policeTitre, charVerif, couleurNoire);
                            FMOD_Channel_Stop(canal);
                            if(!chargerMusique(&system, &musique, &numeroMorceau, emplacementMorceau, &lastSong))
                            {
                                fprintf(stderr, "Erreur de lecture du morceau : %s", emplacementMorceau);
                                exit(EXIT_FAILURE);
                            }
                            if(playNext)
                                FMOD_System_PlaySound(system, musique, 0, 0, &canal);
                            FMOD_Channel_GetPosition(canal, &positionMorceau, FMOD_TIMEUNIT_MS);
                            FMOD_Sound_GetLength(musique, &length, FMOD_TIMEUNIT_MS);
                            synchroniserDurees(&m, &s, &length, &positionMorceau, &minutes, &secondes);
                            dispo[3][2] = PAUSE;
                        }
                    else if(dispo[event.button.y / TAILLE_BLOC][event.button.x / TAILLE_BLOC] == BACKWARD)
                        {
                            commande = BACKWARD;
                            determinerMorceau(&numeroMorceau, &repet, &commande, &lastSong, sonActuel);
                            FMOD_Channel_Stop(canal);
                            if(!chargerMusique(&system, &musique, &numeroMorceau, emplacementMorceau, &lastSong))
                            {
                                fprintf(stderr, "Erreur de lecture du morceau : %s", emplacementMorceau);
                                exit(EXIT_FAILURE);
                            }
                            FMOD_System_PlaySound(system, musique, 0, 0, &canal);
                            FMOD_Sound_GetLength(musique, &length, FMOD_TIMEUNIT_MS);
                            synchroniserDurees(&m, &s, &length, &positionMorceau, &minutes, &secondes);
                            dispo[3][2] = PAUSE;
                        }
                    else if(dispo[event.button.y / TAILLE_BLOC][event.button.x / TAILLE_BLOC] == REPLAY_OFF ||
                            dispo[event.button.y / TAILLE_BLOC][event.button.x / TAILLE_BLOC] == REPLAY ||
                            dispo[event.button.y / TAILLE_BLOC][event.button.x / TAILLE_BLOC] == REPLAY_ONE)
                    {
                        FMOD_Sound_GetLoopCount(musique, &repet);
                        if(repet == 0)
                        {
                            dispo[3][4] = REPLAY;
                            FMOD_Channel_SetLoopCount(canal, -1);
                        }
                        else if(repet < 0)
                        {
                            dispo[3][4] = REPLAY_ONE;
                            FMOD_Channel_SetLoopCount(canal, 1);
                        }
                        else if(repet)
                        {
                            dispo[3][4] = REPLAY_OFF;
                            FMOD_Channel_SetLoopCount(canal, 0);
                        }
                    }
                    else if((event.button.y >= 2 * TAILLE_BLOC + TAILLE_BLOC / 3 + 3 && event.button.y <= 2 * TAILLE_BLOC + TAILLE_BLOC / 3 + 22)
                            &&(event.button.x >= TAILLE_BLOC && event.button.x <= TAILLE_BLOC + NB_BLOCS_BIS_LARGEUR*TAILLE_BLOC_BIS))
                        {
                            positionMorceau = (event.button.x - TAILLE_BLOC)*(length / (5 * TAILLE_BLOC - 19));
                            FMOD_Channel_SetPosition(canal, positionMorceau, FMOD_TIMEUNIT_MS);
                            synchroniserDurees(&m, &s, &length, &positionMorceau, &minutes, &secondes);
                            clicGaucheEnCours = 1;
                        }

                   else if((event.button.y >= POSITION_VOLUME_Y && event.button.y <= (POSITION_VOLUME_Y + sVolume->h))
                            && (event.button.x >= POSITION_VOLUME_X && event.button.x <= POSITION_VOLUME_X + sVolume->w))
                    {
                        FMOD_Channel_GetMute(canal, &mute);
                        if(mute)
                        {
                            FMOD_Channel_SetMute(canal, 0);
                            muet = 0;

                        }
                        else
                        {
                            FMOD_Channel_SetMute(canal, 1);
                            muet = 1;
                        }
                    }
                    else if((event.button.y >= P_BARRE_VOLUME_Y - MARGE && event.button.y <= P_BARRE_VOLUME_Y + MARGE)
                            && (event.button.x >= P_BARRE_VOLUME_X && event.button.x < P_BARRE_VOLUME_X + TAILLE_BARRE_VOLUME))
                    {
                        teste = (event.button.x - P_BARRE_VOLUME_X - event.button.x / 4); // CORRECTION ANOMALIE AVEC " - event.button.x / 4"
                        flVolume = TAILLE_BARRE_VOLUME;
                        teste = teste / flVolume;
                        if(teste <= 0.03)// CORRECTION VOLUME SUITE A DES BIZARRERIES...
                            volume = 0.0;
                        else if(teste >= 0.73)
                            volume = 1.0;
                        else
                            volume = teste*1.2;
                        volumeActuel = volume * TAILLE_BARRE_VOLUME;
                        if(volumeActuel > TAILLE_BARRE_VOLUME - indVolume->w)
                            volumeActuel = TAILLE_BARRE_VOLUME - indVolume->w;
                        FMOD_Channel_SetVolume(canal, volume);
                    }
                }
                break;
            case SDL_MOUSEMOTION:
                if(clicGaucheEnCours)
                {
                    if(event.button.x >= TAILLE_BLOC && event.button.x <= TAILLE_BLOC + NB_BLOCS_BIS_LARGEUR*TAILLE_BLOC_BIS)
                    {
                        positionMorceau = (event.button.x - TAILLE_BLOC)*(length / (5 * TAILLE_BLOC - 19));
                        FMOD_Channel_SetPosition(canal, positionMorceau, FMOD_TIMEUNIT_MS);
                        synchroniserDurees(&m, &s, &length, &positionMorceau, &minutes, &secondes);
                    }
                }
                break;
            case SDL_MOUSEBUTTONUP:
                clicGaucheEnCours = 0;
                break;

        }
        // DETERMINER LE TITRE DU MORCEAU EN COURS

        titre = TTF_RenderText_Solid(policeTitre, sonActuel, couleurNoire);

        //DETERMINER LA REPETITION DU CANAL EN FONCTION DE REPLAY OU DE REPLAY_OFF

        if(dispo[3][4] == REPLAY)
            repet = -1;
        else if(dispo[3][4] == REPLAY_OFF)
            repet = 0;
        else if(dispo[3][4] == REPLAY_ONE)
            repet = 1;
        FMOD_Channel_SetLoopCount(canal, repet);
        // DETERMINER SI LECTURE EN COURS OU PAS, EN PAUSE OU PAS
        FMOD_Channel_IsPlaying(canal, &isPlaying);
        FMOD_BOOL etat;
        FMOD_Channel_GetPaused(canal, &etat);

        // DETERMINER L'AVANCEMENT DE LA DUREE ET DU TEMPS RESTANT DE LA CHANSON EN FCT DE L'ETAT DE LECTURE
        if(isPlaying && !etat) // SI LECTURE EN COURS ET PAS EN PAUSE
        {
            dispo[3][2] = PAUSE;
            tempsActuel = SDL_GetTicks();
            if(tempsActuel - tempsPrecedent > 199)
            {
                tempsPrecedent = tempsActuel;
                //DETETERMINER LA POSITION DE L'INDICATEUR
                FMOD_Channel_GetPosition(canal, &positionMorceau, FMOD_TIMEUNIT_MS);
                FMOD_Channel_GetPosition(canal, &posMorceau, FMOD_TIMEUNIT_MS);
                progressionMorceau = determinerProgression(positionMorceau, length);
                progressionIndicateur = determinerIndicateur(positionMorceau, length);
                positionIndicateur.x = TAILLE_BLOC + progressionIndicateur;
                if(progressionMorceau < NB_BLOCS_BIS_LARGEUR / 2) // SYNCHRONISATION progression ET indicateur
                {
                    for(k=0; k<=progressionMorceau + 1; k++)
                        progression[0][k] = PLAYED;
                    for(k=progressionMorceau + 1; k<NB_BLOCS_BIS_LARGEUR; k++)
                        progression[0][k] = TOPLAY;
                }
                else
                {
                    for(k=0; k<progressionMorceau; k++)
                        progression[0][k] = PLAYED;
                    for(k=progressionMorceau; k<NB_BLOCS_BIS_LARGEUR; k++)
                        progression[0][k] = TOPLAY;
                }
                synchroniserDurees(&m, &s, &length, &positionMorceau, &minutes, &secondes);
                if(s < 10) // CORRECTION ECRITURE DUREE
                    sprintf(charDuree, "%d:0%d", m, s);
                else
                    sprintf(charDuree, "%d:%d", m, s);
                duree = TTF_RenderText_Solid(police, charDuree, couleurNoire);
                if(secondes > 9) // CORRECTION ECRITURE TEMPS RESTANT
                    sprintf(charTempsRestant, "%d:%d", minutes, secondes);
                else if(secondes <=9)
                    sprintf(charTempsRestant, "%d:0%d", minutes, secondes);
                tempsRestant = TTF_RenderText_Solid(police, charTempsRestant, couleurNoire);// CREATION SURFACE TEMPS RESTANT
            }
            else
                SDL_Delay(199 - (tempsActuel - tempsPrecedent));
        }
        else if(!isPlaying)
        {
            dispo[3][2] = PLAY;
            synchroniserDurees(&m, &s, &length, &positionMorceau, &minutes, &secondes);
        }
        // DETERMINATION VOLUME ET CORRECTION DE LA BARRE VOLUME
        FMOD_Channel_SetVolume(canal, volume); // J'ACTUALISE LE VOLUME
        FMOD_Channel_GetVolume(canal, &volume);
        volumeActuel = volume * TAILLE_BARRE_VOLUME;
        if(volumeActuel > TAILLE_BARRE_VOLUME - indVolume->w) // CORRECTION DE LA BARRE DU VOLUME
            volumeActuel = TAILLE_BARRE_VOLUME - indVolume->w;
        FMOD_Channel_SetVolume(canal, volume);
        // DETERMINATION SURFACE VOLUME EN FCT DU VOLUME
        if(!volume)
        {
            SDL_FreeSurface(sVolume);
            sVolume = IMG_Load("volume.png");
        }
        else if(volume < 0.3)
        {
            SDL_FreeSurface(sVolume);
            sVolume = IMG_Load("volume_bas.png");
        }
        else if(volume < 0.65)
        {
            SDL_FreeSurface(sVolume);
            sVolume = IMG_Load("volume_moyen.png");
        }
        else
        {
            SDL_FreeSurface(sVolume);
            sVolume = IMG_Load("volume_total.png");
        }

         //DETERMINER CHANSON SUIVANTE A LA FIN DU SON ACTUEL

         if(positionMorceau >= length - 199)
        {
            if(numeroMorceau == 4 && repet == 0)
            {
                FMOD_Channel_Stop(canal);
            }
            else
            {
                SDL_Delay(250);
                playNext = 0;
                commande = NEXT;
                FMOD_Channel_Stop(canal);
                playNext = determinerMorceau(&numeroMorceau, &repet, &commande, &lastSong, sonActuel);
                if(!chargerMusique(&system, &musique, &numeroMorceau, emplacementMorceau, &lastSong))
                {
                    fprintf(stderr, "Erreur de lecture du morceau : %s", emplacementMorceau);
                    exit(EXIT_FAILURE);
                }
                if(playNext)
                    FMOD_System_PlaySound(system, musique, 0, 0, &canal);
                else
                {
                    FMOD_Channel_Stop(canal);
                    FMOD_Sound_Release(musique);
                }
                FMOD_Sound_GetLength(musique, &length, FMOD_TIMEUNIT_MS);
                synchroniserDurees(&m, &s, &length, &positionMorceau, &minutes, &secondes);
                dispo[3][2] = PAUSE;
            }
        }
        // CORRECTION (imparfaite) MODE MUET QUAND STOPPE
        if(mute)
        {
            FMOD_Channel_SetMute(canal, 0);
            muet = 0;
        }
        else
        {
            FMOD_Channel_SetMute(canal, 1);
            muet = 1;
        }

        SDL_FillRect(ecran, NULL, SDL_MapRGB(ecran->format, 255, 255, 255));
        for(i=0; i<NB_BLOCS_HAUTEUR; i++)
        {
            for(j=0; j<NB_BLOCS_LARGEUR; j++)
            {
                position.x = j * TAILLE_BLOC;
                position.y = i * TAILLE_BLOC;
                switch(dispo[i][j])
                {
                    case PLAY:
                        SDL_BlitSurface(play, NULL, ecran, &position);
                        break;
                    case PAUSE:
                        SDL_BlitSurface(pause, NULL, ecran, &position);
                        break;
                    case FORWARD:
                        SDL_BlitSurface(forward, NULL, ecran, &position);
                        break;
                    case BACKWARD:
                        SDL_BlitSurface(backward, NULL, ecran, &position);
                        break;
                    case STOP:
                        SDL_BlitSurface(stop, NULL, ecran, &position);
                        break;
                    case REPLAY:
                        SDL_BlitSurface(replay, NULL, ecran, &position);
                        break;
                    case REPLAY_OFF:
                        SDL_BlitSurface(replay_OFF, NULL, ecran, &position);
                        break;
                    case REPLAY_ONE:
                        SDL_BlitSurface(replay_ONE, NULL, ecran, &position);
                        break;
                    case DUREE:
                        position.x = j * TAILLE_BLOC;
                        position.y = i * TAILLE_BLOC + TAILLE_BLOC / 4;
                        SDL_BlitSurface(duree, NULL, ecran, &position);
                        break;
                    case TEMPS_RESTANT:
                        position.x = j * TAILLE_BLOC + 3;
                        position.y = i * TAILLE_BLOC + TAILLE_BLOC / 4;
                        SDL_BlitSurface(tempsRestant, NULL, ecran, &position);
                        break;
                    case TITLE:
                        SDL_BlitSurface(titre, NULL, ecran, &position);
                        break;
                    case BARRE:
                        for(k=0; k<NB_BLOCS_BIS_LARGEUR; k++)
                        {
                            position.x = TAILLE_BLOC + k*TAILLE_BLOC_BIS;
                            position.y = i * TAILLE_BLOC + TAILLE_BLOC / 3 + 10;
                            if(progression[0][k] == PLAYED)
                                SDL_BlitSurface(played, NULL, ecran, &position);
                            else if(progression[0][k] == TOPLAY)
                                SDL_BlitSurface(toplay, NULL, ecran, &position);
                        }
                        break;

                    case VOLUME:
                        SDL_BlitSurface(verif, NULL, ecran, &position);
                        position.x = POSITION_VOLUME_X;
                        position.y = POSITION_VOLUME_Y;
                        if(muet)
                            SDL_BlitSurface(sMuet, NULL, ecran, &position);
                        else if(!muet)
                            SDL_BlitSurface(sVolume, NULL, ecran, &position);
                        break;
                    case BARRE_VOLUME:
                        for(i=0; i<volumeActuel; i++)
                        {
                            position.x = P_BARRE_VOLUME_X + i;
                            position.y = P_BARRE_VOLUME_Y;
                            SDL_FillRect(barreVolume[i], NULL, SDL_MapRGB(ecran->format, 0, 0, 255));
                            SDL_BlitSurface(barreVolume[i], NULL, ecran, &position);
                        }
                        for(i=volumeActuel; i<TAILLE_BARRE_VOLUME; i++)
                        {
                            pos.x = P_BARRE_VOLUME_X + i;
                            pos.y = P_BARRE_VOLUME_Y;
                            SDL_FillRect(barreVolume[i], NULL, SDL_MapRGB(ecran->format, 200, 200, 200));
                            SDL_BlitSurface(barreVolume[i], NULL, ecran, &pos);
                        }
                        position.x = P_BARRE_VOLUME_X + volumeActuel;
                        position.y = P_BARRE_VOLUME_Y - MARGE;
                        SDL_BlitSurface(indVolume, NULL, ecran, &position);
                        break;
                }
            }
        }
        SDL_BlitSurface(indicateur, NULL, ecran, &positionIndicateur);
        SDL_Flip(ecran);
    }


    TTF_CloseFont(police);
    TTF_Quit();
    SDL_FreeSurface(titre);
    SDL_FreeSurface(play);
    SDL_FreeSurface(pause);
    SDL_FreeSurface(forward);
    SDL_FreeSurface(backward);
    SDL_FreeSurface(played);
    SDL_FreeSurface(toplay);
    SDL_FreeSurface(replay);
    SDL_FreeSurface(replay_OFF);
    SDL_FreeSurface(duree);
    SDL_FreeSurface(tempsRestant);
    SDL_FreeSurface(verif);
    SDL_FreeSurface(indicateur);
    SDL_FreeSurface(sVolume);
    SDL_FreeSurface(sMuet);
    SDL_FreeSurface(replay_ONE);
    for(i=0; i<TAILLE_BARRE_VOLUME; i++)
        SDL_FreeSurface(barreVolume[i]);
    SDL_FreeSurface(indVolume);
    SDL_Quit();
    FMOD_ChannelGroup_Stop(canaux);
    FMOD_Channel_Stop(canal);

    FMOD_Sound_Release(musique);
    FMOD_System_Close(system);
    FMOD_System_Release(system);
    return 0;
}
