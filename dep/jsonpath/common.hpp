#include "json/json.hpp"
#include "range/v3/all.hpp"
#include <boost/spirit/home/x3.hpp>

namespace costan {
    namespace x3 = boost::spirit::x3;
    namespace view = ranges::view;
    namespace action = ranges::action;
    using json = nlohmann::json;
    using JsonVec = std::vector<json*>;
    using boost::apply_visitor;
}