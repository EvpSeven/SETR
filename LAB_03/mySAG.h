/** \file mySAG.h 
 * \brief Module to manipulate stream of integers
 * 
 * \author André Brandão
 * \date 22/03/2022
 */

#ifndef _MYSAG_H
#define _MYSAG_H

/** Maximum Numbers of elements of the stream
 * \def MAXSIZE
*/
#define MAXSIZE 100

int MySAGInit(int);
void MySAGInsert(int);

int MySAGMax();
int MySAGMin();
int MySAGAvg();

int MySAGFreq(int);

#endif //_MYSAG_H