#pragma once

/*
    This file provides us with important information about the functional prototypes available to us,
    provides us with symbolic constants and keeps a list of all available macros.
*/

#include <Arduino.h>
#include "wifi.h"

extern void InputCommand(char* buf_input);
extern String onlyDecToHex2Digit(byte Dec);
extern boolean freqAfc;

/* Display of all websites from the web server
  https://wiki.selfhtml.org/wiki/HTML/Tutorials/Grundger%C3%BCst
  https://divtable.com/table-styler/
*/

void html_meta(String & html) {     /* added meta to html */
  html = F("<!DOCTYPE html>"
           "<html lang=\"de\">"
           "<head>"
           "<meta charset=\"utf-8\">"
           "<meta http-equiv=\"Content-Type\" content=\"text/html\">"
           "<meta http-equiv=\"Content-language\" content=\"de\">"
           "<meta http-equiv=\"Cache-Control\" content=\"no-cache, no-store, must-revalidate\"/>"
           "<meta http-equiv=\"Pragma\" content=\"no-cache\"/>"
           "<meta http-equiv=\"Expires\" content=\"0\"/>"
           "<meta http-equiv=\"X-Content-Type-Options\" content=\"nosniff\">"
           "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">"
           "<meta content=\"X-Content-Type-Options=nosniff\">"
           "<link rel=\"stylesheet\" type=\"text/css\" href=\"all.css\">"
           "<title>cc110x_rf_Gateway</title>");
}


void html_head_table(String & html) {    /* added table on head with all links */
  html += F("<table>"
            "<tr><th class=\"hth\" colspan=\"6\">cc110x_rf_Gateway</th></tr>"
            "<tr>"
            "<td class=\"htd\"><a href=\"/\" class=\"HOME\">HOME</a></td>"
            "<td class=\"htd\"><a href=\"cc110x\" class=\"CC110x\">CC110x</a></td>"
            "<td class=\"htd\"><a href=\"help\" class=\"Help\">Help</a></td>"
            "<td class=\"htd\"><a href=\"log\" class=\"Logfile\">Logfile</a></td>"
            "<td class=\"htd\">");
  if (CC1101_found) {
    html += F("<a href=\"raw\" class=\"RAW\">RAW data</a>");
  }
  html += F("</td><td class=\"htd\"><a href=\"wlan\" class=\"WLAN\">WLAN</a></td></tr>"
            "</table><br>");
}


void web_index() {
  String website;
  html_meta(website);
  website += F("<link rel=\"stylesheet\" type=\"text/css\" href=\"index.css\">"
               "<script src=\"index.js\"></script>"
               "</head>");
  html_head_table(website);
  website += F("<body><table><thead><tr><th colspan=2>Device - Information</th></tr></thead>"
               "<tbody><tr><td class=\"tdf\">Firmware compiled</td><td>");
  website += compile_date;
  website += F("</td></tr><tr><td>Firmware mode</td><td>"
#ifdef SIGNALduino_comp
               "FHEM compatible"
#else
               "standalone"
#endif
               "</td></tr><tr><td>RAM (free heap)</td><td><span id=\"RAM\">");
  website += freeRam();
  website += F("</span></td></tr><tr><td>Uptime (Seconds)</td><td><span id=\"Upt_Sec\">");
  website += getUptime();
  website += F("</span></td></tr><tr><td>Uptime (Text)</td><td><span id=\"Upt_TXT\">");
  website += elapsedDays(getUptime());
  website += F(" day(s)&emsp;");
  website += numberOfHours(getUptime());
  website += F(" hour(s)&emsp;");
  website += numberOfMinutes(getUptime());
  website += F(" minute(s)&emsp;");
  website += numberOfSeconds(getUptime());
  website += F(" second(s)");
  website += F("</span></td></tr><tr><td>Version</td><td>");
  website += TXT_VERSION;
  website += F("</td></tr></tbody></table></body></html>");

  HttpServer.send(200, "text/html", website);
}


void web_cc110x() {
  String website;
  String cc1101_foundTxt = F("no");
  if (CC1101_found) {
    cc1101_foundTxt = F("yes");
  }

  html_meta(website);
  website += F("<link rel=\"stylesheet\" type=\"text/css\" href=\"cc110x.css\">"
               "</head>");
  html_head_table(website);
  website += F("<body>"
               "<table><thead>"
               "<tr>");

  if (CC1101_found) {
    website += F("<th class=\"thf\">general information</th>"
                 "<th class=\"thf\"><a href=\"cc110x_detail\">detail information</a></th>"
                 "<th class=\"thf\"><a href=\"cc110x_modes\">receive modes</a></th>");
  } else {
    website += F("<th colspan=\"3\">general information</th>");
  }

  website += F("</tr></thead><tbody><tr><td>chip detected</td><td td colspan=\"2\">");
  website += cc1101_foundTxt + F("</td></tr>");

  if (CC1101_found) {
    website += F("<tr><td>chip PARTNUM</td><td colspan=\"2\">");
    website += String(CC1101_readReg(CC1101_PARTNUM, READ_BURST), HEX) +
               F("</td></tr><tr><td>chip VERSION</td><td colspan=\"2\">") + String(CC1101_readReg(CC1101_VERSION, READ_BURST), HEX) +
               F("</td></tr><tr><td>chip MARCSTATE</td><td colspan=\"2\">") + web_Marcstate_read() +
               F("</td></tr><tr><td>chip reception mode</td><td colspan=\"2\">") + activated_mode_name +
               F("</td></tr><tr><td>ToggleBank 0-3</td><td colspan=\"2\">{");

    website += F("&emsp;");
    for (byte i = 0; i < 4; i++) {
      if (ToggleArray[i] == 255) {
        website += '-';
      } else {
        website += ToggleArray[i];
      }
      website += F("&emsp;");
    }
    website += F("}</td></tr><tr><td>ToggleTime (ms)</td><td colspan=\"2\">");
    website += ToggleTime;
    website += F("</td></tr>");
  }

  website += F("</tbody></table></body></html>");
  HttpServer.send(200, "text/html", website);
}

