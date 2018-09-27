#include "eosio.system.hpp"
#include <eosiolib/eosio.hpp>

#include <eosio.token/eosio.token.hpp>

namespace eosiosystem {

// ********** This code was modified by camphor  <begin>*********//
    void system_contract::payfee( account_name payer , uint64_t net_usage , uint64_t virtule_net_limit , account_name act_account ,action_name act_name) {
        using namespace eosio;
        eosio_assert(int64_t(net_usage)>0,"net_usage overflow");
        print( "payer :  ", name{payer},"\n" );
        print( "net_usage : ", net_usage,"\n");
        print( "virtule_net_limit : ", virtule_net_limit,"\n");
        print( "act_account : ", name{act_account},"\n");
        print( "act_name : ", name{act_name},"\n");
//        INLINE_ACTION_SENDER(eosio::token, transfer)( N(eosio.token), {payer,N(active)},
//                                                      { payer, N(eosio.txfee), asset(int64_t(net_usage)), "transaction fee" } );

//        if( _gstate.total_activated_stake < min_activated_stake){
//            return;
//        }

    }
// ********** This code was modified by camphor  <end>*********//

} //namespace eosiosystem
