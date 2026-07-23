import json
import os
import requests
from dotenv import load_dotenv

load_dotenv()
OPENROUTER_API_KEY = os.getenv("OPENROUTER_API_KEY")

session = requests.Session()


def process_chat(q: str):
    if not OPENROUTER_API_KEY:
        raise ValueError(
            "OPENROUTER_API_KEY is missing from environment variables."
        )

    headers = {
        "Authorization": f"Bearer {OPENROUTER_API_KEY}",
        "Content-Type": "application/json",
    }

    payload = {
        "model": "google/gemma-4-31b-it:freee",
        "messages": [
            {
                "role": "system",
                "content": "Answer in under 30 characters total. Be extremely concise. Output only letters",
            },
            {"role": "user", "content": q},
        ],
        "reasoning": {"enabled": False},
        "max_tokens": 15,
    }

    response = session.post(
        "https://openrouter.ai/api/v1/chat/completions",
        headers=headers,
        data=json.dumps(payload),
    )

    data = response.json()

    if response.status_code != 200 or "choices" not in data:
        error_msg = data.get("error", {}).get("message", "Unknown API error")
        print(f"[OpenRouter Error] Status {response.status_code}: {error_msg}")
        return {"res": f"Error: {response.status_code}"}

    content = data["choices"][0]["message"]["content"].strip()
    return {"res": content}