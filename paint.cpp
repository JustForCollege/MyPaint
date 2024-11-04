#include "paint.hpp"
#include "raylib.h"
#include "raymath.h"
#include "shapes.hpp"
#include <cstdint>
#include <rlImGui.h>
#include <imgui.h>

using u8 = uint8_t;

Paint::Paint()
    : currentShape(Shape::FreeHand),
      brushSize(2.0f),
      currentColor(BLACK),
      drawing(true),
      thickness(5),
      erasing(false)
{
    InitWindow(WindowWidth, WindowHeight, "MyPaint");
    rlImGuiSetup(true);
    SetTargetFPS(FPS);
}

Paint::~Paint() {}

static std::vector<Vector2> vLerp(Vector2 start, Vector2 end, float spacing)
{
    std::vector<Vector2> points;
    float distance = Vector2Distance(start, end);

    if(distance < spacing)
    {
        points.push_back(end);
        return points;
    }

    int pointsBetween = static_cast<int>(distance / spacing);

    for(int i = 0; i < pointsBetween; i++)
    {
        float t = i / static_cast<float>(pointsBetween);
        Vector2 p
        {
            start.x + (end.x - start.x)*t,
            start.y + (end.y - start.y)*t,
        };

        points.push_back(p);
    }

    return points;
}

void Paint::RenderColorPicker()
{
    static bool alpha_preview = true;
    static bool alpha_half_preview = false;
    static bool drag_and_drop = true;
    static bool options_menu = true;
    static bool hdr = false;

    ImGuiColorEditFlags misc_flags = (hdr ? ImGuiColorEditFlags_HDR : 0) | (drag_and_drop ? 0 : ImGuiColorEditFlags_NoDragDrop) | (alpha_half_preview ? ImGuiColorEditFlags_AlphaPreviewHalf : (alpha_preview ? ImGuiColorEditFlags_AlphaPreview : 0)) | (options_menu ? 0 : ImGuiColorEditFlags_NoOptions);

    static bool saved_palette_init = true;
    static ImVec4 saved_palette[32] = {};
    if (saved_palette_init)
    {
        for (int n = 0; n < IM_ARRAYSIZE(saved_palette); n++)
        {
            ImGui::ColorConvertHSVtoRGB(n / 31.0f, 0.8f, 0.8f,
                saved_palette[n].x, saved_palette[n].y, saved_palette[n].z);
            saved_palette[n].w = 1.0f;
        }
        saved_palette_init = false;
    }

    static ImVec4 color = ImVec4(114.0f / 255.0f, 144.0f / 255.0f, 154.0f / 255.0f, 200.0f / 255.0f);
    static ImVec4 backup_color;
    bool open_popup = ImGui::ColorButton("MyColor##3b", color, misc_flags);
    ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x);
    open_popup |= ImGui::Button("Palette");
    if (open_popup)
    {
        ImGui::OpenPopup("mypicker");
        backup_color = color;
    }
    if (ImGui::BeginPopup("mypicker"))
    {
        ImGui::ColorPicker4("##picker", (float*)&color, misc_flags | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoSmallPreview);
        ImGui::SameLine();

        ImGui::BeginGroup();
        ImGui::Text("Current");
        ImGui::ColorButton("##current", color, ImGuiColorEditFlags_NoPicker | ImGuiColorEditFlags_AlphaPreviewHalf, ImVec2(60, 40));
        ImGui::Text("Previous");
        if (ImGui::ColorButton("##previous", backup_color, ImGuiColorEditFlags_NoPicker | ImGuiColorEditFlags_AlphaPreviewHalf, ImVec2(60, 40)))
            color = backup_color;
        ImGui::Separator();
        ImGui::Text("Palette");
        for (int n = 0; n < IM_ARRAYSIZE(saved_palette); n++)
        {
            ImGui::PushID(n);
            if ((n % 8) != 0)
                ImGui::SameLine(0.0f, ImGui::GetStyle().ItemSpacing.y);

            ImGuiColorEditFlags palette_button_flags = ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoPicker | ImGuiColorEditFlags_NoTooltip;
            if (ImGui::ColorButton("##palette", saved_palette[n], palette_button_flags, ImVec2(20, 20)))
                color = ImVec4(saved_palette[n].x, saved_palette[n].y, saved_palette[n].z, color.w);

            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(IMGUI_PAYLOAD_TYPE_COLOR_3F))
                    memcpy((float*)&saved_palette[n], payload->Data, sizeof(float) * 3);
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(IMGUI_PAYLOAD_TYPE_COLOR_4F))
                    memcpy((float*)&saved_palette[n], payload->Data, sizeof(float) * 4);
                ImGui::EndDragDropTarget();
            }

            ImGui::PopID();
        }
        ImGui::EndGroup();
        ImGui::EndPopup();
    }

    if(!erasing)
    {
        currentColor = {
            (u8)(color.x * 255),
            (u8)(color.y * 255),
            (u8)(color.z * 255),
            (u8)(color.w * 255)
        };
    }
}

