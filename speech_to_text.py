import serial
import requests
import base64
import time
import wave

PORT = "COM5"
BAUD = 921600
SAMPLE_RATE = 16000
RECORD_SECONDS = 5
SAMPLE_WIDTH = 2
API_KEY = "PUT_YOUR_GOOGLE_API_KEY_HERE"
SAVE_RECORDING = True

def google_speech_to_text(audio):
    encoded = base64.b64encode(audio).decode("utf-8")

    body = {
        "config": {
            "encoding": "LINEAR16",
            "sampleRateHertz": SAMPLE_RATE,
            "languageCode": "en-US"
        },
        "audio": {
            "content": encoded
        }
    }

    url = "https://speech.googleapis.com/v1/speech:recognize"

    print("Sending to Google...")

    response = requests.post(
        url,
        params={"key": API_KEY},
        json=body,
        timeout=30
    )

    print("HTTP:", response.status_code)

    if response.status_code != 200:
        print("Google API error:")
        print(response.text)
        return None

    result = response.json()

    if "results" not in result:
        print("No speech detected.")
        print(result)
        return None

    return result["results"][0]["alternatives"][0]["transcript"]

def save_wav(audio):
    with wave.open("recording.wav", "wb") as wav:
        wav.setnchannels(1)
        wav.setsampwidth(SAMPLE_WIDTH)
        wav.setframerate(SAMPLE_RATE)
        wav.writeframes(audio)

    print("Saved recording.wav")

def main():
    print("Opening", PORT)

    ser = serial.Serial(
        PORT,
        BAUD,
        timeout=10
    )

    time.sleep(2)

    print("Connected.")
    print("Waiting for ESP32...")

    while True:
        line = ser.readline()

        if not line:
            continue

        try:
            message = line.decode(errors="ignore").strip()
        except Exception:
            continue

        print("ESP32:", message)

        if message != "START_AUDIO":
            continue

        expected_bytes = SAMPLE_RATE * RECORD_SECONDS * SAMPLE_WIDTH
        audio = bytearray()

        print()
        print("Recording...")
        print("Speak now!")

        while len(audio) < expected_bytes:
            remaining = expected_bytes - len(audio)
            data = ser.read(min(4096, remaining))

            if data:
                audio.extend(data)
                percentage = len(audio) / expected_bytes * 100
                print(f"\rReceived: {percentage:.1f}%", end="", flush=True)

        print()
        print("Audio received.")

        if SAVE_RECORDING:
            save_wav(bytes(audio))

        text = google_speech_to_text(bytes(audio))

        if text is None:
            ser.write(b"NO_SPEECH\n")
            ser.flush()
            print("Sent NO_SPEECH")
            continue

        print()
        print("Recognized:")
        print(text)

        ser.write((text + "\n").encode("utf-8"))
        ser.flush()

        print("Sent to ESP32.")

if __name__ == "__main__":
    main()
