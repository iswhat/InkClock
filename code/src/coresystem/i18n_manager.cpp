#include "i18n_manager.h"
#include <sstream>

// 静态实例初始化
I18NManager* I18NManager::instance = nullptr;

// BaseLanguagePack 构造函数
BaseLanguagePack::BaseLanguagePack(
    LanguageCode code,
    const String& name,
    const String& nativeName
) : languageCode(code), languageName(name), languageNativeName(nativeName) {
}

// BaseLanguagePack 方法实现
LanguageCode BaseLanguagePack::getLanguageCode() const {
    return languageCode;
}

String BaseLanguagePack::getLanguageName() const {
    return languageName;
}

String BaseLanguagePack::getLanguageNativeName() const {
    return languageNativeName;
}

String BaseLanguagePack::getText(const String& key) const {
    auto it = translations.find(key);
    if (it != translations.end()) {
        return it->second;
    }
    return "";
}

bool BaseLanguagePack::hasText(const String& key) const {
    return translations.find(key) != translations.end();
}

std::vector<String> BaseLanguagePack::getKeys() const {
    std::vector<String> keys;
    for (const auto& pair : translations) {
        keys.push_back(pair.first);
    }
    return keys;
}

void BaseLanguagePack::addTranslation(const String& key, const String& value) {
    translations[key] = value;
}

void BaseLanguagePack::addTranslations(const std::map<String, String>& trans) {
    for (const auto& pair : trans) {
        translations[pair.first] = pair.second;
    }
}

