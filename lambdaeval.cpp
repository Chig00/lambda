#include <cstdlib>

/**
 * Compiles and runs lambda.cpp.
 */
int main() {
    int error;
    
    if (!(error = system("g++ lambda.cpp -o lambda"))) {
        return system("lambda");
    }
    
    else {
        return error;
    }
}