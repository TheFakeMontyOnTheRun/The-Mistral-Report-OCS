#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <unistd.h>

#include "FixP.h"
#include "LoadBitmap.h"
#include "Enums.h"
#include "Engine.h"
#include "FixP.h"
#include "Common.h"
#include "LoadBitmap.h"
#include "Engine.h"
#include "CRenderer.h"
#include "CPackedFileReader.h"

extern size_t biggestOption;

#define WEDGE_TYPE_NEAR_LEFT 4
#define WEDGE_TYPE_NEAR_RIGHT 8

enum DIRECTION {
    DIRECTION_N,
    DIRECTION_E,
    DIRECTION_S,
    DIRECTION_W
};

#define IN_RANGE(V0, V1, V)  ((V0) <= (V) && (V) <= (V1))

int8_t stencilHigh[128];

int8_t cameraX = 33;
int8_t cameraZ = 22;
int8_t cameraRotation = 0;

void fix_line(int16_t x0, int16_t y0, int16_t x1, int16_t y1 );

struct Texture *montyTexture;

struct Projection {
    uint8_t px;
    uint8_t py;
    int16_t dx;
};


struct Pattern {
    int8_t floor;
    int8_t ceiling;
    uint8_t geometryType;
    uint8_t block;
};

const struct Projection projections[32] =
        {
                //                                   Z
                {0,  127, -64}, // 0
                {0,  127, -64}, // 1
                {20, 106, -43}, // 2
                {31, 95,  -32}, // 3
                {37, 89,  -26}, // 4
                {42, 84,  -21}, // 5
                {45, 81,  -18},  // 6
                {47, 79,  -16},  // 7
                {49, 77,  -14},  // 8
                {50, 76,  -13},  // 9
                {51, 75,  -12},  // 10
                {52, 74,  -11},  // 11
                {53, 73,  -10}, // 12
                {54, 72,  -9}, // 13
                {54, 72,  -9}, // 14
                {55, 71,  -8}, // 15
                {55, 71,  -8}, // 16
                {56, 71,  -8}, // 17
                {56, 70,  -7}, // 18
                {57, 70,  -7}, // 19
                {57, 69,  -6}, // 20
                {57, 69,  -6}, // 21
                {57, 69,  -6},  // 22
                {58, 69,  -6},  // 23
                {58, 68,  -5},  // 24
                {58, 68,  -5},  // 25
                {58, 68,  -5},  // 26
                {58, 68,  -5},  // 27
                {59, 67,  -5}, // 28
                {59, 67,  -4}, // 29
                {59, 67,  -4}, // 30
                {59, 67,  -4}, // 31
        };

const struct Pattern patterns[16] = {
        {-1,  5, 0, 0}, //0
        {-1,  5, 0, 1}, // 1
        {-1, -1, 0, 0}, // 2
        {-1, -1, 0, 0}, //3
        {-1, -1, 4, 0}, //4
        {-1, -1, 8, 0}, //5
        {-1,  2, 0, 0}, //6
        {-1,  5, 0, 0}, //7
        { 0,  5, 0, 0}, // 8
        { 2,  5, 0, 0}, // 9
};