void web_cc110x_modes() {
  if (!CC1101_found) {
    HttpServer.send(404, "text/plain", F("Website not found !!!"));
  }
  String InputCmd;                                  // preparation for processing | control html InputCommand
  String submit = HttpServer.arg("submit");         // welcher Button wurde betätigt
  String tb = HttpServer.arg("tgb");
  String tgtime = HttpServer.arg("tgt");            // toggle time
  String web_status = F("<tr><td></td>");
  String website;
  uint8_t countargs = HttpServer.args();            // Anzahl Argumente
  uint8_t tb_nr;                                    // togglebank nr
  uint8_t tb_val;                                   // togglebank value
  char InCmdBuf[11];

#ifdef debug_html
  Serial.println(F("###########################"));
  Serial.print(F("DB web_cc1101_modes, submit "));
  Serial.println(submit);
  Serial.print(F("DB web_cc1101_modes, tb "));
  Serial.println(tb);
  Serial.print(F("DB web_cc1101_modes, tgtime "));
  Serial.println(tgtime);
  Serial.print(F("DB web_cc1101_modes, count "));
  Serial.println(countargs);
#endif

  if (countargs != 0) {
    if (submit != "" && submit != "time") {                 // button "enable reception"
      InputCmd = "m" + submit;

#ifdef debug_html
      Serial.print(F("DB web_cc1101_modes, set reception "));
      Serial.println(submit);
#endif
      web_status = F("<td class=\"in grn\">");
      web_status += String(Registers[submit.toInt()].name) + F(" &#10004;</td>");
    }

    if (submit != "" && submit == "time") {                 // button "START"
      if (tgtime == "") {
        tgtime = String(ToggleTime);
      }
      InputCmd = "tos" + tgtime;
#ifdef debug_html
      Serial.print(F("DB web_cc1101_modes, set toggletime to "));
      Serial.println(tgtime);
#endif
      if (tgtime.toInt() >= ToggleTimeMin && tgtime.toInt() <= ToggleTimeMax) {
        web_status = F("<tr><td class=\"in grn\">toggle started &#10004;</td>");
      } else {
        web_status = F("<tr><td class=\"in red\">toggle value ");
        if (tgtime.toInt() < ToggleTimeMin) {
          web_status += "< ";
          web_status += ToggleTimeMin;
        } else if (tgtime.toInt() > ToggleTimeMax) {
          web_status += "> ";
          web_status += ToggleTimeMax;
        }
        web_status += F(" &#10006;</td>");
      }
    }

    if (tb != "") {
      tb_nr = tb.substring(0, 1).toInt();   /* 0,1,2,3 for valid bank´s and 9 for reset all */
      tb_val = tb.substring(2).toInt();     /* 0 and 1 */

#ifdef debug_html
      Serial.print(F("DB web_cc1101_modes, set togglebank "));
      Serial.print(tb_nr);
      Serial.print(F(" to "));
      Serial.println(tb_val);
#endif

      web_status = F("<tr><td class=\"in grn\">");
      if (tb_nr <= 3) {
        InputCmd = F("tob");    // preparation for processing
        InputCmd += tb_nr;      // preparation for processing
        InputCmd += tb_val;     // preparation for processing
        web_status += F("togglebank ");
        web_status += tb_nr;
        web_status += F(" &#10004;</td>");
      } else {
        InputCmd = F("tob99");
        web_status += F("toggle reseted & stopped &#10004;</td>");
      }
    }

    InputCmd.toCharArray(InCmdBuf, 11);
    InputCommand(InCmdBuf);
    InputCmd = "";              // reset String
  }

  html_meta(website);
  website += F("<link rel=\"stylesheet\" type=\"text/css\" href=\"cc110x_modes.css\">"
               "</head>");
  html_head_table(website);
  website += F("<body>"
               "<form method=\"get\">"            /* form method wichtig für Daten von Button´s !!! */
               //"<body><form method=\"post\">"   /* https://www.w3schools.com/tags/ref_httpmethods.asp */
               "<table>"
               "<thead><tr>"
               "<th class=\"thf1\"><a href=\"cc110x\">general information</a></th>"
               "<th class=\"thf1\"><a href=\"cc110x_detail\">detail information</a></th>"
               "<th class=\"thf1\">receive modes</th></tr></thead>"
               "<tbody>");

  for (byte i = 0; i < RegistersCntMax; i++) {
    website += F("<tr><td>");
    website += Registers[i].name;
    website += F("</td><td class=\"ac\">set to bank&nbsp;&nbsp;");

    for (byte i2 = 0; i2 < 4; i2++) {
      website += F("<button class=");
      if (i == ToggleArray[i2]) {
        website += F("\"btn2\"");
      } else {
        website += F("\"btn\"");
      }
      website += F(" type=\"submit\" name=\"tgb\" value=\"");
      website += i2;
      website += '_';
      website += i;
      website += F("\">&#x3");
      website += i2;
      website += F(";</button>");
    }

    website += F("</td><td class=\"ac\"><button class=");

    if (activated_mode_name == Registers[i].name) {
      website += F("\"btn2\" type=\"submit\" name=\"submit\" value=\"");
      website += i;
      website += F("\">enabled");
    } else {
      website += F("\"btn\" type=\"submit\" name=\"submit\" value=\"");
      website += i;
      website += F("\">enable");
    }
    website += F(" reception</button></td></tr>");
  }

  website += F("<tr>");
  website += web_status +
             F("<td class=\"ac\">toggletime&nbsp;");
  website += F("<input name=\"tgt\" type=\"text\" size=\"7\" maxlength=\"7\" pattern=\"^[\\d]{1,7}$\" placeholder=\"");

  if (ToggleTime != 0) {
    website += ToggleTime;
  } else {
    website += F("(ms)");
  }
  website += F("\"><button class=\"btn\" type=\"submit\" name=\"submit\" value=\"time\">START</button></td>"
               "<td class=\"ac\"><button class=\"btn\" type=\"submit\" name=\"tgb\" value=\"9_0\">reset togglebank & STOP</button>"
               "</td></tr></tbody></table></body></html>");

  HttpServer.send(200, "text / html", website );
}


void web_cc110x_detail_export() {
  String website;
  html_meta(website);
  website += F("<link rel=\"stylesheet\" type=\"text/css\" href=\"cc110x_detail_exp.css\"></head>");

  String website_p2;
  website += F("Export all current register values (without the registers 0x29 ... 0x2E)<br>Just copy and paste string into your application<br>");
  website += F("<br>FHEM - SIGNALduino Format | set &lsaquo;name&rsaquo; cc1101_reg<br>");
  website_p2 = F("<br>FHEM - SIGNALduino Format | SD_ProtocolData.pm, entry register => <br>");

  for (byte i = 0; i <= 46; i++) { /* all registers */
    if (i == 0) {
      website_p2 += '[';
    }
    if (i < 41) {
      website += onlyDecToHex2Digit(i);
      website += onlyDecToHex2Digit(CC1101_readReg(i, READ_BURST)) + ' ';
      website_p2 += '\'';
      website_p2 += onlyDecToHex2Digit(i);
      website_p2 += onlyDecToHex2Digit(CC1101_readReg(i, READ_BURST));
      website_p2 += '\'';
      if (i != 40) {
        website_p2 += ',';
      }
    }
  }
  website += F("<br>");
  website += website_p2;
  website += F("]<br><br><a class=\"back\" href=\"/cc110x_detail\">&crarr; back to detail information</a>");
  website += F("</body></html>");

  HttpServer.send(200, "text/html", website);
}


