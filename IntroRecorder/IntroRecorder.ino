// Record sound as raw data to a SD card, and play it back.
//
// Requires the audio shield:
//   http://www.pjrc.com/store/teensy3_audio.html
//
// Three pushbuttons need to be connected:
//   Record Button: pin 0 to GND
//   Stop Button:   pin 1 to GND
//   Play Button:   pin 2 to GND
//
// This example code is in the public domain.

#include <Bounce2.h>
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// GUItool: begin automatically generated code
AudioInputI2S            i2s2;           //xy=105,63
AudioAnalyzePeak         peak1;          //xy=278,108
AudioRecordQueue         queue1;         //xy=281,63
AudioPlaySdRaw           playRaw1;       //xy=302,157
AudioOutputI2S           i2s1;           //xy=470,120
AudioConnection          patchCord1(i2s2, 0, queue1, 0);
AudioConnection          patchCord2(i2s2, 0, peak1, 0);
AudioConnection          patchCord3(playRaw1, 0, i2s1, 0);
AudioConnection          patchCord4(playRaw1, 0, i2s1, 1);
AudioControlSGTL5000     sgtl5000_1;     //xy=265,212
// GUItool: end automatically generated code

// For a stereo recording version, see this forum thread:
// https://forum.pjrc.com/threads/46150?p=158388&viewfull=1#post158388

// A much more advanced sound recording and data logging project:
// https://github.com/WMXZ-EU/microSoundRecorder
// https://github.com/WMXZ-EU/microSoundRecorder/wiki/Hardware-setup
// https://forum.pjrc.com/threads/52175?p=185386&viewfull=1#post185386

//Enum for sketch state
enum recordingState {
  notRecording,
  currentlyRecording,
  playbackActive,
};

// Bounce objects to easily and reliably read the buttons
Bounce2::Button button = Bounce2::Button(); // INSTANTIATE A Bounce2::Button OBJECT


// which input on the audio shield will be used?
//const int myInput = AUDIO_INPUT_LINEIN;
const int myInput = AUDIO_INPUT_MIC;

// Use these with the Teensy Audio Shield
#define SDCARD_CS_PIN    10
#define SDCARD_MOSI_PIN  11  // Audio Sheild Rev. D2
#define SDCARD_SCK_PIN   13  // Audio Sheild Rev. D2

// Remember which mode we're doing
recordingState mode = notRecording;

// The file where data is recorded
File frec;

//Counter to store the SD Card filenames
long filenameCounter = 0;

void setup() {
  button.attach(1, INPUT_PULLUP);
  button.interval(15);
  button.setPressedState(HIGH);
  button.update();

  // Audio connections require memory, and the record queue
  // uses this memory to buffer incoming audio.
  AudioMemory(60);

  // Enable the audio shield, select input, and enable output
  sgtl5000_1.enable();
  sgtl5000_1.inputSelect(myInput);
  sgtl5000_1.volume(70);
  sgtl5000_1.micGain(20);

  // Initialize the SD card
  SPI.setMOSI(SDCARD_MOSI_PIN);
  SPI.setSCK(SDCARD_SCK_PIN);
  if (!(SD.begin(SDCARD_CS_PIN))) {
    // stop here if no SD card, but print a message
    while (1) {
      Serial.println("Unable to access the SD card");
      delay(500);
    }
  }
}


void loop() {
  // First, read the button
  button.update();
  //Step 0 - In order to do anything the receiver needs to be picked up
  if (button.pressed()) {
  }

  if (button.isPressed()) {
    if (mode == notRecording) {
      startRecording();
    }
  }

  if (button.released()) {
    stopRecording();
  }

  // If we're playing or recording, carry on...
  if (mode == currentlyRecording) {
    continueRecording();
  }

  // when using a microphone, continuously adjust gain
  if (myInput == AUDIO_INPUT_MIC) adjustMicLevel();
}


void startRecording() {
  Serial.println("startRecordingCalled");
  if (mode == notRecording) {
    Serial.println("startRecordingStarted");
    if (SD.exists("INTRO.RAW")) {
      // The SD library writes new data to the end of the
      // file, so to start a new recording, the old file
      // must be deleted before new data is written.
      SD.remove("INTRO.RAW");
    }
    frec = SD.open("INTRO.RAW", FILE_WRITE);
    if (frec) {
      queue1.begin();
      mode = currentlyRecording;
    }
  }
}

void continueRecording() {
  if (mode == currentlyRecording) {
    if (queue1.available() >= 2) {
      byte buffer[512];
      // Fetch 2 blocks from the audio library and copy
      // into a 512 byte buffer.  The Arduino SD library
      // is most efficient when full 512 byte sector size
      // writes are used.
      memcpy(buffer, queue1.readBuffer(), 256);
      queue1.freeBuffer();
      memcpy(buffer+256, queue1.readBuffer(), 256);
      queue1.freeBuffer();
      // write all 512 bytes to the SD card
      //elapsedMicros usec = 0;
      frec.write(buffer, 512);
      // Uncomment these lines to see how long SD writes
      // are taking.  A pair of audio blocks arrives every
      // 5802 microseconds, so hopefully most of the writes
      // take well under 5802 us.  Some will take more, as
      // the SD library also must write to the FAT tables
      // and the SD card controller manages media erase and
      // wear leveling.  The queue1 object can buffer
      // approximately 301700 us of audio, to allow time
      // for occasional high SD card latency, as long as
      // the average write time is under 5802 us.
      //Serial.print("SD write, us=");
      //Serial.println(usec);
    }
  }
}

void stopRecording() {
  Serial.println("stopRecordingCalled");
  if (mode == currentlyRecording) {
    Serial.println("stopRecordingStopped");
    queue1.end();
    while (queue1.available() > 0) {
      frec.write((byte*)queue1.readBuffer(), 256);
      queue1.freeBuffer();
    }
    frec.close();
    mode = notRecording;
  }
}


void adjustMicLevel() {
  // TODO: read the peak1 object and adjust sgtl5000_1.micGain()
  // if anyone gets this working, please submit a github pull request :-)
}



