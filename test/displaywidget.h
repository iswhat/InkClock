#ifndef DISPLAYWIDGET_H
#define DISPLAYWIDGET_H

#include <QWidget>
#include <QPainter>
#include <QDateTime>
#include "inkclocktypes.h"

class DisplayWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DisplayWidget(QWidget *parent = nullptr);
    ~DisplayWidget();

    // Update the display with new data
    void updateDisplay(const DisplayData &data);

protected:
    // Override paint event to draw the display
    void paintEvent(QPaintEvent *event) override;

private:
    // Display data
    DisplayData displayData;
    
    // Draw methods for different display elements
    void drawBackground(QPainter &painter);
    void drawClock(QPainter &painter);
    void drawDate(QPainter &painter);
    void drawLunarDate(QPainter &painter);
    void drawWeather(QPainter &painter);
    void drawBattery(QPainter &painter);
    void drawWiFi(QPainter &painter);
    void drawTemperature(QPainter &painter);
    void drawHumidity(QPainter &painter);
    void drawPressure(QPainter &painter);
    void drawLightLevel(QPainter &painter);
    
    // Helper methods
    void drawInvertedText(QPainter &painter, const QString &text, const QRect &rect, const QFont &font);
    void drawBox(QPainter &painter, const QRect &rect, bool inverted = false);
};

#endif // DISPLAYWIDGET_H