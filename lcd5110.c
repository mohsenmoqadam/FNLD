#include <sys/param.h>  
#include <sys/module.h>
#include <sys/kernel.h>
#include <sys/systm.h>
#include <sys/conf.h>
#include <sys/types.h>  
#include <sys/uio.h>

#include <sys/bus.h>
#include <dev/spibus/spi.h>
#include <dev/spibus/spibusvar.h>
#include <sys/gpio.h>   

#include <sys/lock.h>
#include <sys/mutex.h>

#include "spibus_if.h"
#include "gpio_if.h"

#include "lcd5110.h"

const uint8_t CHARSET[][5] = {
    { 0x00, 0x00, 0x00, 0x00, 0x00 }, // 20 space
    { 0x00, 0x00, 0x5f, 0x00, 0x00 }, // 21 !
    { 0x00, 0x07, 0x00, 0x07, 0x00 }, // 22 "
    { 0x14, 0x7f, 0x14, 0x7f, 0x14 }, // 23 #
    { 0x24, 0x2a, 0x7f, 0x2a, 0x12 }, // 24 $
    { 0x23, 0x13, 0x08, 0x64, 0x62 }, // 25 %
    { 0x36, 0x49, 0x55, 0x22, 0x50 }, // 26 &
    { 0x00, 0x05, 0x03, 0x00, 0x00 }, // 27 '
    { 0x00, 0x1c, 0x22, 0x41, 0x00 }, // 28 (
    { 0x00, 0x41, 0x22, 0x1c, 0x00 }, // 29 )
    { 0x14, 0x08, 0x3e, 0x08, 0x14 }, // 2a *
    { 0x08, 0x08, 0x3e, 0x08, 0x08 }, // 2b +
    { 0x00, 0x50, 0x30, 0x00, 0x00 }, // 2c ,
    { 0x08, 0x08, 0x08, 0x08, 0x08 }, // 2d -
    { 0x00, 0x60, 0x60, 0x00, 0x00 }, // 2e .
    { 0x20, 0x10, 0x08, 0x04, 0x02 }, // 2f /
    { 0x3e, 0x51, 0x49, 0x45, 0x3e }, // 30 0
    { 0x00, 0x42, 0x7f, 0x40, 0x00 }, // 31 1
    { 0x42, 0x61, 0x51, 0x49, 0x46 }, // 32 2
    { 0x21, 0x41, 0x45, 0x4b, 0x31 }, // 33 3
    { 0x18, 0x14, 0x12, 0x7f, 0x10 }, // 34 4
    { 0x27, 0x45, 0x45, 0x45, 0x39 }, // 35 5
    { 0x3c, 0x4a, 0x49, 0x49, 0x30 }, // 36 6
    { 0x01, 0x71, 0x09, 0x05, 0x03 }, // 37 7
    { 0x36, 0x49, 0x49, 0x49, 0x36 }, // 38 8
    { 0x06, 0x49, 0x49, 0x29, 0x1e }, // 39 9
    { 0x00, 0x36, 0x36, 0x00, 0x00 }, // 3a :
    { 0x00, 0x56, 0x36, 0x00, 0x00 }, // 3b ;
    { 0x08, 0x14, 0x22, 0x41, 0x00 }, // 3c <
    { 0x14, 0x14, 0x14, 0x14, 0x14 }, // 3d =
    { 0x00, 0x41, 0x22, 0x14, 0x08 }, // 3e >
    { 0x02, 0x01, 0x51, 0x09, 0x06 }, // 3f ?
    { 0x32, 0x49, 0x79, 0x41, 0x3e }, // 40 @
    { 0x7e, 0x11, 0x11, 0x11, 0x7e }, // 41 A
    { 0x7f, 0x49, 0x49, 0x49, 0x36 }, // 42 B
    { 0x3e, 0x41, 0x41, 0x41, 0x22 }, // 43 C
    { 0x7f, 0x41, 0x41, 0x22, 0x1c }, // 44 D
    { 0x7f, 0x49, 0x49, 0x49, 0x41 }, // 45 E
    { 0x7f, 0x09, 0x09, 0x09, 0x01 }, // 46 F
    { 0x3e, 0x41, 0x49, 0x49, 0x7a }, // 47 G
    { 0x7f, 0x08, 0x08, 0x08, 0x7f }, // 48 H
    { 0x00, 0x41, 0x7f, 0x41, 0x00 }, // 49 I
    { 0x20, 0x40, 0x41, 0x3f, 0x01 }, // 4a J
    { 0x7f, 0x08, 0x14, 0x22, 0x41 }, // 4b K
    { 0x7f, 0x40, 0x40, 0x40, 0x40 }, // 4c L
    { 0x7f, 0x02, 0x0c, 0x02, 0x7f }, // 4d M
    { 0x7f, 0x04, 0x08, 0x10, 0x7f }, // 4e N
    { 0x3e, 0x41, 0x41, 0x41, 0x3e }, // 4f O
    { 0x7f, 0x09, 0x09, 0x09, 0x06 }, // 50 P
    { 0x3e, 0x41, 0x51, 0x21, 0x5e }, // 51 Q
    { 0x7f, 0x09, 0x19, 0x29, 0x46 }, // 52 R
    { 0x46, 0x49, 0x49, 0x49, 0x31 }, // 53 S
    { 0x01, 0x01, 0x7f, 0x01, 0x01 }, // 54 T
    { 0x3f, 0x40, 0x40, 0x40, 0x3f }, // 55 U
    { 0x1f, 0x20, 0x40, 0x20, 0x1f }, // 56 V
    { 0x3f, 0x40, 0x38, 0x40, 0x3f }, // 57 W
    { 0x63, 0x14, 0x08, 0x14, 0x63 }, // 58 X
    { 0x07, 0x08, 0x70, 0x08, 0x07 }, // 59 Y
    { 0x61, 0x51, 0x49, 0x45, 0x43 }, // 5a Z
    { 0x00, 0x7f, 0x41, 0x41, 0x00 }, // 5b [
    { 0x02, 0x04, 0x08, 0x10, 0x20 }, // 5c backslash
    { 0x00, 0x41, 0x41, 0x7f, 0x00 }, // 5d ]
    { 0x04, 0x02, 0x01, 0x02, 0x04 }, // 5e ^
    { 0x40, 0x40, 0x40, 0x40, 0x40 }, // 5f _
    { 0x00, 0x01, 0x02, 0x04, 0x00 }, // 60 `
    { 0x20, 0x54, 0x54, 0x54, 0x78 }, // 61 a
    { 0x7f, 0x48, 0x44, 0x44, 0x38 }, // 62 b
    { 0x38, 0x44, 0x44, 0x44, 0x20 }, // 63 c
    { 0x38, 0x44, 0x44, 0x48, 0x7f }, // 64 d
    { 0x38, 0x54, 0x54, 0x54, 0x18 }, // 65 e
    { 0x08, 0x7e, 0x09, 0x01, 0x02 }, // 66 f
    { 0x0c, 0x52, 0x52, 0x52, 0x3e }, // 67 g
    { 0x7f, 0x08, 0x04, 0x04, 0x78 }, // 68 h
    { 0x00, 0x44, 0x7d, 0x40, 0x00 }, // 69 i
    { 0x20, 0x40, 0x44, 0x3d, 0x00 }, // 6a j
    { 0x7f, 0x10, 0x28, 0x44, 0x00 }, // 6b k
    { 0x00, 0x41, 0x7f, 0x40, 0x00 }, // 6c l
    { 0x7c, 0x04, 0x18, 0x04, 0x78 }, // 6d m
    { 0x7c, 0x08, 0x04, 0x04, 0x78 }, // 6e n
    { 0x38, 0x44, 0x44, 0x44, 0x38 }, // 6f o
    { 0x7c, 0x14, 0x14, 0x14, 0x08 }, // 70 p
    { 0x08, 0x14, 0x14, 0x18, 0x7c }, // 71 q
    { 0x7c, 0x08, 0x04, 0x04, 0x08 }, // 72 r
    { 0x48, 0x54, 0x54, 0x54, 0x20 }, // 73 s
    { 0x04, 0x3f, 0x44, 0x40, 0x20 }, // 74 t
    { 0x3c, 0x40, 0x40, 0x20, 0x7c }, // 75 u
    { 0x1c, 0x20, 0x40, 0x20, 0x1c }, // 76 v
    { 0x3c, 0x40, 0x30, 0x40, 0x3c }, // 77 w
    { 0x44, 0x28, 0x10, 0x28, 0x44 }, // 78 x
    { 0x0c, 0x50, 0x50, 0x50, 0x3c }, // 79 y
    { 0x44, 0x64, 0x54, 0x4c, 0x44 }, // 7a z
    { 0x00, 0x08, 0x36, 0x41, 0x00 }, // 7b {
    { 0x00, 0x00, 0x7f, 0x00, 0x00 }, // 7c |
    { 0x00, 0x41, 0x36, 0x08, 0x00 }, // 7d }
    { 0x10, 0x08, 0x08, 0x10, 0x08 }, // 7e ~
    { 0x00, 0x00, 0x00, 0x00, 0x00 }  // 7f
};

