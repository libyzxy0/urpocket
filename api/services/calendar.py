from datetime import datetime, timedelta
import re
from zoneinfo import ZoneInfo

from config import CALENDAR_ID
from services.google import calendar_service

TIMEZONE = ZoneInfo("Asia/Manila")


def parse_course_code(summary: str) -> str:
    """Extracts course code prefix (e.g. 'IT102' from 'IT102 - Intro to Computing')."""
    if not summary:
        return ""
    match = re.match(r"^([A-Za-z]{2,4}\s?\d{2,4})", summary.strip())
    return match.group(1) if match else ""


def format_time_range(start_iso: str, end_iso: str) -> tuple[str, str]:
    """Converts ISO 8601 strings to firmware 'HH:MM' start time and 'HH:MM-HH:MM' range."""
    if not start_iso or not end_iso:
        return "", ""

    start_dt = datetime.fromisoformat(start_iso)
    end_dt = datetime.fromisoformat(end_iso)

    start_time_str = start_dt.strftime("%H:%M")
    end_time_str = end_dt.strftime("%H:%M")

    return start_time_str, f"{start_time_str}-{end_time_str}"


def _format_events(items: list) -> list:
    """Transforms raw Google Calendar events into firmware-compatible payloads."""
    formatted_events = []

    for item in items:
        # Extract ISO timestamps or fall back to date string
        start_raw = item.get("start", {}).get("dateTime") or item.get("start", {}).get("date", "")
        end_raw = item.get("end", {}).get("dateTime") or item.get("end", {}).get("date", "")

        # Handle timed vs all-day events
        if "T" in start_raw:
            start_time, time_range = format_time_range(start_raw, end_raw)
        else:
            start_time, time_range = "All Day", "All Day"

        summary = item.get("summary", "No Summary")
        course_code = parse_course_code(summary)
        location = item.get("location") or ""

        formatted_events.append({
            "summary": summary,
            "code": course_code,
            "startTime": start_time,
            "timeRange": time_range,
            "location": location
        })

    return formatted_events


def get_today_events():
    service = calendar_service()
    now = datetime.now(TIMEZONE)

    start = now.replace(hour=0, minute=0, second=0, microsecond=0)
    end = start + timedelta(days=1)

    response = service.events().list(
        calendarId=CALENDAR_ID,
        timeMin=start.isoformat(),
        timeMax=end.isoformat(),
        singleEvents=True,
        orderBy="startTime",
    ).execute()

    return _format_events(response.get("items", []))


def get_week_schedule():
    service = calendar_service()
    now = datetime.now(TIMEZONE)

    monday = (now - timedelta(days=now.weekday())).replace(
        hour=0,
        minute=0,
        second=0,
        microsecond=0,
    )
    sunday = monday + timedelta(days=7)

    response = service.events().list(
        calendarId=CALENDAR_ID,
        timeMin=monday.isoformat(),
        timeMax=sunday.isoformat(),
        singleEvents=True,
        orderBy="startTime",
    ).execute()

    return _format_events(response.get("items", []))


def get_events_by_date(date: str):
    """
    date format: YYYY-MM-DD
    Example: 2026-07-20
    """
    service = calendar_service()

    start = datetime.fromisoformat(date).replace(tzinfo=TIMEZONE)
    end = start + timedelta(days=1)

    response = service.events().list(
        calendarId=CALENDAR_ID,
        timeMin=start.isoformat(),
        timeMax=end.isoformat(),
        singleEvents=True,
        orderBy="startTime",
    ).execute()

    return _format_events(response.get("items", []))