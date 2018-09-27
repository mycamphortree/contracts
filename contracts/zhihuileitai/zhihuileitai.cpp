/**
 *  @file
 *  @copyright defined in eos/LICENSE.txt
 */

#include "zhihuileitai.hpp"
#include <eosiolib/crypto.h>
#include <eosiolib/system.h>



//using namespace eosio;

#define MT_SYMBOL S(4,MT)
#define CORE_SYMBOL_TO_MT_SYMBOL 1
#define SUM_TOPIC 12  //题目数量
#define ADAPTATION_TIME 30 //答题之后答题者的适应时间
#define WAIT_TIME 30 //等待下一道题的出题世间


namespace zhihuileitai {
  dalei::dalei(account_name self):contract(self),_topicaccounts(self,self),_accounts(self,self),_daleistate(self,self){
        auto code = _daleistate.find(_self);
        if(code == _daleistate.end()){
            _daleistate.emplace(_self,[&](auto &a){
                a.owner = _self;
                a.start_time = now();
                a.index = 0;
                a.during_time.push_back(1*60*60); // 买卡，出题
                a.during_time.push_back(2*60); // 发布题目，答题
                a.during_time.push_back(2*60); //发布答案，投票撤回
                a.during_time.push_back(2*60); //投票撤回
                a.during_time.push_back(2*60); //审核与发奖
                a.during_time.push_back(2*60); //投票审核团

            });
        }
  }

  void dalei::hi(std::string memo) {
      print("嗨，我的红色蒲公英");
      refresh_status();
  }

  void dalei::givetopic(account_name owner, checksum160 ths, checksum160 ash, int64_t ple, time conti, double oa,
                        double ob, double oc, double od) {
        require_auth(owner);
        eosio_assert(ple > 10000 , "押金要大于1eos");
        eosio_assert(oa > 1.1 && oa < 100.1, "a赔率不在区间");
        eosio_assert(ob > 1.1 && ob < 100.1, "b赔率不在区间");
        eosio_assert(oc > 1.1 && oc < 100.1, "c赔率不在区间");
        eosio_assert(od > 1.1 && od < 100.1, "d赔率不在区间");
        eosio_assert(conti >= 30 && conti <= 120 , "答题时间必须设置在：30s到120s" );
        auto tp =  _topicaccounts.find(owner);
        eosio_assert(tp == _topicaccounts.end(), "题目hash一旦上传，不可更改");
//        eosio_assert(conti >= MIN_ANSWER_SECOND , "答题时间至少30s");
        auto ant = _accounts.find(owner);
        eosio_assert(ant != _accounts.end(),"没有通关卡");
        eosio_assert(ant->card_balance >= ple , "通关卡余额不足");
        int index = refresh_status();
        eosio_assert(index == 0 ,"此时不是充值和出题时间");
        auto tpbyple = _topicaccounts.get_index<N(antpledge)>();
        int size = 0;
        auto itr = tpbyple.cbegin();
        for( ; itr != tpbyple.cend() ; itr++){
            size ++;
        }
        if(SUM_TOPIC == size){
             itr--;
             eosio_assert(itr->pledge < ple , "题目已满，需要提高押金才能入题");
             _topicaccounts.erase(*itr);
        }
        _accounts.modify(ant,0,[&](auto &a){
           a.card_balance -=  ple;
        });
        if(tp == _topicaccounts.end()){
             _topicaccounts.emplace(_self,[&](auto &a){
                 a.doing = 0;
                 a.owner = owner;
                 a.topic_hash = ths;
                 a.answer_hash = ash;
                 a.pledge = ple;
                 a.continuous = conti;
                 a.odds.push_back(oa);
                 a.odds.push_back(ob);
                 a.odds.push_back(oc);
                 a.odds.push_back(od);
                 a.can_bet.push_back(int64_t(ple/(oa-1)));
                 a.can_bet.push_back(int64_t(ple/(ob-1)));
                 a.can_bet.push_back(int64_t(ple/(oc-1)));
                 a.can_bet.push_back(int64_t(ple/(od-1)));
             });
        }
  }

  void dalei::givetext(account_name owner, std::string text) {
      require_auth(owner);
      eosio_assert(refresh_status() == 1, "不是给出题干的时间");
      auto tp =  _topicaccounts.find(owner);
      eosio_assert(tp != _topicaccounts.end() , "can not find topic");
      eosio_assert(text.size()<=1000 , "size of text must litter than 1000");
      char buf[1000];
      strcpy(buf, text.c_str());
      checksum160 calc_hash;
      ripemd160(buf, text.size() , &calc_hash );
      eosio_assert(calc_hash == tp->topic_hash , "提供hash与题目哈希不一致");
      for(auto itr = _topicaccounts.begin() ; itr != _topicaccounts.end() ; itr++){
          if(itr->state == 1){

          }
          if((itr->state == 1) || (itr->state == 2) ){
              eosio_assert(false,"当前尚有题目未结束");
          }
      }

      _topicaccounts.modify(tp,0,[&](auto &a){
          a.text = text;
          a.state = 1;
          a.start_time = now();
          a.dead_time = now() + a.continuous;
      });

  }

