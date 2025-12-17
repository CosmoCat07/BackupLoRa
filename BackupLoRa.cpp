//#define INITIATING_NODE

#include "backupTest.hpp"

// modem configuration
#define FREQUENCY             915.000   // 
#define BANDWIDTH             125.0    // Sets LoRa bandwidth. Allowed values are 7.8, 10.4, 15.6, 20.8, 31.25, 41.7, 62.5, 125.0, 250.0 and 500.0 kHz.
#define SPREADING_FACTOR      7      // Sets LoRa spreading factor. Allowed values range from 5 to 12.
#define CODING_RATE           5      // Sets LoRa coding rate 4/x denominator. Allowed x values range from 5 to 8.
#define CURRENT_LIMIT         140     // mA
#define OUTPUT_POWER          22      // dBm
#define LORA_PREAMBLE_LEN     8       // preambleLength LoRa preamble length in symbols. Allowed values range from 1 to 65535.
#define SYNC_WORD             0x12       // public default=0x12,  LoRaWAN default=0x34
#define LDRO                  false   // normally 'true' for SF-11 or 12
#define CRC                   true    
#define IQINVERTED            false
#define DATA_SHAPING          RADIOLIB_SHAPING_1_0     // Data shaping = 1.0
#define TCXO_VOLTAGE          1.7     // volts
#define WHITENING_INITIAL     0x00FF   // initial whitening LFSR value

PicoHal* hal = new PicoHal(SPI_PORT, SPI_MISO, SPI_MOSI, SPI_SCK);
SX1262 radio = new Module(hal, RFM_NSS, RFM_DIO1, RFM_RST, RFM_BUSY);
int transmissionState = RADIOLIB_ERR_NONE;
bool transmitFlag = false;
volatile bool operationDone = false;
        static int alarm_num = 0;
        static int alarm_irq = timer_hardware_alarm_get_irq_num(timer_hw, 0);
        static volatile bool time_out = false;;
        static void alarm_itrHandler(void){
            hw_clear_bits(&timer_hw->intr, 1u << alarm_num);
            time_out = true;
        }
        void alarm_init(){
            // Enable the interrupt for our alarm (the timer outputs 4 alarm irqs)
            hw_set_bits(&timer_hw->inte, 1u << alarm_num);
            // Set irq handler for alarm irq
            irq_set_exclusive_handler(alarm_irq, alarm_itrHandler);
            // Enable the alarm irqf
            irq_set_enabled(alarm_irq, true);
        }
      
        void setAlarm(uint32_t ms){
            time_out = false;
            uint64_t target = timer_hw->timerawl + ms*1000;
            timer_hw->alarm[alarm_num] = (uint32_t) target;
        }
        bool alarmTimedOut(){
            return time_out;
        }

void setFlag(void){
    operationDone = true;
}

int main()
{
    stdio_init_all();          
  hal->pinMode(RFM_RST, 1);    //output     
  hal->digitalWrite(RFM_RST, 1);     //write high 
  hal->spiBegin();    // fix from https://github.com/jgromes/RadioLib/issues/729
    
     int state = radio.begin(FREQUENCY,
                          BANDWIDTH,
                          SPREADING_FACTOR,
                          CODING_RATE,
                          SYNC_WORD,
                          OUTPUT_POWER,
                          LORA_PREAMBLE_LEN,
                          TCXO_VOLTAGE);
  radio.setCurrentLimit(CURRENT_LIMIT);
  radio.forceLDRO(LDRO);
  radio.setCRC(CRC);
  radio.invertIQ(IQINVERTED);
  radio.setWhitening(true, WHITENING_INITIAL);
  radio.explicitHeader();

   // int state = goFSK(radio);//fsk

    radio.setDio1Action(setFlag);
     #if defined(INITIATING_NODE)
    // send the first packet on this node
    printf("[SX1262] Sending first packet ... ");
    transmissionState = radio.startTransmit("Hello World!");
    transmitFlag = true;
  #else
    // start listening for LoRa packets on this node
    sleep_ms(2000);
    printf("[SX1262] Starting to listen ... ");
    state = radio.startReceive();
    if (state == RADIOLIB_ERR_NONE) {
      printf("success!");
    } else {
      printf("failed, code ");
      while (true) { sleep_ms(10); }
    }
  #endif
    

    while(1){

    // reset flag
    if (alarmTimedOut()) {
      printf("operation timed out! fallback to LoRa!\n");
      // try to reset the module
      radio.reset();
      goLora(radio);
      continue;
    }
    operationDone = false;
    
    if(transmitFlag) {
      // the previous operation was transmission, listen for response
      // print the result
      if (transmissionState == RADIOLIB_ERR_NONE) {
        // packet was successfully sent
        printf("transmission finished!\n");

      } else {
        printf("failed, code ");
        printf("%d", transmissionState);

      }

      // listen for response
      radio.startReceive();
      printf("Listening \n");
      transmitFlag = false;

    } else {
      // the previous operation was reception
      // print data and send another packetSSS
      uint8_t str[50];
      int state = radio.readData(str,50);

      if (state == RADIOLIB_ERR_NONE) {
        setAlarm(2000);
        // packet was successfully received
        printf("[SX1262] Received packet!\n");

        // print data of the packet
        printf("[SX1262] Data:\t\t");
        printf("%s \n", (char*)str);

        // print RSSI (Received Signal Strength Indicator)
        printf("[SX1262] RSSI:\t\t");
        printf("%f \n", radio.getRSSI());
        printf(" dBm");

        // print SNR (Signal-to-Noise Ratio)
        printf("[SX1262] SNR:\t\t");
        printf("%f \n",radio.getSNR());
        printf(" dB\n");

      }

      radio.finishReceive();

      // wait a second before transmitting again
      hal->delay(1000);

      // send another one
      printf("[SX1262] Sending another packet ... \n");
      transmissionState = radio.startTransmit("Hello World!");
      transmitFlag = true;
    }

}
}
