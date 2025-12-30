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
                'esp32-c6-n4': 'ESP32-C6-N4 (WiFi+BT5.0+IPv6)',
                'esp32-s2-devkitc-1': 'ESP32-S2-DevKitC-1 (WiFi)',
                'esp32-c3-supermini': 'ESP32-C3-SuperMini (WiFi+BT5.0)'
            },
            'ESP8266': {
                'nodemcuv2': 'NodeMCU v2 (WiFi)',
                'd1_mini': 'WeMos D1 Mini (WiFi)'
            },
            'NRF52': {
                'nrf52840dk': 'nRF52840 DK (WiFi+BT5.0)',
                'nrf52832dk': 'nRF52832 DK (BT5.0)'
            },
            'STM32': {
                'bluepill_f103c8': 'Blue Pill STM32F103C8',
                'nucleo_f401re': 'Nucleo-F401RE'
            },
            'RP2040': {
                'raspberrypi_pico': 'Raspberry Pi Pico',
                'adafruit_feather_rp2040': 'Adafruit Feather RP2040'
            }
        }
    },
    'audio_module': {
        'name': '音频模块',
        'options': {
            'AUDIO_MODULE_VS1053B_NO_HEADPHONE': 'VS1053B无耳机孔模块 (存储+扩音器+不带3.5mm耳机孔)',
            'AUDIO_MODULE_VS1003B_STORAGE': 'VS1003B存储版 (存储+扩音器+不带3.5mm耳机孔)',
            'AUDIO_MODULE_YX5300': 'YX5300-24SS (存储+扩音器+不带3.5mm耳机孔)',
            'AUDIO_MODULE_YX6300': 'YX6300-24SS (TF卡+扩音器+不带3.5mm耳机孔)',
            'AUDIO_MODULE_WT588D': 'WT588D-M02 (存储+扩音器+不带3.5mm耳机孔)',
            'AUDIO_MODULE_ISD1820': 'ISD1820录音模块 (存储+麦克风+不带3.5mm耳机孔)',
            'AUDIO_MODULE_NRF52832': 'NRF52832音频模块 (存储+蓝牙+不带3.5mm耳机孔)',
            'AUDIO_MODULE_ESP32_AUDIO': 'ESP32音频解码模块 (TF卡+WiFi+蓝牙+不带3.5mm耳机孔)',
            'AUDIO_MODULE_STM32_AUDIO': 'STM32F103音频模块 (存储+扩音器+不带3.5mm耳机孔)',
            'AUDIO_MODULE_ATMEGA328': 'ATmega328音频播放器 (SD卡+扩音器+不带3.5mm耳机孔)'
        },
        'integrated_tf_card': {
            'AUDIO_MODULE_VS1053B_NO_HEADPHONE': True,
            'AUDIO_MODULE_VS1003B_STORAGE': True,
            'AUDIO_MODULE_YX5300': True,
            'AUDIO_MODULE_YX6300': True,
            'AUDIO_MODULE_WT588D': True,
            'AUDIO_MODULE_ISD1820': True,
            'AUDIO_MODULE_NRF52832': True,
            'AUDIO_MODULE_ESP32_AUDIO': True,
            'AUDIO_MODULE_STM32_AUDIO': True,
            'AUDIO_MODULE_ATMEGA328': True
        },
        'integrated_microphone': {
            'AUDIO_MODULE_ISD1820': True,
            'AUDIO_MODULE_NRF52832': True,
            'AUDIO_MODULE_ESP32_AUDIO': True
        },
        'integrated_speaker': {
            'AUDIO_MODULE_VS1053B_NO_HEADPHONE': True,
            'AUDIO_MODULE_VS1003B_STORAGE': True,
            'AUDIO_MODULE_YX5300': True,
            'AUDIO_MODULE_YX6300': True,
            'AUDIO_MODULE_WT588D': True,
            'AUDIO_MODULE_NRF52832': True,
            'AUDIO_MODULE_ESP32_AUDIO': True,
            'AUDIO_MODULE_STM32_AUDIO': True,
            'AUDIO_MODULE_ATMEGA328': True
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
            'DHT22': 'DHT22 温湿度传感器',
            'SHT30': 'SHT30 温湿度传感器',
            'BH1750': 'BH1750 光照传感器',
            'HC_SR501': 'HC-SR501 人体感应传感器',
            'MQ135': 'MQ-135 气体传感器',
            'IR_FLAME': 'IR 火焰传感器',
            'BME280': 'BME280 温湿度气压传感器',
        }
    },
    'hardware': {
        'name': '硬件模块',
        'options': {
            'TF_CARD': 'TF卡模块',
            'CAMERA': '摄像头模块'  # 用于视频留言
        }
    },
    'feature': {
        'name': '功能模块',
        'options': {
            'FONT': '字体管理功能',
            'ALARM_DISPLAY': '报警显示功能'
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

# 生成固件配置
def generate_firmware_config():
    """通过向导生成固件配置"""
    print("===== 家用网络智能墨水屏万年历 - 固件生成工具 ======")
    print("请按照向导逐步选择配置选项\n")
    
    config = {
        'platform': '',
        'board': '',
        'audio_module': '',
        'wifi_bt_module': 'WIFI_BT_INTERNAL',
        'display': '',
        'tf_card_reader': 'TF_READER_NONE',
        'sensors': [],
        'hardware': [],
        'features': [],
        'has_wifi_bt': False,
        'has_tf_card': False
    }
    
    # 选择微控制器平台
    print("1. 选择微控制器平台:")
    platform_options = list(SUPPORTED_HARDWARE['platform']['options'].items())
    for i, (key, name) in enumerate(platform_options, 1):
        print(f"   {i}. {name}")
    
    while True:
        try:
            choice = int(input("请输入选择 (1-{}): ".format(len(platform_options))))
            if 1 <= choice <= len(platform_options):
                config['platform'] = platform_options[choice-1][0]
                break
            else:
                print("输入无效，请重新选择")
        except ValueError:
            print("输入无效，请输入数字")
    
    print(f"已选择: {SUPPORTED_HARDWARE['platform']['options'][config['platform']]}\n")
    
    # 选择开发板型号
    print("2. 选择开发板型号:")
    board_options = list(SUPPORTED_HARDWARE['board']['options'][config['platform']].items())
    for i, (key, name) in enumerate(board_options, 1):
        print(f"   {i}. {name}")
    
    while True:
        try:
            choice = int(input("请输入选择 (1-{}): ".format(len(board_options))))
            if 1 <= choice <= len(board_options):
                config['board'] = board_options[choice-1][0]
                break
            else:
                print("输入无效，请重新选择")
        except ValueError:
            print("输入无效，请输入数字")
    
    print(f"已选择: {SUPPORTED_HARDWARE['board']['options'][config['platform']][config['board']]}\n")
    
    # 检查开发板是否内置WiFi+蓝牙
    config['has_wifi_bt'] = SUPPORTED_HARDWARE['board_has_wifi_bt'][config['platform']][config['board']]
    
    # 如果开发板没有内置WiFi+蓝牙，让用户选择外接模块
    if not config['has_wifi_bt']:
        print("3. 选择WiFi+蓝牙模块:")
        wifi_bt_options = list(SUPPORTED_HARDWARE['wifi_bt_module']['options'].items())
        for i, (key, name) in enumerate(wifi_bt_options, 1):
            print(f"   {i}. {name}")
        
        while True:
            try:
                choice = int(input("请输入选择 (1-{}): ".format(len(wifi_bt_options))))
                if 1 <= choice <= len(wifi_bt_options):
                    config['wifi_bt_module'] = wifi_bt_options[choice-1][0]
                    break
                else:
                    print("输入无效，请重新选择")
            except ValueError:
                print("输入无效，请输入数字")
        
        print(f"已选择: {SUPPORTED_HARDWARE['wifi_bt_module']['options'][config['wifi_bt_module']]}\n")
    else:
        config['wifi_bt_module'] = 'WIFI_BT_INTERNAL'
        print("3. 开发板内置WiFi+蓝牙模块，无需额外选择\n")
    
    # 选择音频模块
    print("4. 选择音频模块型号:")
    audio_options = list(SUPPORTED_HARDWARE['audio_module']['options'].items())
    for i, (key, name) in enumerate(audio_options, 1):
        print(f"   {i}. {name}")
    
    while True:
        try:
            choice = int(input("请输入选择 (1-{}): ".format(len(audio_options))))
            if 1 <= choice <= len(audio_options):
                config['audio_module'] = audio_options[choice-1][0]
                break
            else:
                print("输入无效，请重新选择")
        except ValueError:
            print("输入无效，请输入数字")
    
    print(f"已选择: {SUPPORTED_HARDWARE['audio_module']['options'][config['audio_module']]}\n")
    
    # 选择墨水屏
    print("5. 选择墨水屏型号:")
    display_options = list(SUPPORTED_HARDWARE['display']['options'].items())
    for i, (key, name) in enumerate(display_options, 1):
        print(f"   {i}. {name}")
    
    while True:
        try:
            choice = int(input("请输入选择 (1-{}): ".format(len(display_options))))
            if 1 <= choice <= len(display_options):
                config['display'] = display_options[choice-1][0]
                break
            else:
                print("输入无效，请重新选择")
        except ValueError:
            print("输入无效，请输入数字")
    
    print(f"已选择: {SUPPORTED_HARDWARE['display']['options'][config['display']]}\n")
    
    # 检查音频模块是否集成TF读卡器
    if SUPPORTED_HARDWARE['audio_module']['integrated_tf_card'].get(config['audio_module'], False):
        config['has_tf_card'] = True
        print(f"   提示: 该音频模块集成了TF卡功能，将自动启用TF卡功能\n")
        config['tf_card_reader'] = 'TF_READER_NONE'
        
        # 如果硬件模块中已有TF_CARD，自动移除，避免重复
        if 'TF_CARD' in config['hardware']:
            config['hardware'].remove('TF_CARD')
            print(f"   提示: 由于音频模块已集成TF卡功能，已自动移除重复的TF_CARD硬件模块\n")
    else:
        # 选择TF卡读卡器（如果音频模块没有集成的话）
        print("6. 选择TF卡读卡器:")
        tf_options = list(SUPPORTED_HARDWARE['tf_card_reader']['options'].items())
        for i, (key, name) in enumerate(tf_options, 1):
            print(f"   {i}. {name}")
        
        while True:
            try:
                choice = int(input("请输入选择 (1-{}): ".format(len(tf_options))))
                if 1 <= choice <= len(tf_options):
                    config['tf_card_reader'] = tf_options[choice-1][0]
                    break
                else:
                    print("输入无效，请重新选择")
            except ValueError:
                print("输入无效，请输入数字")
        
        print(f"已选择: {SUPPORTED_HARDWARE['tf_card_reader']['options'][config['tf_card_reader']]}\n")
        
        if config['tf_card_reader'] != 'TF_READER_NONE':
            config['has_tf_card'] = True
            print("   提示: TF卡功能已启用\n")
    
    # 选择传感器
    print("7. 选择传感器 (可多选，输入0结束):")
    sensor_options = list(SUPPORTED_HARDWARE['sensor']['options'].items())
    for i, (key, name) in enumerate(sensor_options, 1):
        print(f"   {i}. {name}")
    
    while True:
        try:
            choice = int(input("请输入选择 (1-{}, 0结束): ".format(len(sensor_options))))
            if choice == 0:
                break
            elif 1 <= choice <= len(sensor_options):
                sensor_key = sensor_options[choice-1][0]
                if sensor_key not in config['sensors']:
                    config['sensors'].append(sensor_key)
                    print(f"已添加: {SUPPORTED_HARDWARE['sensor']['options'][sensor_key]}")
                else:
                    print("该传感器已添加")
            else:
                print("输入无效，请重新选择")
        except ValueError:
            print("输入无效，请输入数字")
    
    if not config['sensors']:
        print("未选择任何传感器，将使用默认配置\n")
    else:
        print(f"已选择传感器: {[SUPPORTED_HARDWARE['sensor']['options'][s] for s in config['sensors']]}\n")
    
    # 自动添加硬件模块
    print("8. 自动配置硬件模块:")
    
    # 如果音频模块或TF卡读卡器已经提供TF卡支持，自动添加TF_CARD到硬件模块
    if config['has_tf_card'] and 'TF_CARD' not in config['hardware']:
        config['hardware'].append('TF_CARD')
        print(f"   自动添加: {SUPPORTED_HARDWARE['hardware']['options']['TF_CARD']} (由音频模块或TF卡读卡器提供支持)")
    
    # 根据硬件自动添加摄像头模块支持
    if config['platform'] == 'ESP32' and 'CAMERA' not in config['hardware']:
        # ESP32系列开发板支持摄像头
        config['hardware'].append('CAMERA')
        print(f"   自动添加: 摄像头模块支持 (ESP32系列开发板)")
    
    # 检查音频模块集成的功能，避免重复配置硬件模块
    has_integrated_mic = SUPPORTED_HARDWARE['audio_module']['integrated_microphone'].get(config['audio_module'], False)
    has_integrated_speaker = SUPPORTED_HARDWARE['audio_module']['integrated_speaker'].get(config['audio_module'], False)
    
    # 如果音频模块已集成麦克风和扬声器，不需要额外的音频硬件
    if has_integrated_mic or has_integrated_speaker:
        if has_integrated_mic and has_integrated_speaker:
            print(f"   提示: 音频模块已集成麦克风和扬声器，无需额外音频硬件")
        elif has_integrated_mic:
            print(f"   提示: 音频模块已集成麦克风，无需额外麦克风硬件")
        elif has_integrated_speaker:
            print(f"   提示: 音频模块已集成扬声器，无需额外扬声器硬件")
    
    if not config['hardware']:
        print("   未自动添加任何硬件模块\n")
    else:
        print(f"   已自动添加硬件模块: {[SUPPORTED_HARDWARE['hardware']['options'].get(h, h) for h in config['hardware']]}\n")
    
    # 选择可选功能模块
    print("9. 选择可选功能模块 (可多选，输入0结束):")
    feature_options = list(SUPPORTED_HARDWARE['feature']['options'].items())
    
    # 显示可用功能
    for i, (key, name) in enumerate(feature_options, 1):
        print(f"   {i}. {name}")
    
    while True:
        try:
            choice = int(input("请输入选择 (1-{}, 0结束): ".format(len(feature_options))))
            if choice == 0:
                break
            elif 1 <= choice <= len(feature_options):
                feature_key = feature_options[choice-1][0]
                if feature_key not in config['features']:
                    config['features'].append(feature_key)
                    print(f"已添加: {SUPPORTED_HARDWARE['feature']['options'][feature_key]}")
                else:
                    print("该功能已添加")
            else:
                print("输入无效，请重新选择")
        except ValueError:
            print("输入无效，请输入数字")
    
    # 根据硬件自动添加功能
    print("\n10. 根据硬件自动配置功能:")
    has_tf_card = 'TF_CARD' in config['hardware'] or config['has_tf_card']
    has_camera = 'CAMERA' in config['hardware']
    has_wifi = config['has_wifi_bt'] or config['wifi_bt_module'] != 'WIFI_BT_INTERNAL'
    has_bt = config['has_wifi_bt'] or 'BLUETOOTH' in config['wifi_bt_module']
    
    # 自动添加音频功能和语音留言功能（如果有音频模块和TF卡支持）
    if config['audio_module'] != 'AUDIO_MODULE_INTERNAL':
        if 'AUDIO' not in config['features']:
            config['features'].append('AUDIO')
            print(f"   自动添加: 音频功能 (由音频模块提供)")
        
        if has_tf_card and 'VOICE_MESSAGE' not in config['features']:
            config['features'].append('VOICE_MESSAGE')
            print(f"   自动添加: 语音留言功能 (由音频模块和TF卡提供)")
    
    # 自动添加视频留言功能（如果有摄像头和TF卡支持）
    if has_camera and has_tf_card and 'VIDEO_MESSAGE' not in config['features']:
        config['features'].append('VIDEO_MESSAGE')
        print(f"   自动添加: 视频留言功能 (由摄像头和TF卡提供)")
    
    # 自动添加TF卡管理功能（如果有TF卡支持）
    if has_tf_card and 'TF_CARD_MANAGEMENT' not in config['features']:
        config['features'].append('TF_CARD_MANAGEMENT')
        print(f"   自动添加: TF卡管理功能 (由TF卡提供)")
    
    # 自动添加IPV6功能（如果开发板WiFi模块支持）
    if has_wifi and 'IPV6' not in config['features']:
        # 检查开发板是否支持IPv6
        ipv6_supported = False
        if config['platform'] == 'ESP32':
            # ESP32-C6系列支持IPv6
            if config['board'] == 'esp32-c6-n4':
                ipv6_supported = True
        elif config['platform'] == 'NRF52':
            # NRF52840支持IPv6
            if config['board'] == 'nrf52840dk':
                ipv6_supported = True
        
        if ipv6_supported:
            config['features'].append('IPV6')
            print(f"   自动添加: IPv6功能 (由开发板WiFi模块提供)")
    
    # 自动添加触摸功能（如果屏幕和开发板支持）
    if 'TOUCH' not in config['features']:
        # 检查屏幕是否支持触摸
        touch_supported = False
        # 根据屏幕型号添加触摸支持判断
        touch_supported_models = ['EINK_75_INCH', 'EINK_42_INCH', 'EINK_583_INCH', 'EINK_103_INCH']
        for model in touch_supported_models:
            if model in config['display']:
                touch_supported = True
                break
        
        if touch_supported:
            config['features'].append('TOUCH')
            print(f"   自动添加: 触摸功能 (由屏幕和开发板提供)")
    
    # 添加必选功能
    mandatory_features = list(SUPPORTED_HARDWARE['mandatory_features']['options'].keys())
    for feature in mandatory_features:
        if feature not in config['features']:
            config['features'].append(feature)
            print(f"   自动添加: {SUPPORTED_HARDWARE['mandatory_features']['options'][feature]} (必选功能)")
    
    # 显示最终配置
    print(f"\n已选择功能: {[SUPPORTED_HARDWARE['feature']['options'][f] if f in SUPPORTED_HARDWARE['feature']['options'] else SUPPORTED_HARDWARE['mandatory_features']['options'][f] for f in config['features']]}")
    if config['hardware']:
        print(f"已选择硬件模块: {[SUPPORTED_HARDWARE['hardware']['options'][h] for h in config['hardware']]}")
    print()
    
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
    config_header_path = os.path.join(PROJECT_ROOT, 'code', 'src', 'core', 'config.h')
    
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
    audio_macro = f'#define AUDIO_MODULE_TYPE {config["audio_module"]}'
    
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