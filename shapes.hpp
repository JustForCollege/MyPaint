#pragma once
#include <raylib.h>

struct FreeHandPoint
{
    Vector2 pos;
    Color color;
    int thickness;

    FreeHandPoint(Vector2 pos, Color color, int thickness)
        : pos(pos), color(color), thickness(thickness) {}
};

struct Rect
{
    float x;
    float y;
    float width;
    float height;
    Color color;
    int thickness;
    bool filled;

    Rect(float x, float y, float width, float height, Color color, int thickness, bool filled)
        : x(x), y(y), width(width), height(height), color(color), thickness(thickness), filled(filled) {}
};

struct Circle
{
    Vector2 center;
    float radius;
    Color color;
    int thickness;
    bool filled;

    Circle(Vector2 center, float radius, Color color, int thickness, bool filled)
        : center(center), radius(radius), color(color), thickness(thickness), filled(filled) {}
};

struct Triangle
{
    Vector2 v1;
    Vector2 v2;
    Vector2 v3;
    Color color;
    bool filled;

    Triangle() = default;
    Triangle(Vector2 v1, Vector2 v2, Vector2 v3, Color color, bool filled)
        : v1(v1), v2(v2), v3(v3), color(color), filled(filled) {}
};

struct Ellipse
{
    Vector2 center;
    float radiusH;
    float radiusV;
    Color color;
    int thickness;
    bool filled;

    Ellipse(Vector2 center, float radiusH, float radiusV, Color color, int thickness, bool filled)
        : center(center), radiusH(radiusH), radiusV(radiusV), color(color), thickness(thickness), filled(filled) {}
};

struct Line
{
    Vector2 start;
    Vector2 end;
    Color color;
    int thickness;

    Line(Vector2 start, Vector2 end, Color color, int thickness)
        : start(start), end(end), color(color), thickness(thickness) {}
};
