#ifndef __JSONPATH_HPP__
#define __JSONPATH_HPP__

#include "json/json.hpp"
#include <string>
#include <memory>

namespace costan { namespace ast {
    struct expression;
}}

namespace costan {
    using json = nlohmann::json;
    class jsonpath {
    public:
        jsonpath(std::string s = "");
        
        inline std::string str() { return str_; }
        inline std::string err() { return err_; }
        inline bool valid() {
            return (!str_.empty()) && err_.empty();
        }
        bool compile(std::string path);
        json eval(json& j);
        json update(json& j, json& data);
    private:
        std::string str_;
        std::string err_;
        std::shared_ptr<ast::expression> path_;
    };
}

#endif