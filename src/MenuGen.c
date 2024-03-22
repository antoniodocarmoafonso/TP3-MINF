// Tp3  manipulation MenuGen avec PEC12
// C. HUBER  10/02/2015 pour SLO2 2014-2015
// Fichier MenuGen.c
// Gestion du menu  du g�n�rateur
// Traitement cyclique � 10 ms

#include <stdint.h>                   
#include <stdbool.h>
#include "MenuGen.h"
#include "Mc32DriverLcd.h"
#include "GesPec12.h"
#include "Mc32NVMUtil.h"
#include "Generateur.h"
#include "Mc32Debounce.h"
#include "bsp.h"
#include "DefMenuGen.h"

/**
 * Function name :MENU_Initialize
 * @author Antonio Do Carmo & Cyril Feliciano 
 * @date 21.03.2024
 *
 * @brief Initialise les param�tres du programme en lisant les donn�es depuis la m�moire flash et affiche un message d'initialisation sur un �cran LCD.
 * 
 * Cette fonction initialise les param�tres du programme en lisant les donn�es depuis la m�moire flash. Elle affiche un message d'initialisation sur 
 * l'�cran LCD pour indiquer le d�marrage du programme et informer l'utilisateur sur l'origine des donn�es charg�es (m�moire flash ou valeurs par d�faut).
 * 
 * @param pParam Pointeur vers la structure S_ParamGen pour stocker les param�tres initialis�s.
 * 
 * @return  La structure S_ParamGen contenant les param�tres initialis�s.
 */

S_ParamGen MENU_Initialize(S_ParamGen *pParam)
{
    //D�claration de la structure acceillant les valeurs lues dans la flash
    S_ParamGen structInter; 
    //Lecture des donn�es dans la flash et enregistrement dans la structure 
    NVM_ReadBlock((uint32_t*)&structInter, sizeof(S_ParamGen));
    //Affichage de l'initialisation du programme 
    lcd_gotoxy(1,1);
    printf_lcd("TP3 GenSig 23-24");
    lcd_gotoxy(1,2);
    printf_lcd("Feliciano");
    lcd_gotoxy(1,3);
    printf_lcd("Do Carmo");
    //Si des donn�es sont pr�sentes dans la flash nous affichons "Donn�es en m�moire", sinon "Donn�es par d�faut"
    if (structInter.Magic == MAGIC)
    {
        lcd_gotoxy(1,4);
        printf_lcd("Donnees en memoire");
    }
    else
    {
        lcd_gotoxy(1,4);
        printf_lcd("Donnees par defaut");
    } 
    //Retour de la structure contenant les donn�es lues dans la flash
    return structInter;
}

/**
 * Function name :MENU_Execute
 * @author Antonio Do Carmo & Cyril Feliciano 
 * @date 21.03.2024
 *
 * @brief Permet � l'utilisateur de s�lectionner, ajuster et enregistrer les valeurs des param�tres via un �cran LCD et des boutons.
 * 
 * Cette fonction g�re l'interface utilisateur permettant de naviguer entre les diff�rents param�tres, d'ajuster leurs valeurs � l'aide
 * d'un encodeur rotatif et de les enregistrer dans la m�moire flash. Elle utilise des indications visuelles sur l'�cran LCD pour guider 
 * l'utilisateur dans le processus de s�lection, d'ajustement et d'enregistrement des valeurs des param�tres.
 * 
 * @param pParam Pointeur vers la structure S_ParamGen contenant les param�tres � ajuster et enregistrer.
 * 
 * @return Aucune valeur de retour.
 */

