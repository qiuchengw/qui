#ifndef behavior_circle_h__
#define behavior_circle_h__

#include <windows.h>

#pragma message("-----1")

namespace htmlayout{};
using namespace htmlayout;
//#include "f:\Code\VC\uibase-g\src\qui\ui\htmlayout\htmlayout_aux.h"
using namespace htmlayout;
#pragma message("-----2")

#include "f:\Code\VC\uibase-g\src\qui\ui\htmlayout\htmlayout_canvas.hpp"

#include <math.h>
#include <time.h>

#include "f:\Code\VC\uibase-g\src\qui\ui\htmlayout\mm_file.h"


namespace htmlayout
{

    const double PI = 3.14159265358979323846;

    struct graphin_circle : public canvas
    {
        typedef canvas super;

        htmlayout::image* pimage;

        // ctor
        graphin_circle() :canvas(HANDLE_TIMER, 
            DRAW_CONTENT /*| DRAW_BACKGROUND | DRAW_FOREGROUND*/), pimage(0)
        {
        }

        ~graphin_circle()
        {
            delete pimage;
        }

        virtual void attached(HELEMENT he)
        {
            super::attached(he);
            dom::element el = he;

        }

        virtual void detached(HELEMENT he)
        {
            super::detached(he);
        }

        // canvas::draw overridable
        virtual void draw(HELEMENT he, graphics& gx, UINT width, UINT height)
        {
            gx.open_path();
             gx.circle(80, 80, 80);
//             gx.fill_color(color(255, 0, 0));
//             gx.circle(80, 80, 50);
//             gx.arc(80, 80, 50, 50, 0.1, 1);
            gx.line_width(30);
            gx.line_cap(LINE_CAP_BUTT);
            gx.arc(80, 80, 50, 50, 0.1, 2);
            gx.draw_path(STROKE_ONLY);
            gx.close_path();

            gx.ellipse(80, 80, 50, 50);

//             draw_clock_hand(he, gx, width, height, (st.wHour * 360.0) / 12.0 + st.wMinute / 2.0, 0);
//             draw_clock_hand(he, gx, width, height, (st.wMinute * 360.0) / 60.0, 1);
//             draw_clock_hand(he, gx, width, height, (st.wSecond * 360.0) / 60.0, 2);
        }

        void draw_clock_hand(HELEMENT he, graphics& gx, UINT sx, UINT sy, double angle_degree, int hand)
        {
            dom::element self(he);
            color c(255, 0, 0, 0);
            int   hand_width_px;

            double radians = (2.0 * PI * (angle_degree - 90.0)) / 360.0;
            int radius = min(sy, sx) / 2 - 16;

            gx.line_cap(LINE_CAP_ROUND);

            switch (hand)
            {
            case 0: // hours
                radius -= 24;
                c = self.attribute("-hand-hours", c);
                if (c.transparent()) return;
                hand_width_px = self.attribute("-hand-hours-width", 5);
                gx.line_color(c);
                gx.line_width(hand_width_px);
                break;
            case 1: // minutes
                radius -= 12;
                c = self.attribute("-hand-minutes", c);
                if (c.transparent()) return;
                hand_width_px = self.attribute("-hand-minutes-width", 3);
                gx.line_color(c);
                gx.line_width(hand_width_px);
                break;
            case 2: // seconds
                c = self.attribute("-hand-seconds", c);
                if (c.transparent()) return;
                hand_width_px = self.attribute("-hand-seconds-width", 1);
                gx.line_color(c);
                gx.line_width(hand_width_px);
                break;
            default:
                assert(false);
            }

            double y = (sy / 2) + 0.5; // + 0.5 is to move it to the center of the pixel.
            double x = (sy / 2) + 0.5;

            double xe = x + int(cos(radians) * radius) + 0.5;
            double ye = y + int(sin(radians) * radius) + 0.5;

            gx.line(x, y, xe, ye);
            if (hand == 2)
            {
                // circle on the end of seconds hand
                if (pimage)
                {
                    int w = pimage->width();
                    int h = pimage->height();
                    gx.draw_image(pimage, xe - double(w) / 2, ye - double(h) / 2, w, h, 0, 0, w, h);
                }
                else // no image
                    gx.circle(xe, ye, 4);
            }
        }

        void draw_caption(HELEMENT he, graphics& gx, UINT width, UINT height, aux::wchars text)
        {
            gx.state_save();
            //gx.rotate(3.1415926 / 2, width / 2, height / 3); // for text rotation testing.
            gx.text_alignment(TEXT_ALIGN_CENTER, TEXT_ALIGN_CENTER);
            gx.font("Times New Roman", 18);
            gx.no_line();
            gx.fill_linear_gradient(width / 3, 0, (2 * width) / 3, 0, color(0xDF, 0, 0), color(0, 0, 0x7F));
            gx.text(width / 2, height / 3, text);
            //gx.rectangle(0,0,width / 2, height / 3);
            gx.state_restore();
        }


    };


    // instantiating and attaching it to the global list
    canvas_factory<graphin_circle> graphin_clock_factory("graphin-circle");

} // htmlayout namespace

#endif // behavior_circle_h__
