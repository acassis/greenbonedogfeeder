#define PERCENT_FOOD    1
#define PERIOD_FOOD     2
#define PERIOD_WATER    4
#define CONFIRM_MODE    8
#define KEY_UP          16
#define KEY_DOWN        32

void set_period_food(uint8_t pfood);

uint8_t get_period_food(void);

void set_period_water(uint8_t pwater);

uint8_t get_period_water(void);

void set_percent_food(uint8_t pfood);

uint8_t get_percent_food(void);

void set_mode(uint8_t mod);

uint8_t get_mode(void);

uint8_t read_button(void);

void write_led(uint8_t mled);

void write_display(uint8_t mdisp);

void set_confirm_mode(void);

void set_running(uint8_t status);

