from flask import Flask, jsonify
import cv2
import numpy as np
import serial
import time

app = Flask(__name__)

cap = cv2.VideoCapture(0)
ser = serial.Serial('/dev/ttyUSB0', 9600, timeout=1)
ser.flush()

bg = cv2.createBackgroundSubtractorMOG2(history=500, varThreshold=50, detectShadows=False)

MIN_AREA = 1200
STABLE_FRAMES = 3
MIN_SEND_INTERVAL = 0.2

last_cmd = None
last_send_time = 0.0
stable_count = 0
last_detected = False

def send_cmd(cmd: str):
    global last_cmd, last_send_time
    now = time.time()
    if cmd == last_cmd:
        return
    if now - last_send_time < MIN_SEND_INTERVAL:
        return
    ser.write((cmd + "\n").encode("utf-8"))
    last_cmd = cmd
    last_send_time = now

@app.route('/detect', methods=['GET'])
def detect():
    global stable_count, last_detected

    ret, frame = cap.read()
    if not ret:
        return jsonify({'error': 'camera error'}), 500

    gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
    fg = bg.apply(gray)

    kernel = np.ones((5, 5), np.uint8)
    fg = cv2.morphologyEx(fg, cv2.MORPH_OPEN, kernel)
    fg = cv2.morphologyEx(fg, cv2.MORPH_DILATE, kernel)

    contours, _ = cv2.findContours(fg, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)

    detected = False
    max_area = 0
    for c in contours:
        area = cv2.contourArea(c)
        if area > max_area:
            max_area = area
        if area > MIN_AREA:
            detected = True
            break

    if detected == last_detected:
        stable_count += 1
    else:
        stable_count = 0
        last_detected = detected

    if stable_count >= STABLE_FRAMES:
        if detected:
            send_cmd("AUTO")
        else:
            send_cmd("MANUAL")

    return jsonify({'object': detected, 'max_area': float(max_area), 'stable': stable_count})

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)
