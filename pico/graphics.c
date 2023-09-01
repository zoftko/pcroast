#include "graphics.h"

static GHandle temperatureLabel;
static char temperatureString[10] = "000.00 C.";

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
    wi.text = temperatureString;

    temperatureLabel = gwinLabelCreate(NULL, &wi);
    graphicsSetTemperature(0);
}

void graphicsSetTemperature(float temperature) {
    sprintf(temperatureString, "%06.2f C.", temperature);
    gwinSetText(temperatureLabel, temperatureString, gTrue);
}
