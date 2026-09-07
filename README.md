# RP2040 Digital Signal Generator

A digital signal generator implemented on a **Raspberry Pi Pico (RP2040)** using five different programming approaches:

1. Arduino IDE
2. MicroPython
3. C with polling
4. C with interrupts
5. C with polling + interrupts

The system generates four types of waveforms and allows the user to configure their amplitude, DC offset, and frequency through a 4×4 matrix keypad.

The project was developed as part of **Laboratorio de Electrónica Digital 3** at Universidad de Antioquia.

---

## Overview

The Digital Signal Generator (DSG) uses a Raspberry Pi Pico to generate configurable electrical waveforms from user-defined parameters.

The available waveforms are:

* Sine
* Triangle
* Sawtooth
* Square

The user interacts with the system through a **4×4 matrix keypad** and a dedicated pushbutton for changing the waveform.

The same functional concept was implemented using different programming environments and input-handling techniques in order to compare their behavior, complexity, and performance on the RP2040.

---

## Features

* Four configurable waveforms:

  * Sine
  * Triangle
  * Sawtooth
  * Square
* Adjustable amplitude
* Adjustable DC offset
* Adjustable frequency
* 4×4 matrix keypad interface
* Dedicated waveform-selection button
* Raspberry Pi Pico / RP2040
* Multiple programming approaches
* Serial/USB parameter monitoring in the corresponding implementations
* External analog output stage

---

## Hardware

The main hardware used in the project includes:

* Raspberry Pi Pico
* 4×4 matrix keypad
* Pushbutton
* Resistors
* Capacitor
* Operational amplifier
* Analog output conditioning stage
* External 8-bit resistor DAC

### Prototype

<img src="media/protoboard.jpeg" width="480">

The complete prototype was assembled on a protoboard during development and testing.

### DAC Implementation Options

Two different approaches were considered for converting the Pico's digital output into an analog signal:

* A **dedicated DAC IC**
* An **8-bit resistor DAC**

These are alternative implementations of the digital-to-analog conversion stage, not circuits used simultaneously.

#### Dedicated DAC IC — Alternative

<img src="media/schematic.jpeg" width="480">

A dedicated DAC IC is a preferable option when better control of the conversion process and improved analog performance are required.

This circuit represents an **alternative hardware implementation** considered for the project. It was **not the DAC implementation used in the final prototype**.

#### 8-bit Resistor DAC — Implemented

<img src="media/DAC_resistors.png" width="480">

The implemented prototype uses an **8-bit resistor-based DAC** connected to the digital output pins of the Raspberry Pi Pico.

This approach was selected for its simplicity and accessibility during the laboratory implementation.

For a production-oriented design or an application requiring better-controlled conversion performance, a dedicated DAC IC would be a preferable alternative.

---

## Hardware Interface

The main pin configuration used by the Arduino, C polling, and C polling + interrupts implementations is:

| Function        | Raspberry Pi Pico GPIO |
| --------------- | ---------------------: |
| DAC bit 0       |                    GP0 |
| DAC bit 1       |                    GP1 |
| DAC bit 2       |                    GP2 |
| DAC bit 3       |                    GP3 |
| DAC bit 4       |                    GP4 |
| DAC bit 5       |                    GP5 |
| DAC bit 6       |                    GP6 |
| DAC bit 7       |                    GP7 |
| Waveform button |                   GP16 |
| Keypad row 1    |                   GP18 |
| Keypad row 2    |                   GP19 |
| Keypad row 3    |                   GP20 |
| Keypad row 4    |                   GP21 |
| Keypad column 1 |                   GP22 |
| Keypad column 2 |                   GP26 |
| Keypad column 3 |                   GP27 |
| Keypad column 4 |                   GP28 |

The eight DAC pins represent the generated sample as an **8-bit digital value**, from 0 to 255.

> **Implementation note:** The C interrupt implementation uses a different DAC/output implementation in its current source code and should not be interpreted as identical to the GP0–GP7 resistor DAC interface described above.

---

## How It Works

The general signal-generation process can be summarized as follows:

```mermaid
flowchart LR
    A[User Input] --> B[Parameter Configuration]
    B --> C[Waveform Calculation]
    C --> D[Digital Sample]
    D --> E[8-bit Digital Output]
    E --> F[Analog Conversion]
    F --> G[Output Conditioning]
    G --> H[Generated Signal]
```

The user first selects or modifies the signal parameters. The selected waveform is then calculated digitally, converted into sample values, and sent to the output stage.

Depending on the implementation, user input is handled through polling, interrupts, or a combination of both.

---

## Waveforms

The generator supports four waveform types.

### Sine

<img src="media/sine.jpeg" width="480">

The sine waveform is calculated from the configured amplitude, frequency, and DC offset.

### Triangle

<img src="media/triangular.jpeg" width="480">

