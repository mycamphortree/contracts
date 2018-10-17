/**
 *  @file
 *  @copyright defined in eos/LICENSE.txt
 */

#include "uosio.union.hpp"
#include <eosiolib/crypto.h>


using namespace eosio;

namespace uosio{
    uosio_union::uosio_union(account_name self)
            :contract(self),
            _utuosstate(self,self),
            _utuosvoter(self,self),
            _uttouostr(self,self),
             _uosuttr(self,self),
             _uosutstate(self,self){}

    void uosio_union::modifymember(std::vector<account_name> & members) {
        require_auth(_self);
        eosio_assert(members.size() > 0  && members.size()< 8, "union-member must be greater than 0");

        auto sta = _utuosstate.find(_self);
        if(sta == _utuosstate.end()){
            _utuosstate.emplace(_self,[&](auto &a){
                a.owner = _self;
                a.members = members;
            });
        }else {
            _utuosstate.modify(sta , _self, [&](auto &a){
                a.members = members;
            });
        }
        for(auto itr = _utuosvoter.begin() ; itr != _utuosvoter.end() ;){
            itr = _utuosvoter.erase(itr);
        }
        for(auto itr = _uttouostr.begin() ; itr !=_uttouostr.end() ; ){
            itr = _uttouostr.erase(itr);
        }

        auto uotst = _uosutstate.find(_self);
        if(uotst == _uosutstate.end()) {
            _uosutstate.emplace(_self, [&](auto &a) {
                a.owner = _self;
                a.max_id = 0;
            });
        }
        sta = _utuosstate.find(_self);
        for(auto itr = _uosuttr.begin() ; itr != _uosuttr.end() ; ){
            auto obj = itr++;
            _uosuttr.modify(itr,0,[&](auto &a){
                a.uosutid = std::vector<std::string>(sta->members.size());
                a.trac = std::vector<std::string>(sta->members.size());
            });
            itr = obj;
        }



    }

    void uosio_union::utuosvotetr(account_name voter,std::string tr_id, account_name owner, int64_t amount) {
        require_auth(voter);
        eosio_assert(amount > 0 , "amount must big than 0");
        eosio_assert(tr_id.size() < 70 , "id size must less than 70");
        auto& sta = _utuosstate.get(_self);
        auto it = std::find(sta.members.begin(), sta.members.end(), voter);
        eosio_assert(it != sta.members.end(),"voter was not in union");
        std::string buffer(tr_id);
        buffer += std::to_string(owner) + std::to_string(amount);
        uint64_t tr_hash = caculate_hash_64(buffer.c_str() , buffer.size());
        auto ver = _utuosvoter.find(voter);
        if(ver == _utuosvoter.end()){
            _utuosvoter.emplace(_self,[&](auto &a){
                a.voter = voter;
                a.tr_hash = tr_hash;
                a.active_time = now();
                a.tr_id = tr_id;
            });
        }else{
            eosio_assert(now() - ver->active_time > 0 , "The time of re-voting must be greater than one block");
            auto & tr = _uttouostr.get(ver->tr_hash , "can find tr");
            if(tr.votes == 1){
                _uttouostr.erase(tr);
            }else{
                _uttouostr.modify(tr,0,[&](auto &a){
                    a.votes --;
                });
            }
            _utuosvoter.modify(ver,0,[&](auto &a){
                a.tr_hash = tr_hash;
                a.active_time = now();
                a.tr_id = tr_id;
            });
        }
        auto tr = _uttouostr.find(tr_hash);
        if(tr == _uttouostr.end()){
            _uttouostr.emplace(_self,[&](auto &a){
                a.tr_hash = tr_hash;
                a.tr_id = tr_id;
                a.owner = owner;
                a.amount = amount;
                a.votes = 1;
            });
        }else{
            _uttouostr.modify(tr,0,[&](auto &a){
                a.votes ++;
            });
        }
        tr = _uttouostr.find(tr_hash);
        //　这里是大于，所以６个节点　最少要５票
        if(tr->votes > sta.members.size() * 2 / 3 ){
//            print("打uos给",eosio::name{tr->owner}," ",tr->amount,";\n");
            action(permission_level{_self, N(active)}, N(eosio.token),
                   N(transfer), std::make_tuple(_self, tr->owner, asset(tr->amount,CORE_SYMBOL),
                                                std::string("from uosio.union")))
                    .send();

            _utuosstate.modify(sta,0,[&](auto &a){
                a.laster_id = tr->tr_id;
                a.laster_owner = tr->owner;
                a.amount = amount;
                a.tr_hash = tr->tr_hash;
            });
            for(auto itr = _utuosvoter.begin() ; itr != _utuosvoter.end() ;){
                itr = _utuosvoter.erase(itr);
            }
            for(auto itr = _uttouostr.begin() ; itr !=_uttouostr.end() ; ){
                itr = _uttouostr.erase(itr);
            }
        }

    }