  void dalei::giveanswer(account_name owner, std::string answer) {
      auto tp =  _topicaccounts.find(owner);
      eosio_assert(tp != _topicaccounts.end(), "can not find topic");
      eosio_assert(answer.size()<=1000 , "size of text must litter than 1000");
      char buf[1000];
      strcpy(buf, answer.c_str());
      checksum160 calc_hash;
      ripemd160(buf, answer.size() , &calc_hash );
      eosio_assert(calc_hash == tp->answer_hash , "提供hash与答案哈希不一致");
      int8_t key = answer.c_str()[0] - 48;
      _topicaccounts.modify(tp,0,[&](auto &a){
          a.answer = answer;
          a.key = key;
          a.reject = (key>=1&&key<=4) ? false : true;
      });
  }

  void dalei::answerqust(account_name owner, account_name topic_owner, int8_t choice, int64_t stack) {
      eosio_assert(stack >= 90 , "压注最小90");
      if(!((choice >= 1) && (choice <= 4) )){
          eosio_assert(false , "提供的选项无效！");
      }
      auto tp =  _topicaccounts.find(topic_owner);
      eosio_assert(tp != _topicaccounts.end(), "can not find topic");
      if(now()>tp->dead_time){
          _topicaccounts.modify(tp,0,[&](auto &a){
             a.doing = false;
          });
      }
      eosio_assert(tp->doing , "此题现在不能作答");
      auto _answerers = answerers(_self,topic_owner);
      auto anser = _answerers.find(owner);
      eosio_assert(anser == _answerers.end() , "买定离手!!!");
      eosio_assert(stack <= tp->can_bet[choice-1] , "提供的押金太高");
      _topicaccounts.modify(tp,0,[&](auto &a){
          a.can_bet[choice-1] -= stack;
          a.sum_bet += stack;
          if(a.can_bet[choice-1] < 90){
              a.can_bet[choice-1] = 0;
          }
          int64_t can_sum = 0 ;
          for(auto it : tp->can_bet){
              can_sum += it;
          }
          if(0 == can_sum){
              a.doing = false;
          }
      });
      _answerers.emplace(_self,[&](auto &a){
          a.owner = owner;
          a.key = choice;
          a.balance = stack;
      });
  }

  // 给答题者发币
  void dalei::distribution_answer(account_name topic_owner) {
      auto _answerers = answerers(_self,topic_owner);
      auto topic = _topicaccounts.get(topic_owner);
      //相公
      if(topic.reject){
          int64_t re_answer = topic.pledge * 30 / 100 ;
          if(topic.sum_bet <=0 ){
              return;
          }
          for(auto itr = _answerers.begin() ; itr != _answerers.end() ; itr++){
              print("相公，转帐给答题人:",name{itr->owner},"  ",itr->balance + (itr->balance * re_answer / topic.sum_bet));
          }
      } else {
          for(auto itr = _answerers.begin() ; itr != _answerers.end() ; itr++){
              //TODO 假设正确答案为“a”
              if(itr->key != topic.key){
                  auto acnt = _accounts.find(topic_owner);
                  if(acnt == _accounts.end()){
                      _accounts.emplace(_self,[&]( auto& a ){
                          a.owner = topic_owner;
                          a.withdraw =  itr->balance;
                      });
                  } else {
                      _accounts.modify( acnt, 0, [&]( auto& a ) {
                          a.withdraw +=  itr->balance;
                      });
                  }
              } else {
                  int64_t reward = int64_t(itr->balance * topic.odds[0]);
                  auto acnt = _accounts.find(topic_owner);
                  if(acnt == _accounts.end()){
                      _accounts.emplace(_self,[&]( auto& a ){
                          a.owner = topic_owner;
                          a.withdraw =  itr->balance + topic.pledge - reward;
                      });
                  } else {
                      _accounts.modify( acnt, 0, [&]( auto& a ) {
                          a.withdraw +=   itr->balance + topic.pledge - reward;
                      });
                  }
                  print("转账给获奖者：" , name{itr->owner} , "  " , reward);
              }
          }

      }


  }

