#include "error_handling.h"
#include <sstream>
#include <iomanip>
#include <new>  // For std::nothrow

// 静态实例初始化
ErrorHandlingManager* ErrorHandlingManager::instance = nullptr;

// ErrorInfo 构造函数
ErrorInfo::ErrorInfo(
    ErrorLevel lev,
    ErrorType typ,
    const String& msg,
    const String& mod,
    int code,
    const String& det,
    ErrorRecoveryStrategy strategy
) : level(lev), type(typ), message(msg), module(mod), errorCode(code), details(det),
    recoveryStrategy(strategy), retryCount(0), lastRetryTime(0) {
    // 生成错误ID
    unsigned long timestamp = millis();
    std::stringstream ss;
    ss << "ERR_" << timestamp << "_" << random(1000, 9999);
    errorId = ss.str().c_str();
    
    // 设置时间戳
    unsigned long sec = timestamp / 1000;
    unsigned long min = sec / 60;
    unsigned long hour = min / 60;
    unsigned long day = hour / 24;
    
    std::stringstream ts;
    ts << day << "d " << (hour % 24) << ":" << (min % 60) << ":" << (sec % 60);
    this->timestamp = String(ts.str().c_str());
}

// ErrorInfo 方法实现
String ErrorInfo::getErrorId() const {
    return errorId;
}

ErrorLevel ErrorInfo::getLevel() const {
    return level;
}

ErrorType ErrorInfo::getType() const {
    return type;
}

String ErrorInfo::getMessage() const {
    return message;
}

String ErrorInfo::getModule() const {
    return module;
}

int ErrorInfo::getErrorCode() const {
    return errorCode;
}

String ErrorInfo::getTimestamp() const {
    return timestamp;
}

String ErrorInfo::getDetails() const {
    return details;
}

ErrorRecoveryStrategy ErrorInfo::getRecoveryStrategy() const {
    return recoveryStrategy;
}

int ErrorInfo::getRetryCount() const {
    return retryCount;
}

unsigned long ErrorInfo::getLastRetryTime() const {
    return lastRetryTime;
}

void ErrorInfo::incrementRetryCount() {
    retryCount++;
}

void ErrorInfo::updateLastRetryTime() {
    lastRetryTime = millis();
}

String ErrorInfo::toString() const {
    std::stringstream ss;
    ss << "[" << ErrorHandlingManager::errorLevelToString(level) << "] "
       << "[" << ErrorHandlingManager::errorTypeToString(type) << "] "
       << "[" << module << "] "
       << message;
    if (errorCode != 0) {
        ss << " (Code: " << errorCode << ")";
    }
    if (!details.isEmpty()) {
        ss << " Details: " << details;
    }
    return ss.str().c_str();
}

String ErrorInfo::toJson() const {
    std::stringstream ss;
    ss << "{";
    ss << "\"errorId\":\"" << errorId << "\",";
    ss << "\"level\":\"" << ErrorHandlingManager::errorLevelToString(level) << "\",";
    ss << "\"type\":\"" << ErrorHandlingManager::errorTypeToString(type) << "\",";
    ss << "\"message\":\"" << message << "\",";
    ss << "\"module\":\"" << module << "\",";
    ss << "\"errorCode\":" << errorCode << ",";
    ss << "\"timestamp\":\"" << timestamp << "\",";
    ss << "\"details\":\"" << details << "\",";
    ss << "\"recoveryStrategy\":\"" << ErrorHandlingManager::recoveryStrategyToString(recoveryStrategy) << "\",";
    ss << "\"retryCount\":" << retryCount;
    ss << "}";
    return ss.str().c_str();
}

// ErrorHandlingManager 构造函数
ErrorHandlingManager::ErrorHandlingManager() : maxErrorHistorySize(100), initialized(false) {
}

// ErrorHandlingManager 单例获取
ErrorHandlingManager* ErrorHandlingManager::getInstance() {
    if (instance == nullptr) {
        // Security: Use nothrow to handle memory allocation failure gracefully
        instance = new (std::nothrow) ErrorHandlingManager();
        if (instance == nullptr) {
            // Fallback: Critical error - cannot allocate error handler
            // In embedded system, use static allocation or memory pool
            static ErrorHandlingManager staticInstance;
            instance = &staticInstance;
            Serial.println("Error: Failed to allocate ErrorHandlingManager, using static instance");
        }
    }
    return instance;
}

// 初始化
void ErrorHandlingManager::init(size_t maxHistorySize) {
    if (initialized) {
        return;
    }
    
    maxErrorHistorySize = maxHistorySize;
    
    // 注册默认的控制台错误处理器
    auto consoleHandler = std::make_shared<ConsoleErrorHandler>();
    registerHandler(consoleHandler);
    
    initialized = true;
}

