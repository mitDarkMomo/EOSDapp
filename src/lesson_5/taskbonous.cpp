#include <eosiolib/eosio.hpp>
#include <eosiolib/print.hpp>
#include <eosiolib/asset.hpp>
#include <eosiolib/action.hpp>
#include <eosiolib/symbol.hpp>
#include <eosiolib/singleton.hpp>
#include <eosiolib/print.hpp>
#include <eosiolib/transaction.hpp>
#include <eosiolib/crypto.h>
#include <eosiolib/dispatcher.hpp>
using namespace eosio;
using namespace std;

class [[eosio::contract]]  taskbonous : public eosio::contract {
    public:
        using contract::contract;

    //新增任务的action：
    [[eosio::action]]
    void addtask(string content, uint32_t level, uint64_t end, asset token) {
        require_auth(_self);
        task_index tasks(_self, _self.value);
        tasks.emplace(
            _self,
            [&](auto& t) {
                t.id = tasks.available_primary_key();
                t.level = level;
                t.content = content;
                t.end_time = end;
                t.reward = token;
            }
        );
    }

    //转账任务的action:
    [[eosio::action]]
    void transfer(name from, name to, asset quantity, string memo) {
        //memo里面附带附加信息:
        //\--taskid:1
        print("transfer from ", from);
        //使用 string find 函数，返回字母 or 字符串在字符串的位置，返回类型是 size_type
        string::size_type idx;
        idx = memo.find("taskid:");
        if(idx != string::npos) {   //string::npos 是一个特殊的值，标明查找没有匹配数据
            string id = memo.substr(idx + 7, memo.length());   //表示跳过"taskid:"
            print("id: ", id.c_str());  //char *c_str()：字符串的指针常量
            
            task_index tasks(_self, _self.value);
            //atoi 字符串转整型
            auto taskitem = tasks.find(atoi(id.c_str()));
            if(taskitem != tasks.end()) {
                asset pay = taskitem -> reward;
                //eosio_assert：断言，终止当前 action 的处理并回滚所有未完成的变化
                eosio_assert(quantity.amount > pay.amount, "transfer error");
                
                //taskuser 数据我们存储到 _self 合约，相关数据标识我们指定的是 from.value
                taskuser_index tasku(_self, from.value);
                auto taskUserItem = tasku.find(taskitem->id);
                if(taskUserItem == tasku.end()) {
                    tasku.emplace(
                        _self,
                        [&](auto& t) {
                            t.taskid = taskitem -> id;
                            t.is_end = 0;
                            t.start_time = current_time();
                            t.reward = asset(quantity.amount + pay.amount, symbol("EOS", 4));
                        }
                    );
                }
            }
        }
    }

    //发送奖励的action
    [[eosio::action]]
    void sendbonous(name to, string memo) {
        //memo里面附带附加信息:
        //\--taskid:1
        print("transfer to ", to);
        require_auth(to);
        print(" memo is: ", memo.c_str());
        //使用 string find 函数，返回字母 or 字符串在字符串的位置，返回类型是 size_type
        string::size_type idx;
        idx = memo.find("taskid:");
        print(" idx is: ", idx);
        if(idx != string::npos) {   //string::npos 是一个特殊的值，标明查找没有匹配数据
            string id = memo.substr(idx + 7, memo.length());   //表示跳过"taskid:"
            print(" id: ", id.c_str());  //char *c_str()：字符串的指针常量
            
            task_index tasks(_self, _self.value);
            //atoi 字符串转整型
            auto taskitem = tasks.find(atoi(id.c_str()));
            if(taskitem != tasks.end()) {
                //taskuser 数据我们存储到 _self 合约，相关数据标识我们指定的是 to.value
                taskuser_index tasku(_self, to.value);
                auto taskUserItem = tasku.find(taskitem->id);
                if(taskUserItem != tasku.end() && taskUserItem -> is_end != 1) {
                    //提交时间未超时则发放奖励
                    auto bonous = 0;
                    uint64_t submit_time = current_time();
                    if(submit_time <= taskitem -> end_time) {
                        bonous = (taskUserItem -> reward).amount;
                    }

                    tasku.modify(
                        taskUserItem,
                        _self,
                        [&](auto& t) {
                            t.is_end = 1;
                            t.end_time = submit_time;
                            t.reward = asset(bonous, symbol("EOS", 4));
                        }
                    );

                    //发放奖励
                    if(bonous > 0) {
                        print(" send bonous: ", bonous);

                        action(
                            permission_level{_self, name("active")},
                            name("eosio.token"),
                            name("transfer"),
                            std::make_tuple(
                                _self, 
                                to, 
                                asset(bonous, symbol("EOS",4)), 
                                std::string(" mission completed, send eos")
                            )
                        ).send();
                    }
                }

            }
        }
    }

    //清除taskuser数据
    [[eosio::action]]
    void erase() {
        taskuser_index tasku(_self, name("testaaa11111").value);
        for(auto itr = tasku.begin(); itr != tasku.end();) {
            itr = tasku.erase(itr);
        }
    }

    //任务相关的表
    struct [[eosio::table]] task {
        uint64_t id;
        string content;
        uint32_t level;
        uint64_t end_time;
        asset reward = asset(0, symbol("EOS", 4));
        
        uint64_t primary_key() const {
            return id;
        }
        
        EOSLIB_SERIALIZE(task,(id)(content)(level)(end_time)(reward))
    };

    typedef eosio::multi_index<name("task"), task> task_index;

    //任务认领相关的表：
    struct [[eosio::table]] taskuser {
        uint64_t taskid;
        uint32_t is_end;        //0：未结束；1：已结束
        uint64_t start_time;    //认领日期
        uint64_t end_time;      //提交日期
        asset reward = asset(0, symbol("EOS", 4));
        uint64_t primary_key() const {
            return taskid;
        }
        
        EOSLIB_SERIALIZE(taskuser, (taskid)(is_end)(start_time)(end_time)(reward))
    };

    typedef eosio::multi_index<name("taskuser"), taskuser> taskuser_index;
};

EOSIO_DISPATCH(taskbonous, (addtask)(transfer)(sendbonous)(erase))