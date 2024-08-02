#pragma once

#include <utility>
#include <string>
#include <string_view>


#include "gd/gd_arguments.h"

#ifndef WINDOW_BEGIN
#  define WINDOW_BEGIN namespace window {
#  define WINDOW_END }
#endif

WINDOW_BEGIN

/**
 * \brief
 *
 *
 *
 \code
 \endcode
 */
class CFrame
{
// ## construction -------------------------------------------------------------
public:
   CFrame() {}
   // copy
   CFrame(const CFrame& o) { common_construct(o); }
   CFrame(CFrame&& o) noexcept { common_construct(std::move(o)); }
   // assign
   CFrame& operator=(const CFrame& o) { common_construct(o); return *this; }
   CFrame& operator=(CFrame&& o) noexcept { common_construct(std::move(o)); return *this; }

   ~CFrame() {}
private:
   // common copy
   void common_construct(const CFrame& o) {}
   void common_construct(CFrame&& o) noexcept {}

// ## operator -----------------------------------------------------------------
public:


// ## methods ------------------------------------------------------------------
public:
/** \name GET/SET
*///@{

//@}

/** \name OPERATION
*///@{
   virtual std::pair<bool, std::string> Initialize() { return {true, ""}; }
   virtual std::pair<bool, std::string> Create() { return {true, ""}; }
   virtual std::pair<bool, std::string> Destroy() { return {true, ""}; }
   virtual std::pair<bool, std::string> Exit() { return {true, ""}; }

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
   gd::argument::arguments m_arguments;   // Named window attributes


// ## free functions ------------------------------------------------------------
public:



};

WINDOW_END