// EnglishLanguagePack 构造函数
EnglishLanguagePack::EnglishLanguagePack() : BaseLanguagePack(LANG_EN, "English", "English") {
    // 系统相关
    addTranslation("system.title", "InkClock");
    addTranslation("system.version", "Version");
    addTranslation("system.uptime", "Uptime");
    addTranslation("system.restart", "Restart");
    addTranslation("system.shutdown", "Shutdown");
    addTranslation("system.settings", "Settings");
    addTranslation("system.status", "Status");
    
    // 网络相关
    addTranslation("network.title", "Network");
    addTranslation("network.wifi", "WiFi");
    addTranslation("network.connected", "Connected");
    addTranslation("network.disconnected", "Disconnected");
    addTranslation("network.ip_address", "IP Address");
    addTranslation("network.signal_strength", "Signal Strength");
    
    // 时间相关
    addTranslation("time.title", "Time");
    addTranslation("time.date", "Date");
    addTranslation("time.time", "Time");
    addTranslation("time.timezone", "Timezone");
    addTranslation("time.sync", "Sync");
    addTranslation("time.synced", "Synced");
    addTranslation("time.format", "Format");
    addTranslation("time.24h", "24-hour");
    addTranslation("time.12h", "12-hour");
    
    // 显示相关
    addTranslation("display.title", "Display");
    addTranslation("display.brightness", "Brightness");
    addTranslation("display.rotation", "Rotation");
    addTranslation("display.update_interval", "Update Interval");
    addTranslation("display.clock_mode", "Clock Mode");
    addTranslation("display.digital", "Digital");
    addTranslation("display.analog", "Analog");
    addTranslation("display.text", "Text");
    
    // 传感器相关
    addTranslation("sensor.title", "Sensors");
    addTranslation("sensor.temperature", "Temperature");
    addTranslation("sensor.humidity", "Humidity");
    addTranslation("sensor.pressure", "Pressure");
    addTranslation("sensor.altitude", "Altitude");
    addTranslation("sensor.light", "Light");
    addTranslation("sensor.co2", "CO2");
    addTranslation("sensor.voc", "VOC");
    addTranslation("sensor.update_interval", "Update Interval");
    
    // 天气相关
    addTranslation("weather.title", "Weather");
    addTranslation("weather.temperature", "Temperature");
    addTranslation("weather.humidity", "Humidity");
    addTranslation("weather.pressure", "Pressure");
    addTranslation("weather.wind_speed", "Wind Speed");
    addTranslation("weather.wind_direction", "Wind Direction");
    addTranslation("weather.rainfall", "Rainfall");
    addTranslation("weather.uv_index", "UV Index");
    addTranslation("weather.air_quality", "Air Quality");
    addTranslation("weather.update_interval", "Update Interval");
    
    // 消息相关
    addTranslation("message.title", "Messages");
    addTranslation("message.new", "New Message");
    addTranslation("message.unread", "Unread");
    addTranslation("message.read", "Read");
    addTranslation("message.delete", "Delete");
    addTranslation("message.clear", "Clear All");
    addTranslation("message.priority", "Priority");
    addTranslation("message.normal", "Normal");
    addTranslation("message.high", "High");
    addTranslation("message.critical", "Critical");
    
    // 电源相关
    addTranslation("power.title", "Power");
    addTranslation("power.battery", "Battery");
    addTranslation("power.charging", "Charging");
    addTranslation("power.full", "Full");
    addTranslation("power.low", "Low");
    addTranslation("power.shutdown", "Shutdown");
    addTranslation("power.restart", "Restart");
    
    // 配置相关
    addTranslation("config.title", "Configuration");
    addTranslation("config.save", "Save");
    addTranslation("config.load", "Load");
    addTranslation("config.reset", "Reset");
    addTranslation("config.backup", "Backup");
    addTranslation("config.restore", "Restore");
    
    // 错误相关
    addTranslation("error.title", "Error");
    addTranslation("error.network", "Network Error");
    addTranslation("error.sensor", "Sensor Error");
    addTranslation("error.display", "Display Error");
    addTranslation("error.storage", "Storage Error");
    addTranslation("error.config", "Configuration Error");
    addTranslation("error.api", "API Error");
    addTranslation("error.power", "Power Error");
    
    // 插件相关
    addTranslation("plugin.title", "Plugins");
    addTranslation("plugin.loaded", "Loaded");
    addTranslation("plugin.unloaded", "Unloaded");
    addTranslation("plugin.enable", "Enable");
    addTranslation("plugin.disable", "Disable");
    addTranslation("plugin.install", "Install");
    addTranslation("plugin.uninstall", "Uninstall");
    
    // 远程控制相关
    addTranslation("remote.title", "Remote Control");
    addTranslation("remote.enable", "Enable Remote Control");
    addTranslation("remote.port", "Port");
    addTranslation("remote.security", "Security");
    addTranslation("remote.api_key", "API Key");
    
    // 通用
    addTranslation("common.ok", "OK");
    addTranslation("common.cancel", "Cancel");
    addTranslation("common.save", "Save");
    addTranslation("common.delete", "Delete");
    addTranslation("common.edit", "Edit");
    addTranslation("common.add", "Add");
    addTranslation("common.remove", "Remove");
    addTranslation("common.up", "Up");
    addTranslation("common.down", "Down");
    addTranslation("common.left", "Left");
    addTranslation("common.right", "Right");
    addTranslation("common.on", "On");
    addTranslation("common.off", "Off");
    addTranslation("common.enabled", "Enabled");
    addTranslation("common.disabled", "Disabled");
    addTranslation("common.success", "Success");
    addTranslation("common.failure", "Failure");
    addTranslation("common.warning", "Warning");
    addTranslation("common.error", "Error");
    addTranslation("common.info", "Info");
    addTranslation("common.confirm", "Confirm");
    addTranslation("common.back", "Back");
    addTranslation("common.next", "Next");
    addTranslation("common.previous", "Previous");
    addTranslation("common.first", "First");
    addTranslation("common.last", "Last");
    addTranslation("common.all", "All");
    addTranslation("common.none", "None");
    addTranslation("common.default", "Default");
    addTranslation("common.custom", "Custom");
    addTranslation("common.auto", "Auto");
    addTranslation("common.manual", "Manual");
    addTranslation("common.system", "System");
    addTranslation("common.user", "User");
    addTranslation("common.application", "Application");
    addTranslation("common.device", "Device");
    addTranslation("common.network", "Network");
    addTranslation("common.internet", "Internet");
    addTranslation("common.local", "Local");
    addTranslation("common.remote", "Remote");
    addTranslation("common.wireless", "Wireless");
    addTranslation("common.wired", "Wired");
    addTranslation("common.serial", "Serial");
    addTranslation("common.usb", "USB");
    addTranslation("common.bluetooth", "Bluetooth");
    addTranslation("common.wifi", "WiFi");
    addTranslation("common.ethernet", "Ethernet");
    addTranslation("common.battery", "Battery");
    addTranslation("common.ac_power", "AC Power");
    addTranslation("common.dc_power", "DC Power");
    addTranslation("common.solar", "Solar");
    addTranslation("common.wind", "Wind");
    addTranslation("common.water", "Water");
    addTranslation("common.thermal", "Thermal");
    addTranslation("common.geothermal", "Geothermal");
    addTranslation("common.biomass", "Biomass");
    addTranslation("common.nuclear", "Nuclear");
    addTranslation("common.fossil", "Fossil");
    addTranslation("common.renewable", "Renewable");
    addTranslation("common.non_renewable", "Non-Renewable");
    addTranslation("common.sustainable", "Sustainable");
    addTranslation("common.unsustainable", "Unsustainable");
    addTranslation("common.efficient", "Efficient");
    addTranslation("common.inefficient", "Inefficient");
    addTranslation("common.economic", "Economic");
    addTranslation("common.uneconomic", "Uneconomic");
    addTranslation("common.environmental", "Environmental");
    addTranslation("common.nonenvironmental", "Non-Environmental");
    addTranslation("common.social", "Social");
    addTranslation("common.nonsocial", "Non-Social");
    addTranslation("common.governance", "Governance");
    addTranslation("common.nongovernance", "Non-Governance");
    addTranslation("common.technical", "Technical");
    addTranslation("common.nontechnical", "Non-Technical");
    addTranslation("common.scientific", "Scientific");
    addTranslation("common.nonscientific", "Non-Scientific");
    addTranslation("common.artistic", "Artistic");
    addTranslation("common.nonartistic", "Non-Artistic");
    addTranslation("common.creative", "Creative");
    addTranslation("common.noncreative", "Non-Creative");
    addTranslation("common.innovative", "Innovative");
    addTranslation("common.noninnovative", "Non-Innovative");
    addTranslation("common.traditional", "Traditional");
    addTranslation("common.nontraditional", "Non-Traditional");
    addTranslation("common.modern", "Modern");
    addTranslation("common.nonmodern", "Non-Modern");
    addTranslation("common.classic", "Classic");
    addTranslation("common.nonclassic", "Non-Classic");
    addTranslation("common.vintage", "Vintage");
    addTranslation("common.nonvintage", "Non-Vintage");
    addTranslation("common.antique", "Antique");
    addTranslation("common.nonantique", "Non-Antique");
    addTranslation("common.retro", "Retro");
    addTranslation("common.nonretro", "Non-Retro");
    addTranslation("common.futuristic", "Futuristic");
    addTranslation("common.nonfuturistic", "Non-Futuristic");
    addTranslation("common.minimalist", "Minimalist");
    addTranslation("common.nonminimalist", "Non-Minimalist");
    addTranslation("common.maximalist", "Maximalist");
    addTranslation("common.nonmaximalist", "Non-Maximalist");
    addTranslation("common.simple", "Simple");
    addTranslation("common.nonsimple", "Non-Simple");
    addTranslation("common.complex", "Complex");
    addTranslation("common.noncomplex", "Non-Complex");
    addTranslation("common.easy", "Easy");
    addTranslation("common.noneasy", "Non-Easy");
    addTranslation("common.difficult", "Difficult");
    addTranslation("common.nondifficult", "Non-Difficult");
    addTranslation("common.simple", "Simple");
    addTranslation("common.complex", "Complex");
    addTranslation("common.easy", "Easy");
    addTranslation("common.difficult", "Difficult");
    addTranslation("common.fast", "Fast");
    addTranslation("common.slow", "Slow");
    addTranslation("common.quick", "Quick");
    addTranslation("common.slow", "Slow");
    addTranslation("common.fast", "Fast");
    addTranslation("common.slow", "Slow");
    addTranslation("common.quick", "Quick");
    addTranslation("common.slow", "Slow");
    addTranslation("common.fast", "Fast");
    addTranslation("common.slow", "Slow");
    addTranslation("common.quick", "Quick");
    addTranslation("common.slow", "Slow");
}

