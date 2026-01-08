<?php
/**
 * 消息服务接口
 */

namespace InkClock\Interfaces;

interface MessageServiceInterface {
    /**
     * 发送消息
     * @param string $deviceId 设备ID
     * @param string $sender 发送者
     * @param string $content 消息内容
     * @param string $type 消息类型
     * @return array 发送结果
     */
    public function sendMessage($deviceId, $sender, $content, $type = 'text');
    
    /**
     * 获取设备消息
     * @param string $deviceId 设备ID
     * @param array $filters 过滤条件
     * @return array 消息列表
     */
    public function getDeviceMessages($deviceId, $filters = []);
    
    /**
     * 标记消息为已读
     * @param string $messageId 消息ID
     * @return array 操作结果
     */
    public function markMessageAsRead($messageId);
    
    /**
     * 删除消息
     * @param string $messageId 消息ID
     * @return array 删除结果
     */
    public function deleteMessage($messageId);
    
    /**
     * 获取未读消息数
     * @param string $deviceId 设备ID
     * @return int 未读消息数
     */
    public function getUnreadMessageCount($deviceId);
    
    /**
     * 批量标记消息为已读
     * @param array $messageIds 消息ID数组
     * @return array 操作结果
     */
    public function batchMarkAsRead($messageIds);
    
    /**
     * 批量删除消息
     * @param array $messageIds 消息ID数组
     * @return array 删除结果
     */
    public function batchDeleteMessages($messageIds);
}
?>