#include <iostream>

#include "Application.h"

#include "main.h"

/// Global pointer to application object
CApplication* papplication_g = nullptr;

int main(int iArgumentCount, char* ppbszArgument)
{
   // ## Initialize application and configure to get the server running
   std::unique_ptr< CApplication > papplication_ = std::make_unique<CApplication>();
   papplication_g = papplication_.get();

   std::cout << "we have a message";

   papplication_->Main();

   return 0;
}