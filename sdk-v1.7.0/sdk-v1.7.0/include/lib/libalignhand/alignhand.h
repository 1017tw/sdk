#ifndef _ALIGNHAND_H_
#define _ALIGNHAND_H_

#include "net.h"

namespace aiva
{

typedef struct _Point_t
{
    float x;
    float y;
} Point;

struct HandPoint
{
    Point landmark[4];
};

class Alignhand
{
public:
    Alignhand() {};
    ~Alignhand() {};

    xnn::Mat align(const xnn::Mat &src, const HandPoint &objs, int width, int height);

    xnn::Mat align_hwc_bgr(const xnn::Mat &src, const HandPoint &objs, int width, int height);
    xnn::Mat align_hwc_rgba(const xnn::Mat &src, const HandPoint &objs, int width, int height);
    xnn::Mat align_gray(const xnn::Mat &src, const HandPoint &objs, int width, int height);
};


} //namespace aiva
#endif
