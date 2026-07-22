from google.oauth2 import service_account
from googleapiclient.discovery import build

from config import SCOPES

SERVICE_ACCOUNT_FILE = "storage/service-account.json"

def calendar_service():
    credentials = service_account.Credentials.from_service_account_file(
        SERVICE_ACCOUNT_FILE,
        scopes=SCOPES,
    )

    return build("calendar", "v3", credentials=credentials)