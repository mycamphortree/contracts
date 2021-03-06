/**
 *  @file
 *  @copyright defined in eos/LICENSE.txt
 */
#pragma once

#include <eosiolib/action.hpp>
#include <eosiolib/public_key.hpp>
#include <eosiolib/types.hpp>
#include <eosiolib/print.hpp>
#include <eosiolib/privileged.h>
#include <eosiolib/optional.hpp>
#include <eosiolib/producer_schedule.hpp>
#include <eosiolib/contract.hpp>

namespace eosiosystem {
   using eosio::permission_level;
   using eosio::public_key;

   typedef std::vector<char> bytes;

   struct permission_level_weight {
      permission_level  permission;
      weight_type       weight;

      // explicit serialization macro is not necessary, used here only to improve compilation time
      EOSLIB_SERIALIZE( permission_level_weight, (permission)(weight) )
   };

   struct key_weight {
      public_key   key;
      weight_type  weight;

      // explicit serialization macro is not necessary, used here only to improve compilation time
      EOSLIB_SERIALIZE( key_weight, (key)(weight) )
   };

   // ********** This code was modified by camphor  <begin>*********//
    struct wait_weight {
        uint32_t     wait_sec;
        weight_type  weight;

        // explicit serialization macro is not necessary, used here only to improve compilation time
        EOSLIB_SERIALIZE( wait_weight, (wait_sec)(weight) )
    };

   struct authority {
       uint32_t                               threshold = 0;
       std::vector<key_weight>                keys;
       std::vector<permission_level_weight>   accounts;
       std::vector<wait_weight>               waits;

      // explicit serialization macro is not necessary, used here only to improve compilation time
      EOSLIB_SERIALIZE( authority, (threshold)(keys)(accounts)(waits))
   };
    // ********** This code was modified by camphor  <end>*********//

   struct block_header {
      uint32_t                                  timestamp;
      account_name                              producer;
      uint16_t                                  confirmed = 0;
      block_id_type                             previous;
      checksum256                               transaction_mroot;
      checksum256                               action_mroot;
      uint32_t                                  schedule_version = 0;
      eosio::optional<eosio::producer_schedule> new_producers;

      // explicit serialization macro is not necessary, used here only to improve compilation time
      EOSLIB_SERIALIZE(block_header, (timestamp)(producer)(confirmed)(previous)(transaction_mroot)(action_mroot)
                                     (schedule_version)(new_producers))
   };


   /*
    * Method parameters commented out to prevent generation of code that parses input data.
    */
   class native : public eosio::contract {
      public:

         using eosio::contract::contract;

         /**
          *  Called after a new account is created. This code enforces resource-limits rules
          *  for new accounts as well as new account naming conventions.
          *
          *  1. accounts cannot contain '.' symbols which forces all acccounts to be 12
          *  characters long without '.' until a future account auction process is implemented
          *  which prevents name squatting.
          *
          *  2. new accounts must stake a minimal number of tokens (as set in system parameters)
          *     therefore, this method will execute an inline buyram from receiver for newacnt in
          *     an amount equal to the current new account creation fee.
          */
         void newaccount( account_name     creator,
                          account_name     newact
                          /*  no need to parse authorites
                          const authority& owner,
                          const authority& active*/ );

       // ********** This code was modified by camphor  <begin>*********//
         void updateauth( account_name     account,
                                 permission_name  permission,
                                 permission_name  parent,
                                 const authority& data) {
            ::eosio::print(::eosio::name{account}," ; ");
            ::eosio::print(::eosio::name{permission}," ; ");
            ::eosio::print(::eosio::name{parent}," ; ");
            ::eosio::print(data.threshold,"  ;;");
            require_recipient( account );

         }
       // ********** This code was modified by camphor  <end>*********//

         void deleteauth( /*account_name account, permission_name permission*/ ) {}

         void linkauth( /*account_name    account,
                               account_name    code,
                               action_name     type,
                               permission_name requirement*/ ) {}

         void unlinkauth( /*account_name account,
                                 account_name code,
                                 action_name  type*/ ) {}

         void canceldelay( /*permission_level canceling_auth, transaction_id_type trx_id*/ ) {}

         void onerror( /*const bytes&*/ ) {}

   };
}