struct lcd5110_disp_buff_t lcd5110_disp_buff;
struct lcd5110_sc_t *lcd_5110_sc;
static devclass_t lcd5110_devclass; 
static d_write_t lcd5110_write;
static struct cdevsw lcd5110_cdevsw =
{
    .d_version = D_VERSION,
    .d_write   = lcd5110_write,
    .d_name    = "lcd5110"
};
static device_method_t
lcd5110_methods[] =
  {
    DEVMETHOD(device_probe, lcd5110_probe),
    DEVMETHOD(device_attach, lcd5110_attach),
    DEVMETHOD(device_detach, lcd5110_detach),
    DEVMETHOD(device_shutdown, lcd5110_shutdown),
    {0, 0}
  };
static driver_t
lcd5110_driver =
  {
    "lcd5110", /* driver’s official name */
    lcd5110_methods, /* device method table */
    sizeof(struct  lcd5110_sc_t)
  };

/* Register LCD5110 Newbus driver */
DRIVER_MODULE(lcd5110, spibus, lcd5110_driver, lcd5110_devclass, NULL, NULL);


void
lcd5110_do_reset(void)
{
    GPIO_PIN_SET(lcd_5110_sc->dev_gpio, LCD5110_RST, GPIO_PIN_HIGH);
    DELAY(10000); // 10 msec
    GPIO_PIN_SET(lcd_5110_sc->dev_gpio, LCD5110_RST, GPIO_PIN_LOW);
    DELAY(10000); // 10 msec
    GPIO_PIN_SET(lcd_5110_sc->dev_gpio, LCD5110_RST, GPIO_PIN_HIGH);
}

