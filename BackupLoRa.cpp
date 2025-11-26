#define INITIATING_NODE
#include "backupTest.hpp"


PicoHal* hal = new PicoHal(SPI_PORT, SPI_MISO, SPI_MOSI, SPI_SCK);
SX1262 radio = new Module(hal, RFM_NSS, RFM_DIO0, RFM_RST, RFM_DIO1);
int transmissionState = RADIOLIB_ERR_NONE;
bool transmitFlag = false;
volatile bool operationDone = false;

static int alarm_num = 0;
static int alarm_irq = timer_hardware_alarm_get_irq_num(timer_hw, 0);
static volatile bool time_out = false;

static void alarm_itrHandler(void) {
  hw_clear_bits(&timer_hw->intr, 1u << alarm_num);
  time_out = true;
}

void alarm_init() {
  // Enable the interrupt for our alarm (the timer outputs 4 alarm IRQs)
  hw_set_bits(&timer_hw->inte, 1u << alarm_num);
  // Set irq handler for alarm irq
  irq_set_exclusive_handler(alarm_irq, alarm_itrHandler);
  // Enable the alarm irq
  irq_set_enabled(alarm_irq, true);
}

void setAlarm(uint32_t ms) {
  time_out = false;
  uint64_t target = timer_hw->timerawl + ms * 1000;
  timer_hw->alarm[alarm_num] = (uint32_t) target;
}

bool alarmTimedOut() {
  return time_out;
}

void setFlag(void) {
  operationDone = true;
}

int main()
{
  stdio_init_all();
  // int state = radio.begin(915.0, 125.0, 9, 7, 0x22, 22, 8, 1.7, false);//lora
  int state = goFSK(radio); // fsk

  radio.setDio1Action(setFlag);

#if defined(INITIATING_NODE)
  // send the first packet on this node
  printf("[SX1262] Sending first packet ... ");
  transmissionState = radio.startTransmit("Hello World!");
  transmitFlag = true;
#else
  // start listening for LoRa packets on this node
  Serial.print(F("[SX1262] Starting to listen ... "));
  state = radio.startReceive();
  if (state == RADIOLIB_ERR_NONE) {
    Serial.println(F("success!"));
  } else {
    Serial.print(F("failed, code "));
    Serial.println(state);
    while (true) { delay(10); }
  }
#endif

  while (1) {
    if (operationDone) {
      // reset flag
      if (alarmTimedOut()) {
        printf("operation timed out! fallback to LoRa!\n");
        // try to reset the module
        radio.reset();
        goLora(radio);
        continue;
      }
      operationDone = false;

      if (transmitFlag) {
        // the previous operation was transmission, listen for response
        // print the result
        if (transmissionState == RADIOLIB_ERR_NONE) {
          // packet was successfully sent
          printf("transmission finished!\n");
        } else {
          printf("failed, code ");
          printf("%d", transmissionState);
        }

        // clean up TX and listen for response (non-blocking)
        radio.finishTransmit();
        radio.startReceive();
        transmitFlag = false;

      } else {
        // the previous operation was reception
        // print data and send another packet
        uint8_t str[50];
        int state = radio.readData(str, 50);

        if (state == RADIOLIB_ERR_NONE) {
          setAlarm(2000);
          // packet was successfully received
          printf("[SX1262] Received packet!\n");

          // print data of the packet
          printf("[SX1262] Data:\t\t");
          printf("%s \n", str);

          // print RSSI (Received Signal Strength Indicator)
          printf("[SX1262] RSSI:\t\t");
          printf("%f \n", radio.getRSSI());
          printf(" dBm");

          // print SNR (Signal-to-Noise Ratio)
          printf("[SX1262] SNR:\t\t");
          printf("%f \n", radio.getSNR());
          printf(" dB\n");
        }

        // wait a second before transmitting again
        hal->delay(1000);

        // send another one
        printf("[SX1262] Sending another packet ... \n");
        radio.finishReceive();
        transmissionState = radio.startTransmit("Hello World!");
        transmitFlag = true;
      }
    }
  }
}
