#include <format>
#include <iostream>

int main(int argc, char *argv[]) {
    std::cout << __cplusplus << std::endl;
    std::cout << std::format("Hello C++{}!\n", 20);
}