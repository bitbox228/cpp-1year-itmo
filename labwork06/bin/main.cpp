#include "lib/parser.h"

using namespace omfl;

int main() {

    std::filesystem::path path("..\\..\\example\\config.omfl");

    const auto root = parse(path);

    path = "..\\..\\example\\config.json";
    root.CreateJSON(path);

    path = "..\\..\\example\\config.xml";
    root.CreateXML(path);

    path = "..\\..\\example\\config.yaml";
    root.CreateYAML(path);

    return 0;
}
