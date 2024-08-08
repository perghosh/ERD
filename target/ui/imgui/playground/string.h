#pragma once

#include <utility>
/**
 * \brief
 *
 *
 *
 \code
 \endcode
 */
class string
{
   // ## construction -------------------------------------------------------------
public:
   string() : m_pbszBuffer(nullptr), m_uLength(0) {}

   string(const char* pbszText);

   string(const char* pbszText, unsigned int uLength);
   // copy
   string(const string& o) { common_construct(o); }
   string(string&& o) noexcept { common_construct(std::move(o)); }
   // assign
   string& operator=(const string& o) { common_construct(o); return *this; }
   string& operator=(string&& o) noexcept { common_construct(std::move(o)); return *this; }

   ~string() {}
private:
   // common copy
   void common_construct(const string& o) {}
   void common_construct(string&& o) noexcept {}

// ## operator -----------------------------------------------------------------
public:
   string& operator=( const char* pbszText ) { assign( pbszText ); return *this; }
   string& operator+=( const char* pbszText ) { append( pbszText ); return *this; }
   

// ## methods ------------------------------------------------------------------
public:
/** \name GET/SET
*///@{

//@}

/** \name OPERATION
*///@{
   const char* c_str() const { return m_pbszBuffer != nullptr ? m_pbszBuffer : ""; }

   void assign(const char* pbszText);
 
   void append(const char* pbszText);

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
  
   char* m_pbszBuffer;
   unsigned int m_uLength;

// ## free functions ------------------------------------------------------------
public:



};