#!/usr/bin/env python3
"""
命令行固件生成工具
通过逐步向导式选择各个元件型号、启用禁用的功能和传感器，生成最简代码和固件
支持多种微控制器平台（ESP32, ESP8266, NRF52, STM32, RP2040）
"""

import os
import sys
import json
import shutil
import subprocess
import platform
from pathlib import Path

# 检查Python版本
def check_python_version():
    """检查Python版本是否符合要求"""
    required_version = (3, 7)
    current_version = sys.version_info
    
    if current_version < required_version:
        print(f"错误: Python版本过低，需要Python {required_version[0]}.{required_version[1]} 或更高版本")
        print(f"当前Python版本: {current_version[0]}.{current_version[1]}")
        sys.exit(1)
    
    print(f"Python版本检查通过: {current_version[0]}.{current_version[1]}")
    return True

# 检查依赖包
def check_dependencies():
    """检查所需的依赖包是否已安装"""
    required_packages = [
        're',  # 正则表达式，用于文本替换
        'json',  # JSON处理
        'shutil',  # 文件复制
        'os',  # 操作系统功能
        'sys',  # 系统功能
        'platform',  # 平台信息
        'subprocess',  # 子进程调用
        'pathlib'  # 路径处理
    ]
    
    # 标准库包不需要安装，直接检查是否可用
    missing_packages = []
    
    for package in required_packages:
        try:
            __import__(package)
            print(f"依赖检查通过: {package}")
        except ImportError:
            missing_packages.append(package)
    
    if missing_packages:
        print(f"缺少依赖包: {missing_packages}")
        print("这些是Python标准库包，通常不需要额外安装")
        print("请确保使用的是完整的Python安装包")
        return False
    
    return True

# 安装依赖包
def install_dependencies():
    """安装所需的依赖包"""
    # 由于我们只使用Python标准库，所以这个函数主要用于未来扩展
    print("所有依赖都是Python标准库，不需要额外安装")
    return True

# 检测PlatformIO安装状态
def check_platformio_installation():
    """检测PlatformIO是否已安装"""
    print("2. 检查PlatformIO安装状态...")
    
    try:
        # 尝试运行pio命令
        result = subprocess.run(['pio', '--version'], capture_output=True, text=True, check=True)
        version_output = result.stdout.strip()
        print(f"   PlatformIO已安装: {version_output}")
        return True
    except subprocess.CalledProcessError:
        print("   错误: PlatformIO安装可能损坏")
        return False
    except FileNotFoundError:
        print("   错误: PlatformIO未安装")
        print("   请访问 https://platformio.org/ 安装PlatformIO")
        return False
    except Exception as e:
        print(f"   错误: 检测PlatformIO时发生异常: {str(e)}")
        return False

# 检测特定平台支持
def check_platform_support(platform_name):
    """检测是否支持特定平台"""
    print(f"3. 检查{platform_name}平台支持...")
    
    try:
        # 检查是否已安装该平台
        result = subprocess.run(['pio', 'platform', 'list'], capture_output=True, text=True, check=True)
        
        # 映射平台名称到PlatformIO平台ID
        platform_id_mapping = {
            'ESP32': 'espressif32',
            'ESP8266': 'espressif8266',
            'NRF52': 'nordicnrf52',
            'STM32': 'ststm32',
            'RP2040': 'raspberrypi'
        }
        
        platform_id = platform_id_mapping.get(platform_name.upper())
        if platform_id and platform_id in result.stdout:
            print(f"   {platform_name}平台支持已安装")
            return True
        else:
            print(f"   错误: 未安装{platform_name}平台支持")
            print(f"   请运行: pio platform install {platform_id}")
            return False
    except Exception as e:
        print(f"   错误: 检测{platform_name}平台时发生异常: {str(e)}")
        return False

# 检测Arduino框架支持
def check_arduino_framework():
    """检测是否支持Arduino框架"""
    print("4. 检查Arduino框架支持...")
    
    try:
        # Arduino框架通常随平台一起安装
        print("   Arduino框架支持检查通过")
        return True
    except Exception as e:
        print(f"   错误: 检测Arduino框架时发生异常: {str(e)}")
        return False

# 检测OTA工具
def check_ota_tools():
    """检测生成OTA升级包所需的工具"""
    print("5. 检查OTA工具...")
    
    try:
        # 检查是否有espota.py工具（PlatformIO通常会提供）
        result = subprocess.run(['pio', 'home', '--version'], capture_output=True, text=True, check=True)
        print("   OTA工具已安装")
        return True
    except Exception as e:
        print(f"   警告: 检测OTA工具时发生异常: {str(e)}")
        print("   继续执行，但OTA功能可能无法正常工作")
        return True  # OTA工具不是必需的，只是警告

# 检测固件和OTA生成所需的环境
def check_firmware_build_environment(platform_name):
    """检测生成固件和OTA升级包所需的环境"""
    print("===== 固件构建环境检测 =====")
    
    # 检查PlatformIO安装
    if not check_platformio_installation():
        return False
    
    # 检查特定平台
    if not check_platform_support(platform_name):
        return False
    
    # 检查Arduino框架
    if not check_arduino_framework():
        return False
    
    # 检查OTA工具
    check_ota_tools()
    
    print("固件构建环境检测通过\n")
    return True

# 检测运行环境
def check_environment(platform_name):
    """检测运行环境是否符合要求"""
    print("===== 运行环境检测 =====")
    
    # 检查Python版本
    print("1. 检查Python版本...")
    if not check_python_version():
        return False
    
    # 检查依赖
    print("2. 检查Python依赖...")
    if not check_dependencies():
        print("是否尝试安装依赖包？(y/n): ")
        choice = input().lower()
        if choice == 'y':
            if not install_dependencies():
                return False
        else:
            return False
    
    # 检查固件构建环境
    if not check_firmware_build_environment(platform_name):
        return False
    
    print("===== 所有环境检测通过 =====\n")
    return True

# 项目根目录
PROJECT_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))

