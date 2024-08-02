#pragma once

#include <utility>
#include <string>
#include <string_view>
#include <vector>
#include <memory>

#include "application/root/Application.h"

#include "window/Frame.h"


/**
 * \brief
 *
 *
 *
 \code
 \endcode
 */
class CApplication : public application::root::CApplication
{
   // ## construction -------------------------------------------------------------
public:
   CApplication() {}
   // copy
   CApplication(const CApplication& o) { common_construct(o); }
   CApplication(CApplication&& o) noexcept { common_construct(std::move(o)); }
   // assign
   CApplication& operator=(const CApplication& o) { common_construct(o); return *this; }
   CApplication& operator=(CApplication&& o) noexcept { common_construct(std::move(o)); return *this; }

   ~CApplication() {}
private:
   // common copy
   void common_construct(const CApplication& o) {}
   void common_construct(CApplication&& o) noexcept {}

// ## operator -----------------------------------------------------------------
public:


// ## methods ------------------------------------------------------------------
public:
/** \name GET/SET
*///@{

//@}

/** \name OPERATION
*///@{
/// Method that can be used to harvest main arguments
   std::pair<bool, std::string> Main( int iArgumentCount, char* ppbszArgument[], std::function<bool ( const std::string_view&, const gd::variant_view& )> process_ );
   /// Initialize application to connect, load needed data and other stuff to make it work
   std::pair<bool, std::string> Initialize();
   /// Use this for clean up
   std::pair<bool, std::string> Exit();

//@}

protected:
/** \name INTERNAL
*///@{

//@}

public:
/** \name DEBUG
*///@{

//@}


// ## attributes ----------------------------------------------------------------
public:
   window::CFrame* m_pframeMain = nullptr;


// ## free functions ------------------------------------------------------------
public:
   static std::pair<bool, std::string> Start( CApplication* papplication );
   static int Main();


};

/// Global pointer to application object
extern CApplication* papplication_g;