void MENU_Execute(S_ParamGen *pParam)
{
    //D�claration des variables et des structures internes � la fonction 
    static int8_t menuCounter = MENUCOUNTERMIN;     //Variable de comptage pour la position du curseur dans le menu
    static int8_t menuMode = SELECTMODE;        //Variable de l'�tat dans lequel se trouve le menu (selection du mode ou s�lection de la valeur)
    static int8_t isSaved = NOSAVEMODE;     //Variable d�terminant si l'utilisateur est en train d'enregistrer les valeurs ou non
    static uint8_t firstTime = true;        //Variable servant � savoir si cette fonction est appel�e pour le premi�re fois ou non
    static uint8_t lineClearedLcd;     //Variable de comptage pour l'effacement de l'affichage du LCD lors du premier appel de la fonction
    static S_ParamGen NewValue;     //Structure contenant les valeurs interm�diaires de pParam avant que l'utilisateur ait appuy� sur OK
    
    //Premi�re fois que la fonction est appel�e?
    if(firstTime == true)
    {
        //Boucle permettant d'effacer l'affichage du LCD
        for(lineClearedLcd = 1; lineClearedLcd < 5; lineClearedLcd++)
        {
            lcd_ClearLine(lineClearedLcd);
        }
        //Lors du premier appel de la fonction NewValue reprend les valeurs de pParam
        NewValue = *pParam;
        //Appel de la fonction d'affichage du LCD permettant � l'utilisateur de choisir le param�tre � modifier
        MENU_SelectMode(pParam, MODEFORME);
        //Toggle de la valeur de firstTime afin que le programme ne rentre plus dans ce "if"
        firstTime = false;
    }
    //Appel de la fonction de debounce pour le bouton de sauvegarde S9
    DoDebounce(&DescrS9,S_OK);
    
    //Est-ce que S9 a �t� press�?
    if(DebounceIsPressed(&DescrS9))
    {
        //Nous alluomons la backlight du LCD au cas ou celle-ci serait �teinte (en cas d'inactivit�)
        lcd_bl_on(); 
        //Indication d'une activit� de l'utilisateur
        Pec12.NoActivity = false;
        Pec12.InactivityDuration = 0;
        //Est-ce que l'utilisateur est d�j� dans le mode de sauvegarde des donn�es?
        if(isSaved == NOSAVEMODE)
        {
            //Remise � z�ro du flag du bouton S9
            DebounceClearPressed(&DescrS9);
            //Boucle permettant d'effacer l'affichage du LCD
            for(lineClearedLcd = 1; lineClearedLcd < 5; lineClearedLcd++)
            {
                lcd_ClearLine(lineClearedLcd);
            }
            //Affichage du mode de sauvegarde du syst�me 
            lcd_gotoxy(1,2);
            printf_lcd("    Sauvegarde ?");
            lcd_gotoxy(1,3);
            printf_lcd("    (appui long)");
            //Le programme passe en mode de sauvegarde des donn�es
            isSaved = SAVEMODE;
        }
    }
   //Est-ce que l'utilisateur n'est pas dans le mode de sauvegarde des donn�es?
    if(isSaved == NOSAVEMODE)
    {
        //Est ce l'utilisateur est dans le mode de s�lection du param�tre?
        if(menuMode == SELECTMODE)
        {
            //Est ce que l'utilisateur incr�mente le param�tre � l'aide du PEC12?
            if(Pec12IsPlus())
            {
                //Remise � z�ro du flag de l'incr�mentation du PEC12
                Pec12ClearPlus();
                //Est-ce que le curseur est au bout du menu?
                if(menuCounter < MENUCOUNTERMAX)
                { 
                    //Incr�mentation du param�tre choisi
                    menuCounter ++;
                }

                else
                { 
                    //Rebouclage du param�tre choisi
                    menuCounter = MENUCOUNTERMIN;
                }
                //Appel de la fonction d'affichage du LCD permettant � l'utilisateur de choisir le param�tre � modifier
                MENU_SelectMode(&GenParam, menuCounter);
            }
            //Est ce que l'utilisateur d�cr�mente le param�tre � l'aide du PEC12?
            else if(Pec12IsMinus())
            {
                //Remise � z�ro du flag de la d�cr�mentation du PEC12
                Pec12ClearMinus();
                //Est-ce que le curseur est au bout du menu?
                if(menuCounter > MENUCOUNTERMIN)
                { 
                    //D�cr�mentation du param�tre choisi
                    menuCounter --;
                }

                else
                {   //Rebouclage du param�tre choisi
                    menuCounter = MENUCOUNTERMAX;
                }
                //Appel de la fonction d'affichage du LCD permettant � l'utilisateur de choisir le param�tre � modifier
                MENU_SelectMode(&GenParam, menuCounter);
            }

            //Est ce que l'utilisateur a appuy� sur OK?
            if(Pec12IsOK())
            {
                //Remise � z�ro du flag du OK
                Pec12ClearOK();
                //Enregistrement dans la structure OldValue des valeurs actuelles de pParam
                OldValue.Forme = pParam->Forme;
                OldValue.Frequence = pParam->Frequence;
                OldValue.Amplitude = pParam->Amplitude;
                OldValue.Offset = pParam->Offset;
                //Actualisation de la structure Newvalue en fonction des valeurs choisies par l'utilisateur
                NewValue = MENU_SelectValue(NewValue, menuCounter);
                //Changement du mode de fonctionnement du programme pour le mode de s�lection de la valeur
                menuMode = SELECTVALUE;
            }
        }
        //Est ce l'utilisateur est dans le mode de la valeur des param�tres?
        else if(menuMode == SELECTVALUE)
        {
            //Actualisation de la structure Newvalue en fonction des valeurs choisies par l'utilisateur
            NewValue = MENU_SelectValue(NewValue, menuCounter);
            //Est ce que l'utilisateur a appuy� sur OK (validation de la valeur regl�e)?
            if(Pec12IsOK())
            {
                //Remise � z�ro du flag du OK
                Pec12ClearOK();
                //Actualisation de la structure pParam avec la nouvelle valeur regl�e par l'utilisateur
                switch (menuCounter)
                {
                    case MODEFORME:
                        pParam->Forme = NewValue.Forme;
                        break;
                    case MODEFREQ:
                        pParam->Frequence = NewValue.Frequence;
                        break;
                    case MODEAMPL:
                        pParam->Amplitude = NewValue.Amplitude;
                        break;
                    case MODEOFFSET:
                        pParam->Offset = NewValue.Offset;
                        break;
                    default:
                        break;
                }
                //Appel de la fonction changeant l'amplitude la forme et l'offset du signal
                GENSIG_UpdateSignal(pParam);
                //Appel de la fonction changeant la fr�quence du signal
                GENSIG_UpdatePeriode(pParam);
                //Appel de la fonction d'affichage du LCD permettant � l'utilisateur de choisir le param�tre � modifier
                MENU_SelectMode(&GenParam, menuCounter);
                //Changement du mode de fonctionnement du programme pour le mode de s�lection du param�tre
                menuMode = SELECTMODE; 
            }
            //Est ce que l'utilisateur a appuy� sur OK (annulation de la valeur regl�e)?
            else if (Pec12IsESC())
            {
                //Remise � z�ro du flag du ESC
                Pec12ClearESC();
                //Effacement de la ligne du param�tre que l'utilisateur �tait en train de regler
                lcd_ClearLine(menuCounter);
                //pParam revient � sa OldValue et NewValue reprend la valeur de pParam
                switch (menuCounter)
                {
                    case MODEFORME:
                        pParam->Forme = OldValue.Forme;
                        NewValue.Forme = pParam->Forme;
                        break;
                    case MODEFREQ:
                        pParam->Frequence = OldValue.Frequence;
                        NewValue.Frequence = pParam->Frequence;
                        break;
                    case MODEAMPL:
                        pParam->Amplitude = OldValue.Amplitude;
                        NewValue.Amplitude = pParam->Amplitude;
                        break;
                    case MODEOFFSET:
                        pParam->Offset = OldValue.Offset;
                        NewValue.Offset = pParam->Offset;
                        break;
                    default:
                        break;
                }
                //Appel de la fonction d'affichage du LCD permettant � l'utilisateur de choisir le param�tre � modifier
                MENU_SelectMode(&GenParam, menuCounter);
                //Changement du mode de fonctionnement du programme pour le mode de s�lection du param�tre
                menuMode = SELECTMODE;
            }
        }
    }
    //Si l'utilisateur est d�j� dans le mode d'enregistrement des valeurs
    else
    {
        //Appel de la fonction enregistrant les valeurs dans la m�moire flash
        isSaved = MENU_SaveValues(&GenParam, menuCounter);
    }
}

