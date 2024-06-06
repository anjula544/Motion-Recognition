#include <Wire.h>
#include <Anjula2001-project-1_inferencing.h>

#define CONVERT_G_TO_MS2    9.80665f
#define MAX_ACCEPTED_RANGE  2.0f

void setup() {
    Serial.begin(115200);
    while (!Serial);

    // Initialize I2C communication
    Wire.begin();

    // Initialize MPU6050
    Wire.beginTransmission(0x68); // MPU6050 address
    Wire.write(0x6B); // PWR_MGMT_1 register
    Wire.write(0); // Wake up MPU6050
    Wire.endTransmission(true);
}

void loop() {
    Serial.println("\nStarting inferencing in 2 seconds...");
    delay(2000);
    Serial.println("Sampling...");

    float buffer[EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE] = { 0 };

    // Read accelerometer data from MPU6050
    Wire.beginTransmission(0x68); // MPU6050 address
    Wire.write(0x3B); // Start with register 0x3B (ACCEL_XOUT_H)
    Wire.endTransmission(false);
    Wire.requestFrom(0x68, 6, true); // Request 6 bytes (ACCEL_XOUT_H, ACCEL_XOUT_L, ACCEL_YOUT_H, ACCEL_YOUT_L, ACCEL_ZOUT_H, ACCEL_ZOUT_L)

    // Read accelerometer data
    int16_t ax = (Wire.read() << 8) | Wire.read();
    int16_t ay = (Wire.read() << 8) | Wire.read();
    int16_t az = (Wire.read() << 8) | Wire.read();

    // Convert to float and adjust for gravity
    buffer[0] = (float)ax * CONVERT_G_TO_MS2 / 16384.0f; // MPU6050 sensitivity: 16384 LSB/g
    buffer[1] = (float)ay * CONVERT_G_TO_MS2 / 16384.0f;
    buffer[2] = (float)az * CONVERT_G_TO_MS2 / 16384.0f;

    // Process the data further as needed
    // For example, you can filter or normalize the data here

    // Continue with the remaining code for inference
    // Allocate signal from buffer
    signal_t signal;
    int err = numpy::signal_from_buffer(buffer, EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE, &signal);
    if (err != 0) {
        Serial.println("Failed to create signal from buffer");
        return;
    }

    // Run classifier
    ei_impulse_result_t result = { 0 };
    err = run_classifier(&signal, &result, false);
    if (err != EI_IMPULSE_OK) {
        Serial.println("Failed to run classifier");
        return;
    }

    // Print predictions
    Serial.println("Predictions:");
    for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
        Serial.print("    ");
        Serial.print(result.classification[ix].label);
        Serial.print(": ");
        Serial.println(result.classification[ix].value, 5);
    }
}
