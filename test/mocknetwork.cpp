#include "mocknetwork.h"
#include <QTimer>
#include <QNetworkRequest>

MockNetwork::MockNetwork(QObject *parent)
    : QObject(parent),
      wifiStatus(WiFiStatus::Disconnected),
      ssid(""),
      signalStrength(0),
      internetAccess(false),
      networkDelay(100),
      mockStatusCode(200),
      useMockResponse(false),
      mockError(QNetworkReply::NoError),
      useMockError(false),
      networkManager(new QNetworkAccessManager(this)),
      mqttConnected(false),
      mqttPort(1883)
{
    // Connect network manager signals
    connect(networkManager, &QNetworkAccessManager::finished, this, &MockNetwork::onNetworkReplyFinished);
}

MockNetwork::~MockNetwork()
{
    delete networkManager;
}

// WiFi connection methods
void MockNetwork::connectToWiFi(const QString &ssid, const QString &password)
{
    Q_UNUSED(password);
    
    // Simulate WiFi connection process
    this->ssid = ssid;
    
    // First change to connecting status
    wifiStatus = WiFiStatus::Connecting;
    emit wifiStatusChanged(wifiStatus);
    
    // Simulate connection delay
    QTimer::singleShot(1500, this, [this]() {
        // Change to connected status
        wifiStatus = WiFiStatus::Connected;
        signalStrength = 80;
        internetAccess = true;
        
        emit wifiConnected();
        emit wifiStatusChanged(wifiStatus);
        emit signalStrengthChanged(signalStrength);
    });
}

void MockNetwork::disconnectWiFi()
{
    // Simulate WiFi disconnection
    wifiStatus = WiFiStatus::Disconnected;
    signalStrength = 0;
    internetAccess = false;
    
    emit wifiDisconnected();
    emit wifiStatusChanged(wifiStatus);
    emit signalStrengthChanged(signalStrength);
}

WiFiStatus MockNetwork::getWiFiStatus() const
{
    return wifiStatus;
}

// Network information
QString MockNetwork::getSSID() const
{
    return ssid;
}

int MockNetwork::getSignalStrength() const
{
    return signalStrength;
}

bool MockNetwork::hasInternetAccess() const
{
    return internetAccess;
}

// HTTP request methods
void MockNetwork::get(const QUrl &url)
{
    Q_UNUSED(url);
    
    if (useMockResponse || useMockError) {
        // Use mock response or error
        QTimer::singleShot(networkDelay, this, &MockNetwork::simulateHttpResponse);
    } else {
        // Use real network request
        networkManager->get(QNetworkRequest(url));
    }
}

void MockNetwork::post(const QUrl &url, const QByteArray &data)
{
    Q_UNUSED(url);
    Q_UNUSED(data);
    
    if (useMockResponse || useMockError) {
        // Use mock response or error
        QTimer::singleShot(networkDelay, this, &MockNetwork::simulateHttpResponse);
    } else {
        // Use real network request
        QNetworkRequest request(url);
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        networkManager->post(request, data);
    }
}

void MockNetwork::put(const QUrl &url, const QByteArray &data)
{
    Q_UNUSED(url);
    Q_UNUSED(data);
    
    if (useMockResponse || useMockError) {
        // Use mock response or error
        QTimer::singleShot(networkDelay, this, &MockNetwork::simulateHttpResponse);
    } else {
        // Use real network request
        QNetworkRequest request(url);
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        networkManager->put(request, data);
    }
}

void MockNetwork::deleteResource(const QUrl &url)
{
    Q_UNUSED(url);
    
    if (useMockResponse || useMockError) {
        // Use mock response or error
        QTimer::singleShot(networkDelay, this, &MockNetwork::simulateHttpResponse);
    } else {
        // Use real network request
        networkManager->deleteResource(QNetworkRequest(url));
    }
}

// MQTT methods (simplified)
void MockNetwork::connectToMQTT(const QString &host, int port, const QString &clientId)
{
    mqttHost = host;
    mqttPort = port;
    mqttClientId = clientId;
    
    // Simulate MQTT connection
    QTimer::singleShot(1000, this, [this]() {
        mqttConnected = true;
        emit mqttConnected();
    });
}

void MockNetwork::disconnectMQTT()
{
    // Simulate MQTT disconnection
    mqttConnected = false;
    emit mqttDisconnected();
}

void MockNetwork::publishMQTT(const QString &topic, const QByteArray &message)
{
    Q_UNUSED(topic);
    Q_UNUSED(message);
    
    // Simulate MQTT publish
    if (mqttConnected) {
        // In a real implementation, this would send the message to the MQTT broker
    }
}

void MockNetwork::subscribeMQTT(const QString &topic)
{
    Q_UNUSED(topic);
    
    // Simulate MQTT subscribe
    if (mqttConnected) {
        // In a real implementation, this would subscribe to the topic on the MQTT broker
    }
}

void MockNetwork::unsubscribeMQTT(const QString &topic)
{
    Q_UNUSED(topic);
    
    // Simulate MQTT unsubscribe
    if (mqttConnected) {
        // In a real implementation, this would unsubscribe from the topic on the MQTT broker
    }
}

// Network configuration
void MockNetwork::setStaticIP(const QString &ip, const QString &netmask, const QString &gateway, const QString &dns)
{
    Q_UNUSED(ip);
    Q_UNUSED(netmask);
    Q_UNUSED(gateway);
    Q_UNUSED(dns);
    
    // Simulate static IP configuration
    // In a real implementation, this would configure the network interface
}

void MockNetwork::useDHCP()
{
    // Simulate DHCP configuration
    // In a real implementation, this would configure the network interface to use DHCP
}

// Set network delay for testing
void MockNetwork::setNetworkDelay(int delayMs)
{
    networkDelay = delayMs;
}

// Set response for testing
void MockNetwork::setMockResponse(const QByteArray &response, int statusCode)
{
    mockResponse = response;
    mockStatusCode = statusCode;
    useMockResponse = true;
    useMockError = false;
}

void MockNetwork::setMockError(QNetworkReply::NetworkError error, const QString &errorString)
{
    mockError = error;
    mockErrorString = errorString;
    useMockError = true;
    useMockResponse = false;
}

// HTTP reply handling
void MockNetwork::onNetworkReplyFinished(QNetworkReply *reply)
{
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray data = reply->readAll();
        int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        emit httpReplyReceived(data, statusCode);
    } else {
        emit httpError(reply->error(), reply->errorString());
    }
    
    reply->deleteLater();
}

// Simulated network response
void MockNetwork::simulateHttpResponse()
{
    if (useMockError) {
        emit httpError(mockError, mockErrorString);
        useMockError = false;
    } else if (useMockResponse) {
        emit httpReplyReceived(mockResponse, mockStatusCode);
        useMockResponse = false;
    }
}
