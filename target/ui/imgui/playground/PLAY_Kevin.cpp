#include "main.h"
#include "string.h"
#include "address.h"
#include "number.h"
#include "circle.h"
#include "catch2/catch_amalgamated.hpp"





TEST_CASE("[Kevin] first", "[Kevin]") {
   std::cout << "goodbye" << std::endl;
   string string1;
   string string2("hello");

   address address1("Kevin", "Gustafsson", "Ulveskogsgatan", "Kung�lv");

   number number1(5);

   number1.pushback(2);

   number1.vector_size();

   circle circle1(5);

   circle1.area();
   circle1.volume();


   std::cout << string1.c_str() << std::endl;
   address1.address_details();

   string1.assign("hello");
   string1.append(" per");

   std::cout << string1.c_str() << std::endl;
}

TEST_CASE("hello", "[Kevin]") {
   std::cout << "hello hello hello" << std::endl;

   //string stringPer("1");
   //stringPer += " 2";
   //stringPer += " 3";
   //stringPer += " 4";
   //stringPer += " 5";

   //std::cout << stringPer.c_str() << std::endl;

   //stringPer = "nu var det slut";

   //std::cout << stringPer.c_str() << std::endl;
}


