// Hardware connections
#define LCD5110_RST      16
#define LCD5110_DC       20
#define LCD5110_BL       21

// Local disply buffer size
#define MAX_DISPLAY_LEN  90

// LCD5110 Instaructions
#define EXTEND_MODE      0x21
#define BIAS_MODE_1_48   0x13
#define TEMPERATURE_COEF 0x06
#define DEFAULT_VOP      0xC2
#define STD_CMD_MODE     0x20
#define NORMAL_OPT_MODE  0x09
#define CLEAR_RAM        0x80
#define CONTRAST_VAL_1   0x40
#define ACTIVATE_CMD_1   0x08
#define ACTIVATE_CMD_2   0x0C
#define SET_ROW          0x40
#define SET_COL          0x80

struct lcd5110_sc_t
{
    device_t dev;
    device_t dev_gpio;
    struct cdev* cdev_p;
    struct mtx mtx;
};

struct lcd5110_disp_buff_t{
    /* screen byte massive */
    uint8_t screen[504];
    /* cursor position */
    uint8_t cursor_x;
    uint8_t cursor_y;
};

void lcd5110_do_reset(void);
void lcd5110_send(uint8_t byte);
void lcd5110_send_cmd(uint8_t cmd);
void lcd5110_send_data(uint8_t *data, int len);
void lcd5110_render(void);
void lcd5110_set_pixel(uint8_t x, uint8_t y, uint8_t value);
void lcd5110_set_cursor(uint8_t x, uint8_t y);
void lcd5110_clear(void);
void lcd5110_write_char(char code, uint8_t scale);
void lcd5110_write_string(const char *str, uint8_t scale);
void lcd5110_init(void);

static int lcd5110_write(struct cdev *dev, struct uio *uio, int ioflag);

static int lcd5110_probe(device_t dev);
static int lcd5110_attach(device_t dev);
static int lcd5110_detach(device_t dev);


