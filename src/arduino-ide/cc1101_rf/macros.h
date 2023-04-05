#pragma once

/*
    This file provides us with important information about the functional prototypes available to us,
    provides us with symbolic constants and keeps a list of all available macros.
*/

#include <Arduino.h>

/* --- all SETTINGS for Arduino Nano ---------------------------------------------------------------------------------------------------------- */
#if defined (ARDUINO_AVR_NANO) || defined (ARDUINO_RADINOCC1101) || defined (ARDUINO_AVR_PRO)
#define MSG_OUTPUT(...) { Serial.print(__VA_ARGS__); }
#define MSG_OUTPUTLN(...) { Serial.println(__VA_ARGS__); }
#define MSG_OUTPUTALL(...) { Serial.print(__VA_ARGS__); }
#define MSG_OUTPUTALLLN(...) { Serial.println(__VA_ARGS__); }

/* output DEC to HEX with a leading zero to serial without ln (all dec values < 16 obtain a zero in HEX value) */
#define MSG_OUTPUT_DecToHEX_lz(dec) {                             \
    uint8_t output = dec;                                         \
    if(output < 16) {                                             \
      Serial.print('0');                                          \
    }                                                             \
    Serial.print(output, HEX);                                    \
  }

/* output DEC to HEX with a leading zero to serial without ln (all dec values < 16 obtain a zero in HEX value) */
#define MSG_OUTPUTALL_DecToHEX_lz(dec) {                          \
    uint8_t output = dec;                                         \
    if(output < 16) {                                             \
      Serial.print('0');                                          \
    }                                                             \
    Serial.print(output, HEX);                                    \
  }

/* output numbers with leading zero (max 99) to serial without ln */
#define MSG_OUTPUTALL_Dec_lz(dec) {                               \
    uint8_t output = dec;                                         \
    if(output < 10) {                                             \
      Serial.print('0');                                          \
    }                                                             \
    Serial.print(output);                                         \
  }
#endif
/* --- END - all SETTINGS for Arduino Nano ---------------------------------------------------------------------------------------------------- */


/* --- all SETTINGS for the ESP8266 and ESP32 ------------------------------------------------------------------------------------------------- */
#if defined (ARDUINO_ARCH_ESP8266) || defined (ARDUINO_ARCH_ESP32)
/* output on the last client with input without ln */
#define MSG_OUTPUT(...) {                                         \
    if (TELNET_CLIENT_COUNTER == 0 || client_now == 255) {        \
      Serial.print(__VA_ARGS__);                                  \
    } else if (TELNET_CLIENT_COUNTER != 0) {                      \
      TelnetClient[client_now].print(__VA_ARGS__);                \
    }                                                             \
  }

/* output on the last client with input with ln */
#define MSG_OUTPUTLN(...) {                                       \
    if (TELNET_CLIENT_COUNTER == 0 || client_now == 255) {        \
      Serial.println(__VA_ARGS__);                                \
    } else if (TELNET_CLIENT_COUNTER != 0) {                      \
      TelnetClient[client_now].println(__VA_ARGS__);              \
    }                                                             \
  }

/* output to all interfaces and clients without ln */
#define MSG_OUTPUTALL(...) {                                      \
    for (uint8_t i = 0; i < TELNET_CLIENTS_MAX; i++) {            \
      if (TelnetClient[i] && TelnetClient[i].connected()) {       \
        TelnetClient[i].print(__VA_ARGS__);                       \
        delay(1);                                                 \
      }                                                           \
    }                                                             \
    Serial.print(__VA_ARGS__);                                    \
  }

/* output to all interfaces and clients with ln */
#define MSG_OUTPUTALLLN(...) {                                    \
    for (uint8_t i = 0; i < TELNET_CLIENTS_MAX; i++) {            \
      if (TelnetClient[i] && TelnetClient[i].connected()) {       \
        TelnetClient[i].println(__VA_ARGS__);                     \
        delay(1);                                                 \
      }                                                           \
    }                                                             \
    Serial.println(__VA_ARGS__);                                  \
  }


/* output DEC to HEX with a leading zero to the last client with input without ln (all dec values < 16 obtain a zero in HEX value) */
#define MSG_OUTPUT_DecToHEX_lz(dec) {                             \
    uint8_t output = dec;                                         \
    if (TELNET_CLIENT_COUNTER == 0 || client_now == 255) {        \
      if(output < 16) {                                           \
        Serial.print('0');                                        \
      }                                                           \
      Serial.print(output, HEX);                                  \
    } else if (TELNET_CLIENT_COUNTER != 0) {                      \
      if(output < 16) {                                           \
        TelnetClient[client_now].print('0');                      \
      }                                                           \
      TelnetClient[client_now].print(output, HEX);                \
    }                                                             \
  }

/* output DEC to HEX with a leading zero to all interfaces and clients without ln (all dec values < 16 obtain a zero in HEX value) */
#define MSG_OUTPUTALL_DecToHEX_lz(dec) {                          \
    uint8_t output = dec;                                         \
    for (uint8_t i = 0; i < TELNET_CLIENTS_MAX; i++) {            \
      if (TelnetClient[i] && TelnetClient[i].connected()) {       \
        if(output < 16) {                                         \
          TelnetClient[i].print('0');                             \
        }                                                         \
        TelnetClient[i].print(output, HEX);                       \
        delay(1);                                                 \
      }                                                           \
    }                                                             \
    if(output < 16) {                                             \
      Serial.print('0');                                          \
    }                                                             \
    Serial.print(output, HEX);                                    \
  }

/* output numbers with leading zero (max 99) to all interfaces and clients without ln */
#define MSG_OUTPUTALL_Dec_lz(dec) {                               \
    uint8_t output = dec;                                         \
    for (uint8_t i = 0; i < TELNET_CLIENTS_MAX; i++) {            \
      if (TelnetClient[i] && TelnetClient[i].connected()) {       \
        if(output < 10) {                                         \
          TelnetClient[i].print('0');                             \
        }                                                         \
        TelnetClient[i].print(output);                            \
        delay(1);                                                 \
      }                                                           \
    }                                                             \
    if(output < 10) {                                             \
      Serial.print('0');                                          \
    }                                                             \
    Serial.print(output);                                         \
  }

/* Useful Constants | macros from DateTime.h */
#define SECS_PER_MIN  (60UL)
#define SECS_PER_HOUR (3600UL)
#define SECS_PER_DAY  (SECS_PER_HOUR * 24L)

/* Useful Macros for getting elapsed time */
#define numberOfSeconds(_time_) (_time_ % SECS_PER_MIN)
#define numberOfMinutes(_time_) ((_time_ / SECS_PER_MIN) % SECS_PER_MIN)
#define numberOfHours(_time_) (( _time_% SECS_PER_DAY) / SECS_PER_HOUR)
#define elapsedDays(_time_) ( _time_ / SECS_PER_DAY)

#endif
/* --- END - all SETTINGS for the ESP8266 and ESP32 ------------------------------------------------------------------------------------------- */