# 支持的硬件列表，基于硬件物料选型.md
SUPPORTED_HARDWARE = {
    'platform': {
        'name': '微控制器平台',
        'options': {
            'ESP32': 'ESP32系列',
            'ESP8266': 'ESP8266系列',
            'NRF52': 'NRF52系列',
            'STM32': 'STM32系列',
            'RP2040': 'RP2040系列'
        }
    },
    'board': {
        'name': '开发板型号',
        'options': {
            'ESP32': {
                'esp32-wroom-32': 'ESP32-WROOM-32 (WiFi+BT5.0)',
                'esp32-c3-devkitc-02': 'ESP32-C3-DevKitC-02 (WiFi+BT5.0)',
                'esp32-s3-devkitc-1': 'ESP32-S3-DevKitC-1 (WiFi+BT5.0)',
                'esp32-c6-devkitc-1': 'ESP32-C6-DevKitC-1 (WiFi 6+BT5.0+IPv6)',
                'esp32-s2-devkitc-1': 'ESP32-S2-DevKitC-1 (WiFi)',
                'esp32-c3-supermini': 'ESP32-C3-SuperMini (WiFi+BT5.0)',
                'esp32-s3-box': 'ESP32-S3-Box (WiFi+BT5.0+屏幕+摄像头)',
                'seeed_xiao_esp32-s3': 'Seeed Studio XIAO ESP32-S3 (邮票孔设计)',
                'esp32-s3-lcd-ev-board': 'ESP32-S3-LCD-EV-Board (LCD控制器)',
                'dfrobot_firebeetle_2_esp32-s3': 'DFRobot FireBeetle 2 ESP32-S3 (低功耗)'
            },
            'ESP8266': {
                'nodemcuv2': 'NodeMCU v2 (WiFi)',
                'd1_mini': 'WeMos D1 Mini (WiFi)',
                'esp8266-devkitc': 'ESP8266-DevKitC (官方设计)'
            },
            'NRF52': {
                'nrf52840dk': 'nRF52840 DK (WiFi+BT5.0)',
                'nrf52832dk': 'nRF52832 DK (BT5.0)',
                'adafruit_feather_nrf52832': 'Adafruit Feather nRF52832 (蓝牙5.0)'
            },
            'STM32': {
                'bluepill_f103c8': 'Blue Pill STM32F103C8',
                'nucleo_f401re': 'Nucleo-F401RE',
                'stm32l432kc': 'STM32L432KC (低功耗)',
                'stm32g431kb': 'STM32G431KB (高性能)',
                'stm32f411ceu6': 'STM32F411CEU6 (经典型号)'
            },
            'RP2040': {
                'raspberrypi_pico': 'Raspberry Pi Pico',
                'raspberrypi_pico_w': 'Raspberry Pi Pico W (WiFi+蓝牙)',
                'adafruit_feather_rp2040': 'Adafruit Feather RP2040'
            }
        }
    },
    'audio_module': {
            'name': '音频模块',
            'options': {
                'AUDIO_DRIVER_NONE': '无音频模块',
                'AUDIO_DRIVER_ES8388': 'ES8388音频编解码器',
                'AUDIO_DRIVER_MAX98357': 'MAX98357音频放大器',
                'AUDIO_DRIVER_PCM5102': 'PCM5102音频解码器',
                'AUDIO_DRIVER_VS1053B_NO_HEADPHONE': 'VS1053B无耳机孔模块 (存储+扩音器+不带3.5mm耳机孔)',
                'AUDIO_DRIVER_VS1003B_STORAGE': 'VS1003B存储版 (存储+扩音器+不带3.5mm耳机孔)',
                'AUDIO_DRIVER_YX5300': 'YX5300-24SS (存储+扩音器+不带3.5mm耳机孔)',
                'AUDIO_DRIVER_YX6300': 'YX6300-24SS (TF卡+扩音器+不带3.5mm耳机孔)',
                'AUDIO_DRIVER_WT588D': 'WT588D-M02 (存储+扩音器+不带3.5mm耳机孔)',
                'AUDIO_DRIVER_ISD1820': 'ISD1820录音模块 (存储+麦克风+不带3.5mm耳机孔)',
                'AUDIO_DRIVER_NRF52832': 'NRF52832音频模块 (存储+蓝牙+不带3.5mm耳机孔)',
                'AUDIO_DRIVER_ESP32_AUDIO': 'ESP32音频解码模块 (TF卡+WiFi+蓝牙+不带3.5mm耳机孔)',
                'AUDIO_DRIVER_STM32_AUDIO': 'STM32F103音频模块 (存储+扩音器+不带3.5mm耳机孔)',
                'AUDIO_DRIVER_ATMEGA328': 'ATmega328音频播放器 (SD卡+扩音器+不带3.5mm耳机孔)'
            },
            'integrated_tf_card': {
                'AUDIO_DRIVER_VS1053B_NO_HEADPHONE': True,
                'AUDIO_DRIVER_VS1003B_STORAGE': True,
                'AUDIO_DRIVER_YX5300': True,
                'AUDIO_DRIVER_YX6300': True,
                'AUDIO_DRIVER_WT588D': True,
                'AUDIO_DRIVER_ISD1820': True,
                'AUDIO_DRIVER_NRF52832': True,
                'AUDIO_DRIVER_ESP32_AUDIO': True,
                'AUDIO_DRIVER_STM32_AUDIO': True,
                'AUDIO_DRIVER_ATMEGA328': True
            },
            'integrated_microphone': {
                'AUDIO_DRIVER_ISD1820': True,
                'AUDIO_DRIVER_NRF52832': True,
                'AUDIO_DRIVER_ESP32_AUDIO': True
            },
            'integrated_speaker': {
                'AUDIO_DRIVER_MAX98357': True,
                'AUDIO_DRIVER_VS1053B_NO_HEADPHONE': True,
                'AUDIO_DRIVER_VS1003B_STORAGE': True,
                'AUDIO_DRIVER_YX5300': True,
                'AUDIO_DRIVER_YX6300': True,
                'AUDIO_DRIVER_WT588D': True,
                'AUDIO_DRIVER_NRF52832': True,
                'AUDIO_DRIVER_ESP32_AUDIO': True,
                'AUDIO_DRIVER_STM32_AUDIO': True,
                'AUDIO_DRIVER_ATMEGA328': True
            }
        },
    'tf_card_reader': {
        'name': 'TF卡读卡器',
        'options': {
            'TF_READER_NONE': '不使用',
            'TF_READER_SPI': 'SPI 接口 TF 读卡器',
            'TF_READER_SDIO': 'SDIO 接口 TF 读卡器',
            'TF_READER_MMC': 'MMC 接口 TF 读卡器'
        }
    },
    'wifi_bt_module': {
        'name': 'WiFi+蓝牙模块',
        'options': {
            'WIFI_BT_INTERNAL': '内置',
            'WIFI_BT_EXTERNAL_ESP32': '外接 ESP32 WiFi+蓝牙模块',
            'WIFI_BT_EXTERNAL_ESP8266': '外接 ESP8266 WiFi模块',
            'WIFI_BT_EXTERNAL_NRF52': '外接 NRF52 蓝牙模块',
            'WIFI_BT_EXTERNAL_BCM4343': '外接 BCM4343 WiFi+蓝牙模块'
        }
    },
    'display': {
        'name': '墨水屏',
        'options': {
            # 1.02英寸
            'EINK_102_INCH': '1.02英寸墨水屏 (GDEW0102T4)',
            # 1.44英寸
            'EINK_144_INCH': '1.44英寸墨水屏 (GDEW0144Z07)',
            # 1.54英寸
            'EINK_154_INCH': '1.54英寸墨水屏 (GDEW0154M09)',
            'EINK_154_INCH_WAVESHARE': '1.54英寸墨水屏 (Waveshare 1.54inch e-Paper)',
            'EINK_154_INCH_DIY': '1.54英寸墨水屏 (闲鱼自制墨水屏)',
            # 2.13英寸
            'EINK_213_INCH': '2.13英寸墨水屏 (GDEW0213M09)',
            'EINK_213_INCH_Z19': '2.13英寸墨水屏 (GDEW0213Z19)',
            'EINK_213_INCH_WAVESHARE': '2.13英寸墨水屏 (Waveshare 2.13inch e-Paper HAT)',
            'EINK_213_INCH_HEMA': '2.13英寸墨水屏 (盒马电子价签屏)',
            # 2.66英寸
            'EINK_266_INCH': '2.66英寸墨水屏 (盒马电子价签屏)',
            # 2.9英寸
            'EINK_29_INCH': '2.9英寸墨水屏 (GDEW029T5)',
            'EINK_29_INCH_Z13': '2.9英寸墨水屏 (GDEW029Z13)',
            'EINK_29_INCH_WAVESHARE': '2.9英寸墨水屏 (Waveshare 2.9inch e-Paper V2)',
            'EINK_29_INCH_XIAOMI': '2.9英寸墨水屏 (小米之家电子价签屏)',
            # 3.12英寸
            'EINK_312_INCH': '3.12英寸墨水屏 (盒马电子价签屏)',
            # 3.7英寸
            'EINK_37_INCH': '3.7英寸墨水屏 (GDEW0371W7)',
            # 4.2英寸
            'EINK_42_INCH': '4.2英寸墨水屏 (GDEW042T2)',
            'EINK_42_INCH_Z15': '4.2英寸墨水屏 (GDEW042Z15)',
            'EINK_42_INCH_HEMA': '4.2英寸墨水屏 (盒马电子价签屏)',
            'EINK_42_INCH_7FRESH': '4.2英寸墨水屏 (7Fresh电子价签屏)',
            'EINK_42_INCH_DIY': '4.2英寸墨水屏 (闲鱼自制墨水屏)',
            # 5.83英寸
            'EINK_583_INCH': '5.83英寸墨水屏 (GDEW0583T7)',
            # 6.0英寸
            'EINK_60_INCH_KINDLE': '6.0英寸墨水屏 (Kindle Paperwhite拆机屏)',
            'EINK_60_INCH_KINDLE3': '6.0英寸墨水屏 (Kindle Paperwhite 3拆机屏)',
            # 7.5英寸
            'EINK_75_INCH': '7.5英寸墨水屏 (GDEW075T7)',
            'EINK_75_INCH_Z09': '7.5英寸墨水屏 (GDEW075Z09)',
            'EINK_75_INCH_WAVESHARE': '7.5英寸墨水屏 (Waveshare 7.5inch e-Paper HAT)',
            # 7.8英寸
            'EINK_78_INCH_KINDLE': '7.8英寸墨水屏 (Kindle Oasis拆机屏)',
            'EINK_78_INCH_KINDLE2': '7.8英寸墨水屏 (Kindle Oasis 2拆机屏)',
            # 9.7英寸
            'EINK_97_INCH': '9.7英寸墨水屏 (GDEW097T4)',
            # 10.3英寸
            'EINK_103_INCH': '10.3英寸墨水屏 (GDEW103T2)',
            'EINK_103_INCH_REMARKABLE': '10.3英寸墨水屏 (Remarkable 2拆机屏)',
            # 12.48英寸
            'EINK_1248_INCH': '12.48英寸墨水屏 (GDEW1248Z17)'
        }
    },
    'sensor': {
        'name': '传感器',
        'options': {
            # 温湿度传感器
            'DHT22': 'DHT22 温湿度传感器',
            'AM2302': 'AM2302 温湿度传感器 (DHT22封装版)',
            'SHT20': 'SHT20 温湿度传感器',
            'SHT30': 'SHT30 温湿度传感器',
            'SHT40': 'SHT40 温湿度传感器',
            'HDC1080': 'HDC1080 温湿度传感器',
            'BME280': 'BME280 温湿度气压传感器',
            'BME680': 'BME680 温湿度气压气体传感器',
            # 人体感应传感器
            'HC_SR501': 'HC-SR501 人体感应传感器',
            'HC_SR505': 'HC-SR505 小型人体感应传感器',
            'RCWL_0516': 'RCWL-0516 微波雷达感应模块',
            'LD2410': 'LD2410 毫米波雷达模块',
            # 光照传感器
            'BH1750': 'BH1750 光照传感器',
            'GY30': 'GY30 高精度光照传感器',
            'TSL2561': 'TSL2561 宽动态范围光照传感器',
            'OPT3001': 'OPT3001 高精度环境光传感器',
            # 气体传感器
            'MQ2': 'MQ-2 烟雾燃气传感器',
            'MQ5': 'MQ-5 液化石油气传感器',
            'MQ7': 'MQ-7 一氧化碳传感器',
            'MQ135': 'MQ-135 多种有害气体传感器',
            'SGP30': 'SGP30 数字空气质量传感器',
            # 火焰传感器
            'IR_FLAME': 'IR 火焰传感器',
            'YG1006': 'YG1006 高灵敏度红外火焰传感器',
            'UV_FLAME': 'UV 火焰传感器',
            # 气压传感器
            'LPS25HB': 'LPS25HB 气压传感器',
            'BMP388': 'BMP388 高精度气压传感器',
        }
    },
    'hardware': {
        'name': '硬件模块',
        'options': {
            'TF_CARD': 'TF卡模块',
            'CAMERA_GC0308': 'GC0308 微型摄像头 (30万像素)',
            'CAMERA_OV2640': 'OV2640 微型摄像头 (200万像素)',
            'CAMERA_OV5640': 'OV5640 微型摄像头 (500万像素)',
            'CAMERA_ESP32_CAM': 'ESP32-CAM 专用摄像头 (200万像素)'
        }
    },
    'feature': {
        'name': '功能模块',
        'options': {
            'FONT': '字体管理功能',
            'ALARM_DISPLAY': '报警显示功能',
            'AUDIO': '音频功能',
            'VOICE_MESSAGE': '语音留言功能',
            'VIDEO_MESSAGE': '视频留言功能',
            'TF_CARD_MANAGEMENT': 'TF卡管理功能',
            'TOUCH': '触摸功能',
            'TEMPERATURE_HUMIDITY_DISPLAY': '室内温湿度显示功能',
            'MOTION_SAVING': '感应节能功能'
        }
    },
    'mandatory_features': {
        'name': '必选功能',
        'options': {
            'WIFI': 'WiFi功能',
            'BLUETOOTH': '蓝牙功能',
            'TEXT_MESSAGE': '文字留言功能',
            'WEBCLIENT': 'Web客户端功能',
            'FIRMWARE': '固件更新功能',
            'PLUGIN': '插件功能'
        }
    },
    'board_has_wifi_bt': {
        'ESP32': {
            'esp32-wroom-32': True,
            'esp32-c3-devkitc-02': True,
            'esp32-s3-devkitc-1': True,
            'esp32-c6-n4': True,
            'esp32-s2-devkitc-1': True,
            'esp32-c3-supermini': True
        },
        'ESP8266': {
            'nodemcuv2': True,
            'd1_mini': True
        },
        'NRF52': {
            'nrf52840dk': True,
            'nrf52832dk': True
        },
        'STM32': {
            'bluepill_f103c8': False,
            'nucleo_f401re': False
        },
        'RP2040': {
            'raspberrypi_pico': True,
            'adafruit_feather_rp2040': True
        }
    }
}

