
# Gesture Recognition on PIC32CZ CA90 with TensorFlow Lite Micro

Real-time hand gesture recognition running entirely on a **PIC32CZ8110CA90208** (Cortex-M7) microcontroller. An OV7670 camera captures frames, a quantized CNN classifies them on-device using TensorFlow Lite Micro, and results are displayed on a 64x64 HUB75 RGB LED panel.

**Recognized gestures:** Palm, Fist, Thumbs Up

## System Overview

```
OV7670 Camera (160x120 RGB565)
       |
       | DMA + HSYNC/VSYNC
       v
  PIC32CZ CA90 (Cortex-M7)
  +---------------------------------+
  |  Downsample to 64x64 grayscale |
  |  TF Lite Micro inference (INT8)|
  |  Gesture classification        |
  +---------------------------------+
       |                   |
       v                   v
 64x64 HUB75 LED      UART / USB
 (live preview +       (debug output /
  gesture label)       dataset capture)
```

## Hardware

| Component | Details |
|-----------|---------|
| MCU | PIC32CZ8110CA90208 on Curiosity Ultra board |
| Camera | OV7670 module (parallel 8-bit data via Port C, I2C control via SERCOM5) |
| Display | 64x64 HUB75 RGB LED matrix panel (driven via Port D GPIOs) |
| Adapter PCB | Custom camera module adapter (schematic and PCB files in `Hardware/`) |

Pin mapping for the OV7670 connection is documented in [`Hardware/OV7670_Module_PinMapping.xlsx`](Hardware/OV7670_Module_PinMapping.xlsx).

## Repository Structure

```
GestureRecognition/
  My_MCC_Config/
    src/
      main.c              Entry point and super loop
      app_cam.c/.h        OV7670 capture, RGB565-to-grayscale conversion
      app_ml.cpp/.h       TF Lite Micro inference engine
      app_display.c/.h    HUB75 LED panel driver (BCM rendering)
      app_usb.c/.h        USB device for dataset capture
      tensorflow/         TF Lite Micro + CMSIS-NN sources
      config/default/     MCC-generated peripheral drivers (I2C, DMA, GPIO, timers)
    mcc/                  MCC project configuration
  ML Model/
    gesture_int8.tflite   Quantized INT8 model (181 KB, 3 classes)
    gesture_int8.h        Auto-generated C array of the model
    ConvertModelToHex.py  Converts .tflite to C header
  dataset/
    palm/                 Training images (64x64 grayscale PNGs)
    fist/
    thumbsup/
  DatasetCapture_via_USB.py   Capture labeled frames from the board over USB
  cmake/                      CMake build configuration
  _build/                     CMake build tree (can be deleted)
  out/                        Final firmware binaries (.elf)
Hardware/
  OV7670_Module_PinMapping.xlsx
  Camera_Module_Adopter_Updated/   Adapter PCB schematic and layout
```

## ML Model

- **Architecture:** CNN with Conv2D, DepthwiseConv2D, and FullyConnected layers
- **Input:** 64x64x1 grayscale (4096 bytes)
- **Output:** 3 classes -- Palm, Fist, Unknown (softmax scores)
- **Quantization:** Post-training INT8
- **Tensor arena:** 250 KB
- **Optimizations:** CMSIS-NN kernels for Cortex-M DSP acceleration

## Getting Started

### Prerequisites

- [MPLAB X IDE](https://www.microchip.com/mplab/mplab-x-ide) with XC32 compiler
- PIC32CZ CA Curiosity Ultra board
- OV7670 camera module with adapter PCB
- 64x64 HUB75 LED panel
- Python 3.x with `pyusb`, `numpy`, `opencv-python` (for dataset capture)

### Build and Flash

1. Open the project in MPLAB X IDE (or use the VS Code workspace under `.vscode/`).
2. Build via CMake:
   ```
   cd GestureRecognition/_build/ML_OV7670_GFX/default
   cmake ../..
   make
   ```
3. Flash `out/ML_OV7670_GFX/default.elf` to the board using PKOB4 (on-board programmer via SWD).

### Capture Training Data

1. Connect the board over USB.
2. Install Python dependencies:
   ```
   pip install pyusb numpy opencv-python
   ```
3. Ensure `libusb-1.0.dll` is in the repo root (Windows).
4. Run the capture script:
   ```
   cd GestureRecognition
   python DatasetCapture_via_USB.py
   ```
5. Press **P** (palm), **F** (fist), or **T** (thumbs up) to record 200 labeled frames per session.

Images are saved as 64x64 grayscale PNGs under `dataset/`.

### Update the Model

1. Train or retrain the model externally (must accept 64x64 grayscale input and output 3 classes).
2. Export as a quantized INT8 `.tflite` file.
3. Generate the C header:
   ```
   cd GestureRecognition
   python "ML Model/ConvertModelToHex.py"
   ```
4. Rebuild and flash the firmware.

## How It Works

1. **Capture** -- The OV7670 streams 160x120 RGB565 frames via DMA into SRAM. HSYNC/VSYNC interrupts synchronize line and frame boundaries.
2. **Preprocessing** -- Each frame is downsampled to 64x64 and converted to grayscale using fixed-point luminance weights (0.299R + 0.587G + 0.114B).
3. **Inference** -- The grayscale frame is quantized to INT8 and fed to the TF Lite Micro interpreter. The model outputs confidence scores for each gesture class.
4. **Display** -- The live camera preview and recognized gesture label are rendered on the HUB75 panel using Binary Code Modulation (5-bit color depth, gamma corrected).

## Key Design Choices

- **64x64 resolution** balances inference speed (~4 KB per frame) with recognition accuracy.
- **INT8 quantization** is essential for real-time performance on the Cortex-M7 without an FPU-heavy workload.
- **DMA-driven capture** frees the CPU during frame transfer, allowing the main loop to handle display refresh and inference.
- **Fixed-point math** in the camera conversion path avoids floating-point overhead in the frame pipeline.
