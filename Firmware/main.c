// Chunks especially the pin management of this file is written by Gemini

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SPI.h>
#include <SD.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SoftwareSerial.h>

#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1 // Reset pin (-1 if sharing Arduino reset pin)

// Create screen instance (0x3C is standard I2C address for SSD1306)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// RX Pin 8, TX Pin 9 (TX connects to MP3 RX pin via a 1k resistor)
SoftwareSerial mp3Serial(8, 9);

const int SD_CS_PIN = 10; // pin config for SD CS

// Global Variables
int currentTrackIndex = 0;
int isPlaying = 0; // 1 is playing, 0 is not playing
int total_tracks = 0; 
int currentVolume = 15;
const int MIN_VOLUME = 0;
const int MAX_VOLUME = 30;

struct Data {
    char title[22]; // max characters shown by the screen are 21
    char artist[22]; // max characters shown by the screen are 21
    char index[16]; // Thats how many we need
};

// Prototypes for funcs
void count_total_tracks(const char *filename, int *total_tracks_ptr);
struct Data track_details(int index);
void next_track(int *currentTrackIndex_ptr);
void prev_track(int *currentTrackIndex_ptr);
void pause_play(int *playing_ptr);
void volume_up(int *volume_ptr);
void volume_down(int *volume_ptr);
void send_mp3_command(uint8_t command, uint16_t parameter);
void mp3_play_track(int trackIndex);
void mp3_pause();
void mp3_resume();
void mp3_set_volume(int volume);
void render_screen(struct Data track, int is_playing, int current_vol, int total);

const int NEXT_PIN       = 6;
const int PREV_PIN       = 4;
const int PLAY_PAUSE_PIN = 5;
const int VOL_UP_PIN     = 2;
const int VOL_DOWN_PIN   = 3;

// Timers for Debouncing
unsigned long lastDebounceTime = 0;
const unsigned long DEBOUNCE_DELAY = 150; // Milliseconds to wait between button reads

void setup_pins() {
    // Configure pins as inputs with internal pullup resistors.
    // When the button is NOT pressed pin reads HIGH (1).
    // When the button IS pressed (connected to Ground) pin reads LOW (0).
    pinMode(NEXT_PIN, INPUT_PULLUP);
    pinMode(PREV_PIN, INPUT_PULLUP);
    pinMode(PLAY_PAUSE_PIN, INPUT_PULLUP);
    pinMode(VOL_UP_PIN, INPUT_PULLUP);
    pinMode(VOL_DOWN_PIN, INPUT_PULLUP);
}

void setup_display() {
    // 0x3C is the default I2C address for 0.96" OLEDs
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println(F("SSD1306 allocation failed"));
        for (;;); // Don't proceed, loop forever if display isn't wired
    }
    
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);
    display.setCursor(10, 25);
    display.println("MP3 PLAYER BOOTING");
    display.display(); // Sends the RAM buffer to the physical screen
}

void setup() {
    // Start hardware Serial
    Serial.begin(115200);
    while (!Serial) { ; } // Wait for serial console to open

    Serial.println("Booting MiniPod Firmware...");

    setup_pins();
    setup_display();
    mp3Serial.begin(9600);
    delay(500);
    mp3_set_volume(currentVolume);

    // Initialize physical SD card hardware
    if (!SD.begin(SD_CS_PIN)) {
        Serial.println("Error: SD Card Initialization Failed!");
        return;
    }
    Serial.println("SD Card Initialized successfully.");

    // Count available songs on the SD card during startup
    count_total_tracks("index.csv", &total_tracks);
    Serial.print("Total Tracks Found: ");
    Serial.println(total_tracks);

    // Fetch details for the first track (Index 0)
    struct Data initialTrack = track_details(currentTrackIndex);
    Serial.print("Initial Track: ");
    Serial.println(initialTrack.title);
    
    render_screen(initialTrack, isPlaying, currentVolume, total_tracks);
}

