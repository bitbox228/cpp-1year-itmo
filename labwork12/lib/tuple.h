#include <iostream>
#include <variant>
#include <vector>
#include <string>
#include <optional>

namespace MyCoolNamespace {

    enum TYPE {
        NONE = 0,
        BOOL,
        INT,
        FLOAT,
        DOUBLE,
        VARCHAR
    };

    using Value = std::optional<std::variant<bool, int, float, double, std::string>>;

    using Tuple = std::vector<Value>;
}

