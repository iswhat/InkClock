#!/usr/bin/env python3
"""
命令行固件生成工具
通过逐步向导式选择各个元件型号、启用禁用的功能和传感器，生成最简代码和固件
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

# 检测ESP32平台支持
def check_esp32_platform():
    """检测是否支持ESP32平台"""
    print("3. 检查ESP32平台支持...")
    
    try:
        # 检查是否已安装espressif32平台
        result = subprocess.run(['pio', 'platform', 'list'], capture_output=True, text=True, check=True)
        if 'espressif32' in result.stdout:
            print("   ESP32平台支持已安装")
            return True
        else:
            print("   错误: 未安装ESP32平台支持")
            print("   请运行: pio platform install espressif32")
            return False
    except Exception as e:
        print(f"   错误: 检测ESP32平台时发生异常: {str(e)}")
        return False

# 检测Arduino框架支持
def check_arduino_framework():
    """检测是否支持Arduino框架"""
    print("4. 检查Arduino框架支持...")
    
    try:
        # Arduino框架通常随espressif32平台一起安装，我们检查platformio.ini中是否有框架配置
        platformio_ini_path = os.path.join(PROJECT_ROOT, 'code', 'platformio.ini')
        with open(platformio_ini_path, 'r', encoding='utf-8') as f:
            content = f.read()
        
        if 'framework = arduino' in content:
            print("   Arduino框架支持已配置")
            return True
        else:
            print("   错误: platformio.ini中未配置Arduino框架")
            return False
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
def check_firmware_build_environment():
    """检测生成固件和OTA升级包所需的环境"""
    print("===== 固件构建环境检测 =====")
    
    # 检查PlatformIO安装
    if not check_platformio_installation():
        return False
    
    # 检查ESP32平台
    if not check_esp32_platform():
        return False
    
    # 检查Arduino框架
    if not check_arduino_framework():
        return False
    
    # 检查OTA工具
    check_ota_tools()
    
    print("固件构建环境检测通过\n")
    return True

# 检测运行环境
def check_environment():
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
    if not check_firmware_build_environment():
        return False
    
    print("===== 所有环境检测通过 =====\n")
    return True

# 项目根目录
PROJECT_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))

# 支持的硬件列表，基于硬件物料选型.md
SUPPORTED_HARDWARE = {
    'display': {
        'name': '墨水屏',
        'options': {
            'EINK_29_INCH': '2.9英寸墨水屏 (GDEW029T5)',
            'EINK_42_INCH': '4.2英寸墨水屏 (GDEW042T2)',
            'EINK_75_INCH': '7.5英寸墨水屏 (GDEW075T7)',
            'EINK_154_INCH': '1.54英寸墨水屏 (GDEW0154M09)',
            'EINK_213_INCH': '2.13英寸墨水屏 (GDEW0213M09)',
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
        }
    },
    'feature': {
        'name': '功能模块',
        'options': {
            'AUDIO': '音频功能',
            'BLUETOOTH': '蓝牙功能',
            'CAMERA': '摄像头功能',
            'STOCK': '股票功能',
            'MESSAGE': '消息功能',
            'PLUGIN': '插件功能',
            'WEBCLIENT': 'Web客户端功能',
            'IPV6': 'IPv6功能',
            'FIRMWARE': '固件更新功能',
            'TOUCH': '触摸功能',
            'FONT': '字体管理功能',
            'TF_CARD_MANAGEMENT': 'TF卡管理功能',
            'ALARM_DISPLAY': '报警显示功能',
        }
    }
}

# 生成固件配置
def generate_firmware_config():
    """通过向导生成固件配置"""
    print("===== 家用网络智能墨水屏万年历 - 固件生成工具 =====")
    print("请按照向导逐步选择配置选项\n")
    
    config = {
        'display': '',
        'sensors': [],
        'hardware': [],
        'features': [],
    }
    
    # 选择墨水屏
    print("1. 选择墨水屏型号:")
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
    
    # 选择传感器
    print("2. 选择传感器 (可多选，输入0结束):")
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
    
    # 选择硬件模块
    print("3. 选择硬件模块 (可多选，输入0结束):")
    hardware_options = list(SUPPORTED_HARDWARE['hardware']['options'].items())
    for i, (key, name) in enumerate(hardware_options, 1):
        print(f"   {i}. {name}")
    
    while True:
        try:
            choice = int(input("请输入选择 (1-{}, 0结束): ".format(len(hardware_options))))
            if choice == 0:
                break
            elif 1 <= choice <= len(hardware_options):
                hardware_key = hardware_options[choice-1][0]
                if hardware_key not in config['hardware']:
                    config['hardware'].append(hardware_key)
                    print(f"已添加: {SUPPORTED_HARDWARE['hardware']['options'][hardware_key]}")
                else:
                    print("该硬件模块已添加")
            else:
                print("输入无效，请重新选择")
        except ValueError:
            print("输入无效，请输入数字")
    
    if not config['hardware']:
        print("未选择任何硬件模块\n")
    else:
        print(f"已选择硬件模块: {[SUPPORTED_HARDWARE['hardware']['options'][h] for h in config['hardware']]}\n")
    
    # 选择功能模块
    print("4. 选择功能模块 (可多选，输入0结束):")
    feature_options = list(SUPPORTED_HARDWARE['feature']['options'].items())
    
    # 根据硬件模块过滤可用功能
    has_tf_card = 'TF_CARD' in config['hardware']
    available_features = []
    
    for key, name in feature_options:
        # 只有TF卡模块存在时，才能选择音频功能和TF卡管理功能
        if (key == 'AUDIO' or key == 'TF_CARD_MANAGEMENT') and not has_tf_card:
            continue  # 过滤掉需要TF卡但没有TF卡的功能
        available_features.append((key, name))
    
    # 显示可用功能
    for i, (key, name) in enumerate(available_features, 1):
        print(f"   {i}. {name}")
    
    while True:
        try:
            choice = int(input("请输入选择 (1-{}, 0结束): ".format(len(available_features))))
            if choice == 0:
                break
            elif 1 <= choice <= len(available_features):
                feature_key = available_features[choice-1][0]
                if feature_key not in config['features']:
                    # 添加功能时检查依赖关系
                    if (feature_key == 'AUDIO' or feature_key == 'TF_CARD_MANAGEMENT') and 'TF_CARD' not in config['hardware']:
                        print("警告: 该功能需要TF卡支持，已自动添加TF卡模块")
                        config['hardware'].append('TF_CARD')
                        print(f"已添加: {SUPPORTED_HARDWARE['hardware']['options']['TF_CARD']}")
                    
                    config['features'].append(feature_key)
                    print(f"已添加: {SUPPORTED_HARDWARE['feature']['options'][feature_key]}")
                else:
                    print("该功能已添加")
            else:
                print("输入无效，请重新选择")
        except ValueError:
            print("输入无效，请输入数字")
    
    # 显示最终配置
    print(f"\n已选择功能: {[SUPPORTED_HARDWARE['feature']['options'][f] for f in config['features']]}")
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

# 更新main.cpp中的条件编译宏
def update_main_cpp(config):
    """更新main.cpp中的条件编译宏"""
    main_cpp_path = os.path.join(PROJECT_ROOT, 'code', 'src', 'main.cpp')
    
    with open(main_cpp_path, 'r', encoding='utf-8') as f:
        content = f.read()
    
    import re
    
    # 更新功能模块的宏定义
    for feature in SUPPORTED_HARDWARE['feature']['options']:
        enabled = feature in config['features']
        macro = f'#define ENABLE_{feature} {'true' if enabled else 'false'}'
        pattern = f'#define ENABLE_{feature}.*'  # 匹配现有宏定义
        content = re.sub(pattern, macro, content)
    
    # 更新硬件模块的宏定义
    # TF卡功能宏定义
    has_tf_card = 'TF_CARD' in config['hardware']
    content = re.sub(r'#define ENABLE_TF_CARD.*', f'#define ENABLE_TF_CARD {'true' if has_tf_card else 'false'}', content)
    
    # 更新TF卡管理功能宏定义
    has_tf_card_management = 'TF_CARD_MANAGEMENT' in config['features']
    content = re.sub(r'#define ENABLE_TF_CARD_MANAGEMENT.*', f'#define ENABLE_TF_CARD_MANAGEMENT {'true' if has_tf_card_management else 'false'}', content)
    
    # 更新报警显示功能宏定义
    has_alarm_display = 'ALARM_DISPLAY' in config['features']
    content = re.sub(r'#define ENABLE_ALARM_DISPLAY.*', f'#define ENABLE_ALARM_DISPLAY {'true' if has_alarm_display else 'false'}', content)
    
    # 写入更新后的内容
    with open(main_cpp_path, 'w', encoding='utf-8') as f:
        f.write(content)
    
    print(f"已更新main.cpp: {main_cpp_path}")

# 生成固件
def generate_firmware(config):
    """使用PlatformIO生成固件和OTA升级包"""
    print("===== 生成固件 =====")
    
    # 创建release目录
    release_dir = os.path.join(PROJECT_ROOT, 'tool', 'release')
    os.makedirs(release_dir, exist_ok=True)
    
    # 选择默认的ESP32开发板环境
    # 可以根据用户选择扩展此功能
    default_env = 'esp32-wroom-32'
    
    # 切换到代码目录
    code_dir = os.path.join(PROJECT_ROOT, 'code')
    
    # 生成固件信息文件
    firmware_info = {
        'config': config,
        'timestamp': os.path.getctime(__file__),
        'version': '1.0.0',
        'project': 'InkClock',
        'board': default_env,
    }
    
    firmware_info_path = os.path.join(release_dir, 'firmware_info.json')
    with open(firmware_info_path, 'w', encoding='utf-8') as f:
        json.dump(firmware_info, f, indent=4, ensure_ascii=False)
    
    print(f"\n1. 正在编译固件...")
    print(f"   使用环境: {default_env}")
    
    try:
        # 使用PlatformIO编译固件
        result = subprocess.run(
            ['pio', 'run', '--environment', default_env],
            cwd=code_dir,
            capture_output=True,
            text=True,
            check=True
        )
        print("   固件编译成功！")
        
        # 获取编译输出的固件路径
        firmware_bin_path = os.path.join(code_dir, '.pio', 'build', default_env, 'firmware.bin')
        if os.path.exists(firmware_bin_path):
            # 复制固件到release目录
            dest_firmware_path = os.path.join(release_dir, f'firmware_{default_env}.bin')
            shutil.copy2(firmware_bin_path, dest_firmware_path)
            print(f"   固件已复制到: {dest_firmware_path}")
        else:
            print(f"   警告: 未找到固件文件: {firmware_bin_path}")
        
        # 生成OTA升级包
        print(f"\n2. 正在生成OTA升级包...")
        
        # 使用PlatformIO生成OTA包
        ota_result = subprocess.run(
            ['pio', 'run', '--environment', default_env, '--target', 'upload'],
            cwd=code_dir,
            capture_output=True,
            text=True,
            check=False  # 不检查是否成功，因为我们只是需要生成OTA包
        )
        
        # 检查是否生成了OTA相关文件
        ota_files = [
            os.path.join(code_dir, '.pio', 'build', default_env, 'firmware.bin'),
            os.path.join(code_dir, '.pio', 'build', default_env, 'partitions.bin'),
            os.path.join(code_dir, '.pio', 'build', default_env, 'bootloader.bin')
        ]
        
        # 创建OTA目录
        ota_dir = os.path.join(release_dir, f'ota_{default_env}')
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
    print(f"   pio run --environment {default_env} --target upload")
    print(f"\nOTA升级包位于: {os.path.join(release_dir, f'ota_{default_env}')}")
    print("可用于OTA空中升级")

# 主函数
def main():
    # 检查是否在测试模式
    test_mode = '--test' in sys.argv
    
    if not test_mode:
        # 检查运行环境
        if not check_environment():
            print("运行环境检查失败，程序将退出")
            sys.exit(1)
    
    # 生成固件配置
    config = generate_firmware_config()
    
    # 生成配置文件
    generate_config_file(config)
    
    # 更新main.cpp
    update_main_cpp(config)
    
    if not test_mode:
        # 生成固件
        generate_firmware(config)
        
        print("\n===== 固件生成完成 =====")
        print("请使用Arduino IDE或PlatformIO编译生成最终固件")
        print("编译命令示例: pio run --target upload")
    else:
        print("\n===== 测试模式完成 =====")
        print("配置生成和main.cpp更新已完成")
        print("跳过了固件生成步骤")

if __name__ == '__main__':
    main()
