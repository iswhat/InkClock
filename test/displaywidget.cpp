#include "displaywidget.h"
#include <QFont>
#include <QFontMetrics>
#include <QColor>
#include <QRect>

DisplayWidget::DisplayWidget(QWidget *parent)
    : QWidget(parent)
{
    // Set widget properties
    setStyleSheet("background-color: #f0f0f0; border: 2px solid #333; border-radius: 5px;");
    
    // Initialize display data with default values
    displayData.powerOn = false;
    displayData.time = QDateTime::currentDateTime();
    displayData.temperature = 22;
    displayData.humidity = 50;
    displayData.pressure = 1013;
    displayData.lightLevel = 200;
    displayData.batteryLevel = 80;
    displayData.wifiStatus = WiFiStatus::Connected;
    displayData.brightness = 70;
    displayData.motionDetected = false;
    displayData.updateInterval = 30;
    displayData.mode = DisplayMode::ClockMode;
}

DisplayWidget::~DisplayWidget()
{
}

void DisplayWidget::updateDisplay(const DisplayData &data)
{
    displayData = data;
    update(); // Trigger repaint
}

void DisplayWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Draw background first
    drawBackground(painter);
    
    if (!displayData.powerOn) {
        // Device is off, draw empty screen
        return;
    }
    
    // Draw different elements based on display mode
    switch (displayData.mode) {
        case DisplayMode::ClockMode:
            drawClock(painter);
            drawDate(painter);
            drawLunarDate(painter);
            drawWeather(painter);
            drawBattery(painter);
            drawWiFi(painter);
            break;
        case DisplayMode::WeatherMode:
            drawWeather(painter);
            drawTemperature(painter);
            drawHumidity(painter);
            drawPressure(painter);
            drawDate(painter);
            break;
        case DisplayMode::SensorMode:
            drawTemperature(painter);
            drawHumidity(painter);
            drawPressure(painter);
            drawLightLevel(painter);
            drawDate(painter);
            break;
        default:
            drawClock(painter);
            drawDate(painter);
    }
}

void DisplayWidget::drawBackground(QPainter &painter)
{
    if (!displayData.powerOn) {
        // Device is off, draw black screen
        painter.fillRect(rect(), Qt::black);
        return;
    }
    
    // Draw e-paper-like background
    painter.fillRect(rect(), Qt::white);
    
    // Add subtle grid pattern for e-paper effect
    QPen gridPen(QColor(230, 230, 230), 0.5);
    painter.setPen(gridPen);
    
    int gridSize = 10;
    for (int x = 0; x <= width(); x += gridSize) {
        painter.drawLine(x, 0, x, height());
    }
    for (int y = 0; y <= height(); y += gridSize) {
        painter.drawLine(0, y, width(), y);
    }
}

void DisplayWidget::drawClock(QPainter &painter)
{
    // Calculate clock area
    QRect clockRect(10, 10, width() - 20, 120);
    
    // Draw time text
    QFont timeFont("Arial", 48, QFont::Bold);
    QString timeString = displayData.time.toString("HH:mm:ss");
    
    QFontMetrics timeMetrics(timeFont);
    int timeWidth = timeMetrics.horizontalAdvance(timeString);
    int timeHeight = timeMetrics.height();
    
    QRect timeRect((width() - timeWidth) / 2, clockRect.top() + (clockRect.height() - timeHeight) / 2, 
                   timeWidth, timeHeight);
    
    painter.setFont(timeFont);
    painter.setPen(Qt::black);
    painter.drawText(timeRect, Qt::AlignCenter, timeString);
}

void DisplayWidget::drawDate(QPainter &painter)
{
    // Calculate date area
    QRect dateRect(10, 130, width() - 20, 40);
    
    // Draw date text
    QFont dateFont("Arial", 16, QFont::Medium);
    QString dateString = displayData.time.toString("yyyy-MM-dd dddd");
    
    QFontMetrics dateMetrics(dateFont);
    int dateWidth = dateMetrics.horizontalAdvance(dateString);
    
    QRect dateTextRect((width() - dateWidth) / 2, dateRect.top(), dateWidth, dateRect.height());
    
    painter.setFont(dateFont);
    painter.setPen(Qt::black);
    painter.drawText(dateTextRect, Qt::AlignCenter, dateString);
}

