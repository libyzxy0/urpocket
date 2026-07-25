#ifndef CALENDARAPP_H
#define CALENDARAPP_H

class CalendarApp {
private:
  static const int MAX_EVENTS = 20;
  static const int ITEMS_PER_PAGE = 4;

  struct LocalEvent {
    String summary;
    String code;
    String startTime;
    String timeRange;
    String location;
  };

  enum AppState {
    STATE_MINI_CALENDAR,
    STATE_EVENT_LIST,
    STATE_EVENT_DETAIL
  };

  AppState currentState = STATE_MINI_CALENDAR;
  int currentYear = 2026;
  int currentMonth = 7;
  int selectedDay = 23;
  int daysInMonth = 31;
  int startDayOfWeek = 3; 

  bool eventsLoaded = false;
  int currentEventIndex = 0;
  int totalEvents = 0;
  LocalEvent events[MAX_EVENTS];

  String getFormattedDate(int year, int month, int day) {
    char buffer[11];
    snprintf(buffer, sizeof(buffer), "%04d-%02d-%02d", year, month, day);
    return String(buffer);
  }

  // Helper method to parse "Location (Professor)" into separate strings
  void parseLocationAndProf(const String &rawLoc, String &cleanLoc, String &prof) {
    int openParen = rawLoc.indexOf('(');
    int closeParen = rawLoc.indexOf(')', openParen);

    if (openParen != -1 && closeParen != -1 && closeParen > openParen) {
      cleanLoc = rawLoc.substring(0, openParen);
      cleanLoc.trim();
      prof = rawLoc.substring(openParen + 1, closeParen);
      prof.trim();
    } else {
      cleanLoc = rawLoc.length() > 0 ? rawLoc : "None";
      prof = "N/A";
    }
  }

