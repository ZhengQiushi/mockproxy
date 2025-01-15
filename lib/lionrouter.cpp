#include "../include/lionrouter.h"
#include "curl/curl.h"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <iostream>
#include <cstdio>

#include <regex>
#include <algorithm>

// 单例实例
LionRouter& LionRouter::getInstance() {
    static LionRouter instance;
    return instance;
}

// 私有构造函数
LionRouter::LionRouter() {
}


LionRouter::~LionRouter() {
}

// 从 JSON 文件读取数据
nlohmann::json LionRouter::ReadJsonFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + path);
    }

    nlohmann::json root;
    try {
        file >> root;
    } catch (const nlohmann::json::parse_error& e) {
        throw std::runtime_error("Failed to parse JSON: " + std::string(e.what()));
    }

    return root;
}
    std::string LionRouter::FetchRemoteData(const std::string& url) {
        CURL* curl = curl_easy_init();
        if (!curl) {
            throw std::runtime_error("Failed to initialize CURL");
        }

        std::string response;
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        // 设置 SSL 验证选项（禁用验证）
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

        // 设置超时选项
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10); // 请求超时时间为 10 秒
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10); // 连接超时时间为 10 秒

        // 执行请求
        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::string error_msg = "CURL request failed: ";
            error_msg += curl_easy_strerror(res);
            curl_easy_cleanup(curl);
            throw std::runtime_error(error_msg);
        }

        // 清理 CURL 句柄
        curl_easy_cleanup(curl);

        // 返回响应数据
        return response;
    }

    size_t LionRouter::WriteCallback(void* ptr, size_t size, size_t nmemb, std::string* data) {
        data->append((char*)ptr, size * nmemb);
        return size * nmemb;
    }
    
// 初始化 TiDB 和 Store 的映射关系
void LionRouter::InitTidb2Store(const std::string& path) {
    nlohmann::json root = ReadJsonFile(path);

    for (const auto& [key, value] : root.items()) {
        tidb2store[key] = value["tikv_ip"].get<std::string>();
        store2tidb[value["tikv_ip"].get<std::string>()] = key;
        tidb2hostgroup[key] = value["hostgroup"].get<int>();
    }
}

// 初始化 Region 和 Store 的映射关系
void LionRouter::InitRegion2Store(const std::string& pd_url) {
    std::string response = FetchRemoteData(pd_url);
    UpdateRegion2Store(response);
}

// 根据 TiDB 名称获取对应的 Store
std::string LionRouter::GetStoreForTidb(const std::string& tidb) const {
    auto it = tidb2store.find(tidb);
    if (it != tidb2store.end()) {
        return it->second;
    }
    return ""; // 如果找不到，返回空字符串
}

// 根据 Region ID 获取对应的主副本 Store
int LionRouter::GetStoreForRegion(int actual_region_id) const {
    auto it = region_primary_store_id_.find(actual_region_id);
    if (it != region_primary_store_id_.end()) {
        return it->second; // 假设 Store 名称格式为 "storeX"
    }
    // throw std::runtime_error("实际 region_id " + std::to_string(actual_region_id) + " 没有主节点信息");
    return -1; // 如果找不到，返回空字符串
}

void LionRouter::InitTikv2Store(const std::string& pd_url) {
    std::string response = FetchRemoteData(pd_url);
    // 更新 TiKV 和 Store 的映射关系
    UpdateTikv2Store(response);
}

void LionRouter::UpdateTikv2Store(const std::string& response) {
    try {
        // 解析 JSON 数据
        json json_data = json::parse(response);

        // 遍历 stores 数组
        for (const auto& store : json_data["stores"]) {
            int store_id = store["store"]["id"];
            std::string address = store["store"]["address"];

            // 提取 IP 地址
            std::string tikv_ip = address.substr(0, address.find(':'));

            // 填充映射关系
            tikv2storeID[tikv_ip] = store_id;
            storeID2tikv[store_id] = tikv_ip;
        }
    } catch (const json::parse_error& e) {
        // JSON 解析错误
        std::cerr << "JSON parse error: " << e.what() << std::endl;
    } catch (const json::type_error& e) {
        // JSON 类型错误（例如访问不存在的字段）
        std::cerr << "JSON type error: " << e.what() << std::endl;
    } catch (const std::exception& e) {
        // 其他异常
        std::cerr << "Error in UpdateTikv2Store: " << e.what() << std::endl;
    }
}

// 根据 TiKV IP 获取对应的 Store ID
int LionRouter::GetStoreIDForTiKV(const std::string& tikv_ip) const {
    auto it = tikv2storeID.find(tikv_ip);
    if (it != tikv2storeID.end()) {
        return it->second;
    }
    return -1; // 未找到
}

// 根据 Store ID 获取对应的 TiKV IP
std::string LionRouter::GetTiKVForStoreID(int store_id) const {
    auto it = storeID2tikv.find(store_id);
    if (it != storeID2tikv.end()) {
        return it->second;
    }
    return ""; // 未找到
}