void DisplayWidget::drawLunarDate(QPainter &painter)
{
    // Calculate lunar date area
    QRect lunarRect(10, 170, width() - 20, 30);
    
    // Draw lunar date text
    QFont lunarFont("SimSun", 14); // Use Chinese font for lunar date
    QString lunarString = QString("农历: 腊月十六"); // Placeholder, should be calculated from actual lunar calendar
    
    QFontMetrics lunarMetrics(lunarFont);
    int lunarWidth = lunarMetrics.horizontalAdvance(lunarString);
    
    QRect lunarTextRect((width() - lunarWidth) / 2, lunarRect.top(), lunarWidth, lunarRect.height());
    
    painter.setFont(lunarFont);
    painter.setPen(Qt::black);
    painter.drawText(lunarTextRect, Qt::AlignCenter, lunarString);
}

void DisplayWidget::drawWeather(QPainter &painter)
{
    // Calculate weather area
    QRect weatherRect(10, 200, width() - 20, 40);
    
    // Draw weather text
    QFont weatherFont("Arial", 16);
    QString weatherString = QString("天气: 晴 %1°C").arg(displayData.temperature);
    
    QFontMetrics weatherMetrics(weatherFont);
    int weatherWidth = weatherMetrics.horizontalAdvance(weatherString);
    
    QRect weatherTextRect((width() - weatherWidth) / 2, weatherRect.top(), weatherWidth, weatherRect.height());
    
    painter.setFont(weatherFont);
    painter.setPen(Qt::black);
    painter.drawText(weatherTextRect, Qt::AlignCenter, weatherString);
}

void DisplayWidget::drawBattery(QPainter &painter)
{
    // Calculate battery area
    QRect batteryRect(width() - 60, 10, 50, 25);
    
    // Draw battery outline
    painter.setPen(Qt::black);
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(batteryRect);
    
    // Draw battery tip
    QRect batteryTipRect(batteryRect.right() + 2, batteryRect.top() + 5, 3, batteryRect.height() - 10);
    painter.drawRect(batteryTipRect);
    
    // Draw battery level
    int batteryLevel = qBound(0, displayData.batteryLevel, 100);
    int batteryFillWidth = (batteryRect.width() - 4) * batteryLevel / 100;
    QRect batteryFillRect(batteryRect.left() + 2, batteryRect.top() + 2, batteryFillWidth, batteryRect.height() - 4);
    
    if (batteryLevel < 20) {
        painter.setBrush(Qt::red);
    } else if (batteryLevel < 50) {
        painter.setBrush(Qt::yellow);
    } else {
        painter.setBrush(Qt::green);
    }
    
    painter.drawRect(batteryFillRect);
    
    // Draw battery percentage
    QFont batteryFont("Arial", 10);
    QString batteryString = QString("%1%").arg(batteryLevel);
    painter.setFont(batteryFont);
    painter.setPen(Qt::black);
    painter.drawText(batteryRect, Qt::AlignCenter, batteryString);
}

void DisplayWidget::drawWiFi(QPainter &painter)
{
    // Calculate WiFi area
    QRect wifiRect(10, 10, 40, 25);
    
    // Draw WiFi icon based on status
    QFont wifiFont("Arial", 16);
    QString wifiIcon;
    
    switch (displayData.wifiStatus) {
        case WiFiStatus::Connected:
            wifiIcon = "WiFi: ✓";
            painter.setPen(Qt::green);
            break;
        case WiFiStatus::Connecting:
            wifiIcon = "WiFi: ...";
            painter.setPen(Qt::yellow);
            break;
        case WiFiStatus::Disconnected:
        default:
            wifiIcon = "WiFi: ✗";
            painter.setPen(Qt::red);
            break;
    }
    
    painter.setFont(wifiFont);
    painter.drawText(wifiRect, Qt::AlignCenter, wifiIcon);
}

