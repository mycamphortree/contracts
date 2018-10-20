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

    static constexpr time BP_VT_PAY_CYCLE = 30;


class uosio_bvpay : public contract {

public:
    uosio_bvpay(account_name self);
    void transfer(account_name from, account_name to, asset quantity, std::string memo);
    asset get_bpvtpay(account_name call);



    void hello(account_name owner);
    void dida();


    ///@abi table bpvtpay i64
    struct bp_and_vt_pay{
        uint32_t                 id;   //　id -- 0  : now   ;   id -- 1 : next
        time                     start_line;
        time                     laster_line;
        time                     dead_line;
        asset                    inherit;
        asset                    quantity;
        uint32_t primary_key()const { return id; }
        EOSLIB_SERIALIZE( bp_and_vt_pay, (id)(start_line)(laster_line)(dead_line)(inherit)(quantity))
    };
    typedef eosio::multi_index<N(bpvtpay) , bp_and_vt_pay> bpvtpay;


    ///@abi table bvrealpay i64
    struct bv_real_pay{
        account_name             owner;   //　id -- 0  : now   ;   id -- 1 : next
        asset                    quantity;
        uint64_t primary_key()const { return owner; }
        EOSLIB_SERIALIZE( bv_real_pay, (owner)(quantity))
    };
    typedef eosio::multi_index<N(bvrealpay) , bv_real_pay> bvrealpay;


private:
    bpvtpay _bpvtpay;
    bvrealpay _bvrealpay;
};



} /// namespace uosio
