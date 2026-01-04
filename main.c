#include <stdio.h>

#include "testFrame.h"

extern TEST_SUITE yammTestSuite;

int main(void)
{
    yammTest(&yammTestSuite);

    return 0;
}