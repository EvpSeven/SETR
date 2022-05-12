/** \file main.c
 * 	\brief Program that implements the State Machine of a vending machine.
 *
 *  The program accepts a subset of coins and allows the user to browse through the available products,
 * buy one product and return the credit. The inputs are push-buttons and the output is done
 * via UART/Terminal. It was implemented using the board Nordic nrf52840-dk
 * 
 * \author André Brandão
 * \author Emanuel Pereira
 * \date 10/05/2022
 */

#include <zephyr.h>
#include <device.h>
#include <devicetree.h>
#include <drivers/gpio.h>
#include <sys/printk.h>
#include <sys/__assert.h>
#include <string.h>
#include <timing/timing.h>
#include <stdio.h>
#include <float.h>

#define NPRODUCTS 3 /**< Number of products available */
#define SLEEP_MS 250  /**< Sleep period (ms) */

// States of State Machine
#define WAIT 0              /**< State Wait definition */
#define UPDATE_CREDIT 1     /**< State Update Credit definition */
#define UPDATE_PRODUCT 2    /**< State Update Product definition */
#define CHECK_CREDIT 3      /**< State Check Credit definition */

// Panel Buttons Flags
#define NONE 0      /**< Flag that indicates no button was pressed */ 
#define UP 1        /**< Flag to signal Up button was pressed */
#define DOWN 2      /**< Flag to signal Down button was pressed */
#define SELECT 3    /**< Flag to signal Select button was pressed */
#define RETURN 4    /**< Flag to signal Return button was pressed */

/** GPIO 0 Node Label from device tree (refer to dts file) */
#define GPIO0_NID DT_NODELABEL(gpio0) 

// Address of Board buttons
#define BOARDBUT1 0xb   /**< Address of Board button 1 used for coin 10 cents */
#define BOARDBUT2 0xc   /**< Address of Board button 2 used for coin 20 cents */
#define BOARDBUT3 0x18  /**< Address of Board button 3 used for coin 50 cents */
#define BOARDBUT4 0x19  /**< Address of Board button 4 used for coin 1 euro */

// Address of GPIO of the external buttons
#define BUTUP 0x03      /**< Address of GPIO where button UP is connected */
#define BUTDOWN 0x04    /**< Address of GPIO where button DOWN is connected */
#define BUTSELECT 0x1c  /**< Address of GPIO where button SELECT is connected */
#define BUTRETURN 0x1d  /**< Address of GPIO where button RETURN is connected */

// Buttons callback structures
static struct gpio_callback butcoin_cb_data;
static struct gpio_callback butpanel_cb_data;


static char *products[NPRODUCTS] = {"Beer", "Tuna Sandwich", "Coffee"}; /**< Array to store the available products */
static float price[NPRODUCTS] = {1.5, 1.0, 0.5};    /**< Array to store price of the products, the order of the prices are on same order as the products on the products array */

volatile float coin = 0;            /**< Value of coin inserted */
volatile int coin_detected = 0;     /**< Flag to signal that a coin was inserted */
volatile int button_pressed = NONE; /**< Flag which indicates which panel button was pressed */

void input_output_config(void);
void float2int(float,int*,int*);

/** \brief Callback function of the interrupt from the four board buttons
 * 
 *  Interrupt function of four buttons that emulates the insertion of coins,
 * whose values are: 10 cents, 20 cents, 50 cents and 1 EUR.
 * This function executes when a board button is pressed and signals that a coin
 * was inserted and returns the value of the inserted coin  
 *
 * \param[out] coin value of coin
 * \param[out] coin_detected flag to signal the insertion of a coin
*/
void butcoinpress_cbfunction(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
    coin_detected = 1;
    
    if(BIT(BOARDBUT1) & pins)
        coin = 0.10;  // 10 cents

    if(BIT(BOARDBUT2) & pins)
        coin = 0.20;  // 20 cents

    if(BIT(BOARDBUT3) & pins)
        coin = 0.50;  // 50 cents

    if(BIT(BOARDBUT4) & pins)
        coin = 1.00;  // 1 euro
}

