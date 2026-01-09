// 统一的菜单模板
const menuTemplate = `
<!-- Sidebar Menu -->
<nav class="mt-2">
    <ul class="nav nav-pills nav-sidebar flex-column" data-widget="treeview" role="menu" data-accordion="false">
        <!-- Add icons to the links using the .nav-icon class
             with font-awesome or any other icon font library -->
        <li class="nav-item">
            <a href="dashboard.html" class="nav-link">
                <i class="nav-icon fas fa-tachometer-alt"></i>
                <p>仪表盘</p>
            </a>
        </li>
        <li class="nav-item">
            <a href="devices.html" class="nav-link">
                <i class="nav-icon fas fa-microchip"></i>
                <p>设备管理</p>
            </a>
        </li>
        <li class="nav-item">
            <a href="messages.html" class="nav-link">
                <i class="nav-icon fas fa-envelope"></i>
                <p>消息管理</p>
            </a>
        </li>
        <li class="nav-item admin-only" style="display: none;">
            <a href="firmware.html" class="nav-link">
                <i class="nav-icon fas fa-download"></i>
                <p>固件管理</p>
            </a>
        </li>
        <li class="nav-item has-treeview">
            <a href="#" class="nav-link">
                <i class="nav-icon fas fa-sitemap"></i>
                <p>
                    设备分组
                    <i class="right fas fa-angle-left"></i>
                </p>
            </a>
            <ul class="nav nav-treeview">
                <li class="nav-item">
                    <a href="groups.html" class="nav-link">
                        <i class="far fa-circle nav-icon"></i>
                        <p>分组管理</p>
                    </a>
                </li>
                <li class="nav-item">
                    <a href="tags.html" class="nav-link">
                        <i class="far fa-circle nav-icon"></i>
                        <p>标签管理</p>
                    </a>
                </li>
            </ul>
        </li>
        <li class="nav-item">
            <a href="notifications.html" class="nav-link">
                <i class="nav-icon fas fa-bell"></i>
                <p>通知管理</p>
            </a>
        </li>
        <li class="nav-item">
            <a href="plugins.html" class="nav-link">
                <i class="nav-icon fas fa-puzzle-piece"></i>
                <p>插件管理</p>
            </a>
        </li>
        <!-- Admin only menu items -->
        <li class="nav-item has-treeview admin-only" style="display: none;">
            <a href="#" class="nav-link">
                <i class="nav-icon fas fa-user-shield"></i>
                <p>
                    管理员
                    <i class="right fas fa-angle-left"></i>
                </p>
            </a>
            <ul class="nav nav-treeview">
                <li class="nav-item">
                    <a href="users.html" class="nav-link">
                        <i class="far fa-circle nav-icon"></i>
                        <p>用户管理</p>
                    </a>
                </li>
                <li class="nav-item">
                    <a href="system.html" class="nav-link">
                        <i class="far fa-circle nav-icon"></i>
                        <p>系统设置</p>
                    </a>
                </li>
                <li class="nav-item">
                    <a href="logs.html" class="nav-link">
                        <i class="far fa-circle nav-icon"></i>
                        <p>系统日志</p>
                    </a>
                </li>
            </ul>
        </li>
    </ul>
</nav>
<!-- /.sidebar-menu -->
`;

// 初始化菜单
function initMenu(activePage) {
    // 替换侧边栏菜单
    const sidebarMenu = document.querySelector('.sidebar > nav');
    if (sidebarMenu) {
        sidebarMenu.innerHTML = menuTemplate;
    }
    
    // 设置当前页面为活动状态
    if (activePage) {
        const activeLink = document.querySelector(`a[href="${activePage}"]`);
        if (activeLink) {
            // 移除所有活动状态
            document.querySelectorAll('.nav-link.active').forEach(link => {
                link.classList.remove('active');
            });
            
            // 设置当前链接为活动状态
            activeLink.classList.add('active');
            
            // 如果是子菜单，展开父菜单
            let parent = activeLink.closest('.nav-item.has-treeview');
            if (parent) {
                parent.classList.add('menu-open');
                parent.querySelector('.nav-link').classList.add('active');
            }
        }
    }
    
    // 检查登录状态并显示管理员菜单
    checkAdminMenu();
}

// 检查管理员菜单显示
function checkAdminMenu() {
    const userStr = localStorage.getItem('user');
    if (userStr) {
        const user = JSON.parse(userStr);
        if (user.role === 'admin') {
            document.querySelectorAll('.admin-only').forEach(item => {
                item.style.display = '';
            });
        }
    }
}