  void dalei::erase_answer_topic_table(account_name topic_owner) {
      auto _answerers = answerers(_self,topic_owner);
      for(auto itr = _answerers.begin(); itr != _answerers.end(); ){
          itr = _answerers.erase(itr);
      }
  }

    void dalei::transfer( account_name from, account_name to,asset quantity, std::string memo)
    {
        eosio_assert( from != to, "transfer : cannot transfer to self" );
        eosio_assert(quantity.symbol == CORE_SYMBOL , "必须系统币转账");
        eosio_assert(quantity.amount >0 , "转账金额必须大于0");
        auto index = refresh_status();
//        if(memo == std::string()){
//
//        }
        if(memo == std::string()){
            eosio_assert(index == 0 ,"要等下一轮才可以冲卡");
            eosio_assert(quantity.amount >= 100 , "转账金额最小为0.01个EOS");
            auto acnt = _accounts.find(from);
            if(acnt == _accounts.end()){
                _accounts.emplace(_self,[&](auto &a){
                    a.owner = from;
                    a.card_balance = quantity.amount;
                });
            } else {
                _accounts.modify( acnt, 0, [&]( auto& a ) {
                    a.card_balance += quantity.amount;
                });
            }
        }
    }

    int dalei::refresh_status() {
        auto sta = _daleistate.get(_self);
        uint64_t sum = 0;
        for(auto i = 0; (i<=sta.index) && (i<sta.during_time.size()); i++){
            sum += sta.during_time[i];
        }
        int old_index = sta.index;
        if(now() >= sum+sta.start_time){
            _daleistate.modify(sta , 0 , [&](auto a){
                a.index ++;
                if(a.index > a.during_time.size()){
                    a.index = a.during_time.size();
                }
            });
        }
        // 出题阶段到答题阶段的转变
        // 1，计算答题阶段的时间
        // 2，编好编号确定每道题的时间顺序？？？
        if(old_index == 0 && sta.index == 1){
            time topic_sum_time = 0;
            for(auto itr = _topicaccounts.begin() ; itr!=_topicaccounts.end() ; itr++){
                topic_sum_time = topic_sum_time + itr->continuous + ADAPTATION_TIME + WAIT_TIME;
            }
            auto code_state = _daleistate.get(_self);
            _daleistate.modify(code_state,0,[&](auto a){
                a.during_time[1] = topic_sum_time;
            });

        }
        return sta.index;
    }


  void dalei::distribut() {
      int64_t code_reward = 0;
      for( auto itr = _topicaccounts.begin() ; itr != _topicaccounts.end() ;){
          //相公题  60%自己  30%答题者  10%审核团
          if(itr->reject){
                auto acnt = _accounts.find(itr->owner);
                if(acnt == _accounts.end()){
                    _accounts.emplace(_self,[&]( auto& a ){
                        a.owner = itr->owner;
                        a.withdraw = itr->pledge * 60 / 100 ;
                    });
                } else {
                    _accounts.modify( acnt, 0, [&]( auto& a ) {
                        a.withdraw = a.withdraw + (itr->pledge * 60 / 100);
                    });
                }
                code_reward = code_reward + itr->pledge - (itr->pledge  * 30 / 100) - (itr->pledge * 60 / 100);
          }
          distribution_answer(itr->owner);
          erase_answer_topic_table(itr->owner);
          itr = _topicaccounts.erase(itr);
      }
  }




} /// namespace zhihuileitai


#define ZHIHUILEILAI_EOSIO_ABI( TYPE, MEMBERS ) \
extern "C" { \
   void apply( uint64_t receiver, uint64_t code, uint64_t action ) { \
      auto self = receiver; \
      if( action == N(onerror)) { \
         /* onerror is only valid if it is for the "eosio" code account and authorized by "eosio"'s "active permission */ \
         eosio_assert(code == N(eosio), "onerror action's are only valid from the \"eosio\" system account"); \
      } \
      if(action == N(transfer) && code == N(eosio.token)) { \
         TYPE thiscontract( self ); \
         eosio::execute_action( &thiscontract, &zhihuileitai::dalei::transfer); \
         return; \
      } \
      if(code == self || action == N(onerror) ) { \
         TYPE thiscontract( self ); \
         switch( action ) { \
            EOSIO_API( TYPE, MEMBERS ) \
         } \
         /* does not allow destructor of thiscontract to run: eosio_exit(0); */ \
      } \
   } \
} \


//EOSIO_ABI(zhihuileitai::dalei, (hi)(givetopic)(giveanswer)(givetext)(answerqust)(distribut))

ZHIHUILEILAI_EOSIO_ABI(zhihuileitai::dalei, (hi)(givetopic)(giveanswer)(givetext)(answerqust)(distribut)(transfer))

