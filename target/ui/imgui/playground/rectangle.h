#pragma once

#include <string>
#include <iostream>

#include "circle.h"

/**
* \brief
*
*
*
\code
\endcode
*/
class rectangle
{
   // ## construction -------------------------------------------------------------
public:
   rectangle() {}

   rectangle(int iWidth, int iHeight);

   rectangle(int iWidth, int iHeight, int iDepth);

   rectangle(int iWidth, int iHeight, int iDepth, std::string stringColor);

   rectangle( const circle& circle_ );

   rectangle(const rectangle& o)
   {
      m_iWidth = o.m_iWidth;
      m_iHeight = o.m_iHeight;
      m_iDepth = o.m_iDepth;
      m_stringColor = o.m_stringColor;
   }

   // copy
   //rectangle(const rectangle& o) { common_construct(o); }
   //rectangle(rectangle&& o) noexcept { common_construct(std::move(o)); }
   // assign
   rectangle& operator=(const rectangle& o) { common_construct(o); return *this; }
   rectangle& operator=(rectangle&& o) noexcept { common_construct(std::move(o)); return *this; }

   ~rectangle() {}
private:
   // common copy
   void common_construct(const rectangle& o) {}
   void common_construct(rectangle&& o) noexcept {}

   // ## operator -----------------------------------------------------------------
public:


   // ## methods ------------------------------------------------------------------
public:
   /** \name GET/SET
   *///@{

   //@}

   bool is_square() const { return(m_iHeight == m_iWidth); }

   void area()
   {
      std::cout << m_iHeight * m_iWidth << std::endl;
   }

   void volume()
   {
      std::cout << m_iHeight * m_iWidth * m_iDepth << std::endl;
   }

   int get_height() const
   {
      return(m_iHeight);
   }

   int get_width() const
   {
      return(m_iWidth);
   }

   /** \name OPERATION
   *///@{

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
   int m_iWidth;
   int m_iHeight;
   int m_iDepth;
   std::string m_stringColor;

   // ## free functions ------------------------------------------------------------
public:



};