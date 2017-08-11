#include <algorithm>
#include <iterator>
#include <random>

#include "ClientProxyMembershipIDFactory.hpp"

namespace apache {
namespace geode {
namespace client {

ClientProxyMembershipIDFactory::ClientProxyMembershipIDFactory(
    std::string dsName)
    : dsName(dsName) {
  static const auto alphabet =
      "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_";
  static const auto numChars = (sizeof(alphabet) / sizeof(char)) - 2;

  std::random_device rd;
  std::default_random_engine rng(rd());
  std::uniform_int_distribution<> dist(0, numChars);

  randString.reserve(7 + 10 + 15);
  randString.append("Native_");
  std::generate_n(std::back_inserter(randString), 10,
                  [&]() { return alphabet[dist(rng)]; });

  auto pid = ACE_OS::getpid();
  randString.append(std::to_string(pid));

  LOGINFO("Using %s as random data for ClientProxyMembershipID",
          randString.c_str());
}

std::unique_ptr<ClientProxyMembershipID> ClientProxyMembershipIDFactory::create(
    const char* hostname, uint32_t hostAddr, uint32_t hostPort,
    const char* durableClientId, const uint32_t durableClntTimeOut) {
  return std::unique_ptr<ClientProxyMembershipID>(new ClientProxyMembershipID(
      dsName, randString, hostname, hostAddr, hostPort, durableClientId,
      durableClntTimeOut));
}

}  // namespace client
}  // namespace geode
}  // namespace apache