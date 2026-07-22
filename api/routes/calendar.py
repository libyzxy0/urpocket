from flask import Blueprint, jsonify, request

from services.calendar import (
    get_today_events,
    get_week_schedule,
    get_events_by_date,
)

calendar_bp = Blueprint("calendar", __name__)


@calendar_bp.get("/calendar/today")
def today():
    return jsonify(get_today_events())


@calendar_bp.get("/calendar/week")
def week():
    return jsonify(get_week_schedule())


@calendar_bp.get("/calendar/date")
def date():
    q = request.args.get("q")

    if not q:
        return jsonify({"error": "Missing query parameter 'q'"}), 400

    return jsonify(get_events_by_date(q))