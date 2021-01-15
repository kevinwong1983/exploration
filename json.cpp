#include "boost/variant.hpp"
#include <iostream>
#include <gtest/gtest.h>
#include <boost/asio.hpp>
#include <time.h>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/system/system_error.hpp>
#include <functional>
#include <unordered_map>

template<typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args)
{
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

#include "rapidjson/document.h"     // rapidjson's DOM-style API
#include "rapidjson/prettywriter.h" // for stringify JSON
#include <cstdio>

using namespace rapidjson;
using namespace std;

TEST(rapidjson, json) {
    const char json[] = " { \"hello\" : \"world\", \"t\" : true , \"f\" : false, \"n\": null, \"i\":123, \"pi\": 3.1416, \"a\":[1, 2, 3, 4] } ";

    Document document;
    EXPECT_FALSE(document.Parse(json).HasParseError());

    EXPECT_TRUE(document["hello"].IsString());
    EXPECT_EQ("world", document["hello"].GetString());
    document["hello"] = "kitty";
    EXPECT_EQ("kitty", document["hello"].GetString());

    EXPECT_TRUE(document["t"].IsBool());
    EXPECT_EQ(true, document["t"].GetBool());
    document["t"] = false;
    EXPECT_EQ(false, document["t"].GetBool());

    EXPECT_TRUE(document["f"].IsBool());

    EXPECT_TRUE(document["n"].IsNull());

    EXPECT_TRUE(document["i"].IsNumber());
    EXPECT_TRUE(document["i"].IsInt());

    EXPECT_TRUE(document["pi"].IsNumber());
    EXPECT_TRUE(document["pi"].IsDouble());

    EXPECT_TRUE(document["a"].IsArray());
    EXPECT_EQ(4, document["a"].Size());
    Value& a = document["a"];
    for (Value::ConstValueIterator itr = a.Begin(); itr != a.End(); ++itr) {
        std::cout << "a = " << itr->GetInt() << std::endl << std::endl;
    }
    a.PushBack(456, document.GetAllocator());
    EXPECT_EQ(5, document["a"].Size());
}

//struct config {
//    int x;
//    int y;
//    std::string toJsonString() {
//
//    }
//    bool fromJsonSting(std::string jsonString) {
//
//    }
//};

TEST(rapidjson, json2) {
    Document d;
    d.SetObject();

    auto kitty = std::string("HELLLOOOO KITTY");
    rapidjson::Value _kitty(kitty.c_str(), kitty.size(),d.GetAllocator());
    d.AddMember("hello", _kitty, d.GetAllocator());
    EXPECT_EQ("HELLLOOOO KITTY", std::string(d["hello"].GetString()));
    auto n = 5;
    d.AddMember("number", rapidjson::Value(n), d.GetAllocator());
    EXPECT_EQ(5, d["number"].GetInt());

    Document e;
    e.SetObject();
    e.AddMember("number1", rapidjson::Value(6), e.GetAllocator());
    e.AddMember("number2", rapidjson::Value(7), e.GetAllocator());

    d.AddMember("Inner", e, d.GetAllocator());

    StringBuffer sb;
    PrettyWriter<StringBuffer> writer(sb);
    d.Accept(writer);    // Accept() traverses the DOM and generates Handler events.
    std::cout << sb.GetString() << std::endl;
}

#include <tao/json.hpp>
using namespace tao::json;
TEST(toa, json) {
    // create json

    // 1.
    const char json[] = " { \"hello\" : \"world\", \"t\" : true , \"f\" : false, \"n\": null, \"i\":123, \"pi\": 3.1416, \"a\":[1, 2, 3, 4] } ";
    const tao::json::value v1 = tao::json::from_string(json);
    std::cout << ">>>>>json:" <<  tao::json::to_string( v1 ) << std::endl;
    auto array = v1.at("a");
    std::cout << ">>>>>json:" <<  tao::json::to_string(array) << std::endl;
    auto array_element = array.at(0); // better that array[0], since it does not thow an error.
    std::cout << ">>>>>json:" <<  tao::json::to_string(array_element) << std::endl;


    // 2.
    const tao::json::value v2 = " { \"hello\" : \"world\", \"t\" : true , \"f\" : false, \"n\": null, \"i\":123, \"pi\": 3.1416, \"a\":[1, 2, 3, 4] } "_json;
    std::cout << "json:" <<  tao::json::to_string( v1 ) << std::endl;

    // 3.
    const tao::json::value v3 = {
            {"hello", "world"},
            {"t", true},
            {"f", false},
            {"n", tao::json::null},
            {"i", 123},
            {"pi", 3.1426},
            {"a", tao::json::value::array( { 1, 2, 3, 4 } )}
    };
    std::cout << "json:" <<  tao::json::to_string( v3 ) << std::endl;

    // 4.
    tao::json::value v4;
    v4.emplace("hello", "world");
    v4.emplace("t", true);
    v4.emplace("f", false);
    v4.emplace("n", tao::json::null);
    v4.emplace("i", 123);
    v4.emplace("pi", 3.1426);
    v4.emplace("a", tao::json::value::array({1,2,3,4}));
    std::cout << "json:" <<  tao::json::to_string( v4 ) << std::endl;

    // setters arrays
    tao::json::value a = tao::json::empty_array;
    a.emplace_back(1);
    a.emplace_back(2);
    a.emplace_back(3);
    a.emplace_back(4);

    tao::json::value v5 = tao::json::empty_object;
    v5["hello"] = "world";
    v5["t"] = true;
    v5["f"] = false;
    v5["n"] = tao::json::null;
    v5["i"] = 123;
    v5["pi"] = 3.1426;
    v5["a"] = a;
    v5["b"]["x"] = 3.1426;          // cool
    v5["b"]["y"] = "test";
    std::cout << "json:" <<  tao::json::to_string( v5 ) << std::endl;

    // getters
    EXPECT_TRUE(v5["hello"].is_string());
    EXPECT_EQ("world", v5["hello"]);

    EXPECT_TRUE(v5["t"].is_boolean());
    EXPECT_EQ(true, v5["t"]);
    EXPECT_TRUE(v5["f"].is_boolean());
    EXPECT_EQ(false, v5["f"]);

    EXPECT_TRUE(v5["n"].is_null());
    EXPECT_EQ( tao::json::null, v5["n"]);

    EXPECT_TRUE(v5["i"].is_integer());
    EXPECT_EQ(123, v5["i"]);
    EXPECT_TRUE(v5["pi"].is_double());
    EXPECT_EQ(3.1426, v5["pi"]);

    EXPECT_EQ(true,  v5["a"].is_array());
    EXPECT_EQ(1,  v5["a"][0].get_signed());
    EXPECT_EQ(2,  v5["a"][1]);
    EXPECT_EQ(3,  v5["a"][2]);
    EXPECT_EQ(4,  v5["a"][3]);

    // use array in for loop
    if( v5["a"].is_array() ) {
        for( const auto& i : v5["a"].get_array() ) {
            std::cout << "element on line " << i << std::endl;
        }
    }

    EXPECT_TRUE(v5["b"]["x"].is_double());
    EXPECT_EQ(3.1426, v5["b"]["x"]);
    EXPECT_TRUE(v5["b"]["y"].is_string());
    EXPECT_EQ("test", v5["b"]["y"]);

    // modify values
    //  same type
    v5["pi"] = 3.123;
    EXPECT_TRUE(v5["pi"].is_double());
    EXPECT_EQ(3.123, v5["pi"]);
    //  different type
    v5["pi"] = "3.123";
    EXPECT_TRUE(v5["pi"].is_string());
    EXPECT_EQ("3.123", v5["pi"]);

    //
    tao::json::value t = v5["i"];
    EXPECT_EQ(128, t.get_signed() + 5);

    // non existing keys
    tao::json::value t2 = v5["asdf"];
    EXPECT_TRUE(t2.empty());
    EXPECT_FALSE(t2.is_signed());
    EXPECT_THROW({
        int val = t2.get_signed();
    },  exception);
}

TEST(toa, json2) {
    tao::json::value any_at_home = tao::json::empty_object;
    any_at_home["any_at_home"] = true;
    tao::json::value geofence = tao::json::empty_object;
    geofence["geofence"] = any_at_home;

    std::cout << tao::json::to_string(geofence) << std::endl;
}
