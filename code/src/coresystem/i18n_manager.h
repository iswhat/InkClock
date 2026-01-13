#ifndef I18N_MANAGER_H
#define I18N_MANAGER_H

#ifdef ARDUINO
#include <Arduino.h>
#else
#include "arduino_compat.h"
#endif

#include <string>
#include <memory>
#include <map>
#include <vector>

// 语言代码枚举
enum LanguageCode {
    LANG_EN,    // 英语
    LANG_ZH_CN, // 简体中文
    LANG_ZH_TW, // 繁体中文
    LANG_ES,    // 西班牙语
    LANG_FR,    // 法语
    LANG_DE,    // 德语
    LANG_IT,    // 意大利语
    LANG_JA,    // 日语
    LANG_KO,    // 韩语
    LANG_RU,    // 俄语
    LANG_UNKNOWN
};

// 语言包接口
class ILanguagePack {
public:
    virtual ~ILanguagePack() {}
    virtual LanguageCode getLanguageCode() const = 0;
    virtual String getLanguageName() const = 0;
    virtual String getLanguageNativeName() const = 0;
    virtual String getText(const String& key) const = 0;
    virtual bool hasText(const String& key) const = 0;
    virtual std::vector<String> getKeys() const = 0;
};

// 基础语言包类
class BaseLanguagePack : public ILanguagePack {
private:
    LanguageCode languageCode;
    String languageName;
    String languageNativeName;
    std::map<String, String> translations;

public:
    BaseLanguagePack(
        LanguageCode code,
        const String& name,
        const String& nativeName
    );

    // 实现接口方法
    LanguageCode getLanguageCode() const override;
    String getLanguageName() const override;
    String getLanguageNativeName() const override;
    String getText(const String& key) const override;
    bool hasText(const String& key) const override;
    std::vector<String> getKeys() const override;

    // 添加翻译
    void addTranslation(const String& key, const String& value);

    // 批量添加翻译
    void addTranslations(const std::map<String, String>& trans);
};

// 英语语言包
class EnglishLanguagePack : public BaseLanguagePack {
public:
    EnglishLanguagePack();
};

// 简体中文语言包
class ChineseSimplifiedLanguagePack : public BaseLanguagePack {
public:
    ChineseSimplifiedLanguagePack();
};

// 繁体中文语言包
class ChineseTraditionalLanguagePack : public BaseLanguagePack {
public:
    ChineseTraditionalLanguagePack();
};

// 国际化管理器类
class I18NManager {
private:
    static I18NManager* instance;
    std::map<LanguageCode, std::shared_ptr<ILanguagePack>> languagePacks;
    LanguageCode currentLanguage;
    std::shared_ptr<ILanguagePack> fallbackLanguagePack;
    bool initialized;

    I18NManager();

public:
    static I18NManager* getInstance();

    // 初始化
    bool init();

    // 注册语言包
    bool registerLanguagePack(std::shared_ptr<ILanguagePack> languagePack);

    // 设置当前语言
    bool setLanguage(LanguageCode language);

    // 获取当前语言
    LanguageCode getCurrentLanguage() const;

    // 获取当前语言名称
    String getCurrentLanguageName() const;

    // 获取当前语言原生名称
    String getCurrentLanguageNativeName() const;

    // 获取所有支持的语言
    std::vector<LanguageCode> getSupportedLanguages() const;

    // 获取语言名称
    String getLanguageName(LanguageCode language) const;

    // 获取语言原生名称
    String getLanguageNativeName(LanguageCode language) const;

    // 翻译文本
    String translate(const String& key, const String& defaultValue = "") const;

    // 检查是否有翻译
    bool hasTranslation(const String& key) const;

    // 格式化翻译文本（支持占位符）
    String format(const String& key, const std::vector<String>& params, const String& defaultValue = "") const;
    String format(const String& key, const String& param1, const String& defaultValue = "") const;
    String format(const String& key, const String& param1, const String& param2, const String& defaultValue = "") const;
    String format(const String& key, const String& param1, const String& param2, const String& param3, const String& defaultValue = "") const;

    // 导出翻译为JSON
    String exportTranslations(LanguageCode language) const;

    // 从JSON导入翻译
    bool importTranslations(LanguageCode language, const String& json);
};

// 国际化宏
#define _(key) I18NManager::getInstance()->translate(key)
#define _F(key, ...) I18NManager::getInstance()->format(key, {__VA_ARGS__})
#define _L(key, lang) I18NManager::getInstance()->translate(key)

#endif // I18N_MANAGER_H