/** \brief Callback function of the interrupt from the four panel control buttons.
 * 
 *  This function executes when a panel button is pressed and updates which button was pressed.  
 *
 * \param[out] button_pressed which button was pressed
*/
void butpanelpress_cbfunction(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
    if(BIT(BUTUP) & pins)
        button_pressed = UP;    // Up arrow button

    if(BIT(BUTDOWN) & pins)
        button_pressed = DOWN;  // Down arrow button

    if(BIT(BUTSELECT) & pins)
        button_pressed = SELECT;  // Select button

    if(BIT(BUTRETURN) & pins)
        button_pressed = RETURN;  // Return button
}

/** \brief Main function of program
 * 
 * The main function has the implementation of the state machine of the vending machine.
 * 
*/
void main(void)
{
    int state = WAIT;   // Stores state of the machine
    float credit = 0;   // Stores the credit available
    int product = 0;    // Stores the index of the price and product to be selected.
    
    int whole = 0;      // Auxiliar variable for printing a float (integer part)
    int remainder = 0;  // Auxiliar variable for printing a float (decimal part)

    // Config input pins and interruptions
    input_output_config();

    while(1)
    {
        switch(state)
        {
            case WAIT:
                
                // print information
                float2int(price[product], &whole, &remainder);
                printk("\rProduct: %s, Cost: %d.%d €,", products[product], whole, remainder);
                
                float2int(credit, &whole, &remainder);
                printk(" Credit: %d.%d €", whole, remainder);
                
                /* Pause  */ 
                k_msleep(SLEEP_MS);  

                if(coin_detected == 1)  // Change state to update credit
                    state = UPDATE_CREDIT;
                
                else if(button_pressed == UP || button_pressed == DOWN)     // Change state to update selected product
                    state = UPDATE_PRODUCT;

                else if(button_pressed == RETURN)   // Return credit to user
                {
                    float2int(credit, &whole, &remainder);
                    printk("\n%d.%d EUR return\n", whole, remainder);
                    credit = 0;
                    button_pressed = NONE;
                    
                }
                else if(button_pressed == SELECT)   // Change state to deliver product
                    state = CHECK_CREDIT;

                break;
            
            case UPDATE_CREDIT:     // Update credit based on inserted coin
                coin_detected = 0;
                credit += coin;
                coin = 0;
                state = WAIT;
                       
                break;

            case UPDATE_PRODUCT:
                if(button_pressed == UP)
                {
                    product = (product + 1 + NPRODUCTS) % NPRODUCTS;  // Next Product
                    button_pressed = NONE;
                }
                else if(button_pressed == DOWN)
                {
                    product = (product - 1 + NPRODUCTS) % NPRODUCTS;  // Previous Product
                    button_pressed = NONE;
                }
                
                printk("\33[2K");   // clear line

                state = WAIT;
                
                break;

            case CHECK_CREDIT:
                if(credit >= price[product])    // Enough credit deliver product and update credit
                {
                    credit = credit - price[product];
                    float2int(credit, &whole, &remainder);
                    printk("\nProduct %s dispensed, remaining credit %d.%d €\n", products[product], whole, remainder);
                }

                else    // Not enough credit
                {
                    float2int(price[product], &whole, &remainder);
                    printk("\nNot enough credit, Product %s costs %d.%d €, ", products[product], whole, remainder);
                    float2int(credit, &whole, &remainder);
                    printk("credit is %d.%d €\n", whole, remainder);
                }
                
                button_pressed = NONE;
                state = WAIT;

                break;
        }
    }
      
    return;
}

