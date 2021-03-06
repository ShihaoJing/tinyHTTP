//
// Created by Shihao Jing on 6/25/17.
//

#ifndef ZION_ROUTING_H
#define ZION_ROUTING_H

#include <unordered_map>
#include <string>
#include <memory>
#include "request.h"
#include "response.h"
#include "utility.h"

#define ALPHABET_SIZE 128
#define PARAMTYPE_NUM  6

namespace zion {

class BaseRule
{
public:
  BaseRule(std::string rule)
      : rule_(rule)
  {
  }

  virtual ~BaseRule()
  {
  }
  //virtual void validate();

  virtual response handle(const request&, const util::routing_param&)
  {
    return response(response::not_found);
  }

  std::string rule_;
  std::string name_;
  unsigned int method_{(int)HTTPMethod::GET};
};

class Rule : public BaseRule
{
public:
  Rule(std::string rule)
      : BaseRule(rule)
  {
  }

  Rule& name(std::string name) {
    name_ = name;
    return *this;
  }

  template <typename Func>
  void operator() (Func f) {
    handler_ = [f] {
      return response(f());
    };
  }

  template <typename Func>
  void operator() (std::string name, Func f) {
    name_ = name;
    handler_ = [f] {
      return response(f());
    };
  }

  response handle(const request&, const util::routing_param&) {
    return handler_();
  }

private:
  std::function<response()> handler_;
};


template <typename ... Args>
class ParamRule : public BaseRule
{
private:
  template <typename T, int Pos>
  struct call_pair
  {
    using type = T;
    static const int pos = Pos;
  };

  template <typename H1, typename H2>
  struct call_params
  {
    H1 &handler;
    H2 &handler_with_req;
    const util::routing_param &params;
    const request &req;
  };

  template <typename F, int NInt, int NFloat, int NString, typename S1, typename S2> struct call
  {
  };

  template <typename F, int NInt, int NFloat, int NString, typename ... Args1, typename ... Args2>
  struct call<F, NInt, NFloat, NString, util::S<int64_t, Args1...>, util::S<Args2...>>
  {
    static response handle(F &cparams)
    {
      using pushed = typename util::S<Args2...>::template push_back<call_pair<int64_t, NInt>>;
      return call<F, NInt+1, NFloat, NString,
                  util::S<Args1...>, pushed>::handle(cparams);
    }
  };

  template <typename F, int NInt, int NFloat, int NString, typename ... Args1, typename ... Args2>
  struct call<F, NInt, NFloat, NString, util::S<float_t, Args1...>, util::S<Args2...>>
  {
    static response handle(F &cparams)
    {
      using pushed = typename util::S<Args2...>::template push_back<call_pair<float_t, NFloat>>;
      return call<F, NInt, NFloat+1, NString,
                  util::S<Args1...>, pushed>::handle(cparams);
    }
  };

  template <typename F, int NInt, int NFloat, int NString, typename ... Args1, typename ... Args2>
  struct call<F, NInt, NFloat, NString, util::S<std::string, Args1...>, util::S<Args2...>>
  {
    static response handle(F &cparams)
    {
      using pushed = typename util::S<Args2...>::template push_back<call_pair<std::string, NString>>;
      return call<F, NInt, NFloat, NString+1,
                  util::S<Args1...>, pushed>::handle(cparams);
    }
  };


  template <typename F, int NInt, int NFloat, int NString, typename ... Args1>
  struct call<F, NInt, NFloat, NString, util::S<>, util::S<Args1...>>
  {
    static response handle(F &cparams)
    {
      if (cparams.handler) {
        return cparams.handler(cparams.params.template get<typename Args1::type>(Args1::pos)... );
      }
      if (cparams.handler_with_req) {
        return cparams.handler_with_req(cparams.req, cparams.params.template get<typename Args1::type>(Args1::pos)... );
      }
      return response::not_found;
    }
  };
public:
  using self_t = ParamRule<Args...>;
  ParamRule(std::string rule) : BaseRule(rule){

  }

  self_t& name(std::string name) {
    name_ = name;
    return *this;
  }

  self_t& method(HTTPMethod method) {
    method_ = (int)method;
    return *this;
  }

  template <typename Func>
  typename std::enable_if<util::CallChecker<Func, util::S<Args...>>::value, void>::type
  operator() (Func f) {
    static_assert(util::CallChecker<Func, util::S<Args...>>::value,
                  "Handler types mismatch with URL args");
    static_assert(!std::is_same<void, decltype(f(std::declval<Args>()...))>::value,
                  "Handler function cannot have void return type");
    handler_ = [f](Args ... args) {
      return response(f(args...));
    };
  }

  template <typename Func>
  typename std::enable_if<!util::CallChecker<Func, util::S<Args...>>::value, void>::type
  operator() (Func &&f) {
    static_assert(util::CallChecker<Func, util::S<request, Args...>>::value,
                  "Handler types mismatch with URL args");
    static_assert(!std::is_same<void, decltype(f(std::declval<request>(), std::declval<Args>()...))>::value,
                  "Handler function cannot have void return type");
    handler_with_req_ = [f = std::move(f)](const request &req, Args ... args) {
      return response(f(req, args...));
    };
  }

