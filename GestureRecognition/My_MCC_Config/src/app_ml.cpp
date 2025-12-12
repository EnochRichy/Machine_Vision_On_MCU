/*******************************************************************************
  Application ML Source File (UPDATED FOR NEW TFLITE-MICRO STANDALONE REPO)
*******************************************************************************/

// *****************************************************************************
// Section: Included Files
// *****************************************************************************

#include "app_ml.h"
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>

#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/c/common.h"

#include "gesture_int8.h"

// --- NEW includes for standalone TFLM ----
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_allocator.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/micro/micro_log.h"
#include "tensorflow/lite/micro/system_setup.h"
#include "gfx/legato/generated/screen/le_gen_screen_Screen0.h"

#include "peripheral/port/plib_port.h"

// *****************************************************************************
// Global Data
// *****************************************************************************

APP_ML_DATA app_mlData;

int8_t recognisedDigit=-1;
int8_t recognitionThreshold=20;

extern int8_t greyscale_img[];
extern bool ml_input_ready;
extern bool inference_complete;

extern leImage gesture;

extern uint8_t Fist_data[21210];
extern uint8_t palm_data[23108];

extern leImageWidget* Screen0_ImageWidget_0;

int number[10];

int8_t prev_best = -1;

// *****************************************************************************
// TensorFlow Lite Micro Configuration
// *****************************************************************************

#define TENSOR_ARENA_SIZE (250 * 1024)
alignas(32) static uint8_t tensor_arena[TENSOR_ARENA_SIZE];


static tflite::MicroInterpreter *interpreter = NULL;
static TfLiteTensor *input_tensor = NULL;
static TfLiteTensor *output_tensor = NULL;

const char *labels[] = {"Palm", "Fist"};


// *****************************************************************************
// NEW: Create a resolver matching your model ops
// *****************************************************************************

// TODO — UPDATE THIS LIST AFTER YOU TELL ME YOUR MODEL OPS
// (I will generate the correct resolver for you)

void RegisterOps(tflite::MicroMutableOpResolver<10> &resolver)
{
    // Add only the ops your model needs:
    resolver.AddQuantize();          // builtin op 114
    resolver.AddConv2D(tflite::Register_CONV_2D_INT8());            // builtin op 3
    resolver.AddDepthwiseConv2D(tflite::Register_DEPTHWISE_CONV_2D_INT8());   // builtin op 17
    resolver.AddFullyConnected(tflite::Register_FULLY_CONNECTED_INT8());    // builtin op 22
    resolver.AddReshape();           // builtin op 9
    resolver.AddSoftmax(tflite::Register_SOFTMAX_INT8());           // builtin op 25
    resolver.AddAveragePool2D();
    resolver.AddMaxPool2D();
    resolver.AddMean();
}


// *****************************************************************************
// Setup Function
// *****************************************************************************

void tflm_setup(void)
{
    // Optional: attach hardware logging for debugging
    tflite::InitializeTarget();

    // Map model from C array
    const tflite::Model *model = tflite::GetModel(gesture_int8);
    if (model->version() != TFLITE_SCHEMA_VERSION) {
        MicroPrintf("Schema mismatch: model %d vs TFLM %d",
                    model->version(), TFLITE_SCHEMA_VERSION);
        return;
    }

    // Create resolver
    static tflite::MicroMutableOpResolver<10> resolver;
    RegisterOps(resolver);

    // Create interpreter
    static tflite::MicroInterpreter static_interpreter(
        model, resolver, tensor_arena, TENSOR_ARENA_SIZE);

    interpreter = &static_interpreter;

    auto* opcodes = model->operator_codes();
    for (int i = 0; i < opcodes->size(); i++) {
        MicroPrintf("Op %d builtin_code: %d", i, opcodes->Get(i)->builtin_code());
    }

    // Allocate tensors
    if (interpreter->AllocateTensors() != kTfLiteOk) {
        MicroPrintf("AllocateTensors FAILED!");
        return;
    }

    input_tensor  = interpreter->input(0);
    output_tensor = interpreter->output(0);
}