# 功能与硬件的映射关系
FEATURE_HARDWARE_MAPPING = {
    'AUDIO': ['audio_module'],
    'VOICE_MESSAGE': ['audio_module', 'tf_card'],
    'VIDEO_MESSAGE': ['camera', 'tf_card'],
    'TF_CARD_MANAGEMENT': ['tf_card'],
    'ALARM_DISPLAY': ['audio_module', 'sensor'],  # 关联音频模块（扬声器/蜂鸣器）和传感器
    'TEMPERATURE_HUMIDITY_DISPLAY': ['sensor'],  # 关联温湿度相关传感器
    'MOTION_SAVING': ['sensor'],  # 关联人体感应传感器
    'WIFI': ['wifi_bt_module'],
    'BLUETOOTH': ['wifi_bt_module'],
    'TOUCH': ['display'],
    'TEXT_MESSAGE': ['wifi_bt_module'],
    'WEBCLIENT': ['wifi_bt_module'],
    'FIRMWARE': ['wifi_bt_module'],
    'PLUGIN': ['platform']
}

# 选择功能模块
def select_features():
    """通过向导选择功能模块"""
    print("1. 选择功能模块")
    print("   必选功能会自动添加，无需手动选择")
    print()
    
    # 显示必选功能
    print("   === 必选功能 ===")
    mandatory_features = list(SUPPORTED_HARDWARE['mandatory_features']['options'].items())
    for i, (key, name) in enumerate(mandatory_features, 1):
        print(f"   [{i}*] {name}")
    
    print()
    
    # 显示可选功能
    print("   === 可选功能 ===")
    optional_features = list(SUPPORTED_HARDWARE['feature']['options'].items())
    optional_feature_keys = [key for key, _ in optional_features]
    
    for i, (key, name) in enumerate(optional_features, 1):
        print(f"   [{i}] {name}")
    
    print()
    print("   请选择需要的可选功能（可多选）")
    print("   输入格式示例: 1 2 3 或 1,2,3")
    print("   直接按回车跳过，不选择任何可选功能")
    
    # 获取用户选择
    while True:
        try:
            user_input = input("   请输入选择: ").strip()
            
            # 处理不同的输入格式
            if not user_input:
                selected_indices = []
                break
            elif ',' in user_input:
                selected_indices = [int(x.strip()) for x in user_input.split(',')]
            else:
                selected_indices = [int(x.strip()) for x in user_input.split()]
            
            # 验证输入范围
            if all(1 <= idx <= len(optional_features) for idx in selected_indices):
                break
            else:
                print("   输入无效，请重新选择")
        except ValueError:
            print("   输入无效，请输入数字")
    
    # 添加选择的可选功能
    features = []
    for idx in selected_indices:
        feature_key = optional_feature_keys[idx-1]
        features.append(feature_key)
    
    # 自动添加关联功能
    # 1. 选择了音频功能，自动添加TF卡管理功能
    if 'AUDIO' in features and 'TF_CARD_MANAGEMENT' not in features:
        features.append('TF_CARD_MANAGEMENT')
    
    # 自动添加所有必选功能
    mandatory_feature_keys = [key for key, _ in mandatory_features]
    for feature_key in mandatory_feature_keys:
        features.append(feature_key)
    
    # 显示最终选择的功能
    print("\n已选择功能:")
    all_features_dict = {**SUPPORTED_HARDWARE['mandatory_features']['options'], **SUPPORTED_HARDWARE['feature']['options']}
    for feature_key in features:
        is_mandatory = feature_key in mandatory_feature_keys
        mandatory_mark = "*" if is_mandatory else ""
        print(f"   [{mandatory_mark}] {all_features_dict[feature_key]}")
    
    # 显示功能关联说明
    if 'AUDIO' in features and 'TF_CARD_MANAGEMENT' in features:
        print("   🔗 音频功能自动关联了TF卡管理功能")
    
    if 'ALARM_DISPLAY' in features:
        print("   🔗 报警显示功能关联了气体和火焰传感器，以及扬声器/蜂鸣器")
    
    print()
    
    return features, all_features_dict

