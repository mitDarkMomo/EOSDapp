import Vue from 'vue'
//加载scatter
import ScatterJS from 'scatterjs-core';
import ScatterEOS from 'scatterjs-plugin-eosjs2';

import { Api, JsonRpc, RpcError } from 'eosjs';

//初始化对象（使用eosj 或者scattereth，或者scattertron）
ScatterJS.plugins(new ScatterEOS());

let scatter = null;
let eosapi;

const network = {
    blockchain: 'eos',
    host: '172.17.2.142',
    port: 8888,
    protocol: 'http',
    chainId: 'cf057bbfb72640471fd910bcb67639c22df9f92470936cddc1ade0e2f2e7dc4f'
};

const rpc = new JsonRpc('http://172.17.2.142:8888');

const requiredFields = { accounts: [network] };

//export命令定义了模块的对外接口
export default class Scatterutil {

    static async init() {
        //ES6 Promise
        return new Promise(async resolve => {
            ScatterJS.scatter.connect("kpigame").then(connected => {
                if (!connected) {
                    resolve("false");
                    return false;
                }
                scatter = ScatterJS.scatter;
                //
                eosapi = scatter.eos(network, Api, { rpc, beta3: true });
                // 必须清空对象防止泄露对象。
                window.scatter = null;
                resolve("true");
            })
        });
    }

    //获取授权用户
    static async userinfo() {

        return new Promise(async resolve => {
            scatter.getIdentity(requiredFields).then(() => {
                let account = { name: "", eos: 0 };
                if (scatter != null) {
                    if (scatter.identity != null) {
                        //获取EOS的account
                        const accounts = scatter.identity.accounts.find(x => x.blockchain === 'eos');
                        account.name = accounts.name;
                    }
                }
                resolve(account);

            }).catch(error => {
                resolve("");
                console.error(error);
            });
        });
    };

   
    //转账
    static async transfer(num, memo) {
        if (scatter == null) {
            alert("请安装scatter插件！");
        }
        
        var numStr = num + ".0000 EOS";
        return new Promise(async resolve => {
            //
            scatter.getIdentity(requiredFields).then(() => {
                const account = scatter.identity.accounts.find(x => x.blockchain === 'eos');
                const transactionOptions = { authorization: [`${account.name}@${account.authority}`] };

                eosapi.transact(
                    {
                        actions: [{
                            account: 'taskbonous',
                            name: 'transfer',
                            authorization: [{
                                actor: account.name,
                                permission: account.authority,
                            }],
                            data: {
                                from: account.name,
                                to: 'taskbonous',
                                quantity: numStr,
                                memo: memo
                            },
                        }]
                    }, 
                    {
                        blocksBehind: 3,
                        expireSeconds: 30,
                    }
                ).then(result => {
                    console.log(` Transaction ID: ${result.transaction_id}`);
                    resolve("true");
                }).catch(error => {
                    console.log(error);
                });

            }).catch(error => {
                console.error(error);
                resolve("false");
            });
        });
    };

    //获取奖励
    static async submitbonous(memo) {
        if (scatter == null) {
            alert("请安装scatter插件！");
        }

        return new Promise(async resolve => {
            scatter.getIdentity(requiredFields).then(() => {
                const account = scatter.identity.accounts.find(x => x.blockchain === 'eos');
                const transactionOptions = { authorization: [`${account.name}@${account.authority}`] };
                
                eosapi.transact(
                    {
                        actions: [{
                            account: 'taskbonous',
                            name: 'sendbonous',
                            authorization: [{
                                actor: account.name,
                                permission: account.authority
                            }],
                            data: {
                                to: account.name,
                                memo: memo
                            },
                        }]
                    }, 
                    {
                        blocksBehind: 3,
                        expireSeconds: 30
                    }
                ).then(result => {
                    console.log(` Transaction ID: ${result.transaction_id}`);
                    resolve("true");
                }).catch(error => {
                    console.log(error);
                });

            }).catch(error => {
                resolve("false");
            });
        });
    };

}
