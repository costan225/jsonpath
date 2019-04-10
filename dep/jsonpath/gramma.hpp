#ifndef __JSONPATH_GRAMMA_HPP__
#define __JSONPATH_GRAMMA_HPP__

#include "jsonpath_ast.hpp"

namespace costan { 
    bool compile(std::string src, ast::expression& exp, std::string& err);
}

#endif