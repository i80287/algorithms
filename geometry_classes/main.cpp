#include <cassert>
#include <iostream>
#include <vector>

#include "polygon.h"
#include "vector.h"
#include "point.h"
#include "line.h"
#include "ray.h"
#include "circle.h"
#include "segment.h"

using geometry::Point, geometry::Polygon, geometry::Vector, geometry::Segment;

[[maybe_unused]] static void TestPolygonContains() {
    std::vector<Point> points = {
        { 2580, 2430 },
        { -1100, 2440 },
        { -2290, 10 },
        { 1230, -100 },
    };
    Polygon poly(points);
    for (int64_t p_x = -4096; p_x <= 4096; p_x++) {
        for (int64_t p_y = -4096; p_y <= 4096; p_y++) {
            Point pnt = { p_x, p_y };
            bool in0 = poly.ContainsPoint(pnt);
            bool in1 = poly.SimplePolygonContainsPoint(pnt);
            bool in2 = poly.SimpleConvexPolygonContainsPoint(pnt);
            assert(in0 == in1 && in1 == in2);
        }
    }
}

int main() {
    Point p0(0, 0);
    Point p1(4, 0);
    Point p2(4, 4);
    Point p3(0, 4);
    Point p4(1, 3);

    Vector vec01 = p1 - p0;
    Vector vec12 = p2 - p1;
    Vector vec24 = p4 - p2;
    Vector vec40 = p0 - p4;

    assert(geometry::VectorMult(vec01, vec12) == 16);
    assert(geometry::VectorMult(vec12, vec24) == 12);
    assert(geometry::VectorMult(vec01, vec40) == -12);
    assert(geometry::VectorMult(vec24, vec40) == 8);

    geometry::Line l(p0, p2);
    std::cout << "Line " << l << " has params " << l.GetA() << ' ' << l.GetB() << ' ' << l.GetC() << '\n';

    geometry::IShape* clone_ptr = p0.Clone();
    assert(clone_ptr != nullptr);
    clone_ptr->Move(vec01)
              .Move(-vec01)
              .Move(vec01)
              .Move(-vec01);
    assert(clone_ptr->ContainsPoint(p0));
    delete clone_ptr;

    TestPolygonContains();
}