void lcd5110_send(uint8_t byte)
{
    struct spi_command spi_cmd;
    uint8_t temp;
    memset(&spi_cmd, 0, sizeof(struct spi_command));
    spi_cmd.tx_data = &byte;
    spi_cmd.rx_data = &temp;
    spi_cmd.rx_data_sz = 1;
    spi_cmd.tx_data_sz = 1;
    SPIBUS_TRANSFER(device_get_parent(lcd_5110_sc->dev), lcd_5110_sc->dev, &spi_cmd);
}

void lcd5110_send_cmd(uint8_t cmd)
{
    // Put LCD in Command Mode
    GPIO_PIN_SET(lcd_5110_sc->dev_gpio, LCD5110_DC, GPIO_PIN_LOW);
    lcd5110_send(cmd);
}

void lcd5110_send_data(uint8_t *data, int len)
{
    int cnt;
    // Put LCD in Data Mode
    GPIO_PIN_SET(lcd_5110_sc->dev_gpio, LCD5110_DC, GPIO_PIN_HIGH); 
    for(cnt = 0; cnt < len; cnt++)
	lcd5110_send(*data++);
}

void
lcd5110_render(void)
{
    // Set Col and Row
    lcd5110_send_cmd(0x80);
    lcd5110_send_cmd(0x40);
    // render
    lcd5110_send_data(lcd5110_disp_buff.screen, 504);
}

void
lcd5110_set_pixel(uint8_t x, uint8_t y, uint8_t value)
{
    uint8_t *byte = &lcd5110_disp_buff.screen[y/8*84+x];
    if (value)
	*byte |= (1 << (y % 8));
    else
	*byte &= ~(1 << (y %8 ));
}

void
lcd5110_set_cursor(uint8_t x, uint8_t y)
{
    lcd5110_disp_buff.cursor_x = x;
    lcd5110_disp_buff.cursor_y = y;
}

void
lcd5110_clear(void)
{
	register unsigned i;
	/* Set column and row to 0 */
	lcd5110_send_cmd(0x80);
	lcd5110_send_cmd(0x40);
	/*Cursor too */
	lcd5110_disp_buff.cursor_x = 0;
	lcd5110_disp_buff.cursor_y = 0;
	/* Clear everything (504 bytes = 84cols * 48 rows / 8 bits) */
	for(i = 0;i < 504; i++)
	    lcd5110_disp_buff.screen[i] = 0x00;
	lcd5110_render();
}

void
lcd5110_write_char(char code, uint8_t scale)
{
    register uint8_t x, y;    
    switch (code){
    case 0x0A : // \n
	lcd5110_disp_buff.cursor_x = 0;
	lcd5110_disp_buff.cursor_y += 7*scale + 1;
	break;
    default:
	if (lcd5110_disp_buff.cursor_x >= 84) {
	    lcd5110_disp_buff.cursor_x = 0;
	    lcd5110_disp_buff.cursor_y += 7*scale + 1;
	}
	if (lcd5110_disp_buff.cursor_y >= 48) {
	    lcd5110_clear();
	    lcd5110_disp_buff.cursor_x = 0;
	    lcd5110_disp_buff.cursor_y = 0;
	}
	for (x = 0; x < 5*scale; x++)
	    for (y = 0; y < 7*scale; y++)
		if (CHARSET[code-32][x/scale] & (1 << y/scale))
		    lcd5110_set_pixel(lcd5110_disp_buff.cursor_x + x, lcd5110_disp_buff.cursor_y + y, 1);
		else
		    lcd5110_set_pixel(lcd5110_disp_buff.cursor_x + x, lcd5110_disp_buff.cursor_y + y, 0);
	lcd5110_disp_buff.cursor_x += 5*scale + 1;
    }
}

