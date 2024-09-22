#include "triangle.h"
#include "rectangle.h"

triangle::triangle(int iHeight, int iWidth)
{
   m_iHeight = iHeight;
   m_iWidth = iWidth;
}

triangle::triangle(const rectangle& rectangle_)
{
   m_iWidth = rectangle_.get_width();
   m_iHeight = rectangle_.get_height();
}