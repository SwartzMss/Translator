#ifndef CONFIG_H
#define CONFIG_H

#include <array>
#include <QString>
#include <QStringList>

namespace Config {
    // 百度翻译API配置
    inline constexpr const char* BAIDU_API_URL = "https://fanyi-api.baidu.com/api/trans/vip/translate";
    inline constexpr const char* BAIDU_APP_ID = "YOUR_APP_ID";  // 需要用户配置
    inline constexpr const char* BAIDU_SECRET_KEY = "YOUR_SECRET_KEY";  // 需要用户配置
    
    // 支持的语言列表
    inline const QStringList SUPPORTED_LANGUAGES = QStringList{
        "auto", "zh", "en", "yue", "wyw", "jp", "kor", "fra", "spa", "th", 
        "ara", "ru", "pt", "de", "it", "el", "nl", "pl", "bul", "est", 
        "dan", "fin", "cs", "rom", "slo", "swe", "hu", "cht", "vie"
    };
    
    inline const QStringList LANGUAGE_NAMES = QStringList{
        "自动检测", "中文", "英语", "粤语", "文言文", "日语", "韩语", "法语", "西班牙语", "泰语",
        "阿拉伯语", "俄语", "葡萄牙语", "德语", "意大利语", "希腊语", "荷兰语", "波兰语", "保加利亚语", "爱沙尼亚语",
        "丹麦语", "芬兰语", "捷克语", "罗马尼亚语", "斯洛文尼亚语", "瑞典语", "匈牙利语", "繁体中文", "越南语"
    };
    
    // 应用配置
    inline constexpr const char* APP_NAME_STR = "Translator";
    inline constexpr const char* APP_VERSION_STR = "1.0.0";
    inline constexpr int WINDOW_WIDTH = 600;
    inline constexpr int WINDOW_HEIGHT = 400;
    
    // 日志配置
    inline constexpr const char* LOG_FILE_PATH = "logs/translator.log";
    inline constexpr const char* LOG_DATE_FORMAT = "yyyy-MM-dd hh:mm:ss.zzz";
    inline constexpr int MAX_LOG_SIZE = 10 * 1024 * 1024; // 10MB
    inline constexpr int MAX_LOG_FILES = 5;
}

#endif // CONFIG_H 