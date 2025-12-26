<?php
/**
 * 插件列表页面
 * 显示推荐的插件，方便添加到设备的设置中
 */

// 设置响应头
header('Content-Type: text/html; charset=utf-8');
?>
<!DOCTYPE html>
<html lang="zh-CN">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>InkClock - 推荐插件列表</title>
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
        
        /* 插件列表样式 */
        .plugins-container {
            display: grid;
            grid-template-columns: repeat(auto-fill, minmax(350px, 1fr));
            gap: 24px;
            margin-top: 24px;
        }
        
        .plugin-card {
            background: white;
            border-radius: 12px;
            box-shadow: 0 4px 6px rgba(0, 0, 0, 0.1);
            padding: 24px;
            transition: all 0.3s ease;
            border: 1px solid #e9ecef;
        }
        
        .plugin-card:hover {
            box-shadow: 0 6px 12px rgba(0, 0, 0, 0.15);
            transform: translateY(-2px);
            border-color: #4a6fa5;
        }
        
        .plugin-card h3 {
            color: #4a6fa5;
            margin-bottom: 16px;
            font-size: 1.3rem;
            font-weight: 600;
            display: flex;
            align-items: center;
            gap: 8px;
        }
        
        .plugin-card h3::before {
            content: '';
            width: 4px;
            height: 20px;
            background-color: #4a6fa5;
            border-radius: 2px;
        }
        
        .plugin-info {
            margin-bottom: 20px;
        }
        
        .plugin-info p {
            margin-bottom: 12px;
            font-size: 0.95rem;
            color: #6c757d;
        }
        
        .plugin-info strong {
            color: #4a6fa5;
            font-weight: 600;
        }
        
        .plugin-url {
            background: #f8f9fa;
            padding: 12px;
            border-radius: 8px;
            margin-bottom: 20px;
            font-family: 'Courier New', Courier, monospace;
            font-size: 0.85rem;
            word-break: break-all;
            border: 1px solid #e9ecef;
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
        
        .btn-secondary {
            background-color: #6c757d;
            color: white;
        }
        
        .btn-secondary:hover {
            background-color: #5a6268;
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
            
            .plugins-container {
                grid-template-columns: 1fr;
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
            <h1>InkClock 推荐插件列表</h1>
            <p>智能墨水屏万年历的推荐插件，方便您添加到设备的设置中</p>
        </header>
        
        <nav class="card">
            <ul>
                <li><a href="https://github.com/iswhat/InkClock">项目主页</a></li>
                <li><a href="#" class="active">推荐插件</a></li>
                <li><a href="/">消息中转</a></li>
            </ul>
        </nav>
        
        <main class="card">
            <h2>推荐插件</h2>
            <div class="plugins-container">
                <!-- 每日古诗插件 -->
                <div class="plugin-card">
                    <h3>每日古诗</h3>
                    <div class="plugin-info">
                        <p><strong>类型:</strong> URL JSON插件</p>
                        <p><strong>描述:</strong> 每天获取一首经典古诗，展示在您的万年历上</p>
                        <p><strong>更新频率:</strong> 每天</p>
                        <p><strong>作者:</strong> iswhat</p>
                    </div>
                    <div class="plugin-url">
                        <strong>插件URL:</strong><br>
                        <?php echo (isset($_SERVER['HTTPS']) && $_SERVER['HTTPS'] === 'on' ? "https" : "http") . "://" . $_SERVER['HTTP_HOST'] . "/plugin/daily_poem/index.php"; ?>
                    </div>
                    <div style="display: flex; gap: 12px;">
                        <button class="btn btn-primary" onclick="copyUrl('<?php echo (isset($_SERVER['HTTPS']) && $_SERVER['HTTPS'] === 'on' ? "https" : "http") . "://" . $_SERVER['HTTP_HOST'] . "/plugin/daily_poem/index.php"; ?>')">复制URL</button>
                        <button class="btn btn-secondary" onclick="window.open('/plugin/daily_poem/index.php', '_blank')">预览效果</button>
                    </div>
                </div>
                
                <!-- 每日英语单词插件 -->
                <div class="plugin-card">
                    <h3>每日英语单词</h3>
                    <div class="plugin-info">
                        <p><strong>类型:</strong> URL JSON插件</p>
                        <p><strong>描述:</strong> 每天获取一个英语单词，包含发音、释义和例句</p>
                        <p><strong>更新频率:</strong> 每天</p>
                        <p><strong>作者:</strong> iswhat</p>
                    </div>
                    <div class="plugin-url">
                        <strong>插件URL:</strong><br>
                        <?php echo (isset($_SERVER['HTTPS']) && $_SERVER['HTTPS'] === 'on' ? "https" : "http") . "://" . $_SERVER['HTTP_HOST'] . "/plugin/daily_word/index.php"; ?>
                    </div>
                    <div style="display: flex; gap: 12px;">
                        <button class="btn btn-primary" onclick="copyUrl('<?php echo (isset($_SERVER['HTTPS']) && $_SERVER['HTTPS'] === 'on' ? "https" : "http") . "://" . $_SERVER['HTTP_HOST'] . "/plugin/daily_word/index.php"; ?>')">复制URL</button>
                        <button class="btn btn-secondary" onclick="window.open('/plugin/daily_word/index.php', '_blank')">预览效果</button>
                    </div>
                </div>
            </div>
        </main>
        
        <footer>
            <p>&copy; 2025 InkClock. All rights reserved. | 开发者: <a href="https://github.com/iswhat" style="color: white; text-decoration: underline;">iswhat</a></p>
        </footer>
    </div>
    
    <script>
        // 复制URL到剪贴板
        function copyUrl(url) {
            navigator.clipboard.writeText(url).then(function() {
                // 显示复制成功提示
                alert('URL已复制到剪贴板！');
            }, function(err) {
                // 复制失败，使用旧方法
                var textArea = document.createElement('textarea');
                textArea.value = url;
                document.body.appendChild(textArea);
                textArea.select();
                document.execCommand('copy');
                document.body.removeChild(textArea);
                alert('URL已复制到剪贴板！');
            });
        }
    </script>
</body>
</html>