<?php
/**
 * 在线插件管理界面
 * 用于管理在线插件列表
 */

// 设置响应头
header('Content-Type: text/html; charset=utf-8');

// 插件JSON文件路径
$pluginJsonPath = dirname(__FILE__) . '/plugin.json';

// 处理表单提交
if ($_SERVER['REQUEST_METHOD'] === 'POST') {
    $action = $_POST['action'] ?? '';
    $plugins = [];
    
    // 读取现有插件数据
    if (file_exists($pluginJsonPath)) {
        $plugins = json_decode(file_get_contents($pluginJsonPath), true) ?? [];
    }
    
    switch ($action) {
        case 'add':
            // 添加新插件
            $newPlugin = [
                'name' => $_POST['name'] ?? '',
                'url' => $_POST['url'] ?? '',
                'description' => $_POST['description'] ?? '',
                'refresh_interval' => $_POST['refresh_interval'] ?? '',
                'settings_url' => $_POST['settings_url'] ?? ''
            ];
            
            // 仅添加必填字段非空的插件
            if (!empty($newPlugin['name']) && !empty($newPlugin['url'])) {
                $plugins[] = $newPlugin;
                file_put_contents($pluginJsonPath, json_encode($plugins, JSON_PRETTY_PRINT | JSON_UNESCAPED_UNICODE));
                header('Location: manage.php?success=add');
                exit;
            }
            break;
            
        case 'delete':
            // 删除插件
            $index = $_POST['index'] ?? -1;
            if (isset($plugins[$index])) {
                array_splice($plugins, $index, 1);
                file_put_contents($pluginJsonPath, json_encode($plugins, JSON_PRETTY_PRINT | JSON_UNESCAPED_UNICODE));
                header('Location: manage.php?success=delete');
                exit;
            }
            break;
    }
}

// 读取插件数据
$plugins = [];
if (file_exists($pluginJsonPath)) {
    $plugins = json_decode(file_get_contents($pluginJsonPath), true) ?? [];
}

