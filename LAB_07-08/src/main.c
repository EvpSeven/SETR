#include <zephyr.h>
#include <device.h>
#include <devicetree.h>
#include <drivers/gpio.h>
#include <sys/printk.h>
#include <sys/__assert.h>
#include <string.h>
#include <timing/timing.h>
#include <stdio.h>

#define NPRODUCTS 3 // Number of products

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

#define BUTUP 0
#define BUTDOWN 0
#define BUTSELECT 0
#define BUTRETURN 0

// Buttons callback structures
static struct gpio_callback but1_cb_data;
static struct gpio_callback but2_cb_data;
static struct gpio_callback but3_cb_data;
static struct gpio_callback but4_cb_data;

static struct gpio_callback butup_cb_data;
static struct gpio_callback butdown_cb_data;
static struct gpio_callback butselect_cb_data;
static struct gpio_callback butreturn_cb_data;

static char *products[NPRODUCTS] = {"Beer", "Tuna Sandwich", "Coffee"};
static float price[NPRODUCTS] = {1.5, 1.0, 0.5};

volatile float coin = 0;  // value of coin inserted
volatile int coin_detected = 0; // Sinalize inserted coin
volatile int button_pressed = 0;    // Sinalize panel button pressed

void but1press_cbfunction(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
    coin = 0.10;  // 10 cents
    coin_detected = 1;
}

void but2press_cbfunction(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
    coin = 0.20;  // 20 cents
    coin_detected = 1;
}

void but3press_cbfunction(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
    coin = 0.50;  // 50 cents
    coin_detected = 1;
}

void but4press_cbfunction(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
    coin = 1.0;  // 1 euro
    coin_detected = 1;
}

// Up arrow button
void butuppress_cbfunction(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
    button_pressed = UP;
}

// Down arrow button
void butdownpress_cbfunction(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
    button_pressed = DOWN;
}

// Select button
void butselectpress_cbfunction(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
    button_pressed = SELECT;
}

void butreturnpress_cbfunction(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
    button_pressed = RETURN;
}

/* Main function */
void main(void) {

    /* Local vars */    
    int state = WAIT;
    float credit = 0;
    int product = 0;

    input_output_config();

    while(1)
    {
        switch(state)
        {
            case WAIT:
                
                printk("Product: %s", products[product]);
                
                if(coin_detected == 1)
                    state = UPDATE_CREDIT;
                
                else if(button_pressed == UP || button_pressed == DOWN)
                    state == UPDATE_PRODUCT;

                else if(button_pressed == RETURN)
                {
                    credit = 0;
                    printk("");
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
                    product = (product + 1) % NPRODUCTS;  // Next Product
                    button_pressed = NONE;
                }
                else if(button_pressed == DOWN)
                {
                    product = (product - 1) % NPRODUCTS;  // Previous Product
                    button_pressed = NONE;
                }

                state == WAIT;
                
                break;

            case CHECK_CREDIT:
                if(credit >= price[product])
                {
                    credit = credit - price[product];
                    printk("");
                }

                else
                {
                    printk("");
                }
                
                state = WAIT;

                break;
        }
    }
      
    return;
}

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

    gpio_pin_interrupt_configure(gpio0_dev, BUTUP, GPIO_INT_EDGE_TO_ACTIVE);
    gpio_pin_interrupt_configure(gpio0_dev, BUTDOWN, GPIO_INT_EDGE_TO_ACTIVE);
    gpio_pin_interrupt_configure(gpio0_dev, BUTSELECT, GPIO_INT_EDGE_TO_ACTIVE);
    gpio_pin_interrupt_configure(gpio0_dev, BUTRETURN, GPIO_INT_EDGE_TO_ACTIVE);
    
    /* Set callback */
    gpio_init_callback(&but1_cb_data, but1press_cbfunction, BIT(BOARDBUT1));
    gpio_add_callback(gpio0_dev, &but1_cb_data);

    gpio_init_callback(&but2_cb_data, but2press_cbfunction, BIT(BOARDBUT2));
    gpio_add_callback(gpio0_dev, &but2_cb_data);

    gpio_init_callback(&but3_cb_data, but3press_cbfunction, BIT(BOARDBUT3));
    gpio_add_callback(gpio0_dev, &but3_cb_data);

    gpio_init_callback(&but4_cb_data, but4press_cbfunction, BIT(BOARDBUT4));
    gpio_add_callback(gpio0_dev, &but4_cb_data);

    gpio_init_callback(&butup_cb_data, butuppress_cbfunction, BIT(BUTUP));
    gpio_add_callback(gpio0_dev, &butup_cb_data);

    gpio_init_callback(&butdown_cb_data, butdownpress_cbfunction, BIT(BUTDOWN));
    gpio_add_callback(gpio0_dev, &butup_cb_data);

    gpio_init_callback(&butselect_cb_data, butselectpress_cbfunction, BIT(BUTSELECT));
    gpio_add_callback(gpio0_dev, &butup_cb_data);

    gpio_init_callback(&butreturn_cb_data, butreturnpress_cbfunction, BIT(BUTRETURN));
    gpio_add_callback(gpio0_dev, &butreturn_cb_data);
}