// ChineseSimplifiedLanguagePack 构造函数
ChineseSimplifiedLanguagePack::ChineseSimplifiedLanguagePack() : BaseLanguagePack(LANG_ZH_CN, "Chinese Simplified", "简体中文") {
    // 系统相关
    addTranslation("system.title", "墨水时钟");
    addTranslation("system.version", "版本");
    addTranslation("system.uptime", "运行时间");
    addTranslation("system.restart", "重启");
    addTranslation("system.shutdown", "关机");
    addTranslation("system.settings", "设置");
    addTranslation("system.status", "状态");
    
    // 网络相关
    addTranslation("network.title", "网络");
    addTranslation("network.wifi", "WiFi");
    addTranslation("network.connected", "已连接");
    addTranslation("network.disconnected", "未连接");
    addTranslation("network.ip_address", "IP地址");
    addTranslation("network.signal_strength", "信号强度");
    
    // 时间相关
    addTranslation("time.title", "时间");
    addTranslation("time.date", "日期");
    addTranslation("time.time", "时间");
    addTranslation("time.timezone", "时区");
    addTranslation("time.sync", "同步");
    addTranslation("time.synced", "已同步");
    addTranslation("time.format", "格式");
    addTranslation("time.24h", "24小时制");
    addTranslation("time.12h", "12小时制");
    
    // 显示相关
    addTranslation("display.title", "显示");
    addTranslation("display.brightness", "亮度");
    addTranslation("display.rotation", "旋转");
    addTranslation("display.update_interval", "更新间隔");
    addTranslation("display.clock_mode", "时钟模式");
    addTranslation("display.digital", "数字");
    addTranslation("display.analog", "模拟");
    addTranslation("display.text", "文字");
    
    // 传感器相关
    addTranslation("sensor.title", "传感器");
    addTranslation("sensor.temperature", "温度");
    addTranslation("sensor.humidity", "湿度");
    addTranslation("sensor.pressure", "气压");
    addTranslation("sensor.altitude", "海拔");
    addTranslation("sensor.light", "光照");
    addTranslation("sensor.co2", "二氧化碳");
    addTranslation("sensor.voc", "挥发性有机物");
    addTranslation("sensor.update_interval", "更新间隔");
    
    // 天气相关
    addTranslation("weather.title", "天气");
    addTranslation("weather.temperature", "温度");
    addTranslation("weather.humidity", "湿度");
    addTranslation("weather.pressure", "气压");
    addTranslation("weather.wind_speed", "风速");
    addTranslation("weather.wind_direction", "风向");
    addTranslation("weather.rainfall", "降雨量");
    addTranslation("weather.uv_index", "紫外线指数");
    addTranslation("weather.air_quality", "空气质量");
    addTranslation("weather.update_interval", "更新间隔");
    
    // 消息相关
    addTranslation("message.title", "消息");
    addTranslation("message.new", "新消息");
    addTranslation("message.unread", "未读");
    addTranslation("message.read", "已读");
    addTranslation("message.delete", "删除");
    addTranslation("message.clear", "清空全部");
    addTranslation("message.priority", "优先级");
    addTranslation("message.normal", "普通");
    addTranslation("message.high", "高");
    addTranslation("message.critical", "紧急");
    
    // 电源相关
    addTranslation("power.title", "电源");
    addTranslation("power.battery", "电池");
    addTranslation("power.charging", "充电中");
    addTranslation("power.full", "已满");
    addTranslation("power.low", "低电量");
    addTranslation("power.shutdown", "关机");
    addTranslation("power.restart", "重启");
    
    // 配置相关
    addTranslation("config.title", "配置");
    addTranslation("config.save", "保存");
    addTranslation("config.load", "加载");
    addTranslation("config.reset", "重置");
    addTranslation("config.backup", "备份");
    addTranslation("config.restore", "恢复");
    
    // 错误相关
    addTranslation("error.title", "错误");
    addTranslation("error.network", "网络错误");
    addTranslation("error.sensor", "传感器错误");
    addTranslation("error.display", "显示错误");
    addTranslation("error.storage", "存储错误");
    addTranslation("error.config", "配置错误");
    addTranslation("error.api", "API错误");
    addTranslation("error.power", "电源错误");
    
    // 插件相关
    addTranslation("plugin.title", "插件");
    addTranslation("plugin.loaded", "已加载");
    addTranslation("plugin.unloaded", "未加载");
    addTranslation("plugin.enable", "启用");
    addTranslation("plugin.disable", "禁用");
    addTranslation("plugin.install", "安装");
    addTranslation("plugin.uninstall", "卸载");
    
    // 远程控制相关
    addTranslation("remote.title", "远程控制");
    addTranslation("remote.enable", "启用远程控制");
    addTranslation("remote.port", "端口");
    addTranslation("remote.security", "安全");
    addTranslation("remote.api_key", "API密钥");
    
    // 通用
    addTranslation("common.ok", "确定");
    addTranslation("common.cancel", "取消");
    addTranslation("common.save", "保存");
    addTranslation("common.delete", "删除");
    addTranslation("common.edit", "编辑");
    addTranslation("common.add", "添加");
    addTranslation("common.remove", "移除");
    addTranslation("common.up", "上");
    addTranslation("common.down", "下");
    addTranslation("common.left", "左");
    addTranslation("common.right", "右");
    addTranslation("common.on", "开");
    addTranslation("common.off", "关");
    addTranslation("common.enabled", "已启用");
    addTranslation("common.disabled", "已禁用");
    addTranslation("common.success", "成功");
    addTranslation("common.failure", "失败");
    addTranslation("common.warning", "警告");
    addTranslation("common.error", "错误");
    addTranslation("common.info", "信息");
    addTranslation("common.confirm", "确认");
    addTranslation("common.back", "返回");
    addTranslation("common.next", "下一步");
    addTranslation("common.previous", "上一步");
    addTranslation("common.first", "第一");
    addTranslation("common.last", "最后");
    addTranslation("common.all", "全部");
    addTranslation("common.none", "无");
    addTranslation("common.default", "默认");
    addTranslation("common.custom", "自定义");
    addTranslation("common.auto", "自动");
    addTranslation("common.manual", "手动");
    addTranslation("common.system", "系统");
    addTranslation("common.user", "用户");
    addTranslation("common.application", "应用");
    addTranslation("common.device", "设备");
    addTranslation("common.network", "网络");
    addTranslation("common.internet", "互联网");
    addTranslation("common.local", "本地");
    addTranslation("common.remote", "远程");
    addTranslation("common.wireless", "无线");
    addTranslation("common.wired", "有线");
    addTranslation("common.serial", "串口");
    addTranslation("common.usb", "USB");
    addTranslation("common.bluetooth", "蓝牙");
    addTranslation("common.wifi", "WiFi");
    addTranslation("common.ethernet", "以太网");
    addTranslation("common.battery", "电池");
    addTranslation("common.ac_power", "交流电源");
    addTranslation("common.dc_power", "直流电源");
    addTranslation("common.solar", "太阳能");
    addTranslation("common.wind", "风能");
    addTranslation("common.water", "水能");
    addTranslation("common.thermal", "热能");
    addTranslation("common.geothermal", "地热能");
    addTranslation("common.biomass", "生物质能");
    addTranslation("common.nuclear", "核能");
    addTranslation("common.fossil", "化石能源");
    addTranslation("common.renewable", "可再生能源");
    addTranslation("common.non_renewable", "不可再生能源");
    addTranslation("common.sustainable", "可持续");
    addTranslation("common.unsustainable", "不可持续");
    addTranslation("common.efficient", "高效");
    addTranslation("common.inefficient", "低效");
    addTranslation("common.economic", "经济");
    addTranslation("common.uneconomic", "不经济");
    addTranslation("common.environmental", "环保");
    addTranslation("common.nonenvironmental", "不环保");
    addTranslation("common.social", "社会");
    addTranslation("common.nonsocial", "非社会");
    addTranslation("common.governance", "治理");
    addTranslation("common.nongovernance", "非治理");
    addTranslation("common.technical", "技术");
    addTranslation("common.nontechnical", "非技术");
    addTranslation("common.scientific", "科学");
    addTranslation("common.nonscientific", "非科学");
    addTranslation("common.artistic", "艺术");
    addTranslation("common.nonartistic", "非艺术");
    addTranslation("common.creative", "创意");
    addTranslation("common.noncreative", "非创意");
    addTranslation("common.innovative", "创新");
    addTranslation("common.noninnovative", "非创新");
    addTranslation("common.traditional", "传统");
    addTranslation("common.nontraditional", "非传统");
    addTranslation("common.modern", "现代");
    addTranslation("common.nonmodern", "非现代");
    addTranslation("common.classic", "经典");
    addTranslation("common.nonclassic", "非经典");
    addTranslation("common.vintage", "复古");
    addTranslation("common.nonvintage", "非复古");
    addTranslation("common.antique", "古董");
    addTranslation("common.nonantique", "非古董");
    addTranslation("common.retro", "怀旧");
    addTranslation("common.nonretro", "非怀旧");
    addTranslation("common.futuristic", "未来");
    addTranslation("common.nonfuturistic", "非未来");
    addTranslation("common.minimalist", "极简");
    addTranslation("common.nonminimalist", "非极简");
    addTranslation("common.maximalist", "极繁");
    addTranslation("common.nonmaximalist", "非极繁");
    addTranslation("common.simple", "简单");
    addTranslation("common.nonsimple", "非简单");
    addTranslation("common.complex", "复杂");
    addTranslation("common.noncomplex", "非复杂");
    addTranslation("common.easy", "简单");
    addTranslation("common.noneasy", "非简单");
    addTranslation("common.difficult", "困难");
    addTranslation("common.nondifficult", "非困难");
    addTranslation("common.fast", "快速");
    addTranslation("common.slow", "慢速");
    addTranslation("common.quick", "迅速");
    addTranslation("common.slow", "缓慢");
}

