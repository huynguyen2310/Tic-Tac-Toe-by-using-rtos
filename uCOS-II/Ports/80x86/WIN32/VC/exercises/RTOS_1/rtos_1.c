/*
*********************************************************************************************************
*                                                uC/OS-II
*                                          The Real-Time Kernel
*
*					WIN32 PORT & LINUX PORT
*                          (c) Copyright 2004, Werner.Zimmermann@fht-esslingen.de
*                 (Similar to Example 1 of the 80x86 Real Mode port by Jean J. Labrosse)
*                                           All Rights Reserved
** *****************************************************************************************************
*		Further modified by mikael.jakas@puv.fi, jukka.matila@vamk.fi
* *****************************************************************************************************
*                                               EXAMPLE #1
*********************************************************************************************************
*/

#include "includes.h"

/*
*********************************************************************************************************
*                                               CONSTANTS
*********************************************************************************************************
*/

#define  TASK_STK_SIZE                 512       /* Size of each task's stacks (# of WORDs)            */
#define  N_TASKS                        3       /* Number of identical tasks                          */
#define PLAYER_1 0x01
#define PLAYER_2 0x02
#define DRAW 0x03

/*
*********************************************************************************************************
*                                               VARIABLES
*********************************************************************************************************
*/

OS_STK        TaskStk[N_TASKS][TASK_STK_SIZE];        /* Tasks stacks                                  */
OS_STK        TaskStartStk[TASK_STK_SIZE];
//OS_EVENT* RandomSem;
//OS_EVENT* Task_Sem;
OS_FLAG_GRP* GameLogic;
/*
*********************************************************************************************************
*                                           FUNCTION PROTOTYPES
*********************************************************************************************************
*/

void TaskStart(void* data);        /* Function prototype of Startup task           */
void MatrixBoard(void* data);           /*3x3 for 2 player*/
void Player1(void* data);
void Player2(void* data);
int CheckResult(char square_default[9]);

/*
*********************************************************************************************************
*                                                MAIN
*********************************************************************************************************
*/

int  main(void)
{
    PC_DispClrScr(DISP_FGND_WHITE + DISP_BGND_BLACK);      /* Clear the screen                         */

    OSInit();                                              /* Initialize uC/OS-II                      */

    OSTaskCreate(TaskStart, (void*)0, &TaskStartStk[TASK_STK_SIZE - 1], 0);
    //Task_Sem =  OSSemCreate(1);							   /* Create a semaphore to protect a shared resource 
                //											from other tasks while being used			*/
    OSStart();                                             /* Start multitasking                       */

    return 0;
}


/*
*********************************************************************************************************
*                                              STARTUP TASK
*********************************************************************************************************
*/
void  TaskStart(void* pdata)
{
    INT16U key;
    INT8U symbol;
    INT8U err;
    pdata = pdata;                                         /* Prevent compiler warning                 */
    symbol = 'X';


    /*char stringA[20] = "Hi, Task A";
    char stringB[20] = "Hi, Task B";*/
    GameLogic = OSFlagCreate(0, &err);
    OSTaskCreate(MatrixBoard, (void*)&symbol, (void*)&TaskStk[0][TASK_STK_SIZE - 1], 1);
    OSTaskCreate(Player1, (void*)&symbol, (void*)&TaskStk[0][TASK_STK_SIZE - 1], 2);
    OSTaskCreate(Player2, (void*)&symbol, (void*)&TaskStk[1][TASK_STK_SIZE - 1], 3);

    for (;;) {

        if (PC_GetKey(&key) == TRUE) {                     /* See if key has been pressed              */
            if (key == 0x1B) {                             /* Yes, see if it's the ESCAPE key          */
                exit(0);  	                           /* End program                              */
            }
        }
        OSTimeDlyHMSM(0, 0, 1, 0);                         /* Wait one second                          */
    }
}

