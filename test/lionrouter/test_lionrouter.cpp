#include "gtest/gtest.h"
#include "../../include/lionrouter.h"

class LionRouterTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 初始化测试数据
        router = &LionRouter::getInstance();
        router->InitTidb2Store("test_data/tidb2store.json");
        // router->InitRegion2Store("http://10.77.70.210:10080/tables/benchbase/usertable/regions");
        // router->InitTikv2Store("http://10.77.70.250:12379/pd/api/v1/stores");
        // 加载 mock 数据
        const char* mock_data = R"(
        {
            "name": "usertable",
            "id": 112,
            "record_regions": [
                {
                    "region_id": 44,
                    "leader": {
                        "id": 45,
                        "store_id": 1
                    },
                    "peers": [
                        {
                            "id": 45,
                            "store_id": 1
                        },
                        {
                            "id": 266,
                            "store_id": 7
                        }
                    ],
                    "region_epoch": {
                        "conf_ver": 5,
                        "version": 60
                    }
                },
                {
                    "region_id": 54,
                    "leader": {
                        "id": 214,
                        "store_id": 5
                    },
                    "peers": [
                        {
                            "id": 55,
                            "store_id": 1
                        },
                        {
                            "id": 214,
                            "store_id": 5
                        },
                        {
                            "id": 276,
                            "store_id": 7
                        }
                    ],
                    "region_epoch": {
                        "conf_ver": 5,
                        "version": 60
                    }
                }
            ],
            "indices": []
        })";

        const char* mock_tikv_data = R"(

        {
        "count": 5,
        "stores": [
            {
            "store": {
                "id": 1,
                "address": "10.77.70.205:20172",
                "labels": [
                {
                    "key": "host",
                    "value": "logic-host-5"
                }
                ],
                "version": "8.5.0",
                "peer_address": "10.77.70.205:20172",
                "status_address": "10.77.70.205:20192",
                "git_hash": "a2c58c94f89cbb410e66d8f85c236308d6fc64f0",
                "start_timestamp": 1736860054,
                "deploy_path": "/data2/tidb-deploy/tikv-20172/bin",
                "last_heartbeat": 1736910992696585908,
                "state_name": "Up"
            },
            "status": {
                "capacity": "500GiB",
                "available": "412.4GiB",
                "used_size": "11.13GiB",
                "leader_count": 18,
                "leader_weight": 1,
                "leader_score": 18,
                "leader_size": 4943,
                "region_count": 40,
                "region_weight": 1,
                "region_score": 8155.295662644787,
                "region_size": 4965,
                "slow_score": 1,
                "slow_trend": {
                "cause_value": 250075.8909395973,
                "cause_rate": 0,
                "result_value": 17,
                "result_rate": 0
                },
                "start_ts": "2025-01-14T13:07:34Z",
                "last_heartbeat_ts": "2025-01-15T03:16:32.696585908Z",
                "uptime": "14h8m58.696585908s"
            }
            },
            {
            "store": {
                "id": 5,
                "address": "10.77.70.208:20172",
                "labels": [
                {
                    "key": "host",
                    "value": "logic-host-3"
                }
                ],
                "version": "8.5.0",
                "peer_address": "10.77.70.208:20172",
                "status_address": "10.77.70.208:20192",
                "git_hash": "a2c58c94f89cbb410e66d8f85c236308d6fc64f0",
                "start_timestamp": 1736860078,
                "deploy_path": "/data2/tidb-deploy/tikv-20172/bin",
                "last_heartbeat": 1736910992900982972,
                "state_name": "Up"
            },
            "status": {
                "capacity": "500GiB",
                "available": "351.9GiB",
                "used_size": "11.12GiB",
                "leader_count": 12,
                "leader_weight": 1,
                "leader_score": 12,
                "leader_size": 12,
                "region_count": 38,
                "region_weight": 1,
                "region_score": 8477.734091431255,
                "region_size": 4963,
                "slow_score": 1,
                "slow_trend": {
                "cause_value": 250095.86912751678,
                "cause_rate": 0,
                "result_value": 2.5,
                "result_rate": 0
                },
                "start_ts": "2025-01-14T13:07:58Z",
                "last_heartbeat_ts": "2025-01-15T03:16:32.900982972Z",
                "uptime": "14h8m34.900982972s"
            }
            },
            {
            "store": {
                "id": 4,
                "address": "10.77.70.209:20172",
                "labels": [
                {
                    "key": "host",
                    "value": "logic-host-4"
                }
                ],
                "version": "8.5.0",
                "peer_address": "10.77.70.209:20172",
                "status_address": "10.77.70.209:20192",
                "git_hash": "a2c58c94f89cbb410e66d8f85c236308d6fc64f0",
                "start_timestamp": 1736860076,
                "deploy_path": "/data2/tidb-deploy/tikv-20172/bin",
                "last_heartbeat": 1736910992545108634,
                "state_name": "Up"
            },
            "status": {
                "capacity": "500GiB",
                "available": "369.8GiB",
                "used_size": "5.287GiB",
                "leader_count": 11,
                "leader_weight": 1,
                "leader_score": 11,
                "leader_size": 11,
                "region_count": 38,
                "region_weight": 1,
                "region_score": 64.02734432668207,
                "region_size": 38,
                "slow_score": 1,
                "slow_trend": {
                "cause_value": 250087.8422818792,
                "cause_rate": 0,
                "result_value": 7,
                "result_rate": 0
                },
                "start_ts": "2025-01-14T13:07:56Z",
                "last_heartbeat_ts": "2025-01-15T03:16:32.545108634Z",
                "uptime": "14h8m36.545108634s"
            }
            },
            {
            "store": {
                "id": 7,
                "address": "10.77.70.207:20171",
                "labels": [
                {
                    "key": "host",
                    "value": "logic-host-2"
                }
                ],
                "version": "8.5.0",
                "peer_address": "10.77.70.207:20171",
                "status_address": "10.77.70.207:20191",
                "git_hash": "a2c58c94f89cbb410e66d8f85c236308d6fc64f0",
                "start_timestamp": 1736860081,
                "deploy_path": "/data2/tidb-deploy/tikv-20171/bin",
                "last_heartbeat": 1736910992733302033,
                "state_name": "Up"
            },
            "status": {
                "capacity": "500GiB",
                "available": "351.4GiB",
                "used_size": "11.24GiB",
                "leader_count": 12,
                "leader_weight": 1,
                "leader_score": 12,
                "leader_size": 12,
                "region_count": 38,
                "region_weight": 1,
                "region_score": 8477.432806029858,
                "region_size": 4963,
                "slow_score": 1,
                "slow_trend": {
                "cause_value": 250080.62248322146,
                "cause_rate": 0,
                "result_value": 26.5,
                "result_rate": 0
                },
                "start_ts": "2025-01-14T13:08:01Z",
                "last_heartbeat_ts": "2025-01-15T03:16:32.733302033Z",
                "uptime": "14h8m31.733302033s"
            }
            },
            {
            "store": {
                "id": 6,
                "address": "10.77.70.206:20170",
                "labels": [
                {
                    "key": "host",
                    "value": "logic-host-1"
                }
                ],
                "version": "8.5.0",
                "peer_address": "10.77.70.206:20170",
                "status_address": "10.77.70.206:20190",
                "git_hash": "a2c58c94f89cbb410e66d8f85c236308d6fc64f0",
                "start_timestamp": 1736860078,
                "deploy_path": "/data2/tidb-deploy/tikv-20170/bin",
                "last_heartbeat": 1736910992480171416,
                "state_name": "Up"
            },
            "status": {
                "capacity": "500GiB",
                "available": "371.9GiB",
                "used_size": "5.286GiB",
                "leader_count": 11,
                "leader_weight": 1,
                "leader_score": 11,
                "leader_size": 11,
                "region_count": 38,
                "region_weight": 1,
                "region_score": 63.93783383873265,
                "region_size": 38,
                "slow_score": 1,
                "slow_trend": {
                "cause_value": 250057.83053691275,
                "cause_rate": 0,
                "result_value": 5,
                "result_rate": 0
                },
                "start_ts": "2025-01-14T13:07:58Z",
                "last_heartbeat_ts": "2025-01-15T03:16:32.480171416Z",
                "uptime": "14h8m34.480171416s"
            }
            }
        ]
        }
        )";

        router->UpdateRegion2Store(mock_data);
        router->UpdateTikv2Store(mock_tikv_data);
    }

    void TearDown() override {
        // 清理资源
    }

    LionRouter* router;
};

