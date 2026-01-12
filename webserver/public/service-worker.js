/**
 * Service Worker for InkClock
 * 提供离线支持和资源缓存
 */

// 缓存名称和版本
const CACHE_NAME = 'inkclock-cache-v2';
const DYNAMIC_CACHE_NAME = 'inkclock-dynamic-cache-v2';

// 需要缓存的资源列表
const ASSETS_TO_CACHE = [
    // 主页面
    '/',
    '/index.html',
    '/login.html',
    '/dashboard.html',
    '/offline.html',
    '/devices.html',
    '/messages.html',
    '/notifications.html',
    '/profile.html',
    '/settings.html',
    
    // PWA 相关文件
    '/manifest.json',
    '/service-worker.js',
    
    // 优化后的资源
    '/dist/app.min.css',
    '/dist/app.min.js',
    '/js/menu.js',
    
    // 关键图片
    '/adminlte/img/AdminLTELogo.png',
    '/adminlte/img/avatar.png',
    
    // 字体图标
    'https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.4.0/css/all.min.css'
];

// 不应该缓存的URL模式
const CACHE_EXCLUDE_PATTERNS = [
    /^\/api/, // API请求
    /^\/admin/, // 管理员页面
    /\.php$/, // PHP文件
    /\.json$/, // JSON文件（除了manifest.json）
    /\?/ // 带查询参数的URL
];

// 安装事件 - 缓存关键资源
self.addEventListener('install', (event) => {
    event.waitUntil(
        caches.open(CACHE_NAME)
            .then((cache) => {
                console.log('Opened cache');
                return cache.addAll(ASSETS_TO_CACHE);
            })
            .then(() => self.skipWaiting())
    );
});

// 激活事件 - 清理旧缓存
self.addEventListener('activate', (event) => {
    const cacheWhitelist = [CACHE_NAME, DYNAMIC_CACHE_NAME];
    event.waitUntil(
        caches.keys().then((cacheNames) => {
            return Promise.all(
                cacheNames.map((cacheName) => {
                    if (cacheWhitelist.indexOf(cacheName) === -1) {
                        console.log('Deleting old cache:', cacheName);
                        return caches.delete(cacheName);
                    }
                })
            );
        }).then(() => self.clients.claim())
    );
});

// 检查URL是否应该被缓存
function shouldCache(url) {
    // 排除API请求和其他不应该缓存的URL
    for (const pattern of CACHE_EXCLUDE_PATTERNS) {
        if (pattern.test(url)) {
            return false;
        }
    }
    // 排除manifest.json和service-worker.js之外的JSON文件
    if (url.endsWith('.json') && !url.includes('manifest.json')) {
        return false;
    }
    return true;
}

//  fetch 事件 - 从缓存中提供资源
self.addEventListener('fetch', (event) => {
    const url = new URL(event.request.url);
    
    // 处理API请求
    if (url.pathname.startsWith('/api/')) {
        event.respondWith(handleApiRequest(event.request));
    } else {
        // 处理静态资源和页面请求
        event.respondWith(handleStaticRequest(event.request));
    }
});

// 处理API请求
async function handleApiRequest(request) {
    try {
        // 尝试从网络获取
        const response = await fetch(request);
        return response;
    } catch (error) {
        console.error('API request failed:', error);
        // API请求失败时返回错误响应
        return new Response(JSON.stringify({
            success: false,
            message: '网络连接失败，请检查网络后重试',
            offline: true
        }), {
            status: 408,
            headers: { 'Content-Type': 'application/json' }
        });
    }
}

// 处理静态资源请求
async function handleStaticRequest(request) {
    // 尝试从缓存中获取
    const cachedResponse = await caches.match(request);
    if (cachedResponse) {
        return cachedResponse;
    }
    
    try {
        // 尝试从网络获取
        const networkResponse = await fetch(request);
        
        // 检查响应是否有效
        if (!networkResponse || networkResponse.status !== 200 || networkResponse.type !== 'basic') {
            return networkResponse;
        }
        
        // 检查是否应该缓存
        if (shouldCache(request.url)) {
            // 克隆响应
            const responseToCache = networkResponse.clone();
            
            // 缓存资源
            const cache = await caches.open(DYNAMIC_CACHE_NAME);
            await cache.put(request, responseToCache);
        }
        
        return networkResponse;
    } catch (error) {
        console.error('Fetch failed:', error);
        // 网络请求失败时的回退策略
        if (request.mode === 'navigate') {
            const offlineResponse = await caches.match('/offline.html');
            return offlineResponse || new Response('网络连接失败', {
                status: 408,
                headers: { 'Content-Type': 'text/plain' }
            });
        }
        
        // 对于静态资源，返回404
        return new Response('资源不可用', {
            status: 404,
            headers: { 'Content-Type': 'text/plain' }
        });
    }
}

// 后台同步事件 - 当网络恢复时同步数据
self.addEventListener('sync', (event) => {
    console.log('Sync event:', event.tag);
    
    if (event.tag === 'sync-messages') {
        event.waitUntil(syncMessages());
    }
    if (event.tag === 'sync-settings') {
        event.waitUntil(syncSettings());
    }
    if (event.tag === 'sync-devices') {
        event.waitUntil(syncDevices());
    }
});

