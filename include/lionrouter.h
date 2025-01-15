#ifndef LIONROUTER_H
#define LIONROUTER_H

#include <map>
#include <string>
#include <vector>
#include <set>
#include <atomic>
#include <thread>
#include <curl/curl.h>
#include "../deps/json/json.hpp"
using json = nlohmann::json;


class LionRouter {
public:
    // 获取单例实例
    static LionRouter& getInstance();

    // 初始化 TiDB 和 Store 的映射关系
    void InitTidb2Store(const std::string& path);

    // 初始化 Region 和 Store 的映射关系
    void InitRegion2Store(const std::string& pd_url);
    void UpdateRegion2Store(const std::string& response);

    // 初始化 Tikv 和 Store 的映射关系
    void InitTikv2Store(const std::string& tiup_command_url);
    void UpdateTikv2Store(const std::string& response);

    // 获取某个虚拟 region 的主节点 store_id
    int GetRegionPrimaryStoreId(int virtual_region_id) const;

    // 获取某个虚拟 region 的从节点 store_id 列表
    std::vector<int> GetRegionSecondaryStoreId(int virtual_region_id) const;

    // 获取所有 store_id
    std::set<int> GetAllStoreIds() const;

    // 根据 TiDB 名称获取对应的 Store
    std::string GetStoreForTidb(const std::string& tidb) const;

    // 根据 Region ID 获取对应的主副本 Store
    int GetStoreForRegion(int actual_region_id) const;

    // 根据 TiKV IP 获取对应的 Store ID
    int GetStoreIDForTiKV(const std::string& tikv_ip) const;

    // 根据 Store ID 获取对应的 TiKV IP
    std::string GetTiKVForStoreID(int store_id) const;

    // 解析 SQL 语句中的 YCSB_KEY，返回涉及的 region_id 数组
    std::vector<int> ParseYcsbKey(const std::string& sql) const;

    // 根据 region_id 数组，计算最优的 hostgroupid
    int EvaluateHost(const std::vector<int>& region_ids) const;


    // 禁止复制和赋值
    LionRouter(const LionRouter&) = delete;
    LionRouter& operator=(const LionRouter&) = delete;

private:
    // 私有构造函数
    LionRouter();
    ~LionRouter();

    // 成员变量
    std::map<std::string, std::string> tidb2store;          // TiDB IP -> TiKV IP
    std::map<std::string, std::string> store2tidb;                //  TiKV Store ID -> TiDB IP 

    std::map<std::string, int> tidb2hostgroup;              // TiDB IP -> Hostgroup
    // 
    std::map<std::string, int> tikv2storeID;                // TiKV IP -> TiKV Store ID
    std::map<int, std::string> storeID2tikv;                // TiKV IP -> TiKV Store ID
    std::set<int> store_ids_;                               // 所有 store_id

    // 双缓冲机制
    struct MetaInfo {
        std::map<int, int> virtual_region_id_map_;               // 虚拟 region_id -> 实际 region_id
        std::map<int, int> region_primary_store_id_;             // 实际 region_id -> 主节点 store_id
        std::map<int, std::vector<int>> region_secondary_store_id_; // 实际 region_id -> 从节点 store_id 列表
    };

    MetaInfo meta_info_[2]; // 双缓冲
    std::atomic<int> version_{0}; // 当前读写版本

    // 线程相关成员变量
    std::thread update_thread_;
    std::atomic<bool> running_;
    const int UPDATE_INTERVAL = 30; // 更新间隔，单位为秒
    static const int REGION_SIZE = 10000; // 分区大小
    int weight_ = 10; // 主副本的权重

    // 辅助函数：从 JSON 文件读取数据
    nlohmann::json ReadJsonFile(const std::string& path);

    // 辅助函数：通过 CURL 获取远程数据
    std::string FetchRemoteData(const std::string& url);

    static size_t WriteCallback(void* ptr, size_t size, size_t nmemb, std::string* data);

    // 更新线程函数
    void UpdateThreadFunction();
};

#endif // LIONROUTER_H