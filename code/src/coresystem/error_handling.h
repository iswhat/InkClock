#ifndef ERROR_HANDLING_H
#define ERROR_HANDLING_H

#ifdef ARDUINO
#include <Arduino.h>
#else
#include "arduino_compat.h"
#endif

#include <string>
#include <memory>
#include <vector>

// 错误级别枚举
enum ErrorLevel {
    ERROR_LEVEL_DEBUG,
    ERROR_LEVEL_INFO,
    ERROR_LEVEL_WARNING,
    ERROR_LEVEL_ERROR,
    ERROR_LEVEL_CRITICAL
};

// 错误类型枚举
enum ErrorType {
    ERROR_TYPE_SYSTEM,
    ERROR_TYPE_NETWORK,
    ERROR_TYPE_SENSOR,
    ERROR_TYPE_DISPLAY,
    ERROR_TYPE_STORAGE,
    ERROR_TYPE_CONFIG,
    ERROR_TYPE_API,
    ERROR_TYPE_POWER,
    ERROR_TYPE_PERIPHERAL,
    ERROR_TYPE_UNKNOWN
};

// 错误恢复策略枚举
enum ErrorRecoveryStrategy {
    RECOVERY_STRATEGY_IGNORE,
    RECOVERY_STRATEGY_RETRY,
    RECOVERY_STRATEGY_RESET,
    RECOVERY_STRATEGY_FALLBACK,
    RECOVERY_STRATEGY_SHUTDOWN
};

// 错误信息类
class ErrorInfo {
private:
    String errorId;
    ErrorLevel level;
    ErrorType type;
    String message;
    String module;
    int errorCode;
    String timestamp;
    String details;
    ErrorRecoveryStrategy recoveryStrategy;
    int retryCount;
    unsigned long lastRetryTime;

public:
    ErrorInfo(
        ErrorLevel lev,
        ErrorType typ,
        const String& msg,
        const String& mod,
        int code = 0,
        const String& det = "",
        ErrorRecoveryStrategy strategy = RECOVERY_STRATEGY_IGNORE
    );

    // 获取错误信息
    String getErrorId() const;
    ErrorLevel getLevel() const;
    ErrorType getType() const;
    String getMessage() const;
    String getModule() const;
    int getErrorCode() const;
    String getTimestamp() const;
    String getDetails() const;
    ErrorRecoveryStrategy getRecoveryStrategy() const;
    int getRetryCount() const;
    unsigned long getLastRetryTime() const;

    // 更新重试信息
    void incrementRetryCount();
    void updateLastRetryTime();

    // 转换为字符串
    String toString() const;
    String toJson() const;
};

// 错误处理器接口
class IErrorHandler {
public:
    virtual ~IErrorHandler() {}
    virtual void handleError(std::shared_ptr<ErrorInfo> error) = 0;
    virtual void logError(std::shared_ptr<ErrorInfo> error) = 0;
    virtual void recoverFromError(std::shared_ptr<ErrorInfo> error) = 0;
};

// 错误处理管理器类
class ErrorHandlingManager {
private:
    static ErrorHandlingManager* instance;
    std::vector<std::shared_ptr<IErrorHandler>> handlers;
    std::vector<std::shared_ptr<ErrorInfo>> errorHistory;
    size_t maxErrorHistorySize;
    bool initialized;

    ErrorHandlingManager();

public:
    static ErrorHandlingManager* getInstance();

    // 初始化
    void init(size_t maxHistorySize = 100);

    // 注册错误处理器
    void registerHandler(std::shared_ptr<IErrorHandler> handler);

    // 报告错误
    void reportError(
        ErrorLevel level,
        ErrorType type,
        const String& message,
        const String& module,
        int errorCode = 0,
        const String& details = "",
        ErrorRecoveryStrategy strategy = RECOVERY_STRATEGY_IGNORE
    );

    // 处理错误
    void handleError(std::shared_ptr<ErrorInfo> error);

    // 获取错误历史
    std::vector<std::shared_ptr<ErrorInfo>> getErrorHistory() const;
    std::vector<std::shared_ptr<ErrorInfo>> getErrorHistory(ErrorLevel minLevel) const;
    std::vector<std::shared_ptr<ErrorInfo>> getErrorHistory(ErrorType type) const;

    // 清除错误历史
    void clearErrorHistory();

    // 检查是否有未处理的严重错误
    bool hasUnresolvedCriticalErrors() const;

    // 获取错误统计
    size_t getErrorCount(ErrorLevel level) const;
    size_t getErrorCount(ErrorType type) const;

    // 转换错误级别为字符串
    static String errorLevelToString(ErrorLevel level);

    // 转换错误类型为字符串
    static String errorTypeToString(ErrorType type);

    // 转换恢复策略为字符串
    static String recoveryStrategyToString(ErrorRecoveryStrategy strategy);
};

// 控制台错误处理器类
class ConsoleErrorHandler : public IErrorHandler {
private:
    ErrorLevel minLogLevel;

public:
    ConsoleErrorHandler(ErrorLevel minLevel = ERROR_LEVEL_DEBUG);
    void handleError(std::shared_ptr<ErrorInfo> error) override;
    void logError(std::shared_ptr<ErrorInfo> error) override;
    void recoverFromError(std::shared_ptr<ErrorInfo> error) override;
};

// 文件错误处理器类
class FileErrorHandler : public IErrorHandler {
private:
    String logFileName;
    ErrorLevel minLogLevel;
    size_t maxLogFileSize;

public:
    FileErrorHandler(
        const String& fileName = "/error_log.txt",
        ErrorLevel minLevel = ERROR_LEVEL_INFO,
        size_t maxSize = 1024 * 1024 // 1MB
    );
    void handleError(std::shared_ptr<ErrorInfo> error) override;
    void logError(std::shared_ptr<ErrorInfo> error) override;
    void recoverFromError(std::shared_ptr<ErrorInfo> error) override;

private:
    void rotateLogFile();
};

// 错误处理宏
#define ERROR_DEBUG(type, message, module, code, details) \
    ErrorHandlingManager::getInstance()->reportError(ERROR_LEVEL_DEBUG, type, message, module, code, details)

#define ERROR_INFO(type, message, module, code, details) \
    ErrorHandlingManager::getInstance()->reportError(ERROR_LEVEL_INFO, type, message, module, code, details)

#define ERROR_WARNING(type, message, module, code, details) \
    ErrorHandlingManager::getInstance()->reportError(ERROR_LEVEL_WARNING, type, message, module, code, details)

#define ERROR_ERROR(type, message, module, code, details) \
    ErrorHandlingManager::getInstance()->reportError(ERROR_LEVEL_ERROR, type, message, module, code, details)

#define ERROR_CRITICAL(type, message, module, code, details) \
    ErrorHandlingManager::getInstance()->reportError(ERROR_LEVEL_CRITICAL, type, message, module, code, details)

#endif // ERROR_HANDLING_H