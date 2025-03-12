#include "../include/extern.h"
#include "../include/eval.h"
#include <math.h>

double fac(double x)
{
    if (x < 0)
        return NAN;
    if (x == 0 || x == 1)
        return 1;

    double result = 1;
    for (int i = 2; i <= (int)x; i++)
    {
        result *= i;
    }
    return result;
}

double hypot3d(double x, double y, double z)
{
    return sqrt(x * x + y * y + z * z);
}

void add_extern_functions()
{
    // Trigonometric functions
    add_function("sin", sin, 1);
    add_function("cos", cos, 1);
    add_function("tan", tan, 1);
    add_function("asin", asin, 1);
    add_function("acos", acos, 1);
    add_function("atan", atan, 1);
    add_function("atan2", atan2, 2);

    // Hyperbolic functions
    add_function("sinh", sinh, 1);
    add_function("cosh", cosh, 1);
    add_function("tanh", tanh, 1);

    // Exponential and logarithmic functions
    add_function("exp", exp, 1);
    add_function("ln", log, 1);
    add_function("log10", log10, 1);

    add_function("log", log10, 1);

    add_function("fac", fac, 1);
    add_function("round", round, 1);
    add_function("hypot3d", hypot3d, 3);
}