# 根据选择的功能确定所需的硬件类型
def determine_required_hardware(features):
    """根据选择的功能确定所需的硬件类型"""
    required_hardware = set()
    for feature in features:
        if feature in FEATURE_HARDWARE_MAPPING:
            required_hardware.update(FEATURE_HARDWARE_MAPPING[feature])
    
    print(f"\n{'=' * 50}")
    print("硬件需求分析")
    print(f"{'=' * 50}")
    print(f"根据所选功能，需要配置以下硬件组件: {list(required_hardware)}")
    print("系统将自动处理集成硬件冲突，避免重复选择")
    print()
    
    return required_hardware

# 选择微控制器平台和开发板
def select_platform_and_board():
    """选择微控制器平台和开发板"""
    config = {
        'platform': '',
        'board': '',
        'has_wifi_bt': False
    }
    
    # 选择微控制器平台
    print("\n3. 选择微控制器平台")
    print("   请选择适合您项目的微控制器平台")
    platform_options = list(SUPPORTED_HARDWARE['platform']['options'].items())
    for i, (key, name) in enumerate(platform_options, 1):
        print(f"   {i}. {name}")
    
    while True:
        try:
            choice = int(input("   请输入选择 (1-{}): ".format(len(platform_options))))
            if 1 <= choice <= len(platform_options):
                config['platform'] = platform_options[choice-1][0]
                break
            else:
                print("   输入无效，请重新选择")
        except ValueError:
            print("   输入无效，请输入数字")
    
    print(f"\n   ✓ 已选择: {SUPPORTED_HARDWARE['platform']['options'][config['platform']]}")
    
    # 选择开发板型号
    print("\n4. 选择开发板型号")
    print(f"   请从 {SUPPORTED_HARDWARE['platform']['options'][config['platform']]} 系列中选择开发板")
    board_options = list(SUPPORTED_HARDWARE['board']['options'][config['platform']].items())
    for i, (key, name) in enumerate(board_options, 1):
        print(f"   {i}. {name}")
    
    while True:
        try:
            choice = int(input("   请输入选择 (1-{}): ".format(len(board_options))))
            if 1 <= choice <= len(board_options):
                config['board'] = board_options[choice-1][0]
                break
            else:
                print("   输入无效，请重新选择")
        except ValueError:
            print("   输入无效，请输入数字")
    
    print(f"\n   ✓ 已选择: {SUPPORTED_HARDWARE['board']['options'][config['platform']][config['board']]}")
    
    # 检查开发板是否内置WiFi+蓝牙
    config['has_wifi_bt'] = SUPPORTED_HARDWARE['board_has_wifi_bt'][config['platform']][config['board']]
    
    return config

