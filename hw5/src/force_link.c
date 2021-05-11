/*
 * A file to force linking of all the modules, even if students commented
 * out all the references to particular modules (or didn't implement the stuff
 * that references them).
 */

#include "protocol.h"
#include "client_registry.h"
#include "client.h"
#include "user_registry.h"
#include "user.h"
#include "mailbox.h"
#include "server.h"
#include "globals.h"

void reference_all_modules(void) {
  void *client_ref = &client_create;
  (void)client_ref;
  void *client_registry_ref = &creg_init;
  (void)client_registry_ref;
  void *mailbox_ref = &mb_init;
  (void)mailbox_ref;
  void *protocol_ref = &proto_send_packet;
  (void)protocol_ref;
  void *server_ref = &chla_client_service;
  (void)server_ref;
  void *user_ref = &user_create;
  (void)user_ref;
  void *user_registry_ref = &ureg_init;
  (void)user_registry_ref;
}
