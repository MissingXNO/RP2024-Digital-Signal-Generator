/**
 * @file GDSv14.ino
 *
 * @mainpage Signal Generator Project
 *
 * @section description Description
 * This program turns the Raspberry Pi Pico (RP2040) into a programmable signal generator.
 * The code includes support to input peripherals such as a push-button or a 4x4 Keypad.
 * The output is given as 8-bits that should be fed to an 8-bit DAC, in order to obtain 
 * an analogue signal. The push-button is used to change between waveformes (sine, square, sawtooth and triangle), 
 * while 4x4 Keypad can be used to introduce the desired amplitud, frequency and offset values.
 *
 * @section circuit Circuit
 * - Push-button connected to GP16
 * - Keypad Rows connected like    (r1, r2, r3, r4) to (GP18, GP19, GP20, GP21) . Could be changed from here, as long as initialization is this case does NOT required consecutive pins
 * - Keypad columns connected like (c1, c2, c3, c4) to (GP22, GP26, GP27, GP28) . Could be changed from here, as long as initialization is this case does NOT required consecutive pins
 *
 * @section libraries Libraries
 * - math.h (https://pubs.opengroup.org/onlinepubs/009695399/basedefs/math.h.html)
 *   - Useful to perform mathematical operations, necessary to build function equations.
 *
 * @section notes Notes
 * - This signal generator works implementing polling methodology.
 * - Default values can be changed with testing purposes. 
 *
 * @section todo ToDo
 * - This code and its structure could be further optimized.
 *
 * @section author Authors
 * - Created by Santiago Giraldo Tabares & Ana María Velasco Montenegro on april, 2024
 *
 *
 * Copyright (c) No-license.
 */

#include <math.h>

#define AMPLITUDE_DEFAULT 100.0    ///< Default amplitude (mV, peak)
#define AMPLITUDE_MIN 100.0       ///< Minimum amplitude (mV)
#define AMPLITUDE_MAX 2500.0      ///< Maximum amplitude (mV)
#define FREQUENCY_MAX 12000000    ///< Maximum frequency (Hz)
#define AMPLITUDE_MAX_DAC 255     ///< Máximum value for 8-bit DAC
#define VREF 3.3                  ///< Reference voltaje for 8-bit DAC

const byte ROWS = 4;             ///< Amount of rows
const byte COLS = 4;             ///< Amount of columns
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};                               ///< 4x4 array, containing each of the characters present in the Keypad Layout. The order of the characters should match the order of the keypad itself.
byte rowPins[ROWS] = {18, 19, 20, 21}; ///< Rows pinout (GPIOs) in the RP2040
byte colPins[COLS] = {22, 26, 27, 28};   ///< Columns pinout (GPIOs) in the RP2040

const byte waveformButtonPin = 16;     ///< Pushbutton GPIO. 

byte dacPins[8] = {0, 1, 2, 3, 4, 5, 6, 7}; ///< output GPIOs to be connected to the DAC. Ordered from LSB to MSB.

enum Waveform { 
SINE, ///< Sinusoidal wave. Sin(2*pi*f*t). centered aroung its DC offset
SQUARE, ///< Square wave, also called pulsed wave, symetric (50% duty cycle) centered aroung its DC offset
SAWTOOTH, ///< Sawtooth wave. Rising time matches the period, while falling time goes to zero. centered aroung its DC offset
TRIANGULAR ///< Triangular wave. Symetric (rising time equals 50% of period, falling time equals 50% of period). Centered aroung its DC offset
}; ///< Predefined signal waveforms.
Waveform currentWaveform = SINE; ///< Current waveform produced by the signal. In this line, set the default Signal waveform as Sinewave.

float amplitude = AMPLITUDE_DEFAULT; ///< Set current amplitude to DEFAULT value
float frequency = 10;     ///< Set current frequency to  DEFAULT value
float dcOffset = (AMPLITUDE_DEFAULT) / 2.0; ///< Set current DC offset to DEFAULT value.


/**
 * Initializes each pin from the Raspberry pi pico needed to output the signal. In thise case, initializez 8 pins according to the 8 bits needed in the DAC. As long as 
 * the pins are in sequence, these could be initialized with a for loop. Also, sets up the Push-button with its own Pull-up resistor.
 */
void setup_gpio() {
    for (int i = 0; i < 8; i++) {
        pinMode(dacPins[i], OUTPUT);
    }
    pinMode(waveformButtonPin, INPUT_PULLUP);
}


/**
 * Writes the value in the DAC. As long as we are using an 8-bit DAC, resulting value (well, its binary representation) should be written through 8 GPIOs. In this case, using only shifting and bitwise and the write on the correponding pin, from LSB to MSB.
 *
 * @param value   This represent the voltage of the signal in the current time. scaled between 0 and 255.
 *
 */
