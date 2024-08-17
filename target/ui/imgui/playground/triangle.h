#pragma once

#include <iostream>
#include "rectangle.h"

/**
* \brief
*
*
*
\code
\endcode
*/
class triangle
{
   // ## construction -------------------------------------------------------------
public:
   triangle() {}
   triangle(int iHeight, int iWidth);
   triangle( const rectangle& rectangle_);

   triangle(const triangle& o)
   {
      m_iHeight = o.m_iHeight;
      m_iWidth = o.m_iWidth;
   }

   // copy
   //triangle(const triangle& o) { common_construct(o); }
   //triangle(triangle&& o) noexcept { common_construct(std::move(o)); }
   // assign
   triangle& operator=(const triangle& o) { common_construct(o); return *this; }
   triangle& operator=(triangle&& o) noexcept { common_construct(std::move(o)); return *this; }

   ~triangle() {}
private:
   // common copy
   void common_construct(const triangle& o) {}
   void common_construct(triangle&& o) noexcept {}

   // ## operator -----------------------------------------------------------------
public:


   // ## methods ------------------------------------------------------------------
public:
   /** \name GET/SET
   *///@{

   //@}

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
   int m_iHeight;
   int m_iWidth;



   // ## free functions ------------------------------------------------------------
public:



};