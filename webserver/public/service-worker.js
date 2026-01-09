/**
 * Service Worker for InkClock
 * 提供离线支持和资源缓存
 */

// 缓存名称和版本
const CACHE_NAME = 'inkclock-cache-v1';

// 需要缓存的资源列表
const ASSETS_TO_CACHE = [
    // 主页面
    '/',
    '/index.html',
    '/login.html',
    '/dashboard.html',
    '/offline.html',
    
    // PWA 相关文件
    '/manifest.json',
    '/service-worker.js',
    
    // 优化后的资源
    '/dist/app.min.css',
    '/dist/app.min.js',
    
    // 关键图片
    '/adminlte/img/AdminLTELogo.png',
    '/adminlte/img/avatar.png',
    
    // 字体图标
    'https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.4.0/css/all.min.css'
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
    const cacheWhitelist = [CACHE_NAME];
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

//  fetch 事件 - 从缓存中提供资源
self.addEventListener('fetch', (event) => {
    event.respondWith(
        caches.match(event.request)
            .then((response) => {
                // 如果缓存中有资源，直接返回
                if (response) {
                    return response;
                }
                
                // 否则，尝试从网络获取
                return fetch(event.request)
                    .then((response) => {
                        // 检查响应是否有效
                        if (!response || response.status !== 200 || response.type !== 'basic') {
                            return response;
                        }
                        
                        // 克隆响应，因为响应流只能使用一次
                        const responseToCache = response.clone();
                        
                        // 将新资源添加到缓存
                        caches.open(CACHE_NAME)
                            .then((cache) => {
                                cache.put(event.request, responseToCache);
                            });
                        
                        return response;
                    })
                    .catch(() => {
                        // 网络请求失败时的回退策略
                        if (event.request.mode === 'navigate') {
                            return caches.match('/offline.html');
                        }
                    });
            })
    );
});

// 后台同步事件 - 当网络恢复时同步数据
self.addEventListener('sync', (event) => {
    if (event.tag === 'sync-messages') {
        event.waitUntil(syncMessages());
    }
    if (event.tag === 'sync-settings') {
        event.waitUntil(syncSettings());
    }
});

// 同步消息数据
async function syncMessages() {
    try {
        const clients = await self.clients.matchAll();
        clients.forEach(client => {
            client.postMessage({ type: 'SYNC_COMPLETED', data: { messages: true } });
        });
    } catch (error) {
        console.error('Sync messages failed:', error);
    }
}

// 同步设置数据
async function syncSettings() {
    try {
        const clients = await self.clients.matchAll();
        clients.forEach(client => {
            client.postMessage({ type: 'SYNC_COMPLETED', data: { settings: true } });
        });
    } catch (error) {
        console.error('Sync settings failed:', error);
    }
}

// 推送通知事件
self.addEventListener('push', (event) => {
    if (!event.data) return;
    
    try {
        const data = event.data.json();
        const options = {
            body: data.body || 'You have a new notification',
            icon: '/adminlte/img/AdminLTELogo.png',
            badge: '/adminlte/img/avatar.png',
            vibrate: [100, 50, 100],
            data: {
                url: data.url || '/dashboard.html'
            }
        };
        
        event.waitUntil(
            self.registration.showNotification(data.title || 'InkClock Notification', options)
        );
    } catch (error) {
        console.error('Push notification error:', error);
    }
});

// 通知点击事件
self.addEventListener('notificationclick', (event) => {
    event.notification.close();
    
    event.waitUntil(
        clients.openWindow(event.notification.data.url || '/dashboard.html')
    );
});