char square_default[9] = { '.','.', '.', '.', '.', '.', '.', '.', '.' };
void MatrixBoard(void* data) {
    INT8U err;
    char row1[3];              //The first row of matrix
    char row2[3];              //The second row of matrix
    char row3[3];              //The third row of matrix
    char square_change[9];
    int return_value = 0;       //Check the result
    for (;;) {
        OSFlagPend(GameLogic, DRAW, OS_FLAG_WAIT_CLR_ANY, 0, &err);

        for (int i = 0; i < 9; i++) {
            square_change[i] = square_default[i];           //get the value X or O
        }
        PC_DispClrScr(DISP_FGND_WHITE + DISP_BGND_BLACK);
        PC_DispStr(15, 6, (INT8U*)"Player1: X | Player2: 0", DISP_FGND_BLACK + DISP_BGND_WHITE);
        sprintf(row1, " %c | %c | %c ", square_change[0], square_change[1], square_change[2]);
        PC_DispStr(21, 7, (INT8U*)row1, DISP_FGND_BLACK + DISP_BGND_WHITE);
        PC_DispStr(21, 8, (INT8U*)"___________", DISP_FGND_BLACK + DISP_BGND_WHITE);
        sprintf(row2, " %c | %c | %c ", square_change[3], square_change[4], square_change[5]);
        PC_DispStr(21, 9, (INT8U*)row2, DISP_FGND_BLACK + DISP_BGND_WHITE);
        PC_DispStr(21, 10, (INT8U*)"___________", DISP_FGND_BLACK + DISP_BGND_WHITE);
        sprintf(row3, " %c | %c | %c ", square_change[6], square_change[7], square_change[8]);
        PC_DispStr(21, 11, (INT8U*)row3, DISP_FGND_BLACK + DISP_BGND_WHITE);
        return_value = CheckResult(square_change);

        if (return_value == -1) {
            OSFlagPost(GameLogic, PLAYER_1, OS_FLAG_SET, &err); //back to player1 function
            OSTimeDlyHMSM(0, 0, 1, 0);
        }
        else if (return_value == 1) {
            PC_DispStr(24, 12, (INT8U*)"X win", DISP_FGND_BLACK + DISP_BGND_WHITE);
        }
        else if (return_value == 2) {
            PC_DispStr(24, 13, (INT8U*)"O win", DISP_FGND_BLACK + DISP_BGND_WHITE);
        }
        else PC_DispStr(24, 14, (INT8U*)"Draw", DISP_FGND_BLACK + DISP_BGND_WHITE);
    }
}
void Player1(void* data) {
    INT8U err;
    srand(time(NULL));
    int position;
    for (;;) {
        OSFlagPend(GameLogic, PLAYER_1, OS_FLAG_WAIT_SET_ANY + OS_FLAG_CONSUME, 0, &err);
        position = rand() % 9;
        while (square_default[position] != '.') {
            position = rand() % 9;
        }
        square_default[position] = 'X';
        OSFlagPost(GameLogic, PLAYER_2, OS_FLAG_SET, &err);
        OSTimeDlyHMSM(0, 0, 1, 0);                         /* Wait one second                          */
    }
}
void Player2(void* data) {
    INT8U err;
    srand(time(NULL));
    int position;
    for (;;) {
        OSFlagPend(GameLogic, PLAYER_2, OS_FLAG_WAIT_SET_ANY + OS_FLAG_CONSUME, 0, &err);
        position = rand() % 9;
        while (square_default[position] != '.') {
            position = rand() % 9;
        }
        square_default[position] = 'O';
        OSFlagPost(GameLogic, DRAW, OS_FLAG_SET, &err); //return to matrix board
        OSTimeDlyHMSM(0, 0, 1, 0);                         /* Wait one second                          */
    }
}
int CheckResult(char square_default[9]) {
    //Check draw
     if (square_default[0] != '.' && square_default[1] != '.' && square_default[2] != '.' && square_default[3] != '.' && square_default[4] != '.'
     && square_default[5] != '.' && square_default[6] != '.' && square_default[7] != '.' && square_default[8] != '.')
     return 4;
    //Check row line
    else if (square_default[0] == square_default[1] && square_default[1] == square_default[2] && square_default[2] != '.') {
        if (square_default[2] == 'X') { return 1; }
        else return 2;
    }
    else if (square_default[3] == square_default[4] && square_default[4] == square_default[5] && square_default[5] != '.') {
        if (square_default[5] == 'X') { return 1; }
        else return 2;
    }
    else if (square_default[6] == square_default[7] && square_default[7] == square_default[8] && square_default[8] != '.') {
        if (square_default[8] == 'X') { return 1; }
        else return 2;
    }
    //Check column line
    else if (square_default[0] == square_default[3] && square_default[3] == square_default[6] && square_default[6] != '.') {
        if (square_default[6] == 'X') { return 1; }
        else return 2;
    }
    else if (square_default[1] == square_default[4] && square_default[4] == square_default[7] && square_default[7] != '.') {
        if (square_default[7] == 'X') { return 1; }
        else return 2;
    }
    else if (square_default[2] == square_default[5] && square_default[5] == square_default[8] && square_default[8] != '.') {
        if (square_default[8] == 'X') { return 1; }
        else return 2;
    }
    //Check diagonal line
    else if (square_default[0] == square_default[4] && square_default[4] == square_default[8] && square_default[8] != '.') {
        if (square_default[8] == 'X') { return 1; }
        else return 2;
    }
    else if (square_default[2] == square_default[4] && square_default[4] == square_default[6] && square_default[6] != '.') {
        if (square_default[6] == 'X') { return 1; }
        else return 2;
    }
    //Board is clear
    else return -1;
}
/*
*********************************************************************************************************
*                                                  TASKS
*********************************************************************************************************
*/

