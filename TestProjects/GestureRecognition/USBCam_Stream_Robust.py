import usb.core, usb.util, usb.backend.libusb1
import numpy as np
import cv2
import time

# USB configuration
VID  = 0x04D8
PID  = 0x0053
EP_IN = 0x81
PKT   = 512

FRAME_WIDTH        = 160
FRAME_HEIGHT       = 120
FRAME_BPP          = 2
FRAME_PAYLOAD_SIZE = FRAME_WIDTH * FRAME_HEIGHT * FRAME_BPP  # 38400
FRAME_HEADER_SIZE  = 8
FRAME_TOTAL_SIZE   = FRAME_HEADER_SIZE + FRAME_PAYLOAD_SIZE  # 38408

MARKER = bytes([0xAA, 0x55, 0xAA, 0x55])

# Load backend for WinUSB
backend = usb.backend.libusb1.get_backend(
    find_library=lambda x: "./libusb-1.0.dll"
)

dev = usb.core.find(idVendor=VID, idProduct=PID, backend=backend)
assert dev is not None, "USB device not found"
dev.set_configuration()

print("USB camera connected. Press ESC to quit.")

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

            # Remaining bytes after header belong to the frame payload
            frame_data = bytearray(buffer[pos + FRAME_HEADER_SIZE:])
            header_found = True
        else:
            # keep last 3 bytes to handle marker across chunk boundaries
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

    # ---- clamp extra bytes, if any ----
    if len(frame_data) > FRAME_PAYLOAD_SIZE:
        frame_data = frame_data[:FRAME_PAYLOAD_SIZE]

    if len(frame_data) != FRAME_PAYLOAD_SIZE:
        print("Corrupt frame size:", len(frame_data), " → resyncing")
        continue

    # Now we have exactly one full frame payload (38400 bytes)
    # ---------- CONVERT RGB565 -> BGR888 ----------
    # Assume little-endian RGB565: [low, high] per pixel
    buf = np.frombuffer(frame_data, dtype=np.uint8)
    buf = buf.reshape((FRAME_HEIGHT, FRAME_WIDTH, 2))

    rgb565 = (buf[:, :, 1].astype(np.uint16) << 8) | buf[:, :, 0].astype(np.uint16)

    r = ((rgb565 >> 11) & 0x1F) << 3
    g = ((rgb565 >> 5)  & 0x3F) << 2
    b = (rgb565         & 0x1F) << 3

    frame_bgr = np.dstack((b, g, r)).astype(np.uint8)

    # ---------- FPS CALC ----------
    frame_count += 1
    now = time.time()
    if now - last_time >= 1.0:
        fps = frame_count / (now - last_time)
        print(f"Frame ID: {frame_id}, FPS: {fps:.1f}")
        frame_count = 0
        last_time   = now

    # ---------- DISPLAY ----------
    cv2.imshow("USB Camera Stream", frame_bgr)

    if cv2.waitKey(1) & 0xFF == 27:  # ESC
        break

cv2.destroyAllWindows()
