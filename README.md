jsonpath (adapt for nlohmann/json)
===========

A C++ jsonpath library using Boost.Spirit V3 and range-v3 for the parser.

The goal of the jsonpath library was to create a conformant jsonpath parser that could be used for nlohmann/json.

follow the majority rules in http://goessner.net/articles/JsonPath/
but also with some minor differences.

this library is till bleeding edge, so use it at your own risk.


# Quick Example


```c++
// header files
#include "json/json.hpp"
#include "jsonpath/jsonpath.hpp"

using nlohmann
using coatan::jsonpath;

jsonpath p("$.school.rank");
if(p.valid()) {
    json j; // sample data
    json r = p.val(j); // result
}  
```

# Examples

given these example data

```javascript
{
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
}
```

example json path syntax.
----

| jsonpath | result|
| :--------- | :-------|
| $.store.bicycle.color 			               | ["red"] |
| $.store.book[0,-1].price                         | [8.95, 22.99] |
| $.store.book[-2:].price                          | [8.99, 22.99] |
| $.store.book[0:3:2].price                        | [8.95, 8.99] |
| $.store.book[?(@.isbn&&@.title=='Moby Dick')].price        | [8.99] |
| $.store.book[?(@.price<$.expensive)].price       | [8.95, 8.99] |
| $.store.book[?(@.title~=/Sword.*/i)].author      | ["Evelyn Waugh"] |
| $..price                                         | [19.95, 8.95, 12.99, 8.99, 22.99] |


# Building 

We have CMake support.

    cd jsonspirit
    mkdir build
    cd build
    cmake ..
    make

# Licensing

`jsonpath` is licensed under the MIT License.
