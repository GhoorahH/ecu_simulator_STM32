#include <SPI.h>
#include <mcp2515.h>
#include <Arduino_RouterBridge.h>
const uint8_t CAN_CS_PIN = 10;
MCP2515 mcp2515(CAN_CS_PIN);
struct can_frame canMsg;
void setup() {
  Monitor.begin(115200);
  while (!Monitor) {}
  Monitor.println("Arduino Uno Q + MCP2515 RX test started");
  SPI.begin();
  mcp2515.reset(); 
  mcp2515.setBitrate(CAN_125KBPS, MCP_8MHZ);
  mcp2515.setNormalMode();
  Monitor.println("MCP2515 initialized successfully");
  Monitor.println("Receiver ready");
}
void loop() {
  if (mcp2515.readMessage(&canMsg) == MCP2515::ERROR_OK) {
    Monitor.print("CAN RX -> ID: 0x");
    Monitor.print(canMsg.can_id & CAN_SFF_MASK, HEX);
    Monitor.print(" | DLC: ");
    Monitor.print(canMsg.can_dlc);

    if ((canMsg.can_id & CAN_SFF_MASK) == 0x100 && canMsg.can_dlc >= 2) {
      uint16_t throttle =
          ((uint16_t)canMsg.data[0] << 8) | canMsg.data[1];

      Monitor.print(" | Throttle = ");
      Monitor.println(throttle);
    } else {
      Monitor.print(" | Data:");
      for (uint8_t i = 0; i < canMsg.can_dlc; i++) {
        Monitor.print(" ");
        if (canMsg.data[i] < 0x10) Monitor.print("0");
        Monitor.print(canMsg.data[i], HEX);
      }
      Monitor.println();
    }
  }
}