void check_buttons() {
    unsigned long currentMillis = millis();

    if (currentMillis - lastDebounceTime > DEBOUNCE_DELAY) {
        bool state_changed = false;

        // 1. Next Track
        if (digitalRead(NEXT_PIN) == LOW) {
            next_track(&currentTrackIndex);
            mp3_play_track(currentTrackIndex); // Send UART Command to Module
            isPlaying = 1;
            state_changed = true;
        } 
        // 2. Previous Track
        else if (digitalRead(PREV_PIN) == LOW) {
            prev_track(&currentTrackIndex);
            mp3_play_track(currentTrackIndex); // Send UART Command to Module
            isPlaying = 1;
            state_changed = true;
        } 
        // 3. Play / Pause Toggle
        else if (digitalRead(PLAY_PAUSE_PIN) == LOW) {
            pause_play(&isPlaying);
            if (isPlaying) {
                mp3_resume();
            } else {
                mp3_pause();
            }
            state_changed = true;
        } 
        // 4. Volume Up
        else if (digitalRead(VOL_UP_PIN) == LOW) {
            volume_up(&currentVolume);
            mp3_set_volume(currentVolume); // Send UART Volume Command
            state_changed = true;
        } 
        // 5. Volume Down
        else if (digitalRead(VOL_DOWN_PIN) == LOW) {
            volume_down(&currentVolume);
            mp3_set_volume(currentVolume); // Send UART Volume Command
            state_changed = true;
        }

        // Re-render UI frame on OLED display
        if (state_changed) {
            struct Data current_track = track_details(currentTrackIndex);
            render_screen(current_track, isPlaying, currentVolume, total_tracks);
            lastDebounceTime = currentMillis;
        }
    }
}

void render_screen(struct Data track, int is_playing, int current_vol, int total) {
    display.clearDisplay(); // Always clear previous frame first!

    // --- Line 1: Header (Track position & Volume) ---
    display.setTextSize(1);
    display.setCursor(0, 0);
    // Print formatted track position: e.g. [001/012]
    display.print("[");
    display.print(track.index);
    display.print("/");
    if (total < 10) display.print("0");
    if (total < 100) display.print("0");
    display.print(total);
    display.print("] ");

    // Print volume on top right
    display.setCursor(80, 0);
    display.print("Vol:");
    display.print(current_vol);

    // --- Divider Line ---
    display.drawLine(0, 10, 128, 10, SSD1306_WHITE);

    // --- Line 2: Track Title ---
    display.setCursor(0, 16);
    display.setTextSize(1); // Fits up to 21 characters
    display.print(track.title);

    // --- Line 3: Artist Name ---
    display.setCursor(0, 32);
    display.print(track.artist);

    // --- Divider Line ---
    display.drawLine(0, 48, 128, 48, SSD1306_WHITE);

    // --- Line 4: Play / Pause Status ---
    display.setCursor(0, 52);
    if (is_playing) {
        display.print("Status: PLAYING >");
    } else {
        display.print("Status: PAUSED ||");
    }

    // Refresh the physical display hardware with new buffer data
    display.display(); 
}

// Helper function to send raw 10-byte command packets to the MP3 module
void send_mp3_command(uint8_t command, uint16_t parameter) {
    // Calculate 16-bit Checksum required by the module protocol
    uint16_t checksum = -(0xFF + 0x06 + command + 0x01 + (uint8_t)(parameter >> 8) + (uint8_t)parameter);

    // Build the 10-byte array
    uint8_t commandPacket[10] = {
        0x7E,                           // Start Byte
        0xFF,                           // Version
        0x06,                           // Length
        command,                        // Command Code (e.g. 0x03 for Play Track)
        0x01,                           // Feedback required
        (uint8_t)(parameter >> 8),      // Parameter High Byte
        (uint8_t)(parameter & 0xFF),    // Parameter Low Byte
        (uint8_t)(checksum >> 8),       // Checksum High Byte
        (uint8_t)(checksum & 0xFF),     // Checksum Low Byte
        0xEF                            // End Byte
    };

    // Transmit all 10 bytes sequentially over the serial pipeline
    for (int i = 0; i < 10; i++) {
        mp3Serial.write(commandPacket[i]);
    }
}

