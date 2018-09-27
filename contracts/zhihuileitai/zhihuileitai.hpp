/**
 *  @file
 *  @copyright defined in eos/LICENSE.txt
 */
#pragma once

#include <eosiolib/asset.hpp>
#include <eosiolib/eosio.hpp>
#include <eosiolib/symbol.hpp>
#include <eosiolib/types.h>
#include <vector>
#include <string>

using namespace eosio;

namespace eosiosystem {
   class system_contract;
}

namespace zhihuileitai {

   using std::string;

   class dalei : public contract {
      public:
         dalei( account_name self );
         // 给出题目以及答案hash
         void givetopic(account_name owner, checksum160 ths, checksum160 ash , int64_t ple, time conti, double oa = 1.11,
                 double ob = 1.11, double oc = 1.11 , double od = 1.11);
         // 给出题干正文
         void givetext(account_name owner,string text);
         // 答题者答题
         void answerqust(account_name owner, account_name topic_owner,int8_t choice , int64_t stack);
         // 出题者给出答案与解释正文
         void giveanswer(account_name owner, string answer);
         // 瓜分奖励
         void distribut();
         // 奖励给答题者
         void distribution_answer(account_name topic_owner);
         // 擦除答题者容器
         void erase_answer_topic_table(account_name topic_owner);
         // 系统合约的转账
         void transfer(account_name from, account_name to,asset quantity,std::string memo);
         // 刷新状态机
         int refresh_status();
         // 测试
         void hi(string memo);

      private:
       ///@abi table tpcacnts i64
         struct topic_account {
            account_name owner; //出题人
            checksum160  topic_hash; //问题hash
            checksum160  answer_hash; //答案hash
            int64_t      pledge; //质押资金
            int8_t       state; //0 没开始答题   --- 1 正在答题  --- 2 答题结束，用户适应期  ---- 3 此题结束
            bool         reject; //题目被审核组驳回
            time         continuous; //答题持续时间
            time         start_time; //开始时间
            time         dead_time; //结束时间
            int64_t       sum_bet;  //总投注
            int8_t        key; //答案
            std::vector<double> odds; //赔率
            std::vector<int64_t>  can_bet; // 剩余可投注
            string       text;
            string       answer;
            uint64_t     primary_key()const { return owner; }
            double       by_pledge()const    { return pledge;}
//            double       by_dding()const  {return doing;}
            EOSLIB_SERIALIZE( topic_account, (owner)(topic_hash)(answer_hash)(pledge)(state)(reject)(continuous)(start_time)(dead_time)(sum_bet)(key)(odds)(can_bet)(text)(answer))
         };

         typedef eosio::multi_index<N(tpcacnts) , topic_account ,
                                     indexed_by<N(antpledge), const_mem_fun<topic_account, double, &topic_account::by_pledge> >
                                    > topicaccounts;


       ///@abi table state i64
       struct state {
           account_name         owner;
           time                 start_time; //开始时间
           uint32_t             index;     //当前状态
           std::vector<time>    during_time; //每个阶段的持续时间
           uint64_t primary_key()const { return owner; }
           EOSLIB_SERIALIZE( state, (owner)(start_time)(index)(during_time))
       };
       typedef eosio::multi_index<N(state) , state> daleistate;
//
//
       ///@abi table accounts i64
       struct account {
           account_name owner;
           int64_t card_balance;
           int64_t withdraw;
           uint64_t primary_key()const { return owner; }
           double   by_balance()const    { return card_balance ? -card_balance : card_balance;  }
           EOSLIB_SERIALIZE( account, (owner)(card_balance)(withdraw))
       };
       typedef eosio::multi_index<N(accounts) , account ,
               indexed_by<N(bybalance), const_mem_fun<account, double, &account::by_balance> >
       > accounts;
//
//
       ///@abi table answerers i64
       struct answerer {
           account_name owner;  //答题人
           uint64_t balance;    //投注
           int8_t   key;        //答案
           account_name review; //投票对象
           bool     arbitration; //申请仲裁
           uint64_t primary_key()const { return owner; }
           double   by_key()const    { return key;  }
           //EOSLIB_SERIALIZE( account, (owner)(balance))
       };
       typedef eosio::multi_index<N(answerers) , answerer ,
               indexed_by<N(key), const_mem_fun<answerer, double, &answerer::by_key> >
       > answerers;
//
//       ///@abi table reviewers i64
//       struct reviewer {
//           account_name owner;  //评审员
//           uint64_t    votes;    //得票数
//           uint64_t primary_key()const { return owner; }
//           double   by_votes()const    { return votes;  }
//           //EOSLIB_SERIALIZE( account, (owner)(balance))
//       };

       topicaccounts _topicaccounts;
       accounts  _accounts;
       daleistate _daleistate;

//
//         typedef eosio::multi_index<N(accounts) , topic_account ,indexed_by<N(accountsbal), const_mem_fun<topic_account, double, &topic_account::by_balance> >
//                                    > accounts;


   };



} /// namespace xiaomiantuan
