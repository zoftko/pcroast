#include "graphics.h"

static GHandle temperatureGraph;
static GGraphObject graphObj;

static GHandle temperatureLabel;
static char temperatureString[7] = "      ";

static gFont dejaVu16Font;

static uint8_t xPoint;
static uint8_t yPoint;

/* clang-format off */
static GGraphStyle graphStyle = {
    {GGRAPH_POINT_DOT, 0, Yellow},
    {GGRAPH_LINE_NONE, 0, Gray},
    {GGRAPH_LINE_SOLID, 0, White},
    {GGRAPH_LINE_NONE, 0, White},
    {GGRAPH_LINE_SOLID, 2, Gray, 32},
    {GGRAPH_LINE_SOLID, 2, Gray, 25},
    0  // Flags
};
/* clang-format on */

void graphicsInit() {
    gfxInit();
    dejaVu16Font = gdispOpenFont("DejaVuSans16");
    gwinSetDefaultFont(dejaVu16Font);
    gwinSetDefaultStyle(&BlackWidgetStyle, FALSE);

    GWidgetInit wi;
    gwinWidgetClearInit(&wi);
    wi.g.show = gTrue;
    wi.g.x = 4;
    wi.g.y = 144;
    wi.g.height = 16;
    wi.g.width = 56;
    wi.text = temperatureString;
    temperatureLabel = gwinLabelCreate(NULL, &wi);

    GWindowInit winInit;
    gwinClearInit(&winInit);
    winInit.show = gTrue;
    winInit.x = winInit.y = 0;
    winInit.width = gdispGetWidth();
    winInit.height = 140;

    /**
     * For unknown reasons if the graph's axis are written on this routine, they are cleared.
     * Therefore xPoint is set to 129, this forces a redraw of the graph when the first temperature
     * reading is taken and displayed on screen, overcoming this. It seems that axis written
     * outside of this routine don't disappear.
     */
    xPoint = 129;
    temperatureGraph = gwinGraphCreate(&graphObj, &winInit);
    gwinGraphSetOrigin(temperatureGraph, 0, 0);
    gwinGraphSetStyle(temperatureGraph, &graphStyle);
}

void graphicsSetTemperature(float temperature) {
    sprintf(temperatureString, "%06.2f", temperature);
    gwinSetText(temperatureLabel, temperatureString, gFalse);

    yPoint = (uint8_t)temperature - 25;
    if (xPoint > 128) {
        gwinClear(temperatureGraph);
        gwinGraphDrawAxis(temperatureGraph);
        xPoint = 0;
    }

    gwinGraphDrawPoint(temperatureGraph, xPoint, yPoint);
    xPoint++;
}