const int8_t map[32][32] = {
        {0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 1, 2, 2, 7, 7, 7, 7, 7, 2, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 1, 2, 2, 7, 0, 0, 0, 7, 2, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 1, 1, 2, 7, 7, 7, 7, 7, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 1, 2, 7, 7, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 0, 0, 0},
        {0, 0, 0, 0, 0, 1, 2, 7, 7, 2, 2, 1, 1, 1, 1, 2, 7, 7, 7, 7, 7, 7, 7, 7, 2, 2, 2, 2, 1, 0, 0, 0},
        {1, 1, 1, 1, 1, 1, 2, 7, 7, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 2, 3, 3, 3, 7, 2, 2, 1, 0, 0, 0},
        {1, 2, 2, 2, 2, 2, 2, 7, 7, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 2, 7, 7, 7, 7, 2, 2, 1, 0, 0, 0},
        {1, 2, 2, 7, 7, 5, 3, 6, 6, 3, 4, 7, 7, 7, 7, 7, 7, 5, 2, 1, 1, 2, 7, 7, 7, 7, 2, 1, 1, 0, 0, 0},
        {1, 2, 2, 7, 7, 7, 7, 7, 7, 7, 7, 7, 0, 0, 0, 0, 7, 7, 2, 1, 1, 2, 7, 7, 6, 7, 2, 1, 1, 0, 0, 0},
        {1, 2, 2, 7, 7, 7, 7, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 2, 1, 1, 2, 7, 7, 7, 7, 2, 2, 1, 1, 1, 0},
        {1, 2, 2, 2, 6, 7, 8, 7, 7, 7, 7, 0, 0, 0, 0, 0, 0, 7, 2, 1, 1, 2, 7, 7, 0, 7, 2, 2, 2, 2, 1, 0},
        {1, 1, 2, 3, 6, 7, 7, 9, 7, 8, 7, 0, 0, 0, 0, 0, 0, 7, 2, 1, 1, 2, 7, 7, 0, 7, 7, 7, 2, 2, 1, 0},
        {0, 1, 2, 3, 6, 7, 7, 7, 7, 7, 7, 0, 0, 0, 0, 0, 0, 7, 2, 1, 1, 7, 7, 7, 0, 0, 0, 7, 2, 2, 1, 0},
        {0, 1, 2, 3, 6, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 2, 1, 1, 0, 0, 7, 0, 0, 0, 7, 2, 1, 1, 0},
        {0, 1, 2, 2, 6, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 2, 1, 7, 7, 7, 7, 0, 0, 0, 7, 2, 2, 1, 0},
        {0, 1, 1, 2, 7, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 2, 1, 2, 2, 2, 7, 0, 7, 7, 7, 2, 2, 1, 0},
        {0, 0, 1, 2, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 2, 1, 2, 2, 2, 7, 0, 7, 2, 2, 2, 2, 1, 0},
        {0, 0, 1, 2, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 2, 1, 2, 2, 2, 7, 0, 7, 2, 2, 1, 1, 1, 0},
        {0, 0, 1, 2, 7, 0, 0, 7, 7, 7, 7, 0, 0, 0, 0, 0, 0, 7, 2, 1, 2, 2, 2, 7, 0, 7, 2, 2, 1, 0, 0, 0},
        {0, 0, 1, 2, 7, 0, 0, 7, 4, 5, 7, 7, 7, 7, 7, 7, 7, 7, 2, 1, 2, 2, 2, 7, 7, 7, 2, 2, 1, 0, 0, 0},
        {0, 0, 1, 2, 7, 0, 0, 7, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 2, 2, 7, 7, 7, 2, 2, 2, 1, 0, 0, 0},
        {0, 0, 1, 2, 7, 0, 0, 7, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 7, 0, 7, 2, 2, 1, 1, 0, 0, 0},
        {0, 1, 1, 2, 7, 0, 0, 7, 2, 2, 2, 2, 2, 7, 0, 0, 7, 7, 3, 1, 2, 2, 7, 0, 7, 2, 1, 1, 0, 0, 0, 0},
        {0, 1, 2, 2, 7, 0, 0, 7, 7, 7, 7, 7, 7, 7, 0, 0, 0, 7, 7, 7, 7, 7, 7, 0, 7, 2, 2, 1, 0, 0, 0, 0},
        {0, 1, 2, 2, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 0, 0, 0, 7, 7, 7, 7, 7, 7, 7, 7, 2, 2, 1, 0, 0, 0, 0},
        {0, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 7, 0, 0, 0, 7, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 0, 0, 0, 0},
        {0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 7, 0, 0, 7, 7, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 2, 7, 7, 7, 7, 2, 2, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 2, 2, 2, 2, 2, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
};

int8_t max(int8_t x1, int8_t x2) {
    return x1 > x2 ? x1 : x2;
}

int8_t min(int8_t x1, int8_t x2) {
    return x1 < x2 ? x1 : x2;
}


void drawWedge(int8_t x0, int8_t y0, int8_t z0, int8_t dX, int8_t dY, int8_t dZ, uint8_t type) {

    int16_t z1;
    int16_t z0px;
    int16_t z1px;
    int16_t z0py;

    int16_t z1py;
    int16_t z0dx;
    int16_t z1dx;

    int16_t px0z0;
    int16_t py0z0;
    int16_t py0z1;

    int16_t py1z0;
    int16_t py1z1;

    int16_t px1z1;

    uint8_t drawContour;

    if (z0 >= 32) {
        z0 = 31;
    }

    z1 = z0 + dZ;

    if (z0 <= 2) {
        return;
    }

    if (z1 <= 2) {
        return;
    }

    if (z1 >= 32) {
        z1 = 31;
    }


    if (type == WEDGE_TYPE_NEAR_LEFT) {
        z0px = (projections[z0].px);
        z1px = (projections[z1].px);
        z0dx = ((projections[z0].dx));
        z1dx = ((projections[z1].dx));

        px0z0 = z0px - (x0 * z0dx);
        px1z1 = z1px - ((dX + x0) * z1dx);

        z1py = (projections[z1].py);
        z0py = (projections[z0].py);

        py0z0 = z0py + ((y0) * z0dx);
        py0z1 = z1py + ((y0) * z1dx);

        py1z0 = z0py + ((y0 + dY) * z0dx);
        py1z1 = z1py + ((y0 + dY) * z1dx);

    } else {
        z0px = (projections[z1].px);
        z1px = (projections[z0].px);
        z0dx = ((projections[z1].dx));
        z1dx = ((projections[z0].dx));

        px0z0 = z0px - ((x0) * z0dx);
        px1z1 = z1px - ((x0 + dX) * z1dx); //extra operations to avoid overflow

        z1py = (projections[z0].py);
        z0py = (projections[z1].py);

        py0z0 = z0py + ((y0) * z0dx);
        py0z1 = z1py + ((y0) * z1dx);

        py1z0 = z0py + ((y0 + dY) * z0dx);
        py1z1 = z1py + ((y0 + dY) * z1dx);
    }

    if (px1z1 < 0 || px0z0 > 127) {
        return;
    }

    drawContour = 1;

    {
        int16_t x0, x1;

        if (drawContour) {

            if (IN_RANGE(0, 127, px0z0) && stencilHigh[px0z0] < py0z0) {
                graphicsVerticalLine(px0z0, py0z0, max(py1z0, stencilHigh[px0z0]), 0);
            }


            if (IN_RANGE(0, 127, px1z1) && py0z1 > stencilHigh[px1z1]) {
                graphicsVerticalLine(px1z1, py0z1, max(py1z1, stencilHigh[px1z1]), 0);
            }
        }



        /* The upper segment */
        x0 = px0z0;
        x1 = px1z1;

        if (x0 != x1) {
            int16_t y0 = py1z0;
            int16_t y1 = py1z1;
            int16_t dx = abs(x1 - x0);
            int16_t sx = x0 < x1 ? 1 : -1;
            int16_t dy = -abs(y1 - y0);
            int16_t sy = y0 < y1 ? 1 : -1;
            int16_t err = dx + dy;  /* error value e_xy */
            int16_t e2;

            while ((x0 != x1 || y0 != y1)) {

                if (IN_RANGE(0, 127, x0)) {
                    if (stencilHigh[x0] <= y0) {
                        if (drawContour) {
                            graphicsPut (x0, y0, 0);
                        }
                    }
                }

                /* loop */
                e2 = err * 2;

                if (e2 >= dy) {
                    err += dy; /* e_xy+e_x > 0 */
                    x0 += sx;
                }

                if (x0 >= 128) {
                    goto done_upper_stroke;
                }

                if (e2 <= dx) {
                    /* e_xy+e_y < 0 */
                    err += dx;
                    y0 += sy;
                }
            }
        }

        done_upper_stroke:

        /* The lower segment */
        x0 = px0z0;
        x1 = px1z1;

        if (x0 != x1) {
            int16_t y0 = py0z0;
            int16_t y1 = py0z1;
            int16_t dx = abs(x1 - x0);
            int16_t sx = x0 < x1 ? 1 : -1;
            int16_t dy = -abs(y1 - y0);
            int16_t sy = y0 < y1 ? 1 : -1;
            int16_t err = dx + dy;  /* error value e_xy */
            int16_t e2;

            while ((x0 != x1 || y0 != y1)) {

                if (IN_RANGE(0, 127, x0)) {
                    if (stencilHigh[x0] < y0) {
                        if (drawContour) {
#ifdef FILLED_POLYS
                            graphicsVerticalLine(x0, y0, stencilHigh[x0], 8);
#endif
                            graphicsPut(x0, y0, 0);
                        }
                        stencilHigh[x0] = y0;
                    }
                }

                /* loop */
                e2 = err * 2;

                if (e2 >= dy) {
                    err += dy; /* e_xy+e_x > 0 */
                    x0 += sx;
                }

                if (x0 >= 128) {
                    return;
                }

                if (e2 <= dx) {
                    /* e_xy+e_y < 0 */
                    err += dx;
                    y0 += sy;
                }
            }
        }
    }
}

void drawHighCubeAt(int8_t x0, int8_t y0, int8_t z0, int8_t dX, int8_t dY, int8_t dZ ) {

    int8_t z1;
    uint8_t z0px;
    uint8_t z0py;
    uint8_t z1px;
    uint8_t z1py;
    int8_t z0dx;
    int8_t z1dx;

    int16_t px0z0;
    int8_t py0z0;
    int16_t px1z0;
    int8_t py1z0;
    int16_t px0z1;
    int8_t py0z1;
    int16_t px1z1;

    uint8_t drawContour;

    if (z0 >= 32) {
        z0 = 31;
    }

    z1 = z0 + dZ;

    if (z1 >= 32) {
        z1 = 31;
    }


    z0px = (projections[z0].px);
    z1px = (projections[z1].px);
    z0dx = ((projections[z0].dx));
    z1dx = ((projections[z1].dx));

    px0z0 = z0px - ((x0) * z0dx);
    px0z1 = z1px - ((x0) * z1dx);

    px1z0 = px0z0 - (dX * z0dx);
    px1z1 = px0z1 - (dX * z1dx);

    z1py = (projections[z1].py);
    z0py = (projections[z0].py);

    py0z0 = z0py + ((y0) * z0dx);
    py1z0 = py0z0 + (dY * z0dx);
    py0z1 = z1py + ((y0) * z1dx);

    if (px1z0 < 0 || px0z0 > 127) {
        return;
    }

    drawContour = (dY);
    {
        int16_t x, x0, x1;

        if (drawContour) {

            if (IN_RANGE(0, 127, px0z0) && stencilHigh[px0z0] < py0z0) {
                graphicsVerticalLine(px0z0, py0z0, stencilHigh[px0z0], 0);
            }

            if (IN_RANGE(0, 127, px1z0) && stencilHigh[px1z0] < py0z0) {
                graphicsVerticalLine(px1z0, py0z0, stencilHigh[px1z0],0);
            }
            if (IN_RANGE(0, 127, px0z1) && px0z1 < px0z0 && py0z1 > stencilHigh[px0z1]) {
                graphicsVerticalLine(px0z1, py0z1, stencilHigh[px0z1], 0);
            }

            if (IN_RANGE(0, 127, px1z1) && px1z1 > px1z0 && py0z1 > stencilHigh[px1z1]) {
                graphicsVerticalLine(px1z1, py0z1, stencilHigh[px1z1], 0);
            }

        }

        /* Draw the horizontal outlines of z0 and z1 */

        if (py0z0 > py0z1) {
            /* Ceiling is lower than camera */
            for (x = px0z0; x <= px1z0; ++x) {
                if (IN_RANGE(0, 127, x) && stencilHigh[x] < py0z0) {
                    if (drawContour) {
#ifdef FILLED_POLYS
                        graphicsVerticalLine(x, py0z0, stencilHigh[x], 9);
#endif
                        graphicsPut(x, py0z0, 0);
                        graphicsPut(x, stencilHigh[x], 0);
                    }
                    stencilHigh[x] = py0z0;
                }
            }
        } else if (drawContour) {
            /* Ceiling is higher than the camera*/
            /* Let's just draw the nearer segment */
            for (x = px0z0; x <= px1z0; ++x) {
                if (IN_RANGE(0, 127, x) && stencilHigh[x] < py0z0) {
#ifdef FILLED_POLYS
                    graphicsVerticalLine(x, py0z0, stencilHigh[x], 9);
#endif
                    graphicsPut(x, py0z0, 0);
                    graphicsPut(x, stencilHigh[x], 0);
                }
            }
        }


        /* The left segment */
        x0 = px0z0;
        x1 = px0z1;

        if (x0 != x1) {
            int16_t y0 = py0z0;
            int16_t y1 = py0z1;
            int16_t dx = abs(x1 - x0);
            int16_t sx = x0 < x1 ? 1 : -1;
            int16_t dy = -abs(y1 - y0);
            int16_t sy = y0 < y1 ? 1 : -1;
            int16_t err = dx + dy;  /* error value e_xy */
            int16_t e2;

            while ((x0 != x1 || y0 != y1)) {

                if (IN_RANGE(0, 127, x0)) {
                    if (stencilHigh[x0] < y0) {
                        if (drawContour) {
#ifdef FILLED_POLYS
                            graphicsVerticalLine(x0, y0, stencilHigh[x0], 9);
#endif
                            graphicsPut(x0, y0, 0);
                            graphicsPut(x0, stencilHigh[x0], 0);
                        }
                        stencilHigh[x0] = y0;
                    }
                }

                /* loop */
                e2 = err * 2;

                if (e2 >= dy) {
                    err += dy; /* e_xy+e_x > 0 */
                    x0 += sx;
                }

                if (x0 >= 128) {
                    goto right_stroke;
                }

                if (e2 <= dx) {
                    /* e_xy+e_y < 0 */
                    err += dx;
                    y0 += sy;
                }
            }
        }

        right_stroke:

        /* The right segment */
        x0 = px1z0;
        x1 = px1z1;

        if (x0 != x1) {
            int16_t y0 = py0z0;
            int16_t y1 = py0z1;
            int16_t dx = abs(x1 - x0);
            int16_t sx = x0 < x1 ? 1 : -1;
            int16_t dy = -abs(y1 - y0);
            int16_t sy = y0 < y1 ? 1 : -1;
            int16_t err = dx + dy;  /* error value e_xy */
            int16_t e2;

            while ((x0 != x1 || y0 != y1)) {

                if (IN_RANGE(0, 127, x0) && stencilHigh[x0] < y0) {
                    if (drawContour) {
#ifdef FILLED_POLYS
                        graphicsVerticalLine(x0, y0, stencilHigh[x0], 9);
#endif
                        graphicsPut(x0, y0, 0);
                        graphicsPut(x0, stencilHigh[x0], 0);
                    }
                    stencilHigh[x0] = y0;
                }

                /* loop */
                e2 = err * 2;

                if (e2 >= dy) {
                    err += dy; /* e_xy+e_x > 0 */
                    x0 += sx;
                }

                if (x0 >= 128) {
                    goto final_stroke;
                }

                if (e2 <= dx) {
                    /* e_xy+e_y < 0 */
                    err += dx;
                    y0 += sy;
                }
            }
        }

        final_stroke:
        if (py0z0 <= py0z1) {
            /* Ceiling is higher than the camera*/
            /* Draw the last segment */

            for (x = px0z1; x <= px1z1; ++x) {
                if (IN_RANGE(0, 127, x) && stencilHigh[x] < py0z1) {
                    if (drawContour) {
                        graphicsPut(x, py0z1, 0);
                    }
                    stencilHigh[x] = py0z1;
                }
            }
        }
    }
}


void drawPattern(uint8_t pattern, uint8_t x0, uint8_t x1, uint8_t y) {

    uint8_t type = patterns[pattern].geometryType;

    if (type == 0) {
        drawHighCubeAt(x0, patterns[pattern].ceiling, y, x1 - x0,
                       patterns[0].ceiling - patterns[pattern].ceiling, 1);

    } else {
        switch( cameraRotation) {
            case DIRECTION_W:
            case DIRECTION_E:
                if (type == WEDGE_TYPE_NEAR_LEFT) {
                    type = WEDGE_TYPE_NEAR_RIGHT;
                } else {
                    type = WEDGE_TYPE_NEAR_LEFT;
                }
                break;

        }

        drawWedge(x0, patterns[pattern].ceiling, y, x1 - x0,
                  patterns[0].ceiling - patterns[pattern].ceiling, 1, type);
    }
}

void renderScene() {
    uint8_t lastPattern, lastIndex;

    switch (cameraRotation) {
        case DIRECTION_N: {

            int8_t limit = max(cameraZ - 19, 0);
            for (int8_t y = min(cameraZ - 3, 31); y >= limit; --y) {
                int8_t x;
                lastIndex = cameraX;
                lastPattern = map[y][lastIndex];

                for (x = lastIndex; x < min(cameraX + 5 + ((cameraZ - 3) - y), 31); ++x) {
                    uint8_t pattern;

                    pattern = map[y][x];

                    if (pattern != lastPattern) {
                        if (lastPattern != 0) {
                            drawPattern(lastPattern, lastIndex - cameraX, x - cameraX, cameraZ - y);
                            lastIndex = x;
                        }
                        lastPattern = pattern;
                    }

                    if (patterns[pattern].block) {
                        break;
                    }

                }
                if (lastPattern != 0) {
                    drawPattern(lastPattern, lastIndex - cameraX, x - cameraX, cameraZ - y);
                }

                lastIndex = max(cameraX - 1, 0);
                lastPattern = map[y][lastIndex];

                for (x = lastIndex; x >= max(cameraX - 3 - ((cameraZ - 3) - y), 0); --x) {
                    uint8_t pattern;
                    pattern = map[y][x];

                    if (pattern != lastPattern) {
                        if (lastPattern != 0) {
                            drawPattern(lastPattern, x + 1 - cameraX, lastIndex + 1 - cameraX, cameraZ - y);
                            lastIndex = x;
                        }
                        lastPattern = pattern;
                    }

                    if (patterns[pattern].block) {
                        break;
                    }

                }
                if (lastPattern != 0) {
                    drawPattern(lastPattern, x + 1 - cameraX, lastIndex + 1 - cameraX, cameraZ - y);
                }
            }
        }
            break;

        case DIRECTION_E: {

            for (int8_t x = min(cameraX - 3, 31); x <= min(cameraX + 13, 31); ++x) {
                int8_t y;

                for (y = cameraZ; y <= min(cameraZ + (x - cameraX), 31); ++y) {
                    drawPattern(map[y][x], y - cameraZ + 3, y + 1 - cameraZ + 3, x - cameraX + 3);
                }

                for (y = max(cameraZ - 1, 0); y >= max(cameraZ - (x - cameraX), 0); --y) {
                    drawPattern(map[y][x], y - cameraZ + 3, y + 1 - cameraZ + 3, x - cameraX + 3);
                }

            }
        }
            break;

        case DIRECTION_S: {

            for (int8_t y = min(cameraZ + 3, 31); y <= min(cameraZ + 19, 31); ++y) {
                int8_t x;
                for (x = cameraX; x <= min(cameraX + (y - (cameraZ + 3)), 31); ++x) {
                    drawPattern(map[y][x], cameraX - x, cameraX - x + 1, y - cameraZ);
                }

                for (x = max(cameraX - 1, 0); x >= max(cameraX - (y - (cameraZ + 3)), 0); --x) {
                    drawPattern(map[y][x], cameraX - x, cameraX - x + 1, y - cameraZ);
                }
            }
        }
            break;

        case DIRECTION_W: {

            for (int8_t x = max(cameraX, 0); x >= max(cameraX - 16, 0); --x) {
                int8_t y;
                for (y = cameraZ; y <= min(cameraZ - (x - (cameraX)), 31); ++y) {
                    drawPattern(map[y][x], y - cameraZ + 3, y + 1 - cameraZ + 3, cameraX - x + 1);
                }

                for (y = max(cameraZ - 1, 0); y >= max(cameraZ + (x - (cameraX)), 0); --y) {
                    drawPattern(map[y][x], y - cameraZ + 3, y + 1 - cameraZ + 3, cameraX - x + 1);
                }
            }
        }
            break;
    }
}


int32_t Interrogation_initStateCallback(int32_t tag, void *data) {

    currentBackgroundBitmap = loadBitmap("pattern.img");
    montyTexture = makeTextureFrom("montytex.img");

    timeUntilNextState = 10000 - 1;

    currentPresentationState = kWaitingForInput;

    cameraX = 5;
    cameraZ = 15;
    cameraRotation = 0;


    memset(&stencilHigh[0], 0, 128);

    return 0;
}

void Interrogation_initialPaintCallback() {
    drawRepeatBitmap(0, 0, 320, 200, currentBackgroundBitmap);

    fill(8, 144, 120, 8, 0, FALSE);
    fill(142, 144, 80, 8, 0, FALSE);
    fill(236, 144, 64, 8, 0, FALSE);

    drawTextAt(3, 19, "Interrogation", 4);
    drawTextAt(20, 19, "Emotions", 4);
    drawTextAt(32, 19, "Stress", 4);


    fill(8 + 8, 144 + 8, 120, 48, 0, TRUE);
    fill(142 + 8, 144 + 8, 80, 48, 0, TRUE);
    fill(236 + 8, 144 + 8, 64, 48, 0, TRUE);

    fill(8, 152, 120, 40, 7, FALSE);
    fill(142, 152, 80, 40, 7, FALSE);
    fill(236, 152, 64, 40, 7, FALSE);

    drawRect(8, 152, 120, 40, 0);
    drawRect(142, 152, 80, 40, 0);

    drawRect(236, 152, 64, 40, 0);
}

void Interrogation_repaintCallback() {
    memset(&stencilHigh[0], 0, 128);

    fill(255, 8, 8, 128, 0, TRUE);
    fill(0, 128, 256, 8, 0, TRUE);

    fill(0, 0, 256, 128, 7, FALSE);

    drawRect(0, 0, 256, 128, 0);
    fill(0, 0, 256, 8, 0, FALSE);
    renderScene();

    drawWall(intToFix(16), intToFix(64), intToFix(32), intToFix(64), intToFix(16), intToFix(96), &montyTexture->rowMajor[0], intToFix(1), 1 );
}

int32_t Interrogation_tickCallback(int32_t tag, void *data) {

    long delta = *((long *) data);

    timeUntilNextState -= delta;
    uint8_t prevX;
    uint8_t prevZ;

    if (currentPresentationState == kWaitingForInput ) {

        prevX = cameraX;
        prevZ = cameraZ;

        switch (tag) {
            case kCommandLeft:
                cameraRotation--;
                if (cameraRotation < 0) {
                    cameraRotation = 3;
                }
                break;

            case kCommandRight:
                cameraRotation = (cameraRotation + 1) & 3;
                break;

            case kCommandBack:
                return kMainMenu;

            case kCommandStrafeLeft:
                cameraX--;
                break;
            case kCommandStrafeRight:
                cameraX++;
                break;
            case kCommandDown:
                cameraZ++;
                break;
            case kCommandUp:
                cameraZ--;
                break;
        }

        if (cameraZ >= 32) {
            cameraZ = 31;
        }

        if (cameraX >= 32) {
            cameraX = 31;
        }

        if (cameraZ < 0) {
            cameraZ = 0;
        }

        if (cameraX < 0) {
            cameraX = 0;
        }

        if (patterns[map[cameraZ - 2][cameraX]].ceiling < 2) {
            cameraX = prevX;
            cameraZ = prevZ;
        }
    }

    if (timeUntilNextState <= 0) {

        switch (currentPresentationState) {
            case kAppearing:
                timeUntilNextState = MENU_ITEM_TIME_TO_BECOME_ACTIVE_MS;
                currentPresentationState = kWaitingForInput;
                break;
            case kConfirmInputBlink1:
            case kConfirmInputBlink2:
            case kConfirmInputBlink3:
            case kConfirmInputBlink4:
            case kConfirmInputBlink5:
            case kConfirmInputBlink6:
                timeUntilNextState = MENU_ITEM_TIME_TO_BLINK_MS;
                currentPresentationState =
                        (enum EPresentationState) ((int) currentPresentationState + 1);
                break;
            case kFade:
                return nextNavigationSelection;
        }
    }


    return -1;
}

void Interrogation_unloadStateCallback() {
    releaseBitmap(currentBackgroundBitmap);
}
