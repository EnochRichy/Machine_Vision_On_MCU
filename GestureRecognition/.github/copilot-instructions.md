## Quick orientation

This repository contains a small embedded gesture-recognition project with three main parts:
- PC-side data capture & tooling (Python scripts under the repo root and `ML Model/`).
- An ML model (TensorFlow Lite `.tflite`) and a generated C header (`gesture_int8.h`) in `ML Model/`.
- The embedded firmware configuration and TensorFlow Lite Micro integration under `My_MCC_Config/` and `_build/` (MPLAB/MCC artifacts and TFLM sources).

When making code or model changes, follow the short workflows below to be productive.

## Key files & locations (source of truth)
- `DatasetCapture_via_USB.py` — captures 64×64 grayscale frames from the USB camera into `dataset/{palm,fist,thumbsup}`. Uses `libusb-1.0.dll`, `pyusb`, `numpy`, `opencv`.
- `dataset/` — stores captured images. Filenames use label prefixes: `p_` = palm, `f_` = fist, `t_` = thumbsup; images are 64×64 grayscale PNGs.
- `ML Model/gesture_int8.tflite` — current quantized model used by the firmware.
- `ML Model/ConvertModelToHex.py` — converts the `.tflite` into a C header array.
- `ML Model/gesture_int8.h` — generated C array + `gesture_int8_len`; included by the embedded project.
- `My_MCC_Config/src/tensorflow/...` — TensorFlow Lite Micro sources. See the `cmsis_nn/README.md` here for TFLM/CMSIS-NN make flags.

## Project-specific conventions and assumptions
- Image input shape: 64×64 grayscale (FRAME_WIDTH/FRAME_HEIGHT in `DatasetCapture_via_USB.py`). Any retraining or preprocessing must match this.
- Capture session size: `CAPTURE_FRAMES = 200` (script saves 200 frames per recording session by default).
- USB device IDs are hard-coded: VID=0x04D8, PID=0x0053 and `libusb-1.0.dll` is referenced from `DatasetCapture_via_USB.py` — change these if using a different device.
- The embedded project expects `gesture_int8.h` to export: `const unsigned char gesture_int8[]` and `const unsigned int gesture_int8_len`.

## Reproducible workflows (concrete commands)
- Capture dataset (Windows PowerShell):
  - Ensure `libusb-1.0.dll` is in repo root, and Python deps are installed: `pip install pyusb numpy opencv-python`.
  - Run: `python .\DatasetCapture_via_USB.py` and press P/F/T to record labeled frames.
- Convert updated model to header (run from repo root where `gesture_int8.tflite` lives):
  - `python .\"ML Model\"\ConvertModelToHex.py`  # produces/overwrites `ML Model/gesture_int8.h`
  - Verify `gesture_int8.h` contains `gesture_int8_len` and the `gesture_int8[]` array.
- Build TensorFlow Lite Micro with CMSIS-NN optimizations (when modifying TFLM or kernels):
  - See `My_MCC_Config/src/tensorflow/lite/micro/tools/make/Makefile`.
  - Example to enable CMSIS-NN optimized kernels: run from TFLM root (or use the path above):
    - `make -f tensorflow/lite/micro/tools/make/Makefile OPTIMIZED_KERNEL_DIR=cmsis_nn TARGET=cortex_m_generic kernel_conv_test`
  - If using external CMSIS or Cortex DFP, pass `CMSIS_PATH`, `CMSIS_NN_PATH`, or `CORTEX_DFP_PATH` as shown in that README.

## Integration points & what to watch for
- Model -> Firmware: after training or replacing `gesture_int8.tflite` you must regenerate `gesture_int8.h` and re-flash the firmware so the binary includes the new model.
- TFLM optimizations: the repo contains TFLM sources; builds can select CMSIS-NN via Makefile flags — this affects binary size and performance on Cortex-M devices.
- USB capture: device-specific endpoints and marker bytes are implemented in `DatasetCapture_via_USB.py`. If a different camera/firmware is used, update the constants (VID, PID, EP_IN, MARKER, frame layout).

## Small examples to reference when coding
- When adding new dataset labels follow the existing folder & prefix convention in `dataset/` and update `save_folders` in `DatasetCapture_via_USB.py`.
- When changing input preprocessing, keep the 64×64 grayscale contract in all places: capture script, training pipeline, and embedded input preprocessing.

## If you're unsure / quick checklist before a PR
- Does the model input shape still match 64×64 grayscale? (check `DatasetCapture_via_USB.py` and your training pipeline)
- If the `.tflite` changed, did you run `ConvertModelToHex.py` and commit the updated `gesture_int8.h`?
- If you changed TFLM or kernel options, test the micro build with the Makefile examples in `My_MCC_Config/src/tensorflow/...`.

Please review this file and tell me if you'd like more details (flash/build steps for MPLAB/MCC, CI hooks, or test harnesses). I'll iterate on gaps you point out.