// ChineseTraditionalLanguagePack 构造函数
ChineseTraditionalLanguagePack::ChineseTraditionalLanguagePack() : BaseLanguagePack(LANG_ZH_TW, "Chinese Traditional", "繁體中文") {
    // 系统相关
    addTranslation("system.title", "墨水時鐘");
    addTranslation("system.version", "版本");
    addTranslation("system.uptime", "執行時間");
    addTranslation("system.restart", "重啟");
    addTranslation("system.shutdown", "關機");
    addTranslation("system.settings", "設定");
    addTranslation("system.status", "狀態");
    
    // 网络相关
    addTranslation("network.title", "網路");
    addTranslation("network.wifi", "WiFi");
    addTranslation("network.connected", "已連接");
    addTranslation("network.disconnected", "未連接");
    addTranslation("network.ip_address", "IP位址");
    addTranslation("network.signal_strength", "信號強度");
    
    // 时间相关
    addTranslation("time.title", "時間");
    addTranslation("time.date", "日期");
    addTranslation("time.time", "時間");
    addTranslation("time.timezone", "時區");
    addTranslation("time.sync", "同步");
    addTranslation("time.synced", "已同步");
    addTranslation("time.format", "格式");
    addTranslation("time.24h", "24小時制");
    addTranslation("time.12h", "12小時制");
    
    // 显示相关
    addTranslation("display.title", "顯示");
    addTranslation("display.brightness", "亮度");
    addTranslation("display.rotation", "旋轉");
    addTranslation("display.update_interval", "更新間隔");
    addTranslation("display.clock_mode", "時鐘模式");
    addTranslation("display.digital", "數字");
    addTranslation("display.analog", "類比");
    addTranslation("display.text", "文字");
    
    // 传感器相关
    addTranslation("sensor.title", "感測器");
    addTranslation("sensor.temperature", "溫度");
    addTranslation("sensor.humidity", "濕度");
    addTranslation("sensor.pressure", "氣壓");
    addTranslation("sensor.altitude", "海拔");
    addTranslation("sensor.light", "光照");
    addTranslation("sensor.co2", "二氧化碳");
    addTranslation("sensor.voc", "揮發性有機物");
    addTranslation("sensor.update_interval", "更新間隔");
    
    // 天气相关
    addTranslation("weather.title", "天氣");
    addTranslation("weather.temperature", "溫度");
    addTranslation("weather.humidity", "濕度");
    addTranslation("weather.pressure", "氣壓");
    addTranslation("weather.wind_speed", "風速");
    addTranslation("weather.wind_direction", "風向");
    addTranslation("weather.rainfall", "降雨量");
    addTranslation("weather.uv_index", "紫外線指數");
    addTranslation("weather.air_quality", "空氣品質");
    addTranslation("weather.update_interval", "更新間隔");
    
    // 消息相关
    addTranslation("message.title", "訊息");
    addTranslation("message.new", "新訊息");
    addTranslation("message.unread", "未讀");
    addTranslation("message.read", "已讀");
    addTranslation("message.delete", "刪除");
    addTranslation("message.clear", "清空全部");
    addTranslation("message.priority", "優先級");
    addTranslation("message.normal", "普通");
    addTranslation("message.high", "高");
    addTranslation("message.critical", "緊急");
    
    // 电源相关
    addTranslation("power.title", "電源");
    addTranslation("power.battery", "電池");
    addTranslation("power.charging", "充電中");
    addTranslation("power.full", "已滿");
    addTranslation("power.low", "低電量");
    addTranslation("power.shutdown", "關機");
    addTranslation("power.restart", "重啟");
    
    // 配置相关
    addTranslation("config.title", "配置");
    addTranslation("config.save", "儲存");
    addTranslation("config.load", "載入");
    addTranslation("config.reset", "重置");
    addTranslation("config.backup", "備份");
    addTranslation("config.restore", "恢復");
    
    // 错误相关
    addTranslation("error.title", "錯誤");
    addTranslation("error.network", "網路錯誤");
    addTranslation("error.sensor", "感測器錯誤");
    addTranslation("error.display", "顯示錯誤");
    addTranslation("error.storage", "儲存錯誤");
    addTranslation("error.config", "配置錯誤");
    addTranslation("error.api", "API錯誤");
    addTranslation("error.power", "電源錯誤");
    
    // 插件相关
    addTranslation("plugin.title", "外掛");
    addTranslation("plugin.loaded", "已載入");
    addTranslation("plugin.unloaded", "未載入");
    addTranslation("plugin.enable", "啟用");
    addTranslation("plugin.disable", "禁用");
    addTranslation("plugin.install", "安裝");
    addTranslation("plugin.uninstall", "解除安裝");
    
    // 远程控制相关
    addTranslation("remote.title", "遠端控制");
    addTranslation("remote.enable", "啟用遠端控制");
    addTranslation("remote.port", "連接埠");
    addTranslation("remote.security", "安全");
    addTranslation("remote.api_key", "API金鑰");
    
    // 通用
    addTranslation("common.ok", "確定");
    addTranslation("common.cancel", "取消");
    addTranslation("common.save", "儲存");
    addTranslation("common.delete", "刪除");
    addTranslation("common.edit", "編輯");
    addTranslation("common.add", "新增");
    addTranslation("common.remove", "移除");
    addTranslation("common.up", "上");
    addTranslation("common.down", "下");
    addTranslation("common.left", "左");
    addTranslation("common.right", "右");
    addTranslation("common.on", "開");
    addTranslation("common.off", "關");
    addTranslation("common.enabled", "已啟用");
    addTranslation("common.disabled", "已禁用");
    addTranslation("common.success", "成功");
    addTranslation("common.failure", "失敗");
    addTranslation("common.warning", "警告");
    addTranslation("common.error", "錯誤");
    addTranslation("common.info", "資訊");
    addTranslation("common.confirm", "確認");
    addTranslation("common.back", "返回");
    addTranslation("common.next", "下一步");
    addTranslation("common.previous", "上一步");
    addTranslation("common.first", "第一");
    addTranslation("common.last", "最後");
    addTranslation("common.all", "全部");
    addTranslation("common.none", "無");
    addTranslation("common.default", "預設");
    addTranslation("common.custom", "自訂");
    addTranslation("common.auto", "自動");
    addTranslation("common.manual", "手動");
    addTranslation("common.system", "系統");
    addTranslation("common.user", "使用者");
    addTranslation("common.application", "應用");
    addTranslation("common.device", "裝置");
    addTranslation("common.network", "網路");
    addTranslation("common.internet", "網際網路");
    addTranslation("common.local", "本機");
    addTranslation("common.remote", "遠端");
    addTranslation("common.wireless", "無線");
    addTranslation("common.wired", "有線");
    addTranslation("common.serial", "序列埠");
    addTranslation("common.usb", "USB");
    addTranslation("common.bluetooth", "藍牙");
    addTranslation("common.wifi", "WiFi");
    addTranslation("common.ethernet", "乙太網路");
    addTranslation("common.battery", "電池");
    addTranslation("common.ac_power", "交流電源");
    addTranslation("common.dc_power", "直流電源");
    addTranslation("common.solar", "太陽能");
    addTranslation("common.wind", "風能");
    addTranslation("common.water", "水能");
    addTranslation("common.thermal", "熱能");
    addTranslation("common.geothermal", "地熱能");
    addTranslation("common.biomass", "生質能");
    addTranslation("common.nuclear", "核能");
    addTranslation("common.fossil", "化石能源");
    addTranslation("common.renewable", "可再生能源");
    addTranslation("common.non_renewable", "不可再生能源");
    addTranslation("common.sustainable", "可持續");
    addTranslation("common.unsustainable", "不可持續");
    addTranslation("common.efficient", "高效");
    addTranslation("common.inefficient", "低效");
    addTranslation("common.economic", "經濟");
    addTranslation("common.uneconomic", "不經濟");
    addTranslation("common.environmental", "環保");
    addTranslation("common.nonenvironmental", "不環保");
    addTranslation("common.social", "社會");
    addTranslation("common.nonsocial", "非社會");
    addTranslation("common.governance", "治理");
    addTranslation("common.nongovernance", "非治理");
    addTranslation("common.technical", "技術");
    addTranslation("common.nontechnical", "非技術");
    addTranslation("common.scientific", "科學");
    addTranslation("common.nonscientific", "非科學");
    addTranslation("common.artistic", "藝術");
    addTranslation("common.nonartistic", "非藝術");
    addTranslation("common.creative", "創意");
    addTranslation("common.noncreative", "非創意");
    addTranslation("common.innovative", "創新");
    addTranslation("common.noninnovative", "非創新");
    addTranslation("common.traditional", "傳統");
    addTranslation("common.nontraditional", "非傳統");
    addTranslation("common.modern", "現代");
    addTranslation("common.nonmodern", "非現代");
    addTranslation("common.classic", "經典");
    addTranslation("common.nonclassic", "非經典");
    addTranslation("common.vintage", "復古");
    addTranslation("common.nonvintage", "非復古");
    addTranslation("common.antique", "古董");
    addTranslation("common.nonantique", "非古董");
    addTranslation("common.retro", "懷舊");
    addTranslation("common.nonretro", "非懷舊");
    addTranslation("common.futuristic", "未來");
    addTranslation("common.nonfuturistic", "非未來");
    addTranslation("common.minimalist", "極簡");
    addTranslation("common.nonminimalist", "非極簡");
    addTranslation("common.maximalist", "極繁");
    addTranslation("common.nonmaximalist", "非極繁");
    addTranslation("common.simple", "簡單");
    addTranslation("common.nonsimple", "非簡單");
    addTranslation("common.complex", "複雜");
    addTranslation("common.noncomplex", "非複雜");
    addTranslation("common.easy", "簡單");
    addTranslation("common.noneasy", "非簡單");
    addTranslation("common.difficult", "困難");
    addTranslation("common.nondifficult", "非困難");
    addTranslation("common.fast", "快速");
    addTranslation("common.slow", "慢速");
    addTranslation("common.quick", "迅速");
    addTranslation("common.slow", "緩慢");
}

