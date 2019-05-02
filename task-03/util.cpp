#include <chrono>
#include <glm/glm.hpp>
#include "util.h"
#include <cstdio>

float global_bound = 6.f;

long get_cur_time_millis() {
    using namespace std::chrono;
    milliseconds ms = duration_cast<milliseconds>(
            system_clock::now().time_since_epoch()
    );
    return ms.count();
}

void print_matrix4(const glm::mat4 matrix) {
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            printf("%f ", matrix[i][j]);
        }
        printf("\n");
    }
}

int positive_rand(int bound) {
    return rand() % bound;
}

int integer_rand(int bound) {
    return rand() % 2 == 0 ? positive_rand(bound) : -positive_rand(bound);
}
