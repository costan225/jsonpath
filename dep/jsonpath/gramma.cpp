#include "jsonpath_ast.hpp"
#include <boost/fusion/include/adapt_struct.hpp>

namespace ast = costan::ast;

// boost fusion adapt struct 
BOOST_FUSION_ADAPT_STRUCT(ast::vregex, str);
BOOST_FUSION_ADAPT_STRUCT(ast::vref, type, key);
BOOST_FUSION_ADAPT_STRUCT(ast::predicate, key, oper, val);
BOOST_FUSION_ADAPT_STRUCT(ast::operand, logic, cond);
BOOST_FUSION_ADAPT_STRUCT(ast::filter, first, lasts);
BOOST_FUSION_ADAPT_STRUCT(ast::scope, begin, end, step);
BOOST_FUSION_ADAPT_STRUCT(ast::node, key, suffixs);
BOOST_FUSION_ADAPT_STRUCT(ast::expression, nodes);

namespace costan { namespace gramma {
using x3::alnum;
using x3::string;
using x3::char_;
using x3::int_;
using x3::uint_;
using x3::double_;

x3::rule<class expression, ast::expression> const expression("expression");
x3::rule<class node, ast::node> const node("node");
x3::rule<class suffix, ast::suffix> const suffix("suffix");
x3::rule<class scope, ast::scope> const scope("scope");
x3::rule<class filter, ast::filter> const filter("filter");
x3::rule<class condition, ast::condition> const condition("condition");
x3::rule<class oper, std::string> const oper("oper");
x3::rule<class logic, std::string> const logic("logic");
x3::rule<class key, std::string> const key("key");
x3::rule<class value, ast::value> const value("value");
x3::rule<class vregex, ast::vregex> const vregex("vregex");
x3::rule<class vref, ast::vref> const vref("vref");

auto const expression_def = "$." >> (node % '.');

auto const node_def = *(alnum) >> *(suffix);

auto const suffix_def = '[' >> (filter | scope | (int_ % ',') | string("*")) >> ']';

auto const scope_def = (-int_ >> ':' >> -int_ >> -(':' >> uint_));

auto const filter_def = "?(" >> condition >> *(oper >> condition) >> ")";

auto const condition_def = "@." >> ((key >> logic >> value) | key);

auto const oper_def = string("||") | string("&&");

auto const logic_def = 
        ( string("<")
        | string(">")
        | string("<=") 
        | string(">=") 
        | string("==")
        | string("~=")
        );

auto const key_def = +alnum;

auto const vregex_def = '/' >> +(~char_('/')) >> "/i";

auto const vref_def = ( char_('@') | char_('$') ) >> '.' >> key;

auto const str_val = ('\'' >> +(~char_('\'')) >> '\'');

auto const value_def = ( str_val | vregex | vref | double_ );

BOOST_SPIRIT_DEFINE(
    expression, node, suffix, scope, filter, condition, oper, logic, key, vregex, vref, value
);
}} //gramma 
    
namespace costan {
bool compile(std::string src, ast::expression& exp, std::string& err) {
    auto& def = gramma::expression;
    auto it = src.begin();
    bool r = parse(it, src.end(), def, exp);
    err = std::string(it, src.end());
    return r && (it == src.end());
}
} //costan