void Paint::RenderUI()
{
    DrawLineEx({0, 60}, {WindowWidth, 60}, 10.0f, {66, 65, 54, 255});

    ImGuiWindowFlags window_flags =
        ImGuiWindowFlags_NoDecoration |
        ImGuiWindowFlags_NoBackground |
        ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoSavedSettings;

    const float TOOLBAR_PADDING = 10.0f;
    ImGui::SetNextWindowPos(
        ImVec2(TOOLBAR_PADDING, TOOLBAR_PADDING),
        ImGuiCond_Always
    );

    ImGui::Begin("##Toolbar", nullptr, window_flags);

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.5f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.3f, 0.3f, 0.7f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.5f, 0.5f, 0.5f, 0.9f));

    if (ImGui::Button("Rectangle", ImVec2(70, 30)))
    {
        currentShape = Shape::Rectangle;
        erasing = false;
    }
    ImGui::SameLine();

    if (ImGui::Button("Circle", ImVec2(70, 30)))
    {
        currentShape = Shape::Circle;
        erasing = false;
    }
    ImGui::SameLine();

    if (ImGui::Button("FreeHand", ImVec2(70, 30)))
    {
        currentShape = Shape::FreeHand;
        erasing = false;
    }
    ImGui::SameLine();

    if (ImGui::Button("Triangle", ImVec2(70, 30)))
    {
        currentShape = Shape::Triangle;
        erasing = false;
    }
    ImGui::SameLine();
    if(ImGui::Button("Ellipse", ImVec2(70, 30)))
    {
        currentShape = Shape::Ellipse;
        erasing = false;
    }
    ImGui::SameLine();

    if(ImGui::Button("Line", ImVec2(70, 30)))
    {
        currentShape = Shape::Line;
        erasing = false;
    }
    ImGui::SameLine();
    if(ImGui::Button("Erase", ImVec2(70, 30)))
    {
        currentShape = Shape::FreeHand;
        currentColor = BackgroundColor;
        erasing = true;
    }
    ImGui::SameLine();
    ImGui::Checkbox("Filled", &filled);
    ImGui::SameLine();

    RenderColorPicker();
    ImGui::SameLine();
    ImGui::SliderInt("Thickness", &thickness, 0, 100, "%d", ImGuiSliderFlags_None);
    ImGui::PopStyleColor(3);
    ImGui::End();
}

void Paint::HandleDrawFreeHand(Vector2 currentPos)
{
    if(currentPos.y <= toolbarPadding) return;

    float spacing = brushSize/2;
    if(newDrawing)
    {
        auto shape = new ShapeObject();
        shape->shapeKind = Shape::FreeHand;
        shape->shape = new FreeHandPoint(currentPos, currentColor, thickness);

        shapes.push_back(shape);

        newDrawing = false;
    }
    else
    {
        if(Vector2Distance(currentPos, lastFreeHandPoint) > spacing)
        {
            auto lerped = vLerp(lastFreeHandPoint, currentPos, spacing);

            for(auto lerpedp: lerped)
            {
                auto shape = new ShapeObject();
                shape->shapeKind = Shape::FreeHand;
                shape->shape = new FreeHandPoint(lerpedp, currentColor, thickness);

                shapes.push_back(shape);
            }
        }
    }

    lastFreeHandPoint = currentPos;
}

void Paint::HandleDrawCircle(Vector2 currentPos)
{
    if(currentPos.y <= toolbarPadding) return;

    if(newDrawing)
    {
        boundingBoxStart = currentPos;
        newDrawing = false;
    }
    else
    {
        Vector2 bottomLeft = { boundingBoxStart.x, currentPos.y };

        auto h = Vector2Distance(boundingBoxStart, bottomLeft);
        auto w = Vector2Distance(bottomLeft, currentPos);

        lastBoundingBox = {boundingBoxStart.x, boundingBoxStart.y, w, h};

        Vector2 center
        {
            lastBoundingBox.x + lastBoundingBox.width/2,
            lastBoundingBox.y + lastBoundingBox.height/2,
        };

        float radius = Vector2Distance(center, {
            lastBoundingBox.x + lastBoundingBox.width/2,
            lastBoundingBox.y + lastBoundingBox.height,
        });

        if(filled)
            DrawCircleV(center, radius, currentColor);
        else
            DrawRing(center, radius, radius + thickness, 0, 360, 0, currentColor);
    }
}

