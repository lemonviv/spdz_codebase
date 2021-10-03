//
// Created by wuyuncheng on 3/11/20.
//

#ifndef SPDZ_CONNECTOR_H_
#define SPDZ_CONNECTOR_H_

// header files from MP-SPDZ library
#include "Math/Setup.h"
#include "Math/gf2n.h"
#include "Math/gfp.h"
#include "Networking/sockets.h"
#include "Networking/ssl_sockets.h"
#include "Protocols/fake-stuff.h"
#include "Tools/int.h"

#include "common.h"

#include <sodium.h>

#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

/**
 * send public values to spdz parties (i.e., cint or cfloat)
 * currently only int vector is supported
 *
 * @param values: the values that need to be sent to spdz engines
 * @param sockets: the ssl socket connections to spdz engines
 * @param n_parties: number of spdz parties
 */
template <class T>
void send_public_values(std::vector<T> values, vector<ssl_socket *> &sockets,
                        int n_parties) {
  octetStream os;
  int size = values.size();
  vector<gfp> parameters(size);

  if (std::is_same<T, int>::value) {
    for (int i = 0; i < size; i++) {
      parameters[i] = gfpvar(values[i]);
      // parameters[i].assign(&values[i]);
      parameters[i].pack(os);
    }
  } else {
    std::cout
        << "Public values other than int type are not supported, aborting."
        << std::endl;
    // cerr << "Public values other than int type are not supported,
    // aborting.\n";
    exit(1);
  }

  for (int i = 0; i < n_parties; i++) {
    os.Send(sockets[i]);
  }
}

/**
 * send the private values masked with a random share:
 * first receive shares of a preprocessed triple from each spdz engine,
 * then combine and check if the triples are valid,
 * finally send to each spdz engine.
 *
 * @param values: the gfp values that need to be sent to spdz engines
 * @param sockets: the ssl socket connections to spdz engines
 * @param n_parties: number of spdz parties
 */
void send_private_values(std::vector<gfp> values, vector<ssl_socket *> &sockets,
                         int n_parties);

/**
 * send private inputs to the spdz parties with secret sharing
 *
 * @param inputs: the plaintext inputs, int or double
 * @param sockets: the ssl socket connections to spdz engines
 * @param n_parties: number of spdz parties
 */
template <class T>
void send_private_inputs(const std::vector<T> &inputs,
                         vector<ssl_socket *> &sockets, int n_parties) {
  // now only support double type
  if (!std::is_same<T, double>::value) {
    std::cout
        << "Private inputs other than double type are not supported, aborting."
        << std::endl;
    exit(1);
  }

  int size = inputs.size();
  std::vector<int64_t> long_shares(size);

  // step 1: convert to int or long according to the fixed precision
  for (int i = 0; i < size; ++i) {
    long_shares[i] = static_cast<int64_t>(
        round(inputs[i] * pow(2, SPDZ_FIXED_POINT_PRECISION)));
  }

  // step 2: convert to the gfp value and call send_private_inputs
  // Map inputs into gfp
  vector<gfp> input_values_gfp(size);
  for (int i = 0; i < size; i++) {
    bigint::tmp = long_shares[i];
    input_values_gfp[i] = gfpvar(bigint::tmp);
  }

  // call sending values
  send_private_values(input_values_gfp, sockets, n_parties);
}

/**
 * Receive the shares and post-process for the following computations.
 *
 * @param sockets: the ssl sockets of the spdz parties
 * @param nparties: the number of parties
 * @param size: size of recieved results
 */
std::vector<double> receive_result(vector<ssl_socket *> &sockets, int n_parties,
                                   int size);

#endif // SPDZ_CONNECTOR_H_
