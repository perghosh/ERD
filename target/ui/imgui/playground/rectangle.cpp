#include <string>
#include "rectangle.h"
#include "circle.h"

rectangle::rectangle(int iWidth, int iHeight, int iDepth, std::string stringColor)
{
   m_iWidth = iWidth;
   m_iHeight = iHeight;
   m_iDepth = iDepth;
   m_stringColor = stringColor;
}

rectangle::rectangle(int iWidth, int iHeight, int iDepth)
{
   m_iWidth = iWidth;
   m_iHeight = iHeight;
   m_iDepth = iDepth;
}

rectangle::rectangle(int iWidth, int iHeight)
{
   m_iWidth = iWidth;
   m_iHeight = iHeight;
}

rectangle::rectangle( const circle& circle_ )
{ 
   m_iWidth = circle_.get_diameter(); 
   m_iHeight = circle_.get_diameter(); 
   m_iDepth = circle_.get_depth();
 }