void Paint::HandleDrawRectangle(Vector2 currentPos)
{
    if(currentPos.y <= toolbarPadding) return;

    if(newDrawing)
    {
        boundingBoxStart = currentPos;
        newDrawing = false;
    }
    else
    {
        Vector2 bottomLeft { boundingBoxStart.x, currentPos.y };
        auto height = Vector2Distance(boundingBoxStart, bottomLeft);
        auto width = Vector2Distance(bottomLeft, currentPos);
        lastBoundingBox = { boundingBoxStart.x, boundingBoxStart.y, width, height };
        if(filled)
            DrawRectangleV({lastBoundingBox.x, lastBoundingBox.y}, {lastBoundingBox.width, lastBoundingBox.height}, currentColor);
        else
            DrawRectangleLinesEx({lastBoundingBox.x, lastBoundingBox.y, lastBoundingBox.width, lastBoundingBox.height}, thickness, currentColor);
    }
}

void Paint::HandleDrawTriangle(Vector2 currentPos)
{
    if(currentPos.y <= toolbarPadding) return;

    if(newDrawing)
    {
        triangleTop = currentPos;
        newDrawing = false;
    }
    else
    {
        Vector2 normal { triangleTop.x, currentPos.y  };
        auto rightNormal = Vector2Distance(normal, currentPos);
        Vector2 left { currentPos.x - 2*rightNormal, currentPos.y  };
        if(filled)
            DrawTriangle(triangleTop, left, currentPos, currentColor);
        else
            DrawTriangleLines(triangleTop, left, currentPos, currentColor);
        lastTriangle = { triangleTop, left, currentPos, currentColor, filled };
    }
}

void Paint::HandleDrawEllipse(Vector2 currentPos)
{
    if(currentPos.y <= toolbarPadding) return;

    if(newDrawing)
    {
        boundingBoxStart = currentPos;
        newDrawing = false;
    }
    else
    {
        Vector2 bottomLeft = { boundingBoxStart.x, currentPos.y };
        auto h = Vector2Distance(boundingBoxStart, bottomLeft);
        auto w = Vector2Distance(bottomLeft, currentPos);

        lastBoundingBox = {boundingBoxStart.x, boundingBoxStart.y, w, h};

        Vector2 center
        {
            lastBoundingBox.x + lastBoundingBox.width/2,
            lastBoundingBox.y + lastBoundingBox.height/2,
        };

        float radiusV = Vector2Distance(center, {
            lastBoundingBox.x + lastBoundingBox.width/2,
            lastBoundingBox.y + lastBoundingBox.height,
        });

        float radiusH = Vector2Distance(
            {lastBoundingBox.x, lastBoundingBox.y + lastBoundingBox.width/2},
            center
        );

        if(filled)
            DrawEllipse(center.x, center.y, radiusH, radiusV, currentColor);
        else
            DrawEllipseLines(center.x, center.y, radiusH, radiusV, currentColor);
    }
}

void Paint::HandleDrawLine(Vector2 currentPos)
{
    if(currentPos.y <= toolbarPadding) return;

    if(newDrawing)
    {
        lineStart = currentPos;
        newDrawing = false;
    }
    else
    {
        lineEnd = currentPos;
        DrawLineEx(lineStart, lineEnd, thickness, currentColor);
    }
}

void Paint::RenderAll()
{
    for(const auto& shape: shapes)
    {
        switch(shape->shapeKind)
        {
            case Shape::Rectangle:
            {
                Rect* rect = (Rect*)shape->shape;
                if(rect->filled)
                    DrawRectangleV({rect->x, rect->y}, {rect->width, rect->height}, rect->color);
                else
                    DrawRectangleLinesEx({rect->x, rect->y, rect->width, rect->height}, rect->thickness, rect->color);
            } break;

            case Shape::Circle:
            {
                Circle* circle = (Circle*)shape->shape;
                if(circle->filled)
                    DrawCircleV(circle->center, circle->radius, circle->color);
                else
                    DrawRing(circle->center, circle->radius, circle->radius + circle->thickness, 0, 360, 0, circle->color);
            } break;

            case Shape::Line:
            {
                Line* line = (Line*)shape->shape;
                DrawLineEx(line->start, line->end, line->thickness, line->color);
            } break;

            case Shape::Ellipse:
            {
                Ellipse* ellipse = (Ellipse*)shape->shape;
                if(ellipse->filled)
                    DrawEllipse(ellipse->center.x, ellipse->center.y, ellipse->radiusH, ellipse->radiusV , ellipse->color);
                else
                    DrawEllipseLines(ellipse->center.x, ellipse->center.y, ellipse->radiusH, ellipse->radiusV , ellipse->color);
            } break;

            case Shape::Triangle:
            {
                Triangle* triangle = (Triangle*)shape->shape;
                if(triangle->filled)
                    DrawTriangle(triangle->v1, triangle->v2, triangle->v3, triangle->color);
                else
                    DrawTriangleLines(triangle->v1, triangle->v2, triangle->v3, triangle->color);
            } break;

            case Shape::FreeHand:
            {
                FreeHandPoint* fhp = (FreeHandPoint*)shape->shape;
                DrawCircleV(fhp->pos, fhp->thickness, fhp->color);
            } break;

            default: {}
        }
    }
}

