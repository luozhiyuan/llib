#include <iostream>

#include "llib/llib.h"
#include "experiment/experiment.h"
int main() {
    llib::test::test_curry_function();
    llib::test::test_is_valid();
    llib::experiment::test::test_integral_type();
    return 0;
}