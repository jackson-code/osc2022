#include "my_math.h"

double pow(double x, double y) {
    return x * pow(x, y - 1);
}
