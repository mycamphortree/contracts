/**
 *  @file
 *  @copyright defined in eos/LICENSE.txt
 */
#pragma once

#include <eosiolib/asset.hpp>
#include <eosiolib/eosio.hpp>
#include <eosiolib/symbol.hpp>


#include <string>
#include <vector>

using namespace eosio;


namespace uosio {


class uosio_union : public contract {

private:
    uint64_t caculate_hash_64(const char *buf , int32_t size);

    ///@abi table utuostr i64
    struct ut_to_uos_tr {
        uint64_t        tr_hash;
        std::string     tr_id;
        account_name    owner;
        int64_t         amount;
        int32_t         votes;
        uint64_t primary_key()const { return tr_hash; }
        EOSLIB_SERIALIZE( ut_to_uos_tr, (tr_hash)(tr_id)(owner)(amount)(votes))
    };
    typedef eosio::multi_index<N(utuostr) , ut_to_uos_tr> uttouostr;

    ///@abi table utuosvoter i64
    struct ut_to_uos_voter{
        account_name  voter;
        uint64_t      tr_hash;
        time          active_time;
        std::string   tr_id;
        uint64_t primary_key()const { return voter; }
        EOSLIB_SERIALIZE( ut_to_uos_voter, (voter)(tr_hash)(active_time)(tr_id))
    };
    typedef eosio::multi_index<N(utuosvoter) , ut_to_uos_voter> utuosvoter;

     ///@abi table utuosstate i64
    struct ut_to_uos_state {
        account_name    owner;
        std::string     laster_id;
        account_name    laster_owner;
        int64_t         amount;
        uint64_t        tr_hash;
        std::vector<account_name> members;
        uint64_t primary_key()const { return owner; }
        EOSLIB_SERIALIZE( ut_to_uos_state, (owner)(laster_id)(laster_owner)(amount)(tr_hash)(members))
    };
    typedef eosio::multi_index<N(utuosstate) , ut_to_uos_state> uttouosstate;


    ///@abi table uosuttr i64
    struct uos_to_ut_tr{
        account_name             owner;
        uint64_t                 id;
        int64_t                  amount;
        std::string              ut_address;
        std::vector<std::string> uosutid;
        std::vector<std::string> trac;
        uint64_t primary_key()const { return owner; }
        EOSLIB_SERIALIZE( uos_to_ut_tr, (owner)(id)(amount)(ut_address)(uosutid)(trac))
    };
    typedef eosio::multi_index<N(uosuttr) , uos_to_ut_tr> uosuttr;

    ///@abi table uosutstate i64
    struct uos_to_ut_state{
        account_name             owner;
        uint64_t                 max_id;
        uint64_t primary_key()const { return owner; }
        EOSLIB_SERIALIZE( uos_to_ut_state, (owner)(max_id))
    };
    typedef eosio::multi_index<N(uosutstate) , uos_to_ut_state> uosutstate;

    uttouosstate _utuosstate;
    utuosvoter   _utuosvoter;
    uttouostr    _uttouostr;
    uosuttr      _uosuttr;
    uosutstate   _uosutstate;

public:
    uosio_union(account_name self);
    void modifymember(std::vector<account_name > &members);
    void utuosvotetr(account_name voter,std::string tr_id , account_name owner , int64_t amount);
    void transfer( account_name from, account_name to,asset quantity,std::string memo);
    void setuttr(account_name voter, account_name tr , std::string sig);
    void setousutid(account_name voter,account_name tr , std::string id);



};


} /// namespace uosio