// 更新路由信息
void LionRouter::UpdateRegion2Store(const std::string& response) {
    json data = json::parse(response);
    auto regions = data.find("record_regions");
    if (regions == data.end()) {
        throw std::runtime_error("Invalid JSON data: missing 'record_regions'");
    }

    virtual_region_id_map_.clear();
    region_primary_store_id_.clear();
    region_secondary_store_id_.clear();
    store_ids_.clear();

    for (size_t virtual_id = 0; virtual_id < regions->size(); ++virtual_id) {
        const auto& region = (*regions)[virtual_id];
        int actual_id = region["region_id"];
        virtual_region_id_map_[virtual_id] = actual_id;

        const auto& leader = region["leader"];
        const auto& peers = region["peers"];

        // 更新主节点
        region_primary_store_id_[actual_id] = leader["store_id"];

        // 更新从节点
        std::vector<int> secondary_store_ids;
        for (const auto& peer : peers) {
            if (peer["id"] != leader["id"]) {
                secondary_store_ids.push_back(peer["store_id"]);
            }
        }
        region_secondary_store_id_[actual_id] = secondary_store_ids;

        // 更新所有 store_id
        store_ids_.insert(leader["store_id"].get<int>());
        for (const auto& peer : peers) {
            store_ids_.insert(peer["store_id"].get<int>());
        }
    }
}


// 获取某个虚拟 region 的主节点 store_id
int LionRouter::GetRegionPrimaryStoreId(int virtual_region_id) const {
    auto it = virtual_region_id_map_.find(virtual_region_id);
    if (it == virtual_region_id_map_.end()) {
        throw std::runtime_error("虚拟 region_id " + std::to_string(virtual_region_id) + " 不存在");
    }
    int actual_region_id = it->second;

    return GetStoreForRegion(actual_region_id);
}

// 获取某个虚拟 region 的从节点 store_id 列表
std::vector<int> LionRouter::GetRegionSecondaryStoreId(int virtual_region_id) const {
    auto it = virtual_region_id_map_.find(virtual_region_id);
    if (it == virtual_region_id_map_.end()) {
        throw std::runtime_error("虚拟 region_id " + std::to_string(virtual_region_id) + " 不存在");
    }
    int actual_region_id = it->second;

    auto secondary_it = region_secondary_store_id_.find(actual_region_id);
    if (secondary_it == region_secondary_store_id_.end()) {
        throw std::runtime_error("实际 region_id " + std::to_string(actual_region_id) + " 没有从节点信息");
    }
    return secondary_it->second;
}

// 获取所有 store_id
std::set<int> LionRouter::GetAllStoreIds() const {
    return store_ids_;
}


// 解析 SQL 语句中的 YCSB_KEY，返回涉及的 region_id 数组
std::vector<int> LionRouter::ParseYcsbKey(const std::string& sql) const {
    std::vector<int> region_ids;
    std::regex key_pattern(R"(YCSB_KEY\s*=\s*(\d+))"); // 匹配 YCSB_KEY = <数字>
    std::smatch matches;

    // 查找所有匹配的 YCSB_KEY
    std::string::const_iterator search_start(sql.cbegin());
    while (std::regex_search(search_start, sql.cend(), matches, key_pattern)) {
        int key = std::stoi(matches[1].str());
        int region_id = key / REGION_SIZE; // 计算 region_id
        region_ids.push_back(region_id);
        search_start = matches.suffix().first;
    }

    // 去重
    std::sort(region_ids.begin(), region_ids.end());
    region_ids.erase(std::unique(region_ids.begin(), region_ids.end()), region_ids.end());

    return region_ids;
}

// 根据 region_id 数组，计算最优的 hostgroupid
int LionRouter::EvaluateHost(const std::vector<int>& region_ids) const {
    std::map<int, int> costs; // store_id -> 开销

    // 遍历所有 store_id
    for (int store_id : GetAllStoreIds()) {
        int primary_count = 0;
        int secondary_count = 0;

        // 计算主副本和从副本的数量
        for (int region_id : region_ids) {
            int primary_store_id = GetRegionPrimaryStoreId(region_id);
            std::vector<int> secondary_store_ids = GetRegionSecondaryStoreId(region_id);

            if (primary_store_id == store_id) {
                primary_count++;
            }
            if (std::find(secondary_store_ids.begin(), secondary_store_ids.end(), store_id) != secondary_store_ids.end()) {
                secondary_count++;
            }
        }

        // 计算开销
        int cost = -(primary_count * weight_ + secondary_count);
        costs[store_id] = cost;
    }

    // 选择开销最小的 store_id
    auto min_cost_it = std::min_element(costs.begin(), costs.end(),
        [](const std::pair<int, int>& a, const std::pair<int, int>& b) {
            return a.second < b.second;
        });

    int best_store_id = min_cost_it->first;

    // 找到与 store_id 相邻部署的 TiDB
    std::string best_tikv_ip = GetTiKVForStoreID(best_store_id);
    if (best_tikv_ip.empty()) {
        throw std::runtime_error("No TiKV found for store_id: " + std::to_string(best_store_id));
    }

    // 找到与 TiKV 相邻部署的 TiDB
    auto tidb_it = store2tidb.find(best_tikv_ip);
    if (tidb_it == store2tidb.end()) {
        throw std::runtime_error("No TiDB found for TiKV StoreID: " + best_store_id);
    }
    std::string best_tidb_ip = tidb_it->second;

    // 找到 TiDB 对应的 hostgroupid
    auto hostgroup_it = tidb2hostgroup.find(best_tidb_ip);
    if (hostgroup_it == tidb2hostgroup.end()) {
        throw std::runtime_error("No hostgroup found for TiDB IP: " + best_tidb_ip);
    }

    return hostgroup_it->second; // 返回最优的 hostgroupid
}