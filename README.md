Otodoor

Otodoor is a smart door system developed using an Arduino and the MFRC522 RFID module. This system is designed to control access by scanning RFID cards. It logs card data, controls access, and allows authorized users to open a door or trigger a relay, providing a simple yet effective solution for managing secured entries.
Features

    RFID-based Access Control: Uses the MFRC522 module to read RFID card data.
    EEPROM Storage: Stores authorized card data in the EEPROM to persist access control settings even after power cycles.
    Master Card System: A master card can be used to add new authorized cards or reset the system.
    Predefined UID Cards: Predefined card UIDs can be loaded into the system for instant access control.
    Relay Control: Triggers a relay to simulate opening a door when an authorized card is scanned.
    Debounce Feature: Prevents multiple reads of the same card within a short period to avoid false triggers.

Hardware Requirements

    Arduino (Uno or compatible)
    MFRC522 RFID module
    Relay module (for controlling the door mechanism)
    Buzzer (optional for auditory feedback)
    LEDs (optional for visual feedback)
    Jump wires and a breadboard

Software Requirements

    Arduino IDE or PlatformIO
    MFRC522 library (GitHub link)
    EEPROM library (comes pre-installed with the Arduino IDE)

Setup Instructions

    Connect the MFRC522 RFID module to the Arduino:
        SDA to pin 10
        SCK to pin 13
        MOSI to pin 11
        MISO to pin 12
        GND to Ground
        RST to pin 9
        3.3V to 3.3V pin on Arduino

    Upload the Arduino code from the repository to your Arduino.

    After uploading the code, use the master card to add new cards. Predefined UIDs are stored in EEPROM to provide initial authorized access.

Usage

    Scan a card: If it matches a predefined or stored UID, the relay will activate, allowing access.
    To add a new card: Use the master card to put the system in add mode, then scan the new card.
    To reset the system: Scan the reset card to clear all stored UIDs from the EEPROM.

Contribution

Feel free to contribute by submitting issues, suggesting new features, or opening pull requests.
License

This project is open-source and licensed under the MIT License. See the LICENSE file for more information.

This draft can be customized further based on specific functionalities or additional information you'd like to include!
