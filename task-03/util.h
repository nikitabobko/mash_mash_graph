#ifndef TASK_02_UTIL_H
#define TASK_02_UTIL_H 1

extern float global_bound;

long get_cur_time_millis();

void print_matrix4(const glm::mat4 matrix);

int positive_rand(int bound);

int integer_rand(int bound);

#endif
