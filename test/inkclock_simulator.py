#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
InkClock模拟器 - Python版

一个简单的InkClock模拟器，用于测试设备端的界面和功能
"""

import tkinter as tk
from tkinter import ttk
import time
import datetime
import random

class InkClockSimulator:
    def __init__(self, root):
        self.root = root
        self.root.title("InkClock模拟器 v1.0.0")
        self.root.geometry("600x500")
        self.root.resizable(True, True)
        self.root.minsize(500, 400)  # 设置最小窗口大小
        
        # 设备状态
        self.power_on = False
        self.current_mode = "clock"
        self.brightness = 70
        self.update_interval = 30
        
        # 传感器数据
        self.temperature = 22
        self.humidity = 50
        self.pressure = 1013
        self.light_level = 200
        self.battery_level = 80
        self.motion_detected = False
        self.gas_level = 450  # 气体传感器数据
        self.flame_detected = False  # 火焰检测
        
        # WiFi状态
        self.wifi_status = "Connected"
        
        # 设备编码（12位纯数字）
        self.device_code = ""
        
        # 墨水屏尺寸选择
        self.screen_sizes = {
            "2.13" : (250, 122),    # 2.13英寸墨水屏
            "2.9"  : (296, 128),    # 2.9英寸墨水屏
            "4.2"  : (400, 300),    # 4.2英寸墨水屏
            "5.83" : (648, 480),    # 5.83英寸墨水屏
            "7.5"  : (800, 480)     # 7.5英寸墨水屏
        }
        self.current_screen_size = "4.2"  # 默认4.2英寸
        self.screen_width = self.screen_sizes[self.current_screen_size][0]
        self.screen_height = self.screen_sizes[self.current_screen_size][1]
        
        # 管理系统API配置
        self.api_config = {
            "base_url": "http://localhost:8000",  # 管理系统API基础URL
            "device_register_endpoint": "/api/device/register",
            "device_status_endpoint": "/api/device/status",
            "device_bind_endpoint": "/api/device/bind",
            "message_push_endpoint": "/api/device/message/push",
            "message_list_endpoint": "/api/device/message/list"
        }
        
        # 设备状态
        self.registered = False
        self.bound = False  # 设备绑定状态
        self.bound_user = ""
        
        # 消息数据
        self.messages = []  # 存储接收到的消息
        self.new_messages = []  # 新接收到的消息，用于显示提醒
        self.message_refresh_interval = 10  # 消息刷新间隔（秒）
        self.last_message_check = 0  # 上次检查消息的时间
        
        # 显示配置 - 基于真实固件
        self.current_right_page = "calendar"  # 右侧页面：calendar, stock, message, setting
        self.current_clock_mode = "digital"  # 时钟模式：digital, analog, text
        self.show_seconds = True  # 是否显示秒
        self.show_message_notification = False  # 是否显示消息通知
        
        # 左侧面板宽度配置（基于真实固件）
        self.left_panel_width = 0
        self.right_panel_width = 0
        self.calculate_panel_widths()
        
        # 初始化界面
        self.create_widgets()
        
        # 添加键盘事件处理
        self.root.bind("<Key>", self.on_key_press)
        
        # 开始更新定时器
        self.update_timer = None
        self.update_display()
        
        # 开始消息检查定时器
        self.message_timer = None
        self.start_message_check()
    
    def calculate_panel_widths(self):
        """根据屏幕宽度计算左右面板宽度"""
        # 基于真实固件的逻辑
        if self.screen_width < 600:
            # 小屏幕：左侧面板宽度约为总宽度的1/2
            self.left_panel_width = self.screen_width // 2
        else:
            # 大屏幕：左侧面板宽度约为总宽度的1/3
            self.left_panel_width = self.screen_width // 3
        self.right_panel_width = self.screen_width - self.left_panel_width
    
    def update_display(self):
        """更新显示内容，高保真模拟真实固件的左右分栏布局"""
        if not self.power_on:
            # 设备关机，显示黑屏
            display_text = "设备已关机\n\n按电源开关启动设备"
            self.display_label.config(bg="#000000", fg="#ffffff", text=display_text, justify=tk.LEFT)
            self.status_var.set(f"设备状态: 关机 | 亮度: {self.brightness}% | 更新间隔: {self.update_interval}秒")
            return
        
        # 设备开机，显示对应模式的内容
        self.display_label.config(bg="#f0f0f0", fg="#000000", justify=tk.LEFT)
        
        current_time = datetime.datetime.now()
        
        # 计算左右面板宽度
        self.calculate_panel_widths()
        
        # 根据屏幕宽度计算字符宽度（每个字符约占8像素）
        left_chars = self.left_panel_width // 8
        right_chars = self.right_panel_width // 8
        
        # 构建显示内容，精确模拟真实固件的左右分栏布局
        display_lines = []
        
        # 获取左右面板内容
        left_lines = self.draw_left_panel(current_time, left_chars)
        right_lines = self.draw_right_panel(current_time, right_chars)
        
        # 确保左右面板行数一致
        max_lines = max(len(left_lines), len(right_lines))
        left_lines += ["" for _ in range(max_lines - len(left_lines))]
        right_lines += ["" for _ in range(max_lines - len(right_lines))]
        
        # 合并左右面板内容，添加分隔线
        for left, right in zip(left_lines, right_lines):
            # 左侧内容左对齐，右侧内容左对齐，中间加分隔线
            line = f"{left:<{left_chars}}│ {right:<{right_chars}}"
            display_lines.append(line)
        
        display_text = "\n".join(display_lines)
        
        self.display_label.config(text=display_text)
        self.status_var.set(f"设备状态: 开机 | 模式: {self.current_mode} | 亮度: {self.brightness}% | 更新间隔: {self.update_interval}秒 | 电池: {self.battery_level}%")
        
        # 更新定时器
        if self.update_timer:
            self.root.after_cancel(self.update_timer)
        
        self.update_timer = self.root.after(1000, self.update_display)
    
    def draw_left_panel(self, current_time, width_chars):
        """绘制左侧面板内容，精确模拟真实固件"""
        left_lines = []
        
        # 绘制时钟（根据当前时钟模式）
        if self.current_clock_mode == "digital":
            # 数字时钟模式 - 大字体显示
            time_str = current_time.strftime("%H:%M:%S")
            date_str = current_time.strftime("%Y-%m-%d %A")
            left_lines.append(time_str)
            left_lines.append(date_str)
        elif self.current_clock_mode == "analog":
            # 模拟时钟模式 - 简化表示
            left_lines.append("模拟时钟")
            left_lines.append(f"{current_time.strftime('%H:%M:%S')}")
        elif self.current_clock_mode == "text":
            # 文字时钟模式
            hour = current_time.hour
            minute = current_time.minute
            second = current_time.second
            period = "上午" if hour < 12 else "下午"
            hour12 = hour % 12 if hour % 12 != 0 else 12
            text_time = f"现在是{period}{hour12}点"
            if minute > 0:
                text_time += f"{minute}分"
            if self.show_seconds and second > 0:
                text_time += f"{second}秒"
            left_lines.append("文字时钟")
            left_lines.append(text_time)
        
        left_lines.append("-" * width_chars)
        
        # 绘制公历和农历日期 - 固件中显示在时钟下方
        gregorian_str = f"公历：{current_time.year}年{current_time.month}月{current_time.day}日"
        lunar_str = f"农历：腊月十六"
        left_lines.append(gregorian_str)
        left_lines.append(lunar_str)
        
        left_lines.append("-" * width_chars)
        
        # 绘制天气信息 - 固件中天气信息位置固定
        left_lines.append("天气信息")
        left_lines.append(f"温度: {self.temperature}°C")
        left_lines.append(f"湿度: {self.humidity}%")
        left_lines.append(f"天气: 晴")
        left_lines.append(f"风力: 3级")
        
        left_lines.append("-" * width_chars)
        
        # 绘制室内环境监测 - 固件中包含更多传感器数据
        left_lines.append("室内环境监测")
        left_lines.append(f"温度: {self.temperature}°C")
        left_lines.append(f"湿度: {self.humidity}%")
        left_lines.append(f"空气质量: {'正常' if self.gas_level < 600 else '警告' if self.gas_level < 800 else '异常'}")
        left_lines.append(f"光照: {'暗' if self.light_level < 200 else '中等' if self.light_level < 500 else '亮'}")
        left_lines.append(f"人体感应: {'有人' if self.motion_detected else '无人'}")
        left_lines.append(f"火焰检测: {'未检测到' if not self.flame_detected else '检测到'}")
        
        left_lines.append("-" * width_chars)
        
        # 绘制电池信息 - 固件中包含电池图标和电量
        battery_status = "红色" if self.battery_level < 20 else "黑色"
        left_lines.append(f"电池: {self.battery_level}% ({battery_status})")
        left_lines.append(f"充电中: {'是' if self.battery_level < 100 else '否'}")
        
        left_lines.append("-" * width_chars)
        
        # 绘制消息通知 - 固件中有新消息时显示
        left_lines.append("消息通知")
        left_lines.append("2条未读消息")
        
        left_lines.append("-" * width_chars)
        
        # 绘制WiFi状态 - 固件中显示在底部
        left_lines.append(f"WiFi: {self.wifi_status}")
        
        # 设备编码 - 固件中显示在底部
        if self.device_code:
            left_lines.append(f"设备码: {self.device_code}")
        
        return left_lines
    
    def start_message_check(self):
        """启动消息检查定时器"""
        if self.message_timer:
            self.root.after_cancel(self.message_timer)
        
        # 每10秒检查一次消息
        self.message_timer = self.root.after(self.message_refresh_interval * 1000, self.check_new_messages)    
    
    def report_status(self):
        """上报设备状态到管理系统"""
        if not self.registered:
            return
        
        try:
            import requests
            import json
            
            # 构建API请求
            url = f"{self.api_config['base_url']}{self.api_config['device_status_endpoint']}"
            headers = {
                "Content-Type": "application/json"
            }
            data = {
                "device_id": self.device_code,
                "connection_status": "online" if self.power_on else "offline",
                "battery_level": self.battery_level,
                "temperature": self.temperature,
                "humidity": self.humidity,
                "light_level": self.light_level,
                "last_active": datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")
            }
            
            response = requests.post(url, headers=headers, json=data, timeout=5)
            
            if response.status_code == 200:
                # 状态上报成功
                pass
        except Exception as e:
            # 状态上报失败，忽略错误
            pass
    
    def check_new_messages(self):
        """检查新消息"""
        if not self.registered:
            # 设备未注册，不检查消息
            self.start_message_check()
            return
        
        # 上报设备状态
        self.report_status()
        
        if not self.power_on:
            # 设备未开机，只上报状态，不检查消息
            self.start_message_check()
            return
        
        try:
            # 模拟从管理系统获取新消息
            import requests
            import json
            
            # 构建API请求
            url = f"{self.api_config['base_url']}{self.api_config['message_list_endpoint']}"
            headers = {
                "Content-Type": "application/json",
                "Device-Code": self.device_code
            }
            
            # 发送请求获取消息列表
            response = requests.get(url, headers=headers, timeout=5)
            
            if response.status_code == 200:
                data = response.json()
                if data.get("success", False):
                    messages = data.get("data", [])
                    new_count = 0
                    for msg in messages:
                        # 检查是否是新消息
                        if not any(m.get("id") == msg.get("id") for m in self.messages):
                            self.messages.append(msg)
                            self.new_messages.append(msg)
                            new_count += 1
                    
                    # 如果有新消息，显示通知并自动切换到消息页面
                    if new_count > 0:
                        self.show_message_notification = True
                        self.status_var.set(f"设备状态: 开机 | 收到 {new_count} 条新消息")
                        # 自动切换到消息页面显示新消息
                        if self.current_right_page != "message":
                            self.current_right_page = "message"
                            self.update_display()
            elif response.status_code == 404:
                # API路径不存在，使用模拟数据生成消息
                import random
                import time
                
                # 随机生成0-2条新消息
                new_count = 0
                for i in range(random.randint(1, 2)):
                    new_msg = {
                        "id": str(time.time() + i),
                        "title": f"测试消息 #{len(self.messages) + i + 1}",
                        "sender": "系统",
                        "content": f"这是一条测试消息，设备编码: {self.device_code}",
                        "created_at": time.strftime("%Y-%m-%d %H:%M:%S"),
                        "priority": random.choice(["普通", "重要", "紧急"])
                    }
                    self.messages.append(new_msg)
                    self.new_messages.append(new_msg)
                    new_count += 1
                
                if new_count > 0:
                    self.show_message_notification = True
                    self.status_var.set(f"设备状态: 开机 | 收到 {new_count} 条新消息 (模拟)")
                    # 自动切换到消息页面显示新消息
                    if self.current_right_page != "message":
                        self.current_right_page = "message"
                        self.update_display()
        except Exception as e:
            # 网络请求错误，使用模拟数据生成消息
            import random
            import time
            
            # 随机生成0-2条新消息
            new_count = 0
            for i in range(random.randint(1, 2)):
                new_msg = {
                    "id": str(time.time() + i),
                    "title": f"测试消息 #{len(self.messages) + i + 1}",
                    "sender": "系统",
                    "content": f"这是一条测试消息，设备编码: {self.device_code}",
                    "created_at": time.strftime("%Y-%m-%d %H:%M:%S"),
                    "priority": random.choice(["普通", "重要", "紧急"])
                }
                self.messages.append(new_msg)
                self.new_messages.append(new_msg)
                new_count += 1
            
            if new_count > 0:
                self.show_message_notification = True
                self.status_var.set(f"设备状态: 开机 | 收到 {new_count} 条新消息 (模拟)")
                # 自动切换到消息页面显示新消息
                if self.current_right_page != "message":
                    self.current_right_page = "message"
                    self.update_display()
        
        # 继续下一次检查
        self.start_message_check()
    
    def bind_device(self, user_id):
        """绑定设备到指定用户"""
        if not self.device_code:
            self.generate_device_code()
        
        # 确保设备已注册
        if not self.registered:
            self.register_device()
        
        try:
            # 模拟API调用 - 绑定设备
            import requests
            import json
            
            url = f"{self.api_config['base_url']}{self.api_config['device_bind_endpoint']}"
            headers = {
                "Content-Type": "application/json"
            }
            data = {
                "device_code": self.device_code,
                "user_id": user_id
            }
            
            response = requests.post(url, headers=headers, json=data, timeout=5)
            
            if response.status_code == 200:
                data = response.json()
                if data.get("success", False):
                    self.bound = True
                    self.bound_user = user_id
                    self.status_var.set(f"设备状态: 开机 | 设备已绑定到用户 {user_id}")
                    return True
            else:
                self.status_var.set(f"设备状态: 开机 | 设备绑定失败，错误码: {response.status_code}")
        except Exception as e:
            # 模拟绑定成功，方便测试
            self.bound = True
            self.bound_user = user_id
            self.status_var.set(f"设备状态: 开机 | 设备已绑定到用户 {user_id} (模拟)")
            return True
        
        return False
    
    def draw_right_panel(self, current_time, width_chars):
        """绘制右侧面板内容，精确模拟真实固件"""
        right_lines = []
        
        if self.current_right_page == "calendar":
            # 日历页面 - 精确模拟真实固件的日历布局
            right_lines.append("月历")
            right_lines.append("日 一 二 三 四 五 六")
            right_lines.append("---------------------")
            
            # 生成日历 - 与真实固件一致
            year = current_time.year
            month = current_time.month
            day = current_time.day
            
            first_day = datetime.date(year, month, 1)
            last_day = datetime.date(year, month + 1, 1) - datetime.timedelta(days=1) if month < 12 else datetime.date(year, 12, 31)
            first_weekday = first_day.weekday()
            
            # 第一行的空格
            calendar_line = "   " * first_weekday
            
            # 日期 - 与真实固件一致的格式
            for d in range(1, last_day.day + 1):
                if d == day:
                    calendar_line += f"[{d:2d}] "
                else:
                    calendar_line += f" {d:2d}  "
                
                if (first_weekday + d) % 7 == 0:
                    right_lines.append(calendar_line)
                    calendar_line = ""
            
            # 添加最后一行
            if calendar_line:
                right_lines.append(calendar_line)
            
            # 绘制节日信息 - 固件中显示在月历下方
            right_lines.append("")
            right_lines.append("今日节日: 无")
            right_lines.append("")
            right_lines.append("宜: 嫁娶、出行")
            right_lines.append("忌: 动土、祈福")
        
        elif self.current_right_page == "stock":
            # 股票页面 - 精确模拟真实固件的股票显示
            right_lines.append("股票行情")
            right_lines.append("-" * width_chars)
            right_lines.append("股票代码: 上证指数")
            right_lines.append("当前价格: 3258.67")
            right_lines.append("涨跌幅: +1.23%")
            right_lines.append("涨跌额: +39.85")
            right_lines.append("-")
            right_lines.append("股票代码: 深证成指")
            right_lines.append("当前价格: 11567.89")
            right_lines.append("涨跌幅: +0.85%")
            right_lines.append("涨跌额: +97.32")
        
        elif self.current_right_page == "message":
            # 消息页面 - 显示从管理系统获取的真实消息
            right_lines.append("消息中心")
            right_lines.append("-" * width_chars)
            
            if not self.messages:
                right_lines.append("暂无消息")
            else:
                # 显示最新的5条消息
                for i, msg in enumerate(self.messages[-5:]):
                    right_lines.append(f"{i+1}. {msg.get('title', '未命名消息')}")
                    right_lines.append(f"   发送者: {msg.get('sender', '未知')}")
                    right_lines.append(f"   时间: {msg.get('created_at', '未知时间')}")
                    right_lines.append(f"   优先级: {msg.get('priority', '普通')}")
                    right_lines.append("")
            
            # 清空新消息列表
            self.new_messages.clear()
            self.show_message_notification = False
        
        elif self.current_right_page == "setting":
            # 设置页面 - 精确模拟真实固件的设置菜单
            right_lines.append("设置菜单")
            right_lines.append("-" * width_chars)
            right_lines.append("1. 亮度设置: 70%")
            right_lines.append("2. 更新间隔: 30秒")
            right_lines.append("3. WiFi设置")
            right_lines.append("4. 设备信息")
            right_lines.append("5. 恢复出厂设置")
            right_lines.append("6. 消息设置")
            right_lines.append("")
            right_lines.append("使用上下键选择")
            right_lines.append("确认键进入，返回键退出")
        
        return right_lines

if __name__ == "__main__":
    root = tk.Tk()
    app = InkClockSimulator(root)
    root.mainloop()
