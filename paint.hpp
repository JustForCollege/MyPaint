#pragma once
#include <raylib.h>
#include <vector>
#include "shapes.hpp"

constexpr int WindowWidth = 950;
constexpr int WindowHeight = 600;

constexpr int FPS = 60;
constexpr int toolbarPadding = 70;

constexpr Color BackgroundColor = {34, 34, 27, 255};

static int g_zIndex = 0;

enum class Shape
{
    FreeHand = 0,
    Rectangle,
    Circle,
    Line,
    Ellipse,
    Triangle,
    Erase,
};

struct ShapeObject
{
    Shape shapeKind;
    void* shape;
};

struct Paint
{
public:
    Paint();
    ~Paint();
    void HandleDrawFreeHand(Vector2 currentPos);
    void HandleDrawCircle(Vector2 currentPos);
    void HandleDrawRectangle(Vector2 currentPos);
    void HandleDrawTriangle(Vector2 currentPos);
    void HandleDrawEllipse(Vector2 currentPos);
    void HandleDrawLine(Vector2 currentPos);
    void RenderColorPicker();
    void RenderAll();
    void RenderUI();
    void Run();
private:
    std::vector<ShapeObject*> shapes;
    bool newDrawing;
    Shape currentShape;
    float brushSize;
    Color currentColor;
    Vector2 triangleTop;
    Triangle lastTriangle;
    Vector2 boundingBoxStart;
    Vector2 lineStart;
    Vector2 lineEnd;
    bool drawing;
    bool erasing;
    Rectangle lastBoundingBox;
    int thickness;
    Vector2 lastFreeHandPoint;
};
