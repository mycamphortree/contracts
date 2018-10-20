/**
 *  @file
 *  @copyright defined in eos/LICENSE.txt
 */

#include "uosio.bvpay.hpp"
#include <eosiolib/transaction.hpp>


using namespace eosio;

namespace uosio{

    uosio_bvpay::uosio_bvpay(account_name self):
                  contract(self),
                  _bpvtpay(self,self),
                  _bvrealpay(self,self){}

    void uosio_bvpay::transfer(account_name from, account_name to, asset quantity, std::string memo) {
        eosio_assert(  quantity.symbol == CORE_SYMBOL, "must use system coin");
        if(from == _self){
            auto realpaydb = _bvrealpay.get(_self);
            eosio_assert(realpaydb.quantity >= quantity , "realpaydb.quantity >= quantity");
            _bvrealpay.modify(realpaydb,0,[&](auto &a){
                a.quantity -= quantity;
            });
            return;
        }
        auto bpvtp =  _bpvtpay.find(0);
        if(bpvtp == _bpvtpay.end()){
            _bpvtpay.emplace(_self,[&](auto &a){
                a.id = 0;
                a.start_line = now();
                a.laster_line = now();
                a.dead_line = now() + BP_VT_PAY_CYCLE;
                a.quantity = quantity;
                a.inherit = asset(0);
            });
            _bpvtpay.emplace(_self,[&](auto &a){
                a.id = 1;
                a.start_line = now() + BP_VT_PAY_CYCLE;
                a.laster_line = now() + BP_VT_PAY_CYCLE;
                a.dead_line = now() + BP_VT_PAY_CYCLE + BP_VT_PAY_CYCLE;
                a.quantity = asset(0);
                a.inherit = asset(0);
            });
        } else {
            eosio_assert(now() >= bpvtp->laster_line , " error : bpvtp->laster_line");
            auto bpvtp1 = _bpvtpay.find(1);
            if(now() < bpvtp->dead_line){
               _bpvtpay.modify(bpvtp1,0,[&](auto &a){
                   a.quantity += quantity;
               });
            } else if( (now() >= bpvtp1->start_line) && (now() < bpvtp1->dead_line) ){
                _bpvtpay.modify(bpvtp,0,[&](auto &a){
                    a.start_line += BP_VT_PAY_CYCLE;
                    a.laster_line +=  BP_VT_PAY_CYCLE;
                    a.dead_line += BP_VT_PAY_CYCLE;
                    a.inherit += a.quantity;
                    a.quantity = bpvtp1->quantity;
                });

                _bpvtpay.modify(bpvtp1,0,[&](auto &a){
                    a.start_line += BP_VT_PAY_CYCLE;
                    a.laster_line +=  BP_VT_PAY_CYCLE;
                    a.dead_line += BP_VT_PAY_CYCLE;
                    a.quantity = quantity;

                });

            } else if(now() >= bpvtp1->dead_line){
                _bpvtpay.modify(bpvtp,0,[&](auto &a){
                    a.start_line = now();
                    a.laster_line = now();
                    a.dead_line = now() + BP_VT_PAY_CYCLE;
                    a.inherit += bpvtp1->quantity;
                    a.inherit += a.quantity;
                    a.quantity = quantity;
                });
                _bpvtpay.modify(bpvtp1,0,[&](auto &a){
                    a.start_line = now() + BP_VT_PAY_CYCLE;
                    a.laster_line = now() + BP_VT_PAY_CYCLE;
                    a.dead_line = now() + BP_VT_PAY_CYCLE + BP_VT_PAY_CYCLE;
                    a.quantity = asset(0);
                });
            }
        }
    }

    asset uosio_bvpay::get_bpvtpay(account_name call) {
        auto bpvtp = _bpvtpay.find(0);
        auto bpvtp1 = _bpvtpay.find(1);
        eosio_assert(bpvtp != _bpvtpay.end() , "no _bpvtpay object");
        int64_t real_pay = 0;
        eosio_assert(now() >  bpvtp->laster_line , "get_bpvtpay time error");
        if(now() < bpvtp->dead_line){
            double pay = (now() - bpvtp->laster_line ) *  bpvtp->quantity.amount * 1.0 / BP_VT_PAY_CYCLE;
            real_pay = int64_t(pay);
            real_pay += bpvtp->inherit.amount;
            _bpvtpay.modify(bpvtp,0,[&](auto &a){
                a.quantity.amount -= real_pay;
                a.inherit = asset(0);
                a.laster_line = now();
            });
        } else if( (now() >= bpvtp->dead_line) && (now() < bpvtp1->dead_line) ){
            real_pay = bpvtp->quantity.amount;
            real_pay += bpvtp->inherit.amount;
            eosio_assert(now() >  bpvtp1->laster_line , "get_bpvtpay time error");
            double pay = (now() - bpvtp1->laster_line ) *  bpvtp1->quantity.amount * 1.0 / BP_VT_PAY_CYCLE;
            int64_t next_pay = int64_t(pay);
            real_pay += next_pay;

            _bpvtpay.modify(bpvtp,0,[&](auto &a){
                a.start_line += BP_VT_PAY_CYCLE;
                a.laster_line = now();
                a.dead_line += BP_VT_PAY_CYCLE;
                a.quantity = bpvtp1->quantity;
                a.quantity.amount -= next_pay;
                a.inherit = asset(0);
            });

            _bpvtpay.modify(bpvtp1,0,[&](auto &a){
                a.start_line += BP_VT_PAY_CYCLE;
                a.laster_line +=   BP_VT_PAY_CYCLE;
                a.dead_line += BP_VT_PAY_CYCLE;
                a.quantity =  asset(0);
            });


        }  else if(now() >= bpvtp1->dead_line){
            real_pay = bpvtp->quantity.amount;
            real_pay += bpvtp->inherit.amount;
            real_pay += bpvtp1->quantity.amount;
            _bpvtpay.erase(bpvtp);
            _bpvtpay.erase(bpvtp1);
        }
        bpvtp = _bpvtpay.find(0);
        bpvtp1 = _bpvtpay.find(1);
        if( (bpvtp->quantity.amount == 0 ) && (bpvtp1->quantity.amount == 0) ){
            _bpvtpay.erase(bpvtp);
            _bpvtpay.erase(bpvtp1);
        }
        auto realpaydb = _bvrealpay.find(_self);
        if(realpaydb == _bvrealpay.end()){
            _bvrealpay.emplace(_self,[&](auto &a){
                a.owner = _self;
                a.quantity = asset(real_pay , CORE_SYMBOL);
            });
        } else {
            _bvrealpay.modify(realpaydb,0,[&](auto &a){
                a.quantity += asset(real_pay , CORE_SYMBOL);
            });
        }
        realpaydb = _bvrealpay.find(_self);
        return realpaydb->quantity;
    }





    void uosio_bvpay::hello(account_name owner) {
        get_bpvtpay(_self);
    }

    void uosio_bvpay::dida() {
        eosio::transaction out;
        out.actions.emplace_back( permission_level{ _self, N(active) }, _self, N(hello),_self);
        out.delay_sec = 60;
        cancel_deferred( _self ); // TODO: Remove this line when replacing deferred trxs is fixed
        out.send( _self, _self, true );
    }

}/// namespace uosio


//EOSIO_ABI( uosio::uosio_bvpay,(hello))


#define UOSIO_BVPAY_ABI( TYPE, MEMBERS ) \
extern "C" { \
   void apply( uint64_t receiver, uint64_t code, uint64_t action ) { \
      auto self = receiver; \
      if( action == N(onerror)) { \
         /* onerror is only valid if it is for the "eosio" code account and authorized by "eosio"'s "active permission */ \
         eosio_assert(code == N(eosio), "onerror action's are only valid from the \"eosio\" system account"); \
      } \
      if(action == N(transfer) && code == N(eosio.token)) { \
         TYPE thiscontract( self ); \
         eosio::execute_action( &thiscontract, &uosio::uosio_bvpay::transfer); \
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


UOSIO_BVPAY_ABI( uosio::uosio_bvpay , (hello)(transfer) )