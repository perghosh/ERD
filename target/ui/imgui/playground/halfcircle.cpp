#include "halfcircle.h"


halfcircle::halfcircle(int iRadius)
{
   m_iRadius = iRadius;
}

halfcircle::halfcircle(const circle& circle_)
{
   m_iRadius = circle_.get_radius();
   m_iDepth = circle_.get_depth();
}

halfcircle::halfcircle(const rectangle& rectangle_)
{
   m_iRadius = rectangle_.get_width() / 2;
   m_iDepth = rectangle_.get_height();
}