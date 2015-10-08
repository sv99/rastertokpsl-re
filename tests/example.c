//
// Created by svolkov on 06.10.15.
//

#include "minunit.h"

int tests_run = 0;

int foo = 7;
int bar = 5;

char * test_foo() {
        mu_assert("error, foo != 7", foo == 7);
        return 0;
}

char * test_bar() {
        mu_assert("error, bar != 5", bar == 5);
        return 0;
}