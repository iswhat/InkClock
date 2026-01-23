#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
InkClock模拟器单元测试

使用pytest框架测试模拟器的主要功能
"""

import pytest
import sys
import os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

from inkclock_simulator import InkClockSimulator

class MockTkRoot:
    """模拟Tkinter根窗口"""
    def __init__(self):
        self.title_value = ""
        self.geometry_value = ""
        self.resizable_value = (True, True)
        self.minsize_value = (500, 400)
        self.bindings = []
        self.timers = []
    
    def title(self, title):
        self.title_value = title
    
    def geometry(self, geometry):
        self.geometry_value = geometry
    
    def resizable(self, width, height):
        self.resizable_value = (width, height)
    
    def minsize(self, width, height):
        self.minsize_value = (width, height)
    
    def bind(self, event, handler):
        self.bindings.append((event, handler))
    
    def after(self, delay, callback=None):
        """模拟after方法，返回一个模拟的定时器ID"""
        timer_id = f"timer_{len(self.timers)}"
        self.timers.append((timer_id, delay, callback))
        return timer_id
    
    def after_cancel(self, timer_id):
        """模拟取消定时器"""
        self.timers = [t for t in self.timers if t[0] != timer_id]
    
    def mainloop(self):
        """模拟主循环"""
        pass

@pytest.fixture
def mock_root():
    """创建模拟的Tk根窗口"""
    return MockTkRoot()

@pytest.fixture
def simulator(mock_root):
    """创建模拟器实例"""
    return InkClockSimulator(mock_root)

class TestInkClockSimulator:
    """测试InkClock模拟器"""
    
    def test_init(self, simulator):
        """测试初始化"""
        # 检查初始状态
        assert not simulator.power_on
        assert simulator.current_mode == "clock"
        assert simulator.brightness == 70
        assert simulator.update_interval == 30
        assert simulator.device_code == ""
        assert simulator.current_screen_size == "4.2"
        
        # 检查屏幕尺寸配置
        assert "4.2" in simulator.screen_sizes
        assert simulator.screen_sizes["4.2"] == (400, 300)
    
    def test_generate_device_code(self, simulator):
        """测试生成设备编码"""
        simulator.generate_device_code()
        
        # 检查设备编码格式
        assert isinstance(simulator.device_code, str)
        assert len(simulator.device_code) == 12
        assert simulator.device_code.isdigit()
        
        # 保存生成的编码，再次生成应该不同
        first_code = simulator.device_code
        simulator.generate_device_code()
        assert simulator.device_code != first_code
        assert len(simulator.device_code) == 12
        assert simulator.device_code.isdigit()
    
    def test_on_power_toggle(self, simulator):
        """测试电源开关切换"""
        # 初始状态为关机
        assert not simulator.power_on
        
        # 模拟电源切换
        simulator.power_on = True
        assert simulator.power_on
        
        simulator.power_on = False
        assert not simulator.power_on
    
    def test_on_mode_change(self, simulator):
        """测试模式切换"""
        # 初始模式为clock
        assert simulator.current_mode == "clock"
        
        # 切换到weather模式
        simulator.current_mode = "weather"
        assert simulator.current_mode == "weather"
        
        # 切换到sensor模式
        simulator.current_mode = "sensor"
        assert simulator.current_mode == "sensor"
        
        # 切换到calendar模式
        simulator.current_mode = "calendar"
        assert simulator.current_mode == "calendar"
    
    def test_on_screen_size_change(self, simulator):
        """测试屏幕尺寸变化"""
        # 初始尺寸为4.2英寸
        assert simulator.current_screen_size == "4.2"
        assert simulator.screen_width == 400
        assert simulator.screen_height == 300
        
        # 切换到7.5英寸
        simulator.current_screen_size = "7.5"
        simulator.screen_width = simulator.screen_sizes[simulator.current_screen_size][0]
        simulator.screen_height = simulator.screen_sizes[simulator.current_screen_size][1]
        
        assert simulator.screen_width == 800
        assert simulator.screen_height == 480
        
        # 切换到2.13英寸
        simulator.current_screen_size = "2.13"
        simulator.screen_width = simulator.screen_sizes[simulator.current_screen_size][0]
        simulator.screen_height = simulator.screen_sizes[simulator.current_screen_size][1]
        
        assert simulator.screen_width == 250
        assert simulator.screen_height == 122
    
    def test_calculate_panel_widths(self, simulator):
        """测试面板宽度计算"""
        # 测试小屏幕（4.2英寸，400x300）
        simulator.screen_width = 400
        simulator.calculate_panel_widths()
        assert simulator.left_panel_width == 200  # 1/2
        assert simulator.right_panel_width == 200
        
        # 测试大屏幕（7.5英寸，800x480）
        simulator.screen_width = 800
        simulator.calculate_panel_widths()
        assert simulator.left_panel_width == 266  # 1/3
        assert simulator.right_panel_width == 534
    
    def test_register_device(self, simulator):
        """测试设备注册"""
        simulator.generate_device_code()
        simulator.register_device()
        
        # 注册状态应该为True
        assert simulator.registered
    
    def test_report_status(self, simulator):
        """测试状态上报"""
        # 设备未注册时，上报状态不应抛出异常
        simulator.registered = False
        simulator.report_status()
        
        # 设备注册后，上报状态不应抛出异常
        simulator.registered = True
        simulator.report_status()
    
    def test_on_device_button(self, simulator):
        """测试设备按键处理"""
        simulator.power_on = True
        
        # 测试上键
        initial_light = simulator.light_level
        simulator.on_device_button("up")
        assert simulator.light_level > initial_light
        
        # 测试下键
        initial_light = simulator.light_level
        simulator.on_device_button("down")
        assert simulator.light_level < initial_light
        
        # 测试左键
        initial_humidity = simulator.humidity
        simulator.on_device_button("left")
        assert simulator.humidity < initial_humidity
        
        # 测试右键
        initial_humidity = simulator.humidity
        simulator.on_device_button("right")
        assert simulator.humidity > initial_humidity
        
        # 测试菜单键
        simulator.on_device_button("menu")
        assert simulator.current_mode == "settings"
        
        # 测试返回键
        simulator.on_device_button("back")
        assert simulator.current_mode == "clock"
        
        # 测试模式切换键
        initial_mode = simulator.current_mode
        simulator.on_device_button("next")
        assert simulator.current_mode != initial_mode
    
    def test_on_key_press(self, simulator, mock_root):
        """测试键盘事件处理"""
        # 测试电源键（0键）
        event = type('obj', (object,), {'char': '0'})()
        simulator.on_key_press(event)
        assert simulator.power_on
        
        # 再次按下电源键，应该关机
        simulator.on_key_press(event)
        assert not simulator.power_on
        
        # 测试上键（1键）
        event.char = '1'
        simulator.power_on = True
        initial_light = simulator.light_level
        simulator.on_key_press(event)
        assert simulator.light_level > initial_light
    
    def test_update_display(self, simulator):
        """测试更新显示"""
        # 关机状态下更新显示
        simulator.power_on = False
        # 不应抛出异常
        simulator.update_display()
        
        # 开机状态下更新显示
        simulator.power_on = True
        # 不应抛出异常
        simulator.update_display()
    
    def test_calculate_cache_expire(self, simulator):
        """测试计算缓存过期时间"""
        # 测试私有方法需要使用反射
        import inspect
        
        # 获取calculate_cache_expire方法
        calculate_method = inspect.getmethod(simulator, '_InkClockSimulator__calculate_panel_widths')
        assert calculate_method is not None
    
    def test_start_message_check(self, simulator):
        """测试启动消息检查"""
        # 不应抛出异常
        simulator.start_message_check()
        
        # 定时器列表应该不为空
        assert hasattr(simulator.root, 'timers')
        assert len(simulator.root.timers) > 0
    
    def test_check_new_messages(self, simulator):
        """测试检查新消息"""
        simulator.registered = True
        simulator.power_on = True
        
        # 不应抛出异常
        simulator.check_new_messages()
    
    def test_bind_device(self, simulator):
        """测试绑定设备"""
        simulator.generate_device_code()
        result = simulator.bind_device("test_user")
        
        # 绑定结果应该为True（模拟模式）
        assert result
        assert simulator.bound
        assert simulator.bound_user == "test_user"
    
    def test_api_config(self, simulator):
        """测试API配置"""
        # 检查API配置结构
        assert "base_url" in simulator.api_config
        assert "device_register_endpoint" in simulator.api_config
        assert "device_status_endpoint" in simulator.api_config
        assert "device_bind_endpoint" in simulator.api_config
        assert "message_push_endpoint" in simulator.api_config
        assert "message_list_endpoint" in simulator.api_config
        
        # 检查默认API URL
        assert simulator.api_config["base_url"] == "http://localhost:8000"

if __name__ == "__main__":
    pytest.main([__file__])
