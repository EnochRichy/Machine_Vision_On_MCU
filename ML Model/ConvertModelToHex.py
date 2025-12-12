# convert_to_header.py
import binascii

with open("gesture_int8.tflite", "rb") as f:
    data = f.read()

with open("gesture_int8.h", "w") as f:
    f.write('#ifndef GESTURE_INT8_H_\n#define GESTURE_INT8_H_\n\n')
    f.write(f'const unsigned char gesture_int8[] = {{\n')
    for i, b in enumerate(data):
        f.write(f'0x{b:02x},')
        if (i + 1) % 12 == 0:
            f.write('\n')
    f.write('\n};\n')
    f.write(f'const unsigned int gesture_int8_len = {len(data)};\n\n')
    f.write('#endif // GESTURE_INT8_H_\n')
