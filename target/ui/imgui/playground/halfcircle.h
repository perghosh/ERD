#pragma once
#include <iostream>
#include "circle.h"
#include "rectangle.h"

/**
* \brief
*
*
*
\code
\endcode
*/
class halfcircle
{
   // ## construction -------------------------------------------------------------
public:
   halfcircle() {}

   halfcircle(int iRadius);

   halfcircle(const circle& circle_);

   halfcircle(const rectangle& rectangle_);

   // copy
   halfcircle(const halfcircle& o) { common_construct(o); }
   halfcircle(halfcircle&& o) noexcept { common_construct(std::move(o)); }
   // assign
   halfcircle& operator=(const halfcircle& o) { common_construct(o); return *this; }
   halfcircle& operator=(halfcircle&& o) noexcept { common_construct(std::move(o)); return *this; }

   ~halfcircle() {}
private:
   // common copy
   void common_construct(const halfcircle& o) {}
   void common_construct(halfcircle&& o) noexcept {}

   // ## operator -----------------------------------------------------------------
public:


   // ## methods ------------------------------------------------------------------

   void area()
   {
      int iArea = 4 * 3.14 * m_iRadius * m_iRadius;

      std::cout << iArea/2 << std::endl;
   }

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

   int m_iRadius;
   int m_iDepth;

   // ## free functions ------------------------------------------------------------
public:



};