    void uosio_union::transfer(account_name from, account_name to, asset quantity, std::string memo) {
        eosio_assert(  to == _self, "must to uosio.union");
        eosio_assert(  quantity.symbol == CORE_SYMBOL, "must use system coin");
        eosio_assert(  memo.size() < 80 , "The ut address size maximum is 80");
        eosio_assert(  quantity.amount >= 100 * 10000 , "minimum transfer 100 uos");
        eosio_assert(  quantity.amount <= 1000 * 10000 , "maxmum transfer 1000 uos");
        auto& sta = _utuosstate.get(_self);
        auto uot = _uosuttr.find(from);
        eosio_assert(uot == _uosuttr.end(),"user has unconfirmed transaction");
        int size = 0;
        for( auto itr = _uosuttr.begin(); itr != _uosuttr.end() ; itr++){
            size ++;
        }
        //　最大为５，为４时仍然可以插入一条数据
        eosio_assert(size < 5 , "The back buffer is full,The maximum is 5");
        auto &uotst = _uosutstate.get(_self);
        _uosuttr.emplace(_self,[&](auto & a){
            a.owner = from;
            a.amount = quantity.amount;
            a.id = uotst.max_id + 1;
            a.ut_address = memo;
            a.uosutid = std::vector<std::string>(sta.members.size());
            a.trac = std::vector<std::string>(sta.members.size());

        });
        _uosutstate.modify(uotst,0,[&](auto &a){
            a.max_id ++;
        });

    }

    void uosio_union::setuttr(account_name voter, account_name tr, std::string uttr) {
        require_auth(voter);
        eosio_assert(uttr != std::string() , "trac can't be empty");
        eosio_assert(uttr.size() < 2*1024,"Sign up to 2K bytes");
        auto& sta = _utuosstate.get(_self);
        auto it = std::find(sta.members.begin(), sta.members.end(), voter);
        eosio_assert(it != sta.members.end(),"voter was not in union");
        int index = it - sta.members.begin();
        auto &ustr =  _uosuttr.get(tr,"No information for this user");
        if(index > 0 ){
            eosio_assert(ustr.trac[index - 1] != std::string() , "ustr.trac Must follow the order");
        }
        _uosuttr.modify(ustr,0,[&](auto &a){
            if(index){
                a.trac[index - 1] = std::string();
            } else {
                a.trac[a.trac.size() - 1] = std::string();
            }
            a.trac[index] = uttr;
        });
    }

    void uosio_union::setousutid(account_name voter, account_name tr, std::string id) {
        require_auth(voter);
        auto& sta = _utuosstate.get(_self);
        auto it = std::find(sta.members.begin(), sta.members.end(), voter);
        eosio_assert(it != sta.members.end(),"voter was not in union");
        int index = it - sta.members.begin();
        auto ustr =  _uosuttr.find(tr);
        eosio_assert(ustr!=_uosuttr.end(),"No information for this user");
        _uosuttr.modify(ustr,0,[&](auto &a){
            a.uosutid[index] = id;
        });
        int size = 0;
        ustr =  _uosuttr.find(tr);
        for(auto itr = ustr->uosutid.begin();itr!= ustr->uosutid.end();itr++){
            if(*itr == id){
                size++;
            }
        }
        if(size > sta.members.size() * 2 / 3){
            _uosuttr.erase(ustr);
        }

    }

    uint64_t uosio_union::caculate_hash_64(const char *buf, int32_t size) {
        checksum160 calc_hash;
        ripemd160(buf, size , &calc_hash );
        uint64_t a = *(int64_t *)calc_hash.hash;
        uint64_t b = *(int64_t *)(calc_hash.hash + 8);
        uint32_t c = *(int32_t *)(calc_hash.hash + 16);
        return a^b^c;

    }


}/// namespace uosio


EOSIO_ABI( uosio::uosio_union,(modifymember)(utuosvotetr)(transfer)(setuttr)(setousutid))
