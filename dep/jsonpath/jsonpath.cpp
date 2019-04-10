#include "jsonpath.hpp"
#include "jsonpath_ast.hpp"
#include "gramma.hpp"

// define costan
namespace costan {
    auto& fmap = view::transform;

    jsonpath::jsonpath(std::string s) {
        str_ = s;
        path_ = std::make_shared<ast::expression>();
        if(!costan::compile(s, *path_, err_)) {
            str_.clear();
            path_->nodes.clear();
        }
    }

    bool jsonpath::compile(std::string s){
        str_.clear();
        path_->nodes.clear();
        if(!costan::compile(s, *path_, err_)) {
            str_.clear();
            path_->nodes.clear();
            return false;
        }
        return true;
    }

    json jsonpath::eval(json& j) {
        auto items = ast::path_items(j, *path_);
        return items | view::indirect | action::join;
    }

    json jsonpath::update(json& j, json& data) {
        auto update = [data](json* i) -> json {
        (*i).update(data);
        return (*i);
        };
        auto items = ast::path_items(j, *path_);
        return items | fmap(update) | action::join;
    }

} //costan