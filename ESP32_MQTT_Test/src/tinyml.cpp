#include "tinyml.h"

namespace {
  tflite::ErrorReporter *error_reporter = nullptr;
  const tflite::Model *model = nullptr;
  tflite::MicroInterpreter *interpreter = nullptr;
  TfLiteTensor *input = nullptr;
  TfLiteTensor *output = nullptr;
  constexpr int kTensorArenaSize = 8 * 1024;
  static uint8_t tensor_arena[kTensorArenaSize];
}

void setupTinyML() {
  Serial.println("Initializing TensorFlow Lite...");
  static tflite::MicroErrorReporter micro_error_reporter;
  error_reporter = &micro_error_reporter;

  model = tflite::GetModel(dht_anomaly_model_tflite);
  if (model->version() != TFLITE_SCHEMA_VERSION) {
    error_reporter->Report("Model schema version %d not supported.", model->version());
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
  Serial.println("TensorFlow Lite Micro initialized.");
}

void tiny_ml_task(void *pvParameters) {
  setupTinyML();

  while (1) {
    if (input && output) {
      input->data.f[0] = glob_temperature;
      input->data.f[1] = glob_humidity;

      TfLiteStatus invoke_status = interpreter->Invoke();
      if (invoke_status != kTfLiteOk) {
        error_reporter->Report("Invoke failed");
      } else {
        float result = output->data.f[0];
        ai_result = result * 100.0;
        alarm_warning = (result > 0.5);
        Serial.print("TinyML result: ");
        Serial.print(result, 4);
        Serial.print(" => alarm_warning=");
        Serial.println(alarm_warning ? "true" : "false");
      }
    }
    vTaskDelay(5000 / portTICK_PERIOD_MS);
  }
}
