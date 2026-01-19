import usb.core, usb.util, usb.backend.libusb1
import numpy as np
import cv2
import time, os

# USB configuration
VID  = 0x04D8
PID  = 0x0053
EP_IN = 0x81
PKT   = 512

FRAME_WIDTH        = 64
FRAME_HEIGHT       = 64
FRAME_BPP          = 1
FRAME_PAYLOAD_SIZE = FRAME_WIDTH * FRAME_HEIGHT * FRAME_BPP  # 4096
FRAME_HEADER_SIZE  = 8
FRAME_TOTAL_SIZE   = FRAME_HEADER_SIZE + FRAME_PAYLOAD_SIZE  # 4096

MARKER = bytes([0xAA, 0x55, 0xAA, 0x55])

# -------- Data Capture Configuration --------
CAPTURE_FRAMES = 200
save_folders = {
    "p": "dataset/palm",
    "f": "dataset/fist",
    "t": "dataset/thumbsup"
}

# Create directories if missing
for folder in save_folders.values():
    os.makedirs(folder, exist_ok=True)

recording = False
record_label = None
frames_collected = 0

# Load backend for WinUSB
backend = usb.backend.libusb1.get_backend(
    find_library=lambda x: "./libusb-1.0.dll"
)

dev = usb.core.find(idVendor=VID, idProduct=PID, backend=backend)
assert dev is not None, "USB device not found"
dev.set_configuration()

print("USB camera connected.")
print("Press P = palm   F = fist   T = thumbsup   ESC = quit")
print(f"Each capture will store {CAPTURE_FRAMES} frames.\n")

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

    # ---------- READ REST OF FRAME PAYLOAD ----------
    while len(frame_data) < FRAME_PAYLOAD_SIZE:
        try:
            chunk = bytes(dev.read(EP_IN, PKT, timeout=1000))
        except usb.core.USBTimeoutError:
            print("Timeout mid-frame — resyncing...")
            frame_data = bytearray()
            break
        frame_data.extend(chunk)

    if len(frame_data) > FRAME_PAYLOAD_SIZE:
        frame_data = frame_data[:FRAME_PAYLOAD_SIZE]

    if len(frame_data) != FRAME_PAYLOAD_SIZE:
        print("Corrupt frame size:", len(frame_data), " → resyncing")
        continue

    # Convert to image
    frame_gray = np.frombuffer(frame_data, dtype=np.uint8)
    frame_gray = frame_gray.reshape((FRAME_HEIGHT, FRAME_WIDTH))

    # ---------- Display ----------
    display = cv2.resize(frame_gray, (256,256), interpolation=cv2.INTER_NEAREST)
    cv2.imshow("USB Camera Stream", display)

    # ---------- FPS Calculation ----------
    frame_count += 1
    now = time.time()
    if now - last_time >= 1.0:
        fps = frame_count / (now - last_time)
        print(f"Frame ID: {frame_id}, FPS: {fps:.1f}")
        frame_count = 0
        last_time   = now

    # ---------- Handle Key Press ----------
    key = cv2.waitKey(1) & 0xFF

    if key == 27:  # ESC
        break

    if key in [ord("p"), ord("f"), ord("t")] and not recording:
        recording = True
        frames_collected = 0
        record_label = chr(key)
        print(f"\n >>> Recording {CAPTURE_FRAMES} frames for label '{record_label}'...\n")

    # ---------- Save frames if recording ----------
    if recording:
        folder = save_folders[record_label]
        filename = f"{record_label}_{frames_collected:04d}.png"
        filepath = os.path.join(folder, filename)

        cv2.imwrite(filepath, frame_gray)
        frames_collected += 1

        if frames_collected >= CAPTURE_FRAMES:
            print(f" >>> Saved {CAPTURE_FRAMES} frames to {folder}\n")
            recording = False

cv2.destroyAllWindows()