/**
 * Function name :MENU_SelectMode
 * @author Antonio Do Carmo & Cyril Feliciano 
 * @date 21.03.2024
 *
 * @brief Affiche les param�tres actuels sur un �cran LCD avec une indication visuelle de la s�lection de mode.
 * 
 * Cette fonction affiche les valeurs actuelles des param�tres, notamment la forme, la fr�quence, 
 * l'amplitude et l'offset, sur un �cran LCD. Elle utilise un ast�risque (*) pour indiquer visuellement le mode 
 * s�lectionn� par l'utilisateur.
 * 
 * @param  pParam Pointeur vers la structure S_ParamGen contenant les param�tres actuels.
 * @param  selectModeMenuCounter Compteur de menu de s�lection pour indiquer visuellement le mode s�lectionn�.
 * 
 * @return  Aucune valeur de retour.
 */

void MENU_SelectMode(S_ParamGen *pParam, int8_t selectModeMenuCounter)
{
    //D�claration du tableau contanant les caract�res � afficher pour le param�tre "forme"
    static const char MenuShapes[4][21] = {"Sinus", "Triangle", "DentDeScie", "Carre"};
    
    //Actualisation de l'affichage du LCD
    lcd_gotoxy(1,1);
    printf_lcd(" Forme = %s", MenuShapes[pParam->Forme]);
    lcd_gotoxy(1,2);
    printf_lcd(" Freq [Hz] = %4d", pParam->Frequence);
    lcd_gotoxy(1,3);
    printf_lcd(" Ampl [mV] = %5d", pParam->Amplitude);
    lcd_gotoxy(1,4);
    printf_lcd(" Offset [mV] = %5d", pParam->Offset); 
    
    //Affichage de l'ast�risque � l'endroit s�lectionn� par l'utilisateur
    lcd_gotoxy(1,selectModeMenuCounter);
    printf_lcd("*");
}

