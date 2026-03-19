#include <iostream>
#include <string>
#include <functional>

int main() {
    std::cout << std::hash<std::string>{}("VIPOverlay.dds") << std::endl;
    return 0;
}
