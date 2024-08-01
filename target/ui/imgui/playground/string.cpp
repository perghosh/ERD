#include <cassert>
#include <cstring>
#include "string.h"



string::string(const char* pbszText)
{
   size_t uLength = std::strlen(pbszText);
   m_pbszBuffer = new char[uLength + 1];
   strcpy(m_pbszBuffer, pbszText);
   m_iLength = (int)uLength;
}

void string::assign(const char* pbszText)
{
   delete[] m_pbszBuffer;
   size_t uLength = std::strlen(pbszText);
   m_pbszBuffer = new char[uLength + 1];
   strcpy(m_pbszBuffer, pbszText);
   m_iLength = (int)uLength;
}

void string::append(const char* pbszText)
{                                                                                                  assert( pbszText != nullptr );
   if (m_iLength != 0)
   {
      size_t uLength = std::strlen(pbszText);
      uLength += m_iLength;
      char* pbszNewBuffer = new char[uLength + 1];
      strcpy(pbszNewBuffer, m_pbszBuffer);
      strcat(pbszNewBuffer, pbszText);
      delete[] m_pbszBuffer;
      m_pbszBuffer = pbszNewBuffer;
      m_iLength = (int)uLength;

   }
   else 
   {
      assign(pbszText);
   }
}