void
lcd5110_write_string(const char *str, uint8_t scale)
{
    while(*str)
	lcd5110_write_char(*str++, scale);
    lcd5110_render();
}

void
lcd5110_init(void)
{
    // Pin configurations
    GPIO_PIN_SETFLAGS(lcd_5110_sc->dev_gpio, LCD5110_RST, GPIO_PIN_OUTPUT);
    GPIO_PIN_SETFLAGS(lcd_5110_sc->dev_gpio, LCD5110_DC, GPIO_PIN_OUTPUT);
    GPIO_PIN_SETFLAGS(lcd_5110_sc->dev_gpio, LCD5110_BL, GPIO_PIN_OUTPUT);
    GPIO_PIN_SET(lcd_5110_sc->dev_gpio, LCD5110_BL, GPIO_PIN_HIGH);
    lcd5110_do_reset();
    // LCD Iintializations
    lcd5110_send_cmd(EXTEND_MODE);
    lcd5110_send_cmd(BIAS_MODE_1_48);
    lcd5110_send_cmd(TEMPERATURE_COEF);  
    lcd5110_send_cmd(DEFAULT_VOP);
    lcd5110_send_cmd(STD_CMD_MODE);
    lcd5110_send_cmd(NORMAL_OPT_MODE);
    lcd5110_send_cmd(CLEAR_RAM);
    lcd5110_send_cmd(CONTRAST_VAL_1);
    lcd5110_send_cmd(ACTIVATE_CMD_1);
    lcd5110_send_cmd(ACTIVATE_CMD_2);
    lcd5110_send_cmd(SET_COL);
    lcd5110_send_cmd(SET_ROW);
    // Clear local display buffer
    memset(&lcd5110_disp_buff, 0, sizeof(struct lcd5110_disp_buff_t));
    lcd5110_clear();
}

static int
lcd5110_write(struct cdev *dev, struct uio *uio, int ioflag)
{
    int error = 0;
    char buff[MAX_DISPLAY_LEN+1];
    mtx_lock(&lcd_5110_sc->mtx); 
    error = copyin(uio->uio_iov->iov_base,
    		   buff,
    		   MIN(uio->uio_iov->iov_len, MAX_DISPLAY_LEN - 1));
    if (error != 0){
    	uprintf("[LCD5110] Write failed.\n");
	mtx_unlock(&lcd_5110_sc->mtx); 
    	return (error);
    }
    *(buff +  MIN(uio->uio_iov->iov_len, MAX_DISPLAY_LEN - 1)) = 0;
    lcd5110_write_string(buff,1);
    mtx_unlock(&lcd_5110_sc->mtx); 
    return (error);
}

/* Adds LCD5110 to SPI bus. */
static int
lcd5110_probe(device_t dev)
{
  device_set_desc(dev, "Nokia 5110 LCD");
  return (BUS_PROBE_SPECIFIC); /* Only I can use this device. */
}

static int
lcd5110_attach(device_t dev)
{
    lcd_5110_sc = device_get_softc(dev);
    lcd_5110_sc->dev = dev;
    lcd_5110_sc->dev_gpio = devclass_get_device(devclass_find("gpio"), 0);
    if (lcd_5110_sc->dev_gpio == NULL)
    {
	device_printf(lcd_5110_sc->dev, "[LCD5110] Error: failed to get the GPIO dev\n");
	return (1);
    }
    mtx_init(&lcd_5110_sc->mtx, "LCD5110 Mutex", NULL, MTX_DEF);
    lcd5110_init();
    lcd_5110_sc->cdev_p = make_dev(&lcd5110_cdevsw,
				   device_get_unit(dev),
				   UID_ROOT,
				   GID_WHEEL,
				   0600, "lcd5110");
    lcd5110_write_string("LCD5110 For\n",1);
    lcd5110_write_string("FreeBSD OS!\n--------------",1);
    return(0);
}

static int
lcd5110_detach(device_t dev)
{
    lcd5110_clear();
    mtx_destroy(&lcd_5110_sc->mtx);
    destroy_dev(lcd_5110_sc->cdev_p);
    GPIO_PIN_SET(lcd_5110_sc->dev_gpio, LCD5110_BL, GPIO_PIN_LOW);
    return(0);
}

/* Shutdown */
static int
lcd5110_shutdown(device_t dev)
{
    return(lcd5110_detach(dev));
}
