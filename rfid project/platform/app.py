from flask import Flask, render_template, request, redirect, url_for, session, send_file
import sqlite3
from datetime import datetime
import csv
import io

app = Flask(_name_)
app.secret_key = "rfid_secret_2025"

DB_FILE   = "presence.db"
XLSX_FILE = "presence.xlsx"
def get_db():
    conn = sqlite3.connect(DB_FILE)
    return conn

@app.route("/", methods=["GET", "POST"])
def login():
    error = None
    if request.method == "POST":
        username = request.form.get("username", "").strip()
        password = request.form.get("password", "").strip()
        conn = get_db()
        prof = conn.execute(
            "SELECT * FROM professeurs WHERE username=? AND password=?",
            (username, password)
        ).fetchone()
        conn.close()
        if prof:
            session["prof"] = username
            return redirect(url_for("dashboard"))
        else:
            error = "Nom d'utilisateur ou mot de passe incorrect."
    return render_template("login.html", error=error)

@app.route("/logout")
def logout():
    session.clear()
    return redirect(url_for("login"))

@app.route("/dashboard")
def dashboard():
    if "prof" not in session:
        return redirect(url_for("login"))
    today = datetime.now().strftime("%Y-%m-%d")
    today_display = datetime.now().strftime("%d/%m/%Y")
    conn = get_db()
    raw = conn.execute(
        "SELECT uid, nom, prenom, heure, statut FROM presence WHERE date=?",
        (today,)
    ).fetchall()
    conn.close()
    rows = [{"uid": r[0], "nom": r[1], "prenom": r[2], "heure": r[3], "statut": r[4]} for r in raw]
    present_count = sum(1 for r in rows if r["statut"] == "Présent")
    absent_count  = len(rows) - present_count
    return render_template("dashboard.html",
        rows=rows,
        present_count=present_count,
        absent_count=absent_count,
        total=len(rows),
        today=today_display,
        prof=session["prof"]
    )

@app.route("/export")
def export():
    if "prof" not in session:
        return redirect(url_for("login"))
    return send_file(XLSX_FILE, as_attachment=True, download_name="presence.xlsx")

@app.route("/export_csv")
def export_csv():
    if "prof" not in session:
        return redirect(url_for("login"))
    today = datetime.now().strftime("%Y-%m-%d")
    conn = get_db()
    raw = conn.execute(
        "SELECT uid, nom, prenom, heure, statut FROM presence WHERE date=?",
        (today,)
    ).fetchall()
    conn.close()
    output = io.StringIO()
    writer = csv.writer(output)
    writer.writerow(["UID", "Nom", "Prénom", "Heure", "Statut"])
    for r in raw:
        writer.writerow([r[0], r[1], r[2], r[3], r[4]])
    output.seek(0)
    return send_file(
        io.BytesIO(output.getvalue().encode("utf-8-sig")),
        mimetype="text/csv",
        as_attachment=True,
        download_name=f"presence_{today}.csv"
    )

if _name_ == "_main_":
    app.run(debug=True, host='0.0.0.0', port=5000)
