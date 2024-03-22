
/*******************************************************************************
  MPLAB Harmony Application Source File
  
  Company:
    Microchip Technology Inc.
  
  File Name:
    app.c

  Summary:
    This file contains the source code for the MPLAB Harmony application.

  Description:
    This file contains the source code for the MPLAB Harmony application.  It 
    implements the logic of the application's state machine and it may call 
    API routines of other MPLAB Harmony modules in the system, such as drivers,
    system services, and middleware.  However, it does not call any of the
    system interfaces (such as the "Initialize" and "Tasks" functions) of any of
    the modules in the system or make any assumptions about when those functions
    are called.  That is the responsibility of the configuration-specific system
    files.
 *******************************************************************************/

// DOM-IGNORE-BEGIN
/*******************************************************************************
Copyright (c) 2013-2014 released Microchip Technology Inc.  All rights reserved.

Microchip licenses to you the right to use, modify, copy and distribute
Software only when embedded on a Microchip microcontroller or digital signal
controller that is integrated into your product or third party product
(pursuant to the sublicense terms in the accompanying license agreement).

You should refer to the license agreement accompanying this Software for
additional information regarding your rights and obligations.

SOFTWARE AND DOCUMENTATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF
MERCHANTABILITY, TITLE, NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE.
IN NO EVENT SHALL MICROCHIP OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER
CONTRACT, NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR
OTHER LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE OR
CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT OF
SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
(INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.
 *******************************************************************************/
// DOM-IGNORE-END


// *****************************************************************************
// *****************************************************************************
// Section: Included Files 
// *****************************************************************************
// *****************************************************************************

#include "app.h"
#include "Mc32DriverLcd.h"
#include "Mc32gestSpiDac.h"
#include "GesPec12.h"
#include "MenuGen.h"
#include "Generateur.h"

// *****************************************************************************
// *****************************************************************************
// Section: Global Data Definitions
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/* Application Data

  Summary:
    Holds application data

  Description:
    This structure holds the application's data.

  Remarks:
    This structure should be initialized by the APP_Initialize function.
    
    Application strings and buffers are be defined outside this structure.
*/

APP_DATA appData;


// *****************************************************************************
// *****************************************************************************
// Section: Application Callback Functions
// *****************************************************************************
// *****************************************************************************

/* TODO:  Add any necessary callback functions.
*/

// *****************************************************************************
// *****************************************************************************
// Section: Application Local Functions
// *****************************************************************************
// *****************************************************************************

void APP_UpdateState(APP_STATES NewState)
{
    // Met � jour l'�tat de l'application avec la nouvelle valeur
    appData.state = NewState;
    
    // Aucune sortie explicite, car la mise � jour est effectu�e directement sur la variable d'�tat globale.
    // La fonction n'a pas de valeur de retour (void).
}

/* TODO:  Add any necessary local functions.
*/


// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    void APP_Initialize ( void )

  Remarks:
    See prototype in app.h.
 */

void APP_Initialize ( void )
{
    /* Place the App state machine in its initial state. */
    appData.state = APP_STATE_INIT;

    
    /* TODO: Initialize your application's state machine and other
     * parameters.
     */
}


/**
 * Function name :APP_Tasks
 * @author Antonio Do Carmo & Cyril Feliciano 
 * @date 21.03.2024
 *
 * @brief G�re les t�ches de l'application en fonction de son �tat actuel.
 * 
 * Cette fonction permet de g�rer les t�ches de l'application en fonction de son �tat actuel. 
 * Elle effectue diff�rentes actions selon l'�tat de l'application, telles que l'initialisation des p�riph�riques, 
 * l'ex�cution du menu, etc.
 * 
 * @return Aucune valeur de retour.
 */

void APP_Tasks ( void )
{
    
    S_ParamGen structInterAPP;    //D�claration de la variable accueillant temporairement les valeurs de pParam

    /* Check the application's current state. */
    switch ( appData.state )
    {
        /* Application's initial state. */
        case APP_STATE_INIT:
        {
            //Initialisation du LCD
            lcd_init();
            //Allumage de la backlight du LCD
            lcd_bl_on();
            //Initialisation du bus SPI
            SPI_InitLTC2604();
            //Appel de la fonction d'initialisation du PEC12
            Pec12Init();
            //Appel de la fonction d'initialisation du menu
            structInterAPP = MENU_Initialize(&GenParam);
            //Appel de la fonction d'initialisation de gensig
            GENSIG_Initialize(&GenParam,&structInterAPP);
            //Appel de la fonction changeant l'amplitude la forme et l'offset du signal
            GENSIG_UpdateSignal(&GenParam);
            //Appel de la fonction changeant la fr�quence du signal
            GENSIG_UpdatePeriode(&GenParam);
            //D�marrage des timers
            DRV_TMR0_Start();
            DRV_TMR1_Start();
            //Changement d'�tat de la machine d'�tat pour APP_STATE_WAIT
            APP_UpdateState(APP_STATE_WAIT);

            break;
        }
        //Etat d'ex�cution du programme
        case APP_STATE_SERVICE_TASKS:
        {
            //Appel de la fonction ex�cutant le menu
            MENU_Execute(&GenParam);
            //Changement d'�tat de la machine d'�tat pour APP_STATE_WAIT
            APP_UpdateState(APP_STATE_WAIT);
            break;
        }
        
        case APP_STATE_WAIT:
        { 
            //Etat d'attente de la machine d'�tat
            break;
        }

        /* TODO: implement your application state machine.*/
        

        /* The default state should never be executed. */
        default:
        {
            /* TODO: Handle error in application's state machine. */
            break;
        }
    }
}

 

/*******************************************************************************
 End of File
 */
