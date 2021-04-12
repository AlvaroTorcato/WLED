#pragma once

#include "wled.h"

//
// v2 Usermod to automatically save settings 
// to preset number AUTOSAVE_PRESET_NUM after a change to any of
//
// * brightness
// * effect speed
// * effect intensity
// * mode (effect)
// * palette
//
// but it will wait for AUTOSAVE_SETTLE_MS milliseconds, a "settle" 
// period in case there are other changes (any change will 
// extend the "settle" window).
//
// It will additionally load preset AUTOSAVE_PRESET_NUM at startup.
// during the first `loop()`.  Reasoning below.
//
// AutoSaveUsermod is standalone, but if FourLineDisplayUsermod 
// is installed, it will notify the user of the saved changes.
//
// Note: I don't love that WLED doesn't respect the brightness 
// of the preset being auto loaded, so the AutoSaveUsermod 
// will set the AUTOSAVE_PRESET_NUM preset in the first loop, 
// so brightness IS honored. This means WLED will effectively 
// ignore Default brightness and Apply N preset at boot when 
// the AutoSaveUsermod is installed.

//  "~ MM-DD HH:MM:SS ~"
#define PRESET_NAME_BUFFER_SIZE 25

// strings
const char _um_AutoSave[]         PROGMEM = "Autosave";
const char _autoSaveAfterSec[]    PROGMEM = "autoSaveAfterSec";
const char _autoSavePreset[]      PROGMEM = "autoSavePreset";
const char _autoSaveApplyOnBoot[] PROGMEM = "autoSaveApplyOnBoot";

class AutoSaveUsermod : public Usermod {

  private:

    unsigned long autoSaveAfterSec = 15;  // 15s by default
    uint8_t autoSavePreset = 250;         // last possible preset
    bool initDone = false;
    bool applyAutoSaveOnBoot = false;

    // If we've detected the need to auto save, this will
    // be non zero.
    unsigned long autoSaveAfter = 0;

    char presetNameBuffer[PRESET_NAME_BUFFER_SIZE];

    bool firstLoop = true;

    uint8_t knownBrightness = 0;
    uint8_t knownEffectSpeed = 0;
    uint8_t knownEffectIntensity = 0;
    uint8_t knownMode = 0;
    uint8_t knownPalette = 0;

    #ifdef USERMOD_FOUR_LINE_DISPLAY
    FourLineDisplayUsermod* display;
    #endif

    void inline saveSettings() {
      updateLocalTime();
      sprintf_P(presetNameBuffer, 
        PSTR("~ %02d-%02d %02d:%02d:%02d ~"),
        month(localTime), day(localTime),
        hour(localTime), minute(localTime), second(localTime));
      savePreset(autoSavePreset, true, presetNameBuffer);
    }

    void inline displayOverlay() {
      #ifdef USERMOD_FOUR_LINE_DISPLAY
      if (display != nullptr) {
        display->wakeDisplay();
        display->overlay("Settings", "Auto Saved", 1500);
      }
      #endif
    }

  public:

    // gets called once at boot. Do all initialization that doesn't depend on
    // network here
    void setup() {
      #ifdef USERMOD_FOUR_LINE_DISPLAY    
      // This Usermod has enhanced funcionality if
      // FourLineDisplayUsermod is available.
      display = (FourLineDisplayUsermod*) usermods.lookup(USERMOD_ID_FOUR_LINE_DISP);
      #endif
      initDone = true;
    }

    // gets called every time WiFi is (re-)connected. Initialize own network
    // interfaces here
    void connected() {}

    /**
     * Da loop.
     */
    void loop() {
      if (!autoSaveAfterSec) return;  // setting 0 as autosave seconds disables autosave

      unsigned long now = millis();
      uint8_t currentMode = strip.getMode();
      uint8_t currentPalette = strip.getSegment(0).palette;
      if (firstLoop) {
        firstLoop = false;
        if (applyAutoSaveOnBoot) applyPreset(autoSavePreset);
        knownBrightness = bri;
        knownEffectSpeed = effectSpeed;
        knownEffectIntensity = effectIntensity;
        knownMode = currentMode;
        knownPalette = currentPalette;
        return;
      }

      unsigned long wouldAutoSaveAfter = now + autoSaveAfterSec*1000;
      if (knownBrightness != bri) {
        knownBrightness = bri;
        autoSaveAfter = wouldAutoSaveAfter;
      } else if (knownEffectSpeed != effectSpeed) {
        knownEffectSpeed = effectSpeed;
        autoSaveAfter = wouldAutoSaveAfter;
      } else if (knownEffectIntensity != effectIntensity) {
        knownEffectIntensity = effectIntensity;
        autoSaveAfter = wouldAutoSaveAfter;
      } else if (knownMode != currentMode) {
        knownMode = currentMode;
        autoSaveAfter = wouldAutoSaveAfter;
      } else if (knownPalette != currentPalette) {
        knownPalette = currentPalette;
        autoSaveAfter = wouldAutoSaveAfter;
      }

      if (autoSaveAfter && now > autoSaveAfter) {
        autoSaveAfter = 0;
        // Time to auto save. You may have some flickry?
        saveSettings();
        displayOverlay();
      }
    }

