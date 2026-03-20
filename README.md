# ECU Simulator over CAN using STM32, Arduino Uno, MCP2515, and Potentiometer

### Project Members
- Ghoorah Hemshini
- Boodhram Ritviksingh

## Authors Contribution
This project was developed as a collaborative effort. Both contributors were equally involved in hardware integration, software development, debugging, and system testing.

## Project Overview
This project implements a simple ECU communication prototype using two microcontroller platforms connected over a CAN bus.
The **STM32 NUCLEO-F446RE** reads a potentiometer value, converts it into a throttle command, and transmits it through an **MCP2515 CAN controller**. An **Arduino Uno** with another **MCP2515** receives the CAN message and reconstructs the transmitted throttle value.
The project demonstrates basic embedded communication between two electronic control nodes using:
- ADC
- SPI
- UART
- CAN
It serves as a practical introduction to ECU-oriented embedded systems, signal acquisition, and bus-based message exchange.

## Project Objective
The objective of this project is to build a small embedded system that simulates a basic automotive signal flow:
- acquire an analog input from a potentiometer
- convert the analog signal into a digital throttle value
- send the throttle value over CAN
- receive the CAN frame on a second controller
- reconstruct the transmitted signal on the receiver side
This project was developed as a beginner-friendly ECU simulation prototype and demonstrates core concepts relevant to embedded controls and automotive communication networks.

## System Architecture
The system is composed of two main nodes.

### Transmitter Node
- **Board:** STM32 NUCLEO-F446RE
- **Input:** potentiometer
- **CAN controller:** MCP2515
- **Tasks:**
  - read analog voltage using ADC
  - scale ADC value into throttle value
  - send throttle value in a CAN frame

### Receiver Node
- **Board:** Arduino Uno
- **CAN controller:** MCP2515
- **Tasks:**
  - receive CAN frame
  - reconstruct throttle value from two data bytes
  - display received result through serial monitor

## Hardware Components
- STM32 NUCLEO-F446RE
- Arduino Uno
- 2 × MCP2515 CAN modules
- 1 × potentiometer
- jumper wires
- USB cable for STM32
- USB cable for Arduino

## Communication Concept
The STM32 reads the potentiometer through ADC and maps the raw ADC value into a throttle command from `0` to `1000`. That value is split into two bytes and transmitted over the CAN bus.

### CAN Frame Structure
- **CAN ID:** `0x100`
- **Frame type:** standard 11-bit identifier
- **Data length:** `2 bytes`

### Data Payload
- **Byte 0:** throttle high byte
- **Byte 1:** throttle low byte
The Arduino receives these two bytes and reconstructs the 16-bit throttle value.

## Wiring

## STM32 Side
### Potentiometer to STM32
- potentiometer outer pin → `3.3V`
- potentiometer outer pin → `GND`
- potentiometer middle pin → `A0 / PA0`

### MCP2515 to STM32
- MCP2515 `SCK` → STM32 `D13 / PA5`
- MCP2515 `MISO` → STM32 `D12 / PA6`
- MCP2515 `MOSI` → STM32 `D11 / PA7`
- MCP2515 `CS` → STM32 `D10 / PB6`
- MCP2515 `VCC` → `5V`
- MCP2515 `GND` → `GND`

## Arduino Side

### MCP2515 to Arduino Uno
- MCP2515 `SCK` → Arduino `D13`
- MCP2515 `MISO` → Arduino `D12`
- MCP2515 `MOSI` → Arduino `D11`
- MCP2515 `CS` → Arduino `D10`
- MCP2515 `INT` → Arduino `D2`
- MCP2515 `VCC` → `5V`
- MCP2515 `GND` → `GND`

## CAN Bus Connection Between Both MCP2515 Modules
- CANH ↔ CANH
- CANL ↔ CANL
- GND ↔ GND

## Software Tools Used
- STM32CubeIDE
- Arduino IDE
- STM32 HAL libraries
- MCP2515 CAN communication routines
- Serial Monitor for debugging

## Development Process
The project was developed in several stages.

### Stage 1 – STM32 Peripheral Setup
The STM32 project was created in STM32CubeIDE. The following peripherals were configured:
- **ADC1** for reading the potentiometer on `PA0`
- **SPI1** for communication with the MCP2515
- **USART2** for serial debug output
- **GPIO output** on `PB6` for MCP2515 chip select

### Stage 2 – ADC Validation
The potentiometer signal was read using ADC and printed over UART. This step confirmed that:
- the potentiometer wiring was correct
- the ADC reading changed when the knob was rotated
- serial communication over USART2 worked correctly

### Stage 3 – Throttle Mapping
The raw ADC value from `0` to `4095` was scaled into a throttle value from `0` to `1000`.
Used formula:

```c
throttle = (uint16_t)((adc_raw * 1000) / 4095);
```
### Stage 4 – SPI Communication with MCP2515
The STM32 communicated with the MCP2515 over SPI. Register reads confirmed that the CAN controller was responding correctly.

### Stage 5 – CAN Transmission
The STM32 configured the MCP2515 for CAN communication and transmitted the throttle value using:
- CAN ID 0x100
- 2 data bytes

### Stage 6 – Arduino Reception
The Arduino node received the frame, reconstructed the 16-bit throttle value, and displayed the result through the serial monitor.

### STM32 Functionality
The STM32 acts as the sender node.

## Main Tasks
- read analog input from potentiometer
- compute throttle value
- package the throttle value into a CAN frame
- transmit CAN frame continuously

### Important STM32 Peripherals
- ADC1 for analog input
- SPI1 for MCP2515 communication
- USART2 for debug print output

### Arduino Functionality
The Arduino acts as the receiver node.

## Main Tasks
- listen for incoming CAN frames
- check for CAN ID 0x100
- read two data bytes
- reconstruct the original throttle value
- print the received value

### Reconstruction Method
```c
uint16_t throttle = ((uint16_t)data[0] << 8) | data[1];
```
### Example Output
## STM32 Serial Output
```c
STM32 ADC + MCP2515 TX test started
CANSTAT = 0xC0
CANCTRL = 0xC3
ADC raw = 1914 | Throttle = 467 | CAN TX sent
ADC raw = 2050 | Throttle = 500 | CAN TX sent
ADC raw = 4095 | Throttle = 1000 | CAN TX sent
```
## Arduino Serial Output
```c
ID: 0x100  throttle: 467
ID: 0x100  throttle: 500
ID: 0x100  throttle: 1000
```
### Key Learning Outcomes
This project helped develop practical understanding of:
- ADC signal acquisition
- embedded C programming
- SPI communication
- UART debugging
- CAN message formatting
- transmission and reception of automotive-style signals
- integration of multiple microcontrollers in one system

### Conclusion
This project successfully demonstrates a basic ECU-style CAN communication system using an STM32 transmitter node and an Arduino receiver node. A potentiometer signal is read, converted into a throttle value, transmitted over CAN, and reconstructed on the receiving side.
The system provides a practical introduction to embedded automotive communication and forms a strong base for more advanced ECU, BMS, and control-system projects.
