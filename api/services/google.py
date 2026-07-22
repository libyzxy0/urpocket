import os
import json
from google.oauth2 import service_account
from googleapiclient.discovery import build
from dotenv import load_dotenv

from config import SCOPES

# Load environment variables from a local .env file if present
load_dotenv()

def calendar_service():
    # Retrieve the JSON string stored in the environment variable
    service_account_info = os.getenv("GOOGLE_SERVICE_ACCOUNT_JSON")
    
    if not service_account_info:
        raise ValueError("GOOGLE_SERVICE_ACCOUNT_JSON environment variable is missing or empty.")

    # Parse the string into a dictionary
    info = json.loads(service_account_info)

    # Load credentials directly from the dictionary
    credentials = service_account.Credentials.from_service_account_info(
        info,
        scopes=SCOPES,
    )

    return build("calendar", "v3", credentials=credentials)