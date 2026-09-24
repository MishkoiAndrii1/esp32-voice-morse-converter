[README.md](https://github.com/user-attachments/files/32597881/README.md)
# ESP32 Voice ↔ Morse Converter

A dual-mode Morse code converter built on ESP32 (ENSC 100, Simon Fraser University). One mode decodes manual button-tap Morse input into text; the other captures live speech, sends it to Google's Speech-to-Text API, and encodes the recognized words back into Morse output.

## Features

**Mode 1 — Morse → Text**
- Dot/dash button input classified by press duration (300 ms threshold)
- Automatic letter completion after a 1500 ms pause — no manual delimiters needed
- Real-time OLED (SSD1306) feedback as each letter is decoded

**Mode 2 — Voice → Morse**
- Live audio captured via an **I2S digital microphone (ICS43434)** on the ESP32
- Raw 16 kHz PCM audio streamed over serial (921600 baud) to a companion Python script
- Python relays the audio to **Google's Speech-to-Text API** and returns the transcript to the ESP32
- Recognized text is encoded back into Morse and output via synchronized LED + buzzer signals

## Architecture

| File | Responsibility |
|---|---|
| `morsec.ino` | Main ESP32 sketch — mode selection, button handling, Morse encode/decode tables, LED/buzzer/OLED output |
| `I2S.h` / `I2S.cpp` | Thin wrapper around the ESP32's I2S peripheral driver for reading microphone samples |
| `speech_to_text.py` | PC-side companion script — receives streamed audio over serial, calls the Google Speech-to-Text API, sends the transcript back |

## How it works (Voice → Morse)

1. User selects "Voice → Morse" mode and presses the select button
2. ESP32 signals `START_AUDIO` over serial and streams 5 seconds of 16-bit PCM audio from the I2S mic
3. `speech_to_text.py` buffers the incoming bytes, saves a `.wav` copy, and sends the audio to Google's Speech Recognition API
4. The recognized transcript is sent back to the ESP32 over serial
5. ESP32 encodes each character of the transcript into Morse, flashing the LED and sounding the buzzer (short pulse for dot, long for dash) while showing progress on the OLED

## Hardware

- ESP32 dev board
- I2S digital microphone (ICS43434)
- SSD1306 OLED display (128×64, I2C)
- 2 pushbuttons (dot/dash) + 1 select button
- LED + piezo buzzer

## Setup

1. Flash `morsec.ino` to the ESP32 (requires the Adafruit_GFX and Adafruit_SSD1306 libraries)
2. Install Python dependencies: `pip install pyserial requests`
3. Add a Google Cloud Speech-to-Text API key to `speech_to_text.py`
4. Update `PORT` in `speech_to_text.py` to match your ESP32's serial port
5. Run `speech_to_text.py`, then select "Voice → Morse" mode on the device
