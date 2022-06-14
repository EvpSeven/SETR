/** \file main.c
 * 	\brief Program implementing PI Controller
 * 
 * \author André Brandão
 * \author Emanuel Pereira
 * \date 03/06/2022
 */
 
#include <zephyr.h>
#include <device.h>
#include <drivers/gpio.h>
#include <drivers/pwm.h>
#include <sys/printk.h>
#include <sys/__assert.h>
#include <string.h>
#include <timing/timing.h>
#include <stdlib.h>
#include <stdio.h>
 
 /* import PI_COntroller file */
#include <PI_Controller.h>

 // Proportional Integral controller - Struct 
typedef struct PI {
    double error;   // Error Signal
    double up;      // Proportional Component    
    double ui;      // Integral Component
    double Kp;      // Constant of the Proportional part of the controller
    double Ti;      // Constant of the Integral part of the controller
    int ULow;       // Integral LOW Limit
    int UHigh;      // Integral HIGH Limit
} PI;

PI pi;  // PI controller definition

// Function to initialze PI Controller Parameters
void PI_init(int Kp, int Ti)
{
    pi.error = 0;   // Error Component
    pi.up = 0;      // Proportional Component    
    pi.ui = 0;      // Integral Component
    pi.Kp = Kp;     // Constant of the Proportional part of the controller
    pi.Ti = Ti;     // Constant of the Integral part of the controller
    pi.ULow = -14;  // Integral LOW Limit
    pi.UHigh = 14;  // Integral HIGH Limit
}

double PI_controller(int ref, int y, double dutycycle)
{
    // Compute error
    pi.error = ref - y;
    
    pi.up = pi.error * pi.Kp;    // Proportional part of the controller
    
    pi.ui += pi.error * pi.Ti;   // Integral part of the controller
    
    // Integral limits
   if (pi.ui > pi.UHigh)
        pi.ui = pi.UHigh;

    else if (pi.ui < pi.ULow)
        pi.ui = pi.ULow;
    
    // Actuate on the duty-cycle
    dutycycle = dutycycle + pi.up + pi.ui; 
    
    // Duty-cycle limit
    if(dutycycle > 100)
        dutycycle = 100;

    else if(dutycycle < 0)
        dutycycle = 0;

    return dutycycle;
}