// 获取成功消息
$success = $_GET['success'] ?? '';
$successMessage = '';
if ($success === 'add') {
    $successMessage = '插件添加成功！';
} elseif ($success === 'delete') {
    $successMessage = '插件删除成功！';
}
?>
<!DOCTYPE html>
<html lang="zh-CN">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>InkClock - 在线插件管理</title>
    <style>
        /* 基础样式 */
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }
        
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: #333;
            line-height: 1.6;
            min-height: 100vh;
            padding: 20px;
        }
        
        .container {
            max-width: 1200px;
            margin: 0 auto;
        }
        
        /* 卡片样式 */
        .card {
            background-color: white;
            border-radius: 12px;
            box-shadow: 0 4px 6px rgba(0, 0, 0, 0.1);
            padding: 24px;
            margin-bottom: 24px;
            transition: all 0.3s ease;
        }
        
        .card:hover {
            box-shadow: 0 6px 12px rgba(0, 0, 0, 0.15);
            transform: translateY(-2px);
        }
        
        /* 头部样式 */
        header.card {
            background: linear-gradient(135deg, #4a6fa5 0%, #3a5d8a 100%);
            color: white;
            text-align: center;
            padding: 32px 24px;
        }
        
        header h1 {
            font-size: 2.5rem;
            font-weight: 700;
            margin-bottom: 8px;
            letter-spacing: -0.5px;
        }
        
        header p {
            font-size: 1.1rem;
            opacity: 0.9;
            margin: 0;
        }
        
        /* 导航样式 */
        nav.card {
            padding: 0;
            background: white;
        }
        
        nav ul {
            list-style: none;
            display: flex;
            justify-content: center;
            gap: 8px;
            flex-wrap: wrap;
            padding: 8px;
            margin: 0;
        }
        
        nav ul li a {
            text-decoration: none;
            color: #6c757d;
            padding: 12px 20px;
            border-radius: 50px;
            font-weight: 500;
            transition: all 0.3s ease;
            font-size: 0.95rem;
            display: block;
        }
        
        nav ul li a:hover, nav ul li a.active {
            background-color: #4a6fa5;
            color: white;
            box-shadow: 0 4px 6px rgba(0, 0, 0, 0.1);
            transform: translateY(-1px);
        }
        
        /* 表单样式 */
        .form-group {
            margin-bottom: 16px;
        }
        
        .form-group label {
            display: block;
            margin-bottom: 8px;
            font-weight: 600;
            color: #4a6fa5;
        }
        
        .form-group input, .form-group textarea {
            width: 100%;
            padding: 12px;
            border: 1px solid #e9ecef;
            border-radius: 8px;
            font-size: 1rem;
            transition: all 0.3s ease;
        }
        
        .form-group input:focus, .form-group textarea:focus {
            outline: none;
            border-color: #4a6fa5;
            box-shadow: 0 0 0 3px rgba(74, 111, 165, 0.1);
        }
        
        /* 按钮样式 */
        .btn {
            display: inline-flex;
            align-items: center;
            justify-content: center;
            gap: 8px;
            padding: 12px 24px;
            border: none;
            border-radius: 8px;
            font-size: 1rem;
            font-weight: 600;
            text-decoration: none;
            transition: all 0.3s ease;
            cursor: pointer;
            font-family: inherit;
        }
        
        .btn-primary {
            background-color: #4a6fa5;
            color: white;
        }
        
        .btn-primary:hover {
            background-color: #3a5d8a;
            transform: translateY(-1px);
            box-shadow: 0 4px 6px rgba(0, 0, 0, 0.1);
        }
        
        .btn-danger {
            background-color: #dc3545;
            color: white;
        }
        
        .btn-danger:hover {
            background-color: #c82333;
        }
        
        .btn-secondary {
            background-color: #6c757d;
            color: white;
        }
        
        .btn-secondary:hover {
            background-color: #5a6268;
        }
        
        /* 插件列表样式 */
        .plugins-table {
            width: 100%;
            border-collapse: collapse;
            margin-top: 24px;
        }
        
        .plugins-table th,
        .plugins-table td {
            padding: 12px;
            text-align: left;
            border-bottom: 1px solid #e9ecef;
        }
        
        .plugins-table th {
            background-color: #4a6fa5;
            color: white;
            font-weight: 600;
        }
        
        .plugins-table tr:hover {
            background-color: #f8f9fa;
        }
        
        .plugins-table tr:last-child td {
            border-bottom: none;
        }
        
        /* 成功消息样式 */
        .success-message {
            background-color: #d4edda;
            color: #155724;
            padding: 12px;
            border-radius: 8px;
            margin-bottom: 24px;
            border: 1px solid #c3e6cb;
        }
        
        /* 响应式设计 */
        @media (max-width: 768px) {
            body {
                padding: 12px;
            }
            
            header h1 {
                font-size: 2rem;
            }
            
            nav ul {
                flex-direction: column;
                align-items: stretch;
            }
            
            .plugins-table {
                font-size: 14px;
            }
            
            .plugins-table th,
            .plugins-table td {
                padding: 8px;
            }
        }
        
        /* 页脚样式 */
        footer {
            text-align: center;
            margin-top: 32px;
            color: white;
            font-size: 0.9rem;
            opacity: 0.9;
            padding: 16px;
            background: rgba(0, 0, 0, 0.1);
            border-radius: 12px;
        }
    </style>
</head>
<body>
    <div class="container">
        <header class="card">
            <h1>InkClock 在线插件管理</h1>
            <p>管理在线插件列表，方便设备获取和添加插件</p>
        </header>
        
        <nav class="card">
            <ul>
                <li><a href="https://github.com/iswhat/InkClock">项目主页</a></li>
                <li><a href="index.php">推荐插件</a></li>
                <li><a href="manage.php" class="active">在线插件管理</a></li>
                <li><a href="/">消息中转</a></li>
            </ul>
        </nav>
        
        <main class="card">
            <?php if ($successMessage): ?>
                <div class="success-message">
                    <?php echo $successMessage; ?>
                </div>
            <?php endif; ?>
            
            <h2>添加新插件</h2>
            <form method="POST" style="margin-bottom: 32px;">
                <input type="hidden" name="action" value="add">
                
                <div class="form-group">
                    <label for="name">插件名称 *</label>
                    <input type="text" id="name" name="name" required placeholder="请输入插件名称">
                </div>
                
                <div class="form-group">
                    <label for="url">插件URL *</label>
                    <input type="text" id="url" name="url" required placeholder="请输入插件的完整URL">
                </div>
                
                <div class="form-group">
                    <label for="description">插件描述</label>
                    <textarea id="description" name="description" rows="3" placeholder="请输入插件的简短描述"></textarea>
                </div>
                
                <div class="form-group">
                    <label for="refresh_interval">刷新频率</label>
                    <input type="text" id="refresh_interval" name="refresh_interval" placeholder="例如：60分钟、24小时">
                </div>
                
                <div class="form-group">
                    <label for="settings_url">设置接口URL</label>
                    <input type="text" id="settings_url" name="settings_url" placeholder="插件的设置接口URL（可选）">
                </div>
                
                <button type="submit" class="btn btn-primary">添加插件</button>
            </form>
            
            <h2>在线插件列表</h2>
            <?php if (empty($plugins)): ?>
                <p style="text-align: center; color: #6c757d; padding: 24px;">暂无在线插件</p>
            <?php else: ?>
                <table class="plugins-table">
                    <thead>
                        <tr>
                            <th>名称</th>
                            <th>URL</th>
                            <th>刷新频率</th>
                            <th>设置接口</th>
                            <th>操作</th>
                        </tr>
                    </thead>
                    <tbody>
                        <?php foreach ($plugins as $index => $plugin): ?>
                            <tr>
                                <td>
                                    <strong><?php echo htmlspecialchars($plugin['name']); ?></strong><br>
                                    <small style="color: #6c757d;"><?php echo htmlspecialchars($plugin['description'] ?? '无描述'); ?></small>
                                </td>
                                <td><?php echo htmlspecialchars($plugin['url']); ?></td>
                                <td><?php echo htmlspecialchars($plugin['refresh_interval'] ?? '默认'); ?></td>
                                <td>
                                    <?php if (!empty($plugin['settings_url'])): ?>
                                        <a href="<?php echo htmlspecialchars($plugin['settings_url']); ?>" target="_blank" style="color: #4a6fa5;">查看</a>
                                    <?php else: ?>
                                        -</p>
                                    <?php endif; ?>
                                </td>
                                <td>
                                    <form method="POST" style="display: inline;">
                                        <input type="hidden" name="action" value="delete">
                                        <input type="hidden" name="index" value="<?php echo $index; ?>">
                                        <button type="submit" class="btn btn-danger" onclick="return confirm('确定要删除这个插件吗？');" style="padding: 6px 12px; font-size: 0.85rem;">删除</button>
                                    </form>
                                </td>
                            </tr>
                        <?php endforeach; ?>
                    </tbody>
                </table>
            <?php endif; ?>
        </main>
        
        <footer>
            <p>&copy; 2025 InkClock. All rights reserved. | 开发者: <a href="https://github.com/iswhat" style="color: white; text-decoration: underline;">iswhat</a></p>
        </footer>
    </div>
</body>
</html>