/** \brief Function to transform float number in two integer numbers
 * 
 *  This function converts a float number to two integral numbers, one of them being the integer part and the other the decimal part of the float number.
 * The variables to store the integer and decimal part of the float number must be passed as pointers on the arguments of the function. 
 *
 * \param[in] arg Value of float number to convert
 * \param[in,out] whole Pointer to variable to store integer part of float number
 * \param[in,out] remainder Pointer to variable to store decimal part of float number
*/
void float2int(float arg, int* whole, int* remainder)
{
    *whole = arg;
    *remainder = (arg - *whole) * 10;
}

/** \brief Configuration Function.
 * 
 *  This function makes all the hardware configuration. Configures the input pins and the interruptions of the microcontroller.
 * 
*/
void input_output_config(void)
{
    const struct device *gpio0_dev;         /* Pointer to GPIO device structure */

    /* Bind to GPIO 0 */
    gpio0_dev = device_get_binding(DT_LABEL(GPIO0_NID));
    
    /* Configure BOARD BUTTON PINS */
    gpio_pin_configure(gpio0_dev, BOARDBUT1, GPIO_INPUT | GPIO_PULL_UP);
    gpio_pin_configure(gpio0_dev, BOARDBUT2, GPIO_INPUT | GPIO_PULL_UP);
    gpio_pin_configure(gpio0_dev, BOARDBUT3, GPIO_INPUT | GPIO_PULL_UP);
    gpio_pin_configure(gpio0_dev, BOARDBUT4, GPIO_INPUT | GPIO_PULL_UP);
    
    /* Configure GPIO to external buttons */
    gpio_pin_configure(gpio0_dev, BUTUP, GPIO_INPUT | GPIO_PULL_UP);
    gpio_pin_configure(gpio0_dev, BUTDOWN, GPIO_INPUT | GPIO_PULL_UP);
    gpio_pin_configure(gpio0_dev, BUTSELECT, GPIO_INPUT | GPIO_PULL_UP);
    gpio_pin_configure(gpio0_dev, BUTRETURN, GPIO_INPUT | GPIO_PULL_UP);

    /* Set interrupt HW - which pin and event generate interrupt
    Board buttons configured to generate interrupt on rising-edge
    Control Panel buttons configured to generate interrupt on falling-edge */

    gpio_pin_interrupt_configure(gpio0_dev, BOARDBUT1, GPIO_INT_EDGE_TO_ACTIVE);
    gpio_pin_interrupt_configure(gpio0_dev, BOARDBUT2, GPIO_INT_EDGE_TO_ACTIVE);
    gpio_pin_interrupt_configure(gpio0_dev, BOARDBUT3, GPIO_INT_EDGE_TO_ACTIVE);
    gpio_pin_interrupt_configure(gpio0_dev, BOARDBUT4, GPIO_INT_EDGE_TO_ACTIVE);

    gpio_pin_interrupt_configure(gpio0_dev, BUTUP, GPIO_INT_EDGE_TO_INACTIVE);
    gpio_pin_interrupt_configure(gpio0_dev, BUTDOWN, GPIO_INT_EDGE_TO_INACTIVE);
    gpio_pin_interrupt_configure(gpio0_dev, BUTSELECT, GPIO_INT_EDGE_TO_INACTIVE);
    gpio_pin_interrupt_configure(gpio0_dev, BUTRETURN, GPIO_INT_EDGE_TO_INACTIVE);
    
    /* Set callbacks */
    gpio_init_callback(&butcoin_cb_data, butcoinpress_cbfunction, BIT(BOARDBUT1)| BIT(BOARDBUT2)| BIT(BOARDBUT3) | BIT(BOARDBUT4));
    gpio_add_callback(gpio0_dev, &butcoin_cb_data);

    gpio_init_callback(&butpanel_cb_data, butpanelpress_cbfunction, BIT(BUTUP)| BIT(BUTDOWN)| BIT(BUTSELECT) | BIT(BUTRETURN));
    gpio_add_callback(gpio0_dev, &butpanel_cb_data);
}
