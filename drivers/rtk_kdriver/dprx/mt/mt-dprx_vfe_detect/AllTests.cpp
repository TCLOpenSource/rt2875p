#include "CppUTest/CommandLineTestRunner.h"
#include "fff.h"
DEFINE_FFF_GLOBALS;

int main(int argc, char** argv)
{
    return RUN_ALL_TESTS(argc, argv);
}
