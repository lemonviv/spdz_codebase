//
// Created by wuyuncheng on 3/11/20.
//

#include "spdz_connector.h"
#include "common.h"

#include "math.h"

void send_private_values(std::vector<gfp> values, vector<ssl_socket *> &sockets,
                         int n_parties) {
  int num_inputs = values.size();
  octetStream os;
  std::vector<std::vector<gfp>> triples(num_inputs, vector<gfp>(3));
  std::vector<gfp> triple_shares(3);

  // receive num_inputs triples from spdz engines
  for (int j = 0; j < n_parties; j++) {
    os.reset_write_head();
    os.Receive(sockets[j]);

    for (int j = 0; j < num_inputs; j++) {
      for (int k = 0; k < 3; k++) {
        triple_shares[k].unpack(os);
        triples[j][k] += triple_shares[k];
      }
    }
  }

  // check triple relations (is a party cheating?)
  for (int i = 0; i < num_inputs; i++) {
    if (triples[i][0] * triples[i][1] != triples[i][2]) {
      std::cout << "Incorrect triple at " << i << ", aborting." << std::endl;
      // cerr << "Incorrect triple at " << i << ", aborting\n";
      exit(1);
    }
  }

  // send inputs + triple[0], so spdz engines can compute shares of each value
  os.reset_write_head();
  for (int i = 0; i < num_inputs; i++) {
    gfp y = values[i] + triples[i][0];
    y.pack(os);
  }
  for (int j = 0; j < n_parties; j++)
    os.Send(sockets[j]);
}

std::vector<double> receive_result(vector<ssl_socket *> &sockets, int n_parties,
                                   int size) {
  std::cout << "Receive mpc computation result from the SPDZ engine"
            << std::endl;
  std::vector<gfp> output_values(size);
  octetStream os;
  for (int i = 0; i < n_parties; i++) {
    os.reset_write_head();
    os.Receive(sockets[i]);
    for (int j = 0; j < size; j++) {
      gfp value;
      value.unpack(os);
      output_values[j] += value;
    }
  }

  std::vector<double> res_shares(size);

  for (int i = 0; i < size; i++) {
    gfp val = output_values[i];
    bigint aa;
    to_signed_bigint(aa, val);
    long long t = aa.get_si();
    // cout<< "i = " << i << ", t = " << t <<endl;
    res_shares[i] =
        static_cast<double>(t * pow(2, 0 - SPDZ_FIXED_POINT_PRECISION));
  }

  return res_shares;
}