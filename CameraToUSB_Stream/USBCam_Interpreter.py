import usb.core, usb.util, usb.backend.libusb1
import numpy as np
import cv2
import time
import tensorflow as tf

# ------------------------------
# TFLITE MODEL LOADING
# ------------------------------

interpreter = tf.lite.Interpreter(model_path="gesture_int8.tflite")
interpreter.allocate_tensors()

input_details  = interpreter.get_input_details()
output_details = interpreter.get_output_details()

print("Model loaded:", input_details, output_details)

GESTURE_LABELS = ["Palm", "Fist", "Thumbs-Up"]


def run_inference(img64):
    """
    img64: 64x64 grayscale uint8 image (0-255)
    """
    img = img64.reshape(1, 64, 64, 1).astype(np.uint8)

    interpreter.set_tensor(input_details[0]['index'], img)
    interpreter.invoke()

    output = interpreter.get_tensor(output_details[0]['index'])[0]
    pred_idx = np.argmax(output)
    confidence = output[pred_idx] / 255.0  # since INT8 output is uint8

    return pred_idx, confidence


# ------------------------------
# USB CAMERA SETUP
# ------------------------------

VID  = 0x04D8
PID  = 0x0053
EP_IN = 0x81
PKT   = 512

FRAME_WIDTH        = 64
FRAME_HEIGHT       = 64
FRAME_BPP          = 1
FRAME_PAYLOAD_SIZE = FRAME_WIDTH * FRAME_HEIGHT * FRAME_BPP
FRAME_HEADER_SIZE  = 8
MARKER = bytes([0xAA, 0x55, 0xAA, 0x55])

# Load WinUSB backend
backend = usb.backend.libusb1.get_backend(find_library=lambda x: "./libusb-1.0.dll")
dev = usb.core.find(idVendor=VID, idProduct=PID, backend=backend)
assert dev is not None, "USB camera not found"
dev.set_configuration()

print("USB Camera connected. Running inference...")


last_time = time.time()
frame_count = 0

while True:
    # ---------- SYNC TO FRAME HEADER ----------
    buffer = b''
    header_found = False

    while not header_found:
        try:
            chunk = bytes(dev.read(EP_IN, PKT, timeout=1000))
        except usb.core.USBTimeoutError:
            print("Timeout waiting for header — resyncing...")
            continue

        buffer += chunk
        pos = buffer.find(MARKER)

        if pos != -1 and len(buffer) >= pos + FRAME_HEADER_SIZE:
            header = buffer[pos:pos + FRAME_HEADER_SIZE]
            frame_id = int.from_bytes(header[4:8], byteorder='little')

            frame_data = bytearray(buffer[pos + FRAME_HEADER_SIZE:])
            header_found = True
        else:
            if len(buffer) > 3:
                buffer = buffer[-3:]

    # ---------- READ FRAME PAYLOAD ----------
    while len(frame_data) < FRAME_PAYLOAD_SIZE:
        try:
            chunk = bytes(dev.read(EP_IN, PKT, timeout=1000))
        except usb.core.USBTimeoutError:
            print("Timeout mid-frame — resyncing...")
            frame_data = bytearray()
            break
        frame_data.extend(chunk)

    if len(frame_data) != FRAME_PAYLOAD_SIZE:
        continue

    # Convert to numpy (64x64 grayscale)
    frame_gray = np.frombuffer(frame_data, dtype=np.uint8)
    frame_gray = frame_gray.reshape((FRAME_HEIGHT, FRAME_WIDTH))

    # ---------- RUN INFERENCE ----------
    pred, conf = run_inference(frame_gray)

    label_text = f"{GESTURE_LABELS[pred]} ({conf*100:.1f}%)"

    # ---------- DISPLAY ----------
    big = cv2.resize(frame_gray, (256, 256), interpolation=cv2.INTER_NEAREST)
    cv2.putText(big, label_text, (10, 245),
                cv2.FONT_HERSHEY_SIMPLEX, 
                0.7, (255), 2)

    cv2.imshow("Gesture Recognition", big)

    # Quit on ESC
    if cv2.waitKey(1) & 0xFF == 27:
        break

    # FPS counter
    frame_count += 1
    now = time.time()
    if now - last_time >= 1.0:
        print(f"FPS: {frame_count / (now - last_time):.1f}, Prediction: {label_text}")
        last_time = now
        frame_count = 0

cv2.destroyAllWindows()
