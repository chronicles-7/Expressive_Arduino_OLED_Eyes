# Expressive Arduino OLED Eyes 🤖

An expressive robot-eye animation project built using an Arduino-compatible microcontroller and a small OLED display. Create a cute robot face with blinking eyes, gentle eye movements, and expressive animations!

## ✨ Features

- 👀 Animated robot eyes on an OLED display
- 😉 Automatic blinking
- 👁️ Gentle random eye movements
- 😠 Angry eye expression
- ⚠️ Alert animation
- 😱 Panic animation
- 😴 Sleep animation with Z symbols
- 💧 Snot bubble sleep animation
- 🔘 Button-triggered interactions

## 🧰 Components

- Arduino-compatible microcontroller
- 128 × 64 OLED display using an SH1106-compatible controller
- Push button
- Jumper wires
- Breadboard (optional)

**Note:** Check your actual OLED controller and wiring before building. The sketch is designed around a 128 × 64 SH1106-style display.

## 🔌 Wiring

The current sketch uses digital pin **D2** for the button.

Connect the OLED's power, ground, clock (SCK/SCL), and data (SDA) pins according to your specific board and display module. Confirm the voltage requirements before connecting power.

The exact I²C pins depend on your microcontroller.

## 💻 Installation

1. Download `Expressive_Arduino_OLED_Eyes.ino` from this repository.
2. Install and open the Arduino IDE.
3. Connect your microcontroller to your computer.
4. Install the required OLED graphics and display libraries, if they are not already installed.
5. Open the `.ino` sketch in Arduino IDE.
6. Select the correct board and port.
7. Compile the sketch and upload it to your board.
8. Check that the OLED is wired correctly if the display stays blank.

## 🎨 Animations

The project includes multiple facial expressions and animation states, including:

- Calm, gently moving eyes
- Blinking
- Angry eyes
- Alert and panic reactions
- Sleepy eyes and sleep effects

## 🛠️ Customization

You can customize the eye expressions, animation timing, movement range, and button behavior by editing the Arduino sketch.

## 🚧 Project Status

This project is a work in progress. Hardware compatibility and display behavior may vary depending on the microcontroller, OLED module, and wiring.

## 📜 License

No license has been specified yet. Please check the repository for licensing information before reusing or redistributing this project's code.

---

Made with ❤️ for expressive little robots.

## 🔌 Wiring Diagram

### OLED Display (I²C)

| OLED Pin to Arduino Nano |
|---|---|
| GND to GND |
| VCC to 5V, only if supported by the module |
| SDA to A4 / SDA |
| SCK / SCL to A5 / SCL |

### Push Button

| Button Terminal | Arduino Nano |
|---|---|
| Terminal 1 to D2 |
| Terminal 2 to GND |

The sketch configures D2 as `INPUT_PULLUP`, so an external pull-up resistor is not required.

**Important:** Confirm your OLED's pin labels and voltage requirements before powering it. Other Arduino-compatible boards may use different I²C pins.
