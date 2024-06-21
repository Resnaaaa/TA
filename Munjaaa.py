from datetime import datetime
import pandas as pd
import pytz
from flask import Flask, jsonify, request

app = Flask(__name__)

all_data = {
    "CO2": [],
    "CO": [],
    "NH4": [],
}


@app.route("/")
def root_route():
    """Route untuk menunjukkan data yang sudah tersimpan"""
    df = pd.DataFrame.from_dict(all_data)
    return df.to_html(), 200


@app.route("/submit", methods=["GET"])
def submit_query():
    """Route untuk memasukkan data menggunakan GET,
    data dimasukkan sebagai query di URL."""
    timestamp = datetime.now(tz=pytz.timezone("Asia/Jakarta")).strftime(
        "%d/%m/%Y %H:%M:%S"
    )
    CO2 = float(request.args.get("CO2"))
    CO = float(request.args.get("CO"))
    NH4 = float(request.args.get("NH4"))

    all_data["CO2"].append(CO2)
    all_data["CO"].append(CO)
    all_data["NH4"].append(NH4)
    return jsonify(
        {
            "CO2": CO2,
            "CO": CO,
            "NH4": NH4,
        }
    )


if __name__ == "__main__":
    app.run(debug=True, host="0.0.0.0")
