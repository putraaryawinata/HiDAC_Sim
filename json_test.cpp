#include <iostream>
#include <json/value.h>
#include <json/reader.h>

int main() {
    Json::Value root;
    root["test"] = "hello";
    std::cout << "JSON test passed!" << std::endl;
    return 0;
}
