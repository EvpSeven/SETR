/** \file main.C
 * 	\brief Programm that have State Machine of a vending machine.
 *
 *  Programm accepts a subset of coins and allows the user to browse available
products, buy one product and return the credit. The inputs are push-buttons and the output is done
via UART/Terminal.
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

#define NPRODUCTS 3 // Number of products
#define SLEEP_MS 250  // Sleep period (ms)

// States
#define WAIT 0
#define UPDATE_CREDIT 1
#define UPDATE_PRODUCT 2
#define CHECK_CREDIT 3

// Buttons
#define NONE 0  
#define UP 1
#define DOWN 2
#define SELECT 3
#define RETURN 4

/* Refer to dts file */
#define GPIO0_NID DT_NODELABEL(gpio0) 
 
#define BOARDBUT1 0xb /* Pin at which BUT1 is connected. Addressing is direct (i.e., pin number) */
#define BOARDBUT2 0xc /* Pin at which BUT2 is connected. Addressing is direct (i.e., pin number) */
#define BOARDBUT3 0x18 /* Pin at which BUT3 is connected. Addressing is direct (i.e., pin number) */
#define BOARDBUT4 0x19 /* Pin at which BUT4 is connected. Addressing is direct (i.e., pin number) */

#define BUTUP 0x03
#define BUTDOWN 0x04
#define BUTSELECT 0x1c
#define BUTRETURN 0x1d

// Buttons callback structures
static struct gpio_callback butcoin_cb_data;
static struct gpio_callback butpanel_cb_data;


static char *products[NPRODUCTS] = {"Beer", "Tuna Sandwich", "Coffee"};
static float price[NPRODUCTS] = {1.5, 1.0, 0.5};

volatile float coin = 0;  // value of coin inserted
volatile int coin_detected = 0; // Sinalize inserted coin
volatile int button_pressed = 0;    // Sinalize panel button pressed

void input_output_config(void);
void float2int(float,int*,int*);


/** \brief Function of interrupt to four buttons
 * 
 * Function of interrupt to four buttons emulate the insertion of coins, whose values are: 10 cents, 20 cents, 50
cents and 1 EUR
 *
 * \param[out] coin value of coin
 * \param[out] coin_detected flag signal coin was inserted
 * 
 *
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

/** \brief Function of interrupt then four buttons are for control.
 * 
 * Function of interrupt then four buttons are for control: Browse Up (Browse Up should display the the next product, its cost and the available
credit), Browse Down (Browse Down should display the the previous product, its cost and the available
credit), Select Product (should print the message “Product x dispensed, remaining credit y”, if the
credit is enough, or “Not enough credit, Product x costs y, credit is z” if credit is not
enough to buy the product) and
Return Credit (Return Credit should print the message “x EUR return”, in which “x” is the available
credit. The credit is set to 0).
 *
 * \param[out] button_pressed Was pressed button
 * 
 *
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
        button_pressed = RETURN;  // Return arrow button
}

/* Main function */
/** \brief Main function of programme
 * 
 * The main function has the implementation of a state diagram of a vending machine.
 * 
 *
*/
void main(void) {

    /* Local vars */    
    int state = WAIT;
    float credit = 0;
    int product = 0;
    
    /* Adjust Vars */
    int whole = 0;
    int remainder = 0;

    input_output_config();

    while(1)
    {
        switch(state)
        {
            case WAIT:
                
                float2int(price[product], &whole, &remainder);
                printk("\rProduct: %s, Cost: %d.%d €,", products[product], whole, remainder);
                
                float2int(credit, &whole, &remainder);
                printk(" Credit: %d.%d €", whole, remainder);
                
                /* Pause  */ 
                k_msleep(SLEEP_MS);  

                if(coin_detected == 1)
                    state = UPDATE_CREDIT;
                
                else if(button_pressed == UP || button_pressed == DOWN)
                    state = UPDATE_PRODUCT;

                else if(button_pressed == RETURN)
                {
                    float2int(credit, &whole, &remainder);
                    printk("\n%d.%d EUR return\n", whole, remainder);
                    credit = 0;
                    button_pressed = NONE;
                    
                }
                else if(button_pressed == SELECT)
                    state = CHECK_CREDIT;

                break;
            
            case UPDATE_CREDIT:
                coin_detected = 0;
                credit += coin;
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
                if(credit >= price[product])
                {
                    credit = credit - price[product];
                    float2int(credit, &whole, &remainder);
                    printk("\nProduct %s dispensed, remaining credit %d.%d €\n", products[product], whole, remainder);
                }

                else
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

/** \brief Function of trasnformer decimal number two integral numbers
 * 
 * That function change a float number para dois numeros inteiros, sendo um deles a parte inteira e o outro a parte decimal do float number.
 *
 * \param[in] arg Value of float number
 * \param[in,out] whole Unitis of float number
 * \param[in,out] remainder Decimal of float number
 * 
 *
*/
void float2int(float arg, int* whole, int* remainder)
{
    *whole = arg;
    *remainder = (arg - *whole) * 10;
}



/** \brief Configuration Function.
 * 
 * This function configure Output and Input pins of microcontroller.
 * 
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

    /* Set interrupt HW - which pin and event generate interrupt */
    gpio_pin_interrupt_configure(gpio0_dev, BOARDBUT1, GPIO_INT_EDGE_TO_ACTIVE);
    gpio_pin_interrupt_configure(gpio0_dev, BOARDBUT2, GPIO_INT_EDGE_TO_ACTIVE);
    gpio_pin_interrupt_configure(gpio0_dev, BOARDBUT3, GPIO_INT_EDGE_TO_ACTIVE);
    gpio_pin_interrupt_configure(gpio0_dev, BOARDBUT4, GPIO_INT_EDGE_TO_ACTIVE);

    gpio_pin_interrupt_configure(gpio0_dev, BUTUP, GPIO_INT_EDGE_TO_INACTIVE);
    gpio_pin_interrupt_configure(gpio0_dev, BUTDOWN, GPIO_INT_EDGE_TO_INACTIVE);
    gpio_pin_interrupt_configure(gpio0_dev, BUTSELECT, GPIO_INT_EDGE_TO_INACTIVE);
    gpio_pin_interrupt_configure(gpio0_dev, BUTRETURN, GPIO_INT_EDGE_TO_INACTIVE);
    
    /* Set callback */
    gpio_init_callback(&butcoin_cb_data, butcoinpress_cbfunction, BIT(BOARDBUT1)| BIT(BOARDBUT2)| BIT(BOARDBUT3) | BIT(BOARDBUT4));
    gpio_add_callback(gpio0_dev, &butcoin_cb_data);

    gpio_init_callback(&butpanel_cb_data, butpanelpress_cbfunction, BIT(BUTUP)| BIT(BUTDOWN)| BIT(BUTSELECT) | BIT(BUTRETURN));
    gpio_add_callback(gpio0_dev, &butpanel_cb_data);
}