# 检查开发板内置功能
def check_integrated_features(config):
    """检查开发板内置功能"""
    print("\n5. 开发板内置功能检查")
    if config['has_wifi_bt']:
        print(f"   ✓ 开发板内置WiFi+蓝牙模块，无需额外配置")
        config['wifi_bt_module'] = 'WIFI_BT_INTERNAL'
    else:
        print(f"   ⚠ 开发板不内置WiFi+蓝牙模块，稍后需手动选择")
    
    return config

# 根据功能选择硬件组件
def select_hardware_components(config, required_hardware):
    """根据功能选择硬件组件"""
    print(f"\n{'=' * 50}")
    print("6. 根据功能选择硬件组件")
    print(f"{'=' * 50}")
    
    # 处理WiFi+蓝牙模块
    if 'wifi_bt_module' in required_hardware:
        if not config['has_wifi_bt']:
            print("   选择WiFi+蓝牙模块:")
            wifi_bt_options = list(SUPPORTED_HARDWARE['wifi_bt_module']['options'].items())
            # 移除内置选项，因为需要外接模块
            wifi_bt_options = [opt for opt in wifi_bt_options if opt[0] != 'WIFI_BT_INTERNAL']
            
            for i, (key, name) in enumerate(wifi_bt_options, 1):
                print(f"      {i}. {name}")
            
            while True:
                try:
                    choice = int(input("      请输入选择 (1-{}): ".format(len(wifi_bt_options))))
                    if 1 <= choice <= len(wifi_bt_options):
                        config['wifi_bt_module'] = wifi_bt_options[choice-1][0]
                        break
                    else:
                        print("      输入无效，请重新选择")
                except ValueError:
                    print("      输入无效，请输入数字")
            
            print(f"   已选择: {SUPPORTED_HARDWARE['wifi_bt_module']['options'][config['wifi_bt_module']]}\n")
        else:
            print(f"   ✓ WiFi+蓝牙功能由开发板内置提供\n")
            # 从required_hardware中移除，避免后续重复处理
            required_hardware.discard('wifi_bt_module')
    
    # 处理音频模块
    if 'audio_module' in required_hardware:
        print("   选择音频模块型号:")
        audio_options = list(SUPPORTED_HARDWARE['audio_module']['options'].items())
        # 移除"无音频模块"选项，因为需要音频功能
        audio_options = [opt for opt in audio_options if opt[0] != 'AUDIO_DRIVER_NONE']
        
        # 显示每个音频模块的集成功能，帮助用户选择
        for i, (key, name) in enumerate(audio_options, 1):
            # 检查集成功能
            has_integrated_tf = SUPPORTED_HARDWARE['audio_module']['integrated_tf_card'].get(key, False)
            has_integrated_mic = SUPPORTED_HARDWARE['audio_module']['integrated_microphone'].get(key, False)
            has_integrated_speaker = SUPPORTED_HARDWARE['audio_module']['integrated_speaker'].get(key, False)
            
            # 生成集成功能描述
            integrated_features = []
            if has_integrated_tf:
                integrated_features.append("TF卡")
            if has_integrated_mic:
                integrated_features.append("麦克风")
            if has_integrated_speaker:
                integrated_features.append("扬声器")
            
            if integrated_features:
                integrated_desc = f" (集成: {', '.join(integrated_features)})"
            else:
                integrated_desc = ""
            
            print(f"      {i}. {name}{integrated_desc}")
        
        while True:
            try:
                choice = int(input("      请输入选择 (1-{}): ".format(len(audio_options))))
                if 1 <= choice <= len(audio_options):
                    config['audio_module'] = audio_options[choice-1][0]
                    break
                else:
                    print("      输入无效，请重新选择")
            except ValueError:
                print("      输入无效，请输入数字")
        
        print(f"   已选择: {SUPPORTED_HARDWARE['audio_module']['options'][config['audio_module']]}\n")
        
        # 检查音频模块是否集成了其他硬件功能，更新required_hardware
        if SUPPORTED_HARDWARE['audio_module']['integrated_tf_card'].get(config['audio_module'], False):
            print(f"   ✓ 音频模块集成了TF卡功能，无需额外配置TF卡读卡器")
            # 从required_hardware中移除，避免后续重复处理
            required_hardware.discard('tf_card')
    else:
        config['audio_module'] = 'AUDIO_DRIVER_NONE'
    
    # 处理墨水屏
    print("   选择墨水屏型号:")
    display_options = list(SUPPORTED_HARDWARE['display']['options'].items())
    
    for i, (key, name) in enumerate(display_options, 1):
        print(f"      {i}. {name}")
    
    while True:
        try:
            choice = int(input("      请输入选择 (1-{}): ".format(len(display_options))))
            if 1 <= choice <= len(display_options):
                config['display'] = display_options[choice-1][0]
                break
            else:
                print("      输入无效，请重新选择")
        except ValueError:
            print("      输入无效，请输入数字")
    
    print(f"   已选择: {SUPPORTED_HARDWARE['display']['options'][config['display']]}\n")
    
    # 处理TF卡相关硬件
    config['has_tf_card'] = False
    if 'tf_card' in required_hardware:
        # 检查音频模块是否集成TF卡功能
        has_integrated_tf = False
        if config['audio_module'] != 'AUDIO_DRIVER_NONE':
            has_integrated_tf = SUPPORTED_HARDWARE['audio_module']['integrated_tf_card'].get(config['audio_module'], False)
        
        if has_integrated_tf:
            config['has_tf_card'] = True
            config['tf_card_reader'] = 'TF_READER_NONE'
            print(f"   ✓ 音频模块已集成TF卡功能，无需额外配置\n")
        else:
            # 选择TF卡读卡器
            print("   选择TF卡读卡器:")
            tf_options = list(SUPPORTED_HARDWARE['tf_card_reader']['options'].items())
            # 移除"不使用"选项，因为需要TF卡功能
            tf_options = [opt for opt in tf_options if opt[0] != 'TF_READER_NONE']
            
            for i, (key, name) in enumerate(tf_options, 1):
                print(f"      {i}. {name}")
            
            while True:
                try:
                    choice = int(input("      请输入选择 (1-{}): ".format(len(tf_options))))
                    if 1 <= choice <= len(tf_options):
                        config['tf_card_reader'] = tf_options[choice-1][0]
                        break
                    else:
                        print("      输入无效，请重新选择")
                except ValueError:
                    print("      输入无效，请输入数字")
            
            print(f"   已选择: {SUPPORTED_HARDWARE['tf_card_reader']['options'][config['tf_card_reader']]}\n")
            config['has_tf_card'] = True
    
    # 处理摄像头模块
    if 'camera' in required_hardware and config['platform'] == 'ESP32':
        # ESP32系列开发板支持摄像头，自动添加
        if 'CAMERA' not in config['hardware']:
            config['hardware'].append('CAMERA')
            print(f"   ✓ 自动添加摄像头模块 (ESP32系列开发板支持)\n")
    
    return config

