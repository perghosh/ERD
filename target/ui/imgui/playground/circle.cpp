#include "circle.h"
#include <string>

circle::circle(int iRadius)
{
   m_iRadius = iRadius;
}

circle::circle(int iRadius, int iDepth)
{
   m_iRadius = iRadius;
   m_iDepth = iDepth;
}

circle::circle(int iRadius, int iDepth, std::string stringColor)
{
   m_iRadius = iRadius;
   m_iDepth = iDepth;
   m_stringColor = stringColor;
}

circle::circle( std::string stringColor)
{
   m_stringColor = stringColor;
}