void web_cc110x_detail_import() {
  String submit = HttpServer.arg("submit");         // welcher Button wurde betätigt
  String imp = HttpServer.arg("imp");               // String der Register inklusive Werte
  String website;
  uint8_t countargs = HttpServer.args();            // Anzahl Argumente
  String Part = "";
  String PartAdr = "";
  String PartVal = "";

  html_meta(website);
  website += F("<link rel=\"stylesheet\" type=\"text/css\" href=\"cc110x_detail_imp.css\"></head>");
  website += F("<body><form method=\"get\">");        /* form method wichtig für Daten von Button´s !!! */

  if (countargs != 0) {
#ifdef debug_html
    Serial.println(F("###########################"));
    Serial.print(F("DB web_cc110x_detail_import, submit     "));
    Serial.println(submit);
    Serial.print(F("DB web_cc110x_detail_import, countargs  "));
    Serial.println(countargs);
    Serial.print(F("DB web_cc110x_detail_import, imp        "));
    Serial.println(imp);
#endif

    if (submit == "registers") {                    // register processing from String imp ['01AB','11FB']
      uint8_t Adr;
      uint8_t Val;

      for (uint16_t i = 0; i < imp.length(); i++) {
        if (imp[i] == ',' || imp[i] == ']') {
          PartAdr = Part.substring(0, 2);
          PartVal = Part.substring(2, 4);
          Adr = hexToDec(PartAdr);
          Val = hexToDec(PartVal);
#ifdef debug_html
          Serial.print(F("DB web_cc110x_detail_import, cc110x adr: "));
          Serial.print(PartAdr);
          Serial.print(' ');
          Serial.println(Adr);
          Serial.print(F("DB web_cc110x_detail_import, cc110x val: "));
          Serial.print(PartVal);
          Serial.print(' ');
          Serial.println(Val);
#endif

          if (Adr > 0x2E) {
            break;
          } else {
            CC1101_writeReg(Adr, Val);    // write in cc1101
            EEPROMwrite(Adr, Val);        // write in flash
          }
          Part = "";          // reset String
        }

        if (imp[i] != '[' && imp[i] != ']' && imp[i] != ',' && imp[i] != '\'') {
          Part += imp[i];
        }
      }
      if (Adr > 0x2E) {
        website += F("ERROR: wrong cc110x adress, nothing write!");
        website += PartAdr;
      } else {
        website += F("Import of the register values closed​​");
      }
    }
  } else {
    website += F("Import of the register values ​​(minimum 2 pieces)<br>");
    website += F("(Please paste the string in SIGNALduino format)<br><br>");
    website += F("<input size=\"100\" maxlength=\"288\" value=\"example ['01AB','11FB']\" name=\"imp\" pattern=\"^\\['[[0-9a-fA-F]{4}'(,'[[0-9a-fA-F]{4}')+\\]$\">");
    website += F("<br><br>");
    website += F("<button class=\"btn\" type=\"submit\" name=\"submit\" value=\"registers\">acceptance of the values ​​in the register</button>");
  }

  website += F("<br><br><a class=\"back\" href=\"/cc110x_detail\">&crarr; back to detail information</a>");
  website += F("</form></body></html>");
  HttpServer.send(200, "text/html", website);
}


