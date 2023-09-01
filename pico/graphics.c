#include "graphics.h"

static GHandle temperatureLabel;
static char temperatureString[7] = "000 C.";

static gFont dejaVu12Font;

void graphicsInit() {
    gfxInit();

    dejaVu12Font = gdispOpenFont("DejaVuSans16");

    gwinSetDefaultFont(dejaVu12Font);
    gwinSetDefaultStyle(&BlackWidgetStyle, FALSE);

    GWidgetInit wi;
    gwinWidgetClearInit(&wi);

    wi.g.show = gTrue;
    wi.g.x = 24;
    wi.g.y = 62;
    wi.text = "000 C.";

    temperatureLabel = gwinLabelCreate(NULL, &wi);
    graphicsSetTemperature(0);
}

void graphicsSetTemperature(uint8_t temperature) {
    sprintf(temperatureString, "%03d C.", temperature);
    gwinSetText(temperatureLabel, temperatureString, gTrue);
}
