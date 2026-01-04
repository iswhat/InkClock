<?php
/**
 * 消息服务接口
 */

namespace InkClock\Interface;

interface MessageServiceInterface {
    /**
     * 发送消息到设备
     * @param array $messageInfo 消息信息
     * @return array 发送结果
     */
    public function sendMessage($messageInfo);
    
    /**
     * 获取消息列表
     * @param int $userId 用户ID
     * @param array $filters 过滤条件
     * @return array 消息列表
     */
    public function getMessageList($userId, $filters = []);
    
    /**
     * 获取设备的消息
     * @param int $userId 用户ID
     * @param string $deviceId 设备ID
     * @return array 设备消息
     */
    public function getDeviceMessages($userId, $deviceId);
    
    /**
     * 标记消息为已读
     * @param string $messageId 消息ID
     * @return array 操作结果
     */
    public function markMessageAsRead($messageId);
    
    /**
     * 删除消息
     * @param int $userId 用户ID
     * @param string $messageId 消息ID
     * @return array 操作结果
     */
    public function deleteMessage($userId, $messageId);
    
    /**
     * 批量发送消息
     * @param int $userId 用户ID
     * @param array $batchInfo 批量消息信息
     * @return array 发送结果
     */
    public function sendBatchMessages($userId, $batchInfo);
}
?>