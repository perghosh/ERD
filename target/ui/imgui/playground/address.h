#include <iostream>
#include "string.h"

/**
 * \brief
 *
 *
 *
 \code
 \endcode
 */
class address
{
   // ## construction -------------------------------------------------------------
public:
   address() {}

   address(const char* sName, const char* sLastName, const char* sStreet, const char* sCity);
   // copy
   address(const address& o) { common_construct(o); }
   address(address&& o) noexcept { common_construct(std::move(o)); }
   // assign
   address& operator=(const address& o) { common_construct(o); return *this; }
   address& operator=(address&& o) noexcept { common_construct(std::move(o)); return *this; }

   ~address() {}
private:
   // common copy
   void common_construct(const address& o) {}
   void common_construct(address&& o) noexcept {}

   // ## operator -----------------------------------------------------------------
public:
   void address_details()
   {
      std::cout << m_sName.c_str() << std::endl;
      std::cout << m_sLastName.c_str() << std::endl;
      std::cout << m_sStreet.c_str() << std::endl;
      std::cout << m_sCity.c_str() << std::endl;
   };

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
   string m_sName;
   string m_sLastName;
   string m_sStreet;
   string m_sCity;


   // ## free functions ------------------------------------------------------------
public:



};