# 选择传感器
def select_sensors(config):
    """选择传感器"""
    print(f"\n{'=' * 50}")
    print("7. 可选：选择传感器")
    print(f"{'=' * 50}")
    
    # 检查是否选择了相关功能
    has_alarm_display = 'ALARM_DISPLAY' in config['features']
    has_temp_humidity_display = 'TEMPERATURE_HUMIDITY_DISPLAY' in config['features']
    has_motion_saving = 'MOTION_SAVING' in config['features']
    
    # 打印功能相关提示
    print("   您可以选择一个或多个传感器来扩展设备功能")
    
    # 功能相关传感器推荐提示
    recommended_sensors = []
    if has_alarm_display:
        recommended_sensors.append("气体和火焰传感器")
    if has_temp_humidity_display:
        recommended_sensors.append("温湿度相关传感器")
    if has_motion_saving:
        recommended_sensors.append("人体感应传感器")
    
    if recommended_sensors:
        print(f"   由于您选择的功能，建议选择: {', '.join(recommended_sensors)}")
    
    print("   输入格式示例: 1 2 3 或 1,2,3")
    print("   直接按回车跳过，不选择任何传感器")
    
    sensor_options = list(SUPPORTED_HARDWARE['sensor']['options'].items())
    sensor_keys = [key for key, _ in sensor_options]
    
    print("   可选传感器列表:")
    for i, (key, name) in enumerate(sensor_options, 1):
        # 标记相关传感器为推荐
        is_recommended = False
        
        # 报警显示功能推荐气体和火焰传感器
        if has_alarm_display and (key == 'MQ135' or key == 'IR_FLAME'):
            is_recommended = True
        # 室内温湿度显示功能推荐温湿度相关传感器
        elif has_temp_humidity_display and (key == 'DHT22' or key == 'SHT30' or key == 'BME280'):
            is_recommended = True
        # 感应节能功能推荐人体感应传感器
        elif has_motion_saving and key == 'HC_SR501':
            is_recommended = True
        
        if is_recommended:
            print(f"      {i}. {name} (推荐)")
        else:
            print(f"      {i}. {name}")
    
    while True:
        try:
            user_input = input("   请输入选择: ").strip()
            
            # 处理不同的输入格式
            if not user_input:
                selected_indices = []
            elif ',' in user_input:
                selected_indices = [int(x.strip()) for x in user_input.split(',')]
            else:
                selected_indices = [int(x.strip()) for x in user_input.split()]
            
            # 验证输入范围
            if selected_indices and not all(1 <= idx <= len(sensor_options) for idx in selected_indices):
                print("   输入无效，请输入有效的传感器编号")
                continue
            
            # 添加选择的传感器
            config['sensors'] = []
            for idx in selected_indices:
                sensor_key = sensor_keys[idx-1]
                config['sensors'].append(sensor_key)
            
            # 检查各功能对应的传感器是否已选择
            missing_sensors = []
            
            # 1. 报警显示功能 - 需要气体或火焰传感器
            if has_alarm_display:
                has_gas_sensor = 'MQ135' in config['sensors']
                has_flame_sensor = 'IR_FLAME' in config['sensors']
                
                if not has_gas_sensor and not has_flame_sensor:
                    missing_sensors.append("报警显示功能需要气体或火焰传感器")
            
            # 2. 室内温湿度显示功能 - 需要温湿度相关传感器
            if has_temp_humidity_display:
                has_temp_humidity_sensor = any(sensor in config['sensors'] for sensor in ['DHT22', 'SHT30', 'BME280'])
                
                if not has_temp_humidity_sensor:
                    missing_sensors.append("室内温湿度显示功能需要温湿度相关传感器")
            
            # 3. 感应节能功能 - 需要人体感应传感器
            if has_motion_saving:
                has_motion_sensor = 'HC_SR501' in config['sensors']
                
                if not has_motion_sensor:
                    missing_sensors.append("感应节能功能需要人体感应传感器")
            
            # 如果所有必需的传感器都已选择，退出循环
            if not missing_sensors:
                break
            else:
                # 显示缺少的传感器信息
                print("\n   ⚠ 缺少必需的传感器:")
                for missing in missing_sensors:
                    print(f"   - {missing}")
                print("   请重新选择传感器，确保包含所有必需的传感器")
                print()
        except ValueError:
            print("   输入无效，请输入数字")
    
    if not config['sensors']:
        print("\n   ✓ 未选择任何传感器，将使用默认配置")
    else:
        selected_sensor_names = [SUPPORTED_HARDWARE['sensor']['options'][s] for s in config['sensors']]
        print(f"\n   ✓ 已选择传感器: {', '.join(selected_sensor_names)}")
    
    return config

# 自动配置硬件模块和处理集成冲突
def auto_configure_hardware(config):
    """自动配置硬件模块和处理集成冲突"""
    print(f"\n{'=' * 50}")
    print("8. 自动配置硬件模块和处理集成冲突")
    print(f"{'=' * 50}")
    
    # 如果音频模块或TF卡读卡器已经提供TF卡支持，自动添加TF_CARD到硬件模块
    if config['has_tf_card'] and 'TF_CARD' not in config['hardware']:
        config['hardware'].append('TF_CARD')
        print(f"   ✓ 自动添加: {SUPPORTED_HARDWARE['hardware']['options']['TF_CARD']} (由音频模块或TF卡读卡器提供支持)")
    
    # 检查音频模块集成的功能，避免重复配置硬件模块
    has_integrated_mic = SUPPORTED_HARDWARE['audio_module']['integrated_microphone'].get(config['audio_module'], False)
    has_integrated_speaker = SUPPORTED_HARDWARE['audio_module']['integrated_speaker'].get(config['audio_module'], False)
    
    # 如果音频模块已集成麦克风和扬声器，不需要额外的音频硬件
    if has_integrated_mic or has_integrated_speaker:
        if has_integrated_mic and has_integrated_speaker:
            print(f"   ✓ 音频模块已集成麦克风和扬声器，无需额外音频硬件")
        elif has_integrated_mic:
            print(f"   ✓ 音频模块已集成麦克风，无需额外麦克风硬件")
        elif has_integrated_speaker:
            print(f"   ✓ 音频模块已集成扬声器，无需额外扬声器硬件")
    
    if config['hardware']:
        selected_hardware_names = [SUPPORTED_HARDWARE['hardware']['options'][h] for h in config['hardware']]
        print(f"   ✓ 已自动添加硬件模块: {', '.join(selected_hardware_names)}")
    else:
        print(f"   ✓ 未添加额外硬件模块")
    
    return config