  bool match (const request &req) {
    return req.uri == rule_;
  }

  response handle(const request& req, const util::routing_param &params) {
    call_params<decltype(handler_), decltype(handler_with_req_)> cp{handler_, handler_with_req_, params, req};
    return
        call<call_params<decltype(handler_), decltype(handler_with_req_)>, 0, 0, 0, util::S<Args...>, util::S<>>
        ::handle(cp);
  }

private:
  std::function<response(Args...)> handler_;
  std::function<response(request, Args...)> handler_with_req_;
};

class Trie
{
  struct TrieNode
  {
    TrieNode *children[ALPHABET_SIZE] = {};
    TrieNode *param_children[PARAMTYPE_NUM] = {};

    int rule_index = -1;
  };

  enum class ParamType
  {
    INT,
    FLOAT,
    STRING
  };

  struct ParamTraits
  {
    ParamType type;
    std::string name;
  };

public:
  Trie() : root_(new TrieNode)
  {
  }

   void insert(std::string key, int rule_index) {
    TrieNode *cur = root_;
    for (int i = 0; i < key.length(); ++i) {
      char c = key[i];
      if (c == '<') {
        bool found = false;

        static ParamTraits paramTraits[] =
            {
                { ParamType::INT, "<int>" },
                { ParamType::FLOAT, "<float>"},
                { ParamType::STRING, "<string>"}
            };

        for(auto it = std::begin(paramTraits); it != std::end(paramTraits); ++it)
        {
          if (key.compare(i, it->name.size(), it->name) == 0)
          {
            if (!cur->param_children[(int)it->type]) {
              cur->param_children[(int)it->type] = new TrieNode;
            }
            cur = cur->param_children[(int)it->type];
            i += it->name.size() - 1; // for loop will increment i by 1
            found = true;
            break;
          }
        }
        if (!found) {
          throw std::runtime_error("Invalid parameter type" + key);
        }
      }
      else {
        int index = key[i];
        if (!cur->children[index]) {
          cur->children[index] = new TrieNode;
        }
        cur = cur->children[index];
      }
    }
    cur->rule_index = rule_index;
  }

  int search(std::string key, util::routing_param &routing_params) {
    TrieNode *cur = root_;
    for (size_t i = 0; i < key.length(); /* */) {
      char c = key[i];
      if (c == '/') {
        int index = key[i];
        if (!cur->children[index])
          return -1;
        cur = cur->children[index];
        ++i;
      }
      else {
        /*try to match param pattern*/
        int j = key.find_first_of('/', i);
        if (j == std::string::npos)
          j = key.length() - 1;
        else --j;
        bool matched = false;

        std::string arg_substr = key.substr(i, j - i + 1);

        // <float> pattern
        if (cur->param_children[1]) {
          try {
            float_t value = std::stof(arg_substr);
            routing_params.float_params.push_back(value);
            i = j + 1;
            cur = cur->param_children[1];
            matched = true;
          }
          catch(std::exception const & e) {
            // do nothing
          }
        }

        // <int> pattern
        if (cur->param_children[0]) {
          try {
            int64_t value = std::stoi(arg_substr);
            routing_params.int_params.push_back(value);
            i = j + 1;
            cur = cur->param_children[0];
            matched = true;
          }
          catch(std::exception const & e) {
            // do nothing
          }
        }

        // <string> pattern
        if (cur->param_children[2]) {
          routing_params.string_params.push_back(arg_substr);
          cur = cur->param_children[2];
          i = j + 1;
          matched = true;
        }


        if (matched) continue;

        // no param pattern match, do normal path matching
        while (i < key.length() && key[i] != '/') {
          int index = key[i];
          if (!cur->children[index])
            return -1;
          cur = cur->children[index];
          ++i;
        }
      }
    }

    if (cur == nullptr || cur->rule_index == -1)
      return -1;

    return cur->rule_index;
  }

private:
  TrieNode *root_;
};

class Router
{
public:
  Router() : trie_()
  {
  }

  template <uint64_t N>
  typename util::arguments<N>::type::template rebind<ParamRule>& new_param_rule(std::string rule) {
    using RuleT = typename util::arguments<N>::type::template rebind<ParamRule>;
    auto ruleObject = new RuleT(rule);
    rules_.emplace_back(ruleObject);
    trie_.insert(rule, rules_.size() - 1);
    return *ruleObject;
  }

  Rule& new_rule(std::string rule) {
    Rule *r(new Rule(rule));
    rules_.emplace_back(r);
    trie_.insert(rule, rules_.size() - 1);
    return *r;
  }

  response handle(const request &req)
  {
    util::routing_param routing_params;
    int rule_index = trie_.search(req.uri, routing_params);

    if (rule_index == -1)
      return response(response::not_found);

    if (rules_[rule_index]->method_ != req.method_code)
      return response(response::not_found);

    return rules_[rule_index]->handle(req, routing_params);
  }

private:
  std::vector<std::unique_ptr<BaseRule>> rules_;
  Trie trie_;
};

}

#endif //ZION_ROUTING_H
