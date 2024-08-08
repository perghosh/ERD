#include "address.h"
#include "string.h"

address::address(const char* sName, const char* sLastName, const char* sStreet, const char* sCity)
{
   m_sName = sName;
   m_sLastName = sLastName;
   m_sStreet = sStreet;
   m_sCity = sCity;
}