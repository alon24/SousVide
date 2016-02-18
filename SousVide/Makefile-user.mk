## Local build configuration
## Parameters configured here will override default and ENV values.
## Uncomment and change examples:

#Add your source directories here separated by space
####MODULES = app app/drivers
MODULES ?= app app/drivers app/pid

## ESP_HOME sets the path where ESP tools and SDK are located.
## Windows:
# ESP_HOME = c:/Espressif

## MacOS / Linux:
#ESP_HOME = /opt/esp-open-sdk

## SMING_HOME sets the path where Sming framework is located.
## Windows:
# SMING_HOME = c:/tools/sming/Sming 

# MacOS / Linux
# SMING_HOME = /opt/sming/Sming

## COM port parameter is reqruied to flash firmware correctly.
## Windows: 
# COM_PORT = COM4

# MacOS / Linux:
COM_PORT = /dev/tty.SLAB_USBtoUART

# Com port speed
COM_SPEED	= 961000

## use rboot build mode
RBOOT_ENABLED ?= 1
## enable big flash support (for multiple roms, each in separate 1mb block of flash)
RBOOT_BIG_FLASH ?= 1
## size of the flash chip
SPI_SIZE        ?= 4M
## size of the spiffs to create
SPIFF_SIZE      ?= 600000
## (spiffs location defaults to the mb after the rom slot on 4mb flash)