void Paint::Run()
{
    while(!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(BackgroundColor);

        rlImGuiBegin();

        RenderAll();
        RenderUI();

        if(IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL))
        {
            if(IsKeyPressed(KEY_Z))
            {
                if(!shapes.empty())
                {
                    undoedShapes.push_back(shapes.back());
                    shapes.pop_back();
                }
            }
            else if(IsKeyPressed(KEY_Y))
            {
                if(!undoedShapes.empty())
                {
                    shapes.push_back(undoedShapes.back());
                    undoedShapes.pop_back();
                }
            }
        }

        if(IsMouseButtonDown(MOUSE_BUTTON_LEFT))
        {
            Vector2 currentPos = GetMousePosition();

            switch(currentShape)
            {
                case Shape::FreeHand:
                    HandleDrawFreeHand(currentPos);
                    break;

                case Shape::Rectangle:
                    HandleDrawRectangle(currentPos);
                    break;

                case Shape::Circle:
                    HandleDrawCircle(currentPos);
                    break;

                case Shape::Triangle:
                    HandleDrawTriangle(currentPos);
                    break;

                case Shape::Line:
                    HandleDrawLine(currentPos);
                    break;

                case Shape::Ellipse:
                    HandleDrawEllipse(currentPos);
                    break;

                default: {}
            }
        }
        else if(IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
        {
            // NASTY TRICK
            Vector2 mousePos = GetMousePosition();
            if(mousePos.y <= toolbarPadding)
                goto endRendering;

            switch(currentShape)
            {
                case Shape::Rectangle:
                {
                    auto shape = new ShapeObject();
                    shape->shapeKind = Shape::Rectangle;
                    shape->shape = new Rect({
                        lastBoundingBox.x,
                        lastBoundingBox.y,
                        lastBoundingBox.width,
                        lastBoundingBox.height,
                        currentColor,
                        thickness,
                        filled,
                    });

                    shapes.push_back(shape);

                } break;

                case Shape::Circle:
                {
                    Vector2 center
                    {
                        lastBoundingBox.x + lastBoundingBox.width/2,
                        lastBoundingBox.y + lastBoundingBox.height/2,
                    };

                    float radius = Vector2Distance(center, {
                        lastBoundingBox.x + lastBoundingBox.width/2,
                        lastBoundingBox.y + lastBoundingBox.height,
                    });

                    auto shape = new ShapeObject();

                    shape->shapeKind = Shape::Circle;
                        shape->shape = new Circle(
                        center,
                        radius,
                        currentColor,
                        thickness,
                        filled
                    );

                    shapes.push_back(shape);
                } break;

                case Shape::Ellipse:
                {
                    Vector2 center
                    {
                        lastBoundingBox.x + lastBoundingBox.width/2,
                        lastBoundingBox.y + lastBoundingBox.height/2,
                    };

                    float radiusV = Vector2Distance(center, {
                        lastBoundingBox.x + lastBoundingBox.width/2,
                        lastBoundingBox.y + lastBoundingBox.height,
                    });

                    float radiusH = Vector2Distance(
                        {lastBoundingBox.x, lastBoundingBox.y + lastBoundingBox.width/2},
                        center
                    );

                    auto shape = new ShapeObject();
                    shape->shapeKind = Shape::Ellipse;
                    shape->shape = new Ellipse(
                       center,
                       radiusH,
                       radiusV,
                       currentColor,
                       thickness,
                       filled
                    );
                    shapes.push_back(shape);

                } break;

                case Shape::Line:
                {
                    auto shape = new ShapeObject();

                    shape->shapeKind = Shape::Line;
                        shape->shape = new Line(
                       lineStart,
                       lineEnd,
                       currentColor,
                       thickness
                    );

                    shapes.push_back(shape);
                } break;

                case Shape::Triangle:
                {
                    auto shape = new ShapeObject();

                    shape->shapeKind = Shape::Triangle;
                        shape->shape = new Triangle(
                       lastTriangle.v1,
                       lastTriangle.v2,
                       lastTriangle.v3,
                       currentColor, filled);

                    shapes.push_back(shape);

                } break;

                default: {}
            }

            newDrawing = true;
        }

endRendering:
        rlImGuiEnd();
        EndDrawing();
    }
}
