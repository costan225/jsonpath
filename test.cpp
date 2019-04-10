#include "json/json.hpp"
#include "jsonpath/jsonpath.hpp"
#include <iostream>

using nlohmann::json;
using costan::jsonpath;

// json sample
json j = R"({
    "store": {
        "book": [
            {
                "category": "reference",
                "author": "Nigel Rees",
                "title": "Sayings of the Century",
                "price": 8.95
            },
            {
                "category": "fiction",
                "author": "Evelyn Waugh",
                "title": "Sword of Honour",
                "price": 12.99
            },
            {
                "category": "fiction",
                "author": "Herman Melville",
                "title": "Moby Dick",
                "isbn": "0-553-21311-3",
                "price": 8.99
            },
            {
                "category": "fiction",
                "author": "J. R. R. Tolkien",
                "title": "The Lord of the Rings",
                "isbn": "0-395-19395-8",
                "price": 22.99
            }
        ],
        "bicycle": {
            "color": "red",
            "price": 19.95
        }
    },
    "expensive": 10
})"_json;

int main() {
    const char* paths [] = {
        "$.store.bicycle.color",                                // get casade keys         
        "$.store.book[0,-1].price",                             // get array items by index
        "$.store.book[-2:].price",                              // get array items by range
        "$.store.book[0:3:2].price",                            // get array items with step
        "$.store.book[?(@.isbn&&@.title=='Moby Dick')].price",  // filter statement with relational operator 
        "$.store.book[?(@.price<$.expensive)].price",           // filter statement with reference value
        "$.store.book[?(@.title~=/Sword.*/i)].author",          // filter statement with regex
        "$..price",                                             // deep scan
        0
    };

    for(int i = 0; paths[i]; ++i) {
        std::cout << "----------------------------------" << std::endl;
        auto p = jsonpath(paths[i]);
        std::cout << "Id: " << (i+1) << std::endl;
        std::cout << "PathStr: " << paths[i] << std::endl;
        std::cout << "Parse: " << (p.valid() ? "Success" : "Failed") << std::endl;
        std::cout << "Result: " << p.eval(j).dump() << std::endl;
    }

    return 0;
}