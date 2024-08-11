#pragma once
#include <iostream>
#include <string>

/**
* \brief
*
*
*
\code
\endcode
*/
class circle
{
   // ## construction -------------------------------------------------------------
public:
   circle() {}

   circle(int iRadius);

   circle(int iRadius, int iDepth);

   circle(int iRadius, int iDepth, std::string stringColor);

   circle(std::string stringColor);

   // copy
   circle(const circle& o) { common_construct(o); }
   circle(circle&& o) noexcept { common_construct(std::move(o)); }
   // assign
   circle& operator=(const circle& o) { common_construct(o); return *this; }
   circle& operator=(circle&& o) noexcept { common_construct(std::move(o)); return *this; }

   ~circle() {}
private:
   // common copy
   void common_construct(const circle& o) {}
   void common_construct(circle&& o) noexcept {}

   // ## operator -----------------------------------------------------------------
public:


   // ## methods ------------------------------------------------------------------

   void volume()
   {
      int iVolume = 4 * 3.14 * m_iRadius * m_iRadius * m_iRadius;

      std::cout << iVolume / 3 << std::endl;
   }

   void area()
   {
      int iArea = 4 * 3.14 * m_iRadius * m_iRadius;

      std::cout << iArea << std::endl;
   }

   int get_diameter() const
   {
      int iReturnValue = m_iRadius * 2;
      return(iReturnValue);
   }

   int get_depth() const
   {
      int iReturnValue = m_iDepth;
      return(iReturnValue);
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
   std::string m_stringColor;


   // ## free functions ------------------------------------------------------------
public:



};