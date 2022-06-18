/** \file PI_controller.c
 * 	\brief Module implementing PI Controller
 * 
 *  \author André Brandão
 *  \author Emanuel Pereira
 *  \date 15/06/2022
 */

#include <PI_controller.h>

/** Proportional Integral controller - Struct */
typedef struct PI {
    float error;   /**< Error Signal */
    float up;      /**< Proportional Component */
    float ui;      /**< Integral Component */
    float Kp;      /**< Constant of the Proportional part of the controller */
    float Ti;      /**< Constant of the Integral part of the controller */
    int ULow;      /**< Integral LOW Limit */
    int UHigh;     /**< Integral HIGH Limit */
} PI;

PI pi;  /**< PI controller variable */

/** \brief Function to initialze PI Controller
 *  
 *  This function initializes the parameters of the PI controller,
 * the proportional and the integral gain with the values passed in
 * the arguments and the signals with zero. The integral limits are
 * initialized with -14 and 14, may be changed for better control
 * behavior in other systems.
 * 
 *  \param[in] Kp Proportional Gain
 *  \param[in] Ti Integral Gain
 */
void PI_init(float Kp, float Ti)
{
    pi.error = 0;   // Error Component
    pi.up = 0;      // Proportional Component    
    pi.ui = 0;      // Integral Component
    pi.Kp = Kp;     // Constant of the Proportional part of the controller
    pi.Ti = Ti;     // Constant of the Integral part of the controller
    pi.ULow = -14;  // Integral LOW Limit
    pi.UHigh = 14;  // Integral HIGH Limit
}

/** \brief PI Controller control function
 *  
 *  This function the control function of the controller.
 * It must be passed, for a given instant (k-1), the reference point,
 * the system output, and the control signal value (dutycycle).
 *  Based on this parameters computes and returns the control signal.
 *  The control signal is considered as the dutycycle of a PWM signal therefor 
 * is limited within the interval [0 100]. 
 * 
 *  \param[in] ref Reference point (instant k-1)
 *  \param[in] y System output (instant k-1)
 *  \param[in] dutycycle control signal (instant k-1)
 *
 *  \return control signal (dutycycle) on instant k
 */
int PI_controller(int ref, int y, int dutycycle)
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