void web_cc110x_detail() {
  if (!CC1101_found) {
    HttpServer.send(404, "text/plain", F("Website not found !!!"));
  }

  String afc = HttpServer.arg("afc");
  String bandw = HttpServer.arg("bandw");
  String datar = HttpServer.arg("datarate");
  String dev = HttpServer.arg("deviation");
  String freq = HttpServer.arg("freq");
  String freqOff = HttpServer.arg("freq_off");
  String mod = HttpServer.arg("modulation");
  String mod_array[8] = { F("2-FSK"), F("GFSK"), "", F("ASK/OOK"), F("4-FSK"), "", "", F("MSK") };
  String return_val;
  String submit = HttpServer.arg("submit");         // welcher Button wurde betätigt
  String temp;
  String web_status = F("<tr><td class=\"in\" colspan=\"6\">current values ​​readed &#10004;</td></tr>");
  String website;
  uint8_t countargs = HttpServer.args();            // Anzahl Argumente


  if (countargs != 0) {
#ifdef debug_html
    Serial.println(F("###########################"));
    Serial.print(F("DB web_cc1101_detail, submit     "));
    Serial.println(submit);
    Serial.print(F("DB web_cc1101_detail, countargs  "));
    Serial.println(countargs);
    Serial.print(F("DB web_cc1101_detail, freq       "));
    Serial.println(freq);
    Serial.print(F("DB web_cc1101_detail, freqOff    "));
    Serial.println(freqOff);
    Serial.print(F("DB web_cc1101_detail, datarate   "));
    Serial.println(datar);
    Serial.print(F("DB web_cc1101_detail, deviation  "));
    Serial.println(dev);
    Serial.print(F("DB web_cc1101_detail, modulation "));
    Serial.println(mod);
#endif

    if (countargs > 0) { /* register values from browser | set registers button -> into array */
      for (byte i = 0; i <= 46; i++) {
        web_regData[i] = HttpServer.arg(String("0x") + String(onlyDecToHex2Digit(i)));
      }
    }

    if (submit == "bfreq") {
#ifdef debug_html
      Serial.println(F("DB web_cc1101_detail, submit set frequency & offset pushed"));
#endif

      if (afc == "1") {
        freqAfc = 1;
      } else {
        afc = '0';
        freqAfc = 0;
      }
      EEPROMwrite(EEPROM_ADDR_AFC, freqAfc);
      freqOffAcc = 0;
      CC1101_writeReg(CC1101_FSCTRL0, 0);                      // 0x0C: FSCTRL0 – Frequency Synthesizer Control
      Freq_offset = freqOff.toFloat();
      EEPROM.put(EEPROM_ADDR_FOFFSET, Freq_offset);
#if defined (ARDUINO_ARCH_ESP8266) || defined (ARDUINO_ARCH_ESP32)
      EEPROM.commit();
#endif

      return_val = web_Freq_set(freq);
      web_status = F("<tr><td class=\"in\" colspan=\"6\">Frequency & Frequency Offset set &#10004;</td></tr>");

      for (byte i = 0; i < 3; i++) {        /* write value to register 0D,0E,0F */
        byte value = hexToDec(return_val.substring(i * 2, i * 2 + 2));
        CC1101_writeReg(i + 13, value);     // write in cc1101
        EEPROMwrite(i + 13, value);         // write in flash
      }
    } else if (submit == "bbandw") {
#ifdef debug_html
      Serial.print(F("DB web_cc1101_detail, button set bandwidth pushed with "));
      Serial.println(bandw);
      Serial.print(F("DB web_cc1101_detail, register 0x10 value is "));
      Serial.println(web_regData[16]);
#endif
      web_status = F("<tr><td class=\"in\" colspan=\"6\">Bandwidth set &#10004;</td></tr>");
      int value = web_Bandw_cal(bandw.toInt(), (hexToDec(web_regData[16]) & 0x0f));   /* input complete | input split */
      CC1101_writeReg(16, value);           // write in cc1101
      EEPROMwrite(16, value);               // write in flash
    } else if (submit == "bdatarate") {
#ifdef debug_html
      Serial.print(F("DB web_cc1101_detail, button set datarate pushed with "));
      Serial.println(datar);
#endif
      web_status = F("<tr><td class=\"in\" colspan=\"6\">DataRate set &#10004;</td></tr>");
      return_val = web_Datarate_set(datar.toFloat());

      for (byte i = 0; i < 2; i++) {        /* write value to register 0D,0E,0F */
        byte value = hexToDec(return_val.substring(i * 2, i * 2 + 2));
        CC1101_writeReg(i + 16, value);     // write in cc1101
        EEPROMwrite(i + 16, value);         // write in flash
      }
    } else if (submit == "bdev") {
#ifdef debug_html
      Serial.print(F("DB web_cc1101_detail, button set deviation pushed with "));
      Serial.println(dev);
#endif
      web_status = F("<tr><td class=\"in\" colspan=\"6\">Deviation set &#10004;</td></tr>");
      return_val = web_Devi_set(dev.toFloat());
      CC1101_writeReg(21, hexToDec(return_val));  // write in cc1101
      EEPROMwrite(21, hexToDec(return_val));      // write in flash
    } else if (submit == "bmod") {
#ifdef debug_html
      Serial.print(F("DB web_cc1101_detail, button set modulation pushed with "));
      Serial.println(mod);
#endif
      web_status = F("<tr><td class=\"in\" colspan=\"6\">Modulation set &#10004;</td></tr>");
      CC1101_writeReg(18, web_Mod_set(mod));      // write in cc1101
      EEPROMwrite(18, web_Mod_set(mod));          // write in flash
    } else if (submit == "breg") {
#ifdef debug_html
      Serial.println(F("DB web_cc1101_detail, button set registers pushed"));
#endif
      web_status = F("<tr><td class=\"in\" colspan=\"6\">all registers set &#10004;</td></tr>");

      for (byte i = 0; i <= 46; i++) { /* all registers */
#ifdef debug_html
        Serial.print(F("DB web_cc1101_detail, regData["));
        Serial.print(i);
        Serial.print(F("] = "));
        Serial.println(web_regData[i]);
#endif

        /* compare value with value to be written */
        temp = onlyDecToHex2Digit(CC1101_readReg(i, READ_BURST));
        temp.toUpperCase();
        if (web_regData[i] != temp) {
#ifdef debug_html
          if (i > 34 && i <= 40) {
            Serial.println(F("DB web_cc1101_detail, automatic control register"));
          }
          if (i > 40) {
            Serial.println(F("DB web_cc1101_detail, test register"));
          }
          Serial.print(F("DB web_cc1101_detail, regData["));
          Serial.print(i);
          Serial.print(F("] value has changed "));
          Serial.print(temp);
          Serial.print(F(" -> "));
          Serial.println(web_regData[i]);
#endif
          if (i == 6) { /* ToDo - probably not optimal -> if register 6 is changed (dependencies) */
            activated_mode_packet_length = hexToDec(web_regData[i]);
          }
          /* write value to registe */
          CC1101_writeReg(i, hexToDec(web_regData[i]));     // write in cc1101
          EEPROMwrite(i, hexToDec(web_regData[i]));         // write in flash
        }
      }
    }
    activated_mode_name = F("CC110x user configuration");
  } else {
    freq = String(web_Freq_read( CC1101_readReg(13, READ_BURST),
                                 CC1101_readReg(14, READ_BURST),
                                 CC1101_readReg(15, READ_BURST)), 3);
  }
  temp = "";

  html_meta(website);
  website += F("<link rel=\"stylesheet\" type=\"text/css\" href=\"cc110x_detail.css\"></head>");
  html_head_table(website);
  website += F("<body><form method=\"get\">"        /* form method wichtig für Daten von Button´s !!! */
               "<table>"
               "<thead><tr>"
               "<th class=\"f1\" colspan=\"2\"><a href=\"cc110x\">general information</a></th>"
               "<th class=\"f1\" colspan=\"2\">detail information</th>"
               "<th class=\"f1\" colspan=\"2\"><a href=\"cc110x_modes\">receive modes</a></th></tr>"
               "</thead><tbody>"
               "<tr><td colspan=\"2\">Frequency (should | is)</td><td class=\"di\" colspan=\"2\">");
  website += String( web_Freq_read(pgm_read_byte_near(Registers[activated_mode_nr].reg_val + 13),
                                   pgm_read_byte_near(Registers[activated_mode_nr].reg_val + 14),
                                   pgm_read_byte_near(Registers[activated_mode_nr].reg_val + 15)
                                  ), 3);
  website += F(" | ");
  website += String( web_Freq_read( CC1101_readReg(13, READ_BURST),
                                    CC1101_readReg(14, READ_BURST),
                                    CC1101_readReg(15, READ_BURST)
                                  ), 3);
  website += F(" MHz</td><td class=\"ce\"><input size=\"7\" maxlength=\"7\" name=\"freq\" value=\"");
  website += freq;
  website += F("\" pattern=\"^[\\d]{3}(\\.[\\d]{1,3})?$\"></td><td class=\"ce\" rowspan=\"2\"><button class=\"btn\" type=\"submit\" name=\"submit\" value=\"bfreq\">set</button></td></tr>");          // end Frequency

  website += F("<tr><td colspan=\"2\">Frequency Offset</td><td class=\"f2 ce\">Afc: <input name=\"afc\" type=\"checkbox\" value=\"1\"");
  if (freqAfc == 1) {
    website += F(" checked");
  }

  /* afc =1 todo !!! */

  website += F("></td><td class=\"di\">");
  website += String(Freq_offset, 3);
  website += F(" MHz</td><td class=\"ce\"><input size=\"7\" maxlength=\"6\" name=\"freq_off\" value=\"");
  website += String(Freq_offset, 3);
  website += F("\" pattern=\"^-?[\\d]{1,3}(\\.[\\d]{1,3})?$\"></td></tr>");    // end Frequency Offset

  website += F("<tr><td colspan=\"2\">Bandwidth</td><td class=\"di\" colspan=\"2\">");
  website += web_Bandw_read();
  website += F(" kHz</td><td class=\"ce\"><input size=\"7\" maxlength=\"5\" name=\"bandw\" value=\"");
  website += String(web_Bandw_read(), 0);
  website += F("\" pattern=\"^[\\d]{2,3}(\\.[\\d]{1,2})?$\"></td><td class=\"ce\"><button class=\"btn\" type=\"submit\" name=\"submit\" value=\"bbandw\">set</button></td></tr>"         // end Bandwidth
               "<tr><td colspan=\"2\">DataRate</td><td class=\"di\" colspan=\"2\">");
  website += String(web_Datarate_read(), 2);
  website += F(" kBaud</td><td class=\"ce\"><input size=\"7\" maxlength=\"6\" name=\"datarate\" value=\"");
  website += String(web_Datarate_read(), 2);
  website += F("\" pattern = \"^[\\d]{1,4}(\\.[\\d]{1,2})?$\"></td><td class=\"ce\"><button class=\"btn\" type=\"submit\" name=\"submit\" value=\"bdatarate\">set</button></td></tr>"    // end DataRate
               "<tr><td colspan=\"2\">Deviation</td><td class=\"di\" colspan=\"2\">");
  website += String(web_Devi_read(), 2);
  website += F(" kHz</td><td class=\"ce\"><input size=\"7\" maxlength=\"5\" name=\"deviation\" value=\"");
  website += String(web_Devi_read(), 2);
  website += F("\" pattern = \"^[\\d]{1,3}(\\.[\\d]{1,2})?$\"></td><td class=\"ce\"><button class=\"btn\" type=\"submit\" name=\"submit\" value=\"bdev\">set</button></td></tr>"   // end Deviation
               "<tr><td colspan=\"2\">Modulation</td><td class=\"di\" colspan=\"2\">");
  website += web_Mod_read();
  website += F("<td class=\"ce\"><select id=\"modulation\" name=\"modulation\">");

  for (byte i = 0; i <= 7; i++) {
    if (i == 2 || i == 5 || i == 6) {
      continue;
    }
    website += F("<option value=");
    website += i;
    if (mod_array[i] == web_Mod_read()) {
      website += F(" selected");
    }
    website += '>';
    website += mod_array[i];
    website += F("</option>");
  }

  website += F("</select></td><td class=\"ce\"><button class=\"btn\" type=\"submit\" name=\"submit\" value=\"bmod\">set</button></td></tr>"                                      // end Modulation
               "<tr><td colspan=\"2\">Packet length config</td><td class=\"ce\" colspan=\"4\">");
  website += web_PackConf_read();
  website += F("</td></tr><tr><td colspan=\"2\">Sync-word qualifier mode</td><td class=\"ce\" colspan=\"4\">");
  website += web_SyncQualiMode_read();
  website += F("</td></tr>"
               "<tr><td class=\"ce\" colspan=\"6\"><button class=\"btn\" type=\"submit\" name=\"submit\" value=\"breg\">set all registers</button>&emsp;"
               "<button class=\"btn\" type=\"button\" onClick=\"location.href='cc110x_detail_export'\">export all registers</button>&emsp;"
               "<button class=\"btn\" type=\"button\" onClick=\"location.href='cc110x_detail_import'\">import registers</button></td></tr>");
  website += web_status +
             F("<tr><td>register</td><td class=\"ce\">value</td><td colspan=\"4\">notes</td></tr>");

  for (byte i = 0; i <= 46; i++) {
    website += F("<tr><td class=\"f4\">0x");
    temp = onlyDecToHex2Digit(i);                               /* register */
    temp.toUpperCase();
    website += temp;
    website += F("&emsp;");
    website += regExplanation_short[i];
    website += F("</td><td class=\"ce\">");
    temp = onlyDecToHex2Digit(CC1101_readReg(i, READ_BURST));   /* value */
    temp.toUpperCase();
    website += F("<input class= \"vw\" size=\"2\" maxlength=\"2\" name=\"0x");
    website += onlyDecToHex2Digit(i);                                                                   /* registername for GET / POST */
    website += F("\" value=\"");
    website += temp;                                                                                    /* value for GET / POST */
    website += F("\" type=\"text\" maxlength=\"2\" pattern=\"^[\\da-fA-F]{1,2}$\" placeholder=\"");
    website += temp;                                                                                    /* placeholder */
    website += F("\"></td><td colspan=\"4\">");
    website += regExplanation[i];                                                                       /* description */
    if (i == 6) {
      website += F(" = ");
      website += String(CC1101_readReg(0x06, READ_BURST));
    }
    website += F("</td></tr>");
  }

  website += F("</tbody></table></form></body></html>");

  HttpServer.send(200, "text/html", website );
}