// 注册错误处理器
void ErrorHandlingManager::registerHandler(std::shared_ptr<IErrorHandler> handler) {
    handlers.push_back(handler);
}

// 报告错误
void ErrorHandlingManager::reportError(
    ErrorLevel level,
    ErrorType type,
    const String& message,
    const String& module,
    int errorCode,
    const String& details,
    ErrorRecoveryStrategy strategy
) {
    auto error = std::make_shared<ErrorInfo>(level, type, message, module, errorCode, details, strategy);
    handleError(error);
}

// 处理错误
void ErrorHandlingManager::handleError(std::shared_ptr<ErrorInfo> error) {
    // 添加到错误历史
    if (errorHistory.size() >= maxErrorHistorySize) {
        errorHistory.erase(errorHistory.begin());
    }
    errorHistory.push_back(error);
    
    // 调用所有注册的错误处理器
    for (const auto& handler : handlers) {
        handler->handleError(error);
    }
}

// 获取错误历史
std::vector<std::shared_ptr<ErrorInfo>> ErrorHandlingManager::getErrorHistory() const {
    return errorHistory;
}

std::vector<std::shared_ptr<ErrorInfo>> ErrorHandlingManager::getErrorHistory(ErrorLevel minLevel) const {
    std::vector<std::shared_ptr<ErrorInfo>> result;
    for (const auto& error : errorHistory) {
        if (error->getLevel() >= minLevel) {
            result.push_back(error);
        }
    }
    return result;
}

std::vector<std::shared_ptr<ErrorInfo>> ErrorHandlingManager::getErrorHistory(ErrorType type) const {
    std::vector<std::shared_ptr<ErrorInfo>> result;
    for (const auto& error : errorHistory) {
        if (error->getType() == type) {
            result.push_back(error);
        }
    }
    return result;
}

// 清除错误历史
void ErrorHandlingManager::clearErrorHistory() {
    errorHistory.clear();
}

// 检查是否有未处理的严重错误
bool ErrorHandlingManager::hasUnresolvedCriticalErrors() const {
    for (const auto& error : errorHistory) {
        if (error->getLevel() == ERROR_LEVEL_CRITICAL) {
            return true;
        }
    }
    return false;
}

// 获取错误统计
size_t ErrorHandlingManager::getErrorCount(ErrorLevel level) const {
    size_t count = 0;
    for (const auto& error : errorHistory) {
        if (error->getLevel() == level) {
            count++;
        }
    }
    return count;
}

size_t ErrorHandlingManager::getErrorCount(ErrorType type) const {
    size_t count = 0;
    for (const auto& error : errorHistory) {
        if (error->getType() == type) {
            count++;
        }
    }
    return count;
}

// 转换错误级别为字符串
String ErrorHandlingManager::errorLevelToString(ErrorLevel level) {
    switch (level) {
        case ERROR_LEVEL_DEBUG:
            return "DEBUG";
        case ERROR_LEVEL_INFO:
            return "INFO";
        case ERROR_LEVEL_WARNING:
            return "WARNING";
        case ERROR_LEVEL_ERROR:
            return "ERROR";
        case ERROR_LEVEL_CRITICAL:
            return "CRITICAL";
        default:
            return "UNKNOWN";
    }
}

// 转换错误类型为字符串
String ErrorHandlingManager::errorTypeToString(ErrorType type) {
    switch (type) {
        case ERROR_TYPE_SYSTEM:
            return "SYSTEM";
        case ERROR_TYPE_NETWORK:
            return "NETWORK";
        case ERROR_TYPE_SENSOR:
            return "SENSOR";
        case ERROR_TYPE_DISPLAY:
            return "DISPLAY";
        case ERROR_TYPE_STORAGE:
            return "STORAGE";
        case ERROR_TYPE_CONFIG:
            return "CONFIG";
        case ERROR_TYPE_API:
            return "API";
        case ERROR_TYPE_POWER:
            return "POWER";
        case ERROR_TYPE_PERIPHERAL:
            return "PERIPHERAL";
        case ERROR_TYPE_UNKNOWN:
            return "UNKNOWN";
        default:
            return "UNKNOWN";
    }
}

// 转换恢复策略为字符串
String ErrorHandlingManager::recoveryStrategyToString(ErrorRecoveryStrategy strategy) {
    switch (strategy) {
        case RECOVERY_STRATEGY_IGNORE:
            return "IGNORE";
        case RECOVERY_STRATEGY_RETRY:
            return "RETRY";
        case RECOVERY_STRATEGY_RESET:
            return "RESET";
        case RECOVERY_STRATEGY_FALLBACK:
            return "FALLBACK";
        case RECOVERY_STRATEGY_SHUTDOWN:
            return "SHUTDOWN";
        default:
            return "UNKNOWN";
    }
}

// ConsoleErrorHandler 构造函数
ConsoleErrorHandler::ConsoleErrorHandler(ErrorLevel minLevel) : minLogLevel(minLevel) {
}