// I18NManager 构造函数
I18NManager::I18NManager() : currentLanguage(LANG_EN), initialized(false) {
}

// I18NManager 单例获取
I18NManager* I18NManager::getInstance() {
    if (instance == nullptr) {
        instance = new I18NManager();
    }
    return instance;
}

// 初始化
bool I18NManager::init() {
    if (initialized) {
        return true;
    }
    
    // 注册默认语言包
    auto englishPack = std::make_shared<EnglishLanguagePack>();
    if (!registerLanguagePack(englishPack)) {
        return false;
    }
    
    auto chineseSimplifiedPack = std::make_shared<ChineseSimplifiedLanguagePack>();
    if (!registerLanguagePack(chineseSimplifiedPack)) {
        return false;
    }
    
    auto chineseTraditionalPack = std::make_shared<ChineseTraditionalLanguagePack>();
    if (!registerLanguagePack(chineseTraditionalPack)) {
        return false;
    }
    
    // 设置默认语言
    setLanguage(LANG_EN);
    
    // 设置回退语言包
    fallbackLanguagePack = englishPack;
    
    initialized = true;
    return true;
}

// 注册语言包
bool I18NManager::registerLanguagePack(std::shared_ptr<ILanguagePack> languagePack) {
    if (!languagePack) {
        return false;
    }
    
    LanguageCode code = languagePack->getLanguageCode();
    languagePacks[code] = languagePack;
    return true;
}