The triangle waveform is generated using a periodic triangular mathematical function.

### Sawtooth

<img src="media/sawtooth.jpeg" width="480">

The sawtooth waveform is generated from the phase of the signal.

### Square

<img src="media/square.jpeg" width="480">

The square waveform alternates between its two signal levels according to the waveform phase.

---

## User Interface

The **4×4 matrix keypad** is used to enter numerical parameters.

The available configuration process allows the user to modify:

* Amplitude
* DC offset
* Frequency

The dedicated pushbutton changes the currently selected waveform.

The exact input-handling mechanism depends on the implementation:

* Arduino: polling
* MicroPython: keypad polling + button interrupt
* C polling: polling
* C interrupts: interrupt-based input handling
* C polling + interrupts: keypad polling + button interrupt logic

---

# Software Implementations

The project contains five implementations of the same general signal-generator concept.

## 1. Arduino

**Directory:** `DSG_Arduino/`

The Arduino implementation was developed using the Arduino IDE environment.

It uses polling for both the keypad and waveform-selection button.

### Characteristics

* Raspberry Pi Pico / RP2040
* 4×4 keypad
* Pushbutton polling
* 8-bit parallel DAC output
* Four waveform types
* Serial communication at 115200 baud
* Approximately 10 samples per waveform cycle
* Uses `micros()` for timing

### Flowchart

<img src="flowcharts/Arduino.png" width="480">

### Source

`DSG_Arduino/GDSv14.ino`

---

## 2. MicroPython

**Directory:** `DSG_Micropython/`

The MicroPython implementation uses a hybrid input architecture.

The keypad is continuously scanned through polling, while the waveform-selection button uses a GPIO interrupt.

Unlike the Arduino and the main C implementations, this version generates its output through **PWM on GP15** rather than the 8-bit parallel resistor DAC interface.

### Characteristics

* MicroPython
* Keypad polling
* Button interrupt
* PWM output on GP15
* Four waveform types
* Configurable amplitude
* Configurable DC offset
* Configurable frequency

### Flowchart

<img src="flowcharts/MicroPython.png" width="480">

### Source

`DSG_Micropython/GDS_Micropython.py`

---

## 3. C with Polling

**Directory:** `DSG_C_POL/`

The C polling implementation was developed using the Raspberry Pi Pico SDK.

All user inputs are handled through polling.

The waveform is calculated and its samples are sent to the external digital output interface.

### Characteristics

* C
* Raspberry Pi Pico SDK
* Keypad polling
* Button polling
* 8-bit parallel DAC output through GP0–GP7
* Four waveform types
* USB standard I/O enabled
* Block-based waveform generation

### Flowchart

<img src="flowcharts/C_Polling.png" width="480">

### Source

`DSG_C_POL/main.c`

---

## 4. C with Interrupts

**Directory:** `DSG_C_INT/`

This implementation explores interrupt-based input handling.

GPIO interrupts are used for the waveform button and keypad input events.

This version is structurally different from the polling implementations and was developed to evaluate the added complexity introduced by interrupt-driven control.

### Characteristics

* C
* Raspberry Pi Pico SDK
* GPIO interrupts
* Interrupt-based keypad/button handling
* Four waveform types
* USB standard I/O enabled
* Interrupt callbacks for user input

> **Implementation note:** The current source uses a different DAC/output mechanism from the GP0–GP7 resistor-DAC interface used by the C polling implementation. The README documents the implementation as it currently exists rather than treating the different output mechanisms as equivalent.

### Flowchart

<img src="flowcharts/C_Interrupt.png" width="480">

### Source

`DSG_C_INT/main.c`

---

## 5. C with Polling + Interrupts

**Directory:** `DSG_C_INT_POL/`

This implementation combines both input-handling techniques.

The waveform-selection button is handled through interrupt logic, while the keypad continues to be handled through polling.

The implementation also retains polling logic for the button in the main program flow.

### Characteristics

* C
* Raspberry Pi Pico SDK
* Keypad polling
* Interrupt-based button logic
* Button handling present in both interrupt and polling paths
* 8-bit parallel DAC output through GP0–GP7
* Four waveform types
* USB standard I/O enabled
* Block-based waveform generation

### Flowchart

<img src="flowcharts/C_polling_interrupt.png" width="480">

### Source

`DSG_C_INT_POL/main.c`

---

## Implementation Comparison

The project allows the different programming approaches to be compared under the same general application.

| Implementation         | Input handling                    | Output approach                        | Reported difficulty |
| ---------------------- | --------------------------------- | -------------------------------------- | ------------------: |
| Arduino                | Polling                           | 8-bit digital output                   |                8/10 |
| MicroPython            | Keypad polling + button interrupt | PWM                                    |                7/10 |
| C Polling              | Polling                           | 8-bit digital output                   |                7/10 |
| C Interrupts           | Interrupts                        | Current implementation-specific output |                4/10 |
| C Polling + Interrupts | Polling + interrupts              | 8-bit digital output                   |                3/10 |