  void fetchEventsForDate(AppNetworkManager &net, String dateStr) {
    eventsLoaded = false;
    totalEvents = 0;
    currentEventIndex = 0;

    String url = "https://urpocket.libyzxy0.me/api/calendar/date?q=" + dateStr;
    String jsonResponse = net.fetchHttpsApi(url.c_str());
    
    DynamicJsonDocument doc(4096);
    DeserializationError error = deserializeJson(doc, jsonResponse);

    if (!error && doc.is<JsonArray>()) {
      JsonArray array = doc.as<JsonArray>();
      for (JsonObject obj : array) {
        if (totalEvents >= MAX_EVENTS) break;

        events[totalEvents].summary   = obj["summary"]   | "No Summary";
        events[totalEvents].code      = obj["code"]      | "";
        events[totalEvents].startTime = obj["startTime"] | "";
        events[totalEvents].timeRange = obj["timeRange"] | "";
        events[totalEvents].location  = obj["location"]  | "";

        totalEvents++;
      }
    }
    eventsLoaded = true;
  }

public:
  void run(Adafruit_SH1106G &display, AppNetworkManager &net, AudioSystem &audio, bool up, bool down, bool select, bool selectDoubleClicked) {
    if (up || down || select || selectDoubleClicked) {
      audio.playClickSound();
    }

    if (selectDoubleClicked) {
      currentState = STATE_MINI_CALENDAR;
    } else {
      switch (currentState) {
        case STATE_MINI_CALENDAR:
          if (down) {
            selectedDay = (selectedDay % daysInMonth) + 1;
          } else if (up) {
            selectedDay = (selectedDay - 2 + daysInMonth) % daysInMonth + 1; 
          } else if (select) {
            display.clearDisplay();
            display.setTextColor(SH110X_WHITE);
            display.setTextSize(1);
            display.setCursor(0, 0);
            display.print("Loading Schedule...");
            display.display();

            fetchEventsForDate(net, getFormattedDate(currentYear, currentMonth, selectedDay));
            currentState = STATE_EVENT_LIST;
          }
          break;

        case STATE_EVENT_LIST:
          if (totalEvents > 0) {
            if (down) {
              currentEventIndex = (currentEventIndex + 1) % totalEvents; 
            } else if (up) {
              currentEventIndex = (currentEventIndex - 1 + totalEvents) % totalEvents;
            } else if (select) {
              currentState = STATE_EVENT_DETAIL;
            }
          }
          break;

        case STATE_EVENT_DETAIL:
          if (up || down || select) {
            currentState = STATE_EVENT_LIST; 
          }
          break;
      }
    }

    display.clearDisplay();
    display.setTextColor(SH110X_WHITE);
    display.setTextSize(1);
    display.setTextWrap(false);

    if (currentState == STATE_MINI_CALENDAR) {
      display.setCursor(0, 0);
      display.print("CALENDAR   ");
      display.print(getFormattedDate(currentYear, currentMonth, selectedDay));
      display.drawLine(0, 10, 128, 10, SH110X_WHITE);

      const char* daysOfWeek = "S  M  T  W  T  F  S";
      display.setCursor(2, 14);
      display.print(daysOfWeek);

      int gridStartX = 2;
      int gridStartY = 24;
      int cellWidth = 18;
      int cellHeight = 8;

      for (int day = 1; day <= daysInMonth; day++) {
        int dayIndex = startDayOfWeek + day - 1;
        int col = dayIndex % 7;
        int row = dayIndex / 7;

        int x = gridStartX + (col * cellWidth);
        int y = gridStartY + (row * cellHeight);

        if (y > 56) break;

        if (day == selectedDay) {
          display.fillRect(x - 1, y - 1, 14, cellHeight, SH110X_WHITE);
          display.setTextColor(SH110X_BLACK);
        } else {
          display.setTextColor(SH110X_WHITE);
        }

        display.setCursor(x, y);
        if (day < 10) display.print("0");
        display.print(day);
      }

    } else if (currentState == STATE_EVENT_LIST) {
      int totalPages = (totalEvents > 0) ? ((totalEvents + ITEMS_PER_PAGE - 1) / ITEMS_PER_PAGE) : 1;
      int currentPage = currentEventIndex / ITEMS_PER_PAGE;
      int pageStartIndex = currentPage * ITEMS_PER_PAGE;
      int pageEndIndex = min(pageStartIndex + ITEMS_PER_PAGE, totalEvents);

      display.setCursor(0, 0);
      display.print("EVENTS ");
      display.print(currentPage + 1);
      display.print("/");
      display.print(totalPages);

      display.drawLine(0, 10, 128, 10, SH110X_WHITE);

      if (totalEvents == 0) {
        display.setCursor(10, 25);
        display.print("No Events Found!");
        display.setCursor(10, 42);
        display.print("Dbl-click to Back");
      } else {
        int linePos = 0;
        for (int i = pageStartIndex; i < pageEndIndex; i++) {
          int y = 14 + (linePos * 11);
          bool selected = (i == currentEventIndex);

          if (selected) {
            display.fillRect(0, y - 1, 128, 10, SH110X_WHITE);
            display.setTextColor(SH110X_BLACK, SH110X_WHITE);
          } else {
            display.setTextColor(SH110X_WHITE, SH110X_BLACK);
          }

          String leftStr = events[i].startTime;
          if (leftStr.length() == 0) leftStr = "--:--";

          String rightStr = events[i].code;
          if (rightStr.length() == 0) {
            rightStr = events[i].summary;
          }
          if (rightStr.length() == 0) {
            rightStr = "Event " + String(i + 1);
          }

          display.setCursor(2, y);
          display.print(leftStr);

          int rightX = 126 - (rightStr.length() * 6);
          if (rightX < 40) rightX = 40; 

          display.setCursor(rightX, y);
          display.print(rightStr);

          linePos++;
        }
      }
    } else if (currentState == STATE_EVENT_DETAIL) {
      LocalEvent &e = events[currentEventIndex];
      
      String cleanLocation, professor;
      parseLocationAndProf(e.location, cleanLocation, professor);

      String headerTitle = (e.code.length() > 0) ? e.code : "EVENT";
      display.setCursor(0, 0);
      display.print(headerTitle);
      display.print(" DETAIL");

      String headerTime = e.timeRange;
      if (headerTime.length() > 0) {
        int timeX = 126 - (headerTime.length() * 6); 
        if (timeX > 50) {
          display.setCursor(timeX, 0);
          display.print(headerTime);
        }
      }

      display.drawLine(0, 10, 128, 10, SH110X_WHITE);

      display.setTextWrap(true);
      int currentY = 13;

      display.setCursor(0, currentY);
      display.print(e.summary);
 
      int summaryLines = (e.summary.length() + 20) / 21;
      if (summaryLines < 1) summaryLines = 1;
      currentY += (summaryLines * 9);

      if (currentY <= 45) { 
        display.setCursor(0, currentY);
        display.print(professor);

        int profLines = ((professor.length() + 3) + 20) / 21;
        if (profLines < 1) profLines = 1;
        currentY += (profLines * 9);
      }

      if (currentY <= 55) {
        display.setCursor(0, currentY);
        display.print(cleanLocation);
      }

      display.setTextWrap(false);
    }

    display.display();
  }
};

#endif // CALENDARAPP_H