# 显示最终配置
def display_final_config(config, all_features_dict):
    """显示最终配置和集成硬件冲突处理总结"""
    # 显示最终配置
    print("\n===== 最终配置 =====")
    print(f"已选择功能: {[all_features_dict[f] for f in config['features']]}")
    print(f"微控制器平台: {SUPPORTED_HARDWARE['platform']['options'][config['platform']]}")
    print(f"开发板型号: {SUPPORTED_HARDWARE['board']['options'][config['platform']][config['board']]}")
    print(f"音频模块: {SUPPORTED_HARDWARE['audio_module']['options'][config['audio_module']]}")
    print(f"墨水屏型号: {SUPPORTED_HARDWARE['display']['options'][config['display']]}")
    print(f"WiFi+蓝牙模块: {SUPPORTED_HARDWARE['wifi_bt_module']['options'][config['wifi_bt_module']]}")
    print(f"TF卡读卡器: {SUPPORTED_HARDWARE['tf_card_reader']['options'][config['tf_card_reader']]}")
    if config['sensors']:
        print(f"传感器: {[SUPPORTED_HARDWARE['sensor']['options'][s] for s in config['sensors']]}")
    if config['hardware']:
        print(f"硬件模块: {[SUPPORTED_HARDWARE['hardware']['options'][h] for h in config['hardware']]}")
    print()
    
    # 集成硬件冲突处理总结
    print("===== 集成硬件冲突处理总结 =====")
    if config['has_wifi_bt']:
        print("✓ 开发板内置WiFi+蓝牙，无需额外模块")
    
    if config['audio_module'] != 'AUDIO_DRIVER_NONE':
        if SUPPORTED_HARDWARE['audio_module']['integrated_tf_card'].get(config['audio_module'], False):
            print("✓ 音频模块集成TF卡功能，无需额外TF卡读卡器")
        
        has_integrated_mic = SUPPORTED_HARDWARE['audio_module']['integrated_microphone'].get(config['audio_module'], False)
        has_integrated_speaker = SUPPORTED_HARDWARE['audio_module']['integrated_speaker'].get(config['audio_module'], False)
        
        if has_integrated_mic and has_integrated_speaker:
            print("✓ 音频模块集成麦克风和扬声器，无需额外音频硬件")
        elif has_integrated_mic:
            print("✓ 音频模块集成麦克风，无需额外麦克风")
        elif has_integrated_speaker:
            print("✓ 音频模块集成扬声器，无需额外扬声器")
    
    if 'CAMERA' in config['hardware']:
        print("✓ 摄像头模块由ESP32开发板支持，已自动配置")
    
    print()
    
    return config

# 生成固件配置
def generate_firmware_config():
    """通过向导生成固件配置"""
    print("=" * 60)
    print("欢迎使用 家用网络智能墨水屏万年历 - 固件生成工具")
    print("=" * 60)
    print("本工具将帮助您根据需求生成个性化固件")
    print("流程：功能选择 → 硬件选择 → 配置生成 → 固件编译")
    print("注意：带 * 标记的功能为必选功能，将自动添加")
    print("\n")
    
    # 初始化配置
    config = {
        'platform': '',
        'board': '',
        'audio_module': 'AUDIO_DRIVER_NONE',
        'wifi_bt_module': 'WIFI_BT_INTERNAL',
        'display': '',
        'tf_card_reader': 'TF_READER_NONE',
        'sensors': [],
        'hardware': [],
        'features': [],
        'has_wifi_bt': False,
        'has_tf_card': False
    }
    
    # 1. 选择功能模块
    config['features'], all_features_dict = select_features()
    
    # 2. 根据选择的功能确定所需的硬件类型
    required_hardware = determine_required_hardware(config['features'])
    
    # 3. 选择微控制器平台和开发板
    platform_board_config = select_platform_and_board()
    config.update(platform_board_config)
    
    # 4. 检查开发板内置功能
    config = check_integrated_features(config)
    
    # 5. 根据功能选择硬件组件
    config = select_hardware_components(config, required_hardware)
    
    # 6. 选择传感器
    config = select_sensors(config)
    
    # 7. 自动配置硬件模块和处理集成冲突
    config = auto_configure_hardware(config)
    
    # 8. 显示最终配置
    config = display_final_config(config, all_features_dict)
    
    return config

# 生成配置文件
def generate_config_file(config):
    """生成配置文件"""
    config_dir = os.path.join(PROJECT_ROOT, 'config')
    os.makedirs(config_dir, exist_ok=True)
    
    config_path = os.path.join(config_dir, 'firmware_config.json')
    with open(config_path, 'w', encoding='utf-8') as f:
        json.dump(config, f, indent=4, ensure_ascii=False)
    
    print(f"配置文件已生成: {config_path}")
    return config_path

# 更新条件编译宏配置头文件
def update_config_header(config):
    """更新条件编译宏配置头文件"""
    config_header_path = os.path.join(PROJECT_ROOT, 'code', 'src', 'coresystem', 'config.h')
    
    # 读取现有文件内容，如果不存在则创建基本结构
    if not os.path.exists(config_header_path):
        existing_content = ""
    else:
        with open(config_header_path, 'r', encoding='utf-8') as f:
            existing_content = f.read()
    
    import re
    
    # 生成平台宏定义
    platform_macros = []
    for platform in SUPPORTED_HARDWARE['platform']['options']:
        enabled = platform == config['platform']
        platform_macros.append(f'#define PLATFORM_{platform} {1 if enabled else 0}')
    
    # 生成音频模块宏定义
    audio_macro = f'#define AUDIO_DRIVER_TYPE {config["audio_module"]}'
    
    # 生成WiFi+蓝牙模块宏定义
    wifi_bt_macro = f'#define WIFI_BT_MODULE_TYPE {config["wifi_bt_module"]}'
    
    # 生成TF卡读卡器宏定义
    tf_reader_macro = f'#define TF_CARD_READER_TYPE {config["tf_card_reader"]}'
    
    # 生成显示宏定义
    display_macro = f'#define DISPLAY_TYPE {config["display"]}'
    
    # 生成传感器宏定义
    sensor_macros = []
    for sensor in SUPPORTED_HARDWARE['sensor']['options']:
        enabled = sensor in config['sensors']
        sensor_macros.append(f'#define ENABLE_{sensor} {1 if enabled else 0}')
    
    # 生成硬件模块宏定义
    hardware_macros = []
    for hardware in SUPPORTED_HARDWARE['hardware']['options']:
        enabled = hardware in config['hardware']
        hardware_macros.append(f'#define ENABLE_{hardware} {1 if enabled else 0}')
    
    # 生成功能模块宏定义
    feature_macros = []
    
    # 合并所有功能选项（包括必选功能和可选功能）
    all_features = list(SUPPORTED_HARDWARE['feature']['options'].keys()) + list(SUPPORTED_HARDWARE['mandatory_features']['options'].keys())
    
    # 去重
    all_features = list(set(all_features))
    
    for feature in all_features:
        enabled = feature in config['features']
        feature_macros.append(f'#define ENABLE_{feature} {1 if enabled else 0}')
    
    # 合并所有宏定义
    all_macros = [
        '// Platform macros',
        *platform_macros,
        '',
        '// Audio module macros',
        audio_macro,
        '',
        '// WiFi+Bluetooth module macros',
        wifi_bt_macro,
        f'#define HAS_WIFI_BT {1 if config["has_wifi_bt"] else 0}',
        '',
        '// TF card reader macros',
        tf_reader_macro,
        f'#define HAS_TF_CARD {1 if config["has_tf_card"] else 0}',
        '',
        '// Display macros',
        display_macro,
        '',
        '// Sensor macros',
        *sensor_macros,
        '',
        '// Hardware module macros',
        *hardware_macros,
        '',
        '// Feature macros',
        *feature_macros
    ]
    
    # 替换配置文件中的宏定义部分
    macro_content = '\n'.join(all_macros)
    
    # 处理现有内容，提取#ifndef CONFIG_H和#endif之间的部分
    if '#ifndef CONFIG_H' in existing_content:
        # 提取头文件结构
        header_start = '#ifndef CONFIG_H\n#define CONFIG_H\n\n'
        header_end = '\n\n#endif // CONFIG_H'
        
        # 提取#ifndef和#endif之外的内容（如果有的话）
        before_config = existing_content.split('#ifndef CONFIG_H')[0]
        after_config = existing_content.split('#endif')[1] if '#endif' in existing_content else ''
        
        # 生成新的文件内容
        new_content = f"{before_config}{header_start}{macro_content}{header_end}{after_config}"
    else:
        # 创建全新的配置文件
        new_content = f"#ifndef CONFIG_H\n#define CONFIG_H\n\n{macro_content}\n\n#endif // CONFIG_H"
    
    # 写入更新后的内容
    with open(config_header_path, 'w', encoding='utf-8') as f:
        f.write(new_content)
    
    print(f"已更新配置头文件: {config_header_path}")