// 测试 TiDB 到 Store 的映射是否正确加载
TEST_F(LionRouterTest, TestTidb2StoreMapping) {
    std::string store = router->GetStoreForTidb("10.77.70.117");
    EXPECT_EQ(store, "10.77.70.205");
}

// 测试无效的 TiDB 名称
TEST_F(LionRouterTest, TestInvalidTidb) {
    std::string store = router->GetStoreForTidb("invalid_tidb");
    EXPECT_TRUE(store.empty());
}

// 测试 Region 到 Store 的映射是否正确加载
TEST_F(LionRouterTest, TestRegion2StoreMapping) {
    int store = router->GetStoreForRegion(44);
    EXPECT_EQ(store, 1);
}
// 测试无效的 Region 名称
TEST_F(LionRouterTest, TestInvalidRegion) {
    int store = router->GetStoreForRegion(-1);
    EXPECT_EQ(store, -1);
}

// 测试 TiKV IP 到 Store ID 的映射
TEST_F(LionRouterTest, TestTiKV2StoreID) {
    EXPECT_EQ(router->GetStoreIDForTiKV("10.77.70.207"), 7);
    EXPECT_EQ(router->GetStoreIDForTiKV("10.77.70.206"), 6);
    EXPECT_EQ(router->GetStoreIDForTiKV("10.77.70.205"), 1);
}

