#ifndef MOCKDISPLAY_H
#define MOCKDISPLAY_H

#include <QObject>
#include <QImage>
#include "inkclocktypes.h"

class MockDisplay : public QObject
{
    Q_OBJECT

public:
    explicit MockDisplay(QObject *parent = nullptr);
    ~MockDisplay();
    
    // Display properties
    int getWidth() const;
    int getHeight() const;
    
    // Display control methods
    void powerOn();
    void powerOff();
    bool isPoweredOn() const;
    
    void refresh();
    void clear();
    void update();
    
    // Brightness control
    int getBrightness() const;
    void setBrightness(int brightness);
    
    // Image manipulation
    QImage getImage() const;
    void setImage(const QImage &image);
    
    // Pixel manipulation
    void drawPixel(int x, int y, const QColor &color);
    QColor getPixel(int x, int y) const;
    
    // Drawing methods
    void drawLine(int x1, int y1, int x2, int y2, const QColor &color);
    void drawRect(int x, int y, int width, int height, const QColor &color, bool filled = false);
    void drawCircle(int x, int y, int radius, const QColor &color, bool filled = false);
    void drawText(int x, int y, const QString &text, const QFont &font, const QColor &color);
    
    // E-paper specific methods
    int getRefreshCount() const;
    void resetRefreshCount();
    
    bool isBusy() const;
    int getRefreshTime() const;
    
    // Update interval
    int getUpdateInterval() const;
    void setUpdateInterval(int interval);

signals:
    // Signal emitted when display is refreshed
    void displayRefreshed();
    
    // Signal emitted when display content changes
    void displayUpdated();

private:
    // Display properties
    int width;
    int height;
    bool poweredOn;
    int brightness;
    int refreshCount;
    bool busy;
    int updateInterval;
    
    // Display buffer
    QImage buffer;
};

#endif // MOCKDISPLAY_H