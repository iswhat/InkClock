#ifndef MOCKNETWORK_H
#define MOCKNETWORK_H

#include <QObject>
#include <QString>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include "inkclocktypes.h"

class MockNetwork : public QObject
{
    Q_OBJECT

public:
    explicit MockNetwork(QObject *parent = nullptr);
    ~MockNetwork();
    
    // WiFi connection methods
    void connectToWiFi(const QString &ssid, const QString &password);
    void disconnectWiFi();
    WiFiStatus getWiFiStatus() const;
    
    // Network information
    QString getSSID() const;
    int getSignalStrength() const;
    bool hasInternetAccess() const;
    
    // HTTP request methods
    void get(const QUrl &url);
    void post(const QUrl &url, const QByteArray &data);
    void put(const QUrl &url, const QByteArray &data);
    void deleteResource(const QUrl &url);
    
    // MQTT methods (simplified)
    void connectToMQTT(const QString &host, int port, const QString &clientId);
    void disconnectMQTT();
    void publishMQTT(const QString &topic, const QByteArray &message);
    void subscribeMQTT(const QString &topic);
    void unsubscribeMQTT(const QString &topic);
    
    // Network configuration
    void setStaticIP(const QString &ip, const QString &netmask, const QString &gateway, const QString &dns);
    void useDHCP();
    
    // Set network delay for testing
    void setNetworkDelay(int delayMs);
    
    // Set response for testing
    void setMockResponse(const QByteArray &response, int statusCode = 200);
    void setMockError(QNetworkReply::NetworkError error, const QString &errorString);

signals:
    // WiFi signals
    void wifiConnected();
    void wifiDisconnected();
    void wifiStatusChanged(WiFiStatus status);
    void signalStrengthChanged(int strength);
    
    // HTTP signals
    void httpReplyReceived(const QByteArray &data, int statusCode);
    void httpError(QNetworkReply::NetworkError error, const QString &errorString);
    
    // MQTT signals
    void mqttConnected();
    void mqttDisconnected();
    void mqttMessageReceived(const QString &topic, const QByteArray &message);
    void mqttError(const QString &errorString);

private slots:
    // HTTP reply handling
    void onNetworkReplyFinished(QNetworkReply *reply);
    
    // Simulated network response
    void simulateHttpResponse();

private:
    // WiFi properties
    WiFiStatus wifiStatus;
    QString ssid;
    int signalStrength;
    bool internetAccess;
    
    // Network delay for testing
    int networkDelay;
    
    // Mock HTTP response
    QByteArray mockResponse;
    int mockStatusCode;
    bool useMockResponse;
    
    // Mock HTTP error
    QNetworkReply::NetworkError mockError;
    QString mockErrorString;
    bool useMockError;
    
    // Network access manager
    QNetworkAccessManager *networkManager;
    
    // MQTT properties
    bool mqttConnected;
    QString mqttHost;
    int mqttPort;
    QString mqttClientId;
};

#endif // MOCKNETWORK_H