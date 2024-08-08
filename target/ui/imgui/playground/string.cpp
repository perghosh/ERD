#include <cassert>
#include <cstring>
#include "string.h"



string::string(const char* pbszText)
{
   size_t uLength = std::strlen(pbszText);
   m_pbszBuffer = new char[uLength + 1];
   strcpy(m_pbszBuffer, pbszText);
   m_uLength = (int)uLength;
}

string::string(const char* pbszText, unsigned int uLength)
{
   m_pbszBuffer = new char[uLength + 1];
   strcpy(m_pbszBuffer, pbszText);
   m_uLength = (int)uLength;
}

void string::assign(const char* pbszText)
{
   delete[] m_pbszBuffer;
   size_t uLength = std::strlen(pbszText);
   m_pbszBuffer = new char[uLength + 1];
   strcpy(m_pbszBuffer, pbszText);
   m_uLength = (int)uLength;
}

void string::append(const char* pbszText)
{                                                                                                  assert( pbszText != nullptr );
   if (m_uLength != 0)
   {
      size_t uLength = std::strlen(pbszText);
      uLength += m_uLength;
      char* pbszNewBuffer = new char[uLength + 1];
      strcpy(pbszNewBuffer, m_pbszBuffer);
      strcat(pbszNewBuffer, pbszText);
      delete[] m_pbszBuffer;
      m_pbszBuffer = pbszNewBuffer;
      m_uLength = (int)uLength;

   }
   else 
   {
      assign(pbszText);
   }
}