The difficulty rating follows the laboratory report, where **1 represents the hardest implementation and 10 the easiest**.

The results illustrate the trade-off between simplicity and low-level control:

* Arduino provides a relatively accessible development environment.
* MicroPython simplifies software development but introduces additional runtime overhead.
* C provides lower-level control of the RP2040.
* Interrupt-based implementations introduce additional complexity.
* Combining polling and interrupts increases architectural complexity but provides a more flexible event-handling approach.

---

## Experimental Results

The following results were reported during laboratory testing.

| Implementation         | Difficulty | Experimental maximum frequency |
| ---------------------- | ---------: | -----------------------------: |
| Arduino                |       8/10 |                        ~10 kHz |
| MicroPython            |       7/10 |                         ~1 kHz |
| C Polling              |       7/10 |                        ~10 kHz |
| C Interrupts           |       4/10 |                        ~10 kHz |
| C Polling + Interrupts |       3/10 |                        ~10 kHz |

An additional observation during visualization reported approximately **16 kHz** for the C polling implementation under a specific test condition.

These values are **experimental results from the laboratory implementation**, not guaranteed maximum frequencies of the Raspberry Pi Pico or of the programming languages themselves.

### Reported RAM Usage

The laboratory report also included approximate RAM usage measurements:

| Implementation         | Approximate RAM usage |
| ---------------------- | --------------------: |
| Arduino                |                 ~2 kB |
| MicroPython            |                 ~8 kB |
| C Polling              |               ~1.5 kB |
| C Interrupts           |                 ~2 kB |
| C Polling + Interrupts |                 ~3 kB |

These values should be interpreted as **approximate experimental measurements reported for the implementations**, rather than a complete characterization of the runtime memory footprint.

---

## Key Takeaways

This project demonstrates how the same embedded application can be implemented using substantially different programming approaches.

### Arduino

Arduino provides a simple development environment and relatively straightforward hardware interaction, making it suitable for rapid implementation.

### MicroPython

MicroPython reduces development complexity and allows rapid experimentation, but its interpreted runtime results in lower experimentally observed signal-generation performance compared with the C implementations.

### C

The C implementations provide more direct control over the RP2040 hardware and allow the use of lower-level mechanisms such as GPIO interrupts.

### Polling vs. Interrupts

Polling is simpler to understand and implement, while interrupts allow the processor to respond to external events without continuously checking the corresponding input.

The laboratory results show that interrupt-based implementations require greater development complexity, particularly when combined with other event-handling mechanisms.

---

## Repository Structure

```text
RP2024-Digital-Signal-Generator/
│
├── DSG_Arduino/
│   └── GDSv14.ino
│
├── DSG_Micropython/
│   └── GDS_Micropython.py
│
├── DSG_C_POL/
│   ├── main.c
│   ├── CMakeLists.txt
│   ├── pico_sdk_import.cmake
│   ├── build/
│   └── doc/
│
├── DSG_C_INT/
│   ├── main.c
│   ├── CMakeLists.txt
│   ├── pico_sdk_import.cmake
│   ├── build/
│   ├── doc/
│   └── doc_doxy/
│
├── DSG_C_INT_POL/
│   ├── main.c
│   ├── CMakeLists.txt
│   ├── pico_sdk_import.cmake
│   ├── build/
│   └── doc/
│
├── flowcharts/
│   ├── Arduino.png
│   ├── MicroPython.png
│   ├── C_Interrupt.png
│   ├── C_Polling.png
│   └── C_polling_interrupt.png
│
├── media/
│   ├── DAC_resistors.png
│   ├── protoboard.jpeg
│   ├── sawtooth.jpeg
│   ├── schematic.jpeg
│   ├── sine.jpeg
│   ├── square.jpeg
│   └── triangular.jpeg
│
└── README.md
```

---

## Documentation

The complete laboratory report contains the theoretical background, hardware description, implementation details, development obstacles, experimental results, and conclusions associated with the project.

The repository also includes flowcharts for each implementation and images documenting the hardware prototype and generated waveforms.

---

## References

The project documentation was developed using the following references:

1. Monk, S. (2014). *Programming Arduino: Getting Started with Sketches* (2nd ed.).
2. Mazidi, M. A., Naimi, S., & Naimi, S. (2010). *The AVR Microcontroller and Embedded Systems: Using Assembly and C*.
3. Horowitz, P., & Hill, W. (2015). *The Art of Electronics* (3rd ed.).
4. Williams, T. *The Circuit Designer's Companion*.
5. Catsoulis, J. (2005). *Designing Embedded Hardware*.

---

## License

This project is released under the MIT License.

See `LICENSE` for the complete license text.
