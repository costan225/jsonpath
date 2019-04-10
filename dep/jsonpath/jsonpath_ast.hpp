#ifndef __JSONPATH_AST_HPP__
#define __JSONPATH_AST_HPP__

#include "common.hpp"
#include <boost/spirit/home/x3/support/ast/variant.hpp>
#include <vector>
#include <string>

namespace costan { namespace ast {

    /////////////////////////////////////////////////////////
    //  the AST
    ///////////////////////////////////////////////////////// 

    using x3::variant;
    using boost::optional;

    struct vregex {
        std::string str;
    };

    struct vref {
        char type;
        std::string key;
    };

    struct value : variant<
        double, 
        std::string,
        vregex,
        vref>
    {
        using base_type::base_type;
        using base_type::operator=;
    };

    struct predicate {
        std::string key;
        std::string oper;
        value val;
    };

    struct condition : variant<
        predicate,
        std::string
        >
    {
        using base_type::base_type;
        using base_type::operator=;
    };

    struct operand {
        std::string logic;
        condition cond;
    };

    struct filter {
        condition first;
        std::vector<operand> lasts;
    };

    struct scope {
        optional<int> begin;
        optional<int> end;
        optional<int> step;
    };

    struct suffix : variant<
        filter,
        scope,
        std::vector<int>,
        std::string
        >
    {
        using base_type::base_type;
        using base_type::operator=;
    };

    struct node {
        bool empty() {
            return key.empty() && suffixs.empty();
        }
        std::string key;
        std::vector<suffix> suffixs;
    };

    struct expression {
        std::vector<node> nodes;
    };


    /////////////////////////////////////////////////////////
    // algorithm functor
    ///////////////////////////////////////////////////////// 

    class pred_visitor : public boost::static_visitor<bool> {
    public:
        pred_visitor(json* data, json* root, predicate p)
            : j_(data), r_(root), p_(p) {}
        // regex value
        bool operator() (vregex r) const;
        // reference value
        bool operator() (vref v) const;
        // normal value
        template<typename T>
        bool operator() (T& t) const {
            auto &tar = (*j_)[p_.key];
            auto op = p_.oper;
            if("==" == op){
                return tar == t;
            } else if("<" == op){
                return tar < t;
            } else if(">" == op){
                return tar > t;
            } else if("<=" == op){
                return tar <= t;
            } else if(">=" == op){
                return tar >= t;
            } else if(":=" == op){
                return (tar = t) || true; 
            } else {
                return false;
            }
        }
    private:
        json* j_;
        json* r_;
        predicate p_;
    };

    class cond_visitor : public boost::static_visitor<bool> {
    public:
        cond_visitor(json* data, json* root)
        : j_(data), r_(root) {}
        bool operator ()(predicate p) const;
        bool operator ()(std::string key) const;
    private:
        bool checkpred(predicate p) const;
    private:
        json* j_;
        json* r_;
    };

    class oper_fold {
    public:
        oper_fold(json* data, json* root) 
            : j_(data), r_(root) {}
        bool operator()(bool flag, operand op);
    private:
        json* j_;
        json* r_;
    };

    class suffix_visitor : public boost::static_visitor<JsonVec> {
    public:
        suffix_visitor(JsonVec jv, json* root) 
            : jv_(jv), r_(root) {};   
        JsonVec operator() (struct filter f) const;
        JsonVec operator() (scope s) const;
        JsonVec operator() (std::vector<int> idxs) const;
        JsonVec operator() (std::string wildcard) const;
    private:
        mutable JsonVec jv_;
        mutable json* r_;
    };

    class node_fold {
    public:
        node_fold(json* root) : r_(root) {}
        inline JsonVec operator()(JsonVec& vec, node n) {
            return n.empty() ? getall(vec, n) : getnode(vec, n);
        }
    private:
        JsonVec getnode(JsonVec& vec, node n);
        JsonVec getall(JsonVec& vec, node n);
    private:
        json* r_;
    };

    /////////////////////////////////////////////////////////
    // get path JsonVec interface
    /////////////////////////////////////////////////////////

    JsonVec path_items(json& j, expression& e);

}} // ast costan

#endif