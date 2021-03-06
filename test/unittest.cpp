//
// Created by Shihao Jing on 6/27/17.
//

#include "gtest/gtest.h"
#include "zion.h"

using namespace zion;
using namespace std;

TEST(Routing, SimplePath) {
  /*Zion app;
  ZION_ROUTE(app, "/<int>/hello")
      ([](int a) {   // resp should be response type
        return "hello, world";
      });
  ZION_ROUTE(app, "/hello/<int>")
      ([](int a) {   // resp should be response type
        return "hello";
      });

  app.route("/")
      ([]{
        return "hello world";
      });

  {
    request req;
    req.uri = "/";
    response res = app.handle(req);
    EXPECT_EQ(static_cast<int>(response::ok), static_cast<int>(res.status_));
  }*/
}

TEST(Routing, Trie) {
  {
    Trie *trie = new Trie;

    vector<string> keys = {"/id/<int>", "/weight/<float>", "/name/<string>"};
    for (int i = 0; i < keys.size(); ++i)
      trie->insert(keys[i], i);

    util::routing_param routing_params;
    ASSERT_EQ(1, trie->search("/weight/1.23", routing_params));
    ASSERT_FLOAT_EQ(1.23, routing_params.float_params.back());
    ASSERT_EQ(0, trie->search("/id/123", routing_params));
    ASSERT_EQ(123, routing_params.int_params.back());
    ASSERT_EQ(2, trie->search("/name/hodor", routing_params));
    EXPECT_EQ("hodor", routing_params.string_params.back());
    /*
    EXPECT_EQ(0, trie->search("/", routing_params));
    EXPECT_EQ(1, trie->search("/hello", routing_params));
    EXPECT_EQ(2, trie->search("/id/123", routing_params));
    EXPECT_EQ(123, routing_params.int_params.back());
    EXPECT_EQ(2, trie->search("/id/+123", routing_params));
    EXPECT_EQ(123, routing_params.int_params.back());
    EXPECT_EQ(2, trie->search("/id/-123", routing_params));
    EXPECT_EQ(-123, routing_params.int_params.back());
    EXPECT_EQ(-1, trie->search("/hello/12a", routing_params));
    EXPECT_EQ(-123, routing_params.int_params.back());
     */
  }

  {
    Trie *trie = new Trie;

    vector<string> keys = {"/", "/hello/", "/id/<int>/"};
    for (int i = 0; i < keys.size(); ++i)
      trie->insert(keys[i], i);

    util::routing_param routing_params;
    EXPECT_EQ(2, trie->search("/id/123/", routing_params));
    // below examples need to be redirected with trailing slash to make TRUE
    EXPECT_EQ(-1, trie->search("/hello", routing_params));
    EXPECT_EQ(-1, trie->search("/id/123", routing_params));
    EXPECT_EQ(-1, trie->search("/id/+123", routing_params));
    EXPECT_EQ(-1, trie->search("/id/-123", routing_params));
  }
}

TEST(Routing, Tagging) {
  EXPECT_EQ(util::get_parameter_tag("<int>"), 1);
  EXPECT_EQ(util::get_parameter_tag("<float>"), 2);
  EXPECT_EQ(util::get_parameter_tag("<string>"), 3);
  EXPECT_EQ(util::get_parameter_tag("<int><int>"), 7);
  EXPECT_EQ(util::get_parameter_tag("<float><float>"), 14);
  EXPECT_EQ(util::get_parameter_tag("<string><string>"), 21);
}