// 设置当前语言
bool I18NManager::setLanguage(LanguageCode language) {
    if (languagePacks.find(language) == languagePacks.end()) {
        return false;
    }
    
    currentLanguage = language;
    return true;
}

// 获取当前语言
LanguageCode I18NManager::getCurrentLanguage() const {
    return currentLanguage;
}

// 获取当前语言名称
String I18NManager::getCurrentLanguageName() const {
    auto it = languagePacks.find(currentLanguage);
    if (it != languagePacks.end()) {
        return it->second->getLanguageName();
    }
    return "";
}

// 获取当前语言原生名称
String I18NManager::getCurrentLanguageNativeName() const {
    auto it = languagePacks.find(currentLanguage);
    if (it != languagePacks.end()) {
        return it->second->getLanguageNativeName();
    }
    return "";
}

// 获取所有支持的语言
std::vector<LanguageCode> I18NManager::getSupportedLanguages() const {
    std::vector<LanguageCode> languages;
    for (const auto& pair : languagePacks) {
        languages.push_back(pair.first);
    }
    return languages;
}

// 获取语言名称
String I18NManager::getLanguageName(LanguageCode language) const {
    auto it = languagePacks.find(language);
    if (it != languagePacks.end()) {
        return it->second->getLanguageName();
    }
    return "";
}

