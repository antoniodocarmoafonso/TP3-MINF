// Canevas manipulation GenSig avec menu

// C. HUBER  09/02/2015

// Fichier Generateur.C

// Gestion  du g�n�rateur
 
// Pr�vu pour signal de 40 echantillons
 
// Migration sur PIC32 30.04.2014 C. Huber
 
 
#include "Generateur.h"

#include "DefMenuGen.h"

#include "Mc32gestSpiDac.h"

#include "app.h"

#include "Mc32NVMUtil.h"

#include "Mc32DriverLcd.h"

#include <math.h>

#include <stdint.h>

#include <stdbool.h>
 

static uint16_t formeSignal [MAX_ECH];
 
/**
 * Function name :GENSIG_Initialize
 * @author Antonio Do Carmo & Cyril Feliciano 
 * @date 21.03.2024
 *
 * @brief Initialise les param�tres du g�n�rateur de signal.
 * 
 * Cette fonction initialise les param�tres du g�n�rateur de 
 * signal en fonction des valeurs lues dans la m�moire flash. Si aucune valeur n'est trouv�e en m�moire,
 * elle initialise les param�tres avec des valeurs par d�faut.
 * 
 * @param pParam Pointeur vers la structure contenant les param�tres du g�n�rateur de signal.
 * @param structInterGENSIG Pointeur vers la structure contenant les valeurs lues dans la m�moire flash.
 * 
 * @return Aucune valeur de retour.
 */

void  GENSIG_Initialize(S_ParamGen *pParam, S_ParamGen *structInterGENSIG)
{
    //Est-ce que la valeur enregistr�e dans la flash correspond au champ magic?
    if (structInterGENSIG->Magic == MAGIC)
    {
        //Enregistrement des valeurs de la flash dans pParam
        *pParam = *structInterGENSIG;
    }
    else
    {
        //Initialisation des param�tres de base de la structure pParam
        pParam->Forme = SignalSinus;
        pParam->Frequence = 100;
        pParam->Amplitude = 10000;
        pParam->Offset = 0;
        pParam->Magic = MAGIC;  
    } 
}

/**
 * Function name :GENSIG_UpdatePeriode
 * @author Antonio Do Carmo & Cyril Feliciano 
 * @date 21.03.2024
 *
 * @brief Met � jour la p�riode du signal en fonction de la fr�quence.
 * 
 * Cette fonction met � jour la p�riode du signal en fonction de la fr�quence sp�cifi�e dans les param�tres du g�n�rateur de signal.
 * 
 * @param pParam Pointeur vers la structure contenant les param�tres du g�n�rateur de signal.
 * 
 * @return Aucune valeur de retour.
 */

void  GENSIG_UpdatePeriode(S_ParamGen *pParam)
{
    //D�claration de la variable stockant la conversion du nombre de pas de la fr�quence 
    static uint16_t nbrPasFrequence = false;
    //Conversion de la valeur de la fr�quence pour le timer 3
    nbrPasFrequence = ((TRANSFORMATION_FREQUENCE/pParam->Frequence)-1)+0.5;
    //Changement de la valeur de recharge du timer 3
    PLIB_TMR_Period16BitSet(TMR_ID_3, nbrPasFrequence);

}
 
/**
 * Function name :GENSIG_UpdateSignal
 * @author Antonio Do Carmo & Cyril Feliciano 
 * @date 21.03.2024
 *
 * @brief Met � jour la forme du signal en fonction des param�tres sp�cifi�s.
 * 
 * Cette fonction met � jour la forme du signal en fonction des param�tres sp�cifi�s tels que la forme, l'amplitude et l'offset.
 * 
 * @param pParam Pointeur vers la structure contenant les param�tres du g�n�rateur de signal.
 * 
 * @return Aucune valeur de retour.
 */

