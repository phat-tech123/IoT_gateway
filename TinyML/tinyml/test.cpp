#include <iostream>
#include <fstream>
#include <cstdlib>
#include <ctime>

float randomFloat(float min, float max) {
    return min + static_cast<float>(rand()) / RAND_MAX * (max - min);
}

int main() {
    std::ofstream file("simulated_data.csv");
    std::srand(std::time(0));

    if (!file.is_open()) {
        std::cout << "Cannot open file!\n";
        return 1;
    }

    file << "temp,humidity,label\n";

    for (int i = 0; i < 300; i++) {

        // Mở rộng range để rule có hiệu lực
        float temp = randomFloat(15.0, 40.0);     // 15–40°C
        float hum  = randomFloat(20.0, 100.0);    // 20–100%

        int label = 0;

        // ===== CRITICAL =====
        if (temp >= 35.0 || temp <= 18.0 ||
            hum >= 90.0 || hum <= 30.0) {
            label = 2;
        }
        // ===== WARNING =====
        else if (temp >= 30.0 || temp <= 22.0 ||
                 hum >= 75.0 || hum <= 45.0) {
            label = 1;
        }
        // ===== NORMAL =====
        else {
            label = 0;
        }

        file << temp << "," << hum << "," << label << "\n";
    }

    file.close();
    std::cout << "Data generated successfully!\n";

    return 0;
}