void web_log() {
  String website;
  html_meta(website);
  website += F("<link rel=\"stylesheet\" type=\"text/css\" href=\"log.css\">"
               "</head>");
  html_head_table(website);

  File logfile = LittleFS.open("/files/log.txt", "r");
  if (!logfile) {
#ifdef debug_html
    Serial.println("File log.txt nothing found");
#endif
    website += F("No file log.txt found. (File system incomplete)</html>");
  } else {
    uint8_t lineCnt = 0;
    while (logfile.available()) {
      website += logfile.readStringUntil('\n');
      website += F("<br>");
      lineCnt++;
    }
    logfile.close();
    if (lineCnt == 0) {
      website += F("Log file is empty");
    }
    website += F("</html>");
  }

  HttpServer.send(200, "text/html", website);
}


void web_raw() {
  if (!CC1101_found) {
    HttpServer.send(404, "text/plain", F("Website not found !!!"));
  }
  String sd = HttpServer.arg("sd");                 // welche Daten genutzt werden sollen
  String submit = HttpServer.arg("submit");         // welcher Button wurde betätigt
  String website;
  uint8_t countargs = HttpServer.args();            // Anzahl Argumente

  html_meta(website);
  website += F("<body><form method=\"get\">"        /* form method wichtig für Daten von Button´s !!! */
               "<link rel=\"stylesheet\" type=\"text/css\" href=\"raw.css\">"
               "<script src=\"raw.js\"></script>"
               "</head>");
  html_head_table(website);
  website += F("<table>"
               "<thead><tr><th colspan=\"3\">send data (with the current register settings)</th></tr></thead>"
               "<tr><td colspan=\"2\" class=\"tdsd\"><input class=\"inp\" size=\"70\" name=\"sd\" type=\"text\" maxlength=\"255\" pattern=\"^[\\da-fA-F]{2,255}$\" placeholder=\"input your hex data to send\"></td>"
               "<td class=\"tdsb\"><button class=\"btn\" type=\"submit\" name=\"submit\" value=\"send\">send</button></td></tr>");

  if (countargs != 0) {
#ifdef debug_html
    Serial.print(F("DB web_raw, countargs "));
    Serial.println(countargs);
    Serial.print(F("DB web_raw, submit "));
    Serial.println(submit);
    Serial.print(F("DB web_raw, sd "));
    Serial.println(sd);
#endif

    if (submit == "send") { /* SEND DATA */
      if (sd != "") {
        if (sd.length() % 2 == 0) {                   /* check datapart is odd */
          char senddata[sd.length() + 1];
          sd.toCharArray(senddata, sd.length() + 1);
          CC1101_setTransmitMode();                   /* enable TX */
          CC1101_sendFIFO(senddata);
          CC1101_setReceiveMode();                    /* enable RX */
        } else {
#ifdef debug_html
          Serial.println(F("DB web_raw, found odd number of nibbles"));
#endif
          website += F("<tr><td colspan=\"3\" class=\"sd\">found odd number of nibbles, no send !!!</td></tr>");
        }
      } else {
        website += F("<tr><td colspan=\"3\" class=\"sd\">Please input data ;-)</td></tr>");
      }
    }
  }

  website += F("</table><br>");
  website += F("<div>"
               "<table id=\"dataTable\">"
               "<thead>"
               "<tr><th class=\"dd\">Time</th><th>current RAW received data</th><th class=\"dd\">RSSI in dB</th><th class=\"dd\">Offset in kHz</th></tr>"
               "</thead>"
               "</table>"
               "</div>"
               "</body></html>");

  HttpServer.send(200, "text/html", website);
}