void write_dac(uint8_t value) {
    for (int i = 0; i < 8; i++) {
        digitalWrite(dacPins[i], (value >> i) & 1);
    }
}


/**
 * The variable "currentWaveform" changes to the NEXT waveform declared in the enum. Once the last (4th) waveform is reached, the next would be the 1st (SINE). Also prints the current waveform.
 */
void changeWaveform() {
    currentWaveform = static_cast<Waveform>((currentWaveform + 1) % 4);
    switch (currentWaveform) {
        case SINE:
            Serial.println("Forma de onda cambiada a: Seno");
            break;
        case SQUARE:
            Serial.println("Forma de onda cambiada a: Cuadrada");
            break;
        case SAWTOOTH:
            Serial.println("Forma de onda cambiada a: Diente de sierra");
            break;
        case TRIANGULAR:
            Serial.println("Forma de onda cambiada a: Triangular");
            break;
    }
}


/**
 * The standard Arduino setup function used for setup and configuration tasks. 115200 baud-rate is desired as we are working on Raspberry Pi Pico. Also prints the initial configuration of the system.
 */
void setup() {
    Serial.begin(115200);
    setup_gpio();
    Serial.println("Configuración predeterminada:");
    Serial.println("Amplitud: " + String(amplitude) + " mV");
    Serial.println("Frecuencia: " + String(frequency) + " Hz");
    Serial.println("Desplazamiento DC: " + String(dcOffset) + " mV");
}


/**
 * Function to get input from the 4x4 matrix keypad. Also stablish each column as INPUT _PULLUP, and each row as OUTPUT. Checks in every ROW/COLUMN combination and if a row and a column match being LOW, the key corresponding to both column and row was pressed. 
 *  Then, save the key, debounces and wait for the key to be released.
 * @return  The key that was pressed.
 */
char getKeypadInput() {
    char key = '\0';

    // Establecer todos los pines de columna como INPUT_PULLUP
    for (byte col = 0; col < COLS; col++) {
        pinMode(colPins[col], INPUT_PULLUP);
    }

    // Escanear los pines de fila en busca de una entrada LOW
    for (byte row = 0; row < ROWS; row++) {
        pinMode(rowPins[row], OUTPUT);
        digitalWrite(rowPins[row], LOW);

        for (byte col = 0; col < COLS; col++) {
            if (digitalRead(colPins[col]) == LOW) {
                // Tecla detectada, determinar la tecla correspondiente
                key = keys[row][col];
                delay(50); // Retraso de rebote
                while (digitalRead(colPins[col]) == LOW) {} // Esperar la liberación de la tecla
            }
        }

        // Restablecer la fila actual
        digitalWrite(rowPins[row], HIGH);
        pinMode(rowPins[row], INPUT);
    }

    return key;
}


/**
 * Only if the pressed key was an "A". Get input from the keypad and saves it in the amplitude variable. Converts it from string to float, then constrains it in the range of [AMPLITUDE_MIN, AMPLITUDE_MAX].
 */
void readAmplitude() {
    Serial.println("Ingrese la amplitud (mV) [" + String(AMPLITUDE_MIN) + "-" + String(AMPLITUDE_MAX) + "]: ");
    String amplitudeStr = "";
    char key = '\0';
    while (key != 'D') {
        key = getKeypadInput();
        if (key >= '0' && key <= '9') {
            amplitudeStr += key;
            Serial.print(key);
        }
        delay(200);
    }
    float amplitudeValue = amplitudeStr.toFloat();
    // Asegurar que la amplitud esté dentro del rango válido
    amplitude = constrain(amplitudeValue, AMPLITUDE_MIN, AMPLITUDE_MAX);
    Serial.println(" mV");
    Serial.println("Amplitud establecida en: " + String(amplitude) + " mV");
}


/**
 * Only if the pressed key was an "B". Get input from the keypad and saves it in the frequency variable. Converts it from string to float, then constrains it in the range of [FREQUENCY_MIN, FREQUENCY_MAX].
 */
void readFrequency() {
    Serial.println("Ingrese la frecuencia (Hz) [" + String(FREQUENCY_MIN) + "-" + String(FREQUENCY_MAX) + "]: ");
    String frequencyStr = "";
    char key = '\0';
    while (key != 'D') {
        key = getKeypadInput();
        if (key >= '0' && key <= '9') {
            frequencyStr += key;
            Serial.print(key);
        }
        delay(200);
    }
    float frequencyValue = frequencyStr.toFloat();
    // Asegurar que la frecuencia esté dentro del rango válido
    frequency = constrain(frequencyValue, FREQUENCY_MIN, FREQUENCY_MAX);
    Serial.println(" Hz");
    Serial.println("Frecuencia establecida en: " + String(frequency) + " Hz");
}


