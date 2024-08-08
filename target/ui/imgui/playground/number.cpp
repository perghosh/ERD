#include <vector>
#include "number.h"

number::number(int iNumber)
{
   m_vNumbers.push_back(iNumber);
}

void number::pushback(int iNumber)
{
   m_vNumbers.push_back(iNumber);
}