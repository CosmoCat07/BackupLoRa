#pragma once
#include <stdio.h>
#include <RadioLib.h>
#include "hal/RPiPico/PicoHal.h"
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/timer.h"
#include "hardware/clocks.h"
#include <string>
#include <functional>

// define pins to be used
#define SPI_PORT spi0
#define SPI_MISO 4
#define SPI_MOSI 3
#define SPI_SCK 2

#define RFM_NSS 26
#define RFM_RST 22
#define RFM_DIO0 14
#define RFM_DIO1 15




int goLora(SX1262& radio){;
    return radio.begin(915.0, 125.0, 9, 7, 0x22, 22, 8, 1.7, false);
}
int goFSK(SX1262& radio){
    return radio.beginFSK(915.0 , 4.8, 5.0, 156.2, 22, 16, 1.7, false);
}
void timeout_fallBack(){
    
}