void GENSIG_UpdateSignal(S_ParamGen *pParam)
{
    //D�claration des variables et du tableau internes � la fonction 
    uint8_t indexEchantillons;
    int16_t Offset;
    uint16_t amplitude_en_mV;
    const uint16_t tbSinus[100] = {
    53, 56, 59, 62, 65, 68, 71, 74, 77, 79, 82, 84, 86, 89, 90, 92, 94, 95, 96, 98, 
    98, 99, 100, 100, 100, 100, 100, 99, 98, 98, 96, 95, 94, 92, 90, 89, 86, 
    84, 82, 79, 77, 74, 71, 68, 65, 62, 59, 56, 53, 50, 47, 44, 41, 38, 35, 
    32, 29, 26, 23, 21, 18, 16, 14, 11, 10, 8, 6, 5, 4, 2, 2, 1, 0, 0, 0, 0, 
    1, 1, 2, 2, 3, 4, 6, 8, 10, 11, 14, 16, 18, 21, 23, 26, 29, 32, 35, 38, 
    41, 44, 47, 50};
    //Multiplication de l'offset par -1 afin d'inverser l'effet de l'offset
    Offset = pParam->Offset * INVERSION;
    //Conversion de l'amplitude saisie par l'utilisateur en mV
    amplitude_en_mV = (pParam ->Amplitude / MAX_ECH) * DEUX ;
    
    //Boucle permettant de parcourir toutes les cases du tableau formeSignal
    for(indexEchantillons = 0; indexEchantillons < MAX_ECH; indexEchantillons++)
    {
        //Action selon la forme choisie
        switch (pParam->Forme) 
        {
            case SignalSinus:
                //Calcul des 100 �chantillons du sinus
                formeSignal[indexEchantillons] = (tbSinus[indexEchantillons] - MOITIE_ECH) * amplitude_en_mV + MOITIE_STEP_AMPLITUDE + Offset;
                break;

            case SignalCarre:
                if (indexEchantillons < MAX_ECH * DIV_PAR_DEUX)
                {
                    //Calcul des 50 �chantillons du signal carr� n�gatif
                    formeSignal[indexEchantillons] = MOITIE_STEP_AMPLITUDE - ((amplitude_en_mV * MAX_ECH)* DIV_PAR_DEUX - Offset);
                }
                else
                {
                    //Calcul des 50 �chantillons du signal carr� positif
                    formeSignal[indexEchantillons] = MOITIE_STEP_AMPLITUDE + ((amplitude_en_mV * MAX_ECH)* DIV_PAR_DEUX + Offset);
                }
                break;

            case SignalDentDeScie:
                //Calcul des 100 �chantillons du signal en dent de scie
                formeSignal[indexEchantillons] = Offset + (indexEchantillons - MOITIE_ECH) * amplitude_en_mV + MOITIE_STEP_AMPLITUDE;
                break;

            case SignalTriangle:
                if((MAX_ECH * DIV_PAR_DEUX) < indexEchantillons)
                {
                    //Calcul des 50 �chantillons du signal triangle positif
                    formeSignal[indexEchantillons] = amplitude_en_mV * (MAX_ECH - DEUX * (indexEchantillons - QUART_ECH)) + Offset + MOITIE_STEP_AMPLITUDE; 
                }
                else
                {
                    //Calcul des 50 �chantillons du signal triangle n�gatif
                    formeSignal[indexEchantillons] = amplitude_en_mV * (DEUX * (indexEchantillons - QUART_ECH)) + Offset + MOITIE_STEP_AMPLITUDE;
                }
                break;
                
            default:
                break;
        }
        //Est-ce que la valeur du signal d�passe l'amplitude maximum n�gative?
        if ((formeSignal[indexEchantillons] > MAX_STEP_AMPLITUDE) && (Offset < false)) 
        {
            //Valeur du signal bloqu�e � z�ro
            formeSignal[indexEchantillons] = false;
        }
        //Est-ce que la valeur du signal d�passe l'amplitude maximum positive?
        else if ((formeSignal[indexEchantillons] > MAX_STEP_AMPLITUDE) && (Offset > false))
        {
            //Valeur du signal bloqu�e � sa valeur maximum (20000)
            formeSignal[indexEchantillons] = MAX_STEP_AMPLITUDE;
        }
        //Conversion des valeurs pr�sente dans les cases du tableau pour le DAC
        formeSignal[indexEchantillons] = (MAXSTEP_DAC * formeSignal [indexEchantillons]/ MAX_STEP_AMPLITUDE);
    }
}
 
 /**
 * Function name :GENSIG_Execute
 * @author Antonio Do Carmo & Cyril Feliciano 
 * @date 21.03.2024
 *
 * @brief Ex�cute le g�n�rateur de signal en envoyant les �chantillons au DAC.
 * 
 * Cette fonction ex�cute le g�n�rateur de signal en envoyant les �chantillons stock�s dans le tableau formeSignal au DAC via le bus SPI.
 * 
 * @return Aucune valeur de retour.
 */

void  GENSIG_Execute(void)
{
   //D�claration de la variable permettant de connaitre l'�chantillon en cours de traitement
   static uint16_t EchNb = 0;
   //Ecriture de l'�chantillon dans le DAC via le bus SPI
   SPI_WriteToDac(0, formeSignal[EchNb] ); 
   //Incr�mentation de la variable permettant de connaitre l'�chantillon en cours de traitement
   EchNb++;
   //Remise � z�ro de EchNb quand celui-ci a atteint sa valeur maximale
   EchNb = EchNb % MAX_ECH;
}