/**
 * Function name :MENU_SelectValue
 * @author Antonio Do Carmo & Cyril Feliciano 
 * @date 21.03.2024
 *
 * @brief S�lectionne et ajuste les valeurs des param�tres � l'aide d'un encodeur rotatif (PEC12) et les affiche sur un �cran LCD.
 * 
 * Cette fonction permet � l'utilisateur de s�lectionner et d'ajuster les valeurs des param�tres � l'aide d'un encodeur rotatif. 
 * Elle g�re les diff�rents modes de s�lection pour chaque param�tre, notamment la forme, la fr�quence, l'amplitude et l'offset, 
 * en fonction du compteur de menu de s�lection. Les valeurs s�lectionn�es sont affich�es en temps r�el sur l'�cran LCD.
 * 
 * @param  selectValueNewValue Structure contenant les nouvelles valeurs des param�tres � ajuster.
 * @param  selectValueMenuCounter Compteur de menu de s�lection pour naviguer entre les diff�rents param�tres.
 * 
 * @return  La structure selectValueNewValue mise � jour avec les valeurs ajust�es.
 */

S_ParamGen MENU_SelectValue(S_ParamGen selectValueNewValue, int8_t selectValueMenuCounter)
{
    //D�claration du tableau contanant les caract�res � afficher pour le param�tre "forme"
    static const char MenuShapes[4][21] = {"Sinus", "Triangle", "DentDeScie", "Carre"};
    //Swicth case pour inr�menter ou d�cr�menter la valeur du param�tre s�lectionn�
    switch (selectValueMenuCounter)
        {
            case MODEFORME:
                //Est ce que l'utilisateur incr�mente le param�tre � l'aide du PEC12?
                if(Pec12IsPlus())
                {
                    //Remise � z�ro du flag de l'incr�mentation du PEC12
                    Pec12ClearPlus();
                    //Est ce que la valeur du param�tre est en dessous de sa valeur max?
                    if(selectValueNewValue.Forme < FORMEMAX)
                    {
                        //Incr�mentation de la forme
                        selectValueNewValue.Forme ++;
                    }
                    else
                    {
                        //Rebouclage de la forme
                        selectValueNewValue.Forme = FORMEMIN;
                    }
                    //Effacement de la ligne de la forme sur le LCD
                    lcd_ClearLine(1);
                }
                //Est ce que l'utilisateur d�cr�mente le param�tre � l'aide du PEC12?
                else if(Pec12IsMinus())
                {
                    //Remise � z�ro du flag de la d�cr�mentation du PEC12
                    Pec12ClearMinus();
                    //Est ce que la valeur du param�tre est en dessus de sa valeur max?
                    if(selectValueNewValue.Forme > FORMEMIN)
                    {
                        //D�cr�mentation de la forme
                        selectValueNewValue.Forme --;
                    }
                    else
                    {
                        //Rebouclage de la forme
                        selectValueNewValue.Forme = FORMEMAX;
                    }
                    //Effacement de la ligne de la forme sur le LCD
                    lcd_ClearLine(1);
                } 
                //Affichage de la valeur r�gl�e sur la ligne "forme"
                lcd_gotoxy(1,1);
                printf_lcd("?Forme = %s", MenuShapes[selectValueNewValue.Forme]);
                
            break;

            case MODEFREQ:
                //Est ce que l'utilisateur incr�mente le param�tre � l'aide du PEC12?
                if(Pec12IsPlus())
                {
                    //Remise � z�ro du flag de l'incr�mentation du PEC12
                    Pec12ClearPlus();
                    //Est ce que la valeur du param�tre est en dessous de sa valeur max?
                    if(selectValueNewValue.Frequence < FREQMAX)
                    {
                        //Incr�mentation de la fr�quence
                        selectValueNewValue.Frequence = selectValueNewValue.Frequence + FREQSTEP;
                    }
                    else
                    {
                        //Rebouclage de la fr�quence 
                        selectValueNewValue.Frequence = FREQMIN;
                    }
                    //Effacement de la ligne de la fr�quence sur le LCD
                    lcd_ClearLine(2);
                }
                //Est ce que l'utilisateur d�cr�mente le param�tre � l'aide du PEC12?
                else if(Pec12IsMinus())
                {
                    //Remise � z�ro du flag de la d�cr�mentation du PEC12
                    Pec12ClearMinus();
                    //Est ce que la valeur du param�tre est en dessus de sa valeur max?
                    if(selectValueNewValue.Frequence > FREQMIN)
                    {
                        //D�cr�mentation de la fr�quence
                        selectValueNewValue.Frequence = selectValueNewValue.Frequence - FREQSTEP;
                    }
                    else
                    {
                        //Rebouclage de la fr�quence
                        selectValueNewValue.Frequence = FREQMAX;
                    }
                    //Effacement de la ligne de la fr�quence sur le LCD
                    lcd_ClearLine(2);
                }
                //Affichage de la valeur r�gl�e sur la ligne "fr�quence"
                lcd_gotoxy(1,2);
                printf_lcd("?Freq [Hz] = %4d", selectValueNewValue.Frequence);
            break;

            case MODEAMPL:
                //Est ce que l'utilisateur incr�mente le param�tre � l'aide du PEC12?
                if(Pec12IsPlus())
                {
                    //Remise � z�ro du flag de l'incr�mentation du PEC12
                    Pec12ClearPlus();
                    //Est ce que la valeur du param�tre est en dessous de sa valeur max?
                    if(selectValueNewValue.Amplitude < AMPLMAX)
                    {
                        //Incr�mentation de l'amplitude
                        selectValueNewValue.Amplitude = selectValueNewValue.Amplitude + AMPLSTEP;
                    }
                    else
                    {
                        //Rebouclage de l'amplitude
                        selectValueNewValue.Amplitude = AMPLMIN;
                    }
                    //Effacement de la ligne de l'amplitude sur le LCD
                    lcd_ClearLine(3);
                }
                //Est ce que l'utilisateur d�cr�mente le param�tre � l'aide du PEC12?
                else if(Pec12IsMinus())
                {
                    //Remise � z�ro du flag de la d�cr�mentation du PEC12
                    Pec12ClearMinus();
                    //Est ce que la valeur du param�tre est en dessus de sa valeur max?
                    if(selectValueNewValue.Amplitude > AMPLMIN)
                    {
                        //D�cr�mentation de l'amplitude
                        selectValueNewValue.Amplitude = selectValueNewValue.Amplitude - AMPLSTEP;
                    }
                    else
                    {
                        //Rebouclage de l'amplitude
                        selectValueNewValue.Amplitude = AMPLMAX;
                    }
                    //Effacement de la ligne de l'amplitude sur le LCD
                    lcd_ClearLine(3);
                }
                //Affichage de la valeur r�gl�e sur la ligne "amplitude"
                lcd_gotoxy(1,3);
                printf_lcd("?Ampl [mV] = %5d", selectValueNewValue.Amplitude);
            break;

            case MODEOFFSET:
                
                if(Pec12IsPlus() && (selectValueNewValue.Offset < OFFSETMAX))
                {
                    //Remise � z�ro du flag de l'incr�mentation du PEC12
                    Pec12ClearPlus();
                    //Incr�mentation de l'offset
                    selectValueNewValue.Offset = selectValueNewValue.Offset + OFFSETSTEP;
                    //Effacement de la ligne de l'offset sur le LCD
                    lcd_ClearLine(4);      
                }
                
                else if(Pec12IsMinus() && (selectValueNewValue.Offset > OFFSETMIN))
                {
                    //Remise � z�ro du flag de la d�cr�mentation du PEC12
                    Pec12ClearMinus();
                    //D�cr�mentation de l'offset
                    selectValueNewValue.Offset = selectValueNewValue.Offset - OFFSETSTEP;
                    //Effacement de la ligne de l'offset sur le LCD
                    lcd_ClearLine(4);      
                }
                //Affichage de la valeur r�gl�e sur la ligne "offset"
                lcd_gotoxy(1,4);
                printf_lcd("?Offset [mV] = %5d", selectValueNewValue.Offset );  
            break;
            
            default:
                break;

        }
    //Retour de la structure NewValue
    return(selectValueNewValue);
}

