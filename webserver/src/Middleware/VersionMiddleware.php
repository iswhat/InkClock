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
     * 所有支持的API版本
     */
    const SUPPORTED_VERSIONS = ['v1'];
    
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
            http_response_code(400);
            header('Content-Type: application/json');
            echo json_encode([
                'success' => false,
                'message' => '不支持的API版本',
                'supported_versions' => [
                    'current' => self::CURRENT_VERSION,
                    'min' => self::MIN_VERSION,
                    'all' => self::SUPPORTED_VERSIONS
                ]
            ]);
            exit;
        }
        
        // 将版本信息添加到请求中
        $request['api_version'] = $version;
        
        // 继续处理请求
        $response = $next($request);
        
        // 添加版本信息到响应中
        if (is_array($response)) {
            $response['api_version'] = $version;
            $response['api_version_current'] = self::CURRENT_VERSION;
            $response['api_version_supported'] = self::SUPPORTED_VERSIONS;
        }
        
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
        // 检查不同的头名称格式
        $headerNames = ['X-API-Version', 'X-Api-Version', 'x-api-version'];
        foreach ($headerNames as $headerName) {
            if (isset($headers[$headerName])) {
                $version = $headers[$headerName];
                if (preg_match('#^v[0-9]+$#', $version)) {
                    return $version;
                }
            }
        }
        
        // 从查询参数中解析版本
        $query = $request['query'] ?? [];
        if (isset($query['api_version'])) {
            $version = $query['api_version'];
            if (preg_match('#^v[0-9]+$#', $version)) {
                return $version;
            }
        }
        if (isset($query['version'])) {
            $version = $query['version'];
            if (preg_match('#^v[0-9]+$#', $version)) {
                return $version;
            }
        }
        
        // 从请求体中解析版本（JSON格式）
        $body = $request['body'] ?? '';
        if (!empty($body)) {
            $bodyData = json_decode($body, true);
            if (json_last_error() === JSON_ERROR_NONE) {
                if (isset($bodyData['api_version'])) {
                    $version = $bodyData['api_version'];
                    if (preg_match('#^v[0-9]+$#', $version)) {
                        return $version;
                    }
                }
                if (isset($bodyData['version'])) {
                    $version = $bodyData['version'];
                    if (preg_match('#^v[0-9]+$#', $version)) {
                        return $version;
                    }
                }
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
        // 检查版本是否在支持列表中
        if (in_array($version, self::SUPPORTED_VERSIONS)) {
            return true;
        }
        
        // 提取版本号数字部分进行范围检查
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
    
    /**
     * 获取版本的数字部分
     * @param string $version API版本字符串
     * @return int 版本数字
     */
    public static function getVersionNumber($version) {
        if (preg_match('#^v([0-9]+)$#', $version, $matches)) {
            return (int)$matches[1];
        }
        return 0;
    }
    
    /**
     * 比较两个版本
     * @param string $version1 第一个版本
     * @param string $version2 第二个版本
     * @return int -1: version1 < version2, 0: version1 == version2, 1: version1 > version2
     */
    public static function compareVersions($version1, $version2) {
        $v1 = self::getVersionNumber($version1);
        $v2 = self::getVersionNumber($version2);
        
        if ($v1 < $v2) {
            return -1;
        } elseif ($v1 > $v2) {
            return 1;
        } else {
            return 0;
        }
    }
}
?>