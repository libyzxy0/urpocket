from flask import Flask

from routes.health import health_bp
from routes.calendar import calendar_bp
from routes.chat import chat_bp

app = Flask(__name__)

app.register_blueprint(health_bp, url_prefix="/api")
app.register_blueprint(calendar_bp, url_prefix="/api")
app.register_blueprint(chat_bp, url_prefix="/api")

if __name__ == "__main__":
    app.run(debug=True)