#include "ex.hpp"

#include <cmath>
#include <enulib/action.hpp>
#include <enulib/asset.hpp>
#include "enu.token.hpp"

using namespace enumivo;
using namespace std;

void ex::receivedenu(const currency::transfer &transfer) {
  if (transfer.to != _self) {
    return;
  }

  // get ENU balance
  double enu_balance = enumivo::token(N(enu.token)).
	   get_balance(N(enu.ftp.mm), enumivo::symbol_type(ENU_SYMBOL).name()).amount;
  enu_balance = enu_balance/10000;

  // get FTP balance
  double ftp_balance = enumivo::token(N(ftp.coin)).
	   get_balance(N(enu.ftp.mm), enumivo::symbol_type(FTP_SYMBOL).name()).amount;
  ftp_balance = ftp_balance/10000;

  // get total shares
  double shares = enumivo::token(N(shares.coin)).
	   get_supply(enumivo::symbol_type(ENUFTP_SYMBOL).name()).amount;
  shares = shares/10000;

  double received = transfer.quantity.amount;
  received = received/10000;

  double a = ftp_balance;
  double b = 2 * ftp_balance * enu_balance;
  double c = - ftp_balance * enu_balance * received;

  double eu = (sqrt((b*b)-(4*a*c)) - b)/(2*a);
  double ee = received - eu;

  double new_shares = shares * (ee/(eu+enu_balance));

  auto quantity = asset(10000*new_shares, ENUFTP_SYMBOL);

  action(permission_level{_self, N(active)}, N(shares.coin), N(issue),
         std::make_tuple(transfer.from, quantity,
                         std::string("Issue new ENUFTP shares")))
      .send();

  action(permission_level{_self, N(active)}, N(enu.token), N(transfer),
         std::make_tuple(_self, N(enu.ftp.mm), transfer.quantity,
                         std::string("Invest ENUFTP shares with ENU")))
      .send();
}

void ex::receivedftp(const currency::transfer &transfer) {
  if (transfer.to != _self) {
    return;
  }

  // get ENU balance
  double enu_balance = enumivo::token(N(enu.token)).
	   get_balance(N(enu.ftp.mm), enumivo::symbol_type(ENU_SYMBOL).name()).amount;
  enu_balance = enu_balance/10000;

  // get FTP balance
  double ftp_balance = enumivo::token(N(ftp.coin)).
	   get_balance(N(enu.ftp.mm), enumivo::symbol_type(FTP_SYMBOL).name()).amount;
  ftp_balance = ftp_balance/10000;

  // get total shares
  double shares = enumivo::token(N(shares.coin)).
	   get_supply(enumivo::symbol_type(ENUFTP_SYMBOL).name()).amount;
  shares = shares/10000;

  double received = transfer.quantity.amount;
  received = received/10000;

  double a = enu_balance;
  double b = 2 * enu_balance * ftp_balance;
  double c = - enu_balance * ftp_balance * received;

  double ue = (sqrt((b*b)-(4*a*c)) - b)/(2*a);
  double uu = received - ue;

  double new_shares = shares * (uu/(ue+ftp_balance));

  auto quantity = asset(10000*new_shares, ENUFTP_SYMBOL);

  action(permission_level{_self, N(active)}, N(shares.coin), N(issue),
         std::make_tuple(transfer.from, quantity,
                         std::string("Issue new ENUFTP shares")))
      .send();

  action(permission_level{_self, N(active)}, N(ftp.coin), N(transfer),
         std::make_tuple(_self, N(enu.ftp.mm), transfer.quantity,
                         std::string("Invest ENUFTP shares with ENU")))
      .send();
}

void ex::receivedshares(const currency::transfer &transfer) {
  if (transfer.to != _self) {
    return;
  }

  // get ENU balance
  double enu_balance = enumivo::token(N(enu.token)).
	   get_balance(N(enu.ftp.mm), enumivo::symbol_type(ENU_SYMBOL).name()).amount;
  enu_balance = enu_balance/10000;

  // get FTP balance
  double ftp_balance = enumivo::token(N(ftp.coin)).
	   get_balance(N(enu.ftp.mm), enumivo::symbol_type(FTP_SYMBOL).name()).amount;
  ftp_balance = ftp_balance/10000;

  // get total shares
  double shares = enumivo::token(N(shares.coin)).
	   get_supply(enumivo::symbol_type(ENUFTP_SYMBOL).name()).amount;
  shares = shares/10000;

  double received = transfer.quantity.amount;
  received = received/10000;

  double ftpportion = ftp_balance*(received/shares);
  auto ftp = asset(10000*ftpportion, FTP_SYMBOL);

  action(permission_level{N(enu.ftp.mm), N(active)}, N(ftp.coin), N(transfer),
         std::make_tuple(N(enu.ftp.mm), transfer.from, ftp,
                         std::string("Divest ENUFTP shares for FTP")))
      .send();

  double enuportion = enu_balance*(received/shares);
  auto enu = asset(10000*enuportion, ENU_SYMBOL);

  action(permission_level{N(enu.ftp.mm), N(active)}, N(enu.token), N(transfer),
         std::make_tuple(N(enu.ftp.mm), transfer.from, enu,
                         std::string("Divest ENUFTP shares for ENU")))
      .send();

  action(permission_level{_self, N(active)}, N(shares.coin), N(retire),
         std::make_tuple(transfer.quantity, std::string("Retire ENUFTP shares")))
      .send();
}

void ex::apply(account_name contract, action_name act) {

  if (contract == N(enu.token) && act == N(transfer)) {
    auto transfer = unpack_action_data<currency::transfer>();

    enumivo_assert(transfer.quantity.symbol == ENU_SYMBOL,
                 "Must send ENU");
    receivedenu(transfer);
    return;
  }

  if (contract == N(ftp.coin) && act == N(transfer)) {
    auto transfer = unpack_action_data<currency::transfer>();

    enumivo_assert(transfer.quantity.symbol == FTP_SYMBOL,
                 "Must send FTP");
    receivedftp(transfer);
    return;
  }

  if (contract == N(shares.coin) && act == N(transfer)) {
    auto transfer = unpack_action_data<currency::transfer>();

    enumivo_assert(transfer.quantity.symbol == ENUFTP_SYMBOL,
                 "Must send ENUFTP");
    receivedshares(transfer);
    return;
  }

  if (act == N(transfer)) {
    auto transfer = unpack_action_data<currency::transfer>();
    enumivo_assert(false, "Must send FTP or ENU or ENUFTP");
    return;
  }

  if (contract != _self) return;

}

extern "C" {
[[noreturn]] void apply(uint64_t receiver, uint64_t code, uint64_t action) {
  ex enuftpshares(receiver);
  enuftpshares.apply(code, action);
  enumivo_exit(0);
}
}