// 获取语言原生名称
String I18NManager::getLanguageNativeName(LanguageCode language) const {
    auto it = languagePacks.find(language);
    if (it != languagePacks.end()) {
        return it->second->getLanguageNativeName();
    }
    return "";
}

// 翻译文本
String I18NManager::translate(const String& key, const String& defaultValue) const {
    // 首先在当前语言中查找
    auto it = languagePacks.find(currentLanguage);
    if (it != languagePacks.end()) {
        String text = it->second->getText(key);
        if (!text.isEmpty()) {
            return text;
        }
    }
    
    // 如果当前语言中没有，在回退语言中查找
    if (fallbackLanguagePack) {
        String text = fallbackLanguagePack->getText(key);
        if (!text.isEmpty()) {
            return text;
        }
    }
    
    // 如果都没有，返回默认值
    return defaultValue;
}

// 检查是否有翻译
bool I18NManager::hasTranslation(const String& key) const {
    // 首先在当前语言中查找
    auto it = languagePacks.find(currentLanguage);
    if (it != languagePacks.end()) {
        if (it->second->hasText(key)) {
            return true;
        }
    }
    
    // 如果当前语言中没有，在回退语言中查找
    if (fallbackLanguagePack) {
        if (fallbackLanguagePack->hasText(key)) {
            return true;
        }
    }
    
    return false;
}