/**
 * Only if the pressed key was an "C". Get input from the keypad and saves it in the dcOffset variable. Converts it from string to float, then constrains it in the range of [AMPLITUDE_MIN / 2.0, AMPLITUDE_MAX / 2.0].
 */
void readDCOffset() {
    Serial.println("Ingrese el desplazamiento DC (mV) [" + String(AMPLITUDE_MIN / 2.0) + "-" + String(AMPLITUDE_MAX / 2.0) + "]: ");
    String offsetStr = "";
    char key = '\0';
    while (key != 'D') {
        key = getKeypadInput();
        if (key >= '0' && key <= '9') {
            offsetStr += key;
            Serial.print(key);
        }
        delay(200);
    }
    float offsetValue = offsetStr.toFloat();
    // Asegurar que el desplazamiento DC esté dentro del rango válido
    dcOffset = constrain(offsetValue, AMPLITUDE_MIN / 2.0, AMPLITUDE_MAX / 2.0);
    Serial.println(" mV");
    Serial.println("Desplazamiento DC establecido en: " + String(dcOffset) + " mV");
}


/**
 * This function calculates the sampling aprameters. As long as the frequency of the generated signal is not always the same, and sampling frequency should match the Nyquist therorem. Sampling frequency should not be constant. In fact, it should be calculated according to the frequency of the signal. Samples per cycle should be calculated as well.
 *
 * @param frequency   The current frequency of the signal that is being generated
 * @param samplingRate  the inverse of sampling frequency, which should be significantly higher (10 times, in this case) than the frequency of the signal
 * @param samplesPerCycle  Amount of samples required to build the signal, according to the sampling frequency.
 *
 */
void calculateSamplingParameters(float frequency, float &samplingRate, int &samplesPerCycle) {
    // Cálculo de la tasa de muestreo
    samplingRate = frequency * 10; // Ajusta este factor según sea necesario
    // Cálculo del número de muestras por ciclo
    samplesPerCycle = samplingRate / frequency;
}


/**
 * Main loop function of the program. Executes the signal generation and polling functions in cyclic way. 
 */
void loop() {
    char key = getKeypadInput();
    if (key == 'A') {
        readAmplitude();
    } else if (key == 'B') {
        readFrequency();
    } else if (key == 'C') {
        readDCOffset();
    }
    
    if (digitalRead(waveformButtonPin) == LOW) {
        delay(50); // Retraso de rebote
        if (digitalRead(waveformButtonPin) == LOW) { // Comprobar nuevamente después del retraso de rebote
            changeWaveform();
            delay(500); // Retraso de rebote
        }
    }

    float samplingRate;
    int samplesPerCycle;
    calculateSamplingParameters(frequency, samplingRate, samplesPerCycle);

    for (size_t i = 0; i < samplesPerCycle; i++) {
        float currentTime = static_cast<float>(micros()) / 1000000.0; // Obtener el tiempo actual en segundos
        float value;
        switch (currentWaveform) {
            case SINE:
                value = (amplitude*0.001*(sin(2.0 * PI * frequency * currentTime)))+(dcOffset*0.001);
                break;
            case SQUARE:
                if (sin(2.0 * PI * frequency * currentTime) >= 0.0) {
                    value = (amplitude * 0.001) + (dcOffset * 0.001); // Positive half of the square wave
                } else {
                    value = -(amplitude * 0.001) + (dcOffset * 0.001); // Negative half of the square wave
                }
                break;

            case SAWTOOTH:
                value = (frequency * currentTime - floor(frequency * currentTime)) * 2.0 - 1.0;
                value = (value * 2 * (amplitude * 0.001 / 2.0)) + dcOffset * 0.001;
                break;


            case TRIANGULAR:
                value = 2.0 * abs(2.0 * (frequency * currentTime - floor(frequency * currentTime + 0.5))) - 1.0;
                value = (value * 2 *  (amplitude * 0.001 / 2.0)) + dcOffset * 0.001;
                break;


        }
        float scaled_value = (value + 1.0) * 127.5;
        uint8_t dac_value = (uint8_t)constrain(scaled_value, 0, 255);
        write_dac(dac_value);

        // Calcular el tiempo de espera necesario para mantener la frecuencia
        float nextTime = static_cast<float>(i + 1) / samplingRate;
        unsigned long delayTime = static_cast<unsigned long>((nextTime - currentTime) * 1000000);
        if (delayTime > 0) {
            delayMicroseconds(delayTime);
        }
    }
}
