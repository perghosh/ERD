#include <iostream>
#include <vector>

/**
 * \brief
 *
 *
 *
 \code
 \endcode
 */
class number
{
   // ## construction -------------------------------------------------------------
public:
   number() {}

   number(int iNumber);

   // copy
   number(const number& o) { common_construct(o); }
   number(number&& o) noexcept { common_construct(std::move(o)); }
   // assign
   number& operator=(const number& o) { common_construct(o); return *this; }
   number& operator=(number&& o) noexcept { common_construct(std::move(o)); return *this; }

   ~number() {}
private:
   // common copy
   void common_construct(const number& o) {}
   void common_construct(number&& o) noexcept {}

   // ## operator -----------------------------------------------------------------
public:
   void vector_size()
   {
      std::cout << m_vNumbers.size() << std::endl;
   };

   void pushback(int iNumber);

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
   std::vector<int> m_vNumbers;

   // ## free functions ------------------------------------------------------------
public:



};