void loop() {
    // Continuously check hardware buttons
    check_buttons();
}

void count_total_tracks(const char *filename, int *total_tracks_ptr) {
    File file = SD.open(filename, FILE_READ);
    if (!file) {
        *total_tracks_ptr = 0; // Dereference pointer value is 0
        return;
    }

    int count = 0;

    // Skip the header row
    if (file.available()) {
        file.readStringUntil('\n');
        while (file.available()) {
            file.readStringUntil('\n');
            count++;
        }
    }

    file.close();

    // Store count back via pointer    
    *total_tracks_ptr = count;
}

struct Data track_details(int index){
    struct Data info;
    char title[22] = "Unknown";
    char artist[22] = "Unknown";
    char target_index[16];
    sprintf(target_index, "%03d", index);

    strncpy(info.title, title, sizeof(info.title) - 1);
    strncpy(info.artist, artist, sizeof(info.artist) - 1);
    strncpy(info.index, target_index, sizeof(info.index) - 1);
    
    info.title[sizeof(info.title) - 1] = '\0';
    info.artist[sizeof(info.artist) - 1] = '\0';
    info.index[sizeof(info.index) - 1] = '\0';

    File Details = SD.open("index.csv", FILE_READ);
    if (!Details){
        strncpy(info.title, "SD CARD NOT FOUND.", sizeof(info.title) - 1);
        return info;
    }

    while (Details.available()) {
        String line = Details.readStringUntil('\n');
        line.trim();

        char buffer[128];
        line.toCharArray(buffer, sizeof(buffer));

        char *file_index = strtok(buffer, ","); // dividing data based on commas

        if (file_index != NULL && strcmp(file_index, target_index) == 0){
            char *file_title = strtok(NULL, ",");
            char *file_artist = strtok(NULL, ",");
            if (file_title) strncpy(info.title, file_title, sizeof(info.title) - 1);
            if (file_artist) strncpy(info.artist, file_artist, sizeof(info.artist) - 1);
            strncpy(info.index, file_index, sizeof(info.index) - 1);
            break;
        }
    }
    Details.close();
    return info;
}

void next_track(int *currentTrackIndex_ptr){
    *currentTrackIndex_ptr = (*currentTrackIndex_ptr + 1) % total_tracks;
}

void prev_track(int *currentTrackIndex_ptr){
    if ((*currentTrackIndex_ptr - 1) < 0){
        *currentTrackIndex_ptr = total_tracks - 1;
    }
    else{
        *currentTrackIndex_ptr = *currentTrackIndex_ptr - 1;
    }
}

void pause_play(int *playing_ptr){
    *playing_ptr = !(*playing_ptr);
}

void volume_up(int *volume_ptr) {
    if (*volume_ptr < MAX_VOLUME) {
        *volume_ptr += 1;
    }
}

void volume_down(int *volume_ptr) {
    if (*volume_ptr > MIN_VOLUME) {
        *volume_ptr -= 1;
    }
}

// Play specific track index (e.g., track index 0 becomes file 000.mp3 or track #1)
void mp3_play_track(int trackIndex) {
    // Command 0x03 = Play Track by Index Number (1-based index: track 0 is file 001.mp3)
    send_mp3_command(0x03, trackIndex + 1); 
}

// Pause audio playback
void mp3_pause() {
    send_mp3_command(0x0E, 0); // Command 0x0E = Pause
}

// Resume audio playback
void mp3_resume() {
    send_mp3_command(0x0D, 0); // Command 0x0D = Play / Resume
}

// Set Hardware Volume (0 to 30)
void mp3_set_volume(int volume) {
    send_mp3_command(0x06, volume); // Command 0x06 = Set Volume
}