    /*
     * addToJsonInfo() can be used to add custom entries to the /json/info part of the JSON API.
     * Creating an "u" object allows you to add custom key/value pairs to the Info section of the WLED web UI.
     * Below it is shown how this could be used for e.g. a light sensor
     */
    //void addToJsonInfo(JsonObject& root) {
      //JsonObject user = root["u"];
      //if (user.isNull()) user = root.createNestedObject("u");
      //JsonArray data = user.createNestedArray(F("Autosave"));
      //data.add(F("Loaded."));
    //}

    /*
     * addToJsonState() can be used to add custom entries to the /json/state part of the JSON API (state object).
     * Values in the state object may be modified by connected clients
     */
    //void addToJsonState(JsonObject& root) {
    //}

    /*
     * readFromJsonState() can be used to receive data clients send to the /json/state part of the JSON API (state object).
     * Values in the state object may be modified by connected clients
     */
    void readFromJsonState(JsonObject& root) {
      if (!initDone) return;  // prevent crash on boot applyPreset()

      if (root[F("Autosave_autoSaveAfterSec")] != nullptr) {
        autoSaveAfterSec = min(60,max(0,(int)root[F("Autosave_autoSaveAfterSec")]));
      }
      if (root[F("Autosave_autoSavePreset")] != nullptr) {
        autoSavePreset = min(250,max(0,(int)root[F("Autosave_autoSavePreset")]));
      }
      if (root[F("Autosave_autoSaveApplyOnBoot")] != nullptr) {
        applyAutoSaveOnBoot = (bool)root[F("Autosave_autoSaveApplyOnBoot")];
      }
    }

    /*
     * addToConfig() can be used to add custom persistent settings to the cfg.json file in the "um" (usermod) object.
     * It will be called by WLED when settings are actually saved (for example, LED settings are saved)
     * If you want to force saving the current state, use serializeConfig() in your loop().
     * 
     * CAUTION: serializeConfig() will initiate a filesystem write operation.
     * It might cause the LEDs to stutter and will cause flash wear if called too often.
     * Use it sparingly and always in the loop, never in network callbacks!
     * 
     * addToConfig() will also not yet add your setting to one of the settings pages automatically.
     * To make that work you still have to add the setting to the HTML, xml.cpp and set.cpp manually.
     * 
     * I highly recommend checking out the basics of ArduinoJson serialization and deserialization in order to use custom settings!
     */
    void addToConfig(JsonObject& root) {
      // we add JSON object: {"Autosave": {"autoSaveAfterSec": 10, "autoSavePreset": 99}}
      JsonObject top = root.createNestedObject(FPSTR(_um_AutoSave)); // usermodname
      top[FPSTR(_autoSaveAfterSec)]    = autoSaveAfterSec;  // usermodparam
      top[FPSTR(_autoSavePreset)]      = autoSavePreset;    // usermodparam
      top[FPSTR(_autoSaveApplyOnBoot)] = applyAutoSaveOnBoot;
    }

    /*
     * readFromConfig() can be used to read back the custom settings you added with addToConfig().
     * This is called by WLED when settings are loaded (currently this only happens once immediately after boot)
     * 
     * readFromConfig() is called BEFORE setup(). This means you can use your persistent values in setup() (e.g. pin assignments, buffer sizes),
     * but also that if you want to write persistent values to a dynamic buffer, you'd need to allocate it here instead of in setup.
     * If you don't know what that is, don't fret. It most likely doesn't affect your use case :)
     */
    void readFromConfig(JsonObject& root) {
      // we look for JSON object: {"Autosave": {"autoSaveAfterSec": 10, "autoSavePreset": 99}}
      JsonObject top = root[FPSTR(_um_AutoSave)];
      if (!top.isNull() && top[FPSTR(_autoSaveAfterSec)] != nullptr) {
        autoSaveAfterSec    = (int) top[FPSTR(_autoSaveAfterSec)];
        autoSavePreset      = (int) top[FPSTR(_autoSavePreset)];
        applyAutoSaveOnBoot = (bool)top[FPSTR(_autoSaveApplyOnBoot)];
      } else {
        DEBUG_PRINTLN(F("No config found. (Using defaults.)"));
      }
  }

    /*
     * getId() allows you to optionally give your V2 usermod an unique ID (please define it in const.h!).
     * This could be used in the future for the system to determine whether your usermod is installed.
     */
    uint16_t getId() {
      return USERMOD_ID_AUTO_SAVE;
    }

};