void DisplayWidget::drawTemperature(QPainter &painter)
{
    // Calculate temperature area
    QRect tempRect(10, 50, width() - 20, 40);
    
    // Draw temperature text
    QFont tempFont("Arial", 24, QFont::Bold);
    QString tempString = QString("温度: %1°C").arg(displayData.temperature);
    
    QFontMetrics tempMetrics(tempFont);
    int tempWidth = tempMetrics.horizontalAdvance(tempString);
    
    QRect tempTextRect((width() - tempWidth) / 2, tempRect.top(), tempWidth, tempRect.height());
    
    painter.setFont(tempFont);
    painter.setPen(Qt::black);
    painter.drawText(tempTextRect, Qt::AlignCenter, tempString);
}

void DisplayWidget::drawHumidity(QPainter &painter)
{
    // Calculate humidity area
    QRect humidityRect(10, 100, width() - 20, 40);
    
    // Draw humidity text
    QFont humidityFont("Arial", 24, QFont::Bold);
    QString humidityString = QString("湿度: %1%").arg(displayData.humidity);
    
    QFontMetrics humidityMetrics(humidityFont);
    int humidityWidth = humidityMetrics.horizontalAdvance(humidityString);
    
    QRect humidityTextRect((width() - humidityWidth) / 2, humidityRect.top(), humidityWidth, humidityRect.height());
    
    painter.setFont(humidityFont);
    painter.setPen(Qt::black);
    painter.drawText(humidityTextRect, Qt::AlignCenter, humidityString);
}

void DisplayWidget::drawPressure(QPainter &painter)
{
    // Calculate pressure area
    QRect pressureRect(10, 150, width() - 20, 40);
    
    // Draw pressure text
    QFont pressureFont("Arial", 24, QFont::Bold);
    QString pressureString = QString("气压: %1 hPa").arg(displayData.pressure);
    
    QFontMetrics pressureMetrics(pressureFont);
    int pressureWidth = pressureMetrics.horizontalAdvance(pressureString);
    
    QRect pressureTextRect((width() - pressureWidth) / 2, pressureRect.top(), pressureWidth, pressureRect.height());
    
    painter.setFont(pressureFont);
    painter.setPen(Qt::black);
    painter.drawText(pressureTextRect, Qt::AlignCenter, pressureString);
}

void DisplayWidget::drawLightLevel(QPainter &painter)
{
    // Calculate light level area
    QRect lightRect(10, 200, width() - 20, 40);
    
    // Draw light level text
    QFont lightFont("Arial", 24, QFont::Bold);
    QString lightString = QString("光照: %1 lux").arg(displayData.lightLevel);
    
    QFontMetrics lightMetrics(lightFont);
    int lightWidth = lightMetrics.horizontalAdvance(lightString);
    
    QRect lightTextRect((width() - lightWidth) / 2, lightRect.top(), lightWidth, lightRect.height());
    
    painter.setFont(lightFont);
    painter.setPen(Qt::black);
    painter.drawText(lightTextRect, Qt::AlignCenter, lightString);
}

void DisplayWidget::drawInvertedText(QPainter &painter, const QString &text, const QRect &rect, const QFont &font)
{
    // Draw inverted text (black on white background)
    QFontMetrics metrics(font);
    int textWidth = metrics.horizontalAdvance(text);
    int textHeight = metrics.height();
    
    QRect textRect((rect.width() - textWidth) / 2 + rect.left(), 
                   (rect.height() - textHeight) / 2 + rect.top(), 
                   textWidth, textHeight);
    
    painter.fillRect(textRect, Qt::black);
    painter.setPen(Qt::white);
    painter.setFont(font);
    painter.drawText(textRect, Qt::AlignCenter, text);
}

void DisplayWidget::drawBox(QPainter &painter, const QRect &rect, bool inverted)
{
    if (inverted) {
        painter.fillRect(rect, Qt::black);
    } else {
        painter.fillRect(rect, Qt::white);
    }
    painter.setPen(Qt::black);
    painter.drawRect(rect);
}