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
    
    def create_widgets(self):
        """创建界面组件"""
        # 配置主窗口的网格权重
        self.root.columnconfigure(0, weight=1)
        self.root.rowconfigure(0, weight=1)
        
        # 创建主框架
        main_frame = ttk.Frame(self.root, padding="10")
        main_frame.grid(row=0, column=0, sticky=(tk.W, tk.E, tk.N, tk.S))
        
        # 配置主框架的网格权重
        main_frame.columnconfigure(0, weight=1)
        main_frame.columnconfigure(1, weight=1)
        main_frame.rowconfigure(0, weight=2)  # 显示区域权重更大
        main_frame.rowconfigure(1, weight=1)
        main_frame.rowconfigure(2, weight=0)  # 状态信息权重最小
        
        # 创建显示区域
        self.display_frame = ttk.LabelFrame(main_frame, text="墨水屏显示", padding="10")
        self.display_frame.grid(row=0, column=0, columnspan=2, sticky=(tk.W, tk.E, tk.N, tk.S), pady=(0, 10))
        
        # 配置显示框架的网格权重
        self.display_frame.columnconfigure(0, weight=1)
        self.display_frame.rowconfigure(0, weight=1)
        
        # 显示内容标签
        self.display_label = tk.Label(self.display_frame, text="", font=('Arial', 14), 
                                     bg="#f0f0f0", relief="solid", borderwidth=2, justify=tk.LEFT)
        self.display_label.pack(fill=tk.BOTH, expand=True, padx=5, pady=5)
        
        # 控制面板
        control_frame = ttk.LabelFrame(main_frame, text="控制面板", padding="10")
        control_frame.grid(row=1, column=0, sticky=(tk.W, tk.E, tk.N, tk.S), pady=(0, 10))
        
        # 配置控制框架的网格权重
        control_frame.columnconfigure(0, weight=0)
        control_frame.columnconfigure(1, weight=1)
        control_frame.rowconfigure(0, weight=0)
        control_frame.rowconfigure(1, weight=0)
        control_frame.rowconfigure(2, weight=0)
        control_frame.rowconfigure(3, weight=1)  # 设备按键区域权重较大
        control_frame.rowconfigure(4, weight=1)  # 键盘快捷键区域权重较大
        
        # 电源开关
        self.power_var = tk.BooleanVar()
        power_button = ttk.Checkbutton(control_frame, text="电源开关", variable=self.power_var, command=self.on_power_toggle)
        power_button.grid(row=0, column=0, sticky=tk.W, pady=(0, 5))
        
        # 墨水屏尺寸选择
        screen_label = ttk.Label(control_frame, text="墨水屏尺寸:")
        screen_label.grid(row=1, column=0, sticky=tk.W, pady=(0, 5))
        
        self.screen_size_var = tk.StringVar(value="4.2")
        screen_combo = ttk.Combobox(control_frame, textvariable=self.screen_size_var, state="readonly")
        screen_combo['values'] = tuple(self.screen_sizes.keys())
        screen_combo.grid(row=1, column=1, sticky=tk.W, pady=(0, 5))
        screen_combo.bind("<<ComboboxSelected>>", self.on_screen_size_change)
        
        # 设备编码显示
        code_label = ttk.Label(control_frame, text="设备编码:")
        code_label.grid(row=2, column=0, sticky=tk.W, pady=(0, 5))
        
        self.device_code_var = tk.StringVar()
        code_entry = ttk.Entry(control_frame, textvariable=self.device_code_var, state="readonly", width=15)
        code_entry.grid(row=2, column=1, sticky=tk.W, pady=(0, 5))
        
        # 生成设备编码按钮
        generate_code_button = ttk.Button(control_frame, text="生成设备编码", command=self.generate_device_code)
        generate_code_button.grid(row=5, column=0, columnspan=2, sticky=tk.W, pady=(0, 5))
        
        # 设备绑定功能
        bind_frame = ttk.LabelFrame(control_frame, text="设备绑定", padding="5")
        bind_frame.grid(row=6, column=0, columnspan=2, sticky=(tk.W, tk.E, tk.N, tk.S), pady=(0, 5))
        
        bind_label = ttk.Label(bind_frame, text="用户ID:")
        bind_label.grid(row=0, column=0, sticky=tk.W, pady=(0, 5))
        
        self.bind_user_var = tk.StringVar(value="user123")
        bind_entry = ttk.Entry(bind_frame, textvariable=self.bind_user_var, width=15)
        bind_entry.grid(row=0, column=1, sticky=tk.W, pady=(0, 5))
        
        bind_button = ttk.Button(bind_frame, text="绑定设备", command=self.on_bind_device)
        bind_button.grid(row=0, column=2, sticky=tk.W, pady=(0, 5), padx=(5, 0))
        
        # 消息控制
        message_frame = ttk.LabelFrame(control_frame, text="消息控制", padding="5")
        message_frame.grid(row=7, column=0, columnspan=2, sticky=(tk.W, tk.E, tk.N, tk.S), pady=(0, 5))
        
        refresh_msg_button = ttk.Button(message_frame, text="刷新消息", command=self.on_refresh_messages)
        refresh_msg_button.grid(row=0, column=0, sticky=tk.W, pady=(0, 5), padx=(0, 5))
        
        clear_msg_button = ttk.Button(message_frame, text="清空消息", command=self.on_clear_messages)
        clear_msg_button.grid(row=0, column=1, sticky=tk.W, pady=(0, 5))
        
        # 模式切换
        mode_label = ttk.Label(control_frame, text="显示模式:")
        mode_label.grid(row=8, column=0, sticky=tk.W, pady=(0, 5))
        
        self.mode_var = tk.StringVar(value="clock")
        mode_combo = ttk.Combobox(control_frame, textvariable=self.mode_var, state="readonly")
        mode_combo['values'] = ("clock", "weather", "sensor", "calendar", "settings", "message")
        mode_combo.grid(row=8, column=1, sticky=tk.W, pady=(0, 5))
        mode_combo.bind("<<ComboboxSelected>>", self.on_mode_change)
        
        # 重置按钮
        reset_button = ttk.Button(control_frame, text="重置设备", command=self.on_reset)
        reset_button.grid(row=9, column=0, columnspan=2, sticky=tk.W, pady=(0, 10))
        
        # 设备按键模拟
        button_frame = ttk.LabelFrame(control_frame, text="设备按键模拟", padding="5")
        button_frame.grid(row=3, column=0, columnspan=2, sticky=(tk.W, tk.E, tk.N, tk.S), pady=(0, 5))
        
        # 上/下/左/右按键
        up_button = ttk.Button(button_frame, text="上键", width=6, command=lambda: self.on_device_button("up"))
        up_button.grid(row=0, column=1, pady=2)
        
        left_button = ttk.Button(button_frame, text="左键", width=6, command=lambda: self.on_device_button("left"))
        left_button.grid(row=1, column=0, padx=2, pady=2)
        
        ok_button = ttk.Button(button_frame, text="确认", width=6, command=lambda: self.on_device_button("ok"))
        ok_button.grid(row=1, column=1, padx=2, pady=2)
        
        right_button = ttk.Button(button_frame, text="右键", width=6, command=lambda: self.on_device_button("right"))
        right_button.grid(row=1, column=2, padx=2, pady=2)
        
        down_button = ttk.Button(button_frame, text="下键", width=6, command=lambda: self.on_device_button("down"))
        down_button.grid(row=2, column=1, pady=2)
        
        # 菜单和返回按键
        menu_button = ttk.Button(button_frame, text="菜单", width=8, command=lambda: self.on_device_button("menu"))
        menu_button.grid(row=3, column=0, padx=2, pady=2)
        
        back_button = ttk.Button(button_frame, text="返回", width=8, command=lambda: self.on_device_button("back"))
        back_button.grid(row=3, column=1, padx=2, pady=2)
        
        next_button = ttk.Button(button_frame, text="模式切换", width=8, command=lambda: self.on_device_button("next"))
        next_button.grid(row=3, column=2, padx=2, pady=2)
        
        # 键盘快捷键说明
        keyboard_frame = ttk.LabelFrame(control_frame, text="键盘快捷键", padding="5")
        keyboard_frame.grid(row=4, column=0, columnspan=2, sticky=(tk.W, tk.E, tk.N, tk.S), pady=(0, 5))
        
        keyboard_help = ttk.Label(keyboard_frame, 
                                 text="键盘数字键对应设备按钮：\n" +
                                 "1: 上键   2: 下键   3: 左键   4: 右键\n" +
                                 "5: 确认   6: 菜单   7: 返回   8: 模式切换\n" +
                                 "0: 电源开关",
                                 font=('Arial', 9), justify=tk.LEFT)
        keyboard_help.pack(anchor=tk.W, fill=tk.BOTH, expand=True, padx=5, pady=5)
        
        # 传感器控制
        sensor_frame = ttk.LabelFrame(main_frame, text="传感器数据", padding="10")
        sensor_frame.grid(row=1, column=1, sticky=(tk.W, tk.E, tk.N, tk.S), pady=(0, 10))
        
        # 配置传感器框架的网格权重
        sensor_frame.columnconfigure(0, weight=0)
        sensor_frame.columnconfigure(1, weight=1)
        sensor_frame.columnconfigure(2, weight=0)
        
        # 温度
        temp_label = ttk.Label(sensor_frame, text="温度:")
        temp_label.grid(row=0, column=0, sticky=tk.W, pady=(0, 5))
        self.temp_entry = ttk.Entry(sensor_frame, width=10)
        self.temp_entry.insert(0, "22")
        self.temp_entry.grid(row=0, column=1, sticky=tk.W, pady=(0, 5))
        temp_unit = ttk.Label(sensor_frame, text="°C")
        temp_unit.grid(row=0, column=2, sticky=tk.W, pady=(0, 5))
        
        # 湿度
        humi_label = ttk.Label(sensor_frame, text="湿度:")
        humi_label.grid(row=1, column=0, sticky=tk.W, pady=(0, 5))
        self.humi_entry = ttk.Entry(sensor_frame, width=10)
        self.humi_entry.insert(0, "50")
        self.humi_entry.grid(row=1, column=1, sticky=tk.W, pady=(0, 5))
        humi_unit = ttk.Label(sensor_frame, text="%")
        humi_unit.grid(row=1, column=2, sticky=tk.W, pady=(0, 5))
        
        # 气压
        pres_label = ttk.Label(sensor_frame, text="气压:")
        pres_label.grid(row=2, column=0, sticky=tk.W, pady=(0, 5))
        self.pres_entry = ttk.Entry(sensor_frame, width=10)
        self.pres_entry.insert(0, "1013")
        self.pres_entry.grid(row=2, column=1, sticky=tk.W, pady=(0, 5))
        pres_unit = ttk.Label(sensor_frame, text="hPa")
        pres_unit.grid(row=2, column=2, sticky=tk.W, pady=(0, 5))
        
        # 电池电量
        batt_label = ttk.Label(sensor_frame, text="电池:")
        batt_label.grid(row=3, column=0, sticky=tk.W, pady=(0, 5))
        self.batt_entry = ttk.Entry(sensor_frame, width=10)
        self.batt_entry.insert(0, "80")
        self.batt_entry.grid(row=3, column=1, sticky=tk.W, pady=(0, 5))
        batt_unit = ttk.Label(sensor_frame, text="%")
        batt_unit.grid(row=3, column=2, sticky=tk.W, pady=(0, 5))
        
        # 更新按钮
        update_button = ttk.Button(sensor_frame, text="更新数据", command=self.update_sensor_data)
        update_button.grid(row=4, column=0, columnspan=3, sticky=tk.W, pady=(0, 5))
        
        # 状态信息
        status_frame = ttk.LabelFrame(main_frame, text="状态信息", padding="10")
        status_frame.grid(row=2, column=0, columnspan=2, sticky=(tk.W, tk.E, tk.N, tk.S))
        
        # 配置状态框架的网格权重
        status_frame.columnconfigure(0, weight=1)
        status_frame.rowconfigure(0, weight=1)
        
        self.status_var = tk.StringVar()
        status_label = ttk.Label(status_frame, textvariable=self.status_var, font=("Arial", 10), 
                               justify=tk.LEFT, wraplength=0)
        status_label.pack(fill=tk.X, expand=True, padx=5, pady=5)
    
    def on_power_toggle(self):
        """电源开关切换"""
        self.power_on = self.power_var.get()
        self.update_display()
    
    def on_mode_change(self, event):
        """显示模式切换"""
        self.current_mode = self.mode_var.get()
        self.update_display()
    
    def on_bind_device(self):
        """绑定设备到指定用户"""
        user_id = self.bind_user_var.get()
        if not user_id:
            self.status_var.set("设备状态: 开机 | 请输入用户ID")
            return
        
        if self.bind_device(user_id):
            self.status_var.set(f"设备状态: 开机 | 设备已成功绑定到用户 {user_id}")
        else:
            self.status_var.set("设备状态: 开机 | 设备绑定失败")
    
    def on_refresh_messages(self):
        """手动刷新消息"""
        self.status_var.set("设备状态: 开机 | 正在刷新消息...")
        self.check_new_messages()
        self.update_display()
    
    def on_clear_messages(self):
        """清空所有消息"""
        self.messages.clear()
        self.new_messages.clear()
        self.show_message_notification = False
        self.status_var.set("设备状态: 开机 | 所有消息已清空")
        self.update_display()
    
    def on_reset(self):
        """重置设备"""
        self.current_mode = "clock"
        self.mode_var.set("clock")
        self.brightness = 70
        self.update_interval = 30
        self.temperature = 22
        self.humidity = 50
        self.pressure = 1013
        self.light_level = 200
        self.battery_level = 80
        self.motion_detected = False
        self.wifi_status = "Connected"
        
        # 清空消息
        self.messages.clear()
        self.new_messages.clear()
        self.show_message_notification = False
        
        # 清除绑定状态
        self.bound = False
        self.bound_user = ""
        
        # 更新输入框
        self.temp_entry.delete(0, tk.END)
        self.temp_entry.insert(0, "22")
        self.humi_entry.delete(0, tk.END)
        self.humi_entry.insert(0, "50")
        self.pres_entry.delete(0, tk.END)
        self.pres_entry.insert(0, "1013")
        self.batt_entry.delete(0, tk.END)
        self.batt_entry.insert(0, "80")
        
        self.update_display()
    
    def update_sensor_data(self):
        """更新传感器数据"""
        try:
            self.temperature = int(self.temp_entry.get())
            self.humidity = int(self.humi_entry.get())
            self.pressure = int(self.pres_entry.get())
            self.battery_level = int(self.batt_entry.get())
            self.update_display()
        except ValueError:
            self.status_var.set("错误：请输入有效的数字")
    
    def register_device(self):
        """注册设备到管理系统"""
        if not self.device_code:
            self.generate_device_code()
        
        # 模拟API调用 - 检查设备编码是否冲突
        conflict = random.choice([True, False, False])  # 30%概率冲突
        
        if conflict:
            # 冲突，重新生成设备编码
            self.status_var.set(f"设备状态: 开机 | 设备编码冲突，正在重新生成...")
            self.generate_device_code()
            # 再次检查新编码是否冲突
            conflict = random.choice([True, False, False])
            if not conflict:
                # 新编码无冲突，注册成功
                self.registered = True
                self.status_var.set(f"设备状态: 开机 | 设备已成功注册到管理系统，编码: {self.device_code}")
            else:
                # 仍有冲突，只更新状态
                self.status_var.set(f"设备状态: 开机 | 设备编码仍有冲突，建议检查网络后重试")
        else:
            # 注册成功
            self.registered = True
            self.status_var.set(f"设备状态: 开机 | 设备已成功注册到管理系统，编码: {self.device_code}")
    
    def report_status(self):
        """上报设备状态到管理系统"""
        if not self.registered:
            self.status_var.set(f"设备状态: 开机 | 设备未注册，无法上报状态")
            return
        
        # 模拟API调用 - 上报状态
        self.status_var.set(f"设备状态: 开机 | 设备状态已上报到管理系统")
    
    def check_device_code(self):
        """检查设备编码是否冲突"""
        if not self.device_code:
            self.status_var.set(f"设备状态: 开机 | 请先生成设备编码")
            return
        
        # 模拟API调用 - 检查设备编码
        conflict = random.choice([True, False, False])  # 30%概率冲突
        
        if conflict:
            self.status_var.set(f"设备状态: 开机 | 设备编码冲突: {self.device_code}")
        else:
            self.status_var.set(f"设备状态: 开机 | 设备编码有效: {self.device_code}")
    
    def on_device_button(self, button):
        """处理设备按键事件"""
        if not self.power_on:
            return
        
        action = ""
        
        if button == "up":
            action = "上键按下"
            # 上键：增加数值或向上导航
            if self.current_mode == "clock":
                self.light_level = min(1000, self.light_level + 10)
            elif self.current_mode == "weather":
                self.temperature += 1
            elif self.current_mode == "sensor":
                self.temperature += 1
            elif self.current_mode == "calendar":
                # 日历模式：向上切换月份
                pass
            elif self.current_mode == "settings":
                # 设置模式：向上选择
                pass
        
        elif button == "down":
            action = "下键按下"
            # 下键：减少数值或向下导航
            if self.current_mode == "clock":
                self.light_level = max(0, self.light_level - 10)
            elif self.current_mode == "weather":
                self.temperature -= 1
            elif self.current_mode == "sensor":
                self.temperature -= 1
            elif self.current_mode == "calendar":
                # 日历模式：向下切换月份
                pass
            elif self.current_mode == "settings":
                # 设置模式：向下选择
                pass
        
        elif button == "left":
            action = "左键按下"
            # 左键：向左导航或切换选项
            self.humidity = max(0, self.humidity - 1)
        
        elif button == "right":
            action = "右键按下"
            # 右键：向右导航或切换选项
            self.humidity = min(100, self.humidity + 1)
        
        elif button == "ok":
            action = "确认键按下"
            # 确认键：进入/退出设置或确认选择
            if self.current_mode == "settings":
                # 设置模式：确认选择
                self.status_var.set(f"设备状态: 开机 | 已确认设置选项")
        
        elif button == "menu":
            action = "菜单键按下"
            # 菜单键：打开设置菜单
            self.current_mode = "settings"
            self.mode_var.set("settings")
            self.status_var.set(f"设备状态: 开机 | 已进入设置菜单")
            self.update_display()
            return
        
        elif button == "back":
            action = "返回键按下"
            # 返回键：返回上一级或退出设置
            if self.current_mode == "settings":
                # 退出设置，返回时钟模式
                self.current_mode = "clock"
                self.mode_var.set("clock")
                self.status_var.set(f"设备状态: 开机 | 已退出设置菜单")
                self.update_display()
                return
            else:
                self.status_var.set(f"设备状态: 开机 | 已返回")
                return
        
        elif button == "next":
            action = "模式切换键按下"
            # 模式切换键：切换显示模式
            modes = ["clock", "weather", "sensor", "calendar", "message"]
            current_index = modes.index(self.current_mode) if self.current_mode in modes else 0
            next_index = (current_index + 1) % len(modes)
            self.current_mode = modes[next_index]
            self.mode_var.set(self.current_mode)
        
        # 更新状态信息
        self.status_var.set(f"设备状态: 开机 | 亮度: {self.brightness}% | 更新间隔: {self.update_interval}秒 | 电池: {self.battery_level}% | {action}")
        
        # 更新显示
        self.update_display()
        
        # 更新输入框
        self.temp_entry.delete(0, tk.END)
        self.temp_entry.insert(0, str(self.temperature))
        self.humi_entry.delete(0, tk.END)
        self.humi_entry.insert(0, str(self.humidity))
    
    def on_key_press(self, event):
        """处理键盘事件"""
        # 键盘数字键映射到设备按钮
        key_mapping = {
            "1": "up",        # 1键：上键
            "2": "down",      # 2键：下键
            "3": "left",      # 3键：左键
            "4": "right",     # 4键：右键
            "5": "ok",        # 5键：确认键
            "6": "menu",      # 6键：菜单键
            "7": "back",      # 7键：返回键
            "8": "next",      # 8键：模式切换键
            "0": "power"      # 0键：电源开关
        }
        
        key = event.char
        if key in key_mapping:
            mapped_button = key_mapping[key]
            
            if mapped_button == "power":
                # 电源开关特殊处理
                self.power_on = not self.power_on
                self.power_var.set(self.power_on)
                self.update_display()
            else:
                # 其他按键处理
                self.on_device_button(mapped_button)
                
            # 更新状态显示
            self.status_var.set(f"设备状态: {'开机' if self.power_on else '关机'} | 亮度: {self.brightness}% | 更新间隔: {self.update_interval}秒 | 电池: {self.battery_level}% | 按键: {key} = {mapped_button}")
    
    def on_screen_size_change(self, event):
        """墨水屏尺寸变化处理"""
        self.current_screen_size = self.screen_size_var.get()
        self.screen_width = self.screen_sizes[self.current_screen_size][0]
        self.screen_height = self.screen_sizes[self.current_screen_size][1]
        
        # 根据屏幕尺寸调整显示标签的字体大小
        font_size = max(8, min(16, int(self.screen_width / 30)))
        self.display_label.config(font=('Arial', font_size))
        
        self.update_display()
        self.status_var.set(f"设备状态: {'开机' if self.power_on else '关机'} | 墨水屏尺寸: {self.current_screen_size}英寸 ({self.screen_width}x{self.screen_height})")
    
    def generate_device_code(self):
        """生成12位纯数字设备编码"""
        # 模拟设备硬件信息 - 使用随机数模拟不同设备的硬件差异
        import random
        hardware_info = [
            random.randint(0x000000, 0xFFFFFF),  # 模拟MAC地址部分
            random.randint(0x000000, 0xFFFFFF),  # 模拟CPU ID部分
            random.randint(0x000000, 0xFFFFFF)   # 模拟其他硬件标识
        ]
        
        # 根据硬件信息生成基础编码
        base_code = 0
        for info in hardware_info:
            base_code = (base_code * 1000000) + (info & 0xFFFFFF)
        
        # 添加时间戳确保唯一性
        now = datetime.datetime.now()
        timestamp = int(now.timestamp()) % 1000000  # 6位时间戳
        
        # 生成12位设备编码：硬件基础编码(6位) + 时间戳(6位)
        device_code = f"{base_code % 1000000:06d}{timestamp:06d}"
        
        # 确保设备编码为12位纯数字
        self.device_code = device_code[:12]
        
        self.device_code_var.set(self.device_code)
        self.status_var.set(f"设备状态: {'开机' if self.power_on else '关机'} | 设备编码已生成: {self.device_code}")    
    
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