// 格式化翻译文本（支持占位符）
String I18NManager::format(const String& key, const std::vector<String>& params, const String& defaultValue) const {
    String text = translate(key, defaultValue);
    
    // 替换占位符 {0}, {1}, {2}, ...
    for (size_t i = 0; i < params.size(); i++) {
        String placeholder = "{" + String(i) + "}";
        size_t pos = text.indexOf(placeholder);
        while (pos != -1) {
            text = text.substring(0, pos) + params[i] + text.substring(pos + placeholder.length());
            pos = text.indexOf(placeholder);
        }
    }
    
    return text;
}

String I18NManager::format(const String& key, const String& param1, const String& defaultValue) const {
    return format(key, {param1}, defaultValue);
}

String I18NManager::format(const String& key, const String& param1, const String& param2, const String& defaultValue) const {
    return format(key, {param1, param2}, defaultValue);
}

String I18NManager::format(const String& key, const String& param1, const String& param2, const String& param3, const String& defaultValue) const {
    return format(key, {param1, param2, param3}, defaultValue);
}

// 导出翻译为JSON
String I18NManager::exportTranslations(LanguageCode language) const {
    auto it = languagePacks.find(language);
    if (it == languagePacks.end()) {
        return "";
    }
    
    auto pack = it->second;
    std::vector<String> keys = pack->getKeys();
    
    Stringstream ss;
    ss << "{";
    ss << "\"language_code\":" << language << ",";
    ss << "\"language_name\":\"" << pack->getLanguageName().c_str() << ",";
    ss << "\"language_native_name\":\"" << pack->getLanguageNativeName().c_str() << ",";
    ss << "\"translations\":{";
    
    bool first = true;
    for (const auto& key : keys) {
        if (!first) {
            ss << ",";
        }
        ss << "\"" << key.c_str() << ":\"" << pack->getText(key).c_str() << "\"";
        first = false;
    }
    
    ss << "}";
    ss << "}";
    return ss.str().c_str();
}

// 从JSON导入翻译
bool I18NManager::importTranslations(LanguageCode language, const String& json) {
    // 这里可以添加JSON解析逻辑
    // 注意：在Arduino环境中，建议使用ArduinoJson库
    Serial.printf("[I18N] Import translations for language %d: %s\n", language, json.c_str());
    return true;
}
