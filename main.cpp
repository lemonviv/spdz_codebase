#include "spdz_connector.h"
#include <Networking/ssl_sockets.h>
#include <iostream>
#include <vector>

void add_computation(int party_num, int party_id,
                     std::vector<int> mpc_tree_port_bases,
                     std::vector<std::string> party_host_names,
                     int public_value_size,
                     const std::vector<int> &public_values,
                     int private_value_size,
                     const std::vector<double> &private_values,
                     std::promise<std::vector<double>> *res) {
  std::vector<ssl_socket *> mpc_sockets(party_num);
  // Here put the whole setup socket code together, as using a function call
  // would result in a problem when deleting the created sockets
  // setup connections from this party to each spdz party socket
  vector<int> plain_sockets(party_num);
  // ssl_ctx ctx(mpc_player_path, "C" + to_string(party_id));
  ssl_ctx ctx("C" + to_string(party_id));
  // std::cout << "correct init ctx" << std::endl;
  ssl_service io_service;
  octetStream specification;
  std::cout << "begin connect to spdz parties" << std::endl;
  std::cout << "party_num = " << party_num << std::endl;
  for (int i = 0; i < party_num; i++) {
    set_up_client_socket(plain_sockets[i], party_host_names[i].c_str(),
                         mpc_tree_port_bases[i] + i);
    send(plain_sockets[i], (octet *)&party_id, sizeof(int));
    mpc_sockets[i] =
        new ssl_socket(io_service, ctx, plain_sockets[i], "P" + to_string(i),
                       "C" + to_string(party_id), true);
    if (i == 0) {
      // receive gfp prime
      specification.Receive(mpc_sockets[0]);
    }
    std::cout << "Set up socket connections for " << i
              << "-th spdz party succeed,"
                 " sockets = "
              << mpc_sockets[i] << ", port_num = " << mpc_tree_port_bases[i] + i
              << "." << std::endl;
  }
  std::cout << "Finish setup socket connections to spdz engines." << std::endl;
  int type = specification.get<int>();
  switch (type) {
  case 'p': {
    gfp::init_field(specification.get<bigint>());
    std::cout << "Using prime " << gfp::pr() << std::endl;
    break;
  }
  default:
    std::cout << "Type " << type << " not implemented" << std::endl;
    exit(1);
  }
  std::cout << "Finish initializing gfp field." << std::endl;

  // send data to spdz parties, for public values, here only party 0 send
  if (party_id == 0) {
    // one party sends tree type and class num to spdz parties
    for (int i = 0; i < public_value_size; i++) {
      std::vector<int> x;
      x.push_back(public_values[i]);
      send_public_values(x, mpc_sockets, party_num);
    }
  }
  // all the parties send private shares
  for (int i = 0; i < private_value_size; i++) {
    vector<double> x;
    x.push_back(private_values[i]);
    send_private_inputs(x, mpc_sockets, party_num);
  }

  // receive result from spdz parties according to the computation type
  std::cout << "receive spdz computation result" << std::endl;
  std::vector<double> return_values = receive_result(mpc_sockets, party_num, 1);
  res->set_value(return_values);

  for (int i = 0; i < party_num; i++) {
    close_client_socket(plain_sockets[i]);
  }

  // free memory and close mpc_sockets
  for (int i = 0; i < party_num; i++) {
    delete mpc_sockets[i];
    mpc_sockets[i] = nullptr;
  }
}

int main(int argc, char *argv[]) {
  std::cout << "Hello, World!" << std::endl;
  int party_num, party_id;
  if (argv[1] != nullptr) {
    party_num = atoi(argv[1]);
  }
  if (argv[2] != nullptr) {
    party_id = atoi(argv[2]);
  }

  // assume that each party provide a secret input
  std::vector<int> public_values;
  public_values.push_back(1);

  // assemble private values, here give one data summation
  std::vector<double> private_values;
  private_values.push_back(1.0 * (party_id + 1));

  std::vector<int> mpc_port_bases;
  std::vector<std::string> mpc_host_names;
  for (int i = 0; i < party_num; i++) {
    mpc_port_bases.push_back(SPDZ_PORT);
    mpc_host_names.emplace_back(SPDZ_HOST);
  }

  // communicate with spdz parties and receive results to compute labels
  // first send computation type, tree type, class num
  // then send private values
  std::promise<std::vector<double>> promise_values;
  std::future<std::vector<double>> future_values = promise_values.get_future();
  std::thread spdz_thread(add_computation, party_num, party_id, mpc_port_bases,
                          mpc_host_names, public_values.size(), public_values,
                          private_values.size(), private_values,
                          &promise_values);
  std::vector<double> res = future_values.get();
  spdz_thread.join();

  // print the result
  for (double v : res) {
    std::cout << "v = " << v << std::endl;
  }

  return 0;
}