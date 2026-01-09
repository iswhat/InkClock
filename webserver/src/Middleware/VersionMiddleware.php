<?php
/**
 * 版本控制中间件
 * 用于处理不同版本的API请求
 */

namespace InkClock\Middleware;

class VersionMiddleware implements MiddlewareInterface {
    /**
     * 当前支持的API版本
     */
    const CURRENT_VERSION = 'v1';
    
    /**
     * 最低支持的API版本
     */
    const MIN_VERSION = 'v1';
    
    /**
     * 处理请求
     * @param array $request 请求信息
     * @param callable $next 下一个中间件
     * @return array 响应信息
     */
    public function handle($request, $next) {
        // 解析API版本
        $version = $this->parseVersion($request);
        
        // 验证版本
        if (!$this->isValidVersion($version)) {
            return [
                'success' => false,
                'message' => '不支持的API版本',
                'supported_versions' => [
                    'current' => self::CURRENT_VERSION,
                    'min' => self::MIN_VERSION
                ]
            ];
        }
        
        // 将版本信息添加到请求中
        $request['api_version'] = $version;
        
        // 继续处理请求
        $response = $next($request);
        
        // 添加版本信息到响应中
        $response['api_version'] = $version;
        $response['api_version_current'] = self::CURRENT_VERSION;
        
        return $response;
    }
    
    /**
     * 解析API版本
     * @param array $request 请求信息
     * @return string API版本
     */
    private function parseVersion($request) {
        $uri = $request['uri'] ?? '';
        
        // 从URI中解析版本
        if (preg_match('#^/api/(v[0-9]+)/#', $uri, $matches)) {
            return $matches[1];
        }
        
        // 从请求头中解析版本
        $headers = $request['headers'] ?? [];
        if (isset($headers['X-API-Version'])) {
            $version = $headers['X-API-Version'];
            if (preg_match('#^v[0-9]+$#', $version)) {
                return $version;
            }
        }
        
        // 从查询参数中解析版本
        $params = $request['params'] ?? [];
        if (isset($params['api_version'])) {
            $version = $params['api_version'];
            if (preg_match('#^v[0-9]+$#', $version)) {
                return $version;
            }
        }
        
        // 默认使用当前版本
        return self::CURRENT_VERSION;
    }
    
    /**
     * 验证API版本
     * @param string $version API版本
     * @return bool 是否有效
     */
    private function isValidVersion($version) {
        // 提取版本号数字部分
        if (preg_match('#^v([0-9]+)$#', $version, $matches)) {
            $versionNum = (int)$matches[1];
            
            // 提取最低版本号数字部分
            preg_match('#^v([0-9]+)$#', self::MIN_VERSION, $minMatches);
            $minVersionNum = (int)$minMatches[1];
            
            // 验证版本号是否在有效范围内
            return $versionNum >= $minVersionNum;
        }
        
        return false;
    }
}
?>