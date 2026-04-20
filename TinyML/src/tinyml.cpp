#include "tinyml.h"
// Dán giá trị từ Python vào đây
#define TEMP_MEAN 28.06113209
#define TEMP_STD  5.80027787
#define HUM_MEAN  62.96128358
#define HUM_STD   19.82929712
// Globals, for the convenience of one-shot setup.
namespace {
tflite::ErrorReporter* error_reporter = nullptr;
const tflite::Model* model = nullptr;
tflite::MicroInterpreter* interpreter = nullptr;
TfLiteTensor* input = nullptr;
TfLiteTensor* output = nullptr;
constexpr int kTensorArenaSize = 8 * 1024; // Adjust size based on your model
uint8_t tensor_arena[kTensorArenaSize];
} // namespace

void setupTinyML(){
    Serial.println("TensorFlow Lite Init....");
    static tflite::MicroErrorReporter micro_error_reporter;
    error_reporter = &micro_error_reporter;

    // model = tflite::GetModel(dht_anomaly_model_tflite); // g_model_data is from model_data.h
    model = tflite::GetModel(environment_classify_model); // g_model_data is from model_data.h
    
    if (model->version() != TFLITE_SCHEMA_VERSION) {
    error_reporter->Report("Model provided is schema version %d, not equal to supported version %d.",
                            model->version(), TFLITE_SCHEMA_VERSION);
    return;
    }

    static tflite::AllOpsResolver resolver;
    static tflite::MicroInterpreter static_interpreter(
        model, resolver, tensor_arena, kTensorArenaSize, error_reporter);
    interpreter = &static_interpreter;

    TfLiteStatus allocate_status = interpreter->AllocateTensors();
    if (allocate_status != kTfLiteOk) {
    error_reporter->Report("AllocateTensors() failed");
    return;
    }

    input = interpreter->input(0);
    output = interpreter->output(0);


    Serial.println("TensorFlow Lite Micro initialized on ESP32.");
}

void tiny_ml_task(void *pvParameters){
    
    setupTinyML();

    while(1){
       
        // Prepare input data (e.g., sensor readings)
        // For a simple example, let's assume a single float input
        if (xSemaphoreTake(sensorMutex, portMAX_DELAY) == pdTRUE) {
            // input->data.f[0] = glob_temperature; 
            // input->data.f[1] = glob_humidity; 

            Serial.print(glob_temperature);
            Serial.print(",");
            Serial.print(glob_humidity);
            Serial.print("\n");
            float temp_norm = (glob_temperature - TEMP_MEAN) / TEMP_STD;
            float hum_norm  = (glob_humidity - HUM_MEAN) / HUM_STD;

            input->data.f[0] = temp_norm;
            input->data.f[1] = hum_norm;
            xSemaphoreGive(sensorMutex);
        }

        // Run inference
        TfLiteStatus invoke_status = interpreter->Invoke();
        if (invoke_status != kTfLiteOk) {
        error_reporter->Report("Invoke failed");
        return;
        }

        // Get and process output
        // Get output probabilities
        float* probabilities = output->data.f;

        // Print raw probabilities (debug)
        Serial.println("Probabilities:");
        for (int i = 0; i < 3; i++) {
            Serial.print("Class ");
            Serial.print(i);
            Serial.print(": ");
            Serial.println(probabilities[i], 6);
        }

        // Find argmax
        int predicted_class = 0;
        float max_value = probabilities[0];

        for (int i = 1; i < 3; i++) {
            if (probabilities[i] > max_value) {
                max_value = probabilities[i];
                predicted_class = i;
            }
        }

        Serial.print("Predicted class: ");
        Serial.println(predicted_class);

        
        xSemaphoreTake(aiMutex, portMAX_DELAY);
        ai_prob = max_value;
        ai_class = predicted_class;
        xSemaphoreGive(aiMutex);

        // Optional: print label name
        switch (predicted_class) {
            case 0:
                Serial.println("Condition: Normal\n");
                break;
            case 1:
                Serial.println("Condition: Warning\n");
                break;
            case 2:
                Serial.println("Condition: Critical\n");
                break;
        }
        vTaskDelay(5000); 
    }
}