// ConsoleErrorHandler 方法实现
void ConsoleErrorHandler::handleError(std::shared_ptr<ErrorInfo> error) {
    if (error->getLevel() >= minLogLevel) {
        logError(error);
    }
    recoverFromError(error);
}

void ConsoleErrorHandler::logError(std::shared_ptr<ErrorInfo> error) {
    String errorStr = error->toString();
    
    switch (error->getLevel()) {
        case ERROR_LEVEL_DEBUG:
            Serial.print("[DEBUG] ");
            break;
        case ERROR_LEVEL_INFO:
            Serial.print("[INFO] ");
            break;
        case ERROR_LEVEL_WARNING:
            Serial.print("[WARNING] ");
            break;
        case ERROR_LEVEL_ERROR:
            Serial.print("[ERROR] ");
            break;
        case ERROR_LEVEL_CRITICAL:
            Serial.print("[CRITICAL] ");
            break;
    }
    
    Serial.println(errorStr);
}

void ConsoleErrorHandler::recoverFromError(std::shared_ptr<ErrorInfo> error) {
    switch (error->getRecoveryStrategy()) {
        case RECOVERY_STRATEGY_RETRY:
            // 简单的重试逻辑
            if (error->getRetryCount() < 3) {
                error->incrementRetryCount();
                error->updateLastRetryTime();
                Serial.printf("[RECOVERY] Retrying operation for error: %s (Attempt %d/3)\n", 
                    error->getErrorId().c_str(), error->getRetryCount());
            }
            break;
        case RECOVERY_STRATEGY_RESET:
            Serial.printf("[RECOVERY] Resetting system due to error: %s\n", 
                error->getErrorId().c_str());
            // 这里可以添加系统重置逻辑
            break;
        case RECOVERY_STRATEGY_SHUTDOWN:
            Serial.printf("[RECOVERY] Shutting down system due to critical error: %s\n", 
                error->getErrorId().c_str());
            // 这里可以添加系统关机逻辑
            break;
        case RECOVERY_STRATEGY_FALLBACK:
            Serial.printf("[RECOVERY] Switching to fallback mode due to error: %s\n", 
                error->getErrorId().c_str());
            // 这里可以添加回退到备用模式的逻辑
            break;
        case RECOVERY_STRATEGY_IGNORE:
        default:
            // 忽略错误，不做任何处理
            break;
    }
}

// FileErrorHandler 构造函数
FileErrorHandler::FileErrorHandler(const String& fileName, ErrorLevel minLevel, size_t maxSize) 
    : logFileName(fileName), minLogLevel(minLevel), maxLogFileSize(maxSize) {
}

// FileErrorHandler 方法实现
void FileErrorHandler::handleError(std::shared_ptr<ErrorInfo> error) {
    if (error->getLevel() >= minLogLevel) {
        logError(error);
    }
    recoverFromError(error);
}

void FileErrorHandler::logError(std::shared_ptr<ErrorInfo> error) {
    // 检查日志文件大小
    rotateLogFile();
    
    // 这里可以添加文件写入逻辑
    // 注意：在Arduino环境中，需要使用SD库或SPIFFS库来操作文件
    String errorJson = error->toJson();
    Serial.printf("[FILE_LOG] Writing error to log file: %s\n", errorJson.c_str());
}

void FileErrorHandler::recoverFromError(std::shared_ptr<ErrorInfo> error) {
    // 与ConsoleErrorHandler相同的恢复逻辑
    switch (error->getRecoveryStrategy()) {
        case RECOVERY_STRATEGY_RETRY:
            if (error->getRetryCount() < 3) {
                error->incrementRetryCount();
                error->updateLastRetryTime();
                Serial.printf("[RECOVERY] Retrying operation for error: %s (Attempt %d/3)\n", 
                    error->getErrorId().c_str(), error->getRetryCount());
            }
            break;
        case RECOVERY_STRATEGY_RESET:
            Serial.printf("[RECOVERY] Resetting system due to error: %s\n", 
                error->getErrorId().c_str());
            break;
        case RECOVERY_STRATEGY_SHUTDOWN:
            Serial.printf("[RECOVERY] Shutting down system due to critical error: %s\n", 
                error->getErrorId().c_str());
            break;
        case RECOVERY_STRATEGY_FALLBACK:
            Serial.printf("[RECOVERY] Switching to fallback mode due to error: %s\n", 
                error->getErrorId().c_str());
            break;
        case RECOVERY_STRATEGY_IGNORE:
        default:
            break;
    }
}

void FileErrorHandler::rotateLogFile() {
    // 这里可以添加日志文件轮换逻辑
    // 注意：在Arduino环境中，需要使用SD库或SPIFFS库来操作文件
    Serial.println("[FILE_LOG] Checking log file size...");
}
