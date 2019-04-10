#include "jsonpath_ast.hpp"
#include "range/v3/all.hpp"

namespace costan { namespace ast {
    using view::filter;
    using ranges::to_vector;
    auto& fold = ranges::accumulate;
    auto& fmap = ranges::view::transform;

    /////////////////////////////////////////////////////////
    //  utility
    /////////////////////////////////////////////////////////

    bool isarray(json* pj) {
        return nullptr != pj && (*pj).is_array();
    }

    bool isobject (json* pj) {
        return nullptr != pj && (*pj).is_object();
    }

    bool isstructure (json* pj) {
        return isarray(pj) || isobject(pj);
    }

    JsonVec getaddrs(json* j) {
        auto fill = [j]() -> JsonVec {
            JsonVec vec;
            for(auto& i : (*j)) {
                vec.emplace_back(&i);
            }
            return vec;
        };
        return (nullptr == j) ? JsonVec() : fill();
    }

    struct HasKey {
        HasKey(std::string s) : k(s) {}
        bool operator() (json* j){
            return (nullptr != j) && 
                ((*j).find(k) != (*j).end());
        }   
        std::string k;
    };

    struct Index {
        Index(json* j) : j_(j) {}
        json* operator()(int idx){
            return &(*j_)[idx];
        }
        json* j_;
    };

    /////////////////////////////////////////////////////////
    //  pred_visitor
    /////////////////////////////////////////////////////////

    bool pred_visitor::operator()(vregex r) const {
       auto& tar = (*j_)[p_.key];
        return (p_.oper == "~=") && tar.is_string() && 
            std::regex_match(tar.get<std::string>(), std::regex(r.str)); 
    }
        
    bool pred_visitor::operator()(vref v) const {
        if (v.type == '@'){
            return HasKey(v.key)(j_) && (*this)((*j_)[v.key]);
        } else if (v.type == '$') {
            return HasKey(v.key)(r_) && (*this)((*r_)[v.key]);
        } else {
            return false;
        }
    }

    /////////////////////////////////////////////////////////
    //  cond_visitor
    /////////////////////////////////////////////////////////

    bool cond_visitor::operator()(predicate p) const {
        auto check = [this, p](){
            return apply_visitor(pred_visitor(j_, r_, p), p.val);
        };
        return HasKey(p.key)(j_) && check();
    }

    bool cond_visitor::operator()(std::string key) const {
        return HasKey(key)(j_);
    }

    /////////////////////////////////////////////////////////
    //  oper_fold
    /////////////////////////////////////////////////////////

    bool oper_fold::operator()(bool flag, operand op) {
        auto res = apply_visitor(cond_visitor(j_, r_), op.cond);
        if(op.logic == "||") {
            return flag || res;
        } else if(op.logic == "&&") {
            return flag && res;
        } else {
            return res;
        }
    }

    /////////////////////////////////////////////////////////
    //  suffix_visitor
    /////////////////////////////////////////////////////////

    JsonVec suffix_visitor::operator()(struct filter f) const {
        auto addr = std::addressof<json>;
        // check filter conditions
        auto check = [this, f](json &j) -> bool {
            auto init = apply_visitor(cond_visitor(&j, r_), f.first);
            return fold(f.lasts, init, oper_fold(&j, r_));
        };
        // get valid items from json::array
        auto get = [check, addr](json *j) -> JsonVec {
            return  (*j) | filter(check) | fmap(addr) | to_vector;
        };
        return jv_ | filter(isarray) | fmap(get) | action::join;
    }

    JsonVec suffix_visitor::operator()(scope pair) const {
        // convert negative index
        auto unify = [](json *j, int idx) -> int {
            auto ridx = std::max<int>(0, (*j).size()+idx);
            return (idx >= 0) ? idx : ridx;
        };
        // get items from array
        auto get = [unify, pair](json *j) -> JsonVec {
            // get optional value
            auto ob = pair.begin.get_value_or(0);
            auto oe = pair.end.get_value_or((*j).size());
            auto os = pair.step.get_value_or(1);
            os = std::max<int>(os, 1);
            // coordinate indexs
            auto beg = unify(j, ob);
            auto end = std::min<int>(unify(j, oe), (*j).size());
            // sample algorithm
            return view::ints(beg, std::max(end, beg)) | 
                    filter([&os](int i){ return !(i % os); }) | 
                    fmap(Index(j)) | 
                    to_vector; 
        };
                
        return jv_ | filter(isarray) | fmap(get) | action::join;
    }

    JsonVec suffix_visitor::operator()(std::vector<int> idxs) const {
        // convert negative indexs 
        auto unify = [](json *j, int idx) -> int {
            auto ridx = std::max<int>(0, (*j).size()+idx);
            return (idx >= 0) ? idx : ridx;
        };
        // validate indexs
        auto check = [unify](json* j, int idx) -> bool {
            return idx >=0 && idx < (*j).size();
        };
        // get items from array
        auto get = [unify, check, idxs](json *j) -> JsonVec {
            auto fm = std::bind(unify, j, std::placeholders::_1);
            auto fn = std::bind(check, j, std::placeholders::_1);
            return idxs | fmap(fm) | filter(fn) | fmap(Index(j)) | to_vector; 
        };

        return jv_ | filter(isarray) | fmap(get) | action::join;
    }

    JsonVec suffix_visitor::operator()(std::string wildcard) const {
        // convert object to pointer
        auto Addr = [](json *j) -> JsonVec { 
            int size = (*j).size();
            return view::ints(0, size) | fmap(Index(j)) | to_vector;
        };
        return jv_ | filter(isarray) | fmap(Addr) | action::join; 
    }

    /////////////////////////////////////////////////////////
    //  node_fold
    ///////////////////////////////////////////////////////// 

    JsonVec node_fold::getnode(JsonVec& vec, node n) {
        // get key object
        auto fn = [n](json *j) -> json* {
            return &(*j)[n.key];
        };
        // get init values
        auto ref = vec | filter(HasKey(n.key)) | fmap(fn) | to_vector;
        // evaluate suffix 
        auto suffix_eval = [this](JsonVec& vec, suffix s) -> JsonVec {
            return apply_visitor(suffix_visitor(vec, r_), s);
        };
        return fold(n.suffixs, ref, suffix_eval);
    }

    JsonVec node_fold::getall(JsonVec& vec, node n) {
        using recfn = std::function<JsonVec(json*)>;
        // validate json sub items
        auto valid = [](json* j) -> bool {
            return j->is_structured();
        };
        // get all sub obj and array
        recfn rfn = [valid, &rfn](json* j) -> JsonVec {
            auto src = getaddrs(j);
            auto son = src | filter(valid) | to_vector;
            auto grandson = son | fmap(rfn) | action::join;
            return view::concat(son, grandson);
        };
        // add self node
        auto get = [rfn](json *j) -> JsonVec {
            auto self = JsonVec({j});
            auto sons = rfn(j);
            return view::concat(self, sons);
        };

        return vec | filter(isstructure) | fmap(get) | action::join;
    }

    /////////////////////////////////////////////////////////
    //  attribute analyse interface
    ///////////////////////////////////////////////////////// 

    JsonVec path_items(json& j, expression& e) {
        if( e.nodes.empty() ) {
            return JsonVec();
        } else {
            return fold(e.nodes, 
                JsonVec({&j}), 
                node_fold(&j));
        }
    }

}}