from flask import Blueprint, jsonify, request

from services.chat import (
    process_chat
)

chat_bp = Blueprint("chat", __name__)

@chat_bp.get("/chat")
def date():
    q = request.args.get("q")

    if not q:
        return jsonify({"error": "Missing query parameter 'q'"}), 400

    return jsonify(process_chat(q))