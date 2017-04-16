KMOD =   lcd5110
SRCS =   lcd5110.c
SRCS +=  device_if.h
SRCS +=  bus_if.h
SRCS +=  gpio_if.h
SRCS +=  spibus_if.h
.include <bsd.kmod.mk>