// 测试 Store ID 到 TiKV IP 的映射
TEST_F(LionRouterTest, TestStoreID2TiKV) {
    EXPECT_EQ(router->GetTiKVForStoreID(7), "10.77.70.207");
    EXPECT_EQ(router->GetTiKVForStoreID(6), "10.77.70.206");
    EXPECT_EQ(router->GetTiKVForStoreID(1), "10.77.70.205");
}

// 测试无效的 TiKV IP
TEST_F(LionRouterTest, TestInvalidTiKV) {
    EXPECT_EQ(router->GetStoreIDForTiKV("10.77.70.999"), -1);
}

// 测试无效的 Store ID
TEST_F(LionRouterTest, TestInvalidStoreID) {
    EXPECT_EQ(router->GetTiKVForStoreID(-1), "");
}

TEST_F(LionRouterTest, TestVirtualRegionMapping) {
    EXPECT_EQ(router->GetRegionPrimaryStoreId(0), 1);
    EXPECT_EQ(router->GetRegionSecondaryStoreId(0), std::unordered_set<int>({ 7}));
    EXPECT_EQ(router->GetAllStoreIds(), std::set<int>({1, 4, 5, 6, 7}));
}

// 测试 ParseYcsbKey 函数
TEST_F(LionRouterTest, TestParseYcsbKey) {
    std::string sql = R"(
        UPDATE benchbase.usertable
        SET FIELD1 = CASE
            WHEN YCSB_KEY = 10 THEN 211
            WHEN YCSB_KEY = 11 THEN 211
            WHEN YCSB_KEY = 12 THEN 211
            WHEN YCSB_KEY = 13 THEN 211
            WHEN YCSB_KEY = 14 THEN 2111
            WHEN YCSB_KEY = 10015 THEN 2211
            WHEN YCSB_KEY = 10016 THEN 211
            WHEN YCSB_KEY = 10017 THEN 212
            WHEN YCSB_KEY = 10018 THEN 213
            WHEN YCSB_KEY = 10019 THEN 215
            ELSE FIELD1
        END
        WHERE YCSB_KEY IN (10, 11, 12, 13, 14, 10015, 10016, 10017, 10018, 10019);
    )";

    std::vector<int> region_ids = router->ParseYcsbKey(sql);
    std::vector<int> expected_region_ids = {1, 0}; // 10/10000=0, 1015/10000=1

    EXPECT_EQ(region_ids, expected_region_ids);
}

// 测试 EvaluateHost 函数
TEST_F(LionRouterTest, TestEvaluateHost) {
    std::vector<int> region_ids = {0, 1}; // 测试 region_id 数组

    // 预期结果：
    // - store_id=1: 主副本数=1 (region_id=0), 从副本数=1 (region_id=1), 开销=-(1*10 + 1)=-11
    // - store_id=5: 主副本数=1 (region_id=1), 从副本数=1 (region_id=0), 开销=-(1*10 + 1)=-11
    // - store_id=7: 主副本数=0, 从副本数=2 (region_id=0 和 region_id=1), 开销=-(0*10 + 2)=-2
    // 最优的 store_id 是 1 或 5（开销最小）

    int hostgroupid = router->EvaluateHost(region_ids);
    EXPECT_TRUE(hostgroupid == 4);
}

TEST_F(LionRouterTest, TestTiupCMD) {
    router->InitTikv2Store("http://10.77.70.250:12379/pd/api/v1/stores");
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}