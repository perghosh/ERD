#include "main.h"
#include "string.h"
#include "catch2/catch_amalgamated.hpp"




TEST_CASE("[Kevin] first", "[Kevin]") {
   std::cout << "goodbye" << std::endl;
   string string1;
   string string2("hello");
   std::cout << string1.c_str() << std::endl;

   string1.assign("hello");
   string1.append(" per");

   std::cout << string1.c_str() << std::endl;
}

TEST_CASE("hello", "[Kevin]") {
   std::cout << "hello hello hello" << std::endl;

   string stringPer("1");
   stringPer += " 2";
   stringPer += " 3";
   stringPer += " 4";
   stringPer += " 5";

   std::cout << stringPer.c_str() << std::endl;
}