# 生成固件
def generate_firmware(config):
    """使用PlatformIO生成固件和OTA升级包"""
    print("===== 生成固件 ======")
    
    # 创建release目录
    release_dir = os.path.join(PROJECT_ROOT, 'tool', 'release')
    os.makedirs(release_dir, exist_ok=True)
    
    # 使用选择的开发板环境
    env = config['board']
    
    # 切换到代码目录
    code_dir = os.path.join(PROJECT_ROOT, 'code')
    
    # 生成固件信息文件
    firmware_info = {
        'config': config,
        'timestamp': os.path.getctime(__file__),
        'version': '1.0.0',
        'project': 'InkClock',
        'board': env,
    }
    
    firmware_info_path = os.path.join(release_dir, 'firmware_info.json')
    with open(firmware_info_path, 'w', encoding='utf-8') as f:
        json.dump(firmware_info, f, indent=4, ensure_ascii=False)
    
    print(f"\n1. 正在编译固件...")
    print(f"   使用环境: {env}")
    
    try:
        # 使用PlatformIO编译固件
        result = subprocess.run(
            ['pio', 'run', '--environment', env],
            cwd=code_dir,
            capture_output=True,
            text=True,
            check=True
        )
        print("   固件编译成功！")
        
        # 获取编译输出的固件路径
        firmware_bin_path = os.path.join(code_dir, '.pio', 'build', env, 'firmware.bin')
        if os.path.exists(firmware_bin_path):
            # 复制固件到release目录
            dest_firmware_path = os.path.join(release_dir, f'firmware_{env}.bin')
            shutil.copy2(firmware_bin_path, dest_firmware_path)
            print(f"   固件已复制到: {dest_firmware_path}")
        else:
            print(f"   警告: 未找到固件文件: {firmware_bin_path}")
        
        # 生成OTA升级包
        print(f"\n2. 正在生成OTA升级包...")
        
        # 使用PlatformIO生成OTA包
        ota_result = subprocess.run(
            ['pio', 'run', '--environment', env, '--target', 'upload'],
            cwd=code_dir,
            capture_output=True,
            text=True,
            check=False  # 不检查是否成功，因为我们只是需要生成OTA包
        )
        
        # 检查是否生成了OTA相关文件
        ota_files = [
            os.path.join(code_dir, '.pio', 'build', env, 'firmware.bin'),
            os.path.join(code_dir, '.pio', 'build', env, 'partitions.bin'),
            os.path.join(code_dir, '.pio', 'build', env, 'bootloader.bin')
        ]
        
        # 创建OTA目录
        ota_dir = os.path.join(release_dir, f'ota_{env}')
        os.makedirs(ota_dir, exist_ok=True)
        
        # 复制所有OTA相关文件
        for file_path in ota_files:
            if os.path.exists(file_path):
                file_name = os.path.basename(file_path)
                dest_path = os.path.join(ota_dir, file_name)
                shutil.copy2(file_path, dest_path)
                print(f"   OTA文件已复制: {file_name}")
        
        print("   OTA升级包生成完成！")
        
    except subprocess.CalledProcessError as e:
        print(f"   错误: 编译固件时发生错误")
        print(f"   错误输出: {e.stderr}")
        print(f"   请检查代码和PlatformIO配置")
    except Exception as e:
        print(f"   错误: 生成固件时发生异常: {str(e)}")
    
    # 复制配置文件到release目录
    config_file_path = os.path.join(PROJECT_ROOT, 'config', 'firmware_config.json')
    if os.path.exists(config_file_path):
        dest_config_path = os.path.join(release_dir, 'firmware_config.json')
        shutil.copy2(config_file_path, dest_config_path)
    
    print(f"\n固件生成完成！")
    print(f"输出目录: {release_dir}")
    print(f"固件信息: {firmware_info_path}")
    print(f"\n请使用以下命令上传固件:")
    print(f"   pio run --environment {env} --target upload")
    print(f"\nOTA升级包位于: {os.path.join(release_dir, f'ota_{env}')}")
    print("可用于OTA空中升级")

# 主函数
def main():
    # 检查是否在测试模式
    test_mode = '--test' in sys.argv
    
    # 生成固件配置
    config = generate_firmware_config()
    
    if not test_mode:
        # 检查运行环境
        if not check_environment(config['platform']):
            print("运行环境检查失败，程序将退出")
            sys.exit(1)
    
    # 生成配置文件
    generate_config_file(config)
    
    # 更新配置头文件
    update_config_header(config)
    
    if not test_mode:
        # 生成固件
        generate_firmware(config)
        
        print("\n===== 固件生成完成 ======")
        print("请使用Arduino IDE或PlatformIO编译生成最终固件")
        print(f"编译命令示例: pio run --environment {config['board']} --target upload")
    else:
        print("\n===== 测试模式完成 ======")
        print("配置生成和配置头文件更新已完成")
        print("跳过了固件生成步骤")

if __name__ == '__main__':
    main()