void web_val_raw() {
  String website = "";
  int16_t offset = (Freq_offset / 1000) + (26000000 / 16384 * freqErr / 1000);

  if (html_raw != "") {
    website = F("{\"RAW\":\"");
    html_raw.toUpperCase();
    website += html_raw;
    website += F("\", \"RAW_rssi\":\"");
    website += RSSI_dez;
    website += F("\", \"RAW_afc\":\"");
    website += offset;
    website += F("\"}");

    html_raw = "";
  } else {
    website = F("{}");
  }
  HttpServer.send(200, "text/plain", website); // Send value, JSON to client ajax request
}


void web_wlan() {
  /* https://arduino-esp8266.readthedocs.io/en/latest/esp8266wifi/scan-class.html
     https://links2004.github.io/Arduino/d4/d52/wl__definitions_8h_source.html
     https://blog.robberg.net/wifi-scanner-with-esp32/ */
  IPAddress ipadr;                                  // IP addresse
  char ipbuffer[16];
  String WifiEncryptionType;
  String WifiMAC;
  String hssid = HttpServer.arg("hiddenssid");      // Auswahl versteckte SSID
  String qdhcp = HttpServer.arg("dhcp");            // 1-DHCP ein, 0-DHCP aus
  String qdns = HttpServer.arg("dns");              // Domain Name Server statisch
  String qip = HttpServer.arg("ip");                // IP-Adresse statisch
  String qpass = HttpServer.arg("pw");              // WLAN Passwort
  String qsgw = HttpServer.arg("gw");               // Gateway statisch
  String qsnm = HttpServer.arg("sn");               // Subnetzmaske  statisch
  String qssid = HttpServer.arg("setssid");         // Auswahl SSID
  String submit = HttpServer.arg("submit");         // welcher Button wurde betätigt
  String website;
  uint8_t countargs = HttpServer.args();            // Anzahl Argumente
  used_ssid_mac = WiFi.BSSIDstr();

#ifdef debug_html
  Serial.print(F("DB web_wlan, setssid    "));
  Serial.println(qssid);
  Serial.print(F("DB web_wlan, hiddenssid "));
  Serial.println(hssid);
  Serial.print(F("DB web_wlan, pw         "));
  Serial.println(qpass);
  Serial.print(F("DB web_wlan, dhcp       "));
  Serial.println(qdhcp);
  Serial.print(F("DB web_wlan, ip         "));
  Serial.println(qip);
  Serial.print(F("DB web_wlan, dns        "));
  Serial.println(qdns);
  Serial.print(F("DB web_wlan, sn         "));
  Serial.println(qsnm);
  Serial.print(F("DB web_wlan, gw         "));
  Serial.println(qsgw);
  Serial.print(F("DB web_wlan, submit     "));
  Serial.println(submit);
  Serial.print(F("DB web_wlan, countargs  "));
  Serial.println(countargs);
  Serial.println(F("DB web_wlan, scan networks ..."));
#endif

  html_meta(website);
  website += F("<link rel=\"stylesheet\" type=\"text/css\" href=\"wlan.css\">"
               "</head>");
  html_head_table(website);
  website += F("<body><form method=\"get\">"      /* form method wichtig für Daten von Button´s !!! */
               "<table>"   /* START Tabelle gesamt */
               "<thead>"
               "<tr><th colspan=\"6\">WLAN - Device status</th></tr>"
               "<tr><th colspan=\"4\" class=\"fw\">Mode</td><th colspan=\"2\">MAC</th></tr>"
               "<script src=\"wlan.js\"></script>"
               "</thead>"
               "<tbody>"
               "<tr><td class=\"alig_c\" colspan=\"4\">");

  if (!WLAN_AP) {
    website += F("WIFI_STA (normal, station mode function)");
  } else {
    website += F("WIFI_AP (wireless access point)");
  }

  website += F("</td><td class=\"alig_c\" colspan=\"2\">");
  website += WiFi.macAddress() + F("</td></tr>");
  website += F("<tr><td class=\"b1\" colspan=\"6\"></td></tr>");
  website += F("<thead><tr><th colspan=\"6\">WLAN - available networks</th></tr>"
               "<tr><th></th><th>SSID</th><th>MAC</th><th>CH</th><th>RSSI</th><th>encryptionType</th></tr></thead>");

  WifiNetworks = WiFi.scanNetworks();         // Scanne Netzwerke

  for (uint8_t i = 0; i < WifiNetworks; ++i) {
    WifiMAC = WLAN_MAC_String(WiFi.BSSID(i));
    WifiEncryptionType = WLAN_encryptionType(WiFi.encryptionType(i));

    /* Tabelle WLAN - available networks */
    website += F("<tr><td class=\"ra1\"><input type=\"radio\" name=\"setssid\" value=\"");
    website += WiFi.SSID(i);

#ifdef debug_html
    Serial.print(F("DB web_wlan,"));
#endif

    if (used_ssid == WiFi.SSID(i) && used_ssid_mac == WifiMAC) {
#ifdef debug_html
      Serial.print('x');
#endif
      website += F("\" checked>");
    } else {
      website += F("\">");
    }

#ifdef debug_html
    Serial.print(' ');
    Serial.print(i + 1);
    Serial.print(' ');
    Serial.print(WiFi.SSID(i));
    Serial.print(' ');
    Serial.print(WifiMAC);
    Serial.print(' ');
    Serial.print(WiFi.channel(i));
    Serial.print(' ');
    Serial.print(WiFi.RSSI(i));
    Serial.print(' ');
    Serial.println(WifiEncryptionType);
#endif

    website += F("</td><td class=\"ssid\">");
    website += WiFi.SSID(i) + F("</td><td class=\"fb\">") + WifiMAC +
               F("</td><td class=\"wch\">") + WiFi.channel(i) +
               F("</td><td class=\"wrs");
    if (WiFi.RSSI(i) > -40) {         // WLAN RSSI good
      website += F(" wrsgrn\">");
    } else if (WiFi.RSSI(i) < -65) {  // WLAN RSSI very bad
      website += F(" wrsred\">");
    } else {
      website += F("\">");            // WLAN RSSI middle
    }
    website += WiFi.RSSI(i);
    website += F(" db</td><td class=\"alig_c\">");
    website += WifiEncryptionType + F("</td></tr>");
  }

  /* Tabelle WLAN - available networks, letzte Zeile für SSID-Eingabe */
  website += F("<tr>"
               "<td class=\"ra1\"><input name=\"setssid\" type=\"radio\" value=\"hidden\"></td>"
               "<td class=\"inp\" colspan=\"2\"><input pattern=\"^[\\da-zA-Z-]{1,32}$\" placeholder=\"your hidden ssid\" name=\"hiddenssid\" type=\"text\" maxlength=\"32\" size=\"32\"></td>"
               "<td class=\"inp\" colspan=\"3\"><input placeholder=\"your password\" name=\"pw\" type=\"password\" maxlength=\"64\" size=\"25\"></td>"
               "</tr>"
               "<tr>"
               "<td class=\"zcb\"></td><td class=\"inp\" colspan=\"2\"><button class=\"btn\" onclick=\"wlan_wps()\" type=\"submit\" name=\"submit\" value=\"wps\">Wi-Fi Protected (WPS)</button></td>"
               "<td class=\"inp\" colspan=\"3\"><button class=\"btn\" onclick=\"wlan_conn()\" type=\"submit\" name=\"submit\" value=\"connect\">connect</button></td>"
               "</tr>"
               "<tr><td colspan=\"6\" class=\"b1\"><span id=\"StatCon\"></span></td></tr>");

  /* Tabelle Setting IP - network */
  website += F("<thead><tr><th colspan=\"6\">Setting IP - network</th></tr></thead>");

  if (WLAN_AP) {
    ipadr = WiFi.softAPIP();     // IP
  } else {
    ipadr = WiFi.localIP();      // IP
  }

  /* Tabelle Setting IP - network | IP DHCP */
  website += F("<tr><td class=\"ra1\"><input type=\"radio\" name=\"dhcp\" value=\"1\"");
  if (used_dhcp == 1) {
    website += F("checked");
  }
  website += F("></td><td colspan=\"2\">IP - adress automatically (DHCP)</td><td class=\"inp\" colspan=\"3\">");
  sprintf(ipbuffer, "%d.%d.%d.%d", ipadr[0], ipadr[1], ipadr[2], ipadr[3]);
  website += ipbuffer;
  website += F("</td></tr>");

  /* Tabelle Setting IP - network | IP statisch */
  website += F("<tr><td class=\"ra1\"><input type=\"radio\" name=\"dhcp\" value=\"0\"");
  if (used_dhcp == 0) {
    website += F("checked");
  }
  website += F("></td><td colspan=\"2\">IP - adress static</td>"
               "<td class=\"inp\" colspan=\"3\"><input pattern=\"^[\\d]{1,3}\\.[\\d]{1,3}\\.[\\d]{1,3}\\.[\\d]{1,3}$\" size=\"15\" maxlength=\"15\" name=\"ip\" type=\"text\" id=\"ip\" value=\"");
  sprintf(ipbuffer, "%d.%d.%d.%d", ipadr[0], ipadr[1], ipadr[2], ipadr[3]);
  website += ipbuffer;
  website += F("\"></td></tr>");

  ipadr = WiFi.gatewayIP();     /* Tabelle Setting IP - network | domain name server */
  website += F("<tr><td class=\"alig_r\" colspan=\"3\">domain name server</td><td class=\"inp\" colspan=\"3\"><input pattern=\"^[\\d]{1,3}\\.[\\d]{1,3}\\.[\\d]{1,3}\\.[\\d]{1,3}$\" size=\"15\" maxlength=\"15\" name=\"dns\" type=\"text\" id=\"dns\" value=\"");
  sprintf(ipbuffer, "%d.%d.%d.%d", ipadr[0], ipadr[1], ipadr[2], ipadr[3]);
  website += ipbuffer;
  website += F("\"></td></tr>");

  ipadr = WiFi.gatewayIP();     /* Tabelle Setting IP - network | Standard-Gateway */
  website += F("<tr><td class=\"alig_r\" colspan=\"3\">default gateway</td><td class=\"inp\" colspan=\"3\"><input pattern=\"^[\\d]{1,3}\\.[\\d]{1,3}\\.[\\d]{1,3}\\.[\\d]{1,3}$\" size=\"15\" maxlength=\"15\" name=\"gw\" type=\"text\" id=\"gw\" value=\"");
  sprintf(ipbuffer, "%d.%d.%d.%d", ipadr[0], ipadr[1], ipadr[2], ipadr[3]);
  website += ipbuffer;
  website += F("\"></td></tr>");

  ipadr = WiFi.subnetMask();    /* Tabelle Setting IP - network | Subnetzkaske */
  website += F("<tr><td class=\"alig_r\" colspan=\"3\">subnet mask</td><td class=\"inp\" colspan=\"3\"><input pattern=\"^[\\d]{1,3}\\.[\\d]{1,3}\\.[\\d]{1,3}\\.[\\d]{1,3}$\" size=\"15\" maxlength=\"15\" name=\"sn\" type=\"text\" id=\"sn\" value=\"");
  sprintf(ipbuffer, "%d.%d.%d.%d", ipadr[0], ipadr[1], ipadr[2], ipadr[3]);
  website += ipbuffer;
  website += F("\"></td></tr>");

  /* Button connect | with WIFI Settings */
  if (submit == "connect") {
#ifdef debug_html
    Serial.println(qdhcp == "1" ? F("DB web_wlan, qdhcp 1 (on)") : F("DB web_wlan, qdhcp 0 (off)"));
#endif

    if (qdhcp == "1") {     /* DHCP ein */
      used_dhcp = 1;
      EEPROMwrite(EEPROM_ADDR_DHCP, 1);
    } else {                /* DHCP aus */
      EEPROMwrite(EEPROM_ADDR_DHCP, 0);
      used_dhcp = 0;

      byte ipaddress[4];
      /* statische IP übernehmen */
      if (str2ip((char*)qip.c_str(), ipaddress)) {              // convert String to Array of 4 Bytes
#ifdef debug_html
        Serial.print(F("DB web_wlan, static IP "));
        Serial.println(qip);
#endif
        EEPROMwrite_ipaddress(EEPROM_ADDR_IP, qip);             // write IPAddress to EEPROM
        eip = EEPROMread_ipaddress(EEPROM_ADDR_IP);
      } else {
        /* Static IP address invalid! */
      }

      /* Standard-Gateway übernehmen */
      if (str2ip((char*)qsgw.c_str(), ipaddress)) {             // convert String to Array of 4 Bytes
#ifdef debug_html
        Serial.print(F("DB web_wlan, Standard-Gateway "));
        Serial.println(qsgw);
#endif
        EEPROMwrite_ipaddress(EEPROM_ADDR_GATEWAY, qsgw);       // write IPAddress to EEPROM
        esgw = EEPROMread_ipaddress(EEPROM_ADDR_GATEWAY);
      } else {
        /* Default gateway invalid! */
      }

      /* statischen Domain Name Server übernehmen */
      if (str2ip((char*)qdns.c_str(), ipaddress)) {             // convert String to Array of 4 Bytes
#ifdef debug_html
        Serial.print(F("DB web_wlan, static Domain Name Server "));
        Serial.println(qdns);
#endif
        EEPROMwrite_ipaddress(EEPROM_ADDR_DNS, qdns);           // write IPAddress to EEPROM
        edns = EEPROMread_ipaddress(EEPROM_ADDR_DNS);
      } else {
        /* Invalid domain name server! */
      }

      /* Subnetmask übernehmen */
      if (str2ip((char*)qsnm.c_str(), ipaddress)) {             // convert String to Array of 4 Bytes
#ifdef debug_html
        Serial.print(F("DB web_wlan, subnet mask "));
        Serial.println(qsnm);
#endif
        EEPROMwrite_ipaddress(EEPROM_ADDR_NETMASK, qsnm);       // write IPAddress to EEPROM
        esnm = EEPROMread_ipaddress(EEPROM_ADDR_NETMASK);
      } else {
        /* Invalid subnet mask! */
      }
    }

    if (qssid.length() > 0 && qpass.length() > 0) {
      if (qssid == "hidden") {              /* verstecktes Netzwerk */
        qssid = hssid;                      /* versteckte SSID übernehmen */
      }
    }

#ifdef debug_html
    Serial.println(F("DB web_wlan, send HTML (connect)"));
#endif

    website = F("<!DOCTYPE html>WLAN - an attempt is made to establish a connection to SSID ");
    website += qssid;
    website += F("<br>If the attempt fails, the old settings are loaded.</html>");

    HttpServer.send(200, "text/html", website );
    delay(250);

    start_WLAN_STATION(qssid, qpass);
    submit = "";
  }
  /* Button connect - END  */

  website += F("</tbody></table></form></body></html>");

  /* Button wps | wps methode for connection */
  if (submit == "wps") {
#ifdef debug_html
    Serial.println(WLAN_AP == 0 ? F("DB web_wlan, wps methode setting (call from station)") : F("DB web_wlan, wps methode setting (call from AP)"));
#endif

    website = F("<!DOCTYPE html>WLAN - Wi-Fi Protected attempt to SSID ");
    website += qssid;
    website += F("<br>If the attempt fails, the old settings are loaded.</html>");

    HttpServer.send(200, "text/html", website);
    delay(250);

    if (!start_WLAN_WPS()) {
      if (WLAN_AP == 1) {
        start_WLAN_AP(ssid_ap, password_ap);
      } else {
        start_WLAN_STATION(EEPROMread_string(EEPROM_ADDR_SSID), EEPROMread_string(EEPROM_ADDR_PASS));
      }
    }
  }
  /* Button wps - END */

  if (countargs == 0) {
#ifdef debug_html
    Serial.println(F("DB web_wlan, send HTML (args == 0)"));
#endif
    HttpServer.send(200, "text/html", website);
  }
}


