#include "mockdisplay.h"
#include <QPainter>
#include <QFont>
#include <QFontMetrics>

MockDisplay::MockDisplay(QObject *parent)
    : QObject(parent),
      width(400),
      height(300),
      poweredOn(false),
      brightness(70),
      refreshCount(0),
      busy(false),
      updateInterval(30)
{
    // Initialize display buffer
    buffer = QImage(width, height, QImage::Format_Mono);
    buffer.fill(Qt::white);
}

MockDisplay::~MockDisplay()
{
}

// Display properties
int MockDisplay::getWidth() const
{
    return width;
}

int MockDisplay::getHeight() const
{
    return height;
}

// Display control methods
void MockDisplay::powerOn()
{
    if (!poweredOn) {
        poweredOn = true;
        emit displayUpdated();
    }
}

void MockDisplay::powerOff()
{
    if (poweredOn) {
        poweredOn = false;
        emit displayUpdated();
    }
}

bool MockDisplay::isPoweredOn() const
{
    return poweredOn;
}

void MockDisplay::refresh()
{
    if (!poweredOn) {
        return;
    }
    
    // Simulate display refresh
    busy = true;
    
    // Increment refresh count
    refreshCount++;
    
    // Simulate refresh time (e-paper takes time to refresh)
    // In a real implementation, this would be a delay
    // For now, we'll just emit the signal immediately
    
    busy = false;
    emit displayRefreshed();
    emit displayUpdated();
}

void MockDisplay::clear()
{
    if (!poweredOn) {
        return;
    }
    
    // Clear the display buffer
    buffer.fill(Qt::white);
    emit displayUpdated();
}

void MockDisplay::update()
{
    if (!poweredOn) {
        return;
    }
    
    // Update the display (this is different from refresh in e-paper terms)
    // In e-paper, update would prepare the display buffer, but not refresh the physical display
    emit displayUpdated();
}

// Brightness control
int MockDisplay::getBrightness() const
{
    return brightness;
}

void MockDisplay::setBrightness(int brightness)
{
    this->brightness = qBound(0, brightness, 100);
    emit displayUpdated();
}

// Image manipulation
QImage MockDisplay::getImage() const
{
    return buffer;
}

void MockDisplay::setImage(const QImage &image)
{
    if (!poweredOn) {
        return;
    }
    
    // Scale image to fit display
    buffer = image.scaled(width, height, Qt::KeepAspectRatio, Qt::SmoothTransformation).convertToFormat(QImage::Format_Mono);
    emit displayUpdated();
}

// Pixel manipulation
void MockDisplay::drawPixel(int x, int y, const QColor &color)
{
    if (!poweredOn || x < 0 || x >= width || y < 0 || y >= height) {
        return;
    }
    
    // Set pixel color in buffer
    buffer.setPixel(x, y, color.rgb());
}

QColor MockDisplay::getPixel(int x, int y) const
{
    if (x < 0 || x >= width || y < 0 || y >= height) {
        return QColor(Qt::white);
    }
    
    return buffer.pixelColor(x, y);
}

// Drawing methods
void MockDisplay::drawLine(int x1, int y1, int x2, int y2, const QColor &color)
{
    if (!poweredOn) {
        return;
    }
    
    QPainter painter(&buffer);
    painter.setPen(color);
    painter.drawLine(x1, y1, x2, y2);
    emit displayUpdated();
}

void MockDisplay::drawRect(int x, int y, int width, int height, const QColor &color, bool filled)
{
    if (!poweredOn) {
        return;
    }
    
    QPainter painter(&buffer);
    painter.setPen(color);
    
    if (filled) {
        painter.setBrush(color);
        painter.drawRect(x, y, width, height);
    } else {
        painter.drawRect(x, y, width, height);
    }
    
    emit displayUpdated();
}

void MockDisplay::drawCircle(int x, int y, int radius, const QColor &color, bool filled)
{
    if (!poweredOn) {
        return;
    }
    
    QPainter painter(&buffer);
    painter.setPen(color);
    
    if (filled) {
        painter.setBrush(color);
        painter.drawEllipse(x - radius, y - radius, radius * 2, radius * 2);
    } else {
        painter.drawEllipse(x - radius, y - radius, radius * 2, radius * 2);
    }
    
    emit displayUpdated();
}

void MockDisplay::drawText(int x, int y, const QString &text, const QFont &font, const QColor &color)
{
    if (!poweredOn) {
        return;
    }
    
    QPainter painter(&buffer);
    painter.setPen(color);
    painter.setFont(font);
    painter.drawText(x, y, text);
    emit displayUpdated();
}

// E-paper specific methods
int MockDisplay::getRefreshCount() const
{
    return refreshCount;
}

void MockDisplay::resetRefreshCount()
{
    refreshCount = 0;
}

bool MockDisplay::isBusy() const
{
    return busy;
}

int MockDisplay::getRefreshTime() const
{
    // Typical e-paper refresh time is around 1-2 seconds
    return 1500; // 1.5 seconds in milliseconds
}

// Update interval
int MockDisplay::getUpdateInterval() const
{
    return updateInterval;
}

void MockDisplay::setUpdateInterval(int interval)
{
    this->updateInterval = qBound(1, interval, 3600);
}