// 同步消息数据
async function syncMessages() {
    try {
        console.log('Syncing messages...');
        // 从IndexedDB获取待同步的消息
        const messages = await getPendingMessages();
        
        if (messages.length > 0) {
            // 发送消息到服务器
            for (const message of messages) {
                await sendMessageToServer(message);
                // 标记为已同步
                await markMessageAsSynced(message.id);
            }
        }
        
        // 通知客户端同步完成
        const clients = await self.clients.matchAll();
        clients.forEach(client => {
            client.postMessage({ 
                type: 'SYNC_COMPLETED', 
                data: { 
                    messages: true,
                    count: messages.length 
                } 
            });
        });
        
        console.log('Messages synced successfully');
    } catch (error) {
        console.error('Sync messages failed:', error);
        throw error;
    }
}

// 同步设置数据
async function syncSettings() {
    try {
        console.log('Syncing settings...');
        // 从IndexedDB获取待同步的设置
        const settings = await getPendingSettings();
        
        if (settings.length > 0) {
            // 发送设置到服务器
            for (const setting of settings) {
                await sendSettingToServer(setting);
                // 标记为已同步
                await markSettingAsSynced(setting.id);
            }
        }
        
        // 通知客户端同步完成
        const clients = await self.clients.matchAll();
        clients.forEach(client => {
            client.postMessage({ 
                type: 'SYNC_COMPLETED', 
                data: { 
                    settings: true,
                    count: settings.length 
                } 
            });
        });
        
        console.log('Settings synced successfully');
    } catch (error) {
        console.error('Sync settings failed:', error);
        throw error;
    }
}

// 同步设备数据
async function syncDevices() {
    try {
        console.log('Syncing devices...');
        // 从IndexedDB获取待同步的设备数据
        const devices = await getPendingDevices();
        
        if (devices.length > 0) {
            // 发送设备数据到服务器
            for (const device of devices) {
                await sendDeviceToServer(device);
                // 标记为已同步
                await markDeviceAsSynced(device.id);
            }
        }
        
        // 通知客户端同步完成
        const clients = await self.clients.matchAll();
        clients.forEach(client => {
            client.postMessage({ 
                type: 'SYNC_COMPLETED', 
                data: { 
                    devices: true,
                    count: devices.length 
                } 
            });
        });
        
        console.log('Devices synced successfully');
    } catch (error) {
        console.error('Sync devices failed:', error);
        throw error;
    }
}

// 模拟从IndexedDB获取待同步的消息
async function getPendingMessages() {
    // 实际实现中，这里应该从IndexedDB获取数据
    return [];
}

// 模拟从IndexedDB获取待同步的设置
async function getPendingSettings() {
    // 实际实现中，这里应该从IndexedDB获取数据
    return [];
}

// 模拟从IndexedDB获取待同步的设备数据
async function getPendingDevices() {
    // 实际实现中，这里应该从IndexedDB获取数据
    return [];
}

// 模拟发送消息到服务器
async function sendMessageToServer(message) {
    // 实际实现中，这里应该发送API请求
    console.log('Sending message to server:', message);
}

// 模拟发送设置到服务器
async function sendSettingToServer(setting) {
    // 实际实现中，这里应该发送API请求
    console.log('Sending setting to server:', setting);
}

// 模拟发送设备数据到服务器
async function sendDeviceToServer(device) {
    // 实际实现中，这里应该发送API请求
    console.log('Sending device to server:', device);
}

// 模拟标记消息为已同步
async function markMessageAsSynced(id) {
    // 实际实现中，这里应该更新IndexedDB
    console.log('Marking message as synced:', id);
}

// 模拟标记设置为已同步
async function markSettingAsSynced(id) {
    // 实际实现中，这里应该更新IndexedDB
    console.log('Marking setting as synced:', id);
}

// 模拟标记设备为已同步
async function markDeviceAsSynced(id) {
    // 实际实现中，这里应该更新IndexedDB
    console.log('Marking device as synced:', id);
}

// 推送通知事件
self.addEventListener('push', (event) => {
    if (!event.data) return;
    
    try {
        const data = event.data.json();
        const options = {
            body: data.body || '您有一条新通知',
            icon: '/adminlte/img/AdminLTELogo.png',
            badge: '/adminlte/img/avatar.png',
            vibrate: [100, 50, 100],
            data: {
                url: data.url || '/dashboard.html',
                id: data.id || Date.now()
            },
            actions: [
                {
                    action: 'view',
                    title: '查看详情'
                },
                {
                    action: 'dismiss',
                    title: '忽略'
                }
            ]
        };
        
        event.waitUntil(
            self.registration.showNotification(data.title || 'InkClock 通知', options)
        );
    } catch (error) {
        console.error('Push notification error:', error);
    }
});

// 通知点击事件
self.addEventListener('notificationclick', (event) => {
    event.notification.close();
    
    if (event.action === 'view') {
        event.waitUntil(
            clients.openWindow(event.notification.data.url || '/dashboard.html')
        );
    }
});

// 后台消息事件
self.addEventListener('message', (event) => {
    console.log('Message from client:', event.data);
    
    if (event.data && event.data.type === 'SKIP_WAITING') {
        self.skipWaiting();
    }
});

// 错误处理
self.addEventListener('error', (event) => {
    console.error('Service Worker error:', event.error);
});

// 未捕获的异常
self.addEventListener('unhandledrejection', (event) => {
    console.error('Unhandled rejection:', event.reason);
});