/**
 * Function name :MENU_SaveValues
 * @author Antonio Do Carmo & Cyril Feliciano 
 * @date 21.03.2024
 *
 * @brief Enregistre les valeurs des param�tres dans la m�moire non volatile (NVM) apr�s confirmation de l'utilisateur.
 * 
 * Cette fonction enregistre les valeurs des param�tres dans la NVM lorsque l'utilisateur confirme l'action en appuyant 
 * sur un bouton sp�cifique pendant une certaine dur�e. Elle fournit un retour visuel sur un �cran (LCD) pour les 
 * op�rations de sauvegarde r�ussies ou annul�es.
 * 
 * @param  pParam Pointeur vers la structure S_ParamGen contenant les valeurs des param�tres.
 * @param  saveValuesMenuCounter Compteur pour naviguer dans le menu des valeurs � sauvegarder.
 * 
 * @return �tat du mode de sauvegarde (SAVEMODE si en mode de sauvegarde, NOSAVEMODE sinon).
 */

int8_t MENU_SaveValues(S_ParamGen *pParam, int8_t saveValuesMenuCounter)
{
    //D�claration des variables internes � la fonction 
    static uint8_t saveCounter = 0;     //Variable comptant le temps durant lequel l'utilisateur appuie sur S9
    static uint8_t saveMode = false;        //Variable indiquant si l'utilisateur a choisi de sauvegarder les valeurs ou non
    static uint8_t saveDisplayCounter = 0;      //Variable comptant le temps durant lequel la confirmation de la sauvegarde doit �tre affich�
   
    //Si l'utilisateur n'a pas encore confirm� ou annul� la sauvegarde
    if(saveMode == false)
    {
        //Est-ce que S9 est press�?
        if(DebounceIsPressed(&DescrS9))
        {
            //Incr�mentation du compteur du temps de pression sur S9
            saveCounter++;
            //D�tection d'un flanc sur le bouton S9
            if(DebounceGetInput(&DescrS9))
            {
                //Remise � z�ro du flag du bouton S9
                DebounceClearPressed(&DescrS9);
                //Est-ce que le bouton S9 a �t� press� plus de 500ms?
                if(saveCounter >= SAVECOUNTERMAX)
                {
                    //Enregistrement des valeurs de pParam dans la m�moire flash
                    NVM_WriteBlock((uint32_t*)pParam, sizeof(S_ParamGen));
                    //Effacement des lignes d'affichage de la sauvegarde sur le LCD
                    lcd_ClearLine(2);
                    lcd_ClearLine(3);
                    //Remise � z�ro du compteur de pression sur S9
                    saveCounter = 0;
                    //Changement du mode de la sauvegarde afin d'afficher la confiamation de l'enregistrement des valeurs
                    saveMode = true;
                    //Affichage de la confirmation de la sauvegarde
                    lcd_gotoxy(1,2);
                    printf_lcd("    Sauvegarde OK"); 
                }
                else
                {
                    //Effacement des lignes d'affichage de la sauvegarde sur le LCD
                    lcd_ClearLine(2);
                    lcd_ClearLine(3);
                    //Remise � z�ro du compteur de pression sur S9
                    saveCounter = 0;
                    //Changement du mode de la sauvegarde afin d'afficher la l'annulation de l'enregistrement des valeurs
                    saveMode = true;
                    //Affichage de la confirmation de la sauvegarde
                    lcd_gotoxy(1,2);
                    printf_lcd(" Sauvegarde ANNULEE"); 
                }            
            } 
        }
        //Retour de l'�tat de l'�tat de la sauvegarde
        return(SAVEMODE);
    }
    else if((saveMode != false) && (saveDisplayCounter < SAVEDISPLAYTIME))
    {
        //Incr�mentation du compteur de l'affichage de la sauvagarde
        saveDisplayCounter++;
        //Retour de l'�tat de l'�tat de la sauvegarde
        return(SAVEMODE);
    }
    else
    {
        //Effacement de la ligne affichant si la valeur a �t� sauvegard�e ou non
        lcd_ClearLine(2);
        //Remise � z�ro du compteur de l'affichage de la sauvagarde
        saveDisplayCounter = 0;
        //Remise � z�ro du mode de la sauvegarde
        saveMode = false;
        //Appel de la fonction d'affichage du LCD permettant � l'utilisateur de choisir le param�tre � modifier
        MENU_SelectMode(&GenParam, saveValuesMenuCounter);
        //Retour de l'�tat de l'�tat de la sauvegarde
        return(NOSAVEMODE);
    }
}


