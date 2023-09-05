#include "graphics.h"

static GHandle temperatureLabel;
static char temperatureString[7] = "000.00";

static gFont dejaVu20Font;

void graphicsInit() {
    gfxInit();

    dejaVu20Font = gdispOpenFont("DejaVuSans20");

    gwinSetDefaultFont(dejaVu20Font);
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
    sprintf(temperatureString, "%06.2f", temperature);
    gwinSetText(temperatureLabel, temperatureString, gTrue);
}