// *****************************************************************************
// Quantization Helper
// *****************************************************************************

static inline uint8_t quantize_pixel(uint8_t pixel, float scale, int32_t zero_point)
{
    float f = ((float)pixel) / 255.0f;
    int32_t q = (int32_t)((f / scale) + zero_point + 0.5f);
    if (q < 0) q = 0;
    if (q > 255) q = 255;
    return (uint8_t)q;
}


// *****************************************************************************
// Inference Function
// *****************************************************************************

int predict_gesture_from_frame()
{
    if (!interpreter) return -1;

    GPIO_PA13_Set(); // ML Inference indicator ON

    inference_complete = false;

    float input_scale = input_tensor->params.scale;
    int32_t input_zero_point = input_tensor->params.zero_point;

    uint8_t *input_buf = input_tensor->data.uint8;

    // Copy & quantize input
    for (int i = 0; i < 64 * 64; i++) {
        input_buf[i] = quantize_pixel(greyscale_img[i], input_scale, input_zero_point);
    }

    if (interpreter->Invoke() != kTfLiteOk) {
        MicroPrintf("Invoke FAILED");
        return -2;
    }

    uint8_t *scores = output_tensor->data.uint8;

    int best = 0;
    uint8_t best_score = scores[0];
    
    if (scores[1] > best_score) {
                best = 1;
                best_score = scores[1];
            }

    if (best)
    {
            gesture =
                {
                    {
                        LE_STREAM_LOCATION_ID_INTERNAL, // data location id
                        (void*)Fist_data, // data variable pointer
                        21210, // data size
                    },
                    LE_IMAGE_FORMAT_RAW,
                    {
                        LE_COLOR_MODE_RGB_565,
                        {
                            105,
                            101
                        },
                        10605,
                        21210,
                        (void*)Fist_data, // data variable pointer
                        0, // flags
                    },
                    0, // image flags
                    {
                        0x0, // color mask
                    },
                    NULL, // alpha mask
                    NULL, // palette
                };
    }
    else
    {
        gesture =
                {
                    {
                        LE_STREAM_LOCATION_ID_INTERNAL, // data location id
                        (void*)palm_data, // data variable pointer
                        23108, // data size
                    },
                    LE_IMAGE_FORMAT_RAW,
                    {
                        LE_COLOR_MODE_RGB_565,
                        {
                            109,
                            106
                        },
                        11554,
                        23108,
                        (void*)palm_data, // data variable pointer
                        0, // flags
                    },
                    0, // image flags
                    {
                        0x0, // color mask
                    },
                    NULL, // alpha mask
                    NULL, // palette
                };
    }

    prev_best = best;

    float out_scale = output_tensor->params.scale;
    int32_t out_zero_point = output_tensor->params.zero_point;
    float confidence = (best_score - out_zero_point) * out_scale;

    GPIO_PA13_Clear(); // ML Inference indicator OFF

   printf("Pred: %s (score=%u, conf=%.2f)\r\n",
           labels[best], best_score, confidence);

    inference_complete = true;
    return best;
}


// *****************************************************************************
// Harmony State Machine
// *****************************************************************************

void APP_ML_Initialize(void)
{
    app_mlData.state = APP_ML_STATE_INIT;
    tflm_setup();
}

void APP_ML_Tasks(void)
{
    switch (app_mlData.state)
    {
        case APP_ML_STATE_INIT:
            app_mlData.state = APP_ML_STATE_SERVICE_TASKS;
            break;

        case APP_ML_STATE_SERVICE_TASKS:
            if (ml_input_ready) {
                ml_input_ready = false;
                predict_gesture_from_frame();
            }
            break;

        default:
            break;
    }
}

int8_t APP_ML_GetRecognisedDigit(void)
{
    return recognisedDigit;
}

void APP_ML_ClearRecognisedDigit(void)
{
    recognisedDigit = -1;
}

