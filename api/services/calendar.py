from datetime import datetime, timedelta
from zoneinfo import ZoneInfo

from config import CALENDAR_ID
from services.google import calendar_service

TIMEZONE = ZoneInfo("Asia/Manila")


def _format_events(items):
    events = []

    for event in items:
        events.append({
            "id": event["id"],
            "summary": event.get("summary"),
            "description": event.get("description"),
            "location": event.get("location"),
            "start": event["start"].get("dateTime", event["start"].get("date")),
            "end": event["end"].get("dateTime", event["end"].get("date")),
        })

    return events


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