void web_val_status() {
  String website = F("{\"CC1101\":\"");
  CC1101_found ? website += F("yes") : website += F("no");
  website += F("\",\"RAM\":\"");
  website += freeRam();
  website += F("\",\"Upt_Sec\":\"");
  website += getUptime();
  website += F("\",\"Upt_TXT\":\"");
  website += elapsedDays(getUptime());
  website += F(" day(s)&emsp;");
  website += numberOfHours(getUptime());
  website += F(" hour(s)&emsp;");
  website += numberOfMinutes(getUptime());
  website += F(" minute(s)&emsp;");
  website += numberOfSeconds(getUptime());
  website += F(" second(s)");
  website += + F("\"}");

  HttpServer.send(200, "text/plain", website); // Send value, JSON to client ajax request
}


void routing_websites() {
  HttpServer.on("/", web_index);
  HttpServer.on("/cc110x", web_cc110x);
  HttpServer.on("/cc110x_detail", web_cc110x_detail);
  HttpServer.on("/cc110x_detail_export", web_cc110x_detail_export);
  HttpServer.on("/cc110x_detail_import", web_cc110x_detail_import);
  HttpServer.on("/cc110x_modes", web_cc110x_modes);
  HttpServer.on("/index.htm", web_index);
  HttpServer.on("/index.html", web_index);
  HttpServer.on("/log", web_log);
  HttpServer.on("/raw", web_raw);
  HttpServer.on("/request_status", web_val_status);
  HttpServer.on("/request_raw", web_val_raw);
  HttpServer.on("/wlan", web_wlan);
  HttpServer.serveStatic("/all.css", LittleFS, "/css/all.css");
  HttpServer.serveStatic("/cc110x.css", LittleFS, "/css/cc110x.css");
  HttpServer.serveStatic("/cc110x_detail.css", LittleFS, "/css/cc110x_detail.css");
  HttpServer.serveStatic("/cc110x_detail_exp.css", LittleFS, "/css/cc110x_detail_exp.css");
  HttpServer.serveStatic("/cc110x_detail_imp.css", LittleFS, "/css/cc110x_detail_imp.css");
  HttpServer.serveStatic("/cc110x_modes.css", LittleFS, "/css/cc110x_modes.css");
  HttpServer.serveStatic("/favicon.ico", LittleFS, "/favicon.ico");
  HttpServer.serveStatic("/help", LittleFS, "/html/help.html");
  HttpServer.serveStatic("/help.css", LittleFS, "/css/help.css");
  HttpServer.serveStatic("/help.js", LittleFS, "/js/help.js");
  HttpServer.serveStatic("/index.css", LittleFS, "/css/index.css");
  HttpServer.serveStatic("/index.js", LittleFS, "/js/index.js");
  HttpServer.serveStatic("/log.css", LittleFS, "/css/log.css");
  HttpServer.serveStatic("/raw.css", LittleFS, "/css/raw.css");
  HttpServer.serveStatic("/raw.js", LittleFS, "/js/raw.js");
  HttpServer.serveStatic("/wlan.css", LittleFS, "/css/wlan.css");
  HttpServer.serveStatic("/wlan.js", LittleFS, "/js/wlan.js");

  HttpServer.onNotFound([]() {
    HttpServer.sendHeader("Location", "/", true);   // Redirect to our html
    HttpServer.send(404, "text/plain", F("Website not found !!!"));
  });
}
