#include <cstring>
#include "string.h"



string::string(const char* pbszText)
{
   int iLength = std::strlen(pbszText);
   m_pbszBuffer = new char[iLength + 1];
   strcpy(m_pbszBuffer, pbszText);
   m_iLength = iLength;
}

void string::assign(const char* pbszText)
{
   delete[] m_pbszBuffer;
   int iLength = std::strlen(pbszText);
   m_pbszBuffer = new char[iLength + 1];
   strcpy(m_pbszBuffer, pbszText);
   m_iLength = iLength;
}

void string::append(const char* pbszText)
{
   if (m_iLength != 0)
   {
      int iLength = std::strlen(pbszText);
      iLength += m_iLength;
      char* pbszNewBuffer = new char[iLength + 1];
      strcpy(pbszNewBuffer, m_pbszBuffer);
      strcat(pbszNewBuffer, pbszText);
      delete[] m_pbszBuffer;
      m_pbszBuffer = pbszNewBuffer;
      m_iLength = iLength;

   }
   else 
   {
      assign(pbszText);
   }
}