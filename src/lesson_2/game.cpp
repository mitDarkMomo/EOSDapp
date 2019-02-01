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
class game : public contract {
  public:
      using contract::contract;

      [[eosio::action]]
      void play(name player, asset quantity) {
		require_auth(name("game"));

        print("game start");
		print("Hello, ", player);
        action(
            permission_level{name("eosio.token"), name("active")},
            name("eosio.token"), 
            name("transfer"),
            std::make_tuple(
                name("eosio.token"), 
                player, 
                asset(quantity.amount * 2, quantity.symbol),
                std::string("game send eos")) 
        ).send();
	}
};


#define EOSIO_DISPATCH_CUSTOM(TYPE, MEMBERS) \
extern "C" { \
   void apply( uint64_t receiver, uint64_t code, uint64_t action ) { \
   auto self = receiver; \
      if(( code == self&&action != name("transfer").value) ) { \
        switch( action ) { \
            EOSIO_DISPATCH_HELPER( TYPE, MEMBERS ) \
         } \
         /* does not allow destructor of this contract to run: eosio_exit(0); */ \
      } \
   } \
} \


EOSIO_